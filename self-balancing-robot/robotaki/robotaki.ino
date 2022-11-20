#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN
const byte address[6] = "Ntu@X";

#include <MPU6500_WE.h>
#include <Wire.h>
#define MPU6500_ADDR 0x68

//#include <Servo.h>
//Servo myservo;  // create servo object to control a servo

#define RAD_TO_DEG 57.295779513082320876798154814105

MPU6500_WE myMPU6500 = MPU6500_WE(MPU6500_ADDR);

double error = 0;
int output = 0;
double theta_deg = 0;

double sum = 0;

struct Operating_Data {
  double Kp = 14;
  double Kd = 3;
  double Ki = 8;
  double setpoint = 2.2;
  double sum_limit = 50;
  double exp_dec_sum = 0.97;
  double exp_inc_sum = 1.01;
} Default_Data, Received_Data, *get_Data;


// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  //Serial.println("STARTING !!!");
  radio_setup();

  //myservo.attach(9);
  //myservo.write(90);
  
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
/*
    do {
      button_status = digitalRead(6);
      if (!button_status) {
          digitalWrite(2, LOW);
          digitalWrite(3, LOW);
          digitalWrite(4, LOW);
          digitalWrite(5, LOW);
          //digitalWrite(10,LOW);

          theta = 0;
          error = 0;
          prev_error = 0;
          output = 0;
          sum = 0;
          
          myservo.write(90);
          delay(300);          
        while(!button_status)
          button_status = digitalRead(6);
        if (power_off)
          power_off = false;
        else
          power_off = true;
      }
    } while(power_off);
    myservo.write(175);
*/

  
  if (radio.available()) {
    radio.read(&Received_Data, sizeof(Received_Data));
    get_Data = &Received_Data;
  }
  else
    get_Data = &Default_Data;

  xyzFloat gValue = myMPU6500.getGValues();
  xyzFloat gyr = myMPU6500.getGyrValues();

             
  if(gValue.z != 0){
    //gValue.z = 2 - gValue.z;
    theta_deg = atan2(gValue.x, gValue.z)*RAD_TO_DEG;
  }

  error = theta_deg - get_Data->setpoint;
  
  //Serial.print("MIN:-90, MAX:90, angle:");
  //Serial.println(theta_deg);

  if (sum*theta_deg<0)
    sum = error + get_Data->exp_dec_sum*sum;
  else
    sum = error + get_Data->exp_inc_sum*sum;
  
  sum = min(max(-get_Data->sum_limit,sum),get_Data->sum_limit);

  output = get_Data->Kp*error - get_Data->Kd*gyr.y + get_Data->Ki*sum;

  drive_motors(output);
}



inline void radio_setup() {
  radio.begin();
  radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();              //This sets the module as receiver
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
    analogWrite(5,output*0.95);
  }
  else {
    digitalWrite(2, HIGH);
    digitalWrite(4, HIGH);

    output = min(-output,255);

    analogWrite(3,255-output);
    analogWrite(5,255-output*0.95);
  }
}
