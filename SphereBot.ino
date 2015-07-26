/*
 * Copyright 2011 by Eberhard Rensch <http://pleasantsoftware.com/developer/3d>
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
 * Part of this code is based on/inspired by the Helium Frog Delta Robot Firmware
 * by Martin Price <http://www.HeliumFrog.com>
 *
 * Updated to run on Adafruit motor shield by Jin Choi <jsc@alum.mit.edu>.
 *
 * !!!!!!!!
 * This sketch needs the following non-standard library (install it in the Arduino library directory):

 * Adafruit Motor Shield: https://github.com/adafruit/Adafruit-Motor-Shield-library
 * !!!!!!!!
 */

/* Adafruit Motor Shield libraries */
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"

/* DualStepper */
#include "DualStepper.h"

/* Servo library */
#include <Servo.h>

#include <EEPROM.h>


/*
 * PINS
 */

#define PEN_AXIS_PORT 1
#define ROTATION_AXIS_PORT 2

#define SERVO_PIN 9

/*
 * Other Configuration
 */

/* Pen servo gets clamped to these values. */
#define MIN_PEN_POSITION 100
#define MAX_PEN_POSITION 130

/* Default pen up position. */
#define PEN_UP_POSITION 107

/* How long to take for pen down moves in ms. */
#define PEN_DOWN_MOVE_TIME 200


/* X axis gets clamped to these values to prevent inadvertent damage. */
#define MIN_PEN_AXIS_STEP -480
#define MAX_PEN_AXIS_STEP 480

/* Suitable for Eggbot template and 200 steps/rev steppers at 16x microstepping. */
#define DEFAULT_ZOOM_FACTOR 1.0

enum {
  VALUES_SAVED_EEPROM_LOCATION, MIN_PEN_EEPROM_LOCATION, MAX_PEN_EEPROM_LOCATION, PEN_UP_EEPROM_LOCATION
};

#define EEPROM_MAGIC_NUMBER 53

byte min_pen_position;
byte max_pen_position;
byte pen_up_position;
byte current_pen_position;

/* --------- */

/* Set up steppers */
Adafruit_MotorShield MS = Adafruit_MotorShield();
SingleStepper *xStepper = new SingleStepper(MS.getStepper(200, ROTATION_AXIS_PORT));
SingleStepper *yStepper = new SingleStepper(MS.getStepper(200, PEN_AXIS_PORT));
DualStepper *steppers = new DualStepper(xStepper, yStepper, 200 * MICROSTEPS);

Servo servo;

// comm variables
const int MAX_CMD_SIZE = 256;
char buffer[MAX_CMD_SIZE]; // buffer for serial commands
char serial_char; // value for each byte read in from serial comms
int serial_count = 0; // current length of command
char *strchr_pointer; // just a pointer to find chars in the cmd string like X, Y, Z, E, etc
boolean comment_mode = false;
// end comm variables

// GCode States
boolean absoluteMode = true;
double feedrate = 160.0; // steps/s
double zoom = DEFAULT_ZOOM_FACTOR;

// steps/s. A no-delay loop takes 1.29 ms per step, so this is the fastest we can go.
#define MAX_FEEDRATE 775.0

// ------

void load_pen_configuration()
{
  // Check EEPROM location 0 for presence of a magic number. If it's there, we have saved values.
  if (EEPROM.read(VALUES_SAVED_EEPROM_LOCATION) == EEPROM_MAGIC_NUMBER) {
    min_pen_position = EEPROM.read(MIN_PEN_EEPROM_LOCATION);
    max_pen_position = EEPROM.read(MAX_PEN_EEPROM_LOCATION);
    pen_up_position = EEPROM.read(PEN_UP_EEPROM_LOCATION);
  } else {
    min_pen_position = MIN_PEN_POSITION;
    max_pen_position = MAX_PEN_POSITION;
    pen_up_position = PEN_UP_POSITION;
  }
}

void save_pen_configuration() {
  EEPROM.update(MIN_PEN_EEPROM_LOCATION, min_pen_position);
  EEPROM.update(MAX_PEN_EEPROM_LOCATION, max_pen_position);
  EEPROM.update(PEN_UP_EEPROM_LOCATION, pen_up_position);
  EEPROM.update(VALUES_SAVED_EEPROM_LOCATION, EEPROM_MAGIC_NUMBER);
}

