# G-Code
The history of the SphereBot G-Code is not clearly documented.  It appears that Eberhard Rensch in his original project (http://pleasantsoftware.com/developer/3d/spherebot/) derived it from the MakerBot Unicorn pen plotter to which documentation seems to no longer exist.  The G-Code also relies on the Unicorn G-Code Extension for Inkscape from Marty McGuire (https://github.com/martymcguire/inkscape-unicorn) which is no longer supported and does not work with version 1.0 of Inkscape.

The original SphereBot G-Code uses 7 common 3D printer/CNC machine ‘G’ word commands (G00, G01, G02, G03, G04, G90, G91), 2 common ‘M’ word commands (M01, M18) and 4 custom (proprietary) commands (M300, M400, M401, M402).  The M300 command is responsible for positioning the pen and appears to be a carryover from the MakerBot Unicorn pen plotter.  The M400 - Reset X-Axis-Stepper settings to new object diameter, M401 - Reset Y-Axis-Stepper settings to new object diameter and M402 - Set global zoom factor are proprietary commands needed to size images for the SphereBot.

The primary words used in a SphereBot file are G00, G01, G02, G03 and M300. The interesting thing is that while G00 and G01 will position the Z axis this is not used to position the pen.  Instead, the custom M300 command is used to position the pen.  This creates a problem where pen positioning can be both unique to the SphereBot design (hardware) and the object being printed on (i.e. egg vs ping pong ball) such that sharing G-Code files is almost impossible without some kind of modification. One of the first challenges is getting G-Code that works on your SphereBot.  Included in this project in the Util/GCode folder are several G-Code files.  They may work out of the box, but chances are you will need to first manual execute M300 commands to determine the best pen up and more importantly pen down settings in order to search and replace the ‘S’ word values for the pen up/down.

The jinschoi/SphereBot fork (https://github.com/jinschoi/SphereBot) of the SphereBot code from which this fork is derived is coded to use the Adafruit motor control shield.  The fork adds 6 custom (proprietary) commands (M301, M302, M303, M500, M501, M503) and removes 2 (M400, M401).  The M400 and M401 commands have been removed as the plotting units have been changed from nominal mm to micro steps. If you are using a 200 steps/revolution stepper, G1 Y3200 will rotate the rotation axis one full turn (using 16x micro stepping). This makes adjusting for object diameter unnecessary and makes it easier to adopt patterns using the EggBot templates, which are 3200 px wide by 800 px tall.  The commands M301 - Set the minimum pen position, M302 - Set the maximum pen position and M303 set the pen up position have been added.  Also added are the commands M500 - Save the pen settings to the Arduino EEPROM, M501 - Load the pen settings from the Arduino EEPROM and M501 - Reset the pen settings to the firmware defaults.

The tgolla/SphereBot fork (https://github.com/tgolla/SphereBot) adds the following custom (proprietary) commands to address pen positioning and deal with the challenges with that sharing G-Code files is almost impossible without some kind of modification.

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

As stated, these commands have been added to eliminate the need to modify G-Code files for a specific SphereBot by allowing you to preconfigure the software such that it can automatically adjust the pen up/down defined in the G-Code file.

The M304 command sets the pen down position.  This command is used in conjunction with the M303 command that sets the pen up position.  These commands allow you to adjust the pen to up/down positions specific to the SphereBot.  The commands work in conjunction with the MZ mode command (M305) and the M330 and Z height adjustment commands (M306, M307).

The M305 command sets the MZ mode.  The MZ mode setting determines how the software interprets the G-Code file.  The mode can be set to one of three settings.  When set to M mode (P0) the M300 command is responsible for setting the height of the pen.  This is the same behavior found in past versions of the software code.  When set to Z mode the Z parameter in the G0, G1, G2 or G3 command is responsible for setting for setting the height of the pen.  When set to Auto mode (P2) the mode is set to the appropriate M or Z mode based on which command is first detected in the G-Code file.  For example, if a G0, G1, G2 or G3 command with a Z parameter is read first the mode is set to Z.  But if an M300 command is read first the mode is set to M.  

The M306 and M307 commands determine how the respective M300 command or G0, G1, G2 or G3 command Z parameter pen height value is interpreted.  If set to Off (P0) the value provided by the command or parameter is used as the absolute pen height. If set to Preset (P1) the pen up preset (M308 or M309) value is used to determine if the pen position is set to the pen up (M304) or pen down (M303) value.  For example, if the value provided is greater than or equal to the preset value, the pen position is set to the pen up value (M304) and if it is less than the pen position is set to the pen down value (M303).  If set to Calculated (P2) the calculated value is determined by assuming the first height position value read is the value used to determine if the pen should be positioned up or down.  For example, if the first pen position value is 5, all values 5 equal and greater than will position the pen at the pen up value (M304).  Those less than will position the pen at the pen down value (M304). 
 
Using a file to preset the M303 through M309 values and saving the configuration with the M500 command provides a great deal of versatility.  For example, the commands M305 P0 and M306 P0 would configure the SphereBot to operate as it has with past software releases requiring specific G-Code files to be customized for the specific SphereBot.  Executing the commands M305 P2, M306 P2 and M307 P2 would configure the SphereBot to its most versatile setting where there should be no need to customize a G-Code file. Both scenarios assume you have set M304 and M303 to the correct heights.

The M310 command allows you to override the XY feed rates set in your G-Code file with a preset value.  If set to 0 the feed rate is initialized with the default feed rate and is set through G0, G1, G2 & G3 codes with X or Y values by Fxxx.

The M311 command allows you to override the Z (pen movement) feed rates set in your G-Code file with a preset value.  If set to 0 the feed rate is initialized with the default feed rate and is set through the M300 Fxxx or G0, G1, G2 & G3 codes with only Z values by Fxxx.  The preference is to set this value as can be unique to either the SphereBot or object being drawn on and prevents the pen from crashing down an egg for example.

The M312 command sets the pen up federate multiplier.  This command allows the SphereBot to move off the object at a faster rate. One means the pen will go up at the same speed (degrees/second) as it goes down. Each increase by one will logarithmically double the pen up speed.

Although you can easily modify the beginning of each G-Code file with the correct M301 through M312 codes for it to work best it is recommended that you instead create multiple scenario setting files.  For example, one for small eggs, one for large eggs and one for ping pong balls.

The M999 command is for debugging and can be useful when determining things like pen up/down settings, feed rates and G-Code interpretation behavior.  This code is optionally configured in the configuration.h file and it should be noted that in an Arduino Uno configuration the code consumes a large portion of dynamic memory.

For those wanting to try their hand at generating G-Code for the SphereBot there is a solution.  InkScape v1.0 comes prepackages with the Gcodetools extension.  I go into the process of converting a drawing into a line drawing and then G-Code using this extension elsewhere in the documentation.
