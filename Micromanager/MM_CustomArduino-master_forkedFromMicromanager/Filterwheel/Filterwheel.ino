//Allows building simple filterwheel with 3 states
//version 3.0(final)
// DY 2022

// References
// Ludl

// in HCW, use serial port settings:
// AnswerTimeout,3000.0000
// BaudRate,9600
// DelayBetweenCharsMs,0.0000
// Handshaking,Off
// Parity,None
// StopBits,1

#include <Servo.h>
int pos;                    // Will be used later for smoothing out servo control
Servo camFilter;
Servo lampFilter;
String cmd = "" ;
const int camSig = 5;
const int lampSig = 3;
float TransDelay = 10.0;   //Defines transmission delay between filterwheel and micromanager
int camPos = 0;
int lampPos = 0;
int camPos_deg = 0;
int lampPos_deg = 0;
const int BF_pin = 8;
void setup() {
  // Initialize filter positions
  camFilter.attach(camSig);
  camFilter.write(camPos);
  lampFilter.attach(lampSig);
  lampFilter.write(lampPos);
  delay(1000);
  camFilter.detach();
  lampFilter.detach();
  pinMode(BF_pin, OUTPUT);
  digitalWrite(BF_pin, LOW);
  // Initialize connectiton to micromanager
  //  Serial.begin(115200);
  Serial.begin(9600);
  reply("Vers:dd");
}

void loop() {
  if (Serial.available()) {
    cmd = Serial.readStringUntil('\r');
    processCommand(cmd);
    cmd = "";
  }
}


void reply(String s) {
  Serial.print(s);
  Serial.print("\n");
}

void processCommand(String s) {
  if (s.startsWith(String(0xff41))) {
    s = s.substring(s.indexOf(String(0xff41)) + 2);
  }
  //Responding to query information
  if (s.startsWith("VER")) {
    reply("Vers: DD");
  }

  else if (s.startsWith("Rconfig")) {
    Serial.print("ControllerName :");
    reply("BigMode");
  }

  else if (s.startsWith("Remres")) {
    delay(10);
  }

  //  else if (s.startsWith("TRXDEL ")){
  //    String cmd = s.substring(s.indexOf("TRXDEL ") + 1);
  //    TransDelay = cmd.toFloat();
  //    reply(":A");
  //    }
  //  else if (s.startsWith("TRXDEL")){
  //    reply("AA");
  //    }


  else if (s.startsWith("TRXDEL")) {
    if (s.length() > 6) {
      String cmd = s.substring(s.indexOf("TRXDEL ") + 8);
      TransDelay = cmd.toFloat();
      reply(":A");
    }
    else {
      reply("AA");
    }
  }

  // Here's where the position definition occurs for the filter wheels

  else if (s.startsWith("STATUS S")) {
    Serial.print("N"); //once for the controller

  }
  else if (s.startsWith("Rotat S")) {

    String cmd = s.substring(s.indexOf("M") + 1, s.length());
    if (cmd == "H") {
      camPos = 1;
      lampPos =1;
    }
    else {
      camPos = cmd.toFloat();
      lampPos = cmd.toFloat();
    }
    turnServo();
    reply("AA");
    if (cmd.toFloat() == 2) {
      digitalWrite(BF_pin, HIGH);
    }
    else {
      digitalWrite(BF_pin, LOW);
    }
  }

  //Creating a sudo-id response to trick MM
  else if (s.endsWith("63:")) {
    Serial.print("66:");
  }
  else {
    Serial.print(s);
  } //Useful for debugging with Micromanager

}

void turnServo() {
  if (camPos < 1) {
    camPos = 1;
    lampPos = 1;
  }
  camPos_deg = map(camPos, 1, 3, 0, 150);
  lampPos_deg = map(lampPos, 1, 3, 0, 150);
  camFilter.attach(camSig);
  lampFilter.attach(lampSig);
  int oldPos_deg = camFilter.read();
  if (camPos_deg > oldPos_deg) {
    for (int pos = oldPos_deg; pos <= camPos_deg; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      camFilter.write(camPos_deg);
      lampFilter.write(lampPos_deg);
      delay(10);
    }
  }
  else {
    for (int pos = oldPos_deg; pos >= camPos_deg; pos -= 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      camFilter.write(camPos_deg);
      lampFilter.write(lampPos_deg);
      delay(10);
    }
  }
  camFilter.detach();
  lampFilter.detach();
}
