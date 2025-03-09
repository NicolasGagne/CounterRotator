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
char dir; 

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

  Serial.println("Use a, d, s, w, to move in a particular direction");
  Serial.println("Use any other keey to stop");

}

void loop() {
  if (Serial.available() > 0) {
    dir = Serial.read();
    while(Serial.available()) Serial.read();
  }

  switch (dir){
  
    case 'a' : 
      Serial.println("Both motor in same direction Clockwise");
      digitalWrite(dirSecondPin, LOW);
      mainStepper.moveTo(-step);
      break; 

    case 'd': 
      Serial.println("Both motor in same direction Counter Clockwise");
      digitalWrite(dirSecondPin, HIGH);
      mainStepper.moveTo(step);
      break;
  
    case 's':
      Serial.println("LOWER ELEVATION");
      digitalWrite(dirSecondPin, LOW);
      mainStepper.moveTo(step);
      break;

    case 'w':
      Serial.println("HIGER ELEVATION");
      digitalWrite(dirSecondPin, HIGH);
      mainStepper.moveTo(-step);
      break;

    default:
      Serial.println("No Movement");
      mainStepper.moveTo(mainStepper.currentPosition());
  }
  mainStepper.run();
  
}