void clear_pen_configuration() {
  min_pen_position = MIN_PEN_POSITION;
  max_pen_position = MAX_PEN_POSITION;
  pen_up_position = PEN_UP_POSITION;
  EEPROM.update(VALUES_SAVED_EEPROM_LOCATION, 0xff);
}

void move_pen(byte pos) {
  if (pos > current_pen_position) {
    // ease it down
    int pen_delay = PEN_DOWN_MOVE_TIME / 10;
    float pen_increment = (pos - current_pen_position) / 10.0;
    for (int i = 1; i < 10; i++) { // loop takes it to one step less than full travel
      servo.write(current_pen_position + pen_increment * i);
      delay(pen_delay);
    }
    servo.write(pos); // Finish off exactly; no round off errors.
  } else {
    // slam it up
    servo.write(pos);
  }

  current_pen_position = pos;
}


void setup()
{
  load_pen_configuration();
  Serial.begin(115200);
  clear_buffer();

  MS.begin();
  TWBR = ((F_CPU / 400000L) - 16) / 2; // Change the i2c clock to 400KHz for faster stepping.
  Serial.println("Ready");

  steppers->setMaxSpeed(MAX_FEEDRATE);
    
  servo.attach(SERVO_PIN);
  servo.write(pen_up_position);
  current_pen_position = pen_up_position;

  delay(100);
}

void loop() // input loop, looks for manual input and then checks to see if and serial commands are coming in
{
  get_command(); // check for Gcodes
}

void get_command() // gets commands from serial connection and then calls up subsequent functions to deal with them
{
  if (Serial.available() > 0) // each time we see something
    {
      serial_char = Serial.read(); // read individual byte from serial connection
    
      if (serial_char == '\n' || serial_char == '\r') // end of a command character
	{ 
	  buffer[serial_count]=0;
	  process_commands(buffer, serial_count);
	  clear_buffer();
	  comment_mode = false; // reset comment mode before each new command is processed
	}
      else // not end of command
	{
	  if (serial_char == ';' || serial_char == '(') // semicolon signifies start of comment
	    {
	      comment_mode = true;
	    }
      
	  if (comment_mode != true) // ignore if a comment has started
	    {
	      buffer[serial_count] = serial_char; // add byte to buffer string
	      serial_count++;
	      if (serial_count > MAX_CMD_SIZE) // overflow, dump and restart
		{
		  clear_buffer();
		  Serial.flush();
		}
	    }
	}
    }
}

void clear_buffer() // empties command buffer from serial connection
{
  serial_count = 0; // reset buffer placement
}

boolean getValue(char key, char command[], double* value)
{  
  // find key parameter
  strchr_pointer = strchr(buffer, key);
  if (strchr_pointer != NULL) // We found a key value
    {
      *value = (double)strtod(&command[strchr_pointer - command + 1], NULL);
      return true;  
    }
  return false;
}

inline double clamp(double x, double a, double b)
{
  return x < a ? a : (x > b ? b : x);
}

