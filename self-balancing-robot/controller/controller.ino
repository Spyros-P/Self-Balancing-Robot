#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN         
const byte address[6] = "00010";     //Byte of array representing the address. This is the address where we will send the data. This should be same on the receiving side.

#define Kp_MAX 100.0
#define Kd_MAX 5000.0

double K[] = {0,0};

void setup() {
  Serial.begin(115200);
  radio.begin();                  //Starting the Wireless communication
  radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.setDataRate(RF24_250KBPS);
  radio.stopListening();          //This sets the module as transmitter
}

void loop() {
  K[0] = analogRead(0)*Kp_MAX/1023;
  K[1] = analogRead(1)*Kd_MAX/1023;

  radio.write(&K, sizeof(K));
  //Serial.println("Data Sent");
  
  //delay(2);
}
