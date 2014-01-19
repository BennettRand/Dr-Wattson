#!/usr/bin/python2
import os
import sys
import getopt
import re
import string
import ConfigParser
import subprocess

options, args = getopt.getopt(sys.argv[1:],"c:o:hp",["config", "output", "program"])

config = ConfigParser.RawConfigParser()
configFileName = ""
outputFileSpecified = False
outputFileName = ".tempOutputFile.bin"
program = False

macAddr = ''
voltageScaling1 = 0
voltageOffset1 = 0
currentScaling1 = 0
currentOffset1 = 0
voltageScaling2 = 0
voltageOffset2 = 0
currentScaling2 = 0
currentOffset2 = 0
linePeriodScaling = 0
PAN_ID = 0
Address = 0
Name = ''
networkValid = False

for opt, val in options:
	if opt in ("-c", "--config"):
		configFileName = val
	if opt in ("-o", "--output"):
		output_fd = open(val, 'w')
		outputFileName = val
		outputFileSpecified = True
	if opt in ("-p", "--program"):
		program = True
	if opt in ("-h") :
		print("Usage : " + sys.argv[0] + " [-c config file] <-o output file> <-p>")
		print("      -c  --config     Configuration file")
		print("      -o  --output     Output file for generated segment")
		print("      -p  --program    Program config to device using avrdude")

config.read(configFileName)
if (configFileName == "") :
	print("Usage : " + sys.argv[0] + " [-c config] <-o output file>")
	print("      -c  --config     Configuration file")
	print("      -o  --output     Output file for generated segment")
	print("      -p  --program    Program config to device using avrdude")

try:
	macAddr = config.get('Mac Address', 'address')
	voltageScaling1 = int(config.get('Calibration Data', 'voltageScaling1'))
	voltageOffset1 = int(config.get('Calibration Data', 'voltageOffset1'))
	currentScaling1 = int(config.get('Calibration Data', 'currentScaling1'))
	currentOffset1 = int(config.get('Calibration Data', 'currentOffset1'))
	voltageScaling2 = int(config.get('Calibration Data', 'voltageScaling2'))
	voltageOffset2 = int(config.get('Calibration Data', 'voltageOffset2'))
	currentScaling2 = int(config.get('Calibration Data', 'currentScaling2'))
	currentOffset2 = int(config.get('Calibration Data', 'currentOffset2'))
	linePeriodScaling = int(config.get('Calibration Data', 'linePeriodScaling'))
except:
	print("Missing values in config file")
	quit()

try:
	PAN_ID = int(config.get('Network Information', 'PAN_ID'))
	Address = int(config.get('Network Information', 'Address'))
	Name = config.get('Network Information', 'Name')
	networkValid = True
except:
	print("No Network Information")

macAddr = re.sub(r"[^ABCDEF0123456789]", "", string.upper(macAddr))[:16]
if len(macAddr) != 16:
	print("Invalid MAC address")
	quit()
macAddr = int(macAddr,16);

if not outputFileSpecified:
	output_fd = open(".tempOutputFile.bin", 'w')

dataArray = [macAddr & 0xFF, (macAddr>>8) & 0xFF, (macAddr>>16) & 0xFF, (macAddr>>24) & 0xFF, (macAddr>>32) & 0xFF, (macAddr>>40) & 0xFF, (macAddr>>48) & 0xFF, (macAddr>>56) & 0xFF]
dataArray += [voltageScaling1 & 0xFF, (voltageScaling1>>8) & 0xFF]
dataArray += [voltageOffset1 & 0xFF, (voltageOffset1>>8) & 0xFF]
dataArray += [currentScaling1 & 0xFF, (currentScaling1>>8) & 0xFF]
dataArray += [currentOffset1 & 0xFF, (currentOffset1>>8) & 0xFF]
dataArray += [voltageScaling1 & 0xFF, (voltageScaling1>>8) & 0xFF]
dataArray += [voltageOffset1 & 0xFF, (voltageOffset1>>8) & 0xFF]
dataArray += [currentScaling1 & 0xFF, (currentScaling1>>8) & 0xFF]
dataArray += [currentOffset1 & 0xFF, (currentOffset1>>8) & 0xFF]
dataArray += [linePeriodScaling & 0xFF, (linePeriodScaling >> 8) & 0xFF]
databytearray = bytearray(dataArray)
if (networkValid):
	dataArray = [0xFF]
	dataArray += [PAN_ID & 0xFF, (PAN_ID>>8) & 0xFF]
	dataArray += [1];
	dataArray += [Address & 0xFF, (Address>>8) & 0xFF]
	databytearray.extend(bytearray(dataArray))
	databytearray.extend(Name)

output_fd.write(databytearray)
output_fd.close();

if program:
	proc = subprocess.Popen(["avrdude", "-patmega256rfr2", "-Pusb", "-cavrisp2", "-Ueeprom:w:"+outputFileName+":r"], stdin=sys.stdin, stdout=sys.stdout)
	proc.wait()

if not outputFileSpecified:
	os.unlink(".tempOutputFile.bin")

