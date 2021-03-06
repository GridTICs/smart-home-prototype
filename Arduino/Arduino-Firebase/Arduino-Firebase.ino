//
// Copyright 2015 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// FirebaseDemo_ESP8266 is a sample that demo the different functions
// of the FirebaseArduino API.

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <DHT.h>
#include "credentials.h"

// Set these to run example.
#define FIREBASE_HOST MY_FIREBASE_HOST
#define FIREBASE_AUTH MY_FIREBASE_PASS
#define WIFI_SSID AP_1
#define WIFI_PASSWORD AP_1_PASS      

#define DHTTYPE DHT11       // DHT 11

uint8_t DHTPin = D8;        // DHT Sensor
DHT dht(DHTPin, DHTTYPE);   // Initialize DHT sensor.

float temperature;
float humidity;
void setup() {
  Serial.begin(9600);

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  pinMode(2, OUTPUT);
}

int n = 0;

void loop() {
  //get value
  
  if(Firebase.getBool("led")){
    digitalWrite(2, LOW);
  }
  else{
    digitalWrite(2, HIGH);
  }
  // set value
  getData();
  Firebase.setFloat("Temperature", temperature);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting / Temperature failed:");
      Serial.println(Firebase.error());  
      return;
  }
  
  delay(1000);
  Firebase.setFloat("Humidity", humidity);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting / Humidity failed:");
      Serial.println(Firebase.error());  
      return;
  }
  
  delay(1000);
  // update value
  Firebase.setFloat("number", 43.0);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(1000);

  // get value 
  Serial.print("number: ");
  Serial.println(Firebase.getFloat("number"));
  delay(1000);

  // remove value
  Firebase.remove("number");
  delay(1000);

  // set string value
  Firebase.setString("message", "hello world");
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(1000);
  
  // set bool value
  Firebase.setBool("truth", false);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /truth failed:");
      Serial.println(Firebase.error());  
      return;
  }
  delay(1000);

  // append a new value to /logs
  String name = Firebase.pushInt("logs", n++);
  // handle error
  if (Firebase.failed()) {
      Serial.print("pushing /logs failed:");
      Serial.println(Firebase.error());  
      return;
  }
  Serial.print("pushed: /logs/");
  Serial.println(name);
  delay(1000);
}

void getData() {
  temperature = dht.readTemperature();                   // Gets the values of the temperature
  humidity = dht.readHumidity();                         // Gets the values of the humidity
  if (!isnan(temperature)) {                             // check if 'is not a number'
  } else { 
    Serial.println("Failed to read temperature");
    temperature = -1;
  }
  
  if (!isnan(humidity)) {                                // check if 'is not a number'
  } else { 
    Serial.println("Failed to read humidity");
    humidity = -1;
  }
  
//  potentiometerValue = analogRead(analogPin);            // read the input pin
//  analogWrite(ledPinWhite, potentiometerValue / 4);      // analogRead values go from 0 to 1023, analogWrite values from 0 to 255

}
