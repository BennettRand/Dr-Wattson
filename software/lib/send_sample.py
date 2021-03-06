import sys
import db_info
import psycopg2
import time

measure_insert = "INSERT INTO measurement VALUES (DEFAULT, %s, %s, %s, %s) RETURNING id;"
sample_insert = "INSERT INTO sample VALUES (DEFAULT, %s, %s, %s, %s);"

class send_sample:
	def __init__(self, n):
		self.connections = []
		self.cursors = []
		self.tocommit = []
		
		for x in xrange(n):
			self.connections.append(psycopg2.connect(database = db_info.database, host = db_info.host, user = db_info.user, password = db_info.password))
			
		for c in self.connections:
			self.cursors.append(c.cursor())
			
		self.lastcommit = time.time()
		self.nextcur = 0
		
	def __del__(self):
		print self.multicommit()
		for c in self.cursors:
			c.close()
			
		for c in self.connections:
			c.commit()
			c.close()
			
	def trycommit(self):
		if time.time() - self.lastcommit > 1:
			print self.multicommit()
			self.lastcommit = time.time()
			
		
	def multicommit(self):
		return ",".join(["("+",".join(x)+")" for x in self.tocommit])
		
	def sample(self, power, voltage, current, frequency, timefrom = time.asctime(), timetill = time.asctime()):
		
		# self.cursors[self.nextcur].execute(measure_insert,(power, voltage, current, frequency))
		# mid = self.cursors[self.nextcur].fetchone()[0]
		# self.cursors[self.nextcur].execute(sample_insert,(timefrom, timetill, mid, 1))
		
		# self.nextcur += 1
		
		# if self.nextcur >= len(self.cursors):
			# self.nextcur = 0;
		
		self.tocommit.append((power,voltage,current,frequency))
		
		self.trycommit()

def main(argc = len(sys.argv), args = sys.argv):
	n = 1
	
	x = send_sample(n)
	
	data_pdpd = 60*60*24 #data per day per device
	
	# print time.asctime()
	start = time.time()
	
	for i in xrange(data_pdpd):
		x.sample(i,i,i,i)
	
	# print time.asctime()
	# print time.time()-start
	print n,(time.time()-start)/data_pdpd
	
	return

if __name__ == "__main__":
	main()