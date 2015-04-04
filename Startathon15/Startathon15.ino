#include <Wire.h>
#include <dht.h>

#define SSID "La Liberta de Espana" // SSID for Prototyping Lab @ NDC
#define PASS "robertganteng"   // WPA2 Password for Prototyping Lab @ NDC
#define IP   "192.168.173.1"  // IP address pointed to by thingspeak.com
#define LED         13
//#define HB_SENSOR_PIN 0
#define HB_SENSOR_PIN A0

//String GET = "GET /update?key=56KD6EOB87O445X7&field1=";
String GET = "GET /startathon2015/record.php?";

const int MPU=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ; double hm_value;

int temp = 0;
dht DHT;
void setup (void)
{
  temp = 0;
  
  // Initialise pin connected to on-board LED as an output, and initialise it to be logic LOW (off)
  pinMode      (LED, OUTPUT) ;
  digitalWrite (LED, LOW)    ;
  
  // Initialise serial communications at 115200 baud
  Serial.begin (115200) ;
  
  Serial.println ("AT") ;
  
  delay (1000) ;
  
  if (Serial.find("OK"))
  {
    blinkLED (2, 100) ; // Communication OK - blink LED twice in rapid succession
    if (connectWiFi() != true){
      panic () ; // Attempt to connect to the Wi-Fi network
    }
  }
  else
  {
    panic () ; // Give up and report the error
  }
  
  //setup the sensor
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  //Serial.begin(9600);
}

//double hbAlpha = 0.75;
//int hbPeriod = 100;
//double hbChange = 0.0;
//double hbMinval = 0.0;
void loop (void)
{
  //static double hbOldValue = 0;
  //static double hbOldChange = 0;
   DHT.read11(HB_SENSOR_PIN);

  hm_value = DHT.humidity;
  //double hbValue = hbAlpha * hbOldValue + (1 - hbAlpha) * hbRawValue;
  
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,14,true);  // request a total of 14 registers
  
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  
  Serial.print("AcX = "); Serial.print(AcX);
  Serial.print(" | AcY = "); Serial.print(AcY);
  Serial.print(" | AcZ = "); Serial.print(AcZ);
  Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53);  //equation for temperature in degrees C from datasheet
  Serial.print(" | GyX = "); Serial.print(GyX);
  Serial.print(" | GyY = "); Serial.print(GyY);
  Serial.print(" | GyZ = "); Serial.println(GyZ);
Serial.print(" | hmValue = "); Serial.println(hm_value);
  
  
  
  // delay(2000);
  ///hbOldValue = hbValue;

  //if (temp == 3) 
    updateData(AcX, AcY, AcZ, Tmp/340.00 + 36.53, GyX, GyY, GyZ, hm_value);
  //else if (temp == 20)
  //  temp = 0;
  //temp++;
}

void blinkLED (unsigned char times, unsigned char duration)
{
  for (unsigned char count=0; count<times; count++)
  {
    digitalWrite (LED, HIGH) ;
    delay        (duration)  ;
    digitalWrite (LED, LOW)  ;
    delay        (duration)  ;
  }
}

void panic (void)
{
  while (true)
  {
    digitalWrite (LED, HIGH) ;
    delay        (100)       ;
    digitalWrite (LED, LOW)  ;
    delay        (100)       ;
  }
}

boolean connectWiFi ()
{
  String cmd = "AT+CWJAP=\"" ;
  
  cmd += SSID    ;
  cmd += "\",\"" ;
  cmd += PASS    ;
  cmd += "\""    ;
  
  Serial.println (cmd) ; // AT+CWJAP="Singtel8800-5AC8","OMG188969"\r\n

  delay (5000) ; // Wait for 5 seconds before checking if connection to Wi-Fi network has been successful

  if (Serial.find("OK"))
  {
    blinkLED (2, 200) ; // Blink LED twice if connection to Wi-Fi network is successful
   // Serial.write("connected");
    return true ;
  }
  else{ 
 // Serial.write("not connected");
  return false ;}
}

boolean updateData (int16_t AcX,int16_t AcY,int16_t AcZ, int16_t Tmp, int16_t GyX,int16_t GyY, int16_t GyZ, double hbValue )
{
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  
  cmd += IP;
  cmd += "\",80";
  
  Serial.println (cmd) ; // AT+CIPSTART="TCP","184.106.153.149",80\r\n
  
  delay (2000) ; // Wait for 2 seconds before checking for TCP connection errors or making a push attempt
  
  if (Serial.find("Error"))
  {
    blinkLED (4, 500) ; // 4 slow flashes to indicate an error
    
    Serial.println ("AT+CIPCLOSE") ; // AT+CIPCLOSE\r\n
    
    return false ;
  }
  
  cmd  = GET                 ;
  cmd += "&acx="         ;
  cmd += AcX         ;
  cmd += "&acy="         ;
  cmd += AcY         ;
  cmd += "&acz="         ;
  cmd += AcZ         ;
  cmd += "&tmp="         ;
  cmd += Tmp         ;
  cmd += "&gyx="         ;
  cmd += GyX       ;
  cmd += "&gyy="         ;
  cmd += GyY         ;
  cmd += "&gyz="         ;
  cmd += GyZ         ;
  cmd += "&hum=";
  cmd += hbValue;
  cmd += " HTTP/1.0\r\n\r\n" ;
  
  Serial.print   ("AT+CIPSEND=") ;
  Serial.println (cmd.length())  ; // AT+CIPSEND=<LENGTH_OF_HTTP_GET_REQUEST>\r\n
  // Serial.print(cmd);
  delay (500) ;
  
  if (Serial.find(">"))
  {
    Serial.print (cmd) ; // GET /update?key=<CHANNEL_API_KEY>&field1=<TEMPERATURE> HTTP/1.1\r\n\r\n"
    
    delay (2000) ; // Allow some time for TCP re-tries, if any
    
    blinkLED (4, 100) ; // 4 quick flashes to indicate push complete
    
    Serial.println ("AT+CIPCLOSE") ; // AT+CIPCLOSE\r\n
    
    return true ;
  }
  else
  {
    blinkLED (4, 500) ; // 4 slow flashes to indicate an error
    
    Serial.println ("AT+CIPCLOSE") ; // AT+CIPCLOSE\r\n
    
    return false ;
  }
}
