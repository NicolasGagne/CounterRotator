#ifndef VERBOSEDEBUGHANDLER_H
#define VERBOSEDEBUGHANDLER_H

#include <stdlib.h> // For atof
#include <stdio.h>
#include <ctype.h>
#include <Arduino.h>

extern int verboseLevel;


// Function prototypes

void verboseDebug(int lv, String msg, bool nl=1);

#endif // VERBOSEDEBUGHANDLER_H


