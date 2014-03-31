/*
    Author: Jari Tervonen <jjtervonen ~Ã„T~ gmail.com>, 2013-
    
    Based on TLC and Mirf library examples by:
    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-03
   and
    Unknown author. 
*/
#include "Tlc5940.h"
//#include <SPI.h>
//#include <Mirf.h>
//#include <nRF24L01.h>
//#include <MirfHardwareSpiDriver.h>

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(10);
  /* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
  Tlc.init(1000);
  
  /*Mirf.cePin = 4;
  Mirf.csnPin = 2;
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  Mirf.setRADDR((byte *)"hmd01");
  Mirf.payload = sizeof(unsigned long);*/
  
  delay(5000);
}

static byte input [4];
static uint32_t control = 0x00000000;
//static uint8_t buf[VW_MAX_MESSAGE_LEN];
//static uint8_t buflen = VW_MAX_MESSAGE_LEN;
boolean first_run = true;
boolean hmd_connected = false;

unsigned long ACK = 0xaaaaaaaa;
unsigned long time, last_update_time = 0;

void loop()
{
 /* Mirf.setTADDR((byte *)"serv1");
  
  time = millis();
  if (!hmd_connected || (time - last_update_time > 2000)) {
    Mirf.send((byte *)&ACK);
    last_update_time = millis();
    
    while(Mirf.isSending()){}
    Serial.println("Finished sending");
    delay(10);
    while(!Mirf.dataReady()) {
      Serial.println("Waiting");
      if ( ( millis() - time ) > 1000 ) {
        Serial.println("Timeout on response from server!");
        hmd_connected = false;
        return;
      }
    }
    hmd_connected = true;
  }
  Mirf.getData((byte *) &input);
  
  control = (uint32_t) ( (uint32_t)(input[0]) | ((uint32_t(input[1])<<8)) | ((uint32_t(input[2])<<16)) | ((uint32_t(input[3])<<24))  );
  input[0] = 0;
  input[1] = 0;
  input[2] = 0;
  input[3] = 0;
  //Serial.print("Got: ");Serial.println(control, BIN);
  updateColors(control);*/
    
  if (Serial.available()) {
    Serial.readBytes((char*)&input, 4);
    first_run = false;
    control = (uint32_t) ( (uint32_t)(input[0]) 
              | ((uint32_t(input[1])<<8)) 
              | ((uint32_t(input[2])<<16)) 
              | ((uint32_t(input[3])<<24)) );
    input[0] = 0;
    input[1] = 0;
    input[2] = 0;
    input[3] = 0;
    Serial.print("Got: ");Serial.println(control, BIN);
    updateColors(control);
  }
  else if (first_run) {
    demo();
  }
  /*if (vw_get_message(buf, &buflen)){
    int i;

    for (i = 0; i < buflen; i++)
    {
      input[i%4] = buf[i];
      //Serial.print(buf[i], HEX);
      //Serial.print(" ");
    }
  }*/
  //long dist = measureDistance();
  //Serial.println(dist);
  delay(10);
}
static int LEDS_IN_GROUP = 16;
static int LED_GROUPS = 2;
static int LED_COUNT = LED_GROUPS*LEDS_IN_GROUP;
static int BRIGHTNESS_BITS [] = {14,15,30,31};
static int BRIGHTNESS_BIT_COUNT = 4;


boolean not_in(int x, int *y, int len) {
  for(int i=0; i<len; i++) {
    if (y[i] == x)
      return false;
  }
  return true;
}

void updateColors(uint32_t c) { 
  int i,j = 0;
  uint8_t brightness = 0;
  
  Tlc.clear();
  //Serial.println(map(d,400,4,0,4095));
 
  brightness = (uint8_t)((c&0xf0000000)>>28);
  
  for (i=0,j=0;i<LED_COUNT;i++) {
    if (i == 14 || i == 15 || i == 30 || i == 31) {
       continue;
    }
    if ( (uint32_t) ((c >> j) & 0x01) == 1 ){
      //Tlc.set(i, map( ((c&0xff000000)>>24), 0, 255, 0, 4095));
      Tlc.set(i, map( brightness, 0, 16, 0, 4095));
    }
    j++;
  }
  Tlc.update();
}

uint32_t demo_val = 0x85555555;

void demo() {
  int i = 0;
  Tlc.clear();
  //Serial.println(map(d,400,4,0,4095));
  for (i=0;i<14;i++) {
    
    if ( (uint32_t) ((demo_val >> i) & 0x01) == 1 )
      Tlc.set(i, 2000);
  }
  for (i=14;i<16;i++) {
    Tlc.set(i, 0);
  }
  
  for (i=16;i<30;i++) {
    
    if ( (uint32_t) ((demo_val >> (i-4)) & 0x01) == 1 )
      Tlc.set(i, 2000);
  }
  for (i=30;i<32;i++) {
    Tlc.set(i, 0);
  }
  
  Tlc.update();
  demo_val ^= 0x0aaaaaaa;
  demo_val ^= 0x05555555;
  delay(500);
}

