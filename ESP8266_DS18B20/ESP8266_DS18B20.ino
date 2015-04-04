#define DS18B20_PIN 4
#define LED         13


#define ButtonGreen         2
#define ButtonYellow        3
#define ButtonBlack         4
#define ServoMotor          6

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h>


#define SSID "ST2015RTP3" // SSID for Prototyping Lab @ NDC
#define PASS "H@ck8266"        // WPA2 Password for Prototyping Lab @ NDC
#define IP   "184.106.153.149"  // IP address pointed to by thingspeak.com

//String GET = "GET /update?key=56KD6EOB87O445X7&field1=";
String GET = "GET /update?key=56KD6EOB87O445X7";
int angle = 0;
OneWire           oneWire(DS18B20_PIN) ;
DallasTemperature sensors(&oneWire)    ;
Servo myServo;


void setup (void)
{
  // Initialise pin connected to on-board LED as an output, and initialise it to be logic LOW (off)
  pinMode      (LED, OUTPUT) ;
  digitalWrite (LED, LOW)    ;
  
  // Initialise serial communications at 115200 baud
  Serial.begin (115200) ;
  
  
  myServo.attach(ServoMotor);
  
  // Test if ESP8266 is properly connected, configured for the correct baud rate and responding to commands
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
}

void loop (void)
{
  char tempASCII[6] ;
  
  char buttonGreenASCII[6] ;
  char buttonYellowASCII[6] ;
  char buttonBlackASCII[6] ;
  
  
  sensors.requestTemperatures () ; // Issue command to read the DS18B20 sensor
  
  int temp = sensors.getTempCByIndex(0);
  
  // Convert that over to a string representation
  dtostrf (temp, 2, 1, tempASCII) ;
  
  
  // Wait for 60 seconds before taking and pushing another reading
  //for (temp=0; temp<15; temp++) delay (1000) ;
  
  int buttonGreen = digitalRead(ButtonGreen);
  int buttonYellow = digitalRead(ButtonYellow);
  int buttonBlack = digitalRead(ButtonBlack);
  
  
   // Convert that over to a string representation
  dtostrf (buttonGreen, 2, 1, buttonGreenASCII) ;
 dtostrf (buttonYellow, 2, 1, buttonYellowASCII) ;
 dtostrf (buttonBlack, 2, 1, buttonBlackASCII) ;
 
 Serial.println ("Green is");
 Serial.print (buttonGreenASCII);
 
  // Push the current reading to Thingspeak
  //updateData (buttonGreenASCII,buttonYellowASCII,buttonBlackASCII) ;
  
  if(buttonGreen == 0)
  {
      angle =  angle + 10;
      myServo.write(angle);
      delay(1000);
  }
  else
  {
    //myServo.write(0);
  }
  if(buttonBlack == 0)
  {
      angle = 0;
      myServo.write(angle);
      delay(1000);
  }
  else
  {
    //myServo.write(0);
  }

  
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

boolean updateData (String top,String pants,String shoes)
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
  cmd += top         ;
  cmd += "&field2="         ;
  cmd += pants         ;
  cmd += "&field3="         ;
  cmd += shoes         ;
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
