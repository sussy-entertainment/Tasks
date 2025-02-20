int ledPins[] = {13, 12, 11, 10, 9, 8, 7};
int delayTime = 350;
int level = 1;
bool greenLed = false;
bool gameOver = false;

void buttonPush();

void setup() {
  pinMode(3, INPUT_PULLUP);
  for (int i = 0; i < 7; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  attachInterrupt(digitalPinToInterrupt(3), buttonPush, FALLING);
  Serial.begin(9600);
}

void loop() {
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
      for (int i = 0; i < 7; i++) {
        digitalWrite(ledPins[i], LOW);
      }
      delay(200);
    }
    level = 1;
    delayTime = 350;
    gameOver = false;
    delay(500);
  }

  if (!gameOver) { //turn LEDs on in order
    for (int i = 0; i < 7; i++) {
      digitalWrite(ledPins[i], HIGH);
      if (i > 4) { 
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
  if (!gameOver) { //turn LEDs off in reverse order
    for (int i = 7; i > -1; i--) {
      Serial.print(""); //without this, code don't work. Explanation is left as an excercise for the reader
      if (gameOver) {
        break; // Exit loop immediately if game over
      }
      digitalWrite(ledPins[i], LOW);
      if (i > 4) { 
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