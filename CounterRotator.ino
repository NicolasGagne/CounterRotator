/*
  Antenna Rotator

*/
#include "SerialHandler.h"
#include "MoveHandler.h"
#include "VerboseDebugHandler.h"
#include <LiquidCrystal.h>
#include <AccelStepper.h>
#include <Wire.h>  // Wire library - used for I2C communication
#include <MPU6050_light.h>



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

// The MPU65050 sensor I2C address
MPU6050 mpu(Wire);
float magDec =  -13.38;  // update with your location

//Limits Switches
const int limitELPin = 9; // X..DIR  NOT USE FOR NOW
const int limitAZPin = 10; // Y.DIR Switch limite for azumute
const int limitzPin = 11; // Z.DIR  NOT USE FOR NOW


const int rs = 12; //SpnEN
const int en = 13; //SpnDir
const int d4 = 14;  //Abort
const int d5 = 15;  //Hold
const int d6 = 16;  //Resume
const int d7 = 17;  //CoodEN 
//LCD V0 1000 ohm resistor (happy medium)
//LCD A 5V
//LCD VDD 5v
//LCD K Gournd
//LCD VSS Ground
//LCD R/W pin to ground
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// store the actual position of the Rotator
float actual_el = 0;
float actual_az = 0;

// store the target position of the Rotator
float target_el = 0;
float target_az = 0;

//Stepper information
long int step_per_turn = 40320;//200 * 4  * 50.4;    To be adjusted using M0, M1, M2 and gearbox
const float precision_limit = 0.00892857;  // 360 / 40320; can also be use to calculate how many step are required
const float ok_pos = 0.5; // Position is in range and OK no move required

// Serial variable
char buffer[32];
char incomingByte;
int BufferCnt = 0;

// declaring custom symbol for up/down arrow
byte DownArrow[8] = {
  B00000,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
  B00000
};
byte UpArrow[8] = {
  B00000,
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00000
};

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("Initialisation...");
  // creating custom symbol for up/dwn arrow
  lcd.createChar(1, DownArrow);
  lcd.createChar(2, UpArrow);
  //Disable the drivers and Initialise drivers
  digitalWrite(enPin, HIGH);
  delay(2000);

  Serial.begin(115200);
  //verboseDebug(1, F("Rotator Initializating..."));
  //verboseDebug(1, F("Verbose level enable for level 1"));
  //verboseDebug(2, F("Verbose level enable for level 2"));

  
  delay(1000);
  pinMode(dirSecondPin, OUTPUT);
  mainStepper.setAcceleration(500.0);
  mainStepper.setMaxSpeed(1000.0);
  mainStepper.setSpeed(1500.0);
  mainStepper.setCurrentPosition(0);
  mainStepper.setMinPulseWidth(40); //has no effect 

  

  //init wire communicaion
  Wire.begin();

  // MPU 6050 initialisation
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  // stop everything if could not connect to MPU6050
  while(status!=0){
    lcd.print("MPU not detected"); 
    delay(5000); 
    } 
  
  lcd.setCursor(0,1);
  lcd.print("Do not move MPU6050 ");
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(); // gyro and accelero
  Serial.println("Done!\n");
  delay(2000);

  // Enable the servo
  digitalWrite(enPin, LOW);

  //To be re-written
  //resetRotator(); //Perform the reset of the Rotator not sure it needed
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Act:");
  lcd.setCursor(0,1);
  lcd.print("Tar:");
  
}

