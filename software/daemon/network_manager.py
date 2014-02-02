import sys
import platform
import time
import serial
import json
import math
import psycopg2
from multiprocessing import Pool, freeze_support
import signal
import copy

sys.path.append('../lib')


import data_readers

pan = sum([ord(x) for x in platform.node()]) % 65536

ser = None

# id from til device_mac v1 v2 i1 i2 p1 p2 f
sample_insert = "INSERT INTO sample VALUES (DEFAULT, to_timestamp(%s), to_timestamp(%s), %s, %s, %s, %s, %s, %s, %s, %s);"

def init_worker():
	signal.signal(signal.SIGINT, signal.SIG_IGN)
	sys.stderr = open('err.out','w')
	sys.stdout = open('std.out','w')

def call_b(data):
	if data != None:
		for d in data:
			print len(data[d]['power']),
	else:
		print "None",
	print "Done",time.asctime()

def commit_power(data, conn, g_cur):
	if data != None:
		for d in data:
			for t in data[d]['power']:
				g_cur.execute(sample_insert,(t['t'],t['t'],d,t['v_1'],t['v_2'],t['i_1'],t['i_2'],t['p_1'],t['p_2'],t['f']))
		conn.commit()
	return data

# commit_p = None#Pool()#(target = commit_power, args=(devices,))

def empty_power():
	global devices
	for d in devices:
		del devices[d]['power']
		devices[d]['power'] = []

def addr_to_mac(a):
	s = ''
	
	for x in xrange(4):
		s = str(hex(a&0xF)[2:]) + s
		a = a>>4
	
	return '00-00-00-00-'+s[:2]+'-'+s[2:]

def addr_to_inet(a):
	return '0.0.'+str(a/256)+'.'+str(a%256)

def add_device(a, g_cur):
	query = '''INSERT INTO device (addr, mac)
	SELECT %s, %s
	WHERE NOT EXISTS (SELECT mac FROM device WHERE mac = %s)'''
	
	g_cur.execute(query, (addr_to_inet(a),addr_to_mac(a),addr_to_mac(a)))
	
	return

def calib_dict(t):
	ret = {}
	
	ret['v_scale_1'] = t[1]/1000000.0
	ret['c_scale_1'] = t[2]/10000000.0
	
	ret['v_scale_2'] = t[3]/1000000.0
	ret['c_scale_2'] = t[4]/10000000.0
	
	ret['p_scale'] = t[5]/1000000000.0
	
	return ret
	
def power_dict(d,t):
	ret = {}
	
	# print t
	
	ret['t'] = time.time()
	
	ret['f'] = 1.0/(t[6]*devices[str(d)]['calib']['p_scale'])
	
	ret['p_1'] = (t[4]*devices[str(d)]['calib']['v_scale_1']*devices[str(d)]['calib']['c_scale_1'])/t[3]
	ret['v_1'] = (math.sqrt(t[7]/t[3])*devices[str(d)]['calib']['v_scale_1'])
	ret['i_1'] = (math.sqrt(t[9]/t[3])*devices[str(d)]['calib']['c_scale_1'])
	
	ret['p_2'] = (t[5]*devices[str(d)]['calib']['v_scale_2']*devices[str(d)]['calib']['c_scale_2'])/t[3]
	ret['v_2'] = (math.sqrt(t[8]/t[3])*devices[str(d)]['calib']['v_scale_2'])
	ret['i_2'] = (math.sqrt(t[10]/t[3])*devices[str(d)]['calib']['c_scale_2'])
	
	return ret

def main(commit_p, conn, g_cur, argc = len(sys.argv), args = sys.argv):
	
	global ser
	global devices
	
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
		if time.time()-last_db_commit >= 2:
			# if commit_p.is_alive(): commit_p.join()
			data = copy.deepcopy(devices)
			print "Spawn DB sync"
			commit_p.apply_async(commit_power, args=(data,),callback=call_b)
			empty_power()
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
				
				add_device(rx_h_data[1], g_cur)
				
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
	global devices
	devices = {}
	conn = psycopg2.connect(database = "wattson", host = "localhost", user = "root", password = "means swim of stream")
	print "Connected to",conn
	g_cur = conn.cursor()
	if platform.system() == "Windows":
		freeze_support()
	commit_p = Pool(4,init_worker)
	try:
		main(commit_p, conn, g_cur)
	except KeyboardInterrupt as e:
		# if commit_p.is_alive(): commit_p.join()
		data = copy.deepcopy(devices)
		commit_p.apply_async(commit_power, args=(data,conn, g_cur),callback=call_b)
		empty_power()
		commit_p.close()
		commit_p.join()
		f = open("collected.json","w")
		json.dump(devices, f, indent = 4, sort_keys=True)
		f.close()
		ser.close()
		conn.commit()
		g_cur.close()
		conn.close()
		print "Exiting..."
