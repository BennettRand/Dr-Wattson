import sys
import platform
import os
import signal
import subprocess
from multiprocessing import Process, freeze_support

config = {}
children = []

def handler(signum,frame):
	print "Ctrl-C"
	for c in children:
		print "Killing child",c.pid
		c.kill()
	quit(-1)

def test_deps(deps):
	import imp
	print "-----Checking Python Modules------------"
	for d in deps:
		try:
			imp.find_module(d)
		except ImportError as e:
			print "Could not find",d
			print e
			print "-----Bailing----------------------------"
			quit(-2)
		else:
			print "Found",d
	print "-----Okay-------------------------------"
	return

def load_cfg():
	global config
	import ConfigParser
	print "-----Loading Config---------------------"
	config = ConfigParser.SafeConfigParser()
	try:
		config.read("cfg.ini")
		print "Loaded",len(config.sections()),"sections."
		print len(config.options("Serial")), "options in Serial"
		if len(config.options("Serial")) != 4:
			raise ConfigParser.Error("Serial != 4")
		print len(config.options("Database")), "options in Database"
		if len(config.options("Database")) != 6:
			raise ConfigParser.Error("Database != 6")
		print len(config.options("Server")), "options in Server"
		if len(config.options("Server")) != 3:
			raise ConfigParser.Error("Server != 3")
		print len(config.options("Logs")), "options in Logs"
		if len(config.options("Logs")) != 3:
			raise ConfigParser.Error("Logs != 3")
	except ConfigParser.Error as e:
		print e
		print "-----Bad INI----------------------------"
		quit(-3)
		
	print "-----Okay-------------------------------"
	return

def test_db():
	import psycopg2
	print "-----Checking Database------------------"
	try:
		conn = psycopg2.connect(database = config.get("Database","db"), host = config.get("Database","host"), user = config.get("Database","user"), password = config.get("Database","password"))
	except psycopg2.Error as e:
		print e
		print "-----Bad Database-----------------------"
		quit(-4)
	print config.get("Database","user")+"@"+config.get("Database","host"),"Available"
	conn.close()
	print "-----Okay-------------------------------"
	return

def test_serial():
	# global config
	import serial
	print "-----Checking Serial--------------------"
	try:
		if platform.system() == "Windows":
			ser_s = config.get("Serial","windows_device")
		else:
			ser_s = config.get("Serial","posix_device")
		ser = serial.Serial(ser_s, config.getint("Serial","baud"), timeout = config.getfloat("Serial","timeout"))
	except serial.SerialException as e:
		print e
		print "-----Bad Serial-----------------------"
		quit(-4)
	print ser_s,"Available"
	print "-----Okay-------------------------------"
	ser.close()
	return

def start_server():
	global config
	print "Server Starting..."
	os.chdir('./server')
	svr = subprocess.Popen(["python","minimal_server.py"])
	children.append(svr)
	os.chdir('..')
	return
	
	
def start_network():
	global config
	print "Network Manager Starting..."
	os.chdir('./daemon')
	if platform.system() == "Windows":
		ser_s = config.get("Serial","windows_device")
	else:
		ser_s = config.get("Serial","posix_device")
	nwk = subprocess.Popen(["python","network_manager.py",ser_s])
	children.append(nwk)
	os.chdir('..')
	return

def main(argv = sys.argv, argc=len(sys.argv)):
	deps = ['sys','os','json','random','time','psycopg2','itertools','paste','SimpleHTTPServer','SocketServer','signal','platform','multiprocessing','serial','math','copy','ConfigParser','subprocess']
	# server_p = Process(target=start_server)
	# network_p = Process(target=start_network)
	signal.signal(signal.SIGINT,handler)
	
	test_deps(deps)
	
	load_cfg()
	test_db()
	test_serial()
	
	start_server()
	start_network()
	
	c_count = len(children)
	
	while c_count > 0:
		for c in children:
			s = c.poll()
			if s != None:
				print c.pid, "Died"
				c.wait()
				c_count -= 1
	
	# server_p.start()
	# network_p.start()
	return

if __name__ == "__main__":
	if platform.system() == "Windows":
		freeze_support()
	main()