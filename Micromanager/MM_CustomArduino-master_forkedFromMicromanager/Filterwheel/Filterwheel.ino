//Allows building simple filterwheel with 3 states
//version 3.0(final)
// DY, XZ, YM 2022

// References
// Ludl Filter wheel from Micromanager

#include <Servo.h>          // Include servo motor controlling libraries
int pos;                    // Will be used later for smoothing out servo control

// Define servo names
Servo camFilter;
Servo lampFilter;
String cmd = "" ;     // Initialize command recieved from serial comm.
// Define motor pins and LED pin
const int camSig = 5;
const int lampSig = 3;
const int BF_pin = 8;

float TransDelay = 10.0;   //Defines transmission delay between filterwheel and micromanager
// Initialize motor commands
int camPos = 0;
int lampPos = 0;
int camPos_deg = 0;
int lampPos_deg = 0;

void setup() {
  // Initialize filter positions. To move and reduce noise, we first move attach the servos, then move them to command position, and then detach the servos
  camFilter.attach(camSig);
  camFilter.write(camPos);
  lampFilter.attach(lampSig);
  lampFilter.write(lampPos);
  delay(1000);
  camFilter.detach();
  lampFilter.detach();

  // Initialize LED
  pinMode(BF_pin, OUTPUT);
  digitalWrite(BF_pin, LOW);
  // Initialize connectiton to micromanager
  //  Serial.begin(115200);   // Set baud rate to 115200
  Serial.begin(9600);         // Set baud rate to 9600
  reply("Vers:dd_Filterwheel");           // Tells Micromanager that filterwheel is connected
}

void loop() { // Main piece of code. Basically, we repeat the process of reading from serial comm ports and responding to command
  if (Serial.available()) {
    cmd = Serial.readStringUntil('\r');
    processCommand(cmd);
    cmd = "";
  }
}


void reply(String s) { // This is how we send command signals
  Serial.print(s);
  Serial.print("\n");
}

void processCommand(String s) {

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
  
  // Responding to change of delay times; not that useful in the code but is necessary for communicating with Micromanager
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
    if (cmd == "H") { //Homing the filter wheel
      camPos = 1;
      lampPos = 1;
    }
    else { // Read in numerical position state
      camPos = cmd.toFloat();
      lampPos = cmd.toFloat();
    }
    turnServo(); // Move servos to correct position
    reply("AA"); // Responce to Micromanager that the command was recieved and the filterwheel has moved
    if (cmd.toFloat() == 2) { //If state is 2, which is BF, then make sure the BF LED is on
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
  else { //Useful for debugging with Micromanager, just to see what Micromanager has sent to Arduino board
    Serial.print(s);
  } 

}

//Function for moving the servo motor
void turnServo() {
  // Limit commands to be greater than or equal to 1
  if (camPos < 1) {
    camPos = 1;
    lampPos = 1;
  }
  // Convert command integer into an angle value
  camPos_deg = map(camPos, 1, 3, 0, 150); //After testing with OP Squad, we found that the range for the filterwheels are 0-150 degs
  lampPos_deg = map(lampPos, 1, 3, 0, 150);
  // Let motors move to angle value
  camFilter.attach(camSig);
  lampFilter.attach(lampSig);

  // Move motors iteratively to desired position as to not miss steps
  int oldPos_deg = camFilter.read();
  if (camPos_deg > oldPos_deg) {
    for (int pos = oldPos_deg; pos <= camPos_deg; pos += 1) { // move from current position to larger angle
      // in steps of 1 degree
      camFilter.write(camPos_deg);
      lampFilter.write(lampPos_deg);
      delay(10);
    }
  }
  else {
    for (int pos = oldPos_deg; pos >= camPos_deg; pos -= 1) { // move from current position to lower angle
      // in steps of 1 degree
      camFilter.write(camPos_deg);
      lampFilter.write(lampPos_deg);
      delay(10);
    }
  }
  // Detach motors to reduce noise
  camFilter.detach();
  lampFilter.detach();
}
