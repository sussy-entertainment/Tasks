#include <SPI.h>
#include <MFRC522.h>
#include <vector>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#define SS_PIN 5  
#define RST_PIN 9  

MFRC522 rfid(SS_PIN, RST_PIN);

int led_1 = 0;
int led_2 = 0;
int led_3 = 0;
int range_1 = 0;
int range_2 = 0;
int range_3 = 0;
int ledPins[] = {10, 4, 18};
int potPins[] = {1, 0, 3};
bool game_start = false;
bool pot_1 = false;
bool pot_2 = false;
bool pot_3 = false;
int assigned = 0;
std::vector<String> game_pause = {""};
String approvedTags[6] = {"color_1", "color_2", "color_3", "color_4", "color_5", "color_6"}; // List of approved tag data

String color = {};
// WiFi
const char *ssid = "BYUI_Visitor"; // Enter your Wi-Fi name
const char *password = "";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "gigachad.website";
const char *topic = "range";
const char *topic1 = "range_done";
const char *mqtt_username = "amogus";
const char *mqtt_password = "sussyimposter";
const int mqtt_port = 8883;
char *finishedTags = "";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  SPI.begin(6, 2, 7, 5);  
  rfid.PCD_Init();

  Serial.println("Tap RFID/NFC Tag on reader");

 
  for (int i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW); // Ensure LEDs start off
  }
 
  for (int i = 0; i < 3; i++) {
    pinMode(potPins[i], INPUT);
  }
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
        digitalWrite(18, LOW);
        digitalWrite(4, LOW);
        digitalWrite(10, LOW);
        game_start = false;
      }
      if (game_pause[0] == "g") {
        game_start = false;
        color = "";
        digitalWrite(18, LOW);
        digitalWrite(4, LOW);
        digitalWrite(10, LOW);
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
  if(!game_start) {
    checkRFID();
  }
if (game_start == true) {
  led_1 = analogRead(1) * (5.0 / 1023.0);
  led_2 = analogRead(0) * (5.0 / 1023.0);
  led_3 = analogRead(3) * (5.0 / 1023.0);

if (led_1 < range_1 && led_1 > (range_1-3)) {
  digitalWrite(10, HIGH);
  pot_1 = true;
}
else {
  digitalWrite(10, LOW);
  pot_1 = false;
}

if (led_2 < range_2 && led_2 > (range_2-3)) {
  digitalWrite(4, HIGH);
  pot_2 = true;
}
else {
  digitalWrite(4, LOW);
  pot_2 = false;
}

if (led_3 < range_3 && led_3 > (range_3-3)) {
  digitalWrite(18, HIGH);
  pot_3 = true;
}
else {
  digitalWrite(18, LOW);
  pot_3 = false;
}
if (pot_1 && pot_2 && pot_3){
  delay(3000);
  game_start = false;
  digitalWrite(18, LOW);
  digitalWrite(4, LOW);
  digitalWrite(10, LOW);
  client.publish(topic1, color.c_str());
  color = "";
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
        game_start = true;
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

