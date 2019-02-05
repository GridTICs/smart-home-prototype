int i = 0;
int j = 0;
char c[100];

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial1.setTimeout(1000);

  while(!Serial); 
  while(!Serial1);
  
  sendToESP8266("AT");
  receiveFromESP8266();
  printInSerialPort();
}

void loop() {
  Serial.println();
  Serial.println();
  sendToESP8266("AT+GMR");
  receiveFromESP8266();
  printInSerialPort();
  
  Serial.println();
  Serial.println();
  sendToESP8266("AT+RST");
  receiveFromESP8266();
  printInSerialPort();
    
  Serial.println();
  Serial.println();
  sendToESP8266("AT+CWLAP");
  receiveFromESP8266();
  printInSerialPort();
}

void sendToESP8266 (String message){
  Serial1.println(message);
  delay (3000);
}

void receiveFromESP8266(){
  i = 0;
  while (Serial1.available() > 0) {
    c[i] = Serial1.read();
    i++;
  }  
}

void printInSerialPort(){
  for (j = 0; j <=i; j++){
  Serial.print(c[j]);
  }
  i = 0;
  j = 0;
}
