import sys
import json
from multiprocessing import Process

import psycopg2

conn = psycopg2.connect(database = "wattson", host = "localhost", user = "root", password = "means swim of stream")
g_cur = conn.cursor()
# id from til device_mac v1 v2 i1 i2 p1 p2 f
sample_insert = "INSERT INTO sample VALUES (DEFAULT, to_timestamp(%s), to_timestamp(%s), %s, %s, %s, %s, %s, %s, %s, %s);"

def commit_power(data):
	for d in data:
		for t in data[d]['power']:
			g_cur.execute(sample_insert,(t['t'],t['t'],d,t['v_1'],t['v_2'],t['c_1'],t['c_2'],t['p_1'],t['p_2'],t['f']))
	conn.commit()
	
data = {}
def empty_power():
	global data
	for d in data:
		del data[d]['power']
		data[d]['power'] = []
	
def main(argc = len(sys.argv), args = sys.argv):
	global data
	f = open("collected.json")
	
	data = json.load(f)
	
	commit_p = Process(target = commit_power, args=(data,))
	
	for d in data:
		print len(data[d]['power'])
	
	commit_p.start()
	empty_power()
	commit_p.join()
	
	for d in data:
		print len(data[d]['power'])
	
	return

if __name__ == "__main__":
	main()