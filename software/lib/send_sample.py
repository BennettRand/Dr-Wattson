import sys
import db_info
import psycopg2

class send_sample:
	def __init__(self, n):
		self.connections = []
		for x in xrange(n):
			print "Opening", x
			self.connections.append(psycopg2.connect(database = db_info.database, host = db_info.host, user = db_info.user, password = db_info.password))
			
	def __del__(self):
		for c in self.connections:
			print "Closing", c
			c.close();

def main(argc = len(sys.argv), args = sys.argv):
	x = send_sample(4)
	return

if __name__ == "__main__":
	main()