/*
  Antenna Rotator

*/
#include "SerialHandler.h"
#include "MoveHandler.h"
#include "VerboseDebugHandler.h"
#include <AccelStepper.h>
/*
Stepper motor
Because of the physical particularity of this desing both stepper are alwyas required
to do the same number of steps. if they are opposite to each other the elevation change
if they are in the same direction the Azimute change.
Step pin for the secondary motor is physicly tie to the step pin of main motor to garantie
the same number of step at the same time. because of that only one stepper is declare
and we only manually control the direction pin of the secondary stepper. 
*/

//Verbose message can be modified sending: VERBOSE 0, VERBOSE 1 OR VERBOSE 2
  /*
  lv - level of debuging
    0 - Error
    1 - Debug
    2 - Verbatime
  */
int verboseLevel = 0;

//Main motor is X 
const int enPin=8; //Enablel pin for the motor
const int stepMainPin = 2; //X.STEP
const int dirMainPin = 5; // X.DIR Low clockwise
AccelStepper mainStepper(AccelStepper::DRIVER, stepMainPin, dirMainPin); // Defaults to AccelStepper::FULL4WIRE (4 pins)
//Secondary motor is Y 
//const int stepSecondPin = 3; //Y.STEP
const int dirSecondPin = 6; // Y.DIR Low clockwise

//Limits Switches
const int limitELPin = 9; // X.DIR Switch limite for Elevation
const int limitAZPin = 10; // Y.DIR Switch limite for azumute
const int limitzPin = 11; // Z.DIR  NOT USE FOR NOW

// store the actual position of the Rotator
float actual_el = 0;
float actual_az = 0;

// store the target position of the Rotator
float target_el = 0;
float target_az = 0;

//Stepper information
long int step_per_turn = 40320;//200 * 4  * 50.4;    To be adjusted using M0, M1, M2 and gearbox
const float precision_limit = 0.00892857;  // 360 / 40320; can also be use to calculate how many step are required

// Serial variable
char buffer[32];
char incomingByte;
int BufferCnt = 0;

