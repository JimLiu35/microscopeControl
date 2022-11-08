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
String cmd = "" ;
const int camSig = 5;
const int lampSig = 6;
float TransDelay = 10.0;   //Defines transmission delay between filterwheel and micromanager
int camPos = 0;
int lampPos = 0;

int camPos_deg = 0;
int lampPos_deg = 0;

void setup() {
  // Initialize filter positions
  camFilter.attach(camSig);
  lampFilter.attach(lampSig);
  camFilter.write(camPos);
  lampFilter.write(lampPos);

  // Initialize connectiton to micromanager
  Serial.begin(9600);
  reply("Vers:dd");
}

void loop() {
  if (Serial.available()) {
    cmd = Serial.readStringUntil('\r');
    processCommand(cmd);
    cmd = "";
  }
  camPos_deg = map(camPos,0,3,0,180);
  lampPos_deg = map(lampPos,0,3,0,180);
  camFilter.write(camPos_deg);
  lampFilter.write(lampPos_deg);
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
    Serial.print("ControllerName :");
    reply("BigMode");
  }
  
  else if(s.startsWith("Remres")){
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

    
  else if (s.startsWith("TRXDEL")){
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
  
  else if (s.startsWith("STATUS S")){
    Serial.print("N"); //once for the controller

    }
  else if (s.startsWith("Rotat S")){
    
    String cmd = s.substring(s.indexOf("M") + 1,s.length());
    if (cmd == "H"){
      camPos = 0;
      }
    else {
      camPos = cmd.toFloat();
      } 
    lampPos = camPos;
    reply("AA");
    }
   //Creating a sudo-id response to trick MM 
   else if (s.endsWith("63:")){
    Serial.print("66:");
    }
    
  
}
