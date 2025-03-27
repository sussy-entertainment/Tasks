#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5  
#define RST_PIN 9  

MFRC522 rfid(SS_PIN, RST_PIN);
byte authorizedUID[4] = {0x69, 0x84, 0xC6, 0xB1}; // Authorized RFID UID

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

String color = {};

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
}
}
else {
  digitalWrite(18, LOW);
  digitalWrite(4, LOW);
  digitalWrite(10, LOW);
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
      if (color == "r") {
        game_start = true;
        Serial.println("Authorized tag detected. Game started.");
      } else {
        Serial.println("Unauthorized tag.");
      }
    } else {
      Serial.println("Failed to read data from tag.");
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}
