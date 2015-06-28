This is the firmware for an EggBot-style SphereBot.
The firmware directly interprets GCode sent over the serial port.

There will be more information on the SphereBot at http://pleasantsoftware.com/developer/3d (in the near future...)


This version has been modified to work with the Adafruit motor shield by Jin Choi <jsc@alum.mit.edu>. Some changes have been made:

* The units are changed from nominal mm to full steps. If you are using a 200 steps/revolution stepper, G1 Y200 will rotate the rotation axis one full turn. This makes adjusting for object diameter unnecessary. (However, note below that the default zoom factor has been set to 1/16 for compatibility with Eggbot files. This affects X/Y coordinates but not feed rates.)
* Microstepping is implemented by multiplying all steps and feed rates internally by MICROSTEPS (usually 16). This makes for very smooth motion, but reduces maximum movement rates. If a too-rapid movement is attempted, the other stepper will be starved and move jerkily. If you want faster travel, you can set STEP_MODE to SINGLE, DOUBLE, or INTERLEAVE.
* The axes convention is swapped, so that the X axis is the rotation axis and the Y axis the pen axis. This was done to make it easier to adopt patterns using the Eggbot templates, which are 3200 px wide.
* Pen axis values are clamped to minPenAxisStep and maxPenAxisStep variables, to prevent damage from overrotation. Change these values as appropriate to your setup.
* Servo values are likewise clamped to PEN_UP_POSITION and PEN_DOWN_POSITION. These should be adjusted as necessary for your setup.
* Some g-code seems to use "M300 S255" to indicate that the servo be disabled. This has not been implemented, as I do not believe it is possible without being able to control power to the servo. Remove any such line as it will drive the pen down instead of disabling it.

**IMPORTANT**: This sketch needs the following non-standard libraries (install them in the Arduino library directory):

* AccelStepper: https://github.com/jinschoi/AccelStepper
* Adafruit Motor Shield: https://github.com/adafruit/Adafruit-Motor-Shield-library

You *must* use this version of AccelStepper, as modifications have been made to allow multiple steppers to coordinate properly.

To use:

Plug in the stepper motors and servo to the motor shield. Use M1/M2 for the pen motor, and M3/M4 for the rotation motor. M1 and M3 should have the same colored pairs of wires, as should M2 and M4, in order to have everything turn in the correct directions. The servo should go into the Servo 2 port with the brown wire towards the edge of the board. If you do things differently, edit PEN_AXIS_PORT, ROTATION_AXIS_PORT, and SERVO_PIN appropriately.

Define PEN_UP_POSITION and PEN_DOWN_POSITION appropriately. The servo gets clamped to these values, so to determine the correct values, set them to 0 and 180, then without the pen arm spring connected, send it values using "M300 Sxxx" to figure out what you should be using. These are safety values, you can set them to 0 and 180, and rely on your G-code to move them appropriately.

Set minPenAxisStep and maxPenAxisStep. -30 and 30 should be fine; Eggbot patterns only use equivalent X values between -25 and 25.

The DEFAULT_ZOOM_FACTOR is set to 0.0625 in order to be able to make use of g-code intended for the Eggbot without modification. Eggbot appears to use 3200x800 as the drawing field, whereas the native units are full steps. With 200 steps/revolution motors, that would give us a 1/16 zoom factor.

I suggest using the [InkScape Unicorn Plugin](https://github.com/martymcguire/inkscape-unicorn) to generate the g-code. Because the usable movement rates are so low (recommended 10-20 steps/s), you will want to edit unicorn.inx and modify the minimum value for the "xy-feedrate" parameter to 1.0 from 100.0. Once you have your g-code generated, you will want to remove the "M300 S255" line at the end, as I have not implemented turning off the servo and that will just drive the pen down.

A simple g-code sender script is available in Utils.


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
