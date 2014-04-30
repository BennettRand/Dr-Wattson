import json
import random
import time
import psycopg2
import itertools
import datetime
import math
import urllib

def segments_to_string(n):
	return "{0} hours {1} minutes".format(n/6, (n%6)*10)
	
def dt_to_epoch(d):
	return (d-datetime.datetime(1969,12,31,17)).total_seconds()
	
def sparkline(id):
	
	if id[-1] == 'a': offset = 0
	elif id[-1] == 'b': offset = 1
	else: return []
	
	query = "SELECT mac FROM device WHERE id="+id[:-1]+";"
	
	conn = psycopg2.connect(database = 'wattson', host = 'localhost', user = 'root', password = 'means swim of stream')
	cur = conn.cursor()
	
	cur.execute(query)
	
	devs = cur.fetchone()
	
	if devs == None:
		return query
	
	data = devs[0]
	
	# query = "SELECT til,v_1,v_2,i_1,i_2,p_1,p_2,f FROM sample WHERE device_mac='"+data+"' AND til > now() - interval '1 hour' ORDER BY til ASC;"
	# query = "SELECT til,p_1,p_2 FROM sample WHERE device_mac='"+data+"' AND til > now() - interval '1 hour' ORDER BY til ASC;"
	
	query = """
	SELECT t.*
	FROM (
	  SELECT til,p_1,p_2,row_number() OVER(ORDER BY til ASC) AS row
	  FROM sample 
	  WHERE device_mac='{0}' AND til > now() - interval '1 hour' 
	  ORDER BY til ASC
	) t
	WHERE (t.row % 10) = 0;
	""".format(data)
	
	n=time.time()
	cur.execute(query)
	
	devs = cur.fetchall()
	p_arr = []
	
	for d in devs:
		epoch = int(dt_to_epoch(d[0]))
		p_arr.append(round(float(d[1+offset]),2))
	
	cur.close()
	conn.close()
	
	t_taken=time.time()-n
	
	# return [v_arr,i_arr,p_arr]
	return p_arr
	
def get_data(id, segs='1', t_offset='0'):
	
	if id[-1] == 'a': offset = 0
	elif id[-1] == 'b': offset = 1
	else: return []
	
	query = "SELECT mac FROM device WHERE id="+id[:-1]+";"
	
	conn = psycopg2.connect(database = 'wattson', host = 'localhost', user = 'root', password = 'means swim of stream')
	cur = conn.cursor()
	
	cur.execute(query)
	
	devs = cur.fetchone()
	data = devs[0]
	
	segs = int(segs) if str(segs).isdigit() else 1
	downscale = math.ceil(segs/6.0)
	segs = segments_to_string(segs)
	
	t_offset = int(t_offset) if str(t_offset).isdigit() else 1
	t_offset = segments_to_string(t_offset)
	
	# query = "SELECT til,v_1,v_2,i_1,i_2,p_1,p_2,f FROM sample WHERE device_mac='"+data+"' AND til > now() - interval '1 hour' ORDER BY til DESC;"
	# query = "SELECT til,v_1,v_2,i_1,i_2,p_1,p_2,f FROM sample WHERE device_mac='"+data+"' ORDER BY til DESC LIMIT 1000;"
	
	query = """
	SELECT t.*
	FROM (
	  SELECT til,v_1,v_2,i_1,i_2,p_1,p_2,f,row_number() OVER(ORDER BY til ASC) AS row
	  FROM sample
	  WHERE device_mac='{0}' AND til BETWEEN ((now() - interval '{3}') - interval '{1}') AND (now() - interval '{3}')
	  ORDER BY til ASC
	) t
	WHERE (t.row % {2}) = 0;
	""".format(data,segs,downscale,t_offset)
	
	# print segs,t_offset
	
	cur.execute(query)
	
	devs = cur.fetchall()
	v_arr = []
	# i_arr = []
	p_arr = []
	
	for d in devs:
		epoch = int(dt_to_epoch(d[0])*1000)
		v_arr.append([epoch, round(float(d[1+offset]),2)])
		# i_arr.append([epoch, round(float(d[3+offset]),2)])
		p_arr.append([epoch, round(float(d[5+offset]),2)])
	
	cur.close()
	conn.close()
	
	# return [v_arr,i_arr,p_arr]
	return [v_arr,p_arr]
	
def get_devices():
	# {"name1":"Foo","id1":1,"name2":"Bar","id2":2}
	
	query = "SELECT * FROM device;"
	
	conn = psycopg2.connect(database = 'wattson', host = 'localhost', user = 'root', password = 'means swim of stream')
	cur = conn.cursor()
	
	cur.execute(query)
	
	devs = cur.fetchall()
	
	data = []
	
	for d in devs:
		data.append({"name1": str(d[2]), "id1": str(d[0])+"a", "name2": str(d[3]), "id2": str(d[0])+"b", "mac": d[4]})
	cur.close()
	conn.close()
	
	return data

def change_dev(id, name, desc):
	query = 'UPDATE device SET name = %s, "desc" = %s WHERE id = %s;'
	
	conn = psycopg2.connect(database = 'wattson', host = 'localhost', user = 'root', password = 'means swim of stream')
	cur = conn.cursor()
	
	cur.execute(query,(urllib.unquote_plus(name),urllib.unquote_plus(desc),id[:-1]))
	conn.commit()
	conn.close()
	
	
	return 1

def application(environ, start_response):
	status = '200 OK'
	
	qs = environ['QUERY_STRING']
	
	query = {}
	
	if qs != "":
		for v in qs.split('&'):
			v = v.split('=')
			query[v[0]] = v[1]
	
	if environ['PATH_INFO'] == "/detail":
		data = get_data(query['id'],query['segs'],query['t_offset'])
		output = json.dumps(data)
		
	elif environ['PATH_INFO'] == "/devices":
		data = get_devices()
		output = json.dumps(data)
		
	elif environ['PATH_INFO'] == "/spark":
		data = sparkline(query['id'])
		output = json.dumps(data)
		
	elif environ['PATH_INFO'] == "/change-dev":
		data = change_dev(query['id'], query['name'], query['desc'])
		output = json.dumps(data)
		
	else:
		status = "400 Bad Request"
		output = json.dumps({}, skipkeys=True, default=lambda obj: 'N/A')
		
		# output = json.dumps([environ,query], skipkeys=True, default=lambda obj: 'N/A')
	
	response_headers = [('Content-type', 'application/json'),
						('Content-Length', str(len(output))),
						('Access-Control-Allow-Origin', '*')]
	start_response(status, response_headers)
	
	return output
