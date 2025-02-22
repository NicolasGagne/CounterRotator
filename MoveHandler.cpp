#include "MoveHandler.h"
#include <stdlib.h> // For atof
#include <stdio.h>
#include <ctype.h>
#include <Arduino.h>
#include <AccelStepper.h>
#include "VerboseDebugHandler.h"

// Reset the elevation to 0 
bool resetElevation() {
  //Function will return True is sucessfull
  verboseDebug(1, "EL resetting...");
  mainStepper.setMaxSpeed(200);
  mainStepper.setSpeed(200);
  //Set pin for secondary stepper
  digitalWrite(dirSecondPin, HIGH);
  // Set new target position
  mainStepper.moveTo(-(step_per_turn/2));
  int p = 0; 
  verboseDebug(2, "Pin Limit EL = ", 0);
  verboseDebug(2, String(digitalRead(limitELPin)));
  while(digitalRead(limitELPin) == LOW && p <= (step_per_turn/2)){
    verboseDebug(2, "Steps done: ", 0);
    verboseDebug(2, String(p));
    if (mainStepper.runSpeed()){
      p++; 
    }
    if (p >= (step_per_turn/3)){
      return false;
    }
  }
  //Reset El and stepper to 0
  mainStepper.setCurrentPosition(0);
  actual_el = 0;
  return true;
}

bool resetAzimute() {
  //Function will return True is sucessfull
  verboseDebug(1, "AZ resetting...");
  int p = 0; //Store number of step perform
  bool d = 0; //Store direction
  //Try half turn in one direction
  mainStepper.moveTo(-(step_per_turn));
  //Set pin for secondary stepper
  digitalWrite(dirSecondPin, LOW);
  verboseDebug(2, "Pin Limit AZ = ", 0);
  verboseDebug(2, String(digitalRead(limitAZPin)));
  while(digitalRead(limitAZPin) == LOW && d == 0){
    verboseDebug(2, "Steps done: ", 0);
    verboseDebug(2, String(p));
    if (mainStepper.runSpeed()){
      p++; 
    }
    if (p >= (step_per_turn / 2 )){     
      d = 1;
    }
  }

  //Switch direction for the other half turn 
  mainStepper.moveTo(step_per_turn);
  //Set pin for secondary stepper
  digitalWrite(dirSecondPin, HIGH);
  p = 0; // reset p
  while(digitalRead(limitAZPin) == LOW && d == 1){
    verboseDebug(2, "Steps done: ", 0);
    verboseDebug(2, String(p));
    if (mainStepper.runSpeed()){
      p++; 
    }
    if (p >= step_per_turn ){     
      return false;
    }

  }
  //Reset El and stepper to 0
  mainStepper.setCurrentPosition(0);
  actual_az = 0;
  return true;
}

//Function that perform the initial Reset
void resetRotator(){
    int p = 0; //Store number of step perform
  verboseDebug(1, "Rotator reseting Elevation to 0.0, please wait");
  delay(2000);
  if (resetElevation()== false){
    delay(250);
    verboseDebug(0, "Error reseting the Rotator, Abort, Rotator stop");
      exit(0);
  }else{
    verboseDebug(1, "Rotator Elevation 0.0");
  }
  delay(2000);
  verboseDebug(1, "Rotator reseting Azmute to 0.0, please wait");
  if (resetAzimute()== false){
      verboseDebug(0, "Error reseting the Rotator, Abort, Rotator stop");
      delay(250);
      exit(0);
  }
  verboseDebug(1, "Rotator Azimute 0.0");

  verboseDebug(1, "Rotator Resetting finish");
return;
}