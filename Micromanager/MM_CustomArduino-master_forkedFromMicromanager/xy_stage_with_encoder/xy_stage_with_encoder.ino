
#include <AccelStepper.h>
#include <Encoder.h>
// 定义电机控制用常量
const int enablePin = 8;  // 使能控制引脚
const int xdirPin = 5;     // x方向控制引脚
const int xstepPin = 2;    // x步进控制引脚
const int ydirPin = 6;     // y方向控制引脚
const int ystepPin = 3;    // y步进控制引脚
const int limitswitchPin = 14;   // setup the pin

bool state_x = 0;
bool state_y = 0;

const float tolerance = 1.0;
const int x_LimPin = 11;    // x limit switch pinout
const int y_LimPin = 12;    // y limit switch pinout

const float Maxspeed = 5000.0;        //step/s
const float Acceleration = 1000.0;     //step/s^2
const float Reduction = 1.0;     //Reduction ratio for actuators
const float BaseRes_y = 0.9;        //Base resolution of actuator in deg
const float BaseRes_x = 1.8;        //Base resolution of actuator in deg
const float Pitch = 2000.0;       //Lead screw pitch in microns
const float stepSize_x = Pitch / Reduction / 360.0 * BaseRes_x; // step size of motor in microns
const float stepSize_y = Pitch / Reduction / 360.0 * BaseRes_y; // step size of motor in microns

AccelStepper stepper1(1, xstepPin, xdirPin); //建立步进电机对象1
AccelStepper stepper2(1, ystepPin, ydirPin); //建立步进电机对象2
int firstSpace;         //Used to parse commands for x
int secondSpace;        //Used to parse commands for y
int Mode_x = 2; // Helps with controlling units for x motor
int Mode_y = 2; // Helps with controlling units for y motor
int Mode_xy = 1; // Helps with controlling units for xy stage

String xyPitch = "2000.0"; // in microns
String cmd = "" ;
float x = 0.0;
float y = 0.0;

float x_old = x;
float y_old = y;

Encoder Enc_y(18, 19);
Encoder Enc_x(20, 21);
//   avoid using pins with LEDs attached
long newPosition = 0.0;
long oldPosition  = -999.0;
long newPosition2 = 0.0;
long oldPosition2  = -999.0;



