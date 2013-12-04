import psycopg2
import random
import sys
import time
import os

db = "wattson"
host = "localhost"
u = "root"
pss = "means swim of stream"

data_to_add = 2000

def main(argv = sys.argv, argc = len(sys.argv)):
	for x in xrange(100):
		print x+1,
		os.system("python ../lib/send_sample.py "+str(x+1)+" "+str(data_to_add))

if __name__ == "__main__":
    main()
