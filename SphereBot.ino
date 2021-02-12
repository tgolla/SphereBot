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
 * 2015-2016 the code was modified to run on an Adafruit Motor Shield V2 by Jin Choi <jsc@alum.mit.edu>.
 * 2016 code support for the Adafruit Motor Shield V1 was added by GrAndAG.
 * 
 * 2/2021 the code was modified with the following improvements by Terence Golla (tgolla).
 *  
 *   - Corrected the naming convention mix of snake_case and camelCase for more prominent C/C++ Arduino 
 *     software use of camelCase.
 * 
 *   - Restructured the ino software file to follow Arduino styling...
 *       - Begin with a set of comments clearly describing the purpose and assumptions of the code.
 *       - Import any required library headers.
 *       - Define constants.
 *       - Define global variables.
 *       - Define setup() and loop().
 *       - Define subroutines (functions).
 * 
 *   - Expanded in code documentation (comments).
 *
 *   - Modified code to operate servo installed in reverse.
 * 
 * This sketch needs the following non-standard library (install it in the Arduino library directory):
 *
 * Adafruit Motor Shield (select appropriate version ):
 *   v1: https://github.com/adafruit/Adafruit-Motor-Shield-library
 *   v2: https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library
 *
 * Be sure to review and make appropriate modification to global constants in the "Configuration.h" file.
 *
 */

#include "Configuration.h"
#include <Wire.h>

/* DualStepper */
#include "DualStepper.h"

/* Servo library */
#include <Servo.h>

#include <EEPROM.h>

// Enum used to assign EEPROM memory locations for pen setting.  i.e. 0, 1, 2, 3
enum
{
  VALUES_SAVED_EEPROM_LOCATION,
  MIN_PEN_EEPROM_LOCATION,
  MAX_PEN_EEPROM_LOCATION,
  PEN_UP_EEPROM_LOCATION
};

// Number expected to be found in EEPROM memory location 0 that indicates pen setting have been stored in memory.
#define EEPROM_MAGIC_NUMBER 53

byte minPenPosition;
byte maxPenPosition;
byte penUpPosition;
byte currentPenPosition;

// Set up stepper motors and servo.
#if ADAFRUIT_MOTOR_SHIELD_VERSION == 1
SingleStepper *xStepper = new SingleStepper(new AF_Stepper(STEPS_PER_REVOLUTION, ROTATION_AXIS_PORT));
SingleStepper *yStepper = new SingleStepper(new AF_Stepper(STEPS_PER_REVOLUTION, PEN_AXIS_PORT));
#else
Adafruit_MotorShield MS = Adafruit_MotorShield();
SingleStepper *xStepper = new SingleStepper(MS.getStepper(STEPS_PER_REVOLUTION, ROTATION_AXIS_PORT));
SingleStepper *yStepper = new SingleStepper(MS.getStepper(STEPS_PER_REVOLUTION, PEN_AXIS_PORT));
#endif

DualStepper *steppers = new DualStepper(xStepper, yStepper, STEPS_PER_REVOLUTION *MICROSTEPS);

Servo servo;

// Serial communication variables.
const int MAX_CMD_SIZE = 256; // Maximun buffer size.
char buffer[MAX_CMD_SIZE]; // Buffer for serial commands.
char serialChar; // Value for each byte read in from serial port.
int serialCount = 0; // Current length of command.
char *strCharPointer; // Just a pointer to find chars in the command string like X, Y, Z, E, etc.
boolean commentMode = false; // Flag indicating comment.

// GCode states.
boolean absoluteMode = true;
double feedrate = 160.0; // steps/second
double zoom = DEFAULT_ZOOM_FACTOR;

void setup()
{
  loadPenConfiguration();
  Serial.begin(115200);
  clearBuffer();

#if ADAFRUIT_MOTOR_SHIELD_VERSION == 2
  MS.begin();
  TWBR = ((F_CPU / 400000L) - 16) / 2; // Change the i2c clock to 400KHz for faster stepping.
#endif
  Serial.println("Ready");

  steppers->setMaxSpeed(MAX_FEEDRATE);

  servo.attach(SERVO_PIN);
  servoWrite(penUpPosition);
  currentPenPosition = penUpPosition;

  delay(100);
}

// Input loop, looks for manual input and then checks to see if and serial commands are coming in.
void loop() 
{
  getCommand(); // Check for Gcodes.
}

void loadPenConfiguration()
{
  // Check EEPROM location 0 for presence of a magic number. If it's there, we have saved pen settings.
  if (EEPROM.read(VALUES_SAVED_EEPROM_LOCATION) == EEPROM_MAGIC_NUMBER)
  {
    minPenPosition = EEPROM.read(MIN_PEN_EEPROM_LOCATION);
    maxPenPosition = EEPROM.read(MAX_PEN_EEPROM_LOCATION);
    penUpPosition = EEPROM.read(PEN_UP_EEPROM_LOCATION);
  }
  else
  {
    minPenPosition = MIN_PEN_POSITION;
    maxPenPosition = MAX_PEN_POSITION;
    penUpPosition = PEN_UP_POSITION;
  }
}

