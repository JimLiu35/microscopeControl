
#include <Servo.h>
#include <SoftwareSerial.h>
#include <Wire.h>


Servo myservo;
int servoPin = 9;
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
String cmd = "" ;
float z = 0.0;




void setup() {
  Serial.begin( 9600 );
  myservo.attach(9, 600, 2300);
  pinMode(13, OUTPUT); //设置13号端口作为输出端口
  digitalWrite(13, HIGH);
  lcd.begin(16, 2);
  lcd.print("MiFoBio 2018");
  lcd.setCursor(0, 1);
  lcd.print("vLStep-Z ready");
  delay(1000);
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
  if (s.startsWith("?ver")) {
    reply ("Vers:LS");
  } else if (s.startsWith("*idn?")) {
    reply ("Id = FW102C BuadRate = 115200");
  } else if (s.startsWith("?pos z")) {
    String zs = String(z, 1);
    reply (zs);
    lcd.clear();
    lcd.print("MiFoBio 2018");
    lcd.setCursor(0, 1);
    lcd.print("Z:");
    lcd.print(zs);
  } else if (s.startsWith("?lim z")) {
    reply ("0.0 100.0");
  } else if (s.startsWith("?status")) {
    reply ("OK...");
  } else if (s.startsWith("pos=(1)")) {
      myservo.write(360); 
      delay(1000);
      digitalWrite(13,LOW);
  } else if (s.startsWith("pos=(2)")) {
      myservo.write(90);              
      delay(1000);
      digitalWrite(13,LOW);
  }else if (s.startsWith("pos=(3)")) {
      myservo.write(0);              
      delay(1000);
      digitalWrite(13,LOW);
  }
}

void reply(String s) {
  Serial.print(s);
  Serial.print("\r");
}

void turnServo() {
  myservo.write(0);
  delay(1000);
  digitalWrite(13, LOW);
}
