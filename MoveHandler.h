#ifndef MOVEHANDLER_H
#define MOVEHANDLER_H
#include <AccelStepper.h>
#include <LiquidCrystal.h>
#include <Adafruit_BNO055.h>
#include "SerialHandler.h"

extern float actual_el;
extern float actual_az;
extern const float precision_limit;
extern long int step_per_turn;
extern const int stepMainPin;
extern const int dirMainPin;
extern AccelStepper mainStepper;
extern const int dirSecondPin;
extern const int limitELPin; // X.DIR Switch limite for Elevation
extern const int limitAZPin; // Y.DIR Switch limite for azumute
extern LiquidCrystal lcd;
extern Adafruit_BNO055 IMU;
extern uint8_t sys, gyro, accel, mg;



// Function prototypes
void calibrationRotator();
bool calibrationElevation(); 
bool calibrationAzimute(); 
void displayCalibration();

#endif // MOVEHANDLER_H