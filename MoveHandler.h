#ifndef MOVEHANDLER_H
#define MOVEHANDLER_H
#include <AccelStepper.h>
#include "VerboseDebugHandler.h"

extern float actual_el;
extern float actual_az;
extern const float precision_limit;
extern int step_per_turn;
extern const int stepMainPin;
extern const int dirMainPin;
extern AccelStepper mainStepper;
extern const int dirSecondPin;
extern const int limitELPin; // X.DIR Switch limite for Elevation
extern const int limitAZPin; // Y.DIR Switch limite for azumute



// Function prototypes
void resetRotator();
bool resetElevation(); 
bool resetAzimute(); 

#endif // MOVEHANDLER_H