#include <Wire.h>

int BH1750_address = 0x23; // i2c Addresse
byte buff[2];

void setup(){
  Wire.begin();
  BH1750_Init(BH1750_address);
  
  delay(200);
  Serial.begin(9600);
  Serial.println("Starte Beleuchtungsstaerkemessung - blog.simtronyx.de");
}

void loop(){
  float valf = 0;
  Serial.println("Bueno empezo");
  if(BH1750_Read(BH1750_address) == 2){
    Serial.println("Bueno entro al loop");
    valf =((buff[0] <<8)| buff[1]) /1.2;
    
    if(valf < 0){
      Serial.print("> 65535");
    }
    else {
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
  byte i=0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  
  while(Wire.available()){
    buff[i] = Wire.read(); 
    i++;
  }
  
  Wire.endTransmission();  
  return i;
}
