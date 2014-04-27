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
import ConfigParser

sys.path.append('../lib')


import data_readers

config = ConfigParser.SafeConfigParser()
config.read("../cfg.ini")

pan = sum([ord(x) for x in platform.node()]) % 65536

ser = None

# id from til device_mac v1 v2 i1 i2 p1 p2 f

def init_worker():
	signal.signal(signal.SIGINT, signal.SIG_IGN)
	# sys.stderr = open('err.out','a')
	# sys.stdout = open('std.out','a')

def call_b(data):
	if data != None:
		for d in data:
			print d,len(data[d]['power'])
	else:
		print "None",
	print "Done",time.asctime()

def commit_power(data):
	conn = psycopg2.connect(database = "wattson", host = "localhost", user = "root", password = "means swim of stream")
	cur = conn.cursor()
	sample_insert_statement = "INSERT INTO sample VALUES "
	sample_insert_format = "(DEFAULT, to_timestamp(%s), to_timestamp(%s), %s, %s, %s, %s, %s, %s, %s, %s)"
	#f = open(config.get("Logs","network_log")+".err",'w+')
	# f.write(time.asctime()+'\n')
	# f.write(str(conn)+'\n')
	# f.flush()
	inserts = []
	try:
		if data != None:
			for d in data:
				for t in data[d]['power']:
					inserts.append(cur.mogrify(sample_insert_format,(t['t'],t['t'],d,t['v_1'],t['v_2'],t['i_1'],t['i_2'],t['p_1'],t['p_2'],t['f'])))
			query = sample_insert_statement+',\n'.join(inserts)+";"
			cur.execute(query)
			conn.commit()

	except Exception as e:
		#f.write(str(e)+'\n')
		#f.flush()
		#f.close()
		cur.close()
		conn.close()
		return None
	else:
		cur.close()
		conn.close()
		# f.close()
		return data

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
	if t[3] == 0 or t[6] == 0:
		return None
	
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
	
	#Confirm PostgreSQL connection.
	print "Connected to",conn
	
	#What OS am I on?
	if platform.system() == "Windows":
		ser_s = config.get("Serial","windows_device")
	else:
		ser_s = config.get("Serial","posix_device")
	
	#Create commonly used serial packets.
	beacon = data_readers.tx_h.pack(data_readers.beacon_p.size, 1, 0xFFFF)
	beacon += data_readers.beacon_p.pack(0, pan, platform.node())
	
	data_request_h = data_readers.tx_h.pack(data_readers.data_req_p.size, 0, 0xFFFF)
	
	cold_start = data_readers.tx_h.pack(data_readers.cold_start_p.size, 0, 0xFFFF)
	cold_start += data_readers.cold_start_p.pack(6)
	
	#Open serial port to mesh network controller.
	ser = serial.Serial(ser_s, config.getint("Serial","baud"), timeout = config.getfloat("Serial","timeout"))
	
	#Set the PAN of the mesh network controller.
	setPAN = data_readers.tx_h.pack(0, 2, pan)
	ser.write(setPAN)
	
	#Soft-restart all listening power monitors.
	ser.write(cold_start)
	
	#Initialize some timers and counters.
	req_seq = 1
	last_sent_b = 0
	last_sent_r = 0
	last_db_commit = 0
	
	#All ready!
	print "Initialized on:", pan
	
	while True:
		if time.time()-last_sent_b >= 10:		#Every 10 seconds, tell the controller to broadcast a beacon.
			ser.write(beacon)
			last_sent_b = time.time()
		if time.time()-last_sent_r >= 1:		#Every second, request data from all listening monitors.
			ser.write(data_request_h + data_readers.data_req_p.pack(3, req_seq%256))
			for d in devices:
				devices[d]['last_seen'][req_seq%256] = 1
			req_seq += 1
			last_sent_r = time.time()
		if time.time()-last_db_commit >= 10:	#Every 10 seconds, commit all existing data to the PostgreSQL DB.
			data = copy.deepcopy(devices)		#Windows passed a dictionary as a deepcopy, Linux did not. This fixes that.
			commit_p.apply_async(commit_power, args=(data,),callback=call_b)	#Makes a process from an existing worker pool call 'commit_power'
																				#with the argument 'data'. Calls 'call_b' when done.
			empty_power()						#Empties the 'devices' dictionary cleanly.
			last_db_commit = time.time()
		
		if ser.inWaiting() >= 4:				#Make sure there is at least a header in the serial buffer.
			
			read_in = ser.read(4)
			rx_h_data = data_readers.rx_h.unpack(read_in)	#Extract the header.
			
			while ser.inWaiting() < rx_h_data[0]: pass		#Wait for the rest of the expected packet.
			payload = ser.read(rx_h_data[0])				#Read it.
			type = data_readers.type.unpack_from(payload)[0]#What type is it?
			
			if type == 1:									#Connection request.
				conn_r = data_readers.conn_req_p.unpack(payload)
				print "Connection request from", addr_to_mac(rx_h_data[1])
				
				add_device(rx_h_data[1], g_cur)				#Add device to database, if it does not exist.
				conn.commit()
				
				devices[addr_to_mac(rx_h_data[1])] = {}		#Add device to data dictionary.
				devices[addr_to_mac(rx_h_data[1])]['calib'] = calib_dict(conn_r)
				devices[addr_to_mac(rx_h_data[1])]['power'] = []
				devices[addr_to_mac(rx_h_data[1])]['last_seen'] = [0]*256
				
				header = data_readers.tx_h.pack(data_readers.conn_ack_p.size,0,rx_h_data[1])	#Acknowledge connection
				
				packet = data_readers.conn_ack_p.pack(2)
				
				ser.write(header+packet)
				print "Accepted", addr_to_mac(rx_h_data[1])
			elif type == 4:									#Power data
				data_e_p = data_readers.data_e_p.unpack(payload)
				
				r_sequence = data_e_p[1]
				sequence = data_e_p[2]
				
				#Put the received data into the power data dictionary.
				if addr_to_mac(rx_h_data[1]) in devices:
					if devices[addr_to_mac(rx_h_data[1])]['last_seen'][r_sequence] == 0:
						print "!",addr_to_mac(rx_h_data[1])[-2:],
					else:
						print addr_to_mac(rx_h_data[1])[-2:],
						d = power_dict(addr_to_mac(rx_h_data[1]), data_e_p)
						if d!= None: devices[addr_to_mac(rx_h_data[1])]['power'].append(d)
						
						header = data_readers.tx_h.pack(data_readers.data_ack_p.size,0,rx_h_data[1])	#Ack data
						
						packet = data_readers.data_ack_p.pack(5,sequence)
						
						devices[addr_to_mac(rx_h_data[1])]['last_seen'][r_sequence] = 0
						ser.write(header+packet)
			else:
				print payload								#Unknown, just print it.
	
	

if __name__ == "__main__":
	global devices
	devices = {}
	conn = psycopg2.connect(database = "wattson", host = "localhost", user = "root", password = "means swim of stream")
	g_cur = conn.cursor()
	if platform.system() == "Windows":
		freeze_support()
	commit_p = Pool(4,init_worker)
	try:
		main(commit_p, conn, g_cur)
	except KeyboardInterrupt as e:
		# if commit_p.is_alive(): commit_p.join()
		data = copy.deepcopy(devices)
		commit_p.apply_async(commit_power, args=(data,),callback=call_b)
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
