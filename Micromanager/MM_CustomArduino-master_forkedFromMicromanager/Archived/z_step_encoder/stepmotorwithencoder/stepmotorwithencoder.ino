
#include <AccelStepper.h>

#include <Encoder.h>

// 定义电机控制用常量
const int enablePin = 8;  // 使能控制引脚

const int zdirPin = 5;     // z方向控制引脚 (用的是cnc shield上的x)
const int zstepPin = 2;    // z步进控制引脚
Encoder myEnc(18, 19); // set encoder pin
//   avoid using pins with LEDs attached
long newPosition = 0;
AccelStepper stepper1(1, zstepPin, zdirPin); //建立步进电机对象1
String cmd = "" ;
float z = 0.0;

void setup() {
  Serial.begin( 9600 );
  pinMode(zstepPin, OUTPUT);    // Arduino控制A4988x步进引脚为输出模式
  pinMode(zdirPin, OUTPUT);     // Arduino控制A4988x方向引脚为输出模式

  pinMode(enablePin, OUTPUT);  // Arduino控制A4988使能引脚为输出模式
  digitalWrite(enablePin, LOW); // 将使能控制引脚设置为低电平从而让
  // 电机驱动板进入工作状态

  stepper1.setMaxSpeed(300.0);     // 设置电机最大速度300
  stepper1.setAcceleration(20.0);  // 设置电机加速度20.0
  reply ("Vers:LS");
  Serial.println("Basic Encoder Test:");
}
char c = '*';

long oldPosition  = -999;
void loop()
{
  if (Serial.available()) {
    cmd = Serial.readStringUntil('\r');
    processCommand(cmd);
    cmd = "";
  }
  //  stepper1.run();


  //  long newPosition = myEnc.read();
  //  if (newPosition != oldPosition) {
  //    oldPosition = newPosition;
  //    Serial.print("newPosition: ");
  //    Serial.println(newPosition/3);
  //    Serial.print("z: ");
  //    Serial.println (z);
  //    Serial.print("adj: ");
  //    Serial.println (z - newPosition / 3);
  //  }
  if (z - newPosition / 3 > 20 ||  newPosition / 3 - z > 20 ) {
    stepper1.move(z - newPosition / 3 );

  }
  else {
    stepper1.stop();
    Serial.println("trying stop");
  }
  //    if ( newPosition / 3 - z > 10) {
  //      stepper1.move(z - newPosition / 3 );
  //    }
  stepper1.run();
}

void processCommand(String s) {
  if (s.startsWith("?ver")) {
    reply ("Vers:LS");
  } else if (s.startsWith("!autostatus 0")) {
    delay(5);
  } else if (s.startsWith("?det")) {
    reply ("60");
  } else if (s.startsWith("?pitch z")) {
    reply ("50.0");
  } else if (s.startsWith("?vel z")) {
    reply ("100.0");
  } else if (s.startsWith("?accel z")) {
    reply ("1.0");
  } else if (s.startsWith("!dim z 1")) {
    delay(5);
  } else if (s.startsWith("!dim z 2")) {
    delay(5);
  } else if (s.startsWith("?statusaxis")) {
    reply ("@");
  } else if (s.startsWith("!vel z")) {
    delay(5);
  } else if (s.startsWith("!accel z")) {
    delay(5);
  } else if (s.startsWith("?pos z")) {
    String zs = String(z, 1);
    reply (zs);
  } else if (s.startsWith("?lim z")) {
    reply ("0.0 100.0");
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
//
//void turnServo() {
//  if (z != newPosition) {
//  stepper1.moveTo((z-newPosition) * 10);
//  }
//  Serial.println(newPosition);
//}


void turnServo() {
  stepper1.moveTo(z );

  //  if (z != newPosition/3) {
  //    stepper1.moveTo(z - newPosition);
  //  }
}
