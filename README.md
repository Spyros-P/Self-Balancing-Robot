# Arduino-Projects
README-DOCUMENTATION OF FINAL PRODUCT

This is a project for a Self Balancing Robot implemented with a PID controller, along with an external controller-transmitter the parameters of which (P, I, D) were modifiable with potentiometers, sent to the robot via a small antenna in real-time, and shown to the user via a screen within the controller interface.

### Result & Verdict

The Robot could reliably balance itsself after tweaking the PID parameters for well over a minute of runtime. It could also keep its balance when very small forces were applied. The reason for that, we believe, were the motors, whose maximum torque was far too low to safely restore balance after every external force application. We found, however, that they were more than capable enough to 'help' the robot restore its' own balance.

### The Robot & The Problem

![image](https://user-images.githubusercontent.com/85502727/211286341-6cc500b6-dd4f-448a-903a-a8ca0f5fd206.png)

The problem we tried to tackle is the equivalent of trying to balance a broomstick at the point of your finger. If it starts tilting (falling) to the right, you have to move your finger to that direction faster than the broomstick to counter its movement so that it doesnt keep falling. Such is the movement of the robot in its attempt to balance itself. 

### What is 'PID'?

![image](https://user-images.githubusercontent.com/85502727/211288493-f618b9b6-6302-440d-a427-da46e0e66604.png)

PID is a mechanism used in balancing (or generally automatic gain control) applications that tweaks the forces applied to the mechanism, in this case the motors, depending on all 3 parameters of 'P', 'I', 'D'. 
#### Proportional
'P' stands for Proportional gain, which is the force (or voltage) which the mechanism will apply to correct the 'error'. In this case, 'error' is the angle at which the robot sits at a given time, regardless of whether it's moving or not. 
#### Integral
'I' stands for Integral gain, which means correction over time. Basically, this parameter is used to detect any movements the robot is inclined to do, for example if it tilts more towards one angle than the other (even though in theory it has been calibrated), then the Integral gain counters that 'behavioral pattern' and many others. In other words, it takes into account previous errors and possibly accumulates a small repeatedly occurring error (since it multiplies over time). 
#### Derivative
'D' stands for Derivative gain and it seeks to counter the robot’s predicted ‘future’ movement. Basically, it takes into account not only the angle at which it stands at a given point in time, but also its speed at that same time, as it is harder, for instance, to balance a rightwards tilted robot that is also falling towards that direction than it is to balance a rightwards tilted robot that is momentarily stationary (due to previously applied forces for example)  In theory and in some well implemented practices, the 'P' factor is enough on it’s own to balance the robot, and the other factors are mostly needed to either reach balance within a tighter time frame or to do so in a smooth manner, but more on that on another chapter.

### Final Build & Components
<sub> Note: We are not in any way, shape or form neither fully nor partially sponsored by any of manufacturers mentioned within this chapter. These are purely the parts used and are indicated so as to enable re-construction of the build by any willing external parties as this is an open source project. <sub>

(2x) Arduino Nano - the Development Board used in the robot and the controller
  
  ![image](https://user-images.githubusercontent.com/85502727/211302574-4858270a-e53b-4300-aad7-e04388e31a7d.png)

  --
MPU 9250/6500 - our gyroscope/accelerometer
  
  ![image](https://user-images.githubusercontent.com/85502727/211302731-3a1371b1-bee2-4a21-be91-8d25317fbee2.png)

  --
(2x) NRF24L01 2.4 GHz - the antennas used for data exchange between the controller and the robot
  
  ![image](https://user-images.githubusercontent.com/85502727/211302890-82fa659f-fd86-46b3-a258-83e731cead0f.png)

  --
(2x) DC Gear Motors - the motors used to spin the wheels of the robot
 
  ![image](https://user-images.githubusercontent.com/85502727/211303588-1f49b88f-a3f0-4e04-ad51-c892a1abb89d.png)

  --
Waveshare motor control board - the board that powered the gear motors with the appropriate voltage
  
  ![image](https://user-images.githubusercontent.com/85502727/211303561-10ae012e-05a9-4fb4-a650-570b0b85141c.png)

  --
18650 Battery Shield - The shield used as a means of connecting the battery safely to the rest of the circuit and also used as a power adaptor of the battery's 5V to 3.3V power output
  
  ![image](https://user-images.githubusercontent.com/85502727/211303840-d92bc0a7-0738-48c7-9658-00c9524394d4.png)

  --
18650 Battery - The battery itself used to power the circuit
  
  ![image](https://user-images.githubusercontent.com/85502727/211307156-86df0010-f314-48e3-b3e4-b0b88fa11951.png)



