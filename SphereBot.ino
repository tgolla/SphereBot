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
 * This sketch needs the following non-standard libraries (install them in the Arduino library directory):

 * AccelStepper: https://github.com/jinschoi/AccelStepper
 * Adafruit Motor Shield: https://github.com/adafruit/Adafruit-Motor-Shield-library
 * !!!!!!!!
 */

/* Adafruit Motor Shield libraries */
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"

/* AccelStepper */
#include <AccelStepper.h>

/* Servo library */
#include <Servo.h>


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
#define PEN_UP_POSITION 115
#define PEN_DOWN_POSITION 125

/* X axis gets clamped to these values to prevent inadvertent damage. */
int minPenAxisStep = -30;
int maxPenAxisStep = 30;

/* Suitable for Eggbot template and 200 steps/rev steppers */
#define DEFAULT_ZOOM_FACTOR 0.0625

#define STEP_MODE MICROSTEP


/* --------- */

/* Set up steppers */
Adafruit_MotorShield MS = Adafruit_MotorShield();
Adafruit_StepperMotor *xStepper = MS.getStepper(200, ROTATION_AXIS_PORT);
Adafruit_StepperMotor *yStepper = MS.getStepper(200, PEN_AXIS_PORT);

void forwardStepRotation() {
  xStepper->onestep(FORWARD, STEP_MODE);
}

void backwardStepRotation() {
  xStepper->onestep(BACKWARD, STEP_MODE);
}

void forwardStepPenAxis() {
  yStepper->onestep(FORWARD, STEP_MODE);
}

void backwardStepPenAxis() {
  yStepper->onestep(BACKWARD, STEP_MODE);
}


AccelStepper penAxisStepper(forwardStepPenAxis, backwardStepPenAxis);
AccelStepper rotationStepper(forwardStepRotation, backwardStepRotation);
  
Servo servo;

boolean isRunning=false;

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
double feedrate = 10.0; // full steps/s
double zoom = DEFAULT_ZOOM_FACTOR;

// full steps/s
#define MAX_FEEDRATE 20.0

// ------

/* Microstepping requires that we call onestep() MICROSTEPS times for
   every full step.  We will handle this by dealing with all
   coordinates and feed rates as full steps and multiplying them all
   up by the appropriate factor whenever we interact with the stepper
   library.

   INTERLEAVE mode seems to act like 2x microstepping. */
inline double convertSteps(double fullSteps)
{
  switch (STEP_MODE) {
  case INTERLEAVE:
    return fullSteps * 2;
  case MICROSTEP:
    return fullSteps * MICROSTEPS;
  default:
    return fullSteps;
  }
}
  

void setup()
{
    Serial.begin(115200);
    clear_buffer();

    MS.begin();
    Serial.println("Ready");

    double maxFeedrate = convertSteps(MAX_FEEDRATE);

    penAxisStepper.setMaxSpeed(maxFeedrate);
    rotationStepper.setMaxSpeed(maxFeedrate);
    
    servo.attach(SERVO_PIN);
    servo.write(PEN_UP_POSITION);
    delay(100);

    minPenAxisStep = convertSteps(minPenAxisStep);
    maxPenAxisStep = convertSteps(maxPenAxisStep);
}

void loop() // input loop, looks for manual input and then checks to see if and serial commands are coming in
{
  get_command(); // check for Gcodes
  if (isRunning) {
    if (penAxisStepper.distanceToGo() == 0 && rotationStepper.distanceToGo() == 0) {
      isRunning = false;
    } else {
      penAxisStepper.runSpeedToPosition();
      rotationStepper.runSpeedToPosition();
    }
  }
}

/* All arguments must be in converted partial step units:
   double for INTERLEAVE, or multiplied by MICROSTEPS for MICROSTEP mode. */
void commitSteppers(long x, long y, double speedrate)
{
  long deltaStepsX = x - rotationStepper.currentPosition();
  long deltaStepsY = y - penAxisStepper.currentPosition();

  // how long is our line length?
  double distance = sqrt(deltaStepsX*deltaStepsX+deltaStepsY*deltaStepsY);

  // go there.
  rotationStepper.moveTo(x);
  penAxisStepper.moveTo(y);

  // compute speed for this move. Must call setSpeed after moveTo for constant speed
  // moves (can't use acceleration because the axes won't coordinate).
  rotationStepper.setSpeed(deltaStepsX * speedrate / distance);
  penAxisStepper.setSpeed(deltaStepsY * speedrate / distance);

  isRunning=true;
}

void get_command() // gets commands from serial connection and then calls up subsequent functions to deal with them
{
  if (!isRunning && Serial.available() > 0) // each time we see something
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
    
    double tempX = rotationStepper.currentPosition();
    double tempY = penAxisStepper.currentPosition();
    
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
        tempX=convertSteps(xVal);
      if(hasYVal)
        tempY=convertSteps(yVal);
    }
    else
    {
      if(hasXVal)
        tempX+=convertSteps(xVal);
      if(hasYVal)
        tempY+=convertSteps(yVal);
    }

    tempY = clamp(tempY, minPenAxisStep, maxPenAxisStep);

    switch(codenum)
    {
      case 0: // G0, Rapid positioning
        commitSteppers(tempX, tempY, convertSteps(MAX_FEEDRATE));
        break;
      case 1: // G1, linear interpolation at specified speed
        commitSteppers(tempX, tempY, convertSteps(feedrate));
        break;
      case 2: // G2, Clockwise arc
      case 3: // G3, Counterclockwise arc
        if(hasIVal && hasJVal)
        {
	  double centerX=rotationStepper.currentPosition()+convertSteps(iVal);
	  double centerY=penAxisStepper.currentPosition()+convertSteps(jVal);
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
    switch(codenum)
    {   
      case 18: // Disable Drives
	xStepper->release();
	yStepper->release();
        break;

      case 300: // Servo Position
        if(getValue('S', command, &value))
        {
          servo.attach(SERVO_PIN);
	  value = clamp(value, PEN_UP_POSITION, PEN_DOWN_POSITION); 

	  servo.write((int)value);
        }
        break;
        
       case 402: // Propretary: Set global zoom factor
        if(getValue('S', command, &value))
        {
          zoom = value;
        }

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
  double currentX = rotationStepper.currentPosition();
  double currentY = penAxisStepper.currentPosition();
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
    commitSteppers(newPointX, newPointY, convertSteps(feedrate));
    
    while(isRunning)
    {
      if (penAxisStepper.distanceToGo() == 0 && rotationStepper.distanceToGo() == 0) {
	isRunning = false;
      } else {
	penAxisStepper.runSpeedToPosition();
	rotationStepper.runSpeedToPosition();
      }
    }
  }
}

/* Make life easier for vim users */
/* vim:set filetype=cpp: */
