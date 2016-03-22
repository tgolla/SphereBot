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
 */
 

#ifndef Configuration_h
#define Configuration_h

/*
* Version of Adafruit motor shield used
*/

//#define ADAFRUIT_MOTOR_SHIELD_VERSION 1
#define ADAFRUIT_MOTOR_SHIELD_VERSION 2


/*
* PINS definitions
* Pen arm motor terminals  : M1 & M2
* Rotation motor terminals : M3 & M4
* Arm servo port           : SER1 (located closer to PCB corner
 */

#define PEN_AXIS_PORT         1
#define ROTATION_AXIS_PORT    2

#define SERVO_PIN             10

/*
* Steppers Configuration
*/

/* Full steps per revolution. Well known NEMA17 1.8 degrees motors have 200 steps. */
#define STEPS_PER_REVOLUTION  200

/* Suitable for Eggbot template and 200 steps/rev steppers at 16x microstepping. */
#define DEFAULT_ZOOM_FACTOR   1.0

/*
 * Pen Arm Configuration
 */

/* Pen servo gets clamped to these values. */
#define MIN_PEN_POSITION      100
#define MAX_PEN_POSITION      130

/* Default pen up position. */
#define PEN_UP_POSITION       107

/* How long to take for pen down moves in ms. */
#define PEN_DOWN_MOVE_TIME    200

/* X axis gets clamped to these values to prevent inadvertent damage. */
#define MIN_PEN_AXIS_STEP    -480
#define MAX_PEN_AXIS_STEP     480



/* Version dependent stuff */
#if ADAFRUIT_MOTOR_SHIELD_VERSION == 1

  #include <AFMotor.h>

  #define ADAFRUIT_CLASS      AF_Stepper
  #define ONE_STEP_TIME       168
  // steps/s. A no-delay loop takes 0.17 ms per step, so this is the fastest we can go.
  #define MAX_FEEDRATE        2900.0

#else

  #include <Adafruit_MotorShield.h>
  //#include <utility/Adafruit_MS_PWMServoDriver.h>
  
  #define ADAFRUIT_CLASS      Adafruit_StepperMotor
  #define ONE_STEP_TIME       1290
  // steps/s. A no-delay loop takes 1.29 ms per step, so this is the fastest we can go.
  #define MAX_FEEDRATE        775.0

#endif


#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#endif /* Configuration_h */
