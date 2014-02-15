#!/usr/bin/python2
import os
import sys
import re
import string
import ConfigParser
import subprocess
import StringIO
import math
import csv

print "Loading Calibration Firmware"
# Read in the firmware from the device so we can reprogram it later.
proc = subprocess.Popen(["avrdude", "-patmega256rfr2", "-Pusb", "-cavrisp2", "-q", "-q", "-Uflash:r:.oldFlash.hex:h"], stdin=sys.stdin, stdout=sys.stdout)
proc.wait()
#if (proc.returncode != 0) :
#	exit()

# program calibration firmware onto DUT
proc = subprocess.Popen(["avrdude", "-patmega256rfr2", "-Pusb", "-cavrisp2", "-q", "-q", "-Uflash:w:calibrationTool.hex:h"], stdin=sys.stdin, stdout=sys.stdout)
proc.wait()
#if (proc.returncode != 0) :
#	# If this failed, try to write the old firmware back and exit
#	proc = subprocess.Popen(["avrdude", "-patmega256rfr2", "-Pusb", "-cavrisp2", "-q", "-q", "-Uflash:w:.oldFlash.hex:h"], stdin=sys.stdin, stdout=sys.stdout)
#	proc.wait()
#	exit()

portname = raw_input("Enter serial port name: ")
try :
	#ser = serial.Serial(portname, 115200, timeout = 1)
	ser = open(os.path.expanduser(os.path.expandvars(portname.strip())), 'r+')
except :
	# If this failed, try to write the old firmware back and exit
	proc = subprocess.Popen(["avrdude", "-patmega256rfr2", "-Pusb", "-cavrisp2", "-q", "-q", "-Uflash:w:.oldFlash.hex:h"], stdin=sys.stdin, stdout=sys.stdout)
	proc.wait()
	exit()

# Collect calibration information
V1offset = 0;
I1offset = 0;
V1scaling = 0;
I1scaling = 0;
V2offset = 0;
I2offset = 0;
V2scaling = 0;
I2scaling = 0;

yn = raw_input("Connect AC load to port 1. Press enter when ready (enter s to skip).")
if not (yn.startswith('s')) :
	#ser.write("s")
	s = ser.readline()
	data = "";
	while (s[0] != 'd') :
		data += s
		s = ser.readline()
	
	Vin = raw_input("Enter calibration voltage for port 1: ")
	Iin = raw_input("Enter calibration current for port 1: ")
	
	reader = csv.reader(StringIO.StringIO(data))
	Vsum = 0
	VsquareSum = 0
	Isum = 0
	IsquareSum = 0
	rowcount = 0
	for row in reader:
		Vsum += int(row[1])
		VsquareSum += int(row[1])*int(row[1])
		Isum += int(row[3])
		IsquareSum += int(row[3])*int(row[3])
		rowcount += 1

	V1offset = Vsum/rowcount
	I1offset = Isum/rowcount
	V1scaling = float(Vin)/math.sqrt(VsquareSum/rowcount)	
	I1scaling = float(Iin)/math.sqrt(IsquareSum/rowcount)	

yn = raw_input("Connect AC load to port 2. Press enter when ready (enter s to skip).")
if not (yn.startswith('s')) :
	#ser.write("s")
	s = ser.readline()
	data = "";
	while (s[0] != 'd') :
		data += s
		s = ser.readline()
	
	Vin = raw_input("Enter calibration voltage for port 2: ")
	Iin = raw_input("Enter calibration current for port 2: ")
	
	reader = csv.reader(StringIO.StringIO(data))
	Vsum = 0
	VsquareSum = 0
	Isum = 0
	IsquareSum = 0
	rowcount = 0
	for row in reader:
		Vsum += int(row[1])
		VsquareSum += int(row[1])*int(row[1])
		Isum += int(row[4])
		IsquareSum += int(row[4])*int(row[4])
		rowcount += 1
	
	V2offset = Vsum/rowcount
	I2offset = Isum/rowcount
	V2scaling = float(Vin)/math.sqrt(VsquareSum/rowcount)	
	I2scaling = float(Iin)/math.sqrt(IsquareSum/rowcount)	

ser.close()


print "Voltage 1 Offset: " + str(V1offset)
print "Current 1 Offset: " + str(I1offset)
print "Voltage 1 Scaling Factor: " + str(V1scaling)
print "Current 1 Scaling Factor: " + str(I1scaling)
print "Voltage 2 Offset: " + str(V2offset)
print "Current 2 Offset: " + str(I2offset)
print "Voltage 2 Scaling Factor: " + str(V2scaling)
print "Current 2 Scaling Factor: " + str(I2scaling)

macAddr = raw_input("Enter MAC address for remote monitoring unit: ")
Name = raw_input("Enter default network name: ")
PAN_IN = int(raw_input("Enter PAN ID for default network: "))
Address = int(raw_input("Enter network address for default network base station: "))
configFileName = os.path.expanduser(os.path.expandvars(raw_input("Enter file name for config file")))

config = ConfigParser.RawConfigParser()
configFileName = ""
outputFileSpecified = False
outputFileName = ".tempOutputFile.bin"
program = False

linePeriodScaling = 500
networkValid = False
calValid = True

config.read(configFileName)

try:
	macAddr = config.get('Mac Address', 'address')
except:
	print("Missing MAC address field")
	quit()

try:
	voltageScaling1 = int(config.get('Calibration Data', 'voltageScaling1'))
	voltageOffset1 = int(config.get('Calibration Data', 'voltageOffset1'))
	currentScaling1 = int(config.get('Calibration Data', 'currentScaling1'))
	currentOffset1 = int(config.get('Calibration Data', 'currentOffset1'))
	voltageScaling2 = int(config.get('Calibration Data', 'voltageScaling2'))
	voltageOffset2 = int(config.get('Calibration Data', 'voltageOffset2'))
	currentScaling2 = int(config.get('Calibration Data', 'currentScaling2'))
	currentOffset2 = int(config.get('Calibration Data', 'currentOffset2'))
	linePeriodScaling = int(config.get('Calibration Data', 'linePeriodScaling'))
	calValid = True;
	print("Calibration values found")
except:
	calValid = False;
	print("No calibration values found")

try:
	PAN_ID = int(config.get('Network Information', 'PAN_ID'))
	Address = int(config.get('Network Information', 'Address'))
	Name = config.get('Network Information', 'Name')
	networkValid = True
	print("Network information found")
except:
	print("No network information found")

macAddr = re.sub(r"[^ABCDEF0123456789]", "", string.upper(macAddr))[:16]
if len(macAddr) != 16:
	print("Invalid MAC address")
	quit()
macAddr = int(macAddr,16);

while len(Name) < 16:
	Name += "\0"
Name = Name[:16]

if not outputFileSpecified:
	output_fd = open(".tempOutputFile.bin", 'w')

dataArray = [macAddr & 0xFF, (macAddr>>8) & 0xFF, (macAddr>>16) & 0xFF, (macAddr>>24) & 0xFF, (macAddr>>32) & 0xFF, (macAddr>>40) & 0xFF, (macAddr>>48) & 0xFF, (macAddr>>56) & 0xFF]
if (calValid):
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
else:
	databytearray.extend([0])

output_fd.write(databytearray)
output_fd.close();

if program:
	proc = subprocess.Popen(["avrdude", "-patmega256rfr2", "-Pusb", "-cavrisp2", "-Ueeprom:w:"+outputFileName+":r"], stdin=sys.stdin, stdout=sys.stdout)
	proc.wait()

if not outputFileSpecified:
	os.unlink(".tempOutputFile.bin")

