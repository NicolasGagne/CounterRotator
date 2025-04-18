#include "MoveHandler.h"
#include <stdlib.h> // For atof
#include <stdio.h>
#include <ctype.h>
#include <Arduino.h>
#include <AccelStepper.h>
#include "SerialHandler.h"



// Reset the elevation to 0 
bool calibrationElevation() {
  //Function will return True is sucessfull

  lcd.setCursor(0,1);
  lcd.print("ELEVATION");

  //calculate elevation base on non calibrated sensors

  imu::Vector<3> acc =IMU.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  float estEl = atan2(acc.x()/9.8,acc.z()/9.8)/2/3.141592654*360;


  if(estEl > 45){
    digitalWrite(dirSecondPin, LOW);

    mainStepper.move(int((estEl+5) / precision_limit) + 1); // move down 
    lcd.setCursor(0,1);
    lcd.print("ELEVATION DW -5");
  }else{
    digitalWrite(dirSecondPin, HIGH);
    mainStepper.move(-(int((80-estEl) / precision_limit) + 1)); // move up 
    lcd.setCursor(0,1);
    lcd.print("ELEVATION UP 90");
    delay(1000);
  }

  mainStepper.runToPosition();
  

  return true;
}

bool calibrationAzimute() {
  //Function will return True is sucessfull
  
  //Full turn  turn in one direction
  lcd.setCursor(0,1);
  lcd.print("AZIMUTE LEFT      ");

  mainStepper.moveTo(-(step_per_turn));
  //Set pin for secondary stepper
  digitalWrite(dirSecondPin, LOW);
  mainStepper.runToPosition();
  
  lcd.setCursor(0,1);
  lcd.print("AZIMUTE RESET");
  mainStepper.moveTo(0);
  digitalWrite(dirSecondPin, HIGH);
  mainStepper.runToPosition();

  lcd.setCursor(0,1);
  lcd.print("AZIMUTE FINISH     ");

  return true;
}

void displayCalibration(){

    IMU.getCalibration(&sys, &gyro, &accel, &mg);  
    lcd.setCursor(12,0);
    lcd.print(accel);
    lcd.setCursor(13,0);
    lcd.print(gyro);
    lcd.setCursor(14,0);
    lcd.print(mg);
    lcd.setCursor(15,0);
    lcd.print(sys);
}


//Function that perform the initial Reset
void calibrationRotator(){
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CALIBRATION");
  displayCalibration();
  
  while(true){ 

    calibrationElevation();
    displayCalibration();
    
    if (accel + gyro + mg + sys >= 7){break;}
    delay(2000);

    calibrationAzimute();
    displayCalibration();
    
    //if (accel ==3 && gyro ==3 &&  mg ==3 && sys == 3){break;}
    if (accel + gyro + mg + sys >= 7){break;}
    delay(2000);
    
  }
  

  lcd.setCursor(0,1);
  lcd.print("CALIBRATION FINISH");
  delay(2000);

return;
}