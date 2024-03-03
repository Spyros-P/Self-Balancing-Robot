#include <ArduinoJson.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
RF24 radio(7, 8); // CE, CSN
const byte write_address[6] = "Ntu@1";     //Byte of array representing the address. This is the address where we will send the data. This should be same on the receiving side.
const byte read_address[6] = "Ntu@2";

// Set if the controller should receive info from the robot
#define RECEIVE_INFO 0

unsigned long start_time;

struct Operating_Data {
  double Kp = 15;
  double Kd = 4;
  double Ki = 3;
  double setpoint = 1.65;
  double sum_limit = 80;
  double sum_weight = 0.5;
  double exp_dec_sum = 0.8;
  double exp_inc_sum = 1.01;
} Transmitting_Data;

struct Robot_Info {
  double theta_deg;
  unsigned long last_connection;
} robot_data;

void setup() {
  Serial.begin(115200);
  radio.begin();                  //Starting the Wireless communication
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.setDataRate(RF24_250KBPS);
  //radio.setDataRate(RF24_1MBPS);
}

void loop() {
  radio.openWritingPipe(write_address); //Setting the address where we will send the data
  radio.stopListening();          //This sets the module as transmitter

  // read serial input (struct is read as json string)
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    // flush serial buffer
    while (Serial.available() > 0) Serial.read();
    // parse json string to struct
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, input);
    Transmitting_Data.Kp = doc["Kp"];
    Transmitting_Data.Kd = doc["Kd"];
    Transmitting_Data.Ki = doc["Ki"];
    Transmitting_Data.setpoint = doc["setpoint"];
    Transmitting_Data.sum_limit = doc["sum_limit"];
    Transmitting_Data.sum_weight = doc["sum_weight"];
    Transmitting_Data.exp_dec_sum = doc["exp_dec_sum"];
    Transmitting_Data.exp_inc_sum = doc["exp_inc_sum"];
  }
  

  start_time = millis();
  while (!radio.write(&Transmitting_Data, sizeof(Transmitting_Data)) &&
          millis() - start_time < 100) delay(2);

#if RECEIVE_INFO == 1
  radio.openReadingPipe(0, read_address);
  radio.startListening();         //This sets the module as receiver

  bool received_data = false;
  start_time = millis();
  
  while(!received_data && millis() - start_time < 20) {
    if (radio.available()) {
      radio.read(&robot_data, sizeof(robot_data));
      received_data = true;
    }
  }

  if (received_data) {
    // pack struct to json string
    DynamicJsonDocument doc(1024);
    doc["theta_deg"] = robot_data.theta_deg;
    doc["last_connection"] = robot_data.last_connection;
    String output;
    serializeJson(doc, output);
    Serial.println(output);
  }
#endif
}