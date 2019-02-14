#include "credentials.h"
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>
#include "Adafruit_SHT31.h"

#define thingSpeakAddress "184.106.153.149"  //  "192.168.1.33"
#define port 80

#define DEBUG 0
#define DEBUG_RED 1

Adafruit_SHT31 sht31 = Adafruit_SHT31();

int ledPinRed = 3;                                      // LED connected to digital pin 3
int ledPinWhite = 5;
int ledPinYellow = 6;
int ledPinGreen = 9;

int analogPin = 10;                                     // potentiometer connected to analog pin 10
int potentiometerValue = 0;                             // variable to store the read value

int BH1750_address = 0x23;                              // i2c Addresse light sensor
byte buff[2];

String writeAPIKey =  THINGSPEAK_WRITE_API_KEY;
String SSID = AP_1;
String PASS = AP_1_PASS;

long updateThingSpeakInterval = 900000; // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

long lastConnectionTime = 0;
int failedCounter = 0;
boolean lastConnected = false;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial1.setTimeout(1000);  

  while(!Serial);
  while(!Serial1);
  
  Wire.begin();
  BH1750_Init(BH1750_address);
  if (DEBUG){
    Serial.println("SHT31 test");  
  }
    
  if (! sht31.begin(0x44)) {   // i2c Addresse sht31 sensor. If pin ADR is in LOW i2c address = 0x44. And pin ADR is in HIGH alternate i2c address = 0x45.
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
    
  pinMode(ledPinRed, OUTPUT);   // sets the pin as output
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinYellow, OUTPUT);
  pinMode(ledPinWhite, OUTPUT);
  
  Serial.println("ESP8266 Demo on Mega2560");
  clearSerialBuff();
  Serial1.println("AT+RST");
  delay(5000);
  Serial.println("Resetting module");
  boolean connected = false;
  
  for(int i=0;i<5;i++){
    if(connectWiFi()){
      connected = true;
      break;
    }
  }

  if (!connected){
    while(1);
  }

  delay(5000);
}

void loop() {
  float temperature = sht31.readTemperature();
  float humidity = sht31.readHumidity();
  //Serial.println();
  
  if (! isnan(temperature)) {  // check if 'is not a number'
    if (DEBUG){
      mySerialPrintDataF ("Temperatura = ", temperature);
    }
    //Serial.print("Temp *C = "); 
    //Serial.println(temperature);
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(humidity)) {                                // check if 'is not a number'
    if (DEBUG){
      mySerialPrintDataF ("Humidity = ", humidity);
    }
    //Serial.print("Hum. % = ");
    //Serial.println(humidity);
  } else { 
    Serial.println("Failed to read humidity");
  }
  
  potentiometerValue = analogRead(analogPin);                      // read the input pin
  analogWrite(ledPinRed, potentiometerValue / 4);                  // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
  analogWrite(ledPinWhite, potentiometerValue / 4);
  analogWrite(ledPinYellow, potentiometerValue / 4);
  analogWrite(ledPinGreen, potentiometerValue / 4);

  float illuminance = 0;
  
  if(BH1750_Read(BH1750_address)==2){
    illuminance = ((buff[0] << 8) | buff[1] ) / 1.2;
    
    if (illuminance < 0){
      Serial.print("> 65535");
    } else {
      if (DEBUG){
      mySerialPrintDataF ("Illuminance: ", illuminance);
      //Serial.print("Illuminance : ");
      //Serial.print((int) illuminance,DEC); 
      }
    }
    //Serial.println(" lx");
  }
  
  char t_buffer[10];
  char h_buffer[10];
  char i_buffer[10];
  char p_buffer[10];
  String temperatureString = dtostrf(temperature,0,5,t_buffer);
  String humidityString = dtostrf(humidity,0,5,h_buffer);
  String iluminanceString = dtostrf(illuminance,0,5,i_buffer);
  String potentiometerString = dtostrf(potentiometerValue,0,5,p_buffer);
  
  if (!clientConnected() && lastConnected){
    Serial.println("...disconnected");
    Serial.println();
    clientStop();
  }
  if (DEBUG_RED) {
     mySerialPrintDataI ("En If que contien AND lastConnected = ", lastConnected);
     Serial.println (clientConnected());
  }
   // Update ThingSpeak
  if(!clientConnected()){
    connectToThingSpeak();
  }
  if (millis() < lastConnectionTime) {                 //Este if nos salva de la excepcion de que ocurra un overflow de millis() 
    lastConnectionTime = 0;
  }
  if(clientConnected() && (millis() - lastConnectionTime > updateThingSpeakInterval)){
    if (DEBUG_RED) {
     Serial.println ("ENTRO AL IF UPDATE");
  }
    sendToThingSpeak("field1="+temperatureString+"&field2="+humidityString+"&field3="+iluminanceString+"&field4="+potentiometerString+"&field5="+potentiometerString+"&field6="+potentiometerString+"&field7="+potentiometerString);
  }
    if (DEBUG_RED) {
    mySerialPrintDataL ("Valor lastConnectionTime luego Update ", lastConnectionTime);
    mySerialPrintDataL ("Valor millis luego Update ", millis());
    mySerialPrintDataL ("Valor UpdateThingSpeakInterval ", updateThingSpeakInterval);
    long diferencia = millis() - lastConnectionTime;
    mySerialPrintDataL ("Valor diferencia luego Update ", diferencia);
  } 
  // Check if Arduino Ethernet needs to be restarted
  if (failedCounter > 3 ) {
    connectWiFi();
  }
  
  lastConnected = clientConnected();
  if (DEBUG_RED) {
     mySerialPrintDataI ("lastConnected = ", lastConnected);
     Serial.println (clientConnected());
     delay (2000);
  }
}

