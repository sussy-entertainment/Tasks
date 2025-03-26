#include <SPI.h>
#include <MD_MAX72xx.h>
#include <MFRC522.h>

// ESP32-C3 MINI-DK Pinout based on the provided image
#define SS_RFID 10      // RFID Chip Select (SS) - GPIO10
#define RST_PIN 4       // RFID Reset Pin - GPIO4
#define CS_MAX72XX 18   // MAX7219 Chip Select (SS) - GPIO18 (USB_D+)
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 1   // Number of cascated MAX7219 modules

MFRC522 rfid(SS_RFID, RST_PIN);
byte authorizedUID[4] = {0x69, 0x84, 0xC6, 0xB1};

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

// Button Pins (ESP32-C3) based on the image provided
int button_pins[] = {8, 9, 2, 3}; // GPIO8, GPIO9, GPIO2, GPIO3

int symbol = 0;
int previous_symbol = 0;
int pressed_button = 0;
int score = 0;
bool game_start = false;

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

}

void loop() {
  if (!game_start) {
    checkRFID(); // Only check RFID if game is not running
  } else {
    playGame(); // Play game if game is running
    if (score >= 10) {
      symbol = 5;
      displaySymbol(symbol);
      game_start = false;
      score = 0;
      Serial.println("Game won! Score reset. Waiting for RFID scan to start new game.");
    }
  }
}

void checkRFID() {
  digitalWrite(CS_MAX72XX, HIGH); // Disable MAX7219
  digitalWrite(SS_RFID, LOW);    // Enable RFID

  if (rfid.PICC_IsNewCardPresent()) {
    Serial.println("New card present.");
    if (rfid.PICC_ReadCardSerial()) {
      Serial.println("Card serial read.");

      // Print the UID
      Serial.print("UID: ");
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      // Check if UID matches
      if (memcmp(rfid.uid.uidByte, authorizedUID, 4) == 0) {
        game_start = true;
        delay(500);
        Serial.println("Authorized card detected. Game started.");
      } else {
        Serial.println("Unauthorized card.");
      }

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    } else {
      Serial.println("Failed to read card serial.");
    }
  }

  digitalWrite(SS_RFID, HIGH); // Disable RFID
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
    default: return;
  }

  for (int i = 0; i < 8; i++) {
    mx.setRow(0, i, symArray[i]);
  }

  mx.update();
}