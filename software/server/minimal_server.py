import sys
import paste.httpserver
import SimpleHTTPServer
import SocketServer
import signal
import platform
import os
from multiprocessing import Process, freeze_support
import ConfigParser

sys.path.append('../wsgi')

import testapp

config = ConfigParser.SafeConfigParser()
config.read("../cfg.ini")

def handler(signum,frame):
	print "Ctrl-C"
	quit()

def serve_wsgi():
	signal.signal(signal.SIGINT,handler)
	sys.stdout = open(config.get("Logs","wsgi_log"),'w+')
	sys.stderr = open(config.get("Logs","wsgi_log")+".err",'w+')
	print "WSGI Starting..."
	paste.httpserver.serve(testapp.application, port=config.getint("Server","wsgi_port"), protocol_version = "HTTP/1.1", socket_timeout = 10000, use_threadpool = True, threadpool_workers = 10, daemon_threads=True)
	
def serve_http():
	signal.signal(signal.SIGINT,handler)
	os.chdir("../web")
	sys.stdout = open(config.get("Logs","http_log"),'w+')
	sys.stderr = open(config.get("Logs","http_log")+".err",'w+')
	Handler = SimpleHTTPServer.SimpleHTTPRequestHandler
	httpd = SocketServer.TCPServer(("", config.getint("Server","http_port")), Handler)
	print "HTTP Starting..."
	httpd.serve_forever()

def main(argv = sys.argv, argc=len(sys.argv)):
	s_wsgi = Process(target=serve_wsgi)
	s_wsgi.start()
	serve_http()
	return

if __name__ == "__main__":
	if platform.system() == "Windows":
		freeze_support()
	main()