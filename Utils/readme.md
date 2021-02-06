# Utilities

This folder contains scripts designed to send G-code files to your SphereBot, an Eggbot clone.

Contained in the folder is the Ruby gcode-sender added by Jin Choi (jsc@alum.mit.edu) from the code this fork was created.

I have also restored the python script from the original repository created by Eberhard Rensch.  While the Ruby script is cleaner and easier to read, Python is used by many more DIY hobbyist on a range of devices including Arduino.  Python is also easier to install on Windows.  The script has also been upgraded to Python 3.

Instruction to install Ruby are in the beginning comments of the gcode-sender.py file.  I have not tried these so if you do encounter problems please document the corrections in a GitHub issue so I can update the fork.

Python can be installed by either going to https://www.python.org/ to download the latest version or the Microsoft Store and searching on Python.  The Python folder has also been configured to be opened with Visual Studio Code.  If you do use VS Code you will want to load the Microsoft extensions for Python (ms-python.python and  ms-python.vscode-pylance). 

Finally, you will find some sample G-code files (.ngc) in the GCode folder which you can test with.
