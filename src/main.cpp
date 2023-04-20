/*
Auteur    : Bilel Belhadj
Titre     : projet de smart bike locker
Date      : 39-03-2023
Version   : 0.0.1
*/

#include <Arduino.h>
#include "WIFIConnector_MKR1000.h"
#include "MQTTConnector.h" 
#include <SPI.h>
#include <Keypad.h>
#include <Servo.h>


//define variables
#define ROWS 4            //le ligne de clavier
#define COLS 4            //le colonne de clavier


//declaration des objets
Servo myservo;    // creation de l'objet servo
const char kp4x4Keys[ROWS][COLS]  = {{'1', '2', '3', 'A'}, {'4', '5', '6', 'B'}, {'7', '8', '9', 'C'}, {'*', '0', '#', 'D'}};
byte rowKp4x4Pin [4] = {4, 5, 6, 7};
byte colKp4x4Pin [4] = {8, 9, 10, 11};


//declaratioon des constantes
const int BREAKS = 0;
const int BREAKS_INDICATOR = 1;
const int STOP_INDICATOR   = 2;
const int MOTOR_PIN = 3;
const int PIDAL     = 13; 
const String password = "1234";


//declaration des variables
Keypad kp4x4  = Keypad(makeKeymap(kp4x4Keys), rowKp4x4Pin, colKp4x4Pin, ROWS, COLS);
char customKey;
bool pidalAceess = false, pwdAccess = false, pwdSet = true, lockSet = true, breaksStat = false;
String inputPwd = "";
int pidalStatus = 0;


// Read button states from keypad
void readKp4x4() { 
  customKey = kp4x4.getKey();
  if (customKey && customKey != 'B') {
    Serial.println(customKey);
    inputPwd += customKey;
  }

  if (customKey && customKey == 'B' && pwdSet){
      Serial.println(customKey);
      if (inputPwd == password)
      {
        pwdAccess = true;
        pwdSet = false;
        pidalAceess = true;
      }else{
        pwdAccess = false;
        inputPwd = "";
      }
    }
}


void setup() {
  Serial.begin(9600);

  //connecter sur le wifi
  wifiConnect();                  
  MQTTConnect(); 

  //pin out modes
  pinMode(BREAKS_INDICATOR, OUTPUT);
  pinMode(STOP_INDICATOR, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(PIDAL, INPUT);
  digitalWrite(BREAKS_INDICATOR, LOW);
  digitalWrite(STOP_INDICATOR  , LOW);

  myservo.attach(0);    //servo broche
  myservo.write(0);
}


void loop() {
  ClientMQTT.loop();     //continuer a ecouter s'il y'a des RPC
  readKp4x4();

  if(digitalRead(PIDAL) == 1 && pidalAceess && pwdAccess){
    myservo.write(0);
    analogWrite(MOTOR_PIN, 178);
    pidalStatus = 1;
  }else{
    analogWrite(MOTOR_PIN, 0);
  }

  if(lock && breaksStat){
    for (int i = 0; i < 179; i++){
      myservo.write(i);
      digitalWrite(BREAKS_INDICATOR, HIGH);
      delay(20);
      pidalAceess = false;
      lockSet = false;
      breaksStat = false;

      if (i == 178){
        digitalWrite(STOP_INDICATOR, HIGH);
      }
    }
  }
  
  if(lock == false){
    digitalWrite(STOP_INDICATOR, LOW);
    digitalWrite(BREAKS_INDICATOR, LOW);
    pidalAceess = true;
    breaksStat = true;
  }
  
  Serial.print("input pwd :");
  Serial.println(inputPwd);
  Serial.print("pidal acess : ");
  Serial.println(pidalAceess);
  Serial.print("password access : ");
  Serial.println(pwdAccess);

  
  appendPayload("pidal", pidalStatus);  
  sendPayload();  
  delay(500);
}