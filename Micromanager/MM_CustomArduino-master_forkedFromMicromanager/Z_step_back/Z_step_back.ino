// Code that controls the z axis, with safety integrated into the code using the limit switch readout

//including libraries
#include <AccelStepper.h>

// Defining constants for pins
const int enablePin = 8;  // 使能控制引脚

const int zdirPin = 5;     // z方向控制引脚 (用的是cnc shield上的x)
const int zstepPin = 2;    // z步进控制引脚
const int zdirPin2 = 6;     // z2方向控制引脚 (用的是cnc shield上的y)
const int zstepPin2 = 3;    // z2步进控制引脚

const float Maxspeed = 30000.0;        //step/s
const float Acceleration = 1000.0;     //step/s^2
const float Reduction = 10.0;     //Reduction ratio for actuators
const float BaseRes = 1.8;        //Base resolution of actuator in deg
const float Pitch = 2000.0;       //Lead screw pitch in microns
const float stepSize = Pitch / Reduction / 360.0 * BaseRes; // step size of motor in microns
const int limitswitchPin = 14;   // setup the pin

// Defining stepper motor pin outs
AccelStepper stepper1(1, zstepPin, zdirPin); //建立步进电机对象1
AccelStepper stepper2(1, zstepPin2, zdirPin2); //建立步进电机对象2

// Initialize comm. variables
String cmd = "" ; // initialize recieved command from serial comm. port
float z = 0.0; // initialize current z axis command to be 0, with units in um
float currentPos = 0.0; //initialize current position to be 0, with units undefined yet
int zDim = 1;  // initialize unit of z axis control to be in um

// Setting up pins
void setup() {
  Serial.begin( 115200 );       //Use for 115200 Baud Rate
  //Serial.begin( 9600 );         //Use for 9600 Baud Rate
  pinMode(zstepPin, OUTPUT);    // Arduino控制A4988x步进引脚为输出模式
  pinMode(zdirPin, OUTPUT);     // Arduino控制A4988x方向引脚为输出模式
  pinMode(zstepPin2, OUTPUT);    // Arduino控制A4988x步进引脚为输出模式
  pinMode(zdirPin2, OUTPUT);     // Arduino控制A4988x方向引脚为输出模式
  pinMode(enablePin, OUTPUT);  // Arduino控制A4988使能引脚为输出模式
  digitalWrite(enablePin, LOW); // 将使能控制引脚设置为低电平从而让
  pinMode(limitswitchPin, INPUT_PULLUP); //limitswitch
  // set up speed and acceleration parameters
  stepper1.setMaxSpeed(Maxspeed);     // step/s
  stepper1.setAcceleration(Acceleration);  //step/s^2
  stepper2.setMaxSpeed(Maxspeed);     // step/s
  stepper2.setAcceleration(Acceleration);  //step/s^2

  reply ("Vers:LS_forZAxis"); // respond to Micromanager to confirm stable connection
}

void loop() // main code starts here
{
  if (digitalRead(limitswitchPin) == HIGH) { //Checking if the limit switch is touched
    if (Serial.available()) {
      cmd = Serial.readStringUntil('\r');
      processCommand(cmd);
      cmd = "";
    }

    turnStepper();
    stepper1.setMaxSpeed(Maxspeed);     // step/s
    stepper1.setAcceleration(Acceleration);  //step/s^2
    stepper2.setMaxSpeed(Maxspeed);     // step/s
    stepper2.setAcceleration(Acceleration);  //step/s^2
    stepper1.run();
    stepper2.run();

  }

  else {
    backStepper(); // turn the stepper motors back to place
    stepper1.setMaxSpeed(Maxspeed);     // step/s
    stepper1.setAcceleration(Acceleration);  //step/s^2
    stepper2.setMaxSpeed(Maxspeed);     // step/s
    stepper2.setAcceleration(Acceleration);  //step/s^2
    stepper1.run();
    stepper2.run();
    // Serial.println("STOP"); // used for degbuging purposes
  }
}


void processCommand(String s) {
  if (s.startsWith("?ver")) {
    reply ("Vers:LS");
  } else if (s.startsWith("!autostatus 0")) {
    delay(5);
  } else if (s.startsWith("?det")) {
    reply ("60");
  } else if (s.startsWith("?pitch z")) {
    reply (String(Pitch/Reduction,1));
  } else if (s.startsWith("?vel z")) { // asking about speed in mm/s
    reply (String((stepper1.speed() + stepper2.speed()) / 2.0 * stepSize / 1000.0, 1)); //report average speed and convert from um/s to mm/s
  } else if (s.startsWith("?accel z")) { // asking about acceleration in m/s^2
    reply(String(Acceleration *  stepSize / 1000000)); //converted acceleration to m/s^2
  } else if (s.startsWith("!dim z 1")) { // switching dimensions to be in um
    zDim = 1;
  } else if (s.startsWith("!dim z 2")) { // switching dimensions to be in mm/s
    zDim = 2;
  } else if (s.startsWith("!dim z 0")) { // switching dimensions to be in steps
    zDim = 0; 
  } else if (s.startsWith("?statusaxis")) {
    if (stepper1.speed() != 0 || stepper2.speed() != 0) {
      reply ("M");
    }
    else {
      reply("@");
    }
  } else if (s.startsWith("!vel z")) {
    delay(5);
  } else if (s.startsWith("!accel z")) {
    delay(5);
  } else if (s.startsWith("?pos z")) {
    currentPos = (stepper1.currentPosition() + stepper2.currentPosition()) / 2.0; // in steps
    if (zDim == 1){
      currentPos = currentPos*stepSize; // in um
      }
    String zs = String(currentPos, 1); //report current average height from the two stepper motors
    reply (zs);
  } else if (s.startsWith("?lim z")) {
    reply ("-100000000000000000.0 1000000000000000000.0"); // report a very large limit for the z axis motion
  } else if (s.startsWith("!pos z")) { // just understanding Micromanager
    delay(5);
  } else if (s.startsWith("?status")) {// responding to Micromanager such that computer knows Arduino is connected
    reply ("OK...");
  } else if (s.startsWith("!speed z")) { // just understanding Micromanager
    delay(5);
  } else if (s.startsWith("!mor z")) { // responding to relative motion command from Micromanager
    String delta = s.substring(s.indexOf("z") + 1);
    if (zDim == 0){delta = delta.toFloat()*stepSize;} // dealing with command dimension in steps
    z = z + delta.toFloat();
    turnStepper();
  } else if (s.startsWith("!moa z")) { // responding to absolute motion command from Micromanager
    String apos = s.substring(s.indexOf("z") + 1);
    apos = apos.toFloat(); // convert string to float, assuming dimension is in um
    if (zDim == 0){apos = apos.toFloat() * stepSize;} // dealing with command dimension in steps
    z = apos.toFloat();
    turnStepper();
  } else if (s.startsWith("?err")) {
    reply ("0");
  }
}

void reply(String s) { // Function used to send responses to Micromanager
  Serial.print(s);
  Serial.print("\r");

}

void turnStepper() {
  stepper1.moveTo(z / stepSize);
  stepper2.moveTo(z / stepSize);
  //  Serial.println(z); // for debugging reasons
}


void backStepper() {
  stepper1.moveTo(-z / (stepSize * 40.0));
  stepper2.moveTo(-z / (stepSize * 40.0));

}