void sendToThingSpeak(String tsData){
  String request;
  
  request += "POST /update HTTP/1.1\n";
  request += "Host: api.thingspeak.com\n";
  request += "Connection: close\n";
  request += "X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n";
  request += "Content-Type: application/x-www-form-urlencoded\n";
  request += "Content-Length: ";
  request += tsData.length();
  request += "\n\n";
  request += tsData;
  
  String cmd;
  cmd = "AT+CIPSEND=";
  cmd += request.length();

  clearSerialBuff();
  Serial1.println(cmd);
  Serial.println(cmd);
  delay (2000);
  
  if(Serial1.find(">")){
    Serial.print(">");
  }else{
    Serial.println("Connect timeout");
    delay(1000);
    return;
  }
  
  clearSerialBuff();
  Serial1.println(request);
  Serial.println(request);
  delay(2000);
  if (Serial1.find("SEND OK")){
    Serial.println("Enviado correctamente");
    lastConnectionTime = millis();
  }
  
}

void connectToThingSpeak(){
  if (clientConnect()){
    if (clientConnected()){
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      failedCounter = 0;
    } else {
      failedCounter++;
      Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");
      Serial.println();
    }
  }else{
     failedCounter++;
     Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");
     Serial.println();
//     lastConnectionTime = millis();
  }
}

boolean clientConnected(){
  clearSerialBuff();
  Serial1.println("AT+CIPSTATUS");
  Serial.println("AT+CIPSTATUS");
  delay(1000);
  
  if (Serial1.find("STATUS:3")){
    Serial.println("Connection TCP is already");
    return true;
  } else {
    Serial.println("Connection TCP is disconnected");
    return false;
  }
}

void clientStop(){
  clearSerialBuff();
  Serial1.println("AT+CIPCLOSE");
  delay(2000);
  
  if (Serial1.find("CLOSED")){
    Serial.println("Connection close.");
  } else {
    Serial.println("There was a problemas when It tries closing connection.");
  }
}

boolean clientConnect(){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += thingSpeakAddress;
  cmd += "\",";
  cmd += port;
  
  clearSerialBuff();
  Serial1.println(cmd);
  Serial.println(cmd);
  delay(1000);
  
  if(Serial1.find("OK")){
    Serial.println("Connection TCP");
    return true;
  } else {
    Serial.println("Connection TCP Failed");
    return false;
  }
}

boolean connectWiFi(){
  delay(5000);
  clearSerialBuff();
  
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";

  Serial1.println(cmd);
  Serial.println(cmd);
  delay(10000);

  if(Serial1.find("OK")){
    Serial.println("OK, Connected to WiFi.");
    return true;
  }else{
    Serial.println("Can not connect to the WiFi.");
    return false;
  }
}

void clearSerialBuff(){
  delay (3000);
  char c;
  while (Serial1.available()!=0){
    c = Serial1.read();
  }
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

void mySerialPrintDataF (String message, float value) {
    Serial.print(message);
    Serial.println(value);
}

void mySerialPrintDataL (String message, long value) {
    Serial.print(message);
    Serial.println(value);
}

void mySerialPrintDataI (String message, int value) {
    Serial.print(message);
    Serial.println(value);
}
