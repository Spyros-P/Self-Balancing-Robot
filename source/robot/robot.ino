#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN
const byte read_address[6] = "Ntu@1";
const byte write_address[6] = "Ntu@2";

// Set if the robot should send info to the controller
#define SEND_INFO 0

#include <MPU6500_WE.h>
#include <Wire.h>
#define MPU6500_ADDR 0x68

#define RAD_TO_DEG 57.295779513082320876798154814105

MPU6500_WE myMPU6500 = MPU6500_WE(MPU6500_ADDR);

double error = 0;
int output = 0;
double theta_deg = 0;

double sum = 0;
double setpoint = 2.0;

unsigned long last_connection = 0;

unsigned long start_time;

unsigned long sum_time = 0;

struct Operating_Data {
  double Kp = 12;
  double Kd = 3;
  double Ki = 7;
  double setpoint = 3;
  double sum_limit = 50;
  double sum_weight = 0.5;
  double exp_dec_sum = 0.95;
  double exp_inc_sum = 1.01;
} Default_Data, Received_Data;

Operating_Data *get_Data = &Default_Data;

// Create a stucture to hold some measurements (in order to send them over the radio)
struct Robot_Info {
  double theta_deg;
  unsigned long last_connection;
} robot_data;

// the setup routine runs once when you press reset:
void setup() {
  radio_setup();
  MPU_setup();
  
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
}

// the loop routine runs over and over again forever:
void loop() {
  radio.openReadingPipe(0, read_address);   //Setting the address at which we will receive the data
  radio.startListening();              //This sets the module as receiver

  bool received_data = false;
  start_time = millis();
  
  while(!received_data && millis() - start_time < 10) {
    if (radio.available()) {
      radio.read(&Received_Data, sizeof(Received_Data));
      get_Data = &Received_Data;
      last_connection = millis();
      received_data = true;
    }
  }
  /*
  if (!received_data && millis() - last_connection < 250) {
    get_Data = &Default_Data;
  }
  */

  xyzFloat gValue = myMPU6500.getGValues();
  xyzFloat gyr = myMPU6500.getGyrValues();

             
  if(gValue.z != 0) theta_deg = atan2(gValue.x, gValue.z)*RAD_TO_DEG;

  error = theta_deg - get_Data->setpoint;

#if SEND_INFO == 1

  radio.openWritingPipe(write_address);
  radio.stopListening();              //This sets the module as transmitter

  robot_data.theta_deg = theta_deg;
  robot_data.last_connection = millis() - last_connection;

  start_time = millis();
  while (!radio.write(&robot_data, sizeof(robot_data)) &&
         millis() - start_time < 10) delay(2);
#endif

  if (theta_deg > 50 || theta_deg < -50) {
    output = 0;
  }
  else {
    unsigned long temp_time = millis();
    if ((sum>0 && theta_deg<0) || (sum<0 && theta_deg>0))
      sum = get_Data->sum_weight*error*(temp_time-sum_time) + get_Data->exp_dec_sum*sum;
    else
      sum = get_Data->sum_weight*error*(temp_time-sum_time) + get_Data->exp_inc_sum*sum;
    
    sum_time = temp_time;
    
    sum = min(max(-get_Data->sum_limit,sum),get_Data->sum_limit);
    output = get_Data->Kp*error - get_Data->Kd*gyr.y + get_Data->Ki*sum;
  }

  drive_motors(output);
}



inline void radio_setup() {
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.setDataRate(RF24_250KBPS);
}

inline void MPU_setup() {
  Wire.begin();

  myMPU6500.setAccOffsets(-14240.0, 18220.0, -17280.0, 15590.0, -20930.0, 12080.0);
  myMPU6500.setGyrOffsets(45.0, 145.0, -105.0);
  myMPU6500.enableGyrDLPF();
  //myMPU6500.disableGyrDLPF(MPU6500_BW_WO_DLPF_8800); // bandwdith without DLPF

  myMPU6500.setGyrDLPF(MPU6500_DLPF_4);
  myMPU6500.setSampleRateDivider(0);
  myMPU6500.setGyrRange(MPU6500_GYRO_RANGE_250);
  myMPU6500.setAccRange(MPU6500_ACC_RANGE_2G);
  myMPU6500.enableAccDLPF(true);
  myMPU6500.setAccDLPF(MPU6500_DLPF_4);

  //myMPU6500.autoOffsets();
}

inline void drive_motors(int output) {
  output *= -1;
  if (output >= 0) {
    digitalWrite(2, LOW);
    digitalWrite(4, LOW);

    output = min(output,255);

    analogWrite(3,output);
    analogWrite(5,output);
  }
  else {
    digitalWrite(2, HIGH);
    digitalWrite(4, HIGH);

    output = min(-output,255);

    analogWrite(3,255-output);
    analogWrite(5,255-output);
  }
}
