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


/*
Stepper motor
Because of the physical particularity of this desing both stepper are alwyas required
to do the same number of steps. if they are opposite to each other the elevation change
if they are in the same direction the Azimute change.
Step pin for the secondary motor is physicly tie to the step pin of main motor to garantie
the same number of step at the same time. because of that only one stepper is declare
and we only manually control the direction pin of the secondary stepper. 
*/


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
float thetaM;
float phiM;
float thetaFold=0;
float thetaFnew;
float phiFold=0;
float phiFnew;
 
float thetaG=0;
float phiG=0;
 
float theta;
float phi;
 
float thetaRad;
float phiRad;
 
float Xm;
float Ym;
float oldXm = 0;
float oldYm = 0;
float psi, psi2;
float dt;
unsigned long millisOld;
uint8_t sys, gyro, accel, mg = 0;
 


float magDec =  13.4;  // update with your location

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
const float ok_pos = 0.5; // Position is in range and OK not move required

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

  
  delay(1000);
  pinMode(dirSecondPin, OUTPUT);
  mainStepper.setAcceleration(500.0);
  mainStepper.setMaxSpeed(3000.0);
  mainStepper.setSpeed(3000.0);
  mainStepper.setCurrentPosition(0);
  mainStepper.setMinPulseWidth(10); //has no effect 


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

  // Enable the servo
  digitalWrite(enPin, LOW);

  //Perform Calibration function
  //calibrationRotator(); 

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("A/T:");
  lcd.setCursor(0,1);
  lcd.print("C");
}

void loop() {
  
  while (Serial.available() > 0){
    readRespondSerial();
  }
  
  if (lcdLoopCounter % 10 == 0){
    // Refresh rate of the sensor is 10ms without those call and calculation loop take about 1 millis so check every 10 loop
    imu::Vector<3> acc =IMU.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
    imu::Vector<3> gyr =IMU.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    imu::Vector<3> mag =IMU.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
    //Serial.println(millis());
    // 2 millis second
    thetaM=-atan2(acc.x()/9.8,acc.z()/9.8)/2/3.141592654*360;
    phiM=-atan2(acc.y()/9.8,acc.z()/9.8)/2/3.141592654*360;
    phiFnew =.95*phiFold+.05*phiM;
    
    thetaFnew=.95*thetaFold+.05*thetaM; 
    actual_el = -thetaFnew;
   

    dt=(millis()-millisOld)/1000.;
    millisOld=millis();
    theta=(theta+gyr.y()*dt)*.95+thetaM*.05;
    phi=(phi-gyr.x()*dt)*.95+ phiM*.05;
    //thetaG=thetaG+gyr.y()*dt;  //Not use with this setup
    //phiG=phiG-gyr.x()*dt;     //Not use with this setup
    
    phiRad=phi/360*(2*3.141592654);
    thetaRad=theta/360*(2*3.141592654);

    Xm=(oldXm *.95) + ((mag.x()*cos(thetaRad)-mag.y()*sin(phiRad)*sin(thetaRad)+mag.z()*cos(phiRad)*sin(thetaRad))*.05);
    Ym=(oldYm *.95) + ((mag.y()*cos(phiRad)+mag.z()*sin(phiRad))*.05);
  
    psi =fmod((((atan2(Ym,Xm)/(2*3.141592654)*360)+ magDec) + 360.0), 360.0);
    actual_az = psi; 
    
    phiFold=phiFnew;
    thetaFold=thetaFnew;
    oldXm = Xm;
    oldYm = Ym;
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

    IMU.getCalibration(&sys, &gyro, &accel, &mg);  
    lcd.setCursor(1,1);
    lcd.print(accel);
    lcd.setCursor(2,1);
    lcd.print(gyro);
    lcd.setCursor(3,1);
    lcd.print(mg);
    lcd.setCursor(4,1);
    lcd.print(sys);

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
  //Serial.println("--------");
  /*
  Serial.print(acc.x()/9.8);
  Serial.print(",");
  Serial.print(acc.y()/9.8);
  Serial.print(",");
  Serial.print(acc.z()/9.8);
  Serial.print(",");
  Serial.print(accel);
  Serial.print(",");
  Serial.print(gyro);
  Serial.print(",");
  Serial.print(mg);
  Serial.print(",");
  Serial.print(sys);
  Serial.print(",");
  Serial.print(thetaM);
  Serial.print(",");
  Serial.print(phiM);
  Serial.print(",");
  Serial.print(thetaFnew);
  Serial.print(",");
  Serial.print(phiFnew);
  Serial.print(",");
  Serial.print(thetaG);
  Serial.print(",");
  Serial.print(phiG);
  Serial.print(",");
  Serial.print(theta);
  Serial.print(",");
  Serial.print(phi);
  Serial.print(",");
  Serial.print(psi); 
  Serial.print(",");
  Serial.print(psi2); 
  Serial.print(",");
  Serial.print(actual_el); 
  Serial.print(",");
  Serial.println(actual_az); 
  */
}