void setup() {
  //Serial.begin(115200);
  Serial.begin(9600);
  pinMode(xstepPin, OUTPUT);    // Arduino控制A4988x步进引脚为输出模式
  pinMode(xdirPin, OUTPUT);     // Arduino控制A4988x方向引脚为输出模式
  pinMode(ystepPin, OUTPUT);    // Arduino控制A4988y步进引脚为输出模式
  pinMode(ydirPin, OUTPUT);     // Arduino控制A4988y方向引脚为输出模式
  pinMode(enablePin, OUTPUT);  // Arduino控制A4988使能引脚为输出模式
  digitalWrite(enablePin, LOW); // 将使能控制引脚设置为低电平从而让
  pinMode(limitswitchPin, INPUT_PULLUP); //limitswitch
  // 电机驱动板进入工作状态

  stepper1.setMaxSpeed(Maxspeed);     // 设置电机最大速度300
  stepper1.setAcceleration(Acceleration);  // 设置电机加速度20.0
  stepper2.setMaxSpeed(Maxspeed);     // 设置电机最大速度300
  stepper2.setAcceleration(Acceleration);  // 设置电机加速度20.0
  reply ("Vers:LS_xy");
  //  calibrateStage();
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
    newPosition = Enc_x.read();
    newPosition2 = Enc_y.read();

    if (newPosition != oldPosition) {
      oldPosition = newPosition;
      //    Serial.println(newPosition);
    }

    if (newPosition2 != oldPosition2) {
      oldPosition2 = newPosition2;
      //    Serial.println(newPosition2/3*2*stepSize_y);
    }

    if (x / stepSize_x - Enc_x.read() / 3.0 / 4.0 > tolerance ||  Enc_x.read() / 3.0 / 4.0 - x / stepSize_x > tolerance ) {
      stepper1.move(x / stepSize_x - newPosition / 3.0 / 4.0 );
      stepper1.setAcceleration(Acceleration);
      stepper1.run();
    }
    else {
      stepper1.stop();
      stepper1.setSpeed(0);
      stepper1.setAcceleration(0);
      //    Serial.println("trying stop");
    }


    if (y / stepSize_y - Enc_y.read() * 2.0 / 3.0 / 4.0  > tolerance ||  Enc_y.read() / 3.0 / 4.0 * 2.0 - y / stepSize_y > tolerance ) {
      stepper2.move(y / stepSize_y  - newPosition2 / 3.0 / 4.0 * 2.0);
      //    Serial.print("Encoder Reads in um: ");
      //    Serial.print(Enc_y.read() / 3.0 * 2.0 * stepSize_y);
      //    Serial.print("   Deviation from command in um: ");
      //    Serial.println(y - Enc_y.read() / 3.0 * 2.0 * stepSize_y);
      stepper2.setAcceleration(Acceleration);
      stepper2.run();
    }
    else {
      stepper2.stop();
      stepper2.setSpeed(0);
      stepper2.setAcceleration(0);
      //    Serial.println("trying stop");
    }



//    Serial.println( newPosition2 / 3.0 * 2.0 * stepSize_y); //Current position in y
//    Serial.println(" Target position is " + String(y));
//    Serial.println("Command is "+ String(y   - newPosition2 / 3.0 * 2.0 * stepSize_y));
//     Serial.println("RUN");
  }

  else {
    delay(10);
//    Serial.println("STOP");
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
    if (Mode_x == 2) {
      reply (String(stepper1.speed() * stepSize_x / 1000)); // mm/s of the x-motor
    }
  } else if (s.startsWith("?vel y")) {
    if (Mode_y == 2) {
      reply (String(stepper2.speed() * stepSize_y / 1000)); // mm/s of the y-motor      
    }
  } else if (s.startsWith("?accel x")) {
    reply(String(Acceleration /  stepSize_x / 1000000)); //converted acceleration to m/s^2
  } else if (s.startsWith("?accel y")) {
    reply(String(Acceleration / stepSize_y / 1000000)); //converted acceleration to m/s^2


    //Setting Acceleration Values
  } else if (s.startsWith("!accel x")) {
    String xa = s.substring(s.indexOf("!accel x ") + 1);
    //    Acceleration = xa.toFloat()*Reduction/stepSize_x*1000000;//converts m/s^2 to step/s^2
    //    stepper1.setAcceleration(Acceleration);
    delay(5);
  } else if (s.startsWith("!accel y")) {
    String ya = s.substring(s.indexOf("!accel y ") + 1);
    //    Acceleration = ya.toFloat()*Reduction/stepSize_y*1000000; //converts m/s^2 to step/s^2
    //    stepper2.setAcceleration(Acceleration);
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
  } else if (s.startsWith("?statusaxis")) { //should return something if
    String response = "@@@";

    if (stepper1.speed()) {
      response[0] = 'M';
    }
    if (stepper2.speed()) {
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
    firstSpace = s.indexOf(' ');
    secondSpace = s.indexOf(' ', firstSpace + 1);
    String delta_x = s.substring(firstSpace, secondSpace);
    String delta_y = s.substring(secondSpace);//Serial.print(apos_y);
    float delta_x_num = delta_x.toFloat();
    float delta_y_num = delta_y.toFloat();
    if (Mode_xy == 0) {
      delta_x_num = delta_x_num * stepSize_x;
      delta_y_num = delta_y_num * stepSize_y;
    }
    x = x + delta_x_num;
    y = y + delta_y_num;
    //    turnServoXY();

    // Absolute Motion Control
  } else if (s.startsWith("!moa ")) {   //relative motion, assume in um
    firstSpace = s.indexOf(' ');
    secondSpace = s.indexOf(' ', firstSpace + 1);
    String apos_x = s.substring(firstSpace, secondSpace);
    String apos_y = s.substring(secondSpace);//Serial.print(apos_y);
    float apos_x_num = apos_x.toFloat();
    float apos_y_num = apos_y.toFloat();
    if (Mode_xy == 0) {
      apos_x_num = apos_x_num * stepSize_x;
      apos_y_num = apos_y_num * stepSize_y;
    }
    x = apos_x_num;
    y = apos_y_num;
//    reply("position for x and y");
//    Serial.print(x);
//    Serial.print(y);
    //    turnServoXY();

    // Setting boundaries for x and y in microns
  } else if (s.startsWith("?lim x")) {
    if (Mode_xy == 1) {
      reply ("-100000000000000.0 100000000000000.0"); // gonna need to check this with Infrastructure
    }
    else if (Mode_xy == 0) {
      reply ("-100000000000000.0 100000000000000.0"); // gonna need to check this with Infrastructure
    }
  } else if (s.startsWith("?lim y")) {
    if (Mode_xy == 1) {
      reply ("-10000.0 10000.0"); // gonna need to check this with Infrastructure
    }
    else if (Mode_xy == 0) {
      reply ("-10000.0 10000.0"); // gonna need to check this with Infrastructure
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
    Enc_x.write(0.0);
    Enc_y.write(0.0);
  } else if (s.startsWith("!pos ")) {   //setting arbitrary position
    String x_set = s.substring(s.indexOf("!pos ") + 1);
    String y_set = s.substring(s.indexOf("!pos ") + 3);
    x = x_set.toFloat();
    y = y_set.toFloat();
    stepper1.setCurrentPosition(x);
    stepper2.setCurrentPosition(y);
    Enc_x.write(x / stepSize_x / 3);
    Enc_y.write(x / stepSize_y / 3 * 2); //Need to include the *2 to because step size of y motor is 0.9 instead of 1.8
  } else if (s.startsWith("?status")) {
    reply ("OK...");
  } else if (s.startsWith("!rm ")) { // Range measure mode, not neccessary
    delay(5);

  } else if (s.startsWith("!speed ")) { //not sure if we need this
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
  stepper1.moveTo(x);

  if (x != newPosition) {
    stepper1.moveTo((x - newPosition / 3));
  }
  stepper2.moveTo(y);

  if (y != newPosition) {
    stepper1.moveTo((y - newPosition2 / 3));

  }

}

void calibrateStage() {   //calibration of system to the limit switches
  stepper1.moveTo(-100000);
  stepper2.moveTo(-100000);
  while (!digitalRead(x_LimPin) && !digitalRead(y_LimPin)) {
    if (!digitalRead(x_LimPin)) {
      stepper1.run();
    }
    if (!digitalRead(y_LimPin)) {
      stepper2.run();
    }
  }
  stepper1.setCurrentPosition(0.0);
  stepper2.setCurrentPosition(0.0);
  Enc_x.write(0.0);
  Enc_y.write(0.0);
}
