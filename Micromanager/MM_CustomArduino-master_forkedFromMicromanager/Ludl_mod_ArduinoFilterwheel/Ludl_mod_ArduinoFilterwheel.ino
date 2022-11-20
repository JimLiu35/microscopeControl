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
int pos;                    // Will be used later for smoothing out servo control
Servo camFilter;
Servo lampFilter;
String cmd = "" ;
const int camSig = 5;
float TransDelay = 10.0;   //Defines transmission delay between filterwheel and micromanager
int camPos = 0;
int camPos_deg = 0;

void setup() {
  // Initialize filter positions
  camFilter.attach(camSig);
  camFilter.write(camPos);
  delay(1000);
  camFilter.detach();

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
}


void reply(String s) {
  Serial.print(s);
  Serial.print("\n");
}

void processCommand(String s){
  if (s.startsWith(String(0xff41))){
    s = s.substring(s.indexOf(String(0xff41))+2);
    }
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
    turnServo();
    reply("AA");
    }
   //Creating a sudo-id response to trick MM 
   else if (s.endsWith("63:")){
    Serial.print("66:");
    }
   else{
    Serial.print(s);} //Useful for debugging with Micromanager
  
}

void turnServo(){
    camPos_deg = map(camPos,0,2,0,150);
    camFilter.attach(camSig);
    int oldPos_deg = camFilter.read();
    if (camPos_deg>oldPos_deg){
         for (int pos = oldPos_deg; pos <= camPos_deg; pos += 1) { // goes from 0 degrees to 180 degrees
              // in steps of 1 degree
              camFilter.write(camPos_deg);
              delay(10);      
         }
    }
    else{
         for (int pos = oldPos_deg; pos >= camPos_deg; pos -= 1) { // goes from 0 degrees to 180 degrees
              // in steps of 1 degree
              camFilter.write(pos);
              delay(10);      
         }      
      }
    camFilter.detach();
}
