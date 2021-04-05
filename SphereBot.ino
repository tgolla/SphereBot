/*
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

2015-2016 the code was modified to run on an Adafruit Motor Shield V2 
by Jin Choi <jsc@alum.mit.edu>.

2016 code support for the Adafruit Motor Shield V1 was added by GrAndAG.

3/2021 the code was modified with the following improvements 
by Terence Golla (tgolla).
  
  - Corrected the naming convention mix of snake_case and camelCase for more
    prominent C/C++ Arduino software use of camelCase.
 
  - Restructured the ino software file to follow Arduino styling...
      - Begin with a set of comments clearly describing the purpose and 
        assumptions of the code.
      - Import any required library headers.
      - Define constants.
      - Define global variables.
      - Define setup() and loop().
      - Define subroutines (functions).
 
  - Expanded in code documentation (comments).

  - Modified code to operate servo installed in reverse.
 
  - A true to G-Code specification parser was added. The class was developed using
    Visual Studio Community (https://visualstudio.microsoft.com/vs/community/)
    and the Microsoft Unit Testing Framework for C++ 
    (https://docs.microsoft.com/en-us/visualstudio/test/writing-unit-tests-for-c-cpp?view=vs-2019).

  - Added the following new M-Codes to allow for increased flexablity
    controlling the pen servo. 
 
      M304 - Sets the pen down position needed for new MZ mode.
      M305 - Sets the G-Code responsible for operating the pen servo.  
             P0 - M300 sets the pen height. P1 - G0, G1, G2 & G3 Z parameter
             is responsible for setting the pen height. P2 - Automatically
             detects which code is responsible for setting the pen height. 
      M306 - Sets the M300 height adjustment. P0 - Off, P1 - Preset, P2 - Calculated
      M307 - Sets the Z height adjustment. P0 - Off, P1 - Preset, P2 - Calculated
      M308 - Sets the M300 pen up preset value. S values less than the value 
             move the pen down.
      M309 - Sets the Z pen up preset value. Z values less than the value 
             move the pen down.
      M310 - Sets the XY feedrate preset value. If zero feedrate is initalized with 
             default value and set through G0, G1, G2 & G3 codes with X or Y values
             by Fxxx. 
      M311 - Sets the pen feedrate preset value. If zero feedrate is initalized with 
             default value and set through the M300 Fxxx or G0, G1, G2 & G3 codes
             with only Z values by Fxxx.
      M312 - Sets the pen up feedrate multiplier. One means the pen will go up at the 
             same speed (degrees/second) as it goes down. Each increase by one will 
             logarithmically double the pen up speed.
      M999 - Debugging command. To add define DEBUG true in configuration.h. Note:
             In an Arduino Uno configuration do not add unless necessary as the code
             consumes a large portion of dynamic memory.

  - Added the ability to set the pen feedrate with the M300 Fxxx or G0, G1, G2 & G3 
    codes with only Z values by Fxxx.

  - Corrected pen up/down (feedrate was reversed) and changed feedrate to 
    degrees/second.

Future feature enhancements are planned to add a touchscreen control, SD card reader 
support and audio alerts.

This sketch needs the following non-standard library which can be installed with the
Arduino Library Manager.

G-Code Parser Library
  https://github.com/tgolla/GCodeParser

EEPROM Typed Library
  https://github.com/tgolla/EEPROMTyped  

Adafruit Motor Shield (select appropriate version ):
  v1: https://github.com/adafruit/Adafruit-Motor-Shield-library
  v2: https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library

Adafruit ILI9341 Arduino Library
  https://github.com/adafruit/Adafruit_ILI9341
 
Adafruit GFX Library
  https://github.com/adafruit/Adafruit-GFX-Library
 
Adafruit ImageReader Arduino Library
  https://github.com/adafruit/Adafruit_ImageReader
 
Adafruit_FT6206 Library
  https://github.com/adafruit/Adafruit_FT6206_Library
 
Be sure to review and make appropriate modification to global constants 
in the "Configuration.h" file. 
*/

