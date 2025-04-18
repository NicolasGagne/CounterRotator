#include "SerialHandler.h"
#include <stdlib.h> // For atof
#include <stdio.h>
#include <ctype.h>
#include <Arduino.h>

// Function to extract and convert the numeric part of a char array to a float
float extractFloat(const char* input) {
  // Buffer to store the numeric part
  char numericPart[20];
  int j = 0;

  // Iterate through the input array
  for (int i = 0; input[i] != '\0'; i++) {
    // Check if the character is a digit or a decimal point
    if (isdigit(input[i]) || input[i] == '.') {
      numericPart[j++] = input[i];
    }
  }
  // Null-terminate the numeric part
  numericPart[j] = '\0';

  // Convert the numeric part to a float
  return atof(numericPart);
}


// Function to split a char array based on a single space character
void splitCharArray(const char* input) {
  int i = 0;
  int j = 0;

    // Arrays to store the split parts
  char part1[32];
  char part2[32];

  // Copy characters to part1 until a space is found
  while (input[i] != ' ' && input[i] != '\0') {
    part1[j++] = input[i++];
  }
  part1[j] = '\0'; // Null-terminate part1

  // Skip the space character
  if (input[i] == ' ') {
    i++;
  }

  j = 0;
  // Copy the remaining characters to part2
  while (input[i] != '\0') {
    part2[j++] = input[i++];
  }
  part2[j] = '\0'; // Null-terminate part2

    // Extract the float number
    // azimut in the part1 and elevation in part2
  target_az = extractFloat(part1); 
  target_el = extractFloat(part2);
  return;
}

void readRespondSerial(){

  String str1, str2, str3, str4, str5, str6;

    incomingByte = Serial.read();
    if (incomingByte == '\n' || incomingByte == '\r') {
        
      if (buffer[0] == 'A' && buffer[1] == 'Z' && BufferCnt == 2) {
        // Send current Azimute position in deg
        str1 = String("AZ");
        str2 = String(actual_az, 2);
        str5 = String("\n");
        Serial.print(str1 + str2 + str5);
      }
      else if(buffer[0] == 'E' && buffer[1] == 'L' && BufferCnt == 2) {
        // Send current Elevation position in deg     
        str3 = String("EL");
        str4 = String(actual_el, 2);
        str5 = String("\n");
        Serial.print(str3 + str4 + str5);
      }  
      else if(buffer[0] == 'A' && buffer[1] == 'Z' && buffer[2] == ' ' && buffer[3] == 'E' && buffer[4] == 'L') {
        // Send current Az and El position in deg     
        str1 = String("AZ");
        str2 = String(actual_az, 2);
        str3 = String(" EL");
        str4 = String(actual_el, 2);
        str5 = String("\n");
        Serial.print(str1 + str2 + str3 + str4 + str5);
      }
      else if(buffer[0] == 'A' && buffer[1] == 'Z' && BufferCnt > 8){
        // Receive Desire Azimute and Elevation position in deg
        // Split the char array and update targets
        splitCharArray(buffer);
      }
      // For testing return the target AZ and target EL
      // Send TEST
      else if(buffer[0] == 'T' && buffer[1] == 'E' && buffer[2] == 'S' && buffer[3] == 'T'){
        Serial.println("Rotator Information...");
        Serial.print("Step per turn: ");
        Serial.println(step_per_turn);
        Serial.print("Precision limit: ");
        Serial.print(precision_limit, 10);
        Serial.println(" degree");    
        Serial.print("Target AZ: ");
        Serial.println(target_az);
        Serial.print("Target EL: ");
        Serial.println(target_el);
        Serial.print("Actual AZ: ");
        Serial.println(actual_az);
        Serial.print("Actual EL: ");
        Serial.println(actual_el);

      }
      
      // Reset the buffer an clean the serial buffer
      BufferCnt = 0;
      Serial.flush();
      for (int i = 0; i < sizeof(buffer) - 1; i++) {
        buffer[i] = "i";
      }
        
    }else{
    // Fill the buffer with incoming data
    buffer[BufferCnt] = incomingByte;
    BufferCnt++;
    }
  return;  
}
