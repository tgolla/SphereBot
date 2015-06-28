This is the firmware for an EggBot-style SphereBot.
The firmware directly interprets GCode send over the serial port.

There will be more information on the SphereBot at http://pleasantsoftware.com/developer/3d (in the near future...)


This version has been modified to work with the Adafruit motor shield by Jin Choi <jsc@alum.mit.edu>. Some changes have been made:

* The units are changed from nominal mm to full steps. If you are using a 200 steps/revolution stepper, G1 Y200 will rotate the rotation axis one full turn. This makes adjusting for object diameter unnecessary.
* Microstepping is implemented by multiplying all steps and feed rates by MICROSTEPS (usually 16). This makes for very smooth motion, but reduces maximum movement rates. If a too-rapid movement is attempted, the other stepper will be starved and move jerkily.
* The axes convention is swapped, so that the X axis is the rotation axis and the Y axis the pen axis. This was done to make it easier to adopt patterns using the Eggbot templates, which are 3200 px wide. Adjust zoom by dividing your stepper's steps/revolution by 3200 (e.g., 1/16 for a 200 steps/revolution stepper).
* Pen axis values are clamped to minPenAxisStep and maxPenAxisStep variables, to prevent damage from overrotation. Change these values as appropriate to your setup.
* Servo values are likewise clamped to PEN_UP_POSITION and PEN_DOWN_POSITION. These should be adjusted as necessary for your servo.

**IMPORTANT**: This sketch needs the following non-standard libraries (install them in the Arduino library directory):

* AccelStepper: https://github.com/jinschoi/AccelStepper
* Adafruit Motor Shield: https://github.com/adafruit/Adafruit-Motor-Shield-library

You *must* use this version of AccelStepper, as modifications have been made to allow multiple steppers to coordinate properly.


Copyright 2011 by Eberhard Rensch <http://pleasantsoftware.com/developer/3d>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>

Part of this code is based on/inspired by the Helium Frog Delta Robot Firmware
by Martin Price <http://www.HeliumFrog.com>

To create tags File: ctags --langmap="C++:+.pde" S*


GCode commands:

G90	Absolut modus
G91	Icremental modus:

M300S0	Servo 0 degree
M300S90	Servo 90 degree


M18	Stepper off

G0X0Y40	Rapid movement (pen 0mm, rotation 40mm)
G1X40Y0 Slower movement (pen 40mm, rotation 0mm)
G1Y10F660 Movement with feed 660mm/s (rotation 10mm)

;	Comment
( .. )	Comment