#include "Configuration.h"
#include <GCodeParser.h>
#include <EEPROMTyped.h>

#include <SPI.h>
#include <Wire.h>

#if ADAFRUIT_TFT_TOUCH_SHIELD == true
// Adafruit 2.8" TFT Touch Shield for Arduino w/Capacitive Touch
#include <Adafruit_GFX.h> // Core Graphics Library
// This contains the low-level code specific to the TFT display.
#include <Adafruit_ILI9341.h>
#include <SdFat.h> // SD card & FAT filesystem library.
// Controller library which does all the low level chatting with
// the FT6206 capacitive touch driver chip.
#include <Adafruit_FT6206.h>
#include <Adafruit_ImageReader.h> // Image-reading functions.
#endif

// DualStepper Library for Adafruit Motor Shield
#include "DualStepper.h"

// Servo Library
#include <Servo.h>

#include <EEPROM.h>

byte minPenPosition;
byte maxPenPosition;
byte penUpPosition;
byte penDownPosition;
byte currentPenPosition;

byte mzMode;
byte mzActiveMode;
byte mAdjust;
byte zAdjust;
byte mAdjustPreset;
byte zAdjustPreset;
short mAdjustCalculated = -1;
short zAdjustCalculated = -1;
double xyFeedrate;
double penFeedrate;
double presetXyFeedrate;
double presetPenFeedrate;
short penUpFeedrateMultiplier;

// EEPROM memory locations.
byte valueSavedEEPROMMemoryLocation = 0;
byte minPenEEPROMMemoryLocation = 1;
byte maxPenEEPROMMemoryLocation =
    minPenEEPROMMemoryLocation + EEPROMTyped.sizeOf(minPenPosition);
byte penUpEEPROMMemoryLocation =
    maxPenEEPROMMemoryLocation + EEPROMTyped.sizeOf(maxPenPosition);
byte penDownEEPROMMemoryLocation =
    penUpEEPROMMemoryLocation + EEPROMTyped.sizeOf(penUpPosition);
byte mzModeEEPROMMemoryLocation =
    penDownEEPROMMemoryLocation + EEPROMTyped.sizeOf(penDownPosition);
byte mAdjustEEPROMMemoryLocation =
    mzModeEEPROMMemoryLocation + EEPROMTyped.sizeOf(mzMode);
byte zAdjustEEPROMMemoryLocation =
    mAdjustEEPROMMemoryLocation + EEPROMTyped.sizeOf(mAdjust);
byte mAdjustPresetEEPROMMemoryLocation =
    zAdjustEEPROMMemoryLocation + EEPROMTyped.sizeOf(zAdjust);
byte zAdjustPresetEEPROMMemoryLocation =
    mAdjustPresetEEPROMMemoryLocation + EEPROMTyped.sizeOf(mAdjustPreset);
byte xyFeedrateEEPROMMemoryLocation =
    zAdjustPresetEEPROMMemoryLocation + EEPROMTyped.sizeOf(zAdjustPreset);
byte penFeedrateEEPROMMemoryLocation =
    xyFeedrateEEPROMMemoryLocation + EEPROMTyped.sizeOf(xyFeedrate);
byte presetXyFeedrateEEPROMMemoryLocation =
    penFeedrateEEPROMMemoryLocation + EEPROMTyped.sizeOf(penFeedrate);
byte presetPenFeedrateEEPROMMemoryLocation =
    presetXyFeedrateEEPROMMemoryLocation + EEPROMTyped.sizeOf(presetXyFeedrate);
byte penUpFeedrateMultiplierEEPROMMemoryLocation =
    presetPenFeedrateEEPROMMemoryLocation + EEPROMTyped.sizeOf(presetPenFeedrate);

// Number expected to be found in EEPROM memory location 0 that indicates
// pen setting have been stored in memory.
#define EEPROM_MAGIC_NUMBER 23

// Enum used with mzMode.
enum
{
  M,
  Z,
  Auto
};

// Enum used with mAdjust and zAdjust.
enum
{
  Off,
  Preset,
  Calculated
};

