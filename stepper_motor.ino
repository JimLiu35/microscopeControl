
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
 
const int zdirPin = 5;     // z方向控制引脚 (用的是cnc shield上的x)
const int zstepPin = 2;    // z步进控制引脚
const int ydirPin = 6;     // y方向控制引脚
const int ystepPin = 3;    // y步进控制引脚
const int xdirPin = 5;     // x方向控制引脚
const int xstepPin = 2;    // x步进控制引脚
 
// AccelStepper stepper1(1,zstepPin,zdirPin);//建立步进电机对象1
AccelStepper stepper2(1,ystepPin,ydirPin);//建立步进电机对象2
AccelStepper stepper3(1,xstepPin,xdirPin);//建立步进电机对象3

String cmd = "" ;
float z = 0.0;
float x = 0.0;
float y = 0.0;

void setup() {
  Serial.begin( 9600 );
  pinMode(xstepPin,OUTPUT);     // Arduino控制A4988x步进引脚为输出模式
  pinMode(xdirPin,OUTPUT);      // Arduino控制A4988x方向引脚为输出模式
  pinMode(ystepPin,OUTPUT);     // Arduino控制A4988y步进引脚为输出模式
  pinMode(ydirPin,OUTPUT);      // Arduino控制A4988y方向引脚为输出模式
  // pinMode(zstepPin,OUTPUT);     // Arduino控制A4988z步进引脚为输出模式
  // pinMode(zdirPin,OUTPUT);      // Arduino控制A4988z方向引脚为输出模式
  
  pinMode(enablePin,OUTPUT);   // Arduino控制A4988使能引脚为输出模式
  digitalWrite(enablePin,LOW); // 将使能控制引脚设置为低电平从而让
                               // 电机驱动板进入工作状态
                                
  // stepper1.setMaxSpeed(300.0);     // 设置电机最大速度300 
  // stepper1.setAcceleration(20.0);  // 设置电机加速度20.0  
  stepper2.setMaxSpeed(300.0);     // 设置电机最大速度300 
  stepper2.setAcceleration(20.0);  // 设置电机加速度20.0 
  stepper3.setMaxSpeed(300.0);     // 设置电机最大速度300 
  stepper3.setAcceleration(20.0);  // 设置电机加速度20.0 
  reply ("Vers:LS");
}
char c = '*';

void loop()
{
  if (Serial.available()) {
    cmd = Serial.readStringUntil('\r');
    processCommand_y(cmd);
    processCommand_x(cmd);
    cmd = "";
  }
}

void processCommand_x(String s) {
  if (s.startsWith("?ver")) {
    reply ("Vers:LS");
  } else if (s.startsWith("!autostatus 0")) {
    delay(5);
  } else if (s.startsWith("?det")) {
    reply ("60");
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
  } else if (s.startsWith("!speed x")) {
    delay(5);
  } else if (s.startsWith("!mor x")) {
    String delta = s.substring(s.indexOf("x") + 1);
    x = x + delta.toFloat();
    turnStepper3();
  } else if (s.startsWith("!moa x")) {
    String apos = s.substring(s.indexOf("x") + 1);
    x = apos.toFloat();
    turnStepper3();
  } else if (s.startsWith("?err")) {
    reply ("0");
  }
}

void processCommand_y(String s) {
  if (s.startsWith("?ver")) {
    reply ("Vers:LS");
  } else if (s.startsWith("!autostatus 0")) {
    delay(5);
  } else if (s.startsWith("?det")) {
    reply ("60");
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
  } else if (s.startsWith("!speed y")) {
    delay(5);
  } else if (s.startsWith("!mor y")) {
    String delta = s.substring(s.indexOf("y") + 1);
    y = y + delta.toFloat();
    turnStepper2();
  } else if (s.startsWith("!moa y")) {
    String apos = s.substring(s.indexOf("y") + 1);
    y = apos.toFloat();
    turnStepper2();
  } else if (s.startsWith("?err")) {
    reply ("0");
  }
}

// void processCommand(String s) {
//   if (s.startsWith("?ver")) {
//     reply ("Vers:LS");
//   } else if (s.startsWith("!autostatus 0")) {
//     delay(5);
//   } else if (s.startsWith("?det")) {
//     reply ("60");
//   } else if (s.startsWith("?pitch z")) {
//     reply ("50.0");
//   } else if (s.startsWith("?vel z")) {
//     reply ("100.0");
//   } else if (s.startsWith("?accel z")) {
//     reply ("1.0");
//   } else if (s.startsWith("!dim z 1")) {
//     delay(5);
//   } else if (s.startsWith("!dim z 2")) {
//     delay(5);
//   } else if (s.startsWith("?statusaxis")) {
//     reply ("@");
//   } else if (s.startsWith("!vel z")) {
//     delay(5);
//   } else if (s.startsWith("!accel z")) {
//     delay(5);
//   } else if (s.startsWith("?pos z")) {
//     String zs = String(z, 1);
//     reply (zs);
//   } else if (s.startsWith("?lim z")) {
//     reply ("0.0 100.0");
//   } else if (s.startsWith("!pos z")) {
//     delay(5);
//   } else if (s.startsWith("?status")) {
//     reply ("OK...");
//   } else if (s.startsWith("!dim z 0")) {
//     delay(5);
//   } else if (s.startsWith("!speed z")) {
//     delay(5);
//   } else if (s.startsWith("!mor z")) {
//     String delta = s.substring(s.indexOf("z") + 1);
//     z = z + delta.toFloat();
//     turnServo();
//   } else if (s.startsWith("!moa z")) {
//     String apos = s.substring(s.indexOf("z") + 1);
//     z = apos.toFloat();
//     turnServo();
//   } else if (s.startsWith("?err")) {
//     reply ("0");
//   }
// }

void reply(String s) {
  Serial.print(s);
  Serial.print("\r");
}

// void turnServo() {
//     stepper1.moveTo(z*1000);
//     stepper1.run();
//   }

void turnStepper3() {
    stepper3.moveTo(x*100);
    stepper3.run();
  }

void turnStepper2() {
    stepper2.moveTo(y*100);
    stepper2.run();
  }