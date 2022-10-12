// An arduino firmware that can be used with the Micro-Manager LStep Z-Stage device adaptor.
// Allows building simple 1-axis stage devices.
// version 0.1
// JM 2016-2018

// References
// https://micro-manager.org/wiki/MarzhauserLStep
// MM device adapter code: https://valelab4.ucsf.edu/svn/micromanager2/trunk/DeviceAdapters/Marzhauser-LStep/LStep.cpp
// LStep documentation from www.marzhauser.com/nc/en/service/downloads.html

// in HCW, use serial port settings:
// AnswerTimeout,3000.0000
// BaudRate,9600
// DelayBetweenCharsMs,0.0000
// Handshaking,Off
// Parity,None
// StopBits,1

#include <AccelStepper.h>
 
// 定义电机控制用常量
const int enablePin = 8;  // 使能控制引脚
 
const int xdirPin = 5;     // x方向控制引脚
const int xstepPin = 2;    // x步进控制引脚
const int ydirPin = 6;     // y方向控制引脚
const int ystepPin = 3;    // y步进控制引脚

const int R_LED_Pin = 9;
const int G_LED_Pin = 10;
bool state_x = 0;
bool state_y = 0;

int Maxspeed = 30000000000.0;        //step/s
int Acceleration = 20.0;     //step/s^2
const int Reduction = 2;     //Reduction ratio for actuators
const int BaseRes = 1.8;        //Base resolution of actuator in deg
const int Pitch = 2000.0;       //Lead screw pitch in microns
const int stepSize = Pitch/Reduction/360*BaseRes; // step size of motor in microns
AccelStepper stepper1(1,xstepPin,xdirPin);//建立步进电机对象1
AccelStepper stepper2(1,ystepPin,ydirPin);//建立步进电机对象2

int Mode_x = 2; // Helps with controlling units for x motor
int Mode_y = 2; // Helps with controlling units for y motor
int Mode_xy = 1; // Helps with controlling units for xy stage

String xyPitch = "2000.0"; // in microns
String cmd = "" ;
float x = 0.0;
float y = 0.0;

float x_old = x;
float y_old = y;


void setup() {
  Serial.begin( 9600 );
  pinMode(xstepPin,OUTPUT);     // Arduino控制A4988x步进引脚为输出模式
  pinMode(xdirPin,OUTPUT);      // Arduino控制A4988x方向引脚为输出模式
  pinMode(ystepPin,OUTPUT);     // Arduino控制A4988y步进引脚为输出模式
  pinMode(ydirPin,OUTPUT);      // Arduino控制A4988y方向引脚为输出模式
  pinMode(R_LED_Pin,OUTPUT);
  pinMode(G_LED_Pin,OUTPUT);
  digitalWrite(R_LED_Pin,LOW);
  digitalWrite(G_LED_Pin,LOW);
  pinMode(enablePin,OUTPUT);   // Arduino控制A4988使能引脚为输出模式
  digitalWrite(enablePin,LOW); // 将使能控制引脚设置为低电平从而让
                               // 电机驱动板进入工作状态
                                
  stepper1.setMaxSpeed(Maxspeed);     // 设置电机最大速度300 
  stepper1.setAcceleration(Acceleration);  // 设置电机加速度20.0  
  stepper2.setMaxSpeed(Maxspeed);     // 设置电机最大速度300 
  stepper2.setAcceleration(Acceleration);  // 设置电机加速度20.0  
  reply ("Vers:LS");
}
char c = '*';

void loop()
{
  if (Serial.available()) {
    cmd = Serial.readStringUntil('\r');
    processCommand(cmd);
    cmd = "";
    
  }
}

