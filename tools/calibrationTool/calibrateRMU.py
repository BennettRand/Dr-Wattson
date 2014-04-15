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
import time
import serial

def signExtend16(val):
	if (val & 0x8000) :
		return ((val^0xFFFF)+1)*-1
	else :
		return val

boardRev = 0
boardRev = int(raw_input("Enter board revision you are calibrating: "))
if (boardRev != 1) and (boardRev != 2) :
	print "Invalid board revision: " + str(boardRev)
	exit


v1_index = 0
i1_index = 0
v2_index = 0
i2_index = 0
if (boardRev == 1) :
	v1_index = 1
	v2_index = 1
	i1_index = 3
	i2_index = 4
if (boardRev == 2) :
	v1_index = 3
	v2_index = 3
	i1_index = 1
	i2_index = 2

print "Reading existing calibration/configuration information..."
proc = subprocess.Popen(["avrdude", "-patmega256rfr2", "-Pusb", "-cavrisp2", "-q", "-q", "-Ueeprom:r:.oldcal.bin:r"], stdin=sys.stdin, stdout=sys.stdout)
proc.wait()
if (proc.returncode != 0):
	print "Failed to connect to remote monitoring unit"
	exit()

oldconfig = open(".oldcal.bin", 'r')
rawChunk = bytearray(oldconfig.read(8))
macAddr = (rawChunk[0]) | (rawChunk[1] << 8) | (rawChunk[2] << 16) | (rawChunk[3] << 24) | (rawChunk[4]<<32) | (rawChunk[5]<<40) | (rawChunk[6]<<48) | (rawChunk[7]<<56)
rawChunk = bytearray(oldconfig.read(2))
old_voltageScaling1 = rawChunk[0] | (rawChunk[1]<<8)
rawChunk = bytearray(oldconfig.read(2))
old_voltageOffset1 = signExtend16(rawChunk[0] | (rawChunk[1]<<8))
rawChunk = bytearray(oldconfig.read(2))
old_currentScaling1 = signExtend16(rawChunk[0] | (rawChunk[1]<<8))
rawChunk = bytearray(oldconfig.read(2))
old_currentOffset1 = signExtend16(rawChunk[0] | (rawChunk[1]<<8))
rawChunk = bytearray(oldconfig.read(2))
old_voltageScaling2 = signExtend16(rawChunk[0] | (rawChunk[1]<<8))
rawChunk = bytearray(oldconfig.read(2))
old_voltageOffset2 = signExtend16(rawChunk[0] | (rawChunk[1]<<8))
rawChunk = bytearray(oldconfig.read(2))
old_currentScaling2 = signExtend16(rawChunk[0] | (rawChunk[1]<<8))
rawChunk = bytearray(oldconfig.read(2))
old_currentOffset2 = signExtend16(rawChunk[0] | (rawChunk[1]<<8))
rawChunk = bytearray(oldconfig.read(2))
old_linePeriodScaling = signExtend16(rawChunk[0] | (rawChunk[1]<<8))
networkValid = False
PAN_ID = 0
Address = 0
Name = ""
rawChunk = bytearray(oldconfig.read(1))
if (rawChunk[0] == 0xFF):
	rawChunk = bytearray(oldconfig.read(5))
	PAN_ID = rawChunk[0] | (rawChunk[1]<<8)
	Address = rawChunk[3] | (rawChunk[4]<<8)
	Name = oldconfig.read(16).strip().strip('\0')
	networkValid = True

oldconfig.close()

print "Original Configuration Data:"
print "    MAC Address:         " + '0x{0:08X}'.format(macAddr)
print "    Voltage 1 Scaling:   " + str(old_voltageScaling1)
print "    Voltage 1 Offset:    " + str(old_voltageOffset1)
print "    Current 1 Scaling:   " + str(old_currentScaling1)
print "    Current 1 Offset:    " + str(old_currentOffset1)
print "    Voltage 2 Scaling:   " + str(old_voltageScaling2)
print "    Voltage 2 Offset:    " + str(old_voltageOffset2)
print "    Current 2 Scaling:   " + str(old_currentScaling2)
print "    Current 2 Offset:    " + str(old_currentOffset2)
print "    Line Period Scaling: " + str(old_linePeriodScaling)
if (networkValid) :
	print "    PAN_ID:              " + str(PAN_ID)
	print "    Address:             " +str(Address)
	print "    Name:                " + Name

print "Loading Calibration Firmware..."
# program calibration firmware onto DUT
if (boardRev == 1) :
	proc = subprocess.Popen(["avrdude", "-patmega256rfr2", "-Pusb", "-cavrisp2", "-Uflash:w:calibrationTool_v1.hex:i"], stdin=sys.stdin, stdout=sys.stdout)
