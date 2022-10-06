

// An arduino firmware that can be used with the Micro-Manager LStep Z-Stage device adaptor.
// Allows building simple 1-axis stage devices.
// version 0.1
// JM 2016-2018

// References
// https://micro-manager.org/wiki/MarzhauserLStep
// MM device adaptor code /DeviceAdapters/Marzhauser-LStep/LStep.cpp
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
    reply ("50.0");
  } else if (s.startsWith("?vel x")) {
    reply ("100.0");
  } else if (s.startsWith("?accel x")) {
    reply ("1.0");
  } else if (s.startsWith("!dim x 1")) {
    delay(5);
  } else if (s.startsWith("!dim x 2")) {
    delay(5);
  } else if (s.startsWith("?statusaxis")) {
    reply ("@");
  } else if (s.startsWith("!vel x")) {
    delay(5);
  } else if (s.startsWith("!accel x")) {
    delay(5);
    


     } else if (s.startsWith("?pitch y")) {
    reply ("50.0");
  } else if (s.startsWith("?vel y")) {
    reply ("100.0");
  } else if (s.startsWith("?accel y")) {
    reply ("1.0");
  } else if (s.startsWith("!dim y 1")) {
    delay(5);
  } else if (s.startsWith("!dim y 2")) {
    delay(5);
  } else if (s.startsWith("?statusaxis")) {
    reply ("@");
  } else if (s.startsWith("!vel y")) {
    delay(5);
  } else if (s.startsWith("!accel y")) {
    delay(5);

    // Defining stage information
  } else if (s.startsWith("?pos x")) {
    String xs = String(x, 1);
    reply (xs);
  } else if (s.startsWith("?lim x")) {
    reply ("0.0 100.0");
  } else if (s.startsWith("!pos x")) {
    delay(5);
  } else if (s.startsWith("?status")) {
    reply ("OK...");
  } else if (s.startsWith("!dim x 0")) {
    delay(5);


  } else if (s.startsWith("?pos y")) {
    String ys = String(y, 1);
    reply (ys);
  } else if (s.startsWith("?lim y")) {
    reply ("0.0 100.0");
  } else if (s.startsWith("!pos y")) {
    delay(5);
  } else if (s.startsWith("?status")) {
    reply ("OK...");
  } else if (s.startsWith("!dim y 0")) {
    delay(5);


    
    
    // In the x axis
  } else if (s.startsWith("!speed x")) {
    delay(5);
  } else if (s.startsWith("!mor x")) {
    String delta_x = s.substring(s.indexOf("x") + 1);
    x = x + delta_x.toFloat();
    turnServoX();
  } else if (s.startsWith("!moa x")) {
    String apos_x = s.substring(s.indexOf("x") + 1);
    x = apos_x.toFloat();
    turnServoX();

    // In the y axis
  } else if (s.startsWith("!mor y")) {
    String delta_y = s.substring(s.indexOf("y") + 1);
    y = y + delta_y.toFloat();
    turnServoY();
  } else if (s.startsWith("!moa y")) {
    String apos = s.substring(s.indexOf("y") + 1);
    y = apos.toFloat();
    turnServoY();

    //error message
  } else if (s.startsWith("?err")) {
    reply ("0");
  }
}

void reply(String s) {
  Serial.print(s);
  Serial.print("\r");
}

void turnServoX() {
    stepper1.moveTo(x*1000);
    stepper1.run();
  }

void turnServoY() {
    stepper2.moveTo(y*1000);
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
