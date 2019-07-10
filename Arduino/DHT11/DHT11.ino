/*
 * NodeMCU Secure MQTT.
 * This code is a simple adaptation of the original.
 * https://gist.github.com/eLement87/133cddc5bd0472daf5cb35a20bfd811e
 * 
 */
#include <Arduino.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <time.h>
#include <Wire.h>
#include <DHT.h>
#include "credentials.h"

#define DEBUG 0
#define DEBUG_RED 0

// Insert your FQDN of your MQTT Broker
#define MQTT_SERVER "test.mosquitto.org"
const char* mqtt_server = MQTT_SERVER;

// WiFi Credentials
const char* ssid = AP_1;
const char* password = AP_1_PASS;

// Fingerprint of the broker CA
// openssl x509 -in  mqttserver.crt -sha1 -noout -fingerprint
const char* fingerprint = MY_FINGERPRINT;
                           
// Topic
char* outTopic = "arielOut";
char* inTopic = "arielIn";
String clientName;

char jsonStr[200];

long lastMsg = 0;
long lastReconnectAttempt = 0;

int test_para = 5000;
unsigned long startMills;

WiFiClientSecure wifiClient;
PubSubClient client(mqtt_server, 8883, wifiClient);

int ledPinRed = 12;     // LED connected to D6 but It is GPIO12
int ledPinGreen = 13;   // D7 - GPIO13

#define DHTTYPE DHT11       // DHT 11

uint8_t DHTPin = D8;        // DHT Sensor
DHT dht(DHTPin, DHTTYPE);   // Initialize DHT sensor.

float temperature;
float humidity;

String redLedState = "false";
String greenLedState = "false";

boolean booleanRedLedState = false;
boolean booleanGreenLedState = false;

void verifytls() {
  // Use WiFiClientSecure class to create TLS connection
  Serial.print("connecting to ");
  Serial.println(mqtt_server);
  if (!wifiClient.connect(mqtt_server, 8883)) {
    Serial.println("connection failed");
    return;
  }

  if (wifiClient.verify(fingerprint, mqtt_server)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }
}
// Load Certificates
void loadcerts() {
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  // Load client certificate file from SPIFFS
  File ca = SPIFFS.open("/mosquitto.org.crt", "r"); //replace ca.der with your uploaded file name
  if (!ca) {
    Serial.println("Failed to open ca ");
  } else {
    Serial.println("Success to open ca");
  }
  delay(1000);
  // Set server CA file
  if(wifiClient.loadCACert(ca)) {
    Serial.println("ca loaded");
  } else {
    Serial.println("ca failed");
  }
}

void getTime() {
  // Synchronize time useing SNTP. This is necessary to verify that
  // the TLS certificates offered by the server are currently valid.
  Serial.print("Setting time using SNTP");
  configTime(8 * 3600, 0, "de.pool.ntp.org");
  time_t now = time(nullptr);
  while (now < 1000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

boolean reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect((char*) clientName.c_str())) {
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
  return client.connected();
}

void wifi_connect() {
  if (WiFi.status() != WL_CONNECTED) {    // WIFI
    Serial.println();
    Serial.print("===> WIFI ---> Connecting to ");
    Serial.println(ssid);
    delay(10);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    int Attempt = 0;
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(". ");
      Serial.print(Attempt);
      delay(5000);
      Attempt++;
      if (Attempt == 150) {
        Serial.println();
        Serial.println("-----> Could not connect to WIFI");
        ESP.restart();
        delay(200);
      }
    }
    
    Serial.println();
    Serial.print("===> WiFi connected");
    Serial.print(" ------> IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);                                         // will pause until serial console opens
  }

  startMills = millis();
  wifi_connect();
  delay(500);
  getTime();
  delay(500);
  loadcerts();
  delay(200);
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);

  pinMode(DHTPin, INPUT);
  dht.begin();
  pinMode(ledPinRed, OUTPUT);   // sets the pin as output
  pinMode(ledPinGreen, OUTPUT);
  client.setCallback(callback);
}

void loop() {
  getData();
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
    reconnect();
    }
    
    client.loop();
    long now = millis();
    if (now - lastMsg > 2500) {
      lastMsg = now;
      if (client.connected()) {
        String json = buildJson(temperature,humidity);
        json.toCharArray(jsonStr,200);
        boolean pubresult = client.publish(outTopic,jsonStr);
        Serial.print("attempt to send ");
        Serial.println(jsonStr);
        Serial.print("to ");
        Serial.println(outTopic);
        if (pubresult) {
          Serial.println("successfully sent");
        } else {
          Serial.println("unsuccessfully sent");
        }
      }
    }
  } else {
    wifi_connect();
  }
}

void sendmqttMsg(char* topictosend, String payload) {
  if (client.connected()) {
    Serial.print("Sending payload: ");
    Serial.print(payload);
    unsigned int msg_length = payload.length();
    Serial.print(" length: ");
    Serial.println(msg_length);
    byte* p = (byte*)malloc(msg_length);
    memcpy(p, (char*) payload.c_str(), msg_length);
    if ( client.publish(topictosend, p, msg_length)) {
      Serial.println("Publish ok");
      free(p);
      //return 1;
    } else {
      Serial.println("Publish failed");
      free(p);
      //return 0;
    }
  }
}

String macToStr(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  
  return result;
}

void callback(char* topic, byte* payload, unsigned int length) {
  const char* toggleRed    = "{\"command\":\"TOGGLE-RELAY\",\"relayIdx\":1}";
  const char* toggleGreen  = "{\"command\":\"TOGGLE-RELAY\",\"relayIdx\":2}";
  char charPayload[39];
  int ledNumber;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    charPayload[i] = (char)payload[i];
    Serial.print((char)payload[i]);
  }
  
  const char*  stringPayload = charPayload;
  Serial.println(stringPayload);
  // Switch on the LED
  if (!strcmp (stringPayload, toggleRed)) {
    ledNumber = 1;
    ledsState (ledNumber);
    Serial.println("Led Rojo");
  }
  
  if (!strcmp (stringPayload, toggleGreen)) {
    ledNumber = 2;
    ledsState (ledNumber);
    Serial.println("Led Verde");
  }
}

void ledsState (int ledToggle) {
  if (ledToggle == 1) {
    if (booleanRedLedState == true) {
      redLedState = "false";
      booleanRedLedState = false;
      digitalWrite(ledPinRed, LOW);
    } else {
      redLedState = "true";
      booleanRedLedState = true;
      digitalWrite(ledPinRed, HIGH);
    }
  }
  
  if (ledToggle == 2) {
    if (booleanGreenLedState == true) {
      greenLedState = "false";
      booleanGreenLedState = false;
      digitalWrite(ledPinGreen, LOW);
    } else {
      greenLedState = "true";
      booleanGreenLedState = true;
      digitalWrite(ledPinGreen, HIGH);
    }
  }
}

String buildJson(float temperatura, float humedad) {
  String data = "{";
  data+="\"bulb1State\":";
  data+=redLedState;
  data+=",";
  data+="\"bulb2State\":";
  data+=greenLedState;
  data+=",";
  data+="\"temp\":";
  data+=temperatura;
  data+=",";
  data+="\"hum\":";
  data+=humedad;
  data+=",";
  data+="}";
  return data;
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
}
