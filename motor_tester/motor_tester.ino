/*
  Motor tester

*/

#include <AccelStepper.h>


//Main motor is X 
const int enPin=8; //Enablel pin for the motor
const int stepMainPin = 2; //X.STEP
const int dirMainPin = 5; // X.DIR Low clockwise
AccelStepper mainStepper(AccelStepper::DRIVER, stepMainPin, dirMainPin); // Defaults to AccelStepper::FULL4WIRE (4 pins)
//Secondary motor is Y 
//const int stepSecondPin = 3; //Y.STEP
const int dirSecondPin = 6; // Y.DIR Low clockwise

const long int step = 40320; //number of step to turn
int test = 0; // rotate

/*
Direction:
0 = Clockwise
1 = counter Clockwise
2 = Go low
3 = go high
*/
const int dir = 2; 

void setup() {

  Serial.begin(115200);
  Serial.println("Motor tester....");
  pinMode(dirSecondPin, OUTPUT);
  digitalWrite(enPin, LOW);
  mainStepper.setAcceleration(500.0);
  mainStepper.setMaxSpeed(1000.0);
  mainStepper.setSpeed(1500.0);
  mainStepper.setCurrentPosition(0);
  mainStepper.setMinPulseWidth(40); //has no effect 

}

void loop() {
  switch (dir){
  
    case 0 : 
      Serial.println("Both motor in same direction Clockwise");
      digitalWrite(dirSecondPin, HIGH);
      mainStepper.moveTo(-step);
      break; 

    case 1: 
      Serial.println("Both motor in same direction Counter Clockwise");
      digitalWrite(dirSecondPin, HIGH);
      mainStepper.moveTo(step);
      break;
  
    case 2:
      Serial.println("LOWER ELEVATION");
      digitalWrite(dirSecondPin, LOW);
      mainStepper.moveTo(step);
      break;

    case 3:
      Serial.println("HIGER ELEVATION");
      digitalWrite(dirSecondPin, HIGH);
      mainStepper.moveTo(-step);
      break;
  }
  mainStepper.run();

}
