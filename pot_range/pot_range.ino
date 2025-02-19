int led_1 = 0;
int led_2 = 0;
int led_3 = 0;

void setup() {
Serial.begin(9600);
pinMode(A3, INPUT);
pinMode(A4,  INPUT);
pinMode(A5, INPUT);

pinMode(11,  OUTPUT);
pinMode(10, OUTPUT);
pinMode(9, OUTPUT);

digitalWrite(11,  LOW);
digitalWrite(10, LOW);
digitalWrite(9, LOW);
}

void loop() {
  led_1 = analogRead(A3) * (5.0 / 1023.0);
  led_2 = analogRead(A4) * (5.0 / 1023.0);
  led_3 = analogRead(A5) * (5.0 / 1023.0);

if (led_1 < 5 && led_1 > 3) {
  digitalWrite(11, HIGH);
}
else {
  digitalWrite(11, LOW);
}

if (led_2 < 4 && led_2 > 2) {
  digitalWrite(10, HIGH);
}
else {
  digitalWrite(10, LOW);
}

if (led_3 < 4 && led_3 > 2) {
  digitalWrite(9, HIGH);
}
else {
  digitalWrite(9, LOW);
}

}