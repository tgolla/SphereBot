# G-Code
The history of the SphereBot G-Code is not clearly documented.  It appears that Eberhard Rensch in his original project (http://pleasantsoftware.com/developer/3d/spherebot/) derived it from the MakerBot Unicorn pen plotter to which documentation seems to no longer exist.  The G-Code also relies on the Unicorn G-Code Extension for Inkscape from Marty McGuire (https://github.com/martymcguire/inkscape-unicorn) which is no longer supported and does not work with version 1.0 of Inkscape.

The original SphereBot G-Code uses 7 common 3D printer/CNC machine ‘G’ word commands (G00, G01, G02, G03, G04, G90, G91), 2 common ‘M’ word commands (M01, M18) and 4 custom (proprietary) commands (M300, M400, M401, M402).  The M300 command is responsible for positioning the pen and appears to be a carryover from the MakerBot Unicorn pen plotter.  The M400 - Reset X-Axis-Stepper settings to new object diameter, M401 - Reset Y-Axis-Stepper settings to new object diameter and M402 - Set global zoom factor are proprietary commands needed to size images for the SphereBot.

The jinschoi/SphereBot fork (https://github.com/jinschoi/SphereBot) of the SphereBot code from which this fork is derived is coded to use the Adafruit motor control shield.  The fork adds 6 custom (proprietary) commands (M301, M302, M303, M500, M501, M503) and removes 2 (M400, M401).  The M400 and M401 commands have been removed as the plotting units have been changed from nominal mm to micro steps. If you are using a 200 steps/revolution stepper, G1 Y3200 will rotate the rotation axis one full turn (using 16x micro stepping). This makes adjusting for object diameter unnecessary and makes it easier to adopt patterns using the EggBot templates, which are 3200 px wide by 800 px tall.  The commands M301 - Set the minimum pen position, M302 - Set the maximum pen position and M303 set the pen up position have been added.  Also added are the commands M500 - Save the pen settings to the Arduino EEPROM, M501 - Load the pen settings from the Arduino EEPROM and M501 - Reset the pen settings to the firmware defaults.

The primary words used in a SphereBot file are G00, G01, G02, G03 and M300. The interesting thing is that while G00 and G01 will position the Z axis this is not used to position the pen.  Instead, the custom M300 command is used to position the pen.  This creates a problem where pen positioning can be both unique to the SphereBot design (hardware) and the object being printed on (i.e. egg vs ping pong ball) such that sharing G-Code files is almost impossible without some kind of modification. 
One of the first challenges is getting G-Code that works on your SphereBot.  Included in this project in the Util/GCode folder are several G-Code files.  They may work out of the box, but chances are you will need to first manual execute M300 commands to determine the best pen up and more importantly pen down settings in order to search and replace the ‘S’ word values for the pen up/down.

For those wanting to try their hand at generating G-Code for the SphereBot there is a solution.  InkScape v1.0 comes prepackages with the Gcodetools extension.  I go into the process of converting a drawing into a line drawing and then G-Code using this extension elsewhere in the documentation.