// Set up stepper motors and servo.
#if ADAFRUIT_MOTOR_SHIELD_VERSION == 1
SingleStepper *xStepper =
    new SingleStepper(new AF_Stepper(STEPS_PER_REVOLUTION, ROTATION_AXIS_PORT));
SingleStepper *yStepper =
    new SingleStepper(new AF_Stepper(STEPS_PER_REVOLUTION, PEN_AXIS_PORT));
#else
Adafruit_MotorShield MS = Adafruit_MotorShield();
SingleStepper *xStepper =
    new SingleStepper(MS.getStepper(STEPS_PER_REVOLUTION, ROTATION_AXIS_PORT));
SingleStepper *yStepper =
    new SingleStepper(MS.getStepper(STEPS_PER_REVOLUTION, PEN_AXIS_PORT));
#endif

DualStepper *steppers =
    new DualStepper(xStepper, yStepper, STEPS_PER_REVOLUTION *MICROSTEPS);

Servo servo;

// GCode states.
boolean absoluteMode = true;
double zoom = DEFAULT_ZOOM_FACTOR;

// Defaults to Serial port for GCode if SD and Touchscreen are not present.
boolean serialMode = true;

GCodeParser GCode = GCodeParser();

#if ADAFRUIT_TFT_TOUCH_SHIELD == true
SdFat SD;                        // SD card filesystem.
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD.

// Instantiate class for TFT screen and Touchscreen
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_FT6206 ts = Adafruit_FT6206();
#endif

void setup()
{
  Serial.begin(115200);

  loadPenConfiguration();

  // Configure Adafruit motor shield.
#if ADAFRUIT_MOTOR_SHIELD_VERSION == 2
  MS.begin();

  // Change the i2c clock to 400KHz for faster stepping.
  TWBR = ((F_CPU / 400000L) - 16) / 2;
#endif

  steppers->setMaxSpeed(MAX_XY_FEEDRATE);

  // Configure pen servo.
  servo.attach(SERVO_PIN);
  servoWrite(penUpPosition);
  currentPenPosition = penUpPosition;

#if ADAFRUIT_TFT_TOUCH_SHIELD == true
  // Look for SD and Touchscreen.
  if (SD.begin(SD_CS))
  {
    if (ts.begin(FT6206_THRESSHOLD))
    {
      tft.begin();
      tft.setRotation(3);
      tft.fillScreen(ILI9341_WHITE);

      serialMode = false;

      // Display Splash Screen
      ImageReturnCode stat = reader.drawBMP("/splash.bmp", tft, 0, 0);

      delay(SPLASH_SCREEN_DELAY);

      // Prompt for operation mode (Serial/USB or SD Card)
    }
  }
#endif

  if (serialMode)
  {
    Serial.println("Ready");
    delay(100);
  }
}

void loop()
{
  if (serialMode)
  {
    // Check serial port for character.
    if (Serial.available() > 0)
    {
      if (GCode.AddCharToLine(Serial.read()))
      {
        GCode.ParseLine();
        processCommand();
      }
    }
  }
#if ADAFRUIT_TFT_TOUCH_SHIELD
  else
  {
    // SD file
  }
#endif
}

