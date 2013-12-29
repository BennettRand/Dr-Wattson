#!/usr/bin/python
import os
import sys
import getopt
import re
import string
import subprocess

options, args = getopt.getopt(sys.argv[1:],"m:o:p",["mac", "output", "program"])

program = False
output_fd = sys.stdout
mac_str = "";

for opt, val in options:
	if opt in ("-p", "--program"):
		program = True
	if opt in ("-m", "--mac"):
		mac_str = val
	if opt in ("-o", "--output"):
		output_fd = open(val, 'w')

if program is True:
	output_fd = open(".mac_output.hex", "w")

mac_str = re.sub(r"[^ABCDEF0123456789]", "", string.upper(mac_str))[:16]
if len(mac_str) != 16:
	print("Invalid MAC address")
	quit()

byte_sum = 0x10 + 0x01
output_fd.write(":10010000")
for count in range(8) :
	byte_sum += int(mac_str[-2:],16)
	output_fd.write(mac_str[-2:])
	mac_str = mac_str[:-2]
output_fd.write("FFFFFFFFFFFFFFFF")
byte_sum += 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF + 0xFF
output_fd.write(("%0.2X" % (0x100-(byte_sum & 0xFF))) + "\n")
output_fd.write(":10011000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEF\n" +
                ":10012000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFDF\n" +
                ":10013000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFCF\n" +
                ":10014000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBF\n" +
                ":10015000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFAF\n" +
                ":10016000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF9F\n" +
                ":10017000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF8F\n" +
                ":10018000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7F\n" +
                ":10019000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF6F\n" +
                ":1001A000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF5F\n" +
                ":1001B000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF4F\n" +
                ":1001C000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF3F\n" +
                ":1001D000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF2F\n" +
                ":1001E000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF1F\n" +
                ":1001F000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0F\n" +
                ":10020000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE\n" +
                ":10021000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEE\n" +
                ":10022000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFDE\n" +
                ":10023000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFCE\n" +
                ":10024000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBE\n" +
                ":10025000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFAE\n" +
                ":10026000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF9E\n" +
                ":10027000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF8E\n" +
                ":10028000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7E\n" +
                ":10029000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF6E\n" +
                ":1002A000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF5E\n" +
                ":1002B000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF4E\n" +
                ":1002C000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF3E\n" +
                ":1002D000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF2E\n" +
                ":1002E000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF1E\n" +
                ":1002F000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0E\n" +
                ":10030000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFD\n" +
                ":10031000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED\n" +
                ":10032000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFDD\n" +
                ":10033000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFCD\n" +
                ":10034000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBD\n" +
                ":10035000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFAD\n" +
                ":10036000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF9D\n" +
                ":10037000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF8D\n" +
                ":10038000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7D\n" +
                ":10039000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF6D\n" +
                ":1003A000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF5D\n" +
                ":1003B000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF4D\n" +
                ":1003C000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF3D\n" +
                ":1003D000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF2D\n" +
                ":1003E000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF1D\n" +
                ":1003F000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0D\n" +
                ":00000001FF")
output_fd.close();

if program is True:
	proc = subprocess.Popen(["avrdude", "-patmega256rfr2", "-Pusb", "-cavrisp2", "-Uusersig:w:.mac_output.hex"], stdin=sys.stdin, stdout=sys.stdout)
	proc.wait()
	os.unlink(".mac_output.hex")
