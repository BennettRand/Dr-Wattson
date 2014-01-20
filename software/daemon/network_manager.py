import sys
import platform
import time
import serial

sys.path.append('../lib')

import data_readers

pan = sum([ord(x) for x in platform.node()]) % 65536

ser = None

def main(argc = len(sys.argv), args = sys.argv):
	
	global ser
	
	ser = serial.Serial(args[1], 38400, timeout = .01)
	
	req_seq = 0
	
	setPAN = data_readers.tx_h.pack(0, 2, pan)
	
	beacon = data_readers.tx_h.pack(data_readers.beacon_p.size, 1, 0xFFFF)
	beacon += data_readers.beacon_p.pack(0, pan, platform.node())
	
	data_request_h = data_readers.tx_h.pack(data_readers.data_req_p.size, 0, 0xFFFF)
	
	print pan
	ser.write(setPAN)
	print '@',
	ser.write(beacon)
	print '!',
	req_seq += 1
	last_sent_b = time.time()
	last_sent_r = time.time()
	
	while True:
		if time.time()-last_sent_b >= 10:
			print '!',
			ser.write(beacon)
			last_sent_b = time.time()
		if time.time()-last_sent_r >= 1:
			ser.write(data_request_h + data_readers.data_req_p.pack(3, req_seq%256))
			print '.',
			req_seq += 1
			last_sent_r = time.time()
		
		read_in = ser.read(4)
		
		if read_in != '':
			rx_h_data = data_readers.rx_h.unpack(read_in)
			print rx_h_data,
			print ser.read(rx_h_data[0]),
		
	
	

if __name__ == "__main__":
	try:
		main()
	except KeyboardInterrupt as e:
		ser.close()
		print "Exiting..."