// Loads the pen configuration from memory.
void loadPenConfiguration()
{
  // Check EEPROM location 0 for presence of a magic number.
  // If it's there, we have saved pen settings.
  if (EEPROM.read(valueSavedEEPROMMemoryLocation) == EEPROM_MAGIC_NUMBER)
  {
    EEPROMTyped.read(minPenEEPROMMemoryLocation, minPenPosition);
    EEPROMTyped.read(maxPenEEPROMMemoryLocation, maxPenPosition);
    EEPROMTyped.read(penUpEEPROMMemoryLocation, penUpPosition);
    EEPROMTyped.read(penDownEEPROMMemoryLocation, penDownPosition);
    EEPROMTyped.read(mzModeEEPROMMemoryLocation, mzMode);
    EEPROMTyped.read(mAdjustEEPROMMemoryLocation, mAdjust);
    EEPROMTyped.read(zAdjustEEPROMMemoryLocation, zAdjust);
    EEPROMTyped.read(mAdjustPresetEEPROMMemoryLocation, mAdjustPreset);
    EEPROMTyped.read(zAdjustPresetEEPROMMemoryLocation, zAdjustPreset);
    EEPROMTyped.read(xyFeedrateEEPROMMemoryLocation, xyFeedrate);
    EEPROMTyped.read(penFeedrateEEPROMMemoryLocation, penFeedrate);
    EEPROMTyped.read(presetXyFeedrateEEPROMMemoryLocation, presetXyFeedrate);
    EEPROMTyped.read(presetPenFeedrateEEPROMMemoryLocation, presetPenFeedrate);
    EEPROMTyped.read(penUpFeedrateMultiplierEEPROMMemoryLocation, penUpFeedrateMultiplier);
  }
  else
  {
    minPenPosition = MIN_PEN_POSITION;
    maxPenPosition = MAX_PEN_POSITION;
    penUpPosition = DEFAULT_PEN_UP_POSITION;
    penDownPosition = DEFAULT_PEN_DOWN_POSITION;
    mzMode = DEFAULT_MZ_MODE;
    mAdjust = DEFAULT_M_ADJUST;
    zAdjust = DEFAULT_Z_ADJUST;
    mAdjustPreset = DEFAULT_M_ADJUST_PRESET;
    zAdjustPreset = DEFAULT_Z_ADJUST_PRESET;
    xyFeedrate = DEFAULT_XY_FEEDRATE;
    penFeedrate = DEFAULT_PEN_FEEDRATE;
    presetXyFeedrate = DEFAULT_PRESET_XY_FEEDRATE;
    presetPenFeedrate = DEFAULT_PRESET_PEN_FEEDRATE;
    penUpFeedrateMultiplier = DEFAULT_PEN_UP_FEEDRATE_MULTIPLIER;
  }

  // Negative 1 indicates the calculated value has not been set.
  short mAdjustCalculated = -1;
  short zAdjustCalculated = -1;

  // If preset values are not zero use to set feedrates.
  if (presetXyFeedrate > 0)
    xyFeedrate = presetXyFeedrate;
  if (presetPenFeedrate > 0)
    penFeedrate = presetPenFeedrate;

  // Used to determine the MZ mode when set to Auto.
  mzActiveMode = mzMode;
}

// Saves the pen configuration to memeoy.
void savePenConfiguration()
{
  EEPROMTyped.write(minPenEEPROMMemoryLocation, minPenPosition);
  EEPROMTyped.write(maxPenEEPROMMemoryLocation, maxPenPosition);
  EEPROMTyped.write(penUpEEPROMMemoryLocation, penUpPosition);
  EEPROMTyped.write(penDownEEPROMMemoryLocation, penDownPosition);
  EEPROMTyped.write(mzModeEEPROMMemoryLocation, mzMode);
  EEPROMTyped.write(mAdjustEEPROMMemoryLocation, mAdjust);
  EEPROMTyped.write(zAdjustEEPROMMemoryLocation, zAdjust);
  EEPROMTyped.write(mAdjustPresetEEPROMMemoryLocation, mAdjustPreset);
  EEPROMTyped.write(zAdjustPresetEEPROMMemoryLocation, zAdjustPreset);
  EEPROMTyped.write(xyFeedrateEEPROMMemoryLocation, xyFeedrate);
  EEPROMTyped.write(penFeedrateEEPROMMemoryLocation, penFeedrate);
  EEPROMTyped.write(presetXyFeedrateEEPROMMemoryLocation, presetXyFeedrate);
  EEPROMTyped.write(presetPenFeedrateEEPROMMemoryLocation, presetPenFeedrate);
  EEPROMTyped.write(penUpFeedrateMultiplierEEPROMMemoryLocation, penUpFeedrateMultiplier);

  EEPROMTyped.write(valueSavedEEPROMMemoryLocation, EEPROM_MAGIC_NUMBER);
}

