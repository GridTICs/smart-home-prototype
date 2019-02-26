/*
 Ejemplo MQTT IOT.
 */

#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>


// Update this to either the MAC address found on the sticker on your ethernet shield (newer shields)
// or a different random hexadecimal value (change at least the last four bytes)
byte mac[]    = { 0xF0, 0x5A, 0x09, 0x58, 0x56, 0x8E };
char macstr[] = "f05a0958568e";
// Note this next value is only used if you intend to test against a local MQTT server
byte localserver[] = {192, 168, 1, 45 };

char servername[]= "test.mosquitto.org";
String clientName = String("arduinomatr") + macstr;
String topicName = String("matrTestTopic");

EthernetClient ethClient;

// Uncomment this next line and comment out the line after it to test against a local MQTT server
//PubSubClient client(localserver, 1883, 0, ethClient);
PubSubClient client(servername, 1883, 0, ethClient);

void setup() {
  if (Ethernet.begin(mac) == 0){
  Serial.println("DHCP Failed, reset Arduino to try again");
  Serial.println();
  }else{
  Serial.println("Arduino connected to network using DHCP");
  Serial.println();
  }
  // Start the ethernet client, open up serial port for debugging, and attach the DHT11 sensor
  //Ethernet.begin(mac, ip);
  Serial.begin(9600);
}

void loop() {
  char clientStr[34];
  clientName.toCharArray(clientStr,34);
  char topicStr[26];
  topicName.toCharArray(topicStr,26);
//  getData();
  if (!client.connected()) {
    Serial.print("Trying to connect to: ");
    Serial.println(clientStr);
    client.connect(clientStr);
  }
  if (client.connected() ) {
    String json = "Hola Mundo! Saludos desde Mendoza";//buildJson();
    char jsonStr[200];
    json.toCharArray(jsonStr,200);
    boolean pubresult = client.publish(topicStr,jsonStr);
    Serial.print("attempt to send ");
    Serial.println(jsonStr);
    Serial.print("to ");
    Serial.println(topicStr);
    if (pubresult)
      Serial.println("successfully sent");
    else
      Serial.println("unsuccessfully sent");
  }
  delay(15000);
}
/*
String buildJson() {
  String data = "{";
  data+="\n";
  data+= "\"d\": {";
  data+="\n";
  data+="\"myName\": \"Arduino DHT11\",";
  data+="\n";
  data+="\"temperature (F)\": ";
  data+=(int)tempF;
  data+= ",";
  data+="\n";
  data+="\"temperature (C)\": ";
  data+=(int)tempC;
  data+= ",";
  data+="\n";
  data+="\"humidity\": ";
  data+=(int)humidity;
  data+="\n";
  data+="}";
  data+="\n";
  data+="}";
  return data;
}*/
/*
void getData() {
  int chk = DHT11.read();
  switch (chk)
  {
  case 0: 
    Serial.println("Read OK"); 
    humidity = (float)DHT11.humidity;
    tempF = DHT11.fahrenheit();
    tempC = DHT11.temperature;
    break;
  case -1: 
    Serial.println("Checksum error"); 
    break;
  case -2: 
    Serial.println("Time out error"); 
    break;
  default: 
    Serial.println("Unknown error"); 
    break;
  }
}*/