void setup() {
  Serial.begin(115200);
  verboseDebug(1, F("Rotator Initializating..."));
  verboseDebug(1, F("Verbose level enable for level 1"));
  verboseDebug(2, F("Verbose level enable for level 2"));
  digitalWrite(enPin, HIGH);
  delay(1000);
  pinMode(dirSecondPin, OUTPUT);
  mainStepper.setAcceleration(1000.0);
  mainStepper.setMaxSpeed(1000.0);
  mainStepper.setSpeed(1000.0);
  mainStepper.setCurrentPosition(0);

  pinMode(limitELPin, INPUT);
  pinMode(limitAZPin, INPUT);
  // Enable the servo
  digitalWrite(enPin, LOW);
  resetRotator(); //Perform the reset of the Rotator
 
}
void loop() {

  while (Serial.available() > 0){
    readRespondSerial();
  }

  //stepper control
  // check if needed to move AZ or EL
  // calculate AZ delta
  float d_az;
  float d_el;
  //Calculate AZ Delta
  d_az = fmod((target_az - actual_az + 360), 360.0);
  if (d_az > 180){
    d_az -= 360.0;
  }
  // calcualte EL delta
  d_el = target_el - actual_el;

  verboseDebug(2, F("Delta AZ: "), 0);
  verboseDebug(2, String(d_az));
  verboseDebug(2, F("Delta EL: "), 0);
  verboseDebug(2, String(d_el));
  verboseDebug(2, F("fabs(d_az) >= precision_limit = "), 0);
  verboseDebug(2, String(fabs(d_az) > precision_limit));
  verboseDebug(2, F("fabs(d_el) >= precision_limit = "), 0);
  verboseDebug(2, String(fabs(d_el) > precision_limit));

  // check if any movement is required
  if ((fabs(d_az) >= precision_limit) || (fabs(d_el) >= precision_limit)){
    verboseDebug(1, "Move required");
    
    if (fabs(d_az) > fabs(d_el)){
      //Azimute move
      verboseDebug(1,F("Azimute move requried"));
          
      if (d_az > 0 ){
        // Clockwise Azimute
        verboseDebug(1, F("Azimute move Clockwise"));
        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, HIGH);
        //Calculate number of step required to move  
        mainStepper.moveTo(mainStepper.currentPosition() + (int(fabs(d_az) / precision_limit) + 1));
        verboseDebug(2, F("(int(fabs(d_az) / precision_limit) + 1) "), 0);
        verboseDebug(2, String(int(fabs(d_az) / precision_limit) + 1));
        if (mainStepper.run() == true){
          actual_az = actual_az + precision_limit;
          if (actual_az >= 360){
            actual_az = actual_az - 360;
          }
        }
        
        verboseDebug(2, F("Step required to reach AZ targert, Clockwise "), 0);
        verboseDebug(2, String(mainStepper.distanceToGo()));
        verboseDebug(2, F("Target: "), 0);
        verboseDebug(2, String(mainStepper.targetPosition()));

      }else{
        //Counter Clockwise Azimute
        verboseDebug(1, F("Azimute move CounterClockwise"));
        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, LOW);
        //Calculate number of step required
        mainStepper.moveTo(mainStepper.currentPosition() - (int(fabs(d_az) / precision_limit) + 1));
        verboseDebug(2, F("(int(fabs(d_az) / precision_limit) + 1) "), 0);
        verboseDebug(2, String(int(fabs(d_az) / precision_limit) + 1));
        if (mainStepper.run() == true){
          actual_az = actual_az - precision_limit;
          if (actual_az <= 0){
            actual_az = actual_az + 360;
          }
        }
        
        verboseDebug(2, F("Step required to reach AZ targert, Counter Clockwise "), 0);
        verboseDebug(2, String(mainStepper.distanceToGo()));
        verboseDebug(2, F("Target: "), 0);
        verboseDebug(2, String(mainStepper.targetPosition()));
        
      }

    }else{
      //Elevation move
      verboseDebug(1, F("Elevation move requried"));
      

      if (d_el > 0 ){
        //higher elevation
        verboseDebug(1, F("Higher elevation"));
        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, HIGH);
        //Calculate the new step target
        mainStepper.moveTo(mainStepper.currentPosition() - (int(fabs(d_el) / precision_limit) + 1));
        verboseDebug(2, F("(int(fabs(d_el) / precision_limit) + 1) "), 0);
        verboseDebug(2, String(int(fabs(d_el) / precision_limit) + 1));
        if (mainStepper.run() == true){
          actual_el = actual_el + precision_limit;
        }

        verboseDebug(2, F("Step required to reach EL targert, Going UP"), 0);
        verboseDebug(2, String(mainStepper.distanceToGo()));
        verboseDebug(2, F("Target: "), 0);
        verboseDebug(2, String(mainStepper.targetPosition()));
        
      }else{
        //lower elevation
        verboseDebug(1, F("Lower elevation"));
        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, LOW);
        //Calculate number of step required to move to el
        mainStepper.moveTo(mainStepper.currentPosition() + (int(fabs(d_el) / precision_limit) + 1));
        verboseDebug(2, F("(int(fabs(d_el) / precision_limit) + 1) "), 0);
        verboseDebug(2, String(int(fabs(d_el) / precision_limit) + 1));
        if (mainStepper.run() == true){
          actual_el = actual_el - precision_limit;
        }

        verboseDebug(2, F("Step required to reach EL targert, Going DOWN"), 0);
        verboseDebug(2, String(mainStepper.distanceToGo()));
        verboseDebug(2, F("Target: "), 0);
        verboseDebug(2, String(mainStepper.targetPosition()));
      }
    }
  }else{
    //NO move required
    verboseDebug(2, F("NO move required"));
    //delay(250);

    // Possible place for the Reset?
    // if Targets are 0
  }

  

}
