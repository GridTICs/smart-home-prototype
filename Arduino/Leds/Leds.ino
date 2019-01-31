int ledPinRed = 3;      // LED connected to digital pin 9
int ledPinWhite = 5;
int ledPinYellow = 6;
int ledPinGreen = 9;



int analogPin = 10;   // potentiometer connected to analog pin 10
int val = 0;         // variable to store the read value

void setup()
{
  pinMode(ledPinRed, OUTPUT);   // sets the pin as output
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinYellow, OUTPUT);
  pinMode(ledPinWhite, OUTPUT);
}

void loop()
{
  val = analogRead(analogPin);   // read the input pin
  analogWrite(ledPinRed, val / 4);  // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
  analogWrite(ledPinWhite, val / 4);
  analogWrite(ledPinYellow, val / 4);
  analogWrite(ledPinGreen, val / 4);
}
