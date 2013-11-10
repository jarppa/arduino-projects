#include <OneWire.h>

#define APP_STATE_IDLE 1
#define APP_STATE_HEATING 2
#define APP_STATE_ADJUST_HEAT 3
#define APP_STATE_COOLING 4
#define APP_STATE_ADJUST_WAIT 5

#define TEMP_SENSOR_PIN 8

#define HOT_LED_PIN 7
#define COLD_LED_PIN 6
#define RIGHT_LED_PIN 5

#define SSR_PIN 10
//#define BUTTON_PIN 2
//#define MODE_SWITCH_PIN 7

#define POT_PIN 1

#define TEMP_ERROR -999

#define TEMP_IND_RANGE 5

OneWire  ds(TEMP_SENSOR_PIN);
byte addr[8];
 
static unsigned long heat_cycle_on_duration = 5000; //msecs
static unsigned long heat_cycle_off_duration = 5000; //msecs
static unsigned long idle_temp_reading_interval = 5000; //msecs
static unsigned long heat_adjust_range = 2; //degrees

int target_temp = 0;
int current_temp = 0;

int state = APP_STATE_IDLE;

void setup() {
  delay(2000);
  //Serial.begin(9600);
  
  if (!ds.search(addr) ) {
      ds.reset_search();
      //Serial.println("reset search");
  }
  
  if (OneWire::crc8( addr, 7) != addr[7] ) {
    ds.reset_search();
    //Serial.println("reset search");
  }

  //
  pinMode(HOT_LED_PIN, OUTPUT);
  pinMode(COLD_LED_PIN, OUTPUT);
  pinMode(RIGHT_LED_PIN, OUTPUT);
  digitalWrite(HOT_LED_PIN, LOW);
  digitalWrite(COLD_LED_PIN, LOW);
  digitalWrite(RIGHT_LED_PIN, LOW);
  
  //pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  pinMode(SSR_PIN, OUTPUT);
  //pinMode(MODE_SWITCH_PIN, INPUT);
  digitalWrite(SSR_PIN, LOW);
  
  //noInterrupts();
  //attachInterrupt(0, button_handler, FALLING);
  //delay(100);
  //interrupts();
}

unsigned long now = 0;
unsigned long elapsed = 0;
unsigned long last_time = 0;

void loop() {
  //Serial.print("mode: ");Serial.print(mode);Serial.print("\n");
  //Serial.print("state: ");Serial.print(state);Serial.print("\n");
  target_temp = readUserSetting();
  //Serial.print("User temp: ");Serial.println(target_temp);
  //Serial.print("STATE: ");if(state==1)Serial.println("IDLE");else if(state==2)Serial.println("HEATING");else if(state==3)Serial.println("ADJUST_HEAT");else if(state==4)Serial.println("COOLING");else Serial.println("ADJUST_WAIT");
  
  now = millis();
  elapsed = now-last_time;
  
  //Serial.println(elapsed);
  //Serial.println(now);
  //Serial.println(last_time);
  switch(state) {  
    case APP_STATE_IDLE:
      digitalWrite(SSR_PIN, LOW);
      checkTemp();
      break;
    case APP_STATE_HEATING:
      digitalWrite(SSR_PIN, HIGH);
      checkTemp();
      break;
    case APP_STATE_COOLING:
      digitalWrite(SSR_PIN, LOW);
      checkTemp();
      break;
    case APP_STATE_ADJUST_HEAT:
      //Should we stop heating?
      if (elapsed >= heat_cycle_on_duration ) {
        digitalWrite(SSR_PIN, LOW);
        //Serial.println("OFF");
        last_time = millis();
        state = APP_STATE_ADJUST_WAIT;
        break;
      }
      checkTemp();
      break;
    case APP_STATE_ADJUST_WAIT:
      //Should we stop waiting?
      if (elapsed >= heat_cycle_off_duration ) {
        checkTemp();
        //digitalWrite(SSR_PIN, HIGH);
        //Serial.println("ON");
        //state = APP_STATE_ADJUST_HEAT;
        //checkTemp();
        last_time = millis();
      }
      break;
    default:
    break;
  }
  delay(200);
}

