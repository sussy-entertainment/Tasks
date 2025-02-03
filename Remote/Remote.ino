#include "IRremote.h"

int receiver = 11; // Signal Pin of IR receiver to Arduino Digital Pin 11
long digitArray[6]; // Array to store digits
long finalNumber;

int currentDigitIndex = 0; // Index for current digit

/*-----( Declare objects )-----*/
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'

void setup()   /*----( SETUP: RUNS ONCE )----*/
{
  Serial.begin(9600);
  Serial.println("IR Receiver Button Decode"); 
  irrecv.enableIRIn(); // Start the receiver

}/*--(end setup )---*/


void loop()   /*----( LOOP: RUNS CONSTANTLY )----*/
{
  if (irrecv.decode(&results)) // have we received an IR signal?

  {
    translateIR(); 
    irrecv.resume(); // receive the next value
  }  
}/* --(end main loop )-- */

/*-----( Function )-----*/
void translateIR() // takes action based on IR code received

// describing Remote IR codes 

{

  switch(results.value)

  {


  case 0xFF6897: digitArray[currentDigitIndex] = 0; // Store digit 0
                 currentDigitIndex++;
                 finalNumber = calculateFinalNumber(digitArray, currentDigitIndex);
                 Serial.println(finalNumber);    break;
  case 0xFF30CF: digitArray[currentDigitIndex] = 1; // Store digit 0
                 currentDigitIndex++;
                 finalNumber = calculateFinalNumber(digitArray, currentDigitIndex);
                 Serial.println(finalNumber);    break;
  case 0xFF18E7: digitArray[currentDigitIndex] = 2; // Store digit 0
                 currentDigitIndex++;
                 finalNumber = calculateFinalNumber(digitArray, currentDigitIndex);
                 Serial.println(finalNumber);    break;
  case 0xFF7A85: digitArray[currentDigitIndex] = 3; // Store digit 0
                 currentDigitIndex++;
                 finalNumber = calculateFinalNumber(digitArray, currentDigitIndex);
                 Serial.println(finalNumber);    break;
  case 0xFF10EF: digitArray[currentDigitIndex] = 4; // Store digit 0
                 currentDigitIndex++;
                 finalNumber = calculateFinalNumber(digitArray, currentDigitIndex);
                 Serial.println(finalNumber);    break;
  case 0xFF38C7: digitArray[currentDigitIndex] = 5; // Store digit 0
                 currentDigitIndex++;
                 finalNumber = calculateFinalNumber(digitArray, currentDigitIndex);
                 Serial.println(finalNumber);    break;
  case 0xFF5AA5: digitArray[currentDigitIndex] = 6; // Store digit 0
                 currentDigitIndex++;
                 finalNumber = calculateFinalNumber(digitArray, currentDigitIndex);
                 Serial.println(finalNumber);    break;
  case 0xFF42BD: digitArray[currentDigitIndex] = 7; // Store digit 0
                 currentDigitIndex++;
                 finalNumber = calculateFinalNumber(digitArray, currentDigitIndex);
                 Serial.println(finalNumber);    break;
  case 0xFF4AB5: digitArray[currentDigitIndex] = 8; // Store digit 0
                 currentDigitIndex++;
                 finalNumber = calculateFinalNumber(digitArray, currentDigitIndex);
                 Serial.println(finalNumber);    break;
  case 0xFF52AD: digitArray[currentDigitIndex] = 9; // Store digit 0
                 currentDigitIndex++;
                 finalNumber = calculateFinalNumber(digitArray, currentDigitIndex);
                 Serial.println(finalNumber);    break;
  case 0xFFA25D: currentDigitIndex = 0; 
                 memset(digitArray, 0, sizeof(digitArray)); break;  

  }// End Case

  delay(500); // Do not get immediate repeat
  

} 
long calculateFinalNumber(long digits[], int numDigits) {

  long result = 0;
  for (int i = 0; i < numDigits; i++) {
    long power = 1;
    for(int j = 0; j <numDigits -i -1; j++){
       power = 10 * power;
    }
    
    result += digits[i] * power; 

  }

  return result;

}