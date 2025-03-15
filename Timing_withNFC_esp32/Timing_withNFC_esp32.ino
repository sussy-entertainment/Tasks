#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5  
#define RST_PIN 9  

MFRC522 rfid(SS_PIN, RST_PIN);
int ledPins[] = {3, 4, 10, 1, 19, 18};  
int buttonPin = 8;  
int delayTime = 350;
int level = 1;
bool greenLed = false;
bool gameOver = false;
bool gameStart = false;

byte authorizedUID[4] = {0x69, 0x84, 0xC6, 0xB1};

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
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    if (memcmp(rfid.uid.uidByte, authorizedUID, 4) == 0) {
      gameStart = true;
      Serial.println("Game start");
    }
    delay(100);  // Prevent immediate halt issues
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
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