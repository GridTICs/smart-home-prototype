int i = 0;
int j = 0;
char c[256];
//int baudRate = 115200;
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
//  Serial1.setTimeout(1000);
  while(!Serial); 
  while(!Serial1);
  receiveFromESP8266();
  delay(1000);
  sendToESP8266("AT");
  receiveFromESP8266();
//  printInSerialPort();
  delay (5000);
}

void loop() {
  Serial.println();
  Serial.println();
  sendToESP8266("AT+GMR");
  receiveFromESP8266();
//  printInSerialPort();
  delay (5000);
  Serial.println();
  Serial.println();
  sendToESP8266("AT+RST");
  delay(4);
  receiveFromESP8266();
//  printInSerialPort();
  delay (5000);  
  Serial.println();
  Serial.println();
  sendToESP8266("AT+CWLAP");
  receiveFromESP8266();
//  printInSerialPort();
  delay (5000);
}

void sendToESP8266 (String message){
  Serial1.println(message);
//  Serial1.flush();
//  delayMicroseconds(800);
}

void cleanBuffer(){
  char c;
  delay(5);
  while (Serial1.available() != 0){
    c = Serial1.read();
  }
}

void receiveFromESP8266(){
  char c;
  while (Serial1.available() != 0) {
    c = Serial1.read();
    Serial.print(c);
//    delayMicroseconds(50);
    i++;
  } 
  Serial.println();
  Serial.print("Datos Leidos = ");
  Serial.println(i);
  i=0; 
}

void printInSerialPort(){
  for (j = 0; j <=i; j++){
  Serial.print(c[j]);
  }
  i = 0;
  j = 0;
}