if (boardRev == 2) :
	proc = subprocess.Popen(["avrdude", "-patmega256rfr2", "-Pusb", "-cavrisp2", "-Uflash:w:calibrationTool_v2.hex:i"], stdin=sys.stdin, stdout=sys.stdout)
proc.wait()
if (proc.returncode != 0) :
	# If this failed, try to write the old firmware back and exit
	print "Failed to write calibration firmware:" + str(proc.returncode)
	exit()

print "Starting calibration procedure..."
portname = raw_input("    Enter serial port name: ")
try :
	ser = serial.Serial(portname.strip(),115200)
	#ser = open(os.path.expanduser(os.path.expandvars(portname.strip())), 'r+')
except :
	print "Could not open serial port"
	exit()

# Collect calibration information

Imoff = float(raw_input("    Enter current meter offset: "))

yn = raw_input("    Connect AC load to port 1. Press enter when ready (enter s to skip).")
if not (yn.startswith('s')) :
	ser.write("s")
	s = ser.readline()
	data = "";
	while (s[0] != 'd') :
		data += s
		s = ser.readline()
	print data;
	Vin = raw_input("    Enter calibration voltage for port 1: ")
	Iin = raw_input("    Enter calibration current for port 1: ")
	
	reader = csv.reader(StringIO.StringIO(data))
	Vsum = 0
	Isum = 0
	rowcount = 0
	for row in reader:
		Vsum += int(row[v1_index])
		Isum += int(row[i1_index])
		rowcount += 1

	voltageOffset1 = int(round(Vsum/rowcount))
	currentOffset1 = int(round(Isum/rowcount))


	reader = csv.reader(StringIO.StringIO(data))
	VsquareSum = 0
	IsquareSum = 0
	for row in reader:
		VsquareSum += (int(row[v1_index]) - voltageOffset1)*(int(row[v1_index]) - voltageOffset1)
		IsquareSum += (int(row[i1_index]) - currentOffset1)*(int(row[i1_index]) - currentOffset1)

	voltageScaling1 = int(round((float(Vin)/math.sqrt(VsquareSum/rowcount))*1000000))
	currentScaling1 = int(round(((float(Iin)-Imoff)/math.sqrt(IsquareSum/rowcount))*10000000))
else:
	voltageOffset1    = old_voltageOffset1
	currentOffset1    = old_currentOffset1
	voltageScaling1   = old_voltageScaling1
	currentScaling1   = old_currentScaling1



yn = raw_input("    Connect AC load to port 2. Press enter when ready (enter s to skip).")
if not (yn.startswith('s')) :
	ser.write("s")
	s = ser.readline()
	data = "";
	while (s[0] != 'd') :
		data += s
		s = ser.readline()

	print data	
	Vin = raw_input("    Enter calibration voltage for port 2: ")
	Iin = raw_input("    Enter calibration current for port 2: ")
	reader = csv.reader(StringIO.StringIO(data))
	Vsum = 0
	Isum = 0
	rowcount = 0
	for row in reader:
		Vsum += int(row[v2_index])
		Isum += int(row[i2_index])
		rowcount += 1
	
	voltageOffset2 = int(round(Vsum/rowcount))
	currentOffset2 = int(round(Isum/rowcount))

	reader = csv.reader(StringIO.StringIO(data))
	VsquareSum = 0
	IsquareSum = 0
	for row in reader:
		VsquareSum += (int(row[v2_index]) - voltageOffset2)*(int(row[v2_index]) - voltageOffset2)
		IsquareSum += (int(row[i2_index]) - currentOffset2)*(int(row[i2_index]) - currentOffset2)
	
	voltageScaling2 = int(round((float(Vin)/math.sqrt(VsquareSum/rowcount))*1000000))
	currentScaling2 = int(round(((float(Iin)-Imoff)/math.sqrt(IsquareSum/rowcount))*10000000))
else:
	voltageOffset2    = old_voltageOffset2
	currentOffset2    = old_currentOffset2
	voltageScaling2   = old_voltageScaling2
	currentScaling2   = old_currentScaling2

ser.close()

linePeriodScaling = 500

print "\nCalibration Parameters:  Original Values:   New Values:"
print "    Voltage 1 Offset:    " + str(old_voltageOffset1).ljust(19) +  str(voltageOffset1)
print "    Current 1 Offset:    " + str(old_currentOffset1).ljust(19) +  str(currentOffset1)
print "    Voltage 1 Scaling:   " + str(old_voltageScaling1).ljust(19) + str(voltageScaling1)
print "    Current 1 Scaling:   " + str(old_currentScaling1).ljust(19) + str(currentScaling1)
print "    Voltage 2 Offset:    " + str(old_voltageOffset2).ljust(19) +  str(voltageOffset2)
print "    Current 2 Offset:    " + str(old_currentOffset2).ljust(19) +  str(currentOffset2)
print "    Voltage 2 Scaling:   " + str(old_voltageScaling2).ljust(19) + str(voltageScaling2)
print "    Current 2 Scaling:   " + str(old_currentScaling2).ljust(19) + str(currentScaling2)
print "    Line Period Scaling: " + str(old_linePeriodScaling).ljust(19) + str(linePeriodScaling)

