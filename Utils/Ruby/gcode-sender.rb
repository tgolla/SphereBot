#!/usr/bin/env ruby
# Copyright 2015 Jin Choi (jsc@alum.mit.edu)

# Under Windows:
# Download and install Ruby using rubyinstaller:
# http://rubyinstaller.org/downloads/
# Also download and unpack a DevKit-mingw from that same page.
# I recommend unpacking it into a new directory.

# Open a command window (Windows-r, cmd), cd to the directory you
# unpacked the DevKit. Run "devkitvars.bat".

# Make sure you have the Arduino device drivers installed, by
# installing the Arduino IDE from arduino.cc.

# For all operating systems:
# Run "gem install serialport".
# Edit the port variable below to be the port name of your SphereBot.

# To use: ruby /path/to/gcode-sender.rb file.gcode

require 'serialport'

port = "/dev/cu.usbmodem145311"
# port = "COM3"

filename = ARGV.shift

SerialPort.open(port, 115200, 8, 1, SerialPort::NONE) do |sp|
  sp.read_timeout = 0 # Necessary for Windows.
  while line = sp.gets
    print line
    break if line.match(/^Ready/)
  end
  File.foreach(filename) do |line|
    line.chomp!
    comment = ""
    if md = line.match(/\((.*)\)/)
      comment = md[1]
    end
    line.gsub!(/\s*\(.*\)/, '')
    line.gsub!(/\s*;.*/, '')
    if !line.empty?
      # puts line

      if line == "M01"
        puts "Pausing: (#{comment}) Press Enter to continue."
        gets
        next
      end
      sp.puts line

      while line = sp.gets
        # print "\t#{line}"
        break if line.match(/^ok:/)
      end
    end
  end
end
