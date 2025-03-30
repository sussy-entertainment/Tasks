#include <SPI.h>
#include <MD_MAX72xx.h>
#include <MFRC522.h>
#include <vector>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>


// ESP32-C3 MINI-DK Pinout based on the provided image
#define SS_RFID 10      // RFID Chip Select (SS) - GPIO10
#define RST_PIN 4       // RFID Reset Pin - GPIO4
#define CS_MAX72XX 18   // MAX7219 Chip Select (SS) - GPIO18 (USB_D+)
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 1   // Number of cascated MAX7219 modules

MFRC522 rfid(SS_RFID, RST_PIN);

// Define SPI and LED Matrix Pins for ESP32-C3
#define DIN_PIN 7       // MOSI - GPIO7
#define CLK_PIN 6       // SCK - GPIO6
#define MISO_PIN 5      // MISO - GPIO5 (added for RFID)

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_MAX72XX, MAX_DEVICES);

// Symbol patterns (8x8)
byte s1[8] = {0x00, 0x3C, 0x42, 0x5A, 0x5A, 0x42, 0x00, 0x00};
byte s2[8] = {0x00, 0x00, 0x42, 0x5A, 0x5A, 0x42, 0x3C, 0x00};
byte s3[8] = {0x00, 0x4C, 0x52, 0x61, 0x01, 0x61, 0x52, 0x4C};
byte s4[8] = {0x00, 0x00, 0x0E, 0x11, 0x71, 0x11, 0x0E, 0x00};
byte s5[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
byte s6[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Button Pins (ESP32-C3) based on the image provided
int button_pins[] = {8, 9, 2, 3}; // GPIO8, GPIO9, GPIO2, GPIO3

int symbol = 0;
int previous_symbol = 0;
int pressed_button = 0;
int score = 0;
int assigned = 0;
bool game_start = false;
String last_tag = {};
std::vector<String> game_pause = {""};
String approvedTags[6] = {"color_1", "color_2", "color_3", "color_4", "color_5", "color_6"}; // List of approved tag data
String scannedTags[2] = {"", ""}; // Store the last two different scanned tags

// WiFi
const char *ssid = "BYUI_Visitor"; // Enter your Wi-Fi name
const char *password = "";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "gigachad.website";
const char *topic = "co-op";
const char *topic1 = "co-op_done";
const char *mqtt_username = "amogus";
const char *mqtt_password = "sussyimposter";
const int mqtt_port = 8883;
char *finishedTags = "";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);


  SPI.begin(CLK_PIN, MISO_PIN, DIN_PIN, SS_RFID); // Initialize SPI with the correct pins

  pinMode(SS_RFID, OUTPUT);
  pinMode(CS_MAX72XX, OUTPUT);
  digitalWrite(SS_RFID, HIGH);
  digitalWrite(CS_MAX72XX, HIGH);

  rfid.PCD_Init();

  for (int i = 0; i < 4; i++) {
    pinMode(button_pins[i], INPUT_PULLUP); // Set button pins as input with pull-up resistors
    Serial.print(button_pins[i]);
  }

  randomSeed(analogRead(0)); // Use an unconnected analog pin for entropy

  mx.begin();
  mx.clear();

  symbol = random(1, 5);

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
        scannedTags[0] = "";
        scannedTags[1] = "";
        game_start = false;
        displaySymbol(5);
      }
      if (game_pause[0] == "g") {
        game_start = false;
        score = 0;
        displaySymbol(5);
        scannedTags[0] = "";
        scannedTags[1] = "";
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
  client.loop();
  if (!game_start) {
    checkRFID(); // Only check RFID if game is not running
  } else {
    playGame(); // Play game if game is running
    if (score >= 10) {
      displaySymbol(5);
      game_start = false;
      for (int i = 0; i < 2; i++) {
    if (!scannedTags[i].isEmpty()) {
 
      client.publish(topic1, approvedTags[i].c_str());
    }
  }
      score = 0;
      for (int i = 0; i < 20; i++){
      displaySymbol(6);
      delay(500);
      displaySymbol(5);
      scannedTags[0] = "";
      scannedTags[1] = "";
      }
      Serial.println("Game won! Score reset. Waiting for RFID scan to start new game.");
    }
  }
}


void checkRFID() {
  digitalWrite(CS_MAX72XX, HIGH);
  digitalWrite(SS_RFID, LOW);
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    MFRC522::StatusCode status;
    byte buffer[18];  // Standard MIFARE block size (16 bytes + CRC)
    byte size = sizeof(buffer);

    status = rfid.MIFARE_Read(4, buffer, &size); // Read block 4

    if (status == MFRC522::STATUS_OK) {
      Serial.print("Raw Data from tag: ");
      
      // Convert to a clean string
      String tagData = "";
      for (byte i = 0; i < size; i++) {
        if (isPrintable(buffer[i])) { // Keep only readable characters
          tagData += (char)buffer[i];
        }
      }

      // Remove leading/trailing whitespace or junk characters
      tagData.trim();
      Serial.println(tagData);
      last_tag = tagData[3];
      // Check if scanned tag is in the approved list
      bool isApproved = false;
      for (const String& tag : approvedTags) {
        if (last_tag == tag) {
          isApproved = true;
          break;
        }
      }

      if (isApproved) {
        // If the tag is different from the last stored tag, store it
        if (tagData != scannedTags[0] || scannedTags[0] != "") {
          scannedTags[1] = scannedTags[0]; // Move previous tag to second slot
          scannedTags[0] = tagData;        // Store new tag

          Serial.println("Valid tag scanned.");
          
          // If two different tags are scanned, start the game
          if (scannedTags[0] != scannedTags[1] && scannedTags[1] != "") {
            game_start = true;
            Serial.println("Two different approved tags scanned. Game started.");
          }
        } else {
          Serial.println("Same tag scanned again, waiting for a different one.");
        }
      } else {
        Serial.println("Unauthorized tag.");
      }
    } else {
      Serial.println("Failed to read data from tag.");
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
  digitalWrite(SS_RFID, HIGH);
}

void playGame() {
  digitalWrite(SS_RFID, HIGH);  // Disable RFID
  digitalWrite(CS_MAX72XX, LOW); // Enable MAX7219
  displaySymbol(symbol);

  pressed_button = 0;
  for (int i = 0; i < 4; i++) {
    int buttonState = digitalRead(button_pins[i]);
    if (buttonState == LOW) {
      pressed_button = i + 1;
      Serial.print("Button ");
      Serial.print(pressed_button);
      Serial.println(" pressed.");
      delay(400); // debounce
    }
  }

  if (pressed_button != 0) {
    if (pressed_button == symbol) {
      score++;
      Serial.print("Correct! Score: ");
      Serial.println(score);
      previous_symbol = symbol;
      symbol = random(1, 5);
      while (previous_symbol == symbol) {
        symbol = random(1, 5);
      }
      Serial.print("New symbol: ");
      Serial.println(symbol);
      delay(500);
    } else {
      score = 0;
      Serial.println("Wrong choice! Score reset.");
      Serial.println(pressed_button);
      delay(500);
    }
  }

  digitalWrite(CS_MAX72XX, HIGH); // Disable MAX7219
}

void displaySymbol(int symbol) {
  byte* symArray;
  switch (symbol) {
    case 1: symArray = s1; break;
    case 2: symArray = s2; break;
    case 3: symArray = s3; break;
    case 4: symArray = s4; break;
    case 5: symArray = s5; break;
    case 6: symArray = s6; break;
    default: return;
  }

  for (int i = 0; i < 8; i++) {
    mx.setRow(0, i, symArray[i]);
  }

  mx.update();
}