void loop() {
  while (Serial.available() > 0){
    readRespondSerial();
  }
  mpu.update();
  actual_az = -(fmod((mpu.getAngleZ() + 360.0), 360.0)) + 360 ;  // +/- 180 degree
  actual_el = mpu.getAngleY();  // +/- 180 degree

  lcd.setCursor(7,0);
  lcd.print(actual_az);
  lcd.setCursor(11,0);
  lcd.print("/");
  lcd.setCursor(12,0);
  lcd.print(actual_el);
  
  lcd.setCursor(7,1);
  lcd.print(target_az);
  lcd.setCursor(11,1);
  lcd.print("/");
  lcd.setCursor(12,1);
  lcd.print(target_el);


  //stepper control
  // check if needed to move AZ or EL



  //verboseDebug(2, F("Actual AZ: "), 0);
  //verboseDebug(2, String(actual_az));
  //verboseDebug(2, F("Actual EL: "), 0);
  //verboseDebug(2, String(actual_el));


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


  //verboseDebug(2, F("Delta AZ: "), 0);
  //verboseDebug(2, String(d_az));
  //verboseDebug(2, F("Delta EL: "), 0);
  //verboseDebug(2, String(d_el));
  //verboseDebug(2, F("fabs(d_az) >= precision_limit = "), 0);
  //verboseDebug(2, String(fabs(d_az) > precision_limit));
  //verboseDebug(2, F("fabs(d_el) >= precision_limit = "), 0);
  //verboseDebug(2, String(fabs(d_el) > precision_limit));

  // check if any movement is required
  if (d_az >= ok_pos || d_el >= ok_pos || d_az <= -ok_pos || d_el <= -ok_pos){
    //verboseDebug(1, "Move required");
    
    //if (fabs(d_az) > fabs(d_el)){
    if (((d_az>= 0) ? d_az : -d_az) > ((d_el>= 0) ? d_el : -d_el)) {
      //Azimute move
      //verboseDebug(1,F("Azimute move requried"));
          
      if (d_az > 0 ){
        // Clockwise Azimute
        lcd.setCursor(5,0);
        lcd.print(char(0b01111110)); //point Right
        //verboseDebug(1, F("Azimute move Clockwise"));
        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, HIGH);
        //Calculate number of step required to move  
        mainStepper.move(int(d_az / precision_limit) + 1);
        //verboseDebug(2, F("(int(fabs(d_az) / precision_limit) + 1) "), 0);
        //verboseDebug(2, String(int(fabs(d_az) / precision_limit) + 1));

        mainStepper.run();
        
        //verboseDebug(2, F("Step required to reach AZ targert, Clockwise "), 0);
        //verboseDebug(2, String(mainStepper.distanceToGo()));
        //verboseDebug(2, F("Target: "), 0);
        //verboseDebug(2, String(mainStepper.targetPosition()));

      }else{
        //Counter Clockwise Azimute
        lcd.setCursor(5,0);
        lcd.print(char(0b01111111)); //point Left
        //verboseDebug(1, F("Azimute move CounterClockwise"));
        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, LOW);
        //Calculate number of step required
        mainStepper.move(int(d_az / precision_limit) + 1);
        //verboseDebug(2, F("(int(fabs(d_az) / precision_limit) + 1) "), 0);
        //verboseDebug(2, String(int(fabs(d_az) / precision_limit) + 1));

        mainStepper.run();
        
        //verboseDebug(2, F("Step required to reach AZ targert, Counter Clockwise "), 0);
        //verboseDebug(2, String(mainStepper.distanceToGo()));
        //verboseDebug(2, F("Target: "), 0);
        //verboseDebug(2, String(mainStepper.targetPosition()));
        
      }

    }else{
      //Elevation move
      //verboseDebug(1, F("Elevation move requried"));
      

      if (d_el > 0 ){
        //higher elevation
        lcd.setCursor(5,0);
        lcd.write(2); //point UP
        //verboseDebug(1, F("Higher elevation"));
        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, HIGH);
        //Calculate the new step target
        mainStepper.move(-(int(d_el / precision_limit) + 1));
        //verboseDebug(2, F("(int(fabs(d_el) / precision_limit) + 1) "), 0);
        //verboseDebug(2, String(int(fabs(d_el) / precision_limit) + 1));

        mainStepper.run();

        //verboseDebug(2, F("Step required to reach EL targert, Going UP"), 0);
        //verboseDebug(2, String(mainStepper.distanceToGo()));
        //verboseDebug(2, F("Target: "), 0);
        //verboseDebug(2, String(mainStepper.targetPosition()));
        
      }else{
        //lower elevation
        lcd.setCursor(5,0);
        lcd.write(1); //point Low
        //verboseDebug(1, F("Lower elevation"));
        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, LOW);
        //Calculate number of step required to move to el
        mainStepper.move(-(int(d_el / precision_limit) + 1));
        //verboseDebug(2, F("(int(fabs(d_el) / precision_limit) + 1) "), 0);
        //verboseDebug(2, String(int(fabs(d_el) / precision_limit) + 1));
        
        mainStepper.run();

        //verboseDebug(2, F("Step required to reach EL targert, Going DOWN"), 0);
        //verboseDebug(2, String(mainStepper.distanceToGo()));
        //verboseDebug(2, F("Target: "), 0);
        //verboseDebug(2, String(mainStepper.targetPosition()));
      }
    }

  }else{
    //NO move required
    lcd.setCursor(5,0);
    lcd.print(" "); //point Right
    //verboseDebug(2, F("NO move required"));

  }
  
}
