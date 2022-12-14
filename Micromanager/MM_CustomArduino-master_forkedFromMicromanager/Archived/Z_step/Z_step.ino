// An arduino firmware that can be used with the Micro-Manager LStep Z-Stage device adaptor.
// Allows building simple 1-axis stage devices.
// version 3.0(final)

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
#include <Encoder.h>
// 定义电机控制用常量
const int enablePin = 8;  // 使能控制引脚

const int zdirPin = 5;     // z方向控制引脚 (用的是cnc shield上的x)
const int zstepPin = 2;    // z步进控制引脚
const int zdirPin2 = 6;     // z2方向控制引脚 (用的是cnc shield上的y)
const int zstepPin2 = 3;    // z2步进控制引脚

const float Maxspeed = 30000.0;        //step/s
const float Acceleration = 1000;     //step/s^2
const float Reduction = 10;     //Reduction ratio for actuators
const float BaseRes = 1.8;        //Base resolution of actuator in deg
const float Pitch = 2000.0;       //Lead screw pitch in microns
const float stepSize = Pitch / Reduction / 360 * BaseRes; // step size of motor in microns
const int limitswitchPin = 14;   // setup the pin

AccelStepper stepper1(1, zstepPin, zdirPin); //建立步进电机对象1
AccelStepper stepper2(1, zstepPin2, zdirPin2); //建立步进电机对象2
Encoder myEnc2(18, 19);
Encoder myEnc(20, 21);

long newPosition = 0.0;
long oldPosition  = -999.0;
long newPosition2 = 0.0;
long oldPosition2  = -999.0;

String cmd = "" ;
float z = 0.0;

void setup() {
  Serial.begin( 115200 );       //Use for 115200 Baud Rate
  //Serial.begin( 9600 );         //Use for 115200 Baud Rate
  pinMode(zstepPin, OUTPUT);    // Arduino控制A4988x步进引脚为输出模式
  pinMode(zdirPin, OUTPUT);     // Arduino控制A4988x方向引脚为输出模式
  pinMode(zstepPin2, OUTPUT);    // Arduino控制A4988x步进引脚为输出模式
  pinMode(zdirPin2, OUTPUT);     // Arduino控制A4988x方向引脚为输出模式

  pinMode(enablePin, OUTPUT);  // Arduino控制A4988使能引脚为输出模式
  digitalWrite(enablePin, LOW); // 将使能控制引脚设置为低电平从而让
  // 电机驱动板进入工作状态

  stepper1.setMaxSpeed(Maxspeed);     // step/s
  stepper1.setAcceleration(Acceleration);  //step/s^2
  stepper2.setMaxSpeed(Maxspeed);     // step/s
  stepper2.setAcceleration(Acceleration);  //step/s^2
  reply ("Vers:LS_forZAxis");
  pinMode(limitswitchPin, INPUT_PULLUP); //limitswitch
}
char c = '*';

void loop()
{
  if (digitalRead(limitswitchPin) == HIGH) {
    if (Serial.available()) {
      cmd = Serial.readStringUntil('\r');
      processCommand(cmd);
      cmd = "";
    }
    //    newPosition = myEnc.read();
    //    newPosition2 = myEnc2.read();
    //
    //
    //    if (newPosition != oldPosition) {
    //      oldPosition = newPosition;
    //      //    Serial.println(newPosition);
    //    }
    //
    //    if (newPosition2 != oldPosition2) {
    //      oldPosition2 = newPosition2;
    //      //    Serial.println(newPosition2/3*2*stepSize_y);
    //    }
    //
    //    if (z - newPosition / 3 > 20 ||  newPosition / 3 - z > 20 ) {
    //      stepper1.move(z - newPosition / 3 );
    //      stepper2.move(z - newPosition / 3 );
    //
    //    }

    turnServo();
    stepper1.setMaxSpeed(Maxspeed);     // step/s
    stepper1.setAcceleration(Acceleration);  //step/s^2
    stepper2.setMaxSpeed(Maxspeed);     // step/s
    stepper2.setAcceleration(Acceleration);  //step/s^2
    stepper1.run();
    stepper2.run();
//    Serial.println(stepper1.speed());
  }

  else {
    delay(200);
    //stepper1.stop();
    stepper1.setSpeed(0);
    stepper1.setAcceleration(0);
    //stepper2.stop();
    stepper2.setSpeed(0);
    stepper2.setAcceleration(0);
//    Serial.println("STOP");
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
    reply ("2000.0");
  } else if (s.startsWith("?vel z")) {
    reply ("100.0");
  } else if (s.startsWith("?accel z")) {
    reply ("1.0");
  } else if (s.startsWith("!dim z 1")) {
    delay(5);
  } else if (s.startsWith("!dim z 2")) {
    delay(5);
  } else if (s.startsWith("?statusaxis")) {
    if (stepper1.speed()!=0||stepper2.speed()!=0){reply ("M");}
    else {reply("@");}
  } else if (s.startsWith("!vel z")) {
    delay(5);
  } else if (s.startsWith("!accel z")) {
    delay(5);
  } else if (s.startsWith("?pos z")) {
    String zs = String(z, 1);
    reply (zs);
  } else if (s.startsWith("?lim z")) {
    reply ("-100000000000000000.0 1000000000000000000.0");
  } else if (s.startsWith("!pos z")) {
    delay(5);
  } else if (s.startsWith("?status")) {
    reply ("OK...");
  } else if (s.startsWith("!dim z 0")) {
    delay(5);
  } else if (s.startsWith("!speed z")) {
    delay(5);
  } else if (s.startsWith("!mor z")) {
    String delta = s.substring(s.indexOf("z") + 1);
    z = z + delta.toFloat();
    turnServo();
  } else if (s.startsWith("!moa z")) {
    String apos = s.substring(s.indexOf("z") + 1);
    z = apos.toFloat();
    turnServo();
  } else if (s.startsWith("?err")) {
    reply ("0");
  }
}

void reply(String s) {
 Serial.print(s);
 Serial.print("\r");

}

void turnServo() {
  stepper1.moveTo(z / stepSize);
  stepper2.moveTo(z / stepSize);
//  Serial.println(z);
}
