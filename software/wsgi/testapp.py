import json
import random
import time

def get_data(id)
	
	v_start = 120
	i_start = 0
	
	v_arr = []
	i_arr = []
	p_arr = []
	
	start = time.time()
	
	for i in xrange(start, start+3600000, 18000):
		
		v_arr.append([i, v_start])
		i_arr.append([i, i_start])
		p_arr.append([i, v_start*i_start])
		
		v_start += (random.random()*.5)-.25
		i_start += (random.random()*.5)-.25
		v_start = if v_start<0 then 0 else v_start
		i_start = if i_start<0 then 0 else i_start
		
	return [v_arr,i_arr,p_arr]

def application(environ, start_response):
	status = '200 OK'
	
	data = get_data(0)
	
	output = json.dumps(data)
	
	response_headers = [('Content-type', 'application/json'),
						('Content-Length', str(len(output)))]
	start_response(status, response_headers)
	
	return [output]
