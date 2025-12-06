#include <SPI.h>

#include <SPI.h>

/*
  Antenna Rotator

*/
#include "SerialHandler.h"
#include "MoveHandler.h"
#include <LiquidCrystal.h>
#include <AccelStepper.h>
#include <Wire.h>  // Wire library - used for I2C communication
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <math.h>
#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <qmc5883p.h>
/*
Stepper motor
Because of the physical particularity of this desing both stepper are alwyas required
to do the same number of steps. if they are opposite to each other the elevation change
if they are in the same direction the Azimute change.
Step pin for the secondary motor is physicly tie to the step pin of main motor to garantie
the same number of step at the same time. because of that only one stepper is declare
and we only manually control the direction pin of the secondary stepper. 
*/

// Use those line to activate the BNO_055 or the combinaison of ADXL_345 & Compas QMC8553P
bool active_bno_055 = false;
bool active_adxl345 =true;
bool active_compass =true;

//Main motor is X 
const int enPin=8; //Enablel pin for the motor
const int stepMainPin = 2; //X.STEP
const int dirMainPin = 5; // X.DIR Low clockwise
AccelStepper mainStepper(AccelStepper::DRIVER, stepMainPin, dirMainPin); // Defaults to AccelStepper::FULL4WIRE (4 pins)
//Secondary motor is Y 
//const int stepSecondPin = 3; //Y.STEP
const int dirSecondPin = 6; // Y.DIR Low clockwise

//BNO055 sensor
Adafruit_BNO055 IMU = Adafruit_BNO055();
uint8_t sys, gyro, accel, mg = 0;

//ADXL345
//I2C device found at address 0x53  !
//#define ADXL345_ADDR 0x53   // SDO = GND
/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified ADXL345 = Adafruit_ADXL345_Unified(12345);




 
//QMC5883P
//I2C device found at address 0x2C  !
QMC5883P mag;
// ===================================================================
// INSERT "SOFT-IRON" CALIBRATION VALUES HERE
// Also update bellow:   mag.setHardIronOffsets(-0.022f, -0.298f);
// ===================================================================
const float SCALE_AVG = 0.411f;
const float SCALE_X   = 0.321f;
const float SCALE_Y   = 0.502f;
float xyz[3];
float magDec =  -13.4;  // update with your location

//LCD setup
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
int lcdLoopCounter = 0;

// store the actual position of the Rotator
float actual_el = 0; 
float actual_az = 0;

// store the target position of the Rotator
float target_el = 0;
float target_az = 0;

// Delta between Actual and Targer + or -
float d_az;
float d_el;

//Stepper information
long int step_per_turn = 40320;//200 * 4  * 50.4;    To be adjusted using M0, M1, M2 and gearbox
const float precision_limit = 0.00892857;  // 360 / 40320; can also be use to calculate how many step are required to move to a certer positon
const float ok_pos = 0.25; // Position is in range and OK not move required

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
  delay(1000);

  Serial.begin(115200);
  //Serial.println("Serial Start...");
  
  delay(1000);
  pinMode(dirSecondPin, OUTPUT);
  mainStepper.setAcceleration(500.0);
  mainStepper.setMaxSpeed(3000.0);
  mainStepper.setSpeed(3000.0);
  mainStepper.setCurrentPosition(0);
  mainStepper.setMinPulseWidth(10); //has no effect 

  if(active_bno_055){
    /* Initialise the sensor */
    if(!IMU.begin())
    {
      /* There was a problem detecting the BNO055 ... check your connections */
      Serial.println("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("NO BNO055 detected");
      while(1);
    }
    IMU.setExtCrystalUse(true);
    delay(2000);
    //Perform Calibration function
    //calibrationRotator()
  }

  if(active_adxl345){
    //initADXL345();
    delay(50);

     /* Initialise the sensor */
    if(!ADXL345.begin())
    {
      /* There was a problem detecting the ADXL345 ... check your connections */
      Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("ADXL345 Problems");
      while(1);
    }
     ADXL345.setRange(ADXL345_RANGE_2_G);
      
  }

  if(active_compass){
    if (!mag.begin()) {
      Serial.println("Initialization failed! Compass QMC5883P");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Compass Problems");
      while (true);
    }
    // ===================================================================
    // INSERT "HARD-IRON" CALIBRATION COMMAND HERE
    // ===================================================================
   
    mag.setHardIronOffsets(-0.663f, -0.015f);
  }

  // Enable the servo
  digitalWrite(enPin, LOW);

  // Setup the LCD for display
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("A/T:");
  
}

