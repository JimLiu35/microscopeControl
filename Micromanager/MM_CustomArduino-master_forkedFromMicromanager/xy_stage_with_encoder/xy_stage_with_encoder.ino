// version 3.0(final)
// Utilized source code from a Chinese manufactuere while learning the ways of AccelStepper.h and CNC shield

//Including important libraries
#include <AccelStepper.h>
#include <Encoder.h>

// Defining pin numbers and initial values
const int enablePin = 8;  // 使能控制引脚
const int xdirPin = 5;     // x方向控制引脚
const int xstepPin = 2;    // x步进控制引脚
const int ydirPin = 6;     // y方向控制引脚
const int ystepPin = 3;    // y步进控制引脚
const int limitswitchPin = 14;   // setup the pin

bool state_x = 0;
bool state_y = 0;

const float tolerance = 1.0; // Setting up error tolerances for control in units of steps
const int x_LimPin = 11;    // x limit switch pinout
const int y_LimPin = 12;    // y limit switch pinout

const float Maxspeed = 5000.0;        //step/s
const float Acceleration = 1000.0;     //step/s^2
const float Reduction = 1.0;     //Reduction ratio for actuators
const float BaseRes_y = 0.9;        //Base resolution of actuator in deg
const float BaseRes_x = 1.8;        //Base resolution of actuator in deg
const float Pitch = 2000.0;       //Lead screw pitch in microns
const float stepSize_x = Pitch / Reduction / 360.0 * BaseRes_x; // step size of motor in microns
const float stepSize_y = -Pitch / Reduction / 360.0 * BaseRes_y; // step size of motor in microns

AccelStepper stepper_x(1, xstepPin, xdirPin); //Setting up stepper motor for x axis
AccelStepper stepper_y(1, ystepPin, ydirPin); //Setting up stepper motor for y axis
int firstSpace;         //Used to parse commands for x
int secondSpace;        //Used to parse commands for y
int Mode_x = 2; // Helps with controlling units for x motor
int Mode_y = 2; // Helps with controlling units for y motor
int Mode_xy = 1; // Helps with controlling units for xy stage

String xyPitch = "2000.0"; // in microns
String cmd = "" ; // initialize command read from serial comm. port
float x = 0.0; // initialize current x position
float y = 0.0; // initialize current y position

float x_old = x; // initialize previous x position, used for encoder related control
float y_old = y; // initialize previous y position, used for encoder related control

Encoder Enc_y(18, 19); //setting up encoder pins for y axis
Encoder Enc_x(20, 21); //setting up encoder pins for x axis

long newPosition = 0.0; // initialize new x position, used for encoder related control
long oldPosition  = -999.0; // initialize old x position, used for encoder related control
long newPosition2 = 0.0; // initialize new y position, used for encoder related control
long oldPosition2  = -999.0; // initialize old y position, used for encoder related control



void setup() {
  //Serial.begin(115200); // Set baud rate to 115200
  Serial.begin(9600); // Set baud rate to 9600
  pinMode(xstepPin, OUTPUT);    // Arduino控制A4988x步进引脚为输出模式
  pinMode(xdirPin, OUTPUT);     // Arduino控制A4988x方向引脚为输出模式
  pinMode(ystepPin, OUTPUT);    // Arduino控制A4988y步进引脚为输出模式
  pinMode(ydirPin, OUTPUT);     // Arduino控制A4988y方向引脚为输出模式
  pinMode(enablePin, OUTPUT);  // Arduino控制A4988使能引脚为输出模式
  digitalWrite(enablePin, LOW); // 将使能控制引脚设置为低电平从而让
  pinMode(limitswitchPin, INPUT_PULLUP); // setting up limitswitch
  // 电机驱动板进入工作状态

  stepper_x.setMaxSpeed(Maxspeed);     // 设置电机最大速度
  stepper_x.setAcceleration(Acceleration);  // 设置电机加速度
  stepper_y.setMaxSpeed(Maxspeed);     // 设置电机最大速度
  stepper_y.setAcceleration(Acceleration);  // 设置电机加速度
  reply ("Vers:LS_xy"); 
  //  calibrateStage(); //keep this function as a comment until limit switch to the xy stage is actually installed
}