void savePenConfiguration()
{
  EEPROM.update(MIN_PEN_EEPROM_LOCATION, minPenPosition);
  EEPROM.update(MAX_PEN_EEPROM_LOCATION, maxPenPosition);
  EEPROM.update(PEN_UP_EEPROM_LOCATION, penUpPosition);
  EEPROM.update(VALUES_SAVED_EEPROM_LOCATION, EEPROM_MAGIC_NUMBER);
}

void clearPenConfiguration()
{
  minPenPosition = MIN_PEN_POSITION;
  maxPenPosition = MAX_PEN_POSITION;
  penUpPosition = PEN_UP_POSITION;
  EEPROM.update(VALUES_SAVED_EEPROM_LOCATION, 0xff);
}

// Corrects servo value if servo has been install reversed.
// Set REVERSE_SERVO in Configuration.h
void servoWrite(int value)
{
  if (REVERSE_SERVO)
    value = 180 - value;

  servo.write(value);
}

// Moves the pen (servo) position. Gently if down and quickly if up.
// Zero is the maximun down position and 180 is the maximun up position.
void movePen(byte toPosition)
{
  if (toPosition > currentPenPosition)
  {
    // Ease it down.
    int penDelay = PEN_DOWN_MOVE_TIME / 10;
    float penIncrement = (toPosition - currentPenPosition) / 10.0;

    // Loop takes it to one step less than full travel.
    for (int i = 1; i < 10; i++)
    { 
      servoWrite(currentPenPosition + penIncrement * i);
      delay(penDelay);
    }

    // Finish off exactly with no round off errors.
    servoWrite(toPosition); 
  }
  else
  {
    // Slam it up.
    servoWrite(toPosition);
  }

  currentPenPosition = toPosition;
}

// Gets a commands from the serial connection and processes it.
void getCommand() 
{
  // Each time we see something on the serial port.
  if (Serial.available() > 0) 
  {
    // Read individual byte from serial port.
    serialChar = Serial.read(); 
    
    if (serialChar == '\n' || serialChar == '\r') 
    {
      // If it is an end of a command character, process the command. 
      buffer[serialCount] = 0;
      processCommand(buffer, serialCount);

      clearBuffer();

      // Reset comment mode before each new command is processed.
      commentMode = false; 
    }
    else 
    {
      // If it's not the end of command look for the start of a comment.
      // TODO: Bug assuming that comments in braces end the line.
      if (serialChar == ';' || serialChar == '(')
      {
        commentMode = true;
      }

      // If not a comment add to buffer.
      if (commentMode != true) 
      {
        // Add byte to buffer string.
        buffer[serialCount] = serialChar; 
        serialCount++;

        // Deal with buffer overflow by clearing the buffer and restarting. 
          if (serialCount > MAX_CMD_SIZE) 
        {
          clearBuffer();
          Serial.flush();
        }
      }
    }
  }
}

// Empties command buffer from serial connection.
void clearBuffer() 
{
  serialCount = 0;
}

boolean getValue(char key, char command[], double *value)
{
  // Find key parameter
  strCharPointer = strchr(buffer, key);

  if (strCharPointer != NULL) // We found a key value
  {
    *value = (double)strtod(&command[strCharPointer - command + 1], NULL);
    return true;
  }
  return false;
}

// Clamps a value between an lower and upper bound.
inline double clamp(double value, double minValue, double maxValue)
{
  return value < minValue ? minValue : (value > maxValue ? maxValue : value);
}

