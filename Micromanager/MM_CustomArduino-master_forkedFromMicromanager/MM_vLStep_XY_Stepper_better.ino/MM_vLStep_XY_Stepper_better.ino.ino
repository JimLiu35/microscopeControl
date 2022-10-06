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

int Maxspeed = 300.0;
int Acceleration = 20.0;
const int Reduction = 1000;     //Reduction ratio for actuators
AccelStepper stepper1(1,xstepPin,xdirPin);//建立步进电机对象1
AccelStepper stepper2(1,xstepPin,xdirPin);//建立步进电机对象2

int Mode = 0; 

String cmd = "" ;
float x = 0.0;
float y = 0.0;

void setup() {
  Serial.begin( 9600 );
  pinMode(xstepPin,OUTPUT);     // Arduino控制A4988x步进引脚为输出模式
  pinMode(xdirPin,OUTPUT);      // Arduino控制A4988x方向引脚为输出模式
  pinMode(ystepPin,OUTPUT);     // Arduino控制A4988y步进引脚为输出模式
  pinMode(ydirPin,OUTPUT);      // Arduino控制A4988y方向引脚为输出模式
  
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
    reply ("50.0"); // Maybe this is the pitch of the x-motor in microns?
  } else if (s.startsWith("?pitch y")) {
    reply ("50.0"); // Maybe this is the pitch of the y-motor in microns?

  } else if (s.startsWith("?vel x")) {
    reply ("100.0"); // Maybe this is the rev/s of the x-motor?
  } else if (s.startsWith("?vel y")) {
    reply ("100.0"); // Maybe this is the rev/s of the y-motor?

  } else if (s.startsWith("?accel x")) {
    reply ("1.0"); // Maybe this is the m/s^2 of the x-motor?
 } else if (s.startsWith("?accel y")) {
    reply ("1.0"); // Maybe this is the m/s^2 of the y-motor?

  } else if (s.startsWith("!dim x 2")) {
    delay(5);
  } else if (s.startsWith("!vel x")) {
    delay(5);
  } else if (s.startsWith("!accel x")) {
    delay(5);
  } else if (s.startsWith("!dim y 2")) {
    delay(5);
  } else if (s.startsWith("!vel y")) {
    delay(5);
  } else if (s.startsWith("!accel y")) {
    delay(5);
    
  //Tells micromanager if stage is moving or not  
  } else if (s.startsWith("?statusaxis")) { //should return something if 
    reply ("@@@");  // used to be "just @ for z axis", should be returning MM if the motor is still moving
  
  //Tells micromanager current stage position  
  } else if (s.startsWith("?pos")) {
    String xPos = String(x, 1);
    String xy_pos = xPos + " " + String(y, 1);
    reply (xy_pos);

  // Relative Motion Control  
  } else if (s.startsWith("!dim 2 2")) { // switch to rev/s 
    delay(5);
  } else if (s.startsWith("!dim 1 1")) { // switch to um 
    delay(5);
  } else if (s.startsWith("!dim 0 0")) { // switch to steps
    delay(5);
  } else if (s.startsWith("!mor ")) {   //relative motion
    String delta_x = s.substring(s.indexOf("!mor ") + 1);
    String delta_y = s.substring(s.indexOf("!mor ") + 3);
    x = x + delta_x.toFloat();
    y = y + delta_y.toFloat();
    turnServoXY();

  // Absolute Motion Control  
  } else if (s.startsWith("!moa ")) {   //relative motion
    String apos_x = s.substring(s.indexOf("!moa ") + 1);
    String apos_y = s.substring(s.indexOf("!moa ") + 3);
    x = x + apos_x.toFloat();
    y = y + apos_y.toFloat();
    turnServoXY();


    // Setting boundaries for x and y in microns
  } else if (s.startsWith("?lim x")) {
    reply ("0.0 100.0"); // gonna need to check this with Infrastructure
  } else if (s.startsWith("?lim y")) {
    reply ("0.0 100.0"); // gonna need to check this with Infrastructure
  } else if (s.startsWith("!pos 0 0")) {  //setting origin
    delay(5);
  } else if (s.startsWith("!pos ")) {   //setting arbitrary position
    String x_set = s.substring(s.indexOf("!pos ") + 1);
    String y_set = s.substring(s.indexOf("!pos ") + 3);
    x = x_set.toFloat();
    y = y_set.toFloat();
    
  } else if (s.startsWith("?status")) {
    reply ("OK...");
  } else if (s.startsWith("!dim 0 0")) {
    delay(5);

    


  } else if (s.startsWith("!speed ")) {
    String vx_ = s.substring(s.indexOf("!speed ") + 1);
    String vy_ = s.substring(s.indexOf("!pos ") + 3);
    int vx = vx_.toFloat(); // speed set as rev/s
    int vy = vy_.toFloat(); // speed set as rev/s


    //error message
  } else if (s.startsWith("?err")) {
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
    stepper1.run();
    stepper2.run();
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
