#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00010";

#include <MPU6500_WE.h>
#include <Wire.h>
#define MPU6500_ADDR 0x68

#include <Servo.h>
Servo myservo;  // create servo object to control a servo

//#define PRINT 0
#define RAD_TO_DEG 57.295779513082320876798154814105

MPU6500_WE myMPU6500 = MPU6500_WE(MPU6500_ADDR);

//int servo_remember = 90;

double setpoint = 335;
double error = 0, prev_error = 0;
bool button_status = false;
bool power_off = true;
int output = 0;
double theta;
double theta_deg;

double sum = 0;

double K[] = {0,0};

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MIN);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();              //This sets the module as receiver

  myservo.attach(9);
  myservo.write(90);
  pinMode(10, OUTPUT);
  digitalWrite(10,LOW);
  
#ifdef PRINT
  //Serial.begin(115200);
#endif
  Wire.begin();

#ifdef PRINT
  if(!myMPU6500.init()){
    //Serial.println("MPU6500 does not respond");
  }
  else{
    //Serial.println("MPU6500 is connected");
  }
  //Serial.println("Position you MPU6500 flat and don't move it - calibrating...");
#endif

  for (int i=0; i<5 ;i++) {
    delay(200);
    digitalWrite(10,LOW);
    delay(200);
    digitalWrite(10,HIGH);
  }
  delay(200);
  digitalWrite(10,HIGH);
  
  myMPU6500.autoOffsets();

#ifdef PRINT
  //Serial.println("Done!");
#endif

  //myMPU6500.setAccOffsets(-14240.0, 18220.0, -17280.0, 15590.0, -20930.0, 12080.0);
  //myMPU6500.setGyrOffsets(45.0, 145.0, -105.0);
  myMPU6500.enableGyrDLPF();
  //myMPU6500.disableGyrDLPF(MPU6500_BW_WO_DLPF_8800); // bandwdith without DLPF

  myMPU6500.setGyrDLPF(MPU6500_DLPF_6);
  myMPU6500.setSampleRateDivider(5);
  myMPU6500.setGyrRange(MPU6500_GYRO_RANGE_250);
  myMPU6500.setAccRange(MPU6500_ACC_RANGE_2G);
  myMPU6500.enableAccDLPF(true);
  myMPU6500.setAccDLPF(MPU6500_DLPF_6);
  
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, INPUT);
  

  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(10, HIGH);
}

// the loop routine runs over and over again forever:
void loop() {
    
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


    
    if (radio.available()) {
      radio.read(&K, sizeof(K));
      //Serial.print("Kp: ");
      //Serial.print(K[0]);
      //Serial.print(", Kd: ");
      //Serial.println(K[1]);
    }
    //else {
      //Serial.println("Could not connect!");
    //}
    
    Serial.println("Attempting to read...");
    xyzFloat gValue = myMPU6500.getGValues();
    Serial.println("Read succesfully!");


  //Serial.println("Acceleration in g (x,y,z):");
  //Serial.print(gValue.x);
  //Serial.print("   ");
  //Serial.print(gValue.y);
  //Serial.print("   ");
  //Serial.println(gValue.z);
    
    
    if(gValue.z != 0){
      theta = atan(gValue.x/gValue.z);
      theta_deg = atan(gValue.x/gValue.z)*RAD_TO_DEG;
      //theta = sin(atan(gValue.x/gValue.z));
    }
    
    
    
#ifdef PRINT
    //Serial.print("angle: ");
    //Serial.println(theta);
#endif

    prev_error = error;
    error = theta_deg;

    sum = 0.9*error + 0.3*sum;

    sum = min(max(-40,sum),40);

    output = K[0]*error + K[1]*(error-prev_error);

#ifdef PRINT
    //Serial.print("output: ");
    //Serial.println(output);
#endif

    output *= 4 /(abs(output)/10+1)+1;
    //output *= 4/(abs(output)/10+1)+1;


    //Serial.println(output);

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
    //delay(40);
    
}

/*
void servo_soft(int x) {
  int a,i=servo_remember;
  if (x > servo_remember) a=1;
  else a=-1;
  while(i!=x) {
    i+=a;
    myservo.write(i);
    delay(3);
  }
  servo_remember = x;
  }
*/
