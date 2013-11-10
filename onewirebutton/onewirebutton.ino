#include <OneWire.h>

OneWire  ds(9);  // on pin 10 (a 4.7K resistor is necessary)

void setup(void) {
  Serial.begin(9600);
}

void loop(void) {
  byte present = 0;
  byte data[8];
  byte addr[8];
  long result_h = 0;
  long result_l = 0;
  int i = 0;
  
  if ( !ds.search(addr)) {
    Serial.println("No more addresses...resetting search");
    Serial.println();
    ds.reset_search();
    delay(500);
    return;
  }
  
  Serial.print("ROM = [");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }
  Serial.print(" ]");
  
  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
  
  //ds.reset();
  present = ds.reset();
  //ds.select(addr);
  ds.write(0x33, 1);
  
  //delay(1000);
  
  data[0] = ds.read();
  Serial.print("Family code: "); Serial.println(data[0], HEX);
  
  Serial.print("Data[");
  for ( i = 1; i < 4; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.write(' ');
    Serial.print(data[i], HEX);
    result_l |= ( ((long)data[i] & 0x000000ff) << (i-1)*8);
  }
  for ( i = 4; i < 7; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.write(' ');
    Serial.print(data[i], HEX);
    result_h |= ( ((long)data[i] & 0x000000ff) << (i-4)*8);
  }
  Serial.println("]");
   
  data[7] = ds.read();
  Serial.print("CRC: "); Serial.println(data[7], HEX);
   
  Serial.println(result_l, HEX);
  Serial.println(result_h, HEX);
  
  Serial.print("Calculated CRC: ");
  Serial.print(OneWire::crc8(data, 7), HEX);
  Serial.println();
  
  Serial.println( (long)( ( (result_h&0x00000000ffffffff) << 32) | (result_l & 0x00000000ffffffff)) );
  Serial.println( (long)( ( (result_h&0x00000000ffffffff) << 32) | (result_l & 0x00000000ffffffff)) , HEX);
  
  delay(1000);
}
