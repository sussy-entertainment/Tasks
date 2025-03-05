#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 10
#define RST_PIN A0

MFRC522 rfid(SS_PIN, RST_PIN);
int ledPins[] = {9, 8, 7, 6, 5, 4};
int delayTime = 350;
int level = 1;
bool greenLed = false;
bool gameOver = false;
bool gameStart = false;
void buttonPush();
byte authorizedUID[4] = {0x69, 0x84, 0xC6, 0xB1};

void setup() {
  Serial.begin(9600);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522

  Serial.println("Tap RFID/NFC Tag on reader");
  pinMode(3, INPUT_PULLUP);
  for (int i = 0; i < 7; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  attachInterrupt(digitalPinToInterrupt(3), buttonPush, FALLING);
}

void loop() {
if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

      if (rfid.uid.uidByte[0] == authorizedUID[0] &&
          rfid.uid.uidByte[1] == authorizedUID[1] &&
          rfid.uid.uidByte[2] == authorizedUID[2] &&
          rfid.uid.uidByte[3] == authorizedUID[3] ) {
            gameStart = true;
          }
           rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
  if (gameOver) {
    Serial.print("You Reached Level: ");
    Serial.println(level);
    delay(3000);
    // Flash all LEDs 
    for (int j = 0; j < 3; j++) { // Flash 3 times
      for (int i = 0; i < 7; i++) {
        digitalWrite(ledPins[i], HIGH);
      }
      delay(200);
      for (int i = 0; i < 6; i++) {
        digitalWrite(ledPins[i], LOW);
      }
      delay(200);
    }
    level = 1;
    delayTime = 350;
    gameOver = false;
    delay(500);
  }

  if (!gameOver && gameStart) { //turn LEDs on in order
    for (int i = 0; i < 6; i++) {
      digitalWrite(ledPins[i], HIGH);
      if (i > 3) { 
        greenLed = true;
      } else {
        greenLed = false;
      }
      delay(delayTime);
      Serial.print(""); //without this, code don't work. Explanation is left as an excercise for the reader
      if (gameOver) {
        break; // Exit loop immediately if game over
      }
    }
  }
  if (!gameOver && gameStart) { //turn LEDs off in reverse order
    for (int i = 6; i > -1; i--) {
      Serial.print(""); //without this, code don't work. Explanation is left as an excercise for the reader
      if (gameOver) {
        break; // Exit loop immediately if game over
      }
      digitalWrite(ledPins[i], LOW);
      if (i > 3) { 
        greenLed = true;
      } else {
        greenLed = false;
      }
      delay(delayTime);
    }
  }
}

void buttonPush() {
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