// Clears the pen configuration from memory.
void clearPenConfiguration()
{
  EEPROMTyped.write(valueSavedEEPROMMemoryLocation, 0xff);

  loadPenConfiguration();
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
// 3/2021 - Corrected feedrate to degrees/second. TFG
// 3/20/2021 - Added pen up feedrate multiplier.
void movePen(byte toPosition)
{
  // Ease the pen down based on the feedrate.
  // One Second (1000ms) divided by feedrate (degrees).
  // i.e. With a feedrate of 180 the pen moves 180 degrees in 1 second.
  // At a feedrate of 90 the pen moves 180 degrees in 2 second.
  // At a feedrate of 360 the pen moves 180 degrees in 1/2 second.
  // Move the pen up based on the feedrate * multiplier. Each increase 
  // by one will logarithmically double the pen up speed.
  int penDelay;

  if (toPosition < currentPenPosition)
    penDelay = 1000 / penFeedrate;
  else 
    penDelay = 1000 / (penFeedrate * penUpFeedrateMultiplier);
  
  while (currentPenPosition != toPosition)
  {
    if (toPosition < currentPenPosition)
      currentPenPosition = currentPenPosition - 1;
    else
      currentPenPosition = currentPenPosition + 1;

    servoWrite(currentPenPosition);
    delay(penDelay);
  }  
}

// Clamps a value between an lower and upper bound.
inline double clamp(double value, double minValue, double maxValue)
{
  return value < minValue ? minValue : (value > maxValue ? maxValue : value);
}

// Processes parsed GCode.
void processCommand()
{
  if (GCode.HasWord('G')) // G-Codes
  {
    int gCodeNumber = (int)GCode.GetWordValue('G');

    double tempX = steppers->xPos();
    double tempY = steppers->yPos();

    if (gCodeNumber >= 0 && gCodeNumber <= 3) //G0, G1, G2, G3
    {
      // If MZ active mode equals auto and a Z coordinate exist
      // set the MZ active mode to Z.
      if (mzActiveMode == Auto && GCode.HasWord('Z'))
        mzActiveMode = Z;

      if (GCode.HasWord('X'))
      {
        if (absoluteMode)
          tempX = GCode.GetWordValue('X') * zoom;
        else
          tempX += GCode.GetWordValue('X') * zoom;
      }

      if (GCode.HasWord('Y'))
      {
        if (absoluteMode)
          tempY = GCode.GetWordValue('Y') * zoom;
        else
          tempY += GCode.GetWordValue('Y') * zoom;
      }

      tempY = clamp(tempY, MIN_PEN_AXIS_STEP, MAX_PEN_AXIS_STEP);

      // Set XY or Z (Pen) feedrate.
      if (GCode.HasWord('F'))
      {
        if (GCode.HasWord('X') || GCode.HasWord('Y'))
        {
          if (presetXyFeedrate <= 0)
            xyFeedrate = GCode.GetWordValue('F');
        }
        else
        {
          if (GCode.HasWord('Z'))
          {
            if (mzActiveMode == Z && presetPenFeedrate <= 0)
              penFeedrate = GCode.GetWordValue('F');
          }
        }
      }

      if (mzActiveMode == Z)
      {
        if (GCode.HasWord('Z'))
        {
          double value = GCode.GetWordValue('Z');

          // Adjust Z based on preset pen up value.
          if (zAdjust == Preset)
          {
            if (value < zAdjustPreset)
              value = penDownPosition;
            else
              value = penUpPosition;
          }

          // Adjust Z based on calculated pen up value.
          if (zAdjust == Calculated)
          {
            // Set the calculated adjustment with the first Z value.
            // The algorithm assume the first value is for the pen to be up.
            if (zAdjustCalculated < 0)
              zAdjustCalculated = (short)value;

            if (value < zAdjustCalculated)
              value = penDownPosition;
            else
              value = penUpPosition;
          }

          value = clamp(value, minPenPosition, maxPenPosition);

          movePen(value);
        }
      }
    }

    switch (gCodeNumber)
    {
    // G00 – Rapid Positioning
    // The G00 command moves the machine at maximum travel speed from a current
    // position to a specified point or the coordinates specified by the command.
    // The machine will move all axis at the same time, so they complete the
    // travel simultaneously. This results in a straight-line movement to the new
    // position point.
    case 0:
      steppers->moveTo(tempX, tempY, MAX_XY_FEEDRATE);
      break;

    // G01 – Linear Interpolation
    // The G01 G-code command instructs the machine to move in a straight line
    // at a set feed rate or speed. We specify the end position with the X, Y
    // and Z values, and the speed with the F value. The machine controller
    // calculates (interpolates) the intermediate points to pass through to get
    // that straight line. Although these G-code commands are simple and quite
    // intuitive to understand, behind them, the machine controller performs
    // thousands of calculations per second in order to make these movements.
    case 1: // Potentially wrap around the sphere if pen is up.
      if (currentPenPosition <= penUpPosition)
        steppers->travelTo(tempX, tempY, xyFeedrate);
      else
        steppers->moveTo(tempX, tempY, xyFeedrate);
      break;

    // G02 – Circular Interpolation Clockwise
    // The G02 command tells the machine to move clockwise in a circular pattern.
    // It is the same concept as the G01 command and it’s used when performing
    // the appropriate machining process. In addition to the end point parameters,
    // here we also need to define the center of rotation, or the distance of
    // the arc start point from the center point of the arc. The start point is
    // actually the end point from the previous command or the current point.
    case 2:
    // G03 – Circular Interpolation Counterclockwise
    // Just like the G02, the G03 G-code command defines the machine to move in
    // circular pattern. The only difference here is that the motion is
    // counterclockwise. All other features and rules are the same as the G02
    // command.
    case 3:
      if (GCode.HasWord('I') && GCode.HasWord('J'))
      {
        double centerX = steppers->xPos() + (GCode.GetWordValue('I') * zoom);
        double centerY = steppers->yPos() + (GCode.GetWordValue('J') * zoom);
        drawArc(centerX, centerY, tempX, tempY, (gCodeNumber == 2));
      }
      else if (GCode.HasWord('R'))
      {
        //drawRadius(tempX, tempY, GCode.GetWordValue('R') * zoom, (gCodeNumber==2));
      }
      break;

    // G04 – Dwell Command
    // G04 is called the Dwell command because it makes the machine stop what it
    // is doing or dwell for a specified length of time. It is helpful to be able
    // to dwell during a cutting operation, and also to facilitate various
    // non-cutting operations of the machine.
    case 4: // G4 - Delay P milliseconds.
      if (GCode.HasWord('P'))
        delay(GCode.GetWordValue('P'));
      break;

    // G90/G91 – Positioning G-code commands
    // With the G90 and G91 commands we tell the machine how to interpret the
    // coordinates. G90 is for absolute mode and G91 is for relative mode.
    //
    // In absolute mode the positioning of the tool is always from the absolute
    // point or zero. So, the command G01 X10 Y5 will take the tool to that
    // exact point (10,5), no matter the previous position.
    //
    // On the other hand, in relative mode, the positioning of the tool is
    // relative to the last point. So, if the machine is currently at point
    // (10,10), the command G01 X10 Y5 will take the tool to point (20,15).
    // This mode is also called “incremental mode”.
    case 90: // G90 - Absolute Positioning.
      absoluteMode = true;
      break;
    case 91: // G91 - Incremental Positioning.
      absoluteMode = false;
      break;
    }
  }
  else if (GCode.HasWord('M')) // M-Codes
  {
    int mCodeNumber = (int)GCode.GetWordValue('M');

    double value;

    switch (mCodeNumber)
    {
    case 18: // M18 - Disable all stepper motors.
      xStepper->release();
      yStepper->release();
      break;

    case 300: // M300 - Set pen (servo) position.
      // If MZ active mode equals auto and a M300 command exist
      // set the MZ active mode to M.
      if (mzActiveMode == Auto)
        mzActiveMode = M;

      if (mzActiveMode == M)
      {
        if (GCode.HasWord('F'))
        {
          penFeedrate = GCode.GetWordValue('F');
        }

        if (GCode.HasWord('S'))
        {
          value = GCode.GetWordValue('S');

          // Adjust S based on preset pen up value.
          if (mAdjust == Preset)
          {
            if (value < mAdjustPreset)
              value = penDownPosition;
            else
              value = penUpPosition;
          }

          // Adjust S based on calculated pen up value.
          if (mAdjust == Calculated)
          {
            // Set the calculated adjustment with the first M300 S value.
            // The algorithm assume the first value is for the pen to be up.
            if (mAdjustCalculated < 0)
              mAdjustCalculated = (short)value;

            if (value < mAdjustCalculated)
              value = penDownPosition;
            else
              value = penUpPosition;
          }

          value = clamp(value, minPenPosition, maxPenPosition);

          movePen(value);
        }
      }
      break;

    case 301: // M301 - Set minimum pen position.
      if (GCode.HasWord('P'))
        minPenPosition = GCode.GetWordValue('P');
      break;

    case 302: // M302 - Set maximum  pen position.
      if (GCode.HasWord('P'))
        maxPenPosition = GCode.GetWordValue('P');
      break;

    case 303: // M303 - Set default pen up position.
      if (GCode.HasWord('P'))
        penUpPosition = GCode.GetWordValue('P');
      break;

    case 304: // M304 - Set default pen down position.
      if (GCode.HasWord('P'))
        penDownPosition = GCode.GetWordValue('P');
      break;

    // M305 - Sets the G-Code responsible for operating the pen servo.
    // P0 - M300 sets the pen height. P1 - G0, G1, G2 & G3 Z parameter is
    // responsible for setting the pen height. P2 - Automatically detects
    // which code is responsible for setting the pen height.
    case 305:
      if (GCode.HasWord('P'))
      {
        mzMode = GCode.GetWordValue('P');

        if (mzMode > Auto)
          mzMode = M;

        mzActiveMode = mzMode;
      }
      break;

    // M306 - Sets the M300 height adjustment. P0 - Off, P1 - Preset, P2 - Calculated
    case 306:
      if (GCode.HasWord('P'))
      {
        mAdjust = GCode.GetWordValue('P');

        if (mAdjust > Calculated)
          mAdjust = Off;

        // Force recalculation of calculated adjustment.
        mAdjustCalculated = -1;
      }
      break;

    // M307 - Sets the Z height adjustment. P0 - Off, P1 - Preset, P2 - Calculated
    case 307:
      if (GCode.HasWord('P'))
      {
        zAdjust = GCode.GetWordValue('P');

        if (zAdjust > Calculated)
          zAdjust = Off;

        // Force recalculation of calculated adjustment.
        zAdjustCalculated = -1;
      }
      break;

    // M308 - Sets the M300 pen up preset value.
    // S values less than the value move the pen down.
    case 308:
      if (GCode.HasWord('P'))
        mAdjustPreset = GCode.GetWordValue('P');
      break;

    // M309 - Sets the Z pen up preset value.
    // Z values less than the value move the pen down.
    case 309:
      if (GCode.HasWord('P'))
        zAdjustPreset = GCode.GetWordValue('P');
      break;

    // M310 - Sets the XY feedrate preset value. If zero feedrate is initalized
    // with default value and set through G0, G1, G2 & G3 codes with X or Y
    // values by Fxxx. 
    case 310:
      if (GCode.HasWord('P'))
      {
        presetXyFeedrate = GCode.GetWordValue('P');

        if (presetXyFeedrate > 0)
          xyFeedrate = presetXyFeedrate;
      }
      break;

    // M311 - Sets the pen feedrate preset value. If zero feedrate is initalized
    // with default value and set through the M300 Fxxx or G0, G1, G2 & G3 codes
    // with only Z values by Fxxx.
    case 311:
      if (GCode.HasWord('P')) 
      {
        presetPenFeedrate = GCode.GetWordValue('P');

        if (presetPenFeedrate > 0)
          penFeedrate = presetPenFeedrate;
      }
      break;

    // M312 - Sets the pen up feedrate multiplier. One means the pen will go up at the 
    // same speed (degrees/second) as it goes down. Each increase by one will 
    // logarithmically double the pen up speed.
    case 312:
      if (GCode.HasWord('P')) 
      {
        penUpFeedrateMultiplier = GCode.GetWordValue('P');

        if (penUpFeedrateMultiplier < 1)
          penUpFeedrateMultiplier = 1;
      }
    break;

    case 402: // M402 - Set global zoom factor.
      if (GCode.HasWord('S'))
        zoom = GCode.GetWordValue('S');
      break;

    case 500: // M500 - Save pen configuration to EEPROM.
      savePenConfiguration();
      break;

    case 501: // M501 - Load pen configuration from EEPROM.
      loadPenConfiguration();
      break;

    case 502: // M502 - Clear pen configuration from EEPROM.
      clearPenConfiguration();
      break;

#if DEBUG == true
    case 999: // M999 - Debugging command.
      Serial.println();
      Serial.print("M301 Pxxx - Minimun Pen Position: ");
      Serial.println(minPenPosition);
      Serial.print("M302 Pxxx - Maximun Pen Position: ");
      Serial.println(maxPenPosition);
      Serial.print("M303 Pxxx - Pen Up Position: ");
      Serial.println(penUpPosition);
      Serial.print("M304 Pxxx - Pen Down Position: ");
      Serial.println(penDownPosition);
      Serial.print("M305 Px - MZ Mode (0-M, 1-Z, 2-Auto): ");
      Serial.print(mzMode);
      Serial.print("  MZ Active Mode: ");
      Serial.println(mzActiveMode);
      Serial.print("M306 Px - M Adjust (0-Off, 1-Preset, 2-Calculated): ");
      Serial.println(mAdjust);
      Serial.print("M307 Px - Z Adjust: (0-Off, 1-Preset, 2-Calculated): ");
      Serial.println(zAdjust);
      Serial.print("M308 Pxxx - M Adjust Preset: ");
      Serial.println(mAdjustPreset);
      Serial.print("M309 Pxxx - Z Adjust Preset: ");
      Serial.println(zAdjustPreset);
      Serial.print("M Adjust Calculated: ");
      Serial.println(mAdjustCalculated);
      Serial.print("Z Adjust Calculated: ");
      Serial.println(zAdjustCalculated);
      Serial.print("Gx Xxxx Yxxx Fxxx - XY Feedrate: ");
      Serial.println(xyFeedrate);
      Serial.print("M300 Sxxx Fxxx or Gx Zxxx Fxxx - Pen Feedrate: ");
      Serial.println(penFeedrate);
      Serial.print("M310 - Preset XY Feedrate : ");
      Serial.println(presetXyFeedrate);
      Serial.print("M311 - Preset Pen Feedrate: ");
      Serial.println(presetPenFeedrate);
      Serial.print("M312 - Pen Up Feedrate Multiplier: ");
      Serial.println(penUpFeedrateMultiplier);
      Serial.print("Values Saved in EEPROM ("); 
      Serial.print(EEPROM_MAGIC_NUMBER);
      Serial.print("): ");
      Serial.println(EEPROM.read(valueSavedEEPROMMemoryLocation));
      Serial.println("M18 - Disable stepper motors.");
      Serial.println("M500 - Save pen configuration to EEPROM.");
      Serial.println("M501 - Load pen configuration from EEPROM (or defaults if cleared).");
      Serial.println("M502 - Clear pen configuration from EEPROM.");
      Serial.println();
      break;
#endif
    }
  }

  // Done processing commands.
  if (Serial.available() <= 0)
  {
    Serial.print("ok:");
    Serial.println(GCode.line);
  }
}

/* This code was ported from the Makerbot/ReplicatorG java sources */
void drawArc(double centerX, double centerY, double endpointX,
             double endpointY, boolean clockwise)
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
    steppers->moveTo(newPointX, newPointY, xyFeedrate);
  }
}