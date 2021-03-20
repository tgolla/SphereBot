/*
 * Copyright 2016 by GrAndAG
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Updated 3/2021 by Terence Golla
 */

#ifndef Configuration_h
#define Configuration_h

// Added debugging (M999) command.
#define DEBUG true

// Set to true if you are using an Adafruit 2.8" TFT Touch Shield for 
// Arduino w/Capacitive Touch with an Arduino Mega 2560 Note: A Mega
// 2560 is required due to the memory requirements.  
#define ADAFRUIT_TFT_TOUCH_SHIELD false

// Adafruit 2.8" TFT Touch Shield for Arduino w/Capacitive Touch
// The display uses hardware SPI, plus #9 & #10.
#define TFT_CS 10
#define TFT_DC 9
#define FT6206_THRESSHOLD 0x80

// Splash screen display delay in milliseconds.
#define SPLASH_SCREEN_DELAY 7000

// SD Chip Select Pin
#define SD_CS 4


// Version of Adafruit motor shield used.  One of the two lines should be commented out.

//#define ADAFRUIT_MOTOR_SHIELD_VERSION 1
#define ADAFRUIT_MOTOR_SHIELD_VERSION 2

/*
 * Adafruit motor sheild stepper port and servo pin definitions.
 * 
 * Pen arm motor terminals : M1 & M2 stepper number port 1 (PEN_AXIS_PORT).
 * Rotation motor terminals : M3 & M4 stepper number port 2 (ROTATION_AXIS_PORT).
 * Note: M1 and M3 should have the same colored pairs of wires, as should M2 and M4.
 * 
 * Pen Arm servo port : Servo 1, located closer to PCB corner. SER1 on V1 board. Pin 9 
 * If you are adding the Adafruit 2.8" TFT Touch Shield for Arduino w/Capacitive Touch 
 * board you will need to solder a 3 pin right-angle male header to the break out area
 * of the motor shield facing the edge with pin 1 wired to ground, pin 2 wired to 5V and
 * pin 3 wired to pin 6 of the Arduino. Note: In this case you will define SERVO_PIN
 * as 6, not 9.
 */

#define PEN_AXIS_PORT 1
#define ROTATION_AXIS_PORT 2

#define SERVO_PIN 6
#define REVERSE_SERVO true


// Steppers Configuration

// Full steps per revolution. Well known NEMA17 1.8 degrees motors have 200 steps.
#define STEPS_PER_REVOLUTION  200

// Suitable for Eggbot template and 200 steps/rev steppers at 16x microstepping. */
#define DEFAULT_ZOOM_FACTOR 1.0

// The default XY feedrate in steps/second. Necessary should G-Code not contain feedrate.
#define DEFAULT_XY_FEEDRATE 400.0

// The default preset XY feedrate in steps/second. Use preset feedrate if not zero.
#define DEFAULT_PRESET_XY_FEEDRATE 0.0


// Pen Arm Configuration. You will want to fine tune these settings by manually sending M300 codes.

// Pen servo gets clamped to these values.
#define MIN_PEN_POSITION 100
#define MAX_PEN_POSITION 160

// Default pen up/down positions.
#define DEFAULT_PEN_UP_POSITION 145
#define DEFAULT_PEN_DOWN_POSITION 115

// The default Pen feedrate in degrees/second.  Necessary should G-Code not contain feedrate.
#define DEFAULT_PEN_FEEDRATE 200.0

// The default preset Pen feedrate in degrees/second. Use perset feedrate if not zero.
#define DEFAULT_PRESET_PEN_FEEDRATE 0.0

// Default settings of M and Z code modes to allow for increased 
// flexablity controlling the pen servo. 

// MZ Mode
// 0 - M mode allows for normal pen control using M300 commands.
// 1 - Z mode allows for pen control using G0, G1, G2 & G3 Z code parameter.
// 2 - Auto mode scans the file on the SD to automatically set the active MZ mode 
//     to either M or Z mode. When using the serial USB port the active mode 
//     defaults to M mode.
#define DEFAULT_MZ_MODE 0

// The M Adjusted mode allows for the M300 S code parameter to be adjusted.  This 
// allows the for the use of G-Code files that have been calibrated to use on other SphereBots.
// 0 - Off does not adjust the S code value.
// 1 - Preset defines the S value at which all S values at or above are adjusted to the pen up
//     setting. All values below the preset value are adjusted to the pen down seting.
// 2 - Calculated determines the S value at which all S values at or above are adjusted to the
//     pen up setting by taking the first M300 command S values. All values below the calculated
//     value are adjusted to the pen down seting.
#define DEFAULT_M_ADJUST 0

// The Z Adjusted mode allows for the G0, G1, G2 & G3 Z code parameter to be adjusted.  
// This allows for the use of G-Code files that have been calibrated to use on other SphereBots.
// 0 - Off does not adjust the Z code value.
// 1 - Preset defines the Z value at which all Z values at or above are adjusted to the pen up
//     setting. All values below the preset value are adjusted to the pen down seting.
// 2 - Calculated determines the Z value at which all Z values at or above are adjusted to the
//     pen up setting by taking the first G0, G1, G2 or G3 (usually G0) command's Z values. All
//     values below the calculated  value are adjusted to the pen down seting. 
#define DEFAULT_Z_ADJUST 0

// If in serial USB mode the G-Code cannot be preprocessed (scanned) for maximun and minimun 
// values to set the absolute or average adjustment threshold. In this case the following 
// default is used for M300 commands. The value can also be set with the M308 command.
#define DEFAULT_M_ADJUST_PRESET 145

// If in serial USB mode the G-Code cannot be preprocessed (scanned) for maximun and minimun 
// values to set the absolute or average adjustment threshold. In this case the following 
// default is used for G0, G1, G2, & G3 Z parameters. The value can also be set with the M309 command.
#define DEFAULT_Z_ADJUST_PRESET 5


// X axis gets clamped to these values to prevent inadvertent damage.
// Most drawings are 800 (-400 and 400) by 3200.
#define MIN_PEN_AXIS_STEP -480
#define MAX_PEN_AXIS_STEP 480

// Version dependent configurations.
#if ADAFRUIT_MOTOR_SHIELD_VERSION == 1

  #include <AFMotor.h>

  #define ADAFRUIT_CLASS      AF_Stepper
  #define ONE_STEP_TIME 168
  // steps/s. A no-delay loop takes 0.17 ms per step, so this is the fastest we can go.
  #define MAX_XY_FEEDRATE 2900.0

#else

  #include <Adafruit_MotorShield.h>
  //#include <utility/Adafruit_MS_PWMServoDriver.h>
  
  #define ADAFRUIT_CLASS      Adafruit_StepperMotor
  #define ONE_STEP_TIME 1290
  // steps/s. A no-delay loop takes 1.29 ms per step, so this is the fastest we can go.
  #define MAX_XY_FEEDRATE 775.0

#endif

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#endif /* Configuration_h */