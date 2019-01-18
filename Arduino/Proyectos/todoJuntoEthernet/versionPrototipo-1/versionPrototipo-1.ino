#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include "Adafruit_SHT31.h"

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

String writeAPIKey = "1YF1AO4UQ2BGALRL";

byte mac[] = { 0xF0, 0x5A, 0x09, 0x58, 0x56, 0x8E };
byte thingSpeakAddress[] = {184, 106, 153, 149};                         //ip address of server.
int port = 80 ;// a traves de proxy utn IP 192.168.16.8 puerto 8080
const int updateThingSpeakInterval = 16 * 1000; // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

IPAddress ip(192,168,1,44);

EthernetClient client;
long lastConnectionTime = 0;
int failedCounter = 0;
boolean lastConnected = false;

void setup() {
  Serial.begin(9600);

  while (!Serial){
    delay(10);                                         // will pause Zero, Leonardo, etc until serial console opens
  }
  
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
  
  startEthernet();
  /*if (client.connect(thingSpeakAddress, port)) {
    Serial.println("connected");
    //client.println("Hello!! Desde Arduino");
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }*/
  
} //void setup()

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

  // Disconnect from ThingSpeak
  if (!client.connected() && lastConnected){
    Serial.println("...disconnected");
    Serial.println();
    client.stop();
  }
  if (DEBUG_RED) {
     mySerialPrintDataI ("En If que contien AND lastConnected = ", lastConnected);
     Serial.println (client.connected());
  }
  //MOSTRAR CLIENT.CONNECTED H LASTCONNECTED 
  // Update ThingSpeak
  if(!client.connected())
  {
    connectToThingSpeak();
  }
  
  if(client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval)){
    if (DEBUG_RED) {
     Serial.println ("ENTRO AL IF UPDATE");
  }
    sendToThingSpeak("field1="+temperatureString+"&field2="+humidityString+"&field3="+iluminanceString+"&field4="+potentiometerString+"&field5="+potentiometerString+"&field6="+potentiometerString+"&field7="+potentiometerString);
  }
   if (DEBUG_RED) {
    mySerialPrintDataL ("Valor lastConnectionTime luego Update ", lastConnectionTime);
    mySerialPrintDataL ("Valor millis luego Update ", millis());
    long diferencia = millis() - lastConnectionTime;
    mySerialPrintDataL ("Valor diferencia luego Update ", diferencia);
  } 
  // Check if Arduino Ethernet needs to be restarted
  if (failedCounter > 3 ) {
    startEthernet();
  }
  
  lastConnected = client.connected();
  if (DEBUG_RED) {
     mySerialPrintDataI ("lastConnected = ", lastConnected);
     Serial.println (client.connected());
     delay (2000);
  }
  
} //void loop ()

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
void sendToThingSpeak(String tsData){
    Serial.println(Ethernet.localIP());
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");
    client.print(tsData);
    lastConnectionTime = millis();
   
} // sendToThingSpeak

void connectToThingSpeak(){//String tsData){
  if (client.connect(thingSpeakAddress, port)){
//    Serial.println(Ethernet.localIP());
//    client.print("POST /update HTTP/1.1\n");
//    client.print("Host: api.thingspeak.com\n");
//    client.print("Connection: close\n");
//    client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
//    client.print("Content-Type: application/x-www-form-urlencoded\n");
//    client.print("Content-Length: ");
//    client.print(tsData.length());
//    client.print("\n\n");
//    client.print(tsData);
//    lastConnectionTime = millis();
   
    if (client.connected()){
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      failedCounter = 0;
    }else {
      failedCounter++;
      Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");
      Serial.println();
    }
    
  }else{
     failedCounter++;
     Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");
     Serial.println();
     lastConnectionTime = millis();
  }
} // connectToThingSpeak

void startEthernet(){
  client.stop();
  Serial.println("Connecting Arduino to network...");
  Serial.println();
  delay(1000);
  
  // Connect to network amd obtain an IP address using DHCP
  if (Ethernet.begin(mac) == 0){
    Serial.println("DHCP Failed, reset Arduino to try again");
    Serial.println();
  }else{
    Serial.println("Arduino connected to network using DHCP");
    Serial.println();
  }
  
  delay(1000);
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
