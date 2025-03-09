#include "VerboseDebugHandler.h"
#include <stdlib.h> // For atof
#include <stdio.h>
#include <ctype.h>
#include <Arduino.h>



//function that handle verbose for debuging 
// accept 2 levels of verbose short and long
void verboseDebug(int lv, String msg, bool nl = 1){
  /*
  lv - level of debuging
    0 - Error
    1 - Debug
    2 - Verbatime
  msg - message to print
  nl - new line
  */
  if(verboseLevel >= lv ){
    if(nl == 1 ){
      Serial.println(msg);
    }else if (nl == 0 ){
      Serial.print(msg);
    }
  }
  
}