void process_commands(char command[], int command_length) // deals with standardized input from serial connection
{
  if (command_length>0 && command[0] == 'G') // G code
    {
      int codenum = (int)strtod(&command[1], NULL);
    
      double tempX = steppers->xPos();
      double tempY = steppers->yPos();
    
      double xVal;
      boolean hasXVal = getValue('X', command, &xVal);
      if(hasXVal) xVal*=zoom;
      double yVal;
      boolean hasYVal = getValue('Y', command, &yVal);
      if(hasYVal) yVal*=zoom;
      double iVal;
      boolean hasIVal = getValue('I', command, &iVal);
      if(hasIVal) iVal*=zoom;
      double jVal;
      boolean hasJVal = getValue('J', command, &jVal);
      if(hasJVal) jVal*=zoom;
      double rVal;
      boolean hasRVal = getValue('R', command, &rVal);
      if(hasRVal) rVal*=zoom;
      double pVal;
      boolean hasPVal = getValue('P', command, &pVal);
    
      getValue('F', command, &feedrate);

      if(absoluteMode)
	{
	  if(hasXVal)
	    tempX=xVal;
	  if(hasYVal)
	    tempY=yVal;
	}
      else
	{
	  if(hasXVal)
	    tempX+=xVal;
	  if(hasYVal)
	    tempY+=yVal;
	}

      tempY = clamp(tempY, MIN_PEN_AXIS_STEP, MAX_PEN_AXIS_STEP);

      switch(codenum)
	{
	case 0: // G0, Rapid positioning
	  steppers->moveTo(tempX, tempY, MAX_FEEDRATE);
	  break;
	case 1: // G1, linear interpolation at specified speed
	  if (current_pen_position <= pen_up_position) {
	    // potentially wrap around the sphere if pen is up
	    steppers->travelTo(tempX, tempY, feedrate);
	  } else {
	    steppers->moveTo(tempX, tempY, feedrate);
	  }
	  break;
	case 2: // G2, Clockwise arc
	case 3: // G3, Counterclockwise arc
	  if(hasIVal && hasJVal)
	    {
	      double centerX=steppers->xPos()+iVal;
	      double centerY=steppers->yPos()+jVal;
	      drawArc(centerX, centerY, tempX, tempY, (codenum==2));
	    }
	  else if(hasRVal)
	    {
	      //drawRadius(tempX, tempY, rVal, (codenum==2));
	    }
	  break;
	case 4: // G4, Delay P ms
	  if(hasPVal)
	    {
	      delay(pVal);
	    }
	  break;
	case 90: // G90, Absolute Positioning
	  absoluteMode = true;
	  break;
	case 91: // G91, Incremental Positioning
	  absoluteMode = false;
	  break;
	}
    }  
  else if (command_length>0 && command[0] == 'M') // M code
    {
      double value;
      int codenum = (int)strtod(&command[1], NULL);
      switch(codenum) {
      case 18: // Disable Drives
	xStepper->release();
	yStepper->release();
	break;

      case 300: // Servo Position
	if(getValue('S', command, &value))
	  {
	    if (value > 180)
	      value = pen_up_position;
	    value = clamp(value, min_pen_position, max_pen_position);
	    move_pen(value);
	  }
	break;

      case 301: // Set min pen position.
	if (getValue('P', command, &value)) {
	  min_pen_position = value;
	}
	break;

      case 302: // Set max pen position.
	if (getValue('P', command, &value)) {
	  max_pen_position = value;
	}
	break;

      case 303: // set default pen up position.
	if (getValue('P', command, &value)) {
	  pen_up_position = value;
	}
	break;
        
      case 402: // Propretary: Set global zoom factor
	if(getValue('S', command, &value)) {
	  zoom = value;
	}
	break;

      case 500:
	save_pen_configuration();
	break;

      case 501:
	load_pen_configuration();
	break;

      case 502:
	clear_pen_configuration();
	break;
      }
    }

  // done processing commands
  if (Serial.available() <= 0) {
    Serial.print("ok:");
    Serial.println(command);
  }
}

/* This code was ported from the Makerbot/ReplicatorG java sources */
void drawArc(double centerX, double centerY, double endpointX, double endpointY, boolean clockwise) 
{
  // angle variables.
  double angleA;
  double angleB;
  double angle;
  double radius;
  double length;

  // delta variables.
  double aX;
  double aY;
  double bX;
  double bY;

  // figure out our deltas
  double currentX = steppers->xPos();
  double currentY = steppers->yPos();
  aX = currentX - centerX;
  aY = currentY - centerY;
  bX = endpointX - centerX;
  bY = endpointY - centerY;

  // Clockwise
  if (clockwise) {
    angleA = atan2(bY, bX);
    angleB = atan2(aY, aX);
  }
  // Counterclockwise
  else {
    angleA = atan2(aY, aX);
    angleB = atan2(bY, bX);
  }

  // Make sure angleB is always greater than angleA
  // and if not add 2PI so that it is (this also takes
  // care of the special case of angleA == angleB,
  // ie we want a complete circle)
  if (angleB <= angleA)
    angleB += 2. * M_PI;
  angle = angleB - angleA;
		
  // calculate a couple useful things.
  radius = sqrt(aX * aX + aY * aY);
  length = radius * angle;

  // for doing the actual move.
  int steps;
  int s;
  int step;

  // Maximum of either 2.4 times the angle in radians
  // or the length of the curve divided by the curve section constant
  steps = (int)ceil(max(angle * 2.4, length));

  // this is the real draw action.
  double newPointX = 0.;
  double newPointY = 0.;
  
  for (s = 1; s <= steps; s++) {
    // Forwards for CCW, backwards for CW
    if (!clockwise)
      step = s;
    else
      step = steps - s;

    // calculate our waypoint.
    newPointX = centerX + radius * cos(angleA + angle * ((double) step / steps));
    newPointY= centerY + radius	* sin(angleA + angle * ((double) step / steps));

    // start the move
    steppers->moveTo(newPointX, newPointY, feedrate);
  }
}
