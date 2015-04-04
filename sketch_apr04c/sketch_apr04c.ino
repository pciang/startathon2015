#include <dht.h>

#define dht_dpin A1

dht DHT;
void setup() {
  // put your setup code here, to run once:
  
}

void loop() {
  // put your main code here, to run repeatedly:
//This is the "heart" of the program.
  DHT.read11(dht_dpin);
  int16_t dhtHumidity = DHT.humidity;
    Serial.print("Current humidity = ");
    Serial.print(DHT.humidity);
  
    
  delay(800);
}
