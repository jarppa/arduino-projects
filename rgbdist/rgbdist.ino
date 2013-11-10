/*
    Basic Pin setup:
    ------------                                  ---u----
    ARDUINO   13|-> SCLK (pin 25)           OUT1 |1     28| OUT channel 0
              12|                           OUT2 |2     27|-> GND (VPRG)
              11|-> SIN (pin 26)            OUT3 |3     26|-> SIN (pin 11)
              10|-> BLANK (pin 23)          OUT4 |4     25|-> SCLK (pin 13)
               9|-> XLAT (pin 24)             .  |5     24|-> XLAT (pin 9)
               8|                             .  |6     23|-> BLANK (pin 10)
               7|                             .  |7     22|-> GND
               6|                             .  |8     21|-> VCC (+5V)
               5|                             .  |9     20|-> 2K Resistor -> GND
               4|                             .  |10    19|-> +5V (DCPRG)
               3|-> GSCLK (pin 18)            .  |11    18|-> GSCLK (pin 3)
               2|                             .  |12    17|-> SOUT
               1|                             .  |13    16|-> XERR
               0|                           OUT14|14    15| OUT channel 15
    ------------                                  --------

    -  Put the longer leg (anode) of the LEDs in the +5V and the shorter leg
         (cathode) in OUT(0-15).
    -  +5V from Arduino -> TLC pin 21 and 19     (VCC and DCPRG)
    -  GND from Arduino -> TLC pin 22 and 27     (GND and VPRG)
    -  digital 3        -> TLC pin 18            (GSCLK)
    -  digital 9        -> TLC pin 24            (XLAT)
    -  digital 10       -> TLC pin 23            (BLANK)
    -  digital 11       -> TLC pin 26            (SIN)
    -  digital 13       -> TLC pin 25            (SCLK)
    -  The 2K resistor between TLC pin 20 and GND will let ~20mA through each
       LED.  To be precise, it's I = 39.06 / R (in ohms).  This doesn't depend
       on the LED driving voltage.
    - (Optional): put a pull-up resistor (~10k) between +5V and BLANK so that
                  all the LEDs will turn off when the Arduino is reset.

    If you are daisy-chaining more than one TLC, connect the SOUT of the first
    TLC to the SIN of the next.  All the other pins should just be connected
    together:
        BLANK on Arduino -> BLANK of TLC1 -> BLANK of TLC2 -> ...
        XLAT on Arduino  -> XLAT of TLC1  -> XLAT of TLC2  -> ...
    The one exception is that each TLC needs it's own resistor between pin 20
    and GND.

    This library uses the PWM output ability of digital pins 3, 9, 10, and 11.
    Do not use analogWrite(...) on these pins.

    This sketch does the Knight Rider strobe across a line of LEDs.

    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-03 */

#include "Tlc5940.h"

const int trigPin = 8;
const int echoPin = 7;

const int RED = 1;
const int GREEN = 2;
const int BLUE = 3;

void setup()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
  Serial.begin(9600);
  /* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
  Tlc.init(0);
}

/* This loop will create a Knight Rider-like effect if you have LEDs plugged
   into all the TLC outputs.  NUM_TLCS is defined in "tlc_config.h" in the
   library folder.  After editing tlc_config.h for your setup, delete the
   Tlc5940.o file to save the changes. */

void loop()
{
  long dist = measureDistance();
  //Serial.println(dist);
  updateColors(dist);
  
  delay(100);
}
static int LED_COUNT = 5;

void updateColors(long d) { 
  int i = 0;
  Tlc.clear();
  //Serial.println(map(d,400,4,0,4095));
  for (i=0;i<LED_COUNT*3;i+=3) {
    
    if (d > 4) {
      Tlc.set(i, map(d,400,4,0,4095));
      Tlc.set(i+1, map(d,4,400,0,4095));
      Tlc.set(i+2, map(d,4,400,0,4095));
    }
    else {
      Tlc.set(i, 4095);
      Tlc.set(i+1, 0);
      Tlc.set(i+2, 0);
    }
    
  }
  Tlc.update();
}

static long RANGE_ECHO_MAX = 23200;

long measureDistance() {
  float d = 0;
  long trig_t,echo_t,diff = 0;
  float result  = 0;
  int i,meas = 0;
  //Serial.println("Measure");
  
  for (i=0;i<5;i++) {
    //Serial.print("for:");Serial.println(i);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
  
    while ( digitalRead(echoPin) == LOW) ;
    trig_t = micros();
  
    while (digitalRead(echoPin) == HIGH) ;
    echo_t = micros();
    diff = echo_t-trig_t;
    
    if (diff < 0 || diff > RANGE_ECHO_MAX){
      Serial.print("Value error:");Serial.println(diff);
      delay(100);
      continue;
    }
    meas++;
    
    d += diff/58.0;
    Serial.println(d/(meas));
    delay(100);
  }
  if (meas > 0)
    return (long)(d/meas);
  else
    return long(400);
}
