#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

extern float target_el;
extern float target_az;
extern float actual_el;
extern float actual_az;
extern const float precision_limit;
extern int step_per_turn;

extern char buffer[32];
extern char incomingByte;
extern int BufferCnt;

extern const int limitELPin;
extern const int limitAZPin; 
extern const int limitzPin; 

// Function prototypes
void splitCharArray(const char* input);
float extractFloat(const char* input);
void readRespondSerial();

#endif // SERIALHANDLER_H