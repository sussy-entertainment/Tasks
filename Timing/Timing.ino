
int ledPins[] = {13, 12, 11, 10, 9, 8, 7}; // Have used Analog Pins as this was easier to set up on Arduino
int delayTime = 250; // Starting Led Speed
int level = 1; // Start at Level 1
boolean greenLed = false; // Boolean to define when greenLed is on
boolean gameOver = false; // Boolean to define when Game Over

//void buttonPush(); // Function for interrupt

void setup() {           
  pinMode(2, INPUT);
  for(int i=0; i<7; i++){
  pinMode(ledPins[i], OUTPUT);
  }
  attachInterrupt(digitalPinToInterrupt(2), buttonPush, FALLING); 
  Serial.begin(9600);
}

void loop() {
  if(gameOver == true){ // Game Over sequence
    Serial.print("You Reached Level: "); // Display level reached
    Serial.println(level);
    delay(3000);
    for(int i=0; i<7; i++){ // Flash all Leds to start Game
    digitalWrite(ledPins[i], LOW);
    }
    delay(200);
    for(int i=0; i<7; i--){
    digitalWrite(ledPins[i], HIGH);
    }
    delay(200);
    for(int i=0; i<7; i++){
    digitalWrite(ledPins[i], LOW);
    }
    Serial.print("where");
    delay(200);
    for(int i=0; i<7; i++){
    digitalWrite(ledPins[i], HIGH);
    }
    delay(200);
    for(int i=0; i<7; i++){
    digitalWrite(ledPins[i], LOW);
    }
    delay(200);
    for(int i=0; i<7; i++){
    digitalWrite(ledPins[i], HIGH);
    }
    delay(200);
    for(int i=0; i<7; i++){
    digitalWrite(ledPins[i], LOW);
    }
    Serial.print("here");
    level = 1; // Reset Level to 1
    delayTime = 250; // Starting delay time
    gameOver = false; // Set Game Over to false
    delay(500);
   }
   
   
  if(gameOver == false){ // Check Game Over indicator before each Led (this ensures Game ends immediately as an interrupt returns to the position it left off)
    digitalWrite(13, HIGH);
    delay(delayTime);
  }   
  if(gameOver == false){
    digitalWrite(12, HIGH);
    delay(delayTime);
  }
  if(gameOver == false){
  digitalWrite(11, HIGH);
    delay(delayTime);
  }
  if(gameOver == false){
  digitalWrite(10, HIGH);
    delay(delayTime);
  }
  if(gameOver == false){
  digitalWrite(9, HIGH);
    delay(delayTime);
  }
  if(gameOver == false){
  greenLed = true; // Set greenLed True to use during interrupt (correct hit)
  digitalWrite(8, HIGH);
    delay(delayTime);
  }
  if(gameOver == false){
  digitalWrite(7, HIGH);
    delay(delayTime);
  }
  if(gameOver == false){
  digitalWrite(7, LOW);
    delay(delayTime);
  }
  if(gameOver == false){
  digitalWrite(8, LOW);
  greenLed = false; // Set greenLed False to use during interrupt (incorrect hit)
    delay(delayTime);
  }
  if(gameOver == false){
  digitalWrite(9, LOW);
    delay(delayTime);
  }
  if(gameOver == false){
  digitalWrite(10, LOW);
    delay(delayTime);
  }
  if(gameOver == false){
  digitalWrite(11, LOW); 
    delay(delayTime);
  }
  if(gameOver == false){
  digitalWrite(12, LOW); 
  }
}

void buttonPush(){ // Button push interrupt
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 300){ // Debounce
    if(greenLed == true){ // If Button pushed during greenLed True
      delayTime = delayTime * 9 / 10; // Increase Delaytime by 10%
      level++; // Increase level by 1
    }
    else{
     gameOver = true; // If greenLed is not true, it must be false, hence set Game Over as True
    }
    last_interrupt_time = interrupt_time;
  }
}
