#!/usr/bin/env ruby

require 'serialport'

filename = ARGV.shift


SerialPort.open("/dev/cu.usbmodem1411", 115200, 8, 1, SerialPort::NONE) do |sp|
  while line = sp.gets
    print line
    break if line.match(/^Ready/)
  end
  File.foreach(filename) do |line|
    line.chomp!
    line.gsub!(/\(.*\)/, '')
    line.gsub!(/;.*/, '')
    if !line.empty?
      puts line
      sp.puts line

      while line = sp.gets
        print "\t#{line}"
        break if line.match(/^ok:/)
      end
    end
  end
end
