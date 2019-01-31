#include <Arduino.h>
#include <Wire.h>
/*#include <SPI.h>
#include <Ethernet.h>*/

#include "Adafruit_SHT31.h"

Adafruit_SHT31 sht31 = Adafruit_SHT31();

int ledPinRed = 3;      // LED connected to digital pin 9
int ledPinWhite = 5;
int ledPinYellow = 6;
int ledPinGreen = 9;

int analogPin = 3;   // potentiometer connected to analog pin 3
int val = 0;         // variable to store the read value

int BH1750_address = 0x23; // i2c Addresse
byte buff[2];

/*byte mac[] = { 0xF0, 0x5A, 0x09, 0x58, 0x56, 0x8E };
byte server[] = { 192, 168, 1, 44};
IPAddress ip(192,168,1,45);
EthernetClient client;*/

void setup() {
  
  Serial.begin(9600);

  while (!Serial){
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens
  }
  
  Wire.begin();
  BH1750_Init(BH1750_address);
  Serial.println("SHT31 test");
  
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
    
  pinMode(ledPinRed, OUTPUT);   // sets the pin as output
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinYellow, OUTPUT);
  pinMode(ledPinWhite, OUTPUT);
  /*
  Serial.println("Empezando Ethernet");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
  if (client.connect(server, 3000)) {
    Serial.println("connected");
    client.println("Hello!! Desde Arduino");
  }
  else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed");
  }*/
}

void loop() {
  
  float temperature = sht31.readTemperature();
  float humidity = sht31.readHumidity();

  if (! isnan(temperature)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.println(temperature);
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(humidity)) {                                // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(humidity);
  } else { 
    Serial.println("Failed to read humidity");
  }
  Serial.println();
  
  val = analogRead(analogPin);                      // read the input pin
  analogWrite(ledPinRed, val / 4);                  // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
  analogWrite(ledPinWhite, val / 4);
  analogWrite(ledPinYellow, val / 4);
  analogWrite(ledPinGreen, val / 4);

  float valf = 0;
  
  if(BH1750_Read(BH1750_address)==2){
    valf = ((buff[0] << 8) | buff[1] ) / 1.2;
    
    if (valf < 0){
      Serial.print("> 65535");
    } else {
      Serial.print((int)valf,DEC); 
    }
    
    Serial.println(" lx"); 
  } 
  delay(1000);
}

void BH1750_Init(int address){
  Wire.beginTransmission(address);
  Wire.write(0x10); // 1 [lux] aufloesung
  Wire.endTransmission();
}

byte BH1750_Read(int address){
  byte i = 0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  
  while(Wire.available()){
    buff[i] = Wire.read(); 
    i++;
  }
  
  Wire.endTransmission();  
  return i;
}
