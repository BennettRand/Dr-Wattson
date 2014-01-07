import sys
import db_info
# import psycopg2
import time

measure_insert = "INSERT INTO measurement VALUES"
sample_insert = "INSERT INTO sample VALUES"

class send_sample:
	def __init__(self, n):
		self.connection = psycopg2.connect(database = db_info.database, host = db_info.host, user = db_info.user, password = db_info.password)
		self.cursor = self.connection.cursor()
		self.powerinfo = []
		self.timestamps = []
		
		self.lastcommit = time.time()
		
	def __del__(self):
		self.commit()
		self.cursor.close()
		self.connection.close()
		
	def trycommit(self):
		if time.time() - self.lastcommit > 1:
			self.commit()
			self.lastcommit = time.time()
			
		
	def commit(self):
		return
		
	def multicommit(self):
		str = ",".join(["("+",".join(x)+")" for x in self.powerinfo])
		self.powerinfo = []
		return str
		
	def sample(self, power, voltage, current, frequency, timefrom = time.asctime(), timetill = time.asctime()):
		
		self.powerinfo.append((str(power), str(voltage), str(current), str(frequency)))
		self.timestamps.append((timefrom, timetill))
		
		self.trycommit()

def main(argc = len(sys.argv), args = sys.argv):
	n = 1
	
	x = send_sample(n)
	
	data_pdpd = 60*60*24*5 #data per day per device
	
	# print time.asctime()
	# start = time.time()
	
	for i in xrange(data_pdpd):
		x.sample(i,i,i,i)
	
	# print time.asctime()
	# print time.time()-start
	# print n,(time.time()-start)/data_pdpd
	
	return

if __name__ == "__main__":
	main()