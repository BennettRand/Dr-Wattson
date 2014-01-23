import sys
import platform
import time
import serial
import json
import math
import psycopg2

sys.path.append('../lib')

conn = psycopg2.connect(database = "wattson", host = "localhost", user = "root", password = "means swim of stream")
g_cur = conn.cursor()

import data_readers

pan = sum([ord(x) for x in platform.node()]) % 65536

ser = None

devices = {}
threads = {}

def addr_to_mac(a):
	s = ''
	
	for x in xrange(4):
		s = str(hex(a&0xF)[2:]) + s
		a = a>>4
	
	return '00-00-00-00-'+s[:2]+'-'+s[2:]

def addr_to_inet(a):
	return '0.0.'+str(a/256)+'.'+str(a%256)

def add_device(dev):
	query = "INSERT INTO device (addr, mac) VALUES (%s, %s) SELECT %s, %s WHERE NOT EXISTS (SELECT mac FROM device WHERE mac = %s)"
	
	g_cur.execute(query, (addr_to_inet(a),addr_to_mac(a),addr_to_inet(a),addr_to_mac(a),addr_to_mac(a)))
	
	return

def calib_dict(t):
	ret = {}
	
	ret['v_scale_1'] = t[1]/10000000.0
	ret['c_scale_1'] = t[2]/10000000.0
	
	ret['v_scale_2'] = t[3]/10000000.0
	ret['c_scale_2'] = t[4]/10000000.0
	
	ret['p_scale'] = t[5]/1000000000.0
	
	return ret
	
def power_dict(d,t):
	ret = {}
	
	# print t
	
	ret['frequency'] = 1.0/(t[6]*devices[str(d)]['calib']['p_scale'])
	
	ret['power_1'] = (t[4]*devices[str(d)]['calib']['v_scale_1']*devices[str(d)]['calib']['c_scale_1'])/t[3]
	ret['voltage_1'] = (math.sqrt(t[7]/t[3])*devices[str(d)]['calib']['v_scale_1'])
	ret['current_1'] = (math.sqrt(t[9]/t[3])*devices[str(d)]['calib']['c_scale_1'])
	
	ret['power_2'] = (t[5]*devices[str(d)]['calib']['v_scale_2']*devices[str(d)]['calib']['c_scale_2'])/t[3]
	ret['voltage_2'] = (math.sqrt(t[8]/t[3])*devices[str(d)]['calib']['v_scale_2'])
	ret['current_2'] = (math.sqrt(t[10]/t[3])*devices[str(d)]['calib']['c_scale_2'])
	
	return ret

def main(argc = len(sys.argv), args = sys.argv):
	
	global ser
	
	ser = serial.Serial(args[1], 38400, timeout = .01)
	
	req_seq = 0
	
	setPAN = data_readers.tx_h.pack(0, 2, pan)
	
	beacon = data_readers.tx_h.pack(data_readers.beacon_p.size, 1, 0xFFFF)
	beacon += data_readers.beacon_p.pack(0, pan, platform.node())
	
	data_request_h = data_readers.tx_h.pack(data_readers.data_req_p.size, 0, 0xFFFF)
	
	cold_start = data_readers.tx_h.pack(data_readers.cold_start_p.size, 0, 0xFFFF)
	cold_start += data_readers.cold_start_p.pack(6)
	
	ser.write(setPAN)
	
	ser.write(cold_start)
	
	req_seq += 1
	last_sent_b = 0
	last_sent_r = 0
	last_db_commit = 0
	
	print "Initialized on:", pan
	while True:
		if time.time()-last_sent_b >= 10:
			ser.write(beacon)
			last_sent_b = time.time()
		if time.time()-last_sent_r >= 1:
			ser.write(data_request_h + data_readers.data_req_p.pack(3, req_seq%256))
			req_seq += 1
			last_sent_r = time.time()
		if time.time()-last_db_commit >= 1:
			conn.commit()
			last_db_commit = time.time()
		
		if ser.inWaiting() >= 4:
			
			read_in = ser.read(4)
			rx_h_data = data_readers.rx_h.unpack(read_in)
			# print rx_h_data
			
			while ser.inWaiting() < rx_h_data[0]: pass
			payload = ser.read(rx_h_data[0])
			type = data_readers.type.unpack_from(payload)[0]
			
			if type == 1:
				conn_r = data_readers.conn_req_p.unpack(payload)
				print "Connection request from", addr_to_mac(rx_h_data[1])
				
				add_device(rx_h_data[1])
				
				devices[addr_to_mac(rx_h_data[1])] = {}
				devices[addr_to_mac(rx_h_data[1])]['calib'] = calib_dict(conn_r)
				devices[addr_to_mac(rx_h_data[1])]['power'] = []
				
				header = data_readers.tx_h.pack(data_readers.conn_ack_p.size,0,rx_h_data[1])
				
				packet = data_readers.conn_ack_p.pack(2)
				
				ser.write(header+packet)
				print "Accepted", addr_to_mac(rx_h_data[1])
			elif type == 4:
				data_e_p = data_readers.data_e_p.unpack(payload)
				print "Data received from", addr_to_mac(rx_h_data[1])
				
				devices[addr_to_mac(rx_h_data[1])]['power'].append(power_dict(addr_to_mac(rx_h_data[1]), data_e_p))
				
				header = data_readers.tx_h.pack(data_readers.data_ack_p.size,0,rx_h_data[1])
				
				packet = data_readers.data_ack_p.pack(5,data_e_p[2])
				
				ser.write(header+packet)
				# print "Sent Data Ack"
			else:
				print payload
	
	

if __name__ == "__main__":
	try:
		main()
	except KeyboardInterrupt as e:
		f = open("collected.json","w")
		json.dump(devices, f, indent = 4, sort_keys=True)
		f.close()
		ser.close()
		conn.commit()
		g_cur.close()
		conn.close()
		print "Exiting..."