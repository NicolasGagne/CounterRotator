#include "MoveHandler.h"
#include <stdlib.h> // For atof
#include <stdio.h>
#include <ctype.h>
#include <Arduino.h>
#include <AccelStepper.h>
#include "VerboseDebugHandler.h"
#include "SerialHandler.h"



// Reset the elevation to 0 
bool calibrationElevation() {
  //Function will return True is sucessfull
  uint8_t system, gyro, accel, mg = 0;

  lcd.setCursor(0,1);
  lcd.print("ELEVATION");

  IMU.getCalibration(&system, &gyro, &accel, &mg);  
  lcd.setCursor(12,0);
  lcd.print(accel);
  lcd.setCursor(13,0);
  lcd.print(gyro);
  lcd.setCursor(14,0);
  lcd.print(mg);
  lcd.setCursor(15,0);
  lcd.print(system);

  //calculate elevation base on non calibrated sensors
  for(int i = 0; i <= 5; i++){
  imu::Vector<3> acc =IMU.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  float estEl = atan2(acc.x()/9.8,acc.z()/9.8)/2/3.141592654*360;

    if (estEl < 15){
      digitalWrite(dirSecondPin, HIGH);
      mainStepper.move(-(int(90 / precision_limit) + 1)); // move up 90degree
      lcd.setCursor(0,1);
      lcd.print("ELEVATION UP 90");
    }else if(estEl > 15 && estEl < 45){
      digitalWrite(dirSecondPin, HIGH);
      mainStepper.move(-(int(50 / precision_limit) + 1)); // move up 50degree
      lcd.setCursor(0,1);
      lcd.print("ELEVATION UP 50");
    }else if(estEl < 75 && estEl > 45){
      digitalWrite(dirSecondPin, LOW);
      mainStepper.move(int(50 / precision_limit) + 1); // move down 50degree
      lcd.setCursor(0,1);
      lcd.print("ELEVATION DW 50");
    }else if(estEl > 75){
      digitalWrite(dirSecondPin, LOW);
      mainStepper.move(int(90 / precision_limit) + 1); // move down 90degree
      lcd.setCursor(0,1);
      lcd.print("ELEVATION DW 90");
    }

  mainStepper.runToPosition();
  IMU.getCalibration(&system, &gyro, &accel, &mg);  
  lcd.setCursor(12,0);
  lcd.print(accel);
  lcd.setCursor(13,0);
  lcd.print(gyro);
  lcd.setCursor(14,0);
  lcd.print(mg);
  lcd.setCursor(15,0);
  lcd.print(system);
  }

  return true;
}

bool calibrationAzimute() {
  //Function will return True is sucessfull
  //verboseDebug(1, "AZ calibrating...");
  uint8_t system, gyro, accel, mg = 0;
  
  //Full turn  turn in one direction
  lcd.setCursor(0,1);
  lcd.print("AZIMUTE LEFT      ");

  mainStepper.moveTo(-(step_per_turn));
  //Set pin for secondary stepper
  digitalWrite(dirSecondPin, LOW);
  mainStepper.runToPosition();
  
  IMU.getCalibration(&system, &gyro, &accel, &mg);  
  lcd.setCursor(12,0);
  lcd.print(accel);
  lcd.setCursor(13,0);
  lcd.print(gyro);
  lcd.setCursor(14,0);
  lcd.print(mg);
  lcd.setCursor(15,0);
  lcd.print(system);

  //verboseDebug(1, "Switch direction"); 
  lcd.setCursor(0,1);
  lcd.print("AZIMUTE RIGHT     ");
  mainStepper.moveTo(step_per_turn);
  digitalWrite(dirSecondPin, HIGH);
  mainStepper.runToPosition();

  IMU.getCalibration(&system, &gyro, &accel, &mg);  
  lcd.setCursor(12,0);
  lcd.print(accel);
  lcd.setCursor(13,0);
  lcd.print(gyro);
  lcd.setCursor(14,0);
  lcd.print(mg);
  lcd.setCursor(15,0);
  lcd.print(system);


  lcd.setCursor(0,1);
  lcd.print("AZIMUTE RESET");
  mainStepper.moveTo(0);
  digitalWrite(dirSecondPin, LOW);
  mainStepper.runToPosition();

  IMU.getCalibration(&system, &gyro, &accel, &mg);  
  lcd.setCursor(12,0);
  lcd.print(accel);
  lcd.setCursor(13,0);
  lcd.print(gyro);
  lcd.setCursor(14,0);
  lcd.print(mg);
  lcd.setCursor(15,0);
  lcd.print(system);

  lcd.setCursor(0,1);
  lcd.print("AZIMUTE FINISH     ");


  return true;
}

//Function that perform the initial Reset
void calibrationRotator(){
  uint8_t system, gyro, accel, mg = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CALIBRATION");
  //verboseDebug(1, "Rotator Azimute move for calibration");
  
  while(accel !=3 || gyro !=3 ||  mg !=3 || system != 3){ 
  lcd.setCursor(12,0);
  lcd.print(accel);
  lcd.setCursor(13,0);
  lcd.print(gyro);
  lcd.setCursor(14,0);
  lcd.print(mg);
  lcd.setCursor(15,0);
  lcd.print(system);

  calibrationElevation();
  delay(2000);
  
  calibrationAzimute();
  delay(2000);
  IMU.getCalibration(&system, &gyro, &accel, &mg);  

  }
  
  //verboseDebug(1, "Rotator Elevation move for calibration");



  //verboseDebug(1, "Rotator Calibration finish");
return;
}