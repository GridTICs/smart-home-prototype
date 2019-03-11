#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Adafruit_SHT31.h"
#include "credentials.h"

#define DEBUG 0
#define DEBUG_RED 0
Adafruit_SHT31 sht31 = Adafruit_SHT31();

int ledPinRed = 14;     // LED connected to D5 but It is GPIO14
int ledPinWhite = 12;   // D6 - GPIO12 
int ledPinYellow = 13;  // D7 - GPIO13
int ledPinGreen = 15;   // D8 - GPIO15

int analogPin = analogRead(A0);   // potentiometer connected to analog pin 10
int potentiometerValue = 0;       // variable to store the read value

float illuminance = 0;
float temperature = 0;
float humidity = 0;
int BH1750_address = 0x23;                              // i2c Addresse light sensor
byte buff[2];

const char* mqtt_server = "192.168.1.37";

String SSID = AP_3;
String PASS = AP_3_PASS;

const char* clientName = "arduino";
const char* inTopic = "inTopic";
const char* outTopic = "outTopic";
char jsonStr[200];

long lastMsg = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  Serial.println("Start");
   
  while (!Serial){
    delay(10);                                         // will pause until serial console opens
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
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);  
}

void loop() {
  getData();
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  long now = millis();
  if (now - lastMsg > 5000){
    lastMsg = now;
    if (client.connected() ) {
      String json = buildJson(temperature,humidity,illuminance); //"Hola Mundo! Saludos desde Mendoza";
      json.toCharArray(jsonStr,200);
      boolean pubresult = client.publish(outTopic,jsonStr);
      Serial.print("attempt to send ");
      Serial.println(jsonStr);
      Serial.print("to ");
      Serial.println(outTopic);
      if (pubresult)
        Serial.println("successfully sent");
      else
        Serial.println("unsuccessfully sent");
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientName)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, jsonStr);
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
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

String buildJson(float temperatura, float humedad, float luminosidad) {
  String data = "{";
  data+="\n\"Temperatura\":\"";
  data+=temperatura;
  data+="\",";
  data+="\n\"Humedad\":\"";
  data+=potentiometerValue;
  data+="\",";
  data+="\n\"Luminosidad\":\"";
  data+=luminosidad;
  data+="\",\n}";
  return data;
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASS);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.println("Waiting WiFi");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
 
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
    digitalWrite(ledPinRed, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(ledPinRed, HIGH);  // Turn the LED off by making the voltage HIGH
  } 
}

void getData(){
  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();
  
  if (! isnan(temperature)) {  // check if 'is not a number'
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(humidity)) {    // check if 'is not a number'
  } else { 
    Serial.println("Failed to read humidity");
  }
  
  potentiometerValue = analogRead(analogPin);             // read the input pin
                                                          // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
  analogWrite(ledPinWhite, potentiometerValue / 4);
  analogWrite(ledPinYellow, potentiometerValue / 4);
  analogWrite(ledPinGreen, potentiometerValue / 4);
  
  if(BH1750_Read(BH1750_address)==2){
    illuminance = ((buff[0] << 8) | buff[1] ) / 1.2;
    
    if (illuminance < 0){
      Serial.print("> 65535");
    }
  }
}