// Deals with standardized input from serial connection.
void processCommand(char command[], int commandLength) 
{
  if (commandLength > 0 && command[0] == 'G') // G-codes
  {
    int codenum = (int)strtod(&command[1], NULL);

    double tempX = steppers->xPos();
    double tempY = steppers->yPos();

    double xVal;
    boolean hasXVal = getValue('X', command, &xVal);
    if (hasXVal)
      xVal *= zoom;
    double yVal;
    boolean hasYVal = getValue('Y', command, &yVal);
    if (hasYVal)
      yVal *= zoom;
    double iVal;
    boolean hasIVal = getValue('I', command, &iVal);
    if (hasIVal)
      iVal *= zoom;
    double jVal;
    boolean hasJVal = getValue('J', command, &jVal);
    if (hasJVal)
      jVal *= zoom;
    double rVal;
    boolean hasRVal = getValue('R', command, &rVal);
    if (hasRVal)
      rVal *= zoom;
    double pVal;
    boolean hasPVal = getValue('P', command, &pVal);

    getValue('F', command, &feedrate);

    if (absoluteMode)
    {
      if (hasXVal)
        tempX = xVal;
      if (hasYVal)
        tempY = yVal;
    }
    else
    {
      if (hasXVal)
        tempX += xVal;
      if (hasYVal)
        tempY += yVal;
    }

    tempY = clamp(tempY, MIN_PEN_AXIS_STEP, MAX_PEN_AXIS_STEP);

    switch (codenum)
    {
    case 0: // G0, Rapid positioning.
      steppers->moveTo(tempX, tempY, MAX_FEEDRATE);
      break;
    case 1: // G1, linear interpolation at specified speed.
      if (currentPenPosition <= penUpPosition)
      {
        // Potentially wrap around the sphere if pen is up.
        steppers->travelTo(tempX, tempY, feedrate);
      }
      else
      {
        steppers->moveTo(tempX, tempY, feedrate);
      }
      break;
    case 2: // G2, Clockwise arc.
    case 3: // G3, Counterclockwise arc.
      if (hasIVal && hasJVal)
      {
        double centerX = steppers->xPos() + iVal;
        double centerY = steppers->yPos() + jVal;
        drawArc(centerX, centerY, tempX, tempY, (codenum == 2));
      }
      else if (hasRVal)
      {
        //drawRadius(tempX, tempY, rVal, (codenum==2));
      }
      break;
    case 4: // G4, Delay P ms.
      if (hasPVal)
      {
        delay(pVal);
      }
      break;
    case 90: // G90, Absolute Positioning.
      absoluteMode = true;
      break;
    case 91: // G91, Incremental Positioning.
      absoluteMode = false;
      break;
    }
  }
  else if (commandLength > 0 && command[0] == 'M') // M-codes
  {
    double value;
    int codenum = (int)strtod(&command[1], NULL);
    switch (codenum)
    {
    case 18: // Disable Drives.
      xStepper->release();
      yStepper->release();
      break;

    case 300: // Servo Position.
      if (getValue('S', command, &value))
      {
        if (value > 180)
          value = penUpPosition;
        value = clamp(value, minPenPosition, maxPenPosition);
        movePen(value);
      }
      break;

    case 301: // Set min pen position.
      if (getValue('P', command, &value))
      {
        minPenPosition = value;
      }
      break;

    case 302: // Set max pen position.
      if (getValue('P', command, &value))
      {
        maxPenPosition = value;
      }
      break;

    case 303: // Set default pen up position.
      if (getValue('P', command, &value))
      {
        penUpPosition = value;
      }
      break;

    case 402: // Propretary: Set global zoom factor.
      if (getValue('S', command, &value))
      {
        zoom = value;
      }
      break;

    case 500:
      savePenConfiguration();
      break;

    case 501:
      loadPenConfiguration();
      break;

    case 502:
      clearPenConfiguration();
      break;
    }
  }
  else if (commandLength > 0 && command[0] == 'N') // N-codes
  {
    // Skip line number.
    int i = 1;
    while (i < commandLength && command[i] != ' ')
      ++i;
    if (i < commandLength - 1)
    {
      processCommand(command + i + 1, commandLength - i - 1);
      return;
    }
  }

  // Done processing commands.
  if (Serial.available() <= 0)
  {
    Serial.print("ok:");
    Serial.println(command);
  }
}

/* This code was ported from the Makerbot/ReplicatorG java sources */
void drawArc(double centerX, double centerY, double endpointX, double endpointY, boolean clockwise)
{
  // Angle variables.
  double angleA;
  double angleB;
  double angle;
  double radius;
  double length;

  // Delta variables.
  double aX;
  double aY;
  double bX;
  double bY;

  // Figure out our deltas.
  double currentX = steppers->xPos();
  double currentY = steppers->yPos();
  aX = currentX - centerX;
  aY = currentY - centerY;
  bX = endpointX - centerX;
  bY = endpointY - centerY;

  // Clockwise
  if (clockwise)
  {
    angleA = atan2(bY, bX);
    angleB = atan2(aY, aX);
  }
  // Counterclockwise
  else
  {
    angleA = atan2(aY, aX);
    angleB = atan2(bY, bX);
  }

  // Make sure angleB is always greater than angleA
  // and if not add 2PI so that it is (this also takes
  // care of the special case of angleA == angleB,
  // i.e. we want a complete circle)
  if (angleB <= angleA)
    angleB += 2. * M_PI;
  angle = angleB - angleA;

  // Calculate a couple useful things.
  radius = sqrt(aX * aX + aY * aY);
  length = radius * angle;

  // For doing the actual move.
  int steps;
  int s;
  int step;

  // Maximum of either 2.4 times the angle in radians
  // or the length of the curve divided by the curve section constant
  steps = (int)ceil(max(angle * 2.4, length));

  // this is the real draw action.
  double newPointX = 0.;
  double newPointY = 0.;

  for (s = 1; s <= steps; s++)
  {
    // Forwards for CCW, backwards for CW.
    if (!clockwise)
      step = s;
    else
      step = steps - s;

    // Calculate our waypoint.
    newPointX = centerX + radius * cos(angleA + angle * ((double)step / steps));
    newPointY = centerY + radius * sin(angleA + angle * ((double)step / steps));

    // Start the move.
    steppers->moveTo(newPointX, newPointY, feedrate);
  }
}