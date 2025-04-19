# CounterRotator

***WORK in progress ***

To come:
- Link for the 3d printed file
- Scematic
- Code update

This is my attemp to build a new rotator concept using 2 horizontal stepper motors.
It is desing for the 1.7 Ghz HRPT Helicone Antenna from T0nito https://www.thingiverse.com/thing:6436342

Goal;
- All parts 3d print
- Using off the shelf parts as much as possible
- Using servos motors
- Learn stuff...


TO DO:
- Calibration function
- Clean up 

Parts List for CounterRotator:
- 2 Nema 17 motors
- 1 Arduino Nano
- 1 CNC Shield V3 
- 2 A4988 or 8825 stepper drivers
- 1 LCD 16x2
- 1 BNO055 sensors or another 9axis sensor 
- 8 10mm M6 bolts
- 4 50mm M6 bolts 
- 2 60mm M6 bolts and nuts
- 3 15mm M5 bolts and nuts
- 8 15mm M3 bolts 
- 8 10mm M3 bolts

3d printed part:
- Can be download form my thingiverse: (insert link)




Special thanks to the following concepts that I remix to use in my desing:
- Reduction Gearbox: https://www.thingiverse.com/thing:2054378
- Nema 17 bracket: https://www.thingiverse.com/thing:4561244
- LCD screen case: https://www.printables.com/model/97206-16x2-lcd-housing
- Arduino support: https://www.printables.com/model/43002-classic-arduino-mouning-plate
- Main gearbox: https://www.printables.com/model/898603-90deg-gear-box-model
 




###Satdump
Satdump is a general purpuse Satellite decoder it can be download and install form the developper github: https://github.com/SatDump/SatDump

###rotctld
Satdump can't communicate directly with the microcontroller; we need the Hamlib library "rotctld" to make the link between satdump and the microcontroller
Download and install rotctld 

## Installation and usage 
To get started with the project, follow these steps:

Clone the Repository
- git clone https://github.com/NicolasGagne/CounterRotator.git

Open the directory
- cd Rotator 

Open CounterRotator.ino with the Arduino IDE
Install the dependency in your IDE
Upload the file to your Arduino
Start rotctld use: rotctld -m 202 -r COM6 -s 115200 -T 127.0.0.1 -t 4533 -vvvvv (modified COM, IP and port as needed)
Start Satdump and configure the Rotator matching the above



Feel free to modify and expand this as needed to fit your specific project details. 
If you need any additional sections or information, let me know!