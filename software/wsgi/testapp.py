import json
import random
import time
import psycopg2

def get_data(id):
	
	v_start = 120
	i_start = 0
	
	v_arr = []
	i_arr = []
	p_arr = []
	
	start = int(time.time()*1000)
	
	for i in xrange(start, start+3600000, 18000):
		
		v_arr.append([i, v_start])
		i_arr.append([i, i_start])
		p_arr.append([i, v_start*i_start])
		
		v_start += (random.random()*.5)-.25
		i_start += (random.random()*.5)-.25
		v_start = 0 if v_start < 0 else v_start
		i_start = 0 if i_start < 0 else i_start
		
	return [v_arr,i_arr,p_arr]
	
def get_devices()
	devs = [{"name":"Foo","id":1},{"name":"Bar","id":2}]
	
	query = "SELECT (mac,name) FROM device;"
	
	conn = psycopg2.connect(database = 'wattson', host = 'localhost', user = 'root', password = 'means swim of stream')
	cur = conn.cursor()
	
	cur.execute(query)
	
	devs = cur.fetchall()
	
	cur.close()
	conn.close()
	
	return devs

def application(environ, start_response):
	status = '200 OK'
	
	if environ['PATH_INFO'] == "/detail":
		data = get_data(0)
		output = json.dumps(data)
		
	else:
		output = json.dumps(environ, skipkeys=True, default=lambda obj: 'N/A')
	
	response_headers = [('Content-type', 'application/json'),
						('Content-Length', str(len(output)))]
	start_response(status, response_headers)
	
	return [output]
