# CounterRotator

***WORK in progress ***

To come:
- Link for the 3d printed file
- Scematic
- Code update

This is my attemp to build a new rotator concept

Goal;
- All parts 3d print
- Using off the shelf parts as much as possible
- Using servos motors
- Learn stuff...


TO DO:
- Elevation bellow 0 to check
- Switch over to MDU9250 or bno055 for easier orientation of mag north
- Clean up 


Running form Satdump using the Hamlib library "rotctld" to make the link between satdump and the microcontroller

Start rotctld

  rotctld -m 202 -r COM6 -s 115200 -T 127.0.0.1 -t 4533 -vvvvv

## Installation and usage
To get started with the project, follow these steps:

Clone the Repository
- git clone https://github.com/NicolasGagne/CounterRotator.git

Open the directory
- cd Rotator 

Open CounterRotator.ino with the Arduino IDE



Feel free to modify and expand this as needed to fit your specific project details. 
If you need any additional sections or information, let me know!