void loop()// Main piece of code. Basically, we repeat the process of reading from serial comm ports, responding to command, and move motor to desired position
{
  if (digitalRead(limitswitchPin) == HIGH) {
    if (Serial.available()) {
      cmd = Serial.readStringUntil('\r');
      processCommand(cmd);
      cmd = "";
    }

    // Position Feedback control using position read from encoder
    newPosition = Enc_x.read();
    newPosition2 = Enc_y.read();

    // Reading in positions for position control
    if (newPosition != oldPosition) {
      oldPosition = newPosition;
      //    Serial.println(newPosition); //used for debugging
    }
    if (newPosition2 != oldPosition2) {
      oldPosition2 = newPosition2;
      //    Serial.println(newPosition2/3*2*stepSize_y); //used for debugging

    }
    
    // Sending position commands to the stepper motor
    if (x / stepSize_x - Enc_x.read() / 3.0 / 4.0 > tolerance ||  Enc_x.read() / 3.0 / 4.0 - x / stepSize_x > tolerance ) { //determining if position feedback is needed to reach desired tolerance
      stepper_x.move(x / stepSize_x - newPosition / 3.0 / 4.0 ); // Set position for control
      stepper_x.setAcceleration(Acceleration);
      stepper_x.run(); // Set stepper motor to run
    }
    else { // if position is reached, then stop the motor
      stepper_x.stop();
      stepper_x.setSpeed(0);
      stepper_x.setAcceleration(0);
      //    Serial.println("trying stop"); // for debuging
    }


    if (y / stepSize_y - Enc_y.read() * 2.0 / 3.0 / 4.0  > tolerance ||  Enc_y.read() / 3.0 / 4.0 * 2.0 - y / stepSize_y > tolerance ) { //determining if position feedback is needed to reach desired tolerance
      stepper_y.move(y / stepSize_y  - newPosition2 / 3.0 / 4.0 * 2.0); // Set position for control
      stepper_y.setAcceleration(Acceleration);
      stepper_y.run(); // Set stepper motor to run
    }
    else { // if position is reached, then stop the motor
      stepper_y.stop();
      stepper_y.setSpeed(0);
      stepper_y.setAcceleration(0);
      //    Serial.println("trying stop"); // for debugging
    }

  }

  else { // this is when the limit switch is pressed, which pauses the system
    delay(10);
//    Serial.println("STOP");
  }
}
void processCommand(String s) {
  // Initial Settings and command queue
  if (s.startsWith("?ver")) {
    reply ("Vers:LS");
  } else if (s.startsWith("!autostatus 0")) {
    delay(5);
  } else if (s.startsWith("?det")) {
    reply ("60");

    // Orientation information
  } else if (s.startsWith("?pitch x")) {
    reply (xyPitch); // Pitch of the x-motor in microns
  } else if (s.startsWith("?pitch y")) {
    reply (xyPitch); // Pitch of the y-motor in microns

  } else if (s.startsWith("?vel x")) {
    if (Mode_x == 2) {
      reply (String(stepper_x.speed() * stepSize_x / 1000)); // mm/s of the x-motor
    }
  } else if (s.startsWith("?vel y")) {
    if (Mode_y == 2) {
      reply (String(stepper_y.speed() * stepSize_y / 1000)); // mm/s of the y-motor      
    }
  } else if (s.startsWith("?accel x")) {
    reply(String(Acceleration *  stepSize_x / 1000000)); //converted acceleration to m/s^2
  } else if (s.startsWith("?accel y")) {
    reply(String(Acceleration * stepSize_y / 1000000)); //converted acceleration to m/s^2


    //Setting Acceleration and Velocity Values (we decided to comment out some scripts here as we found acceleration control from Micromanager is not ideal)
  } else if (s.startsWith("!accel x")) {
    String xa = s.substring(s.indexOf("!accel x ") + 1);
    //    Acceleration = xa.toFloat()*Reduction/stepSize_x*1000000;//converts m/s^2 to step/s^2
    //    stepper_x.setAcceleration(Acceleration);
    delay(5);
  } else if (s.startsWith("!accel y")) {
    String ya = s.substring(s.indexOf("!accel y ") + 1);
    //    Acceleration = ya.toFloat()*Reduction/stepSize_y*1000000; //converts m/s^2 to step/s^2
    //    stepper_y.setAcceleration(Acceleration);
    delay(5);
  } else if (s.startsWith("!vel y")) {
    String yv = s.substring(s.indexOf("!vel y ") + 1);
    float yv_num = yv.toFloat();
    if (Mode_y == 2) { // mm/sec of output
      float vy = yv_num * Reduction / stepSize_y;
    }
  } else if (s.startsWith("!vel x")) {
    String xv = s.substring(s.indexOf("!vel x ") + 1);
    float xv_num = xv.toFloat();
    if (Mode_x == 2) { // mm/sec of output
      float vx = xv_num * Reduction / stepSize_x;
    }

    //Tells micromanager if stage is moving or not
  } else if (s.startsWith("?statusaxis")) { 
    String response = "@@@"; //should return @ if motors are not moving
    // return M if motor is moving
    if (stepper_x.speed()) {
      response[0] = 'M';
    }
    if (stepper_y.speed()) {
      response[1] = 'M';
    }
    reply(response);

    //Tells micromanager current stage position
  } else if (s.startsWith("?pos")) {
    if (Mode_xy == 1) { // Info should be in microns
      String xPos = String(Enc_x.read() /3.0 / 4.0 * stepSize_x);
      String xy_pos = xPos + " " + String(Enc_y.read() /3.0 / 4.0 *2.0 * stepSize_y);
      reply (xy_pos);
    }
    else if (Mode_xy == 0) { // Asking for current step
      String xPos = String(Enc_x.read() /3.0 / 4.0, 1);
      String xy_pos = xPos + " " + String(Enc_y.read() /3.0 / 4.0 *2.0, 1);
      reply (xy_pos);
    }

    // Dimension and Modes
  } else if (s.startsWith("!dim 2 2")) { // switch to rev/s
    Mode_xy = 2;
  } else if (s.startsWith("!dim 1 1")) { // switch to um
    Mode_xy = 1;
  } else if (s.startsWith("!dim 0 0")) { // switch to steps
    Mode_xy = 0;
  } else if (s.startsWith("!dim x 2")) { // switch to steps
    Mode_x = 2;
  } else if (s.startsWith("!dim y 2")) { // switch to steps
    Mode_y = 2;

    // Relative Motion Control
  } else if (s.startsWith("!mor ")) {   //relative motion
    // Finding the locations for x and y command
    firstSpace = s.indexOf(' ');
    secondSpace = s.indexOf(' ', firstSpace + 1);
    String delta_x = s.substring(firstSpace, secondSpace);
    String delta_y = s.substring(secondSpace);//Serial.print(apos_y);
    float delta_x_num = delta_x.toFloat();
    float delta_y_num = delta_y.toFloat();
    if (Mode_xy == 0) { // Changing step commands to um commands
      delta_x_num = delta_x_num * stepSize_x;
      delta_y_num = delta_y_num * stepSize_y;
    }
    // Setting target x and y locations
    x = x + delta_x_num;
    y = y + delta_y_num;

    // Absolute Motion Control
  } else if (s.startsWith("!moa ")) {   //relative motion, assume in um
    firstSpace = s.indexOf(' ');
    secondSpace = s.indexOf(' ', firstSpace + 1);
    String apos_x = s.substring(firstSpace, secondSpace);
    String apos_y = s.substring(secondSpace);//Serial.print(apos_y);
    float apos_x_num = apos_x.toFloat();
    float apos_y_num = apos_y.toFloat();
    if (Mode_xy == 0) { // Changing step commands to um command
      apos_x_num = apos_x_num * stepSize_x;
      apos_y_num = apos_y_num * stepSize_y;
    }
    // Setting target x and y locations
    x = apos_x_num;
    y = apos_y_num;

    // Setting boundaries for x and y in microns (we have a huge stage, so we just load the limit to be large)
  } else if (s.startsWith("?lim x")) {
    if (Mode_xy == 1) {
      reply ("-100000000000000.0 100000000000000.0");
    }
    else if (Mode_xy == 0) {
      reply ("-100000000000000.0 100000000000000.0");
    }
  } else if (s.startsWith("?lim y")) {
    if (Mode_xy == 1) {
      reply ("-100000000000000.0 100000000000000.0");
    }
    else if (Mode_xy == 0) {
      reply ("-100000000000000.0 100000000000000.0");
    }

    // Origins and Callibrations
  } else if (s.startsWith("!cal x")) {  //scalibrate x origin
    x = 0.0;
  } else if (s.startsWith("!cal y")) {  //scalibrate y origin
    y = 0.0;
  } else if (s.startsWith("!pos 0 0")) {  //setting origin
    x = 0.0;
    y = 0.0;
    stepper_x.setCurrentPosition(0.0);
    stepper_y.setCurrentPosition(0.0);
    Enc_x.write(0.0);
    Enc_y.write(0.0);
  } else if (s.startsWith("!pos ")) {   //setting arbitrary position
    String x_set = s.substring(s.indexOf("!pos ") + 1);
    String y_set = s.substring(s.indexOf("!pos ") + 3);
    x = x_set.toFloat();
    y = y_set.toFloat();
    stepper_x.setCurrentPosition(x);
    stepper_y.setCurrentPosition(y);
    Enc_x.write(x / stepSize_x / 3);
    Enc_y.write(x / stepSize_y / 3 * 2); //Need to include the *2 to because step size of y motor is 0.9 instead of 1.8
  } else if (s.startsWith("?status")) {
    reply ("OK...");
  } else if (s.startsWith("!rm ")) { // Range measure mode, not neccessary
    delay(5);
    
    // Setting speed command from Micromanager
  } else if (s.startsWith("!speed ")) { //we do not really need this, but we do  need this for Micromanager response
    String vx_ = s.substring(s.indexOf("!speed ") + 1);
    String vy_ = s.substring(s.indexOf("!pos ") + 3);
    float vx = vx_.toFloat();
    float vy = vy_.toFloat();
    if (Mode_xy == 2) { //data is in rev/s of the output
      //need to change to step/s
      vx = vx * Reduction * 360 / 1.8;
      vy = vy * Reduction * 360 / 1.8;
    }

    //error messages
  } else if (s.startsWith("a")) {  //stop everything
    stepper_x.stop();
    stepper_y.stop();
  } else if (s.startsWith("?err")) { //respond to error message inquiry
    reply ("0");
  }
}

void reply(String s) { // Function used to send responses to Micromanager
  Serial.print(s);
  Serial.print("\r");
}

void calibrateStage() {   //Function used to calibrate system to the limit switches
  // Ask motors to move all the way down
  stepper_x.moveTo(-100000);
  stepper_y.moveTo(-100000);
  while (!digitalRead(x_LimPin) && !digitalRead(y_LimPin)) { // let motors keep moving until limit switches are touched
    if (!digitalRead(x_LimPin)) {
      stepper_x.run();
    }
    if (!digitalRead(y_LimPin)) {
      stepper_y.run();
    }
  }
  // Set origin
  stepper_x.setCurrentPosition(0.0);
  stepper_y.setCurrentPosition(0.0);
  Enc_x.write(0.0);
  Enc_y.write(0.0);
}
