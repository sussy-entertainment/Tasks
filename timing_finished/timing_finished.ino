#include <SPI.h>
#include <MFRC522.h>
#include <vector>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#define SS_PIN 5  
#define RST_PIN 9  

MFRC522 rfid(SS_PIN, RST_PIN);
int ledPins[] = {3, 4, 10, 1, 19, 18};  
int buttonPin = 8;  
int delayTime = 350;
int level = 1;
int assigned = 0;
bool greenLed = false;
bool gameOver = false;
bool gameStart = false;
std::vector<String> game_pause = {""};
String approvedTags[6] = {"color_1", "color_2", "color_3", "color_4", "color_5", "color_6"}; // List of approved tag data

String color = {};

// WiFi
const char *ssid = "BYUI_Visitor"; // Enter your Wi-Fi name
const char *password = "";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "gigachad.website";
const char *topic = "timing";
const char *topic1 = "timing_done";
const char *mqtt_username = "amogus";
const char *mqtt_password = "sussyimposter";
const int mqtt_port = 8883;
char *finishedTags = "";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void IRAM_ATTR buttonPush();

void setup() {
  Serial.begin(115200);
  SPI.begin(6, 2, 7, 5);  
  rfid.PCD_Init();

  Serial.println("Tap RFID/NFC Tag on reader");

  pinMode(buttonPin, INPUT_PULLUP);  
  for (int i = 0; i < 6; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW); // Ensure LEDs start off
  }
  attachInterrupt(buttonPin, buttonPush, FALLING);
  espClient.setInsecure();
  // Connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // Publish and subscribe
    client.publish(topic, "Hi, I'm ESP32 ^^");
    client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    if (length == 1) {
      approvedTags[assigned] = (char) payload[0];
      Serial.println(approvedTags[assigned]);
      assigned += 1;
    }
    else{
      game_pause[0] = (char) payload[0];
      if (game_pause[0] == "m") {
        Serial.println("meeting");
        color = "";
        gameOver = true;
      }
      if (game_pause[0] == "g") {
        gameOver = true;
        color = "";
        for (int i = 0; i < 7; i++){
          approvedTags[i] = "";
        }
      }
    }
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}
void loop() {
  if (!gameStart) {
  checkRFID();
  }
  if (gameOver) {
    Serial.print("You Reached Level: ");
    Serial.println(level);
    gameStart = false;
    delay(3000);
     
    for (int j = 0; j < 3; j++) {
      for (int i = 0; i < 6; i++) digitalWrite(ledPins[i], HIGH);
      delay(200);
      for (int i = 0; i < 6; i++) digitalWrite(ledPins[i], LOW);
      delay(200);
    }
    
    level = 1;
    delayTime = 350;
    gameOver = false;
    delay(500);
  }
  if (level == 11){
    client.publish(topic1, color.c_str());
    color = "";
    for (int i = 0; i < 6; i++) digitalWrite(ledPins[i], HIGH);
    delay(5000);
    for (int i = 0; i < 6; i++) digitalWrite(ledPins[i], LOW);

    gameOver = true;
  }
  if (!gameOver && gameStart) {
    for (int i = 0; i < 6; i++) {
      digitalWrite(ledPins[i], HIGH);
      greenLed = (i > 3);
      delay(delayTime);
      digitalWrite(ledPins[i], LOW);  // Turn off LED immediately
      if (gameOver) break;
    }
  }
}

void checkRFID() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    MFRC522::StatusCode status;
    byte buffer[18];  // Standard MIFARE block size (16 bytes + CRC)
    byte size = sizeof(buffer);

    status = rfid.MIFARE_Read(4, buffer, &size); // Read block 4

    if (status == MFRC522::STATUS_OK) {
      
      // Convert to a clean string
      String tagData = "";
      for (byte i = 0; i < size; i++) {
        if (isPrintable(buffer[i])) { // Keep only readable characters
          tagData += (char)buffer[i];
        }
      }

      // Remove leading/trailing whitespace or junk characters
      tagData.trim();
      color = tagData[3];

      // Validate expected value
      for (int i = 0; i < 7; i++) {
      if (color == approvedTags[i]) {
        gameStart = true;
        Serial.println("Authorized tag detected. Game started.");
      } else {
        Serial.println("Unauthorized tag.");
      }
    }
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}

void IRAM_ATTR buttonPush() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 300) {
    if (greenLed) {
      delayTime = delayTime * 9 / 10;
      level++;
    } else {
      gameOver = true;
    }
    last_interrupt_time = interrupt_time;
  }
}