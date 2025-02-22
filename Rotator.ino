/*
  Antenna Rotator

*/
#include "SerialHandler.h"
#include "MoveHandler.h"
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

/*
TO DO:
- Elevation bellow 0 to check
- Reset when hittiing switch
*/

// Start rotctld
// rotctld -m 202 -r COM5 -s 9600-T 127.0.0.1 -t 4533 -vvvvv

//Main motor is X 
const int enPin=8;
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
int step_per_turn = 800;//200 * 4;    To be adjusted using M0, M1, M2
const float precision_limit = 0.45;  // 360 / 800; can also be use to calculate how many step are required

// Serial variable
char buffer[32];
char incomingByte;
int BufferCnt = 0;

void setup() {
  Serial.begin(9600);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, HIGH);
  //pinMode(stepMainPin, OUTPUT);
  //pinMode(dirMainPin, OUTPUT);
  pinMode(dirSecondPin, OUTPUT);
  mainStepper.setAcceleration(10);
  mainStepper.setMaxSpeed(10);
  mainStepper.setSpeed(10);
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
  // calculate AZ delat
  float d_az;
  float d_el;
  if (target_az - actual_az < 180){
    d_az = target_az - actual_az;
  }else{
    d_az = (-360 + target_az) - actual_az;
  }

  // calcualte EL delta
  d_el = target_el - actual_el;
  /*
  Serial.print("Delta AZ: ");
  Serial.println(d_az);
  Serial.print("Delta EL: ");
  Serial.println(d_el);
  Serial.println(fabs(d_az > precision_limit) || fabs(d_el) > precision_limit);
  Serial.println(fabs(d_az) > precision_limit);
  Serial.println(fabs(d_el) > precision_limit);
  Serial.println(fabs(d_az));
  Serial.println(fabs(d_el));
  */
  // check if any movement is required
  // Stop if movement is already in progress
  if ((fabs(d_az) > precision_limit) || (fabs(d_el) > precision_limit)){
    //Serial.println("Move required");
    
    if (fabs(d_az) > fabs(d_el)){
      //Azimute move
      //Serial.println("Azimute move requried");
      
     
      if (d_az < 0 ){
        // Clockwise Azimute
        //Serial.println("Azimute move Clockwise");
        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, LOW);
        //Calculate number of step required to move  
        mainStepper.moveTo(mainStepper.currentPosition() + int(d_az / precision_limit) - 1);
        if (mainStepper.run() == true){
          actual_az = actual_az - precision_limit;
          if (actual_az < 0){
            actual_az = actual_az + 360;
          }
        }
        /*
        Serial.print("Step required to reach AZ targert, Clockwise ");
        Serial.println(mainStepper.distanceToGo());
        Serial.print("Target: ");
        Serial.println(mainStepper.targetPosition());
        */
      }else{
        //Counter Clockwise Azimute
        //Serial.println("Azimute move Counter Clockwise");
        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, HIGH);
        //Calculate number of step required
        mainStepper.moveTo(mainStepper.currentPosition() + (int(d_az / precision_limit) + 1));
        if (mainStepper.run() == true){
          actual_az = actual_az + precision_limit;
          if (actual_az > 360){
            actual_az = actual_az - 360;
          }
        }
        /*
        Serial.print("Step required to reach AZ targert, Counter Clockwise ");
        Serial.println(mainStepper.distanceToGo());
        Serial.print("Target: ");
        Serial.println(mainStepper.targetPosition());
        */
      }

    }else{
      //Elevation move
      //Serial.println("Elevation move requried");
      

      if (d_el > 0 ){
        //higher elevation
        //Serial.println("Higher elevation");
        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, LOW);
        //Calculate the new step target
        mainStepper.moveTo(mainStepper.currentPosition() + int(d_el / precision_limit) + 1);
        if (mainStepper.run() == true){
          actual_el = actual_el + precision_limit;
        }
        /*
        Serial.print("Step required to reach EL targert ");
        Serial.println(mainStepper.distanceToGo());
        Serial.print("Target: ");
        Serial.println(mainStepper.targetPosition());
        */
      }else{
        //lower elevation
        //Serial.println("Lower elevation");
        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, HIGH);
        //Calculate number of step required to move to el
        mainStepper.moveTo(mainStepper.currentPosition() + (int(d_el / precision_limit) - 1));
        if (mainStepper.run() == true){
          actual_el = actual_el - precision_limit;
        }
        /*
        Serial.print("Step required to reach EL targert ");
        Serial.println(mainStepper.distanceToGo());
        Serial.print("Target: ");
        Serial.println(mainStepper.targetPosition());
        */
      }
    }
  }else{
    //NO move required
    //Serial.println("NO move required");
    //delay(250);
  }

  

}
