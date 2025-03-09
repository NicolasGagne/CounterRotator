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
 
  //Set pin for secondary stepper
  digitalWrite(dirSecondPin, LOW);
  // Set new target position
  mainStepper.moveTo(step_per_turn/2);
  unsigned int p = 0; //count steps as safety
  while(digitalRead(limitELPin) == LOW){
    verboseDebug(2, "Steps done: ", 0);
    verboseDebug(2, String(p));
    if (mainStepper.run()){
      p++; 
    }
    if (p >= (step_per_turn/2)){
      verboseDebug(1, "Exceed allow travel to reset EL");
      verboseDebug(2, "Steps done: ", 0);
      verboseDebug(1, String(p));
      delay(1000);
      return false;
    }
    delay(2); //Slow down loop for stepper
  }
  actual_el = 0;
  return true;
}

bool resetAzimute() {
  //Function will return True is sucessfull
  verboseDebug(1, "AZ resetting...");
  unsigned int p = 0; //Store number of step perform
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
    if (mainStepper.run()){
      p++; 
    }
    if (p >= (step_per_turn / 2 )){    
      verboseDebug(1, "Switch direction"); 
      d = 1;
    }
    delay(2); //Slow down loop for stepper
  }

  //Switch direction for the other half turn 
  mainStepper.moveTo(step_per_turn);
  //Set pin for secondary stepper
  digitalWrite(dirSecondPin, HIGH);
  p = 0; // reset p
  while(digitalRead(limitAZPin) == LOW && d == 1){
    verboseDebug(2, "Steps done: ", 0);
    verboseDebug(2, String(p));
    if (mainStepper.run()){
      p++; 
    }
    if (p >= step_per_turn ){   
      verboseDebug(1, "Exceed allow travel to reset AZ");  
      return false;
    }
  delay(2); //Slow down loop for stepper
  }
  actual_az = 0;
  return true;
}

//Function that perform the initial Reset
void resetRotator(){
  verboseDebug(1, "Rotator reseting Elevation to 0.0, please wait");
  verboseDebug(1, "Actual Elevation: ", 0);
  verboseDebug(1, String(actual_el));
  delay(2000);
  if (resetElevation()== false){
    verboseDebug(0, "Error reseting the Rotator, Abort, Rotator stop");
    delay(250);
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
  //Reset El and stepper to 0
  mainStepper.setCurrentPosition(0);
  verboseDebug(1, "Rotator Azimute 0.0");

  verboseDebug(1, "Rotator Resetting finish");
return;
}