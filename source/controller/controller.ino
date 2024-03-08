#include <stdio.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
RF24 radio(7, 8); // CE, CSN
const byte write_address[6] = "Ntu@1";     //Byte of array representing the address. This is the address where we will send the data. This should be same on the receiving side.
const byte read_address[6] = "Ntu@2";

// Set if the controller should receive info from the robot
#define RECEIVE_INFO 1
// Set if the controller should send info to the robot
#define SEND_INFO 1


unsigned long start_time;


struct Operating_Data {
  float Kp = 15;
  float Kd = 4;
  float Ki = 3;
  float setpoint = 1.65;
  float sum_limit = 80;
  float sum_weight = 0.5;
  float exp_dec_sum = 0.8;
  float exp_inc_sum = 1.01;
} Transmitting_Data;

struct Robot_Info {
  float theta_deg;
  float gyr;
  float acc_x;
  float acc_z;
  float sum;
  float output;
} robot_data;

FILE uart_stream;

int sput(char c, __attribute__((unused)) FILE* f) {return !Serial.write(c);}


void setup() {
  Serial.begin(57600);
  fdev_setup_stream(&uart_stream, sput, nullptr, _FDEV_SETUP_WRITE);
  stdout = &uart_stream;
  
  radio.begin();                  //Starting the Wireless communication
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.setDataRate(RF24_250KBPS);
  //radio.setDataRate(RF24_1MBPS);
}

void loop() {
#if SEND_INFO == 1
  radio.openWritingPipe(write_address); //Setting the address where we will send the data
  radio.stopListening();          //This sets the module as transmitter

  // read serial input (struct is read as json string)
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    // flush serial buffer
    while (Serial.available() > 0) Serial.read();
    
    int res = sscanf(input.c_str(),
                     "{\"Kp\": %f, "\
                     "\"Kd\": %f, "\
                     "\"Ki\": %f, "\
                     "\"setpoint\": %f, "\
                     "\"sum_limit\": %f, "\
                     "\"sum_weight\": %f, "\
                     "\"exp_dec_sum\": %f, "\
                     "\"exp_inc_sum\": %f}",
                     &Transmitting_Data.Kp,
                     &Transmitting_Data.Kd,
                     &Transmitting_Data.Ki,
                     &Transmitting_Data.setpoint,
                     &Transmitting_Data.sum_limit,
                     &Transmitting_Data.sum_weight,
                     &Transmitting_Data.exp_dec_sum,
                     &Transmitting_Data.exp_inc_sum);

    // printf("result: %d\n", res);

    if (res > 0) {
      start_time = millis();
      while (!radio.write(&Transmitting_Data, sizeof(Transmitting_Data)) &&
              millis() - start_time < 100) delay(2);
    }
  }
#endif


#if RECEIVE_INFO == 1
  radio.openReadingPipe(0, read_address);
  radio.startListening();         //This sets the module as receiver

  bool received_data = false;
  start_time = millis();
  
  while(!received_data && millis() - start_time < 100) {
    if (radio.available()) {
      radio.read(&robot_data, sizeof(robot_data));
      received_data = true;
    }
  }

  if (received_data) {
    printf("{\"theta_deg\":%f,"\
            "\"gyr\":%f,"\
            "\"acc_x\":%f,"\
            "\"acc_z\":%f,"\
            "\"sum\":%f,"\
            "\"output\":%f,"\
            "\"acc_tot\":%f}\n",
            robot_data.theta_deg,
            robot_data.gyr,
            robot_data.acc_x,
            robot_data.acc_z,
            robot_data.sum,
            robot_data.output,
            sqrt(robot_data.acc_x*robot_data.acc_x +
                 robot_data.acc_z*robot_data.acc_z));
  }
#endif
}