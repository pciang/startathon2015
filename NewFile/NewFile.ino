//#define DS18B20_PIN 4
//#define LED         13


//#define ButtonGreen         2
//#define ButtonYellow        3
//#define ButtonBlack         4
//#define ServoMotor          6

//#include <OneWire.h>
//#include <DallasTemperature.h>
//#include <Servo.h>
#include<Wire.h>


#define SSID "ST2015RTP3" // SSID for Prototyping Lab @ NDC
#define PASS "H@ck8266"        // WPA2 Password for Prototyping Lab @ NDC
#define IP   "184.106.153.149"  // IP address pointed to by thingspeak.com

//String GET = "GET /update?key=56KD6EOB87O445X7&field1=";
String GET = "GET /update?key=56KD6EOB87O445X7";

const int MPU=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

void setup (void)
{
  // Initialise pin connected to on-board LED as an output, and initialise it to be logic LOW (off)
  pinMode      (LED, OUTPUT) ;
  digitalWrite (LED, LOW)    ;
  
  // Initialise serial communications at 115200 baud
  Serial.begin (115200) ;
  
  Serial.println ("AT") ;
  
  delay (500) ;
  
  if (Serial.find("OK"))
  {
    blinkLED (2, 100) ; // Communication OK - blink LED twice in rapid succession
    
    if (connectWiFi() != true) panic () ; // Attempt to connect to the Wi-Fi network
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
  Serial.begin(9600);
}

void loop (void)
{
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
  delay(333);
  
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
    Serial.write("connected");
    return true ;
  }
  else{ 
  Serial.write("not connected");
  return false ;}
}

boolean updateData (int16_t AcX,int16_t AcY,int16_t AcZ, int16_t Tmp, int16_t GyX,int16_t GyY, int16_t GyZ )
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
  cmd += "&field1="         ;
  cmd += AcX         ;
  cmd += "&field2="         ;
  cmd += AcY         ;
  cmd += "&field3="         ;
  cmd += AcZ         ;
  cmd += "&field4="         ;
  cmd += Tmp         ;
  cmd += "&field5="         ;
  cmd += GyX       ;
  cmd += "&field6="         ;
  cmd += GyY         ;
   cmd += "&field7="         ;
  cmd += GyZ         ;
  cmd += " HTTP/1.1\r\n\r\n" ;
  
  Serial.print   ("AT+CIPSEND=") ;
  Serial.println (cmd.length())  ; // AT+CIPSEND=<LENGTH_OF_HTTP_GET_REQUEST>\r\n
  
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
