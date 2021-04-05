# SphereBot, an EggBot clone.
## tgolla/SphereBot Fork ReadMe
### https://github.com/tgolla/SphereBot
Arduino Firmware for use with the Adafruit Motor Shield v2 and 2.8" TFT Touch Shield for Arduino w/Capacitive Touch.

This fork takes on a large number of changes including adding additional custom G-Code commands, a touchscreen control, SD card reader support and audio alerts. Many of these features are being actively developed such that additional documentation can be found in the G-Code.md file and comments in both the SphereBot.ino and configuration.h files.

Please do not fork this code just to have a copy unless you plan on making meaningful contributions which could be merged back in as GitHub forks are overused, abused, and misused.  Forks are not meant to be used to keep a copy of the code in your personal repository.  More information on the proper use of forks can be found at…

https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/about-forks
https://docs.github.com/en/github/getting-started-with-github/fork-a-repo

Instead clone the project and create a personal branch in which you can store your modification to configuration.h.  In this way you can pull updates to master and merge them into your branch without losing your specific configuration.

I ask this because this project currently has 52 forks of which maybe 5 are not exact copies of the orginal project.

# jinschoi/SphereBot Fork README
### https://github.com/jinschoi/SphereBot
This is a fork of Eberhard Rensch's SphereBot firmware. This version has been modified to work with the Adafruit motor shield by Jin Choi <jsc@alum.mit.edu>. Some changes have been made:

* The units are changed from nominal mm to microsteps. If you are using a 200 steps/revolution stepper, G1 Y3200 will rotate the rotation axis one full turn (using 16x microstepping). This makes adjusting for object diameter unnecessary.
* The axes convention is swapped, so that the X axis is the rotation axis and the Y axis the pen axis. This was done to make it easier to adopt patterns using the Eggbot templates, which are 3200 px wide by 800 px tall.
* Pen axis values are clamped to MIN_PEN_AXIS_STEP and MAX_PEN_AXIS_STEP constants, to prevent damage from overrotation. Change these values as appropriate to your setup.
* Servo values are likewise clamped to min_pen_position and max_pen_position variables. These are initialized as constants in the code, and can be set using "M301 Pxxx" and "M302 Pxxx" respectively. The default pen up position, used at startup, is set to pen_up_position, and likewise settable using "M303 Pxxx". "M500" saves values to EEPROM, and "M502" restores values to firmware defaults. "M501" will reload limit values from EEPROM. These should be adjusted as necessary for your setup.
* Servo values are tracked. Pen down moves occur in increments with a delay to make them more gentle (edit PEN_DOWN_MOVE_TIME to modify speed). X travel moves will wrap around if it is determined that the pen is up and that would be faster.
* Some g-code seems to use "M300 S255" to indicate that the servo be disabled. This is not possible without being able to control the power lines to the servo once a control signal has been sent. Any value over 180 will move the pen to the default pen up position.
* Stepper control is handled by a class that implements Bresenham's line algorithm and calls to Adafruit_StepperMotor to do the steps. Maximum speed possible is about 265 steps/s, or about 30 degrees/s.

**IMPORTANT**: This sketch needs the following non-standard library (install them in the Arduino library directory):

* Adafruit Motor Shield v1: https://github.com/adafruit/Adafruit-Motor-Shield-library
  OR
* Adafruit Motor Shield v2: https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library

To use:

Plug in the stepper motors and servo to the motor shield. Use M1/M2 for the pen motor, and M3/M4 for the rotation motor. M1 and M3 should have the same colored pairs of wires, as should M2 and M4, in order to have everything turn in the correct directions. The servo should go into the Servo 2 port with the brown wire towards the edge of the board. If you do things differently, edit PEN_AXIS_PORT, ROTATION_AXIS_PORT, and SERVO_PIN appropriately.

Define MIN_PEN_POSITION, MAX_PEN_POSITION, and PEN_UP_POSITION appropriately. The servo gets clamped to the min and max values, and is initialized to the pen up position on startup. You can use M300 Sxxx to experiment with servo positions, and M301, M302, and M303 to set the min, max, and default pen up positions dynamically. Use "P" parameters with M301-303. Issue an M500 to save the values.

The actual pen up/pen down servo positions used during printing will be sent in the gcode. If the gcode pen up position is the same as the firmware pen_up_position, an optimization will be made where a pen up travel move that requires more than half a full rotation in the X axis will take the short way around. The pen up position should be just high enough to clear an egg, and the pen down position low enough to firmly contact the egg, but you don't want the servo to move too much or the pen momentum can break the shell.

Set MIN_PEN_AXIS_STEP and MAX_PEN_AXIS_STEP. -480 and 480 should be fine; Eggbot patterns only use equivalent X values between -400 and 400.

I suggest using the [InkScape Unicorn Plugin](https://github.com/martymcguire/inkscape-unicorn) to generate the g-code. The maximum possible feedrate is ~265, because the servo loop will not step any faster.

A simple g-code sender script is available in Utils. It traps "M01" to allow pauses between layers.

# Original README
## zaggo/SphereBot  
### https://github.com/zaggo/SphereBot

This is the firmware for an EggBot-style SphereBot.
The firmware directly interprets GCode sent over the serial port.

There will be more information on the SphereBot at http://pleasantsoftware.com/developer/3d (in the near future...)

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