yn = raw_input("Would you program these values (y/n)? ")
if not (yn.lower().startswith("y")):
	exit()

if networkValid:
	new_macAddr = raw_input("    Enter MAC address for remote monitoring unit (default: "+'0x{0:08X}'.format(macAddr)+"): ")
	if (new_macAddr.strip() != ""):
		macAddr = new_macAddr.strip()
	
	new_PAN_ID = raw_input("    Enter PAN ID for default network (default: "+str(PAN_ID)+"): ")
	if (new_PAN_ID.strip() != ""):
		PAN_ID = int(PAN_ID.strip())
	
	new_Address = raw_input("    Enter network address for default network base station (default: "+str(Address)+"): ")
	if (new_Address.strip() != ""):
		Address = int(new_Address.strip())

	new_Name = raw_input(("    Enter default network name (default: "+Name)+"): ")
	if (new_Name.strip() != ""):
		Name = new_Name
else:
	macAddr = raw_input("    Enter MAC address for remote monitoring unit: ")
	PAN_IN = int(raw_input("    Enter PAN ID for default network: "))
	Address = int(raw_input("    Enter network address for default network base station: "))
	Name = raw_input("    Enter default network name: ")
configFileName = os.path.expanduser(os.path.expandvars(raw_input("    Enter file name for config file: ")))

config = ConfigParser.SafeConfigParser()

print "Writing Configuration File..."
config.add_section("Mac Address")
config.set("Mac Address","address",'{0:08X}'.format(macAddr))

config.add_section("Calibration Data")
config.set('Calibration Data', 'voltageScaling1',str(voltageScaling1))
config.set('Calibration Data', 'voltageOffset1',str(voltageOffset1))
config.set('Calibration Data', 'currentScaling1',str(currentScaling1))
config.set('Calibration Data', 'currentOffset1',str(currentOffset1))
config.set('Calibration Data', 'voltageScaling2',str(voltageScaling2))
config.set('Calibration Data', 'voltageOffset2',str(voltageOffset2))
config.set('Calibration Data', 'currentScaling2',str(currentScaling2))
config.set('Calibration Data', 'currentOffset2',str(currentOffset2))
config.set('Calibration Data', 'linePeriodScaling',str(linePeriodScaling))

config.add_section("Network Information")
config.set('Network Information', 'PAN_ID', str(PAN_ID))
config.set('Network Information', 'Address', str(Address))
config.set('Network Information', 'Name', str(Name))

cfgFile = open(configFileName,'w')
config.write(cfgFile)
cfgFile.close()

print "Programming new configuration..."
while len(Name) < 16:
	Name += "\0"
Name = Name[:16]

output_fd = open(".outputFile.bin", 'w')

dataArray = [macAddr & 0xFF, (macAddr>>8) & 0xFF, (macAddr>>16) & 0xFF, (macAddr>>24) & 0xFF, (macAddr>>32) & 0xFF, (macAddr>>40) & 0xFF, (macAddr>>48) & 0xFF, (macAddr>>56) & 0xFF]
dataArray += [voltageScaling1 & 0xFF, (voltageScaling1>>8) & 0xFF]
dataArray += [voltageOffset1 & 0xFF, (voltageOffset1>>8) & 0xFF]
dataArray += [currentScaling1 & 0xFF, (currentScaling1>>8) & 0xFF]
dataArray += [currentOffset1 & 0xFF, (currentOffset1>>8) & 0xFF]
dataArray += [voltageScaling2 & 0xFF, (voltageScaling2>>8) & 0xFF]
dataArray += [voltageOffset2 & 0xFF, (voltageOffset2>>8) & 0xFF]
dataArray += [currentScaling2 & 0xFF, (currentScaling2>>8) & 0xFF]
dataArray += [currentOffset2 & 0xFF, (currentOffset2>>8) & 0xFF]
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

proc = subprocess.Popen(["avrdude", "-patmega256rfr2", "-Pusb", "-cavrisp2", "-Ueeprom:w:.outputFile.bin:r"], stdin=sys.stdin, stdout=sys.stdout)
proc.wait()

os.unlink(".outputFile.bin")
os.unlink(".oldcal.bin")

