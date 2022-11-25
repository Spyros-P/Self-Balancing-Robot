#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
RF24 radio(7, 8); // CE, CSN         
const byte address[6] = "Ntu@X";     //Byte of array representing the address. This is the address where we will send the data. This should be same on the receiving side.

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define Kp_MAX 50.0
#define Kd_MAX 10.0
#define Ki_MAX 15.0

unsigned long prev_time = 0;

struct Operating_Data {
  double Kp = 7;
  double Kd = 3;
  double Ki = 3;
  double setpoint = 1.65;
  double sum_limit = 80;
  double sum_weight = 0.5;
  double exp_dec_sum = 0.97;
  double exp_inc_sum = 1.03;
} Transmitting_Data;

void setup() {
	lcd.begin();
	lcd.backlight();
  //Serial.begin(115200);
  radio.begin();                  //Starting the Wireless communication
  radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.setDataRate(RF24_250KBPS);
  //radio.setDataRate(RF24_1MBPS);
  radio.stopListening();          //This sets the module as transmitter
}

void loop() {

  Transmitting_Data.Kp = (1023-analogRead(0))*Kp_MAX/1023;
  Transmitting_Data.Kd = (1023-analogRead(1))*Kd_MAX/1023;
  Transmitting_Data.Ki = (1023-analogRead(2))*Ki_MAX/1023;

  radio.write(&Transmitting_Data, sizeof(Transmitting_Data));

  //delay(1);

  if (millis() - prev_time > 150 ) {
    lcd.setCursor(0,0);
    lcd.print("Kp:");
    if(Transmitting_Data.Kp<10) lcd.print(" ");
    lcd.print(Transmitting_Data.Kp);
    lcd.setCursor(9,0);
    lcd.print("Kd:");
    lcd.print(Transmitting_Data.Kd);
    lcd.setCursor(0,1);
    lcd.print("Ki:");
    if(Transmitting_Data.Ki<10) lcd.print(" ");
    lcd.print(Transmitting_Data.Ki);
    lcd.setCursor(9,1);
    lcd.print("st:");
    lcd.print(Transmitting_Data.setpoint);
   prev_time = millis();
  }

/*
  Serial.print("Kp:");
  Serial.print(Transmitting_Data.Kp);
  Serial.print(", Kd:");
  Serial.print(Transmitting_Data.Kd);
  Serial.print(", Ki:");
  Serial.print(Transmitting_Data.Ki);
  Serial.print(", setpoint:");
  Serial.println(Transmitting_Data.setpoint);
*/
}
