import sys
import json
import struct

def dummy_data(size):
	
	s = ""
	for x in xrange(size):
		s+=chr(x%256)
		
	return s

formats = json.load(open("struct_formats.json"))

# structs = {f:struct.Struct(str(formats[f])) for f in formats}

for f in formats:
	globals()[f] = struct.Struct(str(formats[f]))

def main(argc = len(sys.argv), args = sys.argv):
	print formats,'\n'
	print globals(),'\n'
	print beacon_p.size
	return

if __name__ == "__main__":
	main()