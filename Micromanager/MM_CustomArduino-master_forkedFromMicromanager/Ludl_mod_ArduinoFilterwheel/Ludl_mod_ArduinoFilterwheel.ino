//Allows building simple filterwheel with 3 states
//version 1.0
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

Servo camFilter;
Servo lampFilter;

const int camSig = 7;
const int lampSig = 6;

int camPos = 0;
int lampPos = 0;

void setup() {
  // Initialize filter positions
  camFilter.attach(camSig);
  lampFilter.attach(lampSig);
  camFilter.write(camPos);
  lampFilter.write(lampPos);

  // Initialize connectiton to micromanager
  Serial.begin(9600);
  reply("Vers:dd")
}

void loop() {
  if (Serial.available()) {
    cmd = Serial.readStringUntil('\r');
    processCommand(cmd);
    cmd = "";
  }
  camFilter.write(camPos);
  lampFilter.write(lampPos);
}


void reply(String s) {
  Serial.print(s);
  Serial.print("\n");
}

void processCommand(String s){
  //Responding to query information
  if (s.startsWith("VER")){
    reply("Vers: DD");
  }
 
  else if (s.startsWith("Rconfig")){
    reply(":BigModeController");
  }

  else if (s.startsWith("Rconfig")){
    Serial.print("ControllerName :");
    reply("BigMode");
  }
  
  else if()
  
  
}