void loop() {

  while (Serial.available() > 0){
    readRespondSerial();
  }
  
  if (lcdLoopCounter % 10 == 0){

    // Fusion Data BNO_055
    if(active_bno_055){
      sensors_event_t event;
      IMU.getEvent(&event);

      actual_el = event.orientation.y;
      actual_az = fmod(((event.orientation.x + 450) + magDec), 360.0);
    }

    if(active_adxl345){

      /* Get a new sensor event */ 
      sensors_event_t event; 
      ADXL345.getEvent(&event);

      actual_el  = atan2(event.acceleration.y, sqrt(event.acceleration.x * event.acceleration.x + event.acceleration.z * event.acceleration.z)) * 180.0 / PI;
      
      //Serial.print("EL= ");
      //Serial.println(actual_el);
    }

    if(active_compass){
      mag.readXYZ(xyz);
      // Apply soft-iron correction
      xyz[0] *= SCALE_AVG / SCALE_X;
      xyz[1] *= SCALE_AVG / SCALE_Y;
      actual_az = mag.getHeadingDeg(magDec); // Adjust declination
      // Correct for the sensor orientation 
      actual_az = fmod((- actual_az + 180 + 360), 360.0);
      //Serial.print("AZ= ");
      //Serial.println(actual_az);
    }


  }

  //Limit the time the code write to the LCD improve performance
  lcdLoopCounter++;
  if (lcdLoopCounter>=200){
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

    if(active_bno_055){
      IMU.getCalibration(&sys, &gyro, &accel, &mg);  
      lcd.setCursor(0,1);
      lcd.print("C");
      lcd.setCursor(1,1);
      lcd.print(accel);
      lcd.setCursor(2,1);
      lcd.print(gyro);
      lcd.setCursor(3,1);
      lcd.print(mg);
      lcd.setCursor(4,1);
      lcd.print(sys);
    }

    lcdLoopCounter = 0;  
  }


  //Calculate AZ Delta
  d_az = fmod((target_az - actual_az + 360), 360.0);
  if (d_az > 180){
    d_az -= 360.0;
  }
  // calcualte EL delta
  d_el = target_el - actual_el;



  // check if any movement is required
  if (d_az >= ok_pos || d_el >= ok_pos || d_az <= -ok_pos || d_el <= -ok_pos){

    
    if ((((d_az>= 0) ? d_az : -d_az) > ((d_el>= 0) ? d_el : -d_el)) && actual_el > 0 ){
      //Azimute move
          
      if (d_az > 0 ){
        // Clockwise Azimute
        if (lcdLoopCounter>100){
          lcd.setCursor(5,0);
          lcd.print(char(0b01111110)); //point Right
        }

        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, HIGH);

        //Calculate number of step required to move  
        mainStepper.move(int(d_az / precision_limit) + 1);

        mainStepper.run();
        

      }else{
        //Counter Clockwise Azimute
        if (lcdLoopCounter>100){
          lcd.setCursor(5,0);
          lcd.print(char(0b01111111)); //point Left
        }

        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, LOW);

        //Calculate number of step required
        mainStepper.move(int(d_az / precision_limit) + 1);

        mainStepper.run();
      }

    }else{
      //Elevation move
      

      if (d_el > 0 ){
        //higher elevation
        if (lcdLoopCounter>100){
          lcd.setCursor(5,0);
          lcd.write(2); //point UP
        }

        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, HIGH);

        //Calculate the new step target
        mainStepper.move(-(int(d_el / precision_limit) + 1));
        mainStepper.run();
        
      }else{
        //lower elevation
        if (lcdLoopCounter>100){
          lcd.setCursor(5,0);
          lcd.write(1); //point Low
        }

        //Set pin for secondary stepper
        digitalWrite(dirSecondPin, LOW);

        //Calculate number of step required to move to el
        mainStepper.move(-(int(d_el / precision_limit) + 1));
        
        mainStepper.run();
      }
    }

  }else{
    //NO move required
    if (lcdLoopCounter>100){
      lcd.setCursor(5,0);
      lcd.print(" "); //point Right
    }

  }
  //Serial.println(millis()); //Useto check how long the loop take
}
