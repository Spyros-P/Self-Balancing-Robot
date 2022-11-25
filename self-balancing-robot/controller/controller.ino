#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN         
const byte address[6] = "Ntu@X";     //Byte of array representing the address. This is the address where we will send the data. This should be same on the receiving side.

#define Kp_MAX 50.0
#define Kd_MAX 10.0
#define Ki_MAX 15.0

struct Operating_Data {
  double Kp = 14;
  double Kd = 3;
  double Ki = 8;
  double setpoint = 2.5;
  double sum_limit = 120;
  double sum_weight = 0.6;
  double exp_dec_sum = 0.97;
  double exp_inc_sum = 1.03;
} Transmitting_Data;

void setup() {
  Serial.begin(115200);
  radio.begin();                  //Starting the Wireless communication
  radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.setDataRate(RF24_250KBPS);
  //radio.setDataRate(RF24_1MBPS);
  radio.stopListening();          //This sets the module as transmitter
}

void loop() {

  Transmitting_Data.Kp = analogRead(0)*Kp_MAX/1023;
  Transmitting_Data.Kd = analogRead(1)*Kd_MAX/1023;
  Transmitting_Data.Ki = analogRead(2)*Ki_MAX/1023;

  radio.write(&Transmitting_Data, sizeof(Transmitting_Data));

  //delay(1);


  Serial.print("Kp:");
  Serial.print(Transmitting_Data.Kp);
  Serial.print(", Kd:");
  Serial.print(Transmitting_Data.Kd);
  Serial.print(", Ki:");
  Serial.print(Transmitting_Data.Ki);
  Serial.print(", setpoint:");
  Serial.println(Transmitting_Data.setpoint);

}
