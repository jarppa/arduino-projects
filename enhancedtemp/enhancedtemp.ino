#include <VirtualWire.h>
#include <ShiftDisplay.h>

const uint8_t latchPin = 1;
const uint8_t clockPin = 2;
const uint8_t dataPin = 0;
const uint8_t potPin = 5;
const uint8_t tempPin = 4;

ShiftDisplay lcd(latchPin, dataPin, clockPin);

void setup() {
  /*pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);*/
  pinMode(potPin, INPUT);
  pinMode(tempPin, OUTPUT);
  digitalWrite(tempPin, LOW);
  
  //vw_setup(2000);	 // Bits per sec  
}

static int potval = 0;
void loop() {
  digitalWrite(tempPin, LOW);
  delay(500);
  const char *msg = "hello";
  
  //vw_send((uint8_t *)msg, strlen(msg));
  //vw_wait_tx(); // Wait until the whole message is gone
  
  potval = analogRead(potPin);
  lcd.setCursor(0,1);
  lcd.print(potval);
  digitalWrite(tempPin, HIGH);
  delay(500);
}