void processCommand(String s) {
  // Initial Settings
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
    if (Mode_x == 2){
      reply (String(Maxspeed*stepSize/1000)); // mm/s of the x-motor
      }
  } else if (s.startsWith("?vel y")) {
    if (Mode_y == 2){
      reply (String(Maxspeed*stepSize/1000)); // mm/s of the y-motor
      }
  } else if (s.startsWith("?accel x")) {
    reply(String(Acceleration/Reduction*stepSize/1000000)); //converted acceleration to m/s^2
 } else if (s.startsWith("?accel y")) {
    reply(String(Acceleration*stepSize/1000000)); //converted acceleration to m/s^2
    
  
    //Setting Acceleration Values
  } else if (s.startsWith("!accel x")) {
    String xa = s.substring(s.indexOf("!accel x ") + 1);
    Acceleration = xa.toFloat()*Reduction/stepSize*1000000;//converts m/s^2 to step/s^2
    stepper1.setAcceleration(Acceleration); 
  } else if (s.startsWith("!accel y")) {
    String ya = s.substring(s.indexOf("!accel y ") + 1);
    Acceleration = ya.toFloat()*Reduction/stepSize*1000000; //converts m/s^2 to step/s^2
    stepper2.setAcceleration(Acceleration); 

  } else if (s.startsWith("!vel y")) {
    String yv = s.substring(s.indexOf("!vel x ") + 1);
    float yv_num = yv.toFloat();
    if (Mode_y == 2){ // mm/sec of output
      float vy = yv_num*Reduction/stepSize;
      }
  } else if (s.startsWith("!vel x")) {
    String xv = s.substring(s.indexOf("!vel x ") + 1);
    float xv_num = xv.toFloat();
    if (Mode_x == 2){ // mm/sec of output
      float vx = xv_num*Reduction/stepSize;
      }
      
  //Tells micromanager if stage is moving or not  
  } else if (s.startsWith("?statusaxis")) { //should return something if 
    if (1==1) {
      reply ("@@@");  // used to be "just @ for z axis", should be returning MM if the motor is still moving
    }
    else{
      reply ("MMM");
      }
 
  //Tells micromanager current stage position  
  } else if (s.startsWith("?pos")) { 
    if (Mode_xy == 1) { // Info should be in microns
      String xPos = String(x);
      String xy_pos = xPos + " " + String(y);
      reply (xy_pos);
      }
    else if (Mode_xy == 0){ // Asking for current step
      String xPos = String(int(x/stepSize), 1);
      String xy_pos = xPos + " " + String(int(y/stepSize), 1);
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
    String delta_x = s.substring(s.indexOf("!mor ") + 5);
    Serial.println(delta_x+" ");
    String delta_y = s.substring(s.indexOf("!mor ") + 6);
    Serial.println(delta_y+" ");
    float delta_x_num = delta_x.toFloat();
    float delta_y_num = delta_y.toFloat();
      if (Mode_xy == 0){ 
      delta_x_num = delta_x_num*stepSize;
      delta_y_num = delta_y_num*stepSize;
      }
    x = x + delta_x_num;
    y = y + delta_y_num;
    turnServoXY();

  // Absolute Motion Control  
  } else if (s.startsWith("!moa ")) {   //relative motion, assume in um
    String apos_x = s.substring(s.indexOf("!moa ") + 5);
    String apos_y = s.substring(s.indexOf("!moa ") + 6);
    float apos_x_num = apos_x.toFloat();
    float apos_y_num = apos_y.toFloat();
    if (Mode_xy == 0){ 
      apos_x_num = apos_x_num*stepSize;
      apos_y_num = apos_y_num*stepSize;
      }
    x = apos_x_num;
    y = apos_y_num;   
    turnServoXY();

    // Setting boundaries for x and y in microns
  } else if (s.startsWith("?lim x")) {
    if (Mode_xy==1){
      reply ("0.0 100.0"); // gonna need to check this with Infrastructure
      }
    else if (Mode_xy==0){
      reply ("0.0 100.0"); // gonna need to check this with Infrastructure
      }
  } else if (s.startsWith("?lim y")) {
    if (Mode_xy==1){
      reply ("0.0 100.0"); // gonna need to check this with Infrastructure
      }
    else if (Mode_xy==0){
      reply ("0.0 100.0"); // gonna need to check this with Infrastructure
      }
 
    // Origins and Callibrations
  } else if (s.startsWith("!cal x")) {  //scalibrate x origin
    x = 0.0;
    turnServoXY();
  } else if (s.startsWith("!cal y")) {  //scalibrate y origin
    y = 0.0;
    turnServoXY();
  } else if (s.startsWith("!pos 0 0")) {  //setting origin
    x = 0.0;
    y = 0.0;
    stepper1.setCurrentPosition(0.0);
    stepper2.setCurrentPosition(0.0);
  } else if (s.startsWith("!pos ")) {   //setting arbitrary position
    String x_set = s.substring(s.indexOf("!pos ") + 1);
    String y_set = s.substring(s.indexOf("!pos ") + 3);
    x = x_set.toFloat();
    y = y_set.toFloat();
    stepper1.setCurrentPosition(x);
    stepper2.setCurrentPosition(y);
    
  } else if (s.startsWith("?status")) {
    reply ("OK...");
  } else if (s.startsWith("!rm ")) { // Range measure mode, not neccessary
    delay(5);
    
  } else if (s.startsWith("!speed ")) { //not sure if we need this
    String vx_ = s.substring(s.indexOf("!speed ") + 1);
    String vy_ = s.substring(s.indexOf("!pos ") + 3);
    float vx = vx_.toFloat(); 
    float vy = vy_.toFloat();
    if (Mode_xy==2){ //data is in rev/s of the output
      //need to change to step/s
      vx = vx*Reduction*360/1.8; 
      vy = vy*Reduction*360/1.8;
      }


    //error messages
  } else if (s.startsWith("a")) {  //stop everything
    stepper1.stop();
    stepper2.stop();    
  } else if (s.startsWith("?err")) { //respond to error message inquiry
    reply ("0");
  }
}

void reply(String s) {
  Serial.print(s);
  Serial.print("\r");
}

void turnServoXY() {
    stepper1.moveTo(x*10000);
    stepper2.moveTo(y*10000);
    stepper1.runToPosition();
    stepper2.runToPosition();
  }

/*
  // "?ver"   ->   Vers:LS
  // "?det"   ->   11F31  seul le 2nd least significant byte (le 3) est important = 3 axes
  // "?statusaxis" -> @ si dispo et M si busy
  // "!autostatus 0"
  // "?pitch z" -> 50.0
  // "!dim z 2" mettre en microns
  // "!dim z 1"
  // "?vel z"  -> 100.0
  // "!vel z 100"
  // "?accel z" -> 1.0
  // "!accel z 1"
  "a" -> abort ??
  "!moa z 10"  move z to absolute 10
  // "?err"  retourner 0
  // "!mor z " move z by relative 10
  // "?pos z" query current z pos -> return z pos
  // "?lim z" query limits, -> 0.0 100.0
  // "!pos z 0" set origin
  // "!pos z 20 fixe cette origine pour le controlle
  // "?status"  -> OK...
  "!dim z 0" set as steps.
  "MOA Z " move to abs step pos
  "?pos z" get position as steps
  //"!speed z 10" lance le mouvement en mm/sec
*/