int readUserSetting()
{
  int val = analogRead(POT_PIN);
  //Serial.println(val);
  return map(val,0,1023,50,101);
}

void showTemp() {
  int relative = 0;
  
  if (current_temp == TEMP_ERROR) {
    digitalWrite(HOT_LED_PIN, HIGH);
    digitalWrite(COLD_LED_PIN, HIGH);
    digitalWrite(RIGHT_LED_PIN, HIGH);
  }
  
  else if (current_temp == target_temp)
  {
    digitalWrite(HOT_LED_PIN, LOW);
    digitalWrite(COLD_LED_PIN, LOW);
    digitalWrite(RIGHT_LED_PIN, HIGH);
  }
  
  else if (current_temp < target_temp) { //cold
    if (current_temp > (target_temp - TEMP_IND_RANGE) ) { //In range
      relative = target_temp-current_temp;
      analogWrite(COLD_LED_PIN, map(relative,0,TEMP_IND_RANGE,800,1023));
      analogWrite(RIGHT_LED_PIN, map(relative,0,TEMP_IND_RANGE,1023,800) );
      digitalWrite(HOT_LED_PIN, LOW);
    }
    else {
      digitalWrite(HOT_LED_PIN, LOW);
      digitalWrite(COLD_LED_PIN, HIGH);
      digitalWrite(RIGHT_LED_PIN, LOW);
    }
  }
  
  else if (current_temp > target_temp) { //hot
    if (current_temp < (target_temp + TEMP_IND_RANGE) ) { //In range
      relative = current_temp - target_temp;
      analogWrite(HOT_LED_PIN, map(relative,0,TEMP_IND_RANGE,800,1023));
      analogWrite(RIGHT_LED_PIN, map(relative,0,TEMP_IND_RANGE,1023,800));
      digitalWrite(COLD_LED_PIN, LOW);
    }
    else {
      digitalWrite(HOT_LED_PIN, HIGH);
      digitalWrite(COLD_LED_PIN, LOW);
      digitalWrite(RIGHT_LED_PIN, LOW);
    }
  }
}

void checkTemp() {
   int temp = getTemp();
  //Serial.print("TEMP READ: ");Serial.println(temp);
  
  if (temp == TEMP_ERROR || temp == target_temp)
    state = APP_STATE_IDLE;
  
  else if (temp < (target_temp-heat_adjust_range)) {
    state = APP_STATE_HEATING;
  }
  
  else if (temp >= (target_temp-heat_adjust_range) && temp < target_temp)
  {
    state = APP_STATE_ADJUST_HEAT;
  }
  
  else if (temp > target_temp) {
    state = APP_STATE_COOLING;
  }
  
  current_temp = temp;
  showTemp();
}

int getTemp() {
  int HighByte, LowByte, SignBit, Fract, TReading, Tc_100;
  int Whole = TEMP_ERROR;
  byte present = 0;
  int i = 0;
  byte data[12];

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);

  if (!ds.reset())
    return TEMP_ERROR;
  
  ds.select(addr);    
  ds.write(0xBE);

  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  
  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  
  if (SignBit) {
    TReading = -TReading;
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25
  Whole = Tc_100 / 100;          // separate off the whole and fractional portions
  Fract = Tc_100 % 100;
  //Serial.print(Whole);Serial.print(".");Serial.print(Fract);
  if (Fract > 49) {
    if (SignBit) {
      --Whole;
    } else {
      ++Whole;
    }
  }
  
  return Whole;
}

volatile unsigned long debounce = 0;

void button_handler()
{
  unsigned long now = millis();
  
  if ( (now - debounce) < 1000)
    return;
  
  debounce = now;
    
}
