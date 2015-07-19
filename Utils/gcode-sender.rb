#!/usr/bin/env ruby

require 'serialport'

filename = ARGV.shift


SerialPort.open("/dev/cu.usbmodem145311", 115200, 8, 1, SerialPort::NONE) do |sp|
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
