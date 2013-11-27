import psycopg2
import random
import sys
import time

db = "wattson"
host = "localhost"
u = "root"
pss = "means swim of stream"

data_pdpd = 60*60*24 #data per day per device

insert_str = ("INSERT INTO sample VALUES (DEFAULT, %s, %s, %s, 1);", \
                  "INSERT INTO measurement VALUES (DEFAULT, %s, %s, %s, %s) RETURNING id;")

def main(argv = sys.argv, argc = len(sys.argv)):
    try:
        conn = psycopg2.connect(database=db, host = host, user = u, password = pss)
    except:
        print "Failed"
    else:
        cur = conn.cursor()
        print conn, cur
        print "Start", time.asctime()
        start = time.time()
        for i in xrange(data_pdpd):
            cur.execute(insert_str[1],(i, i ,i ,i))
            mid = cur.fetchone()[0]
            cur.execute(insert_str[0],(time.asctime(),time.asctime(), mid))
            #conn.commit()
        conn.commit()
        cur.close()
        conn.close()
        print "End", time.asctime()
        print (time.time()-start)/data_pdpd
        
    print "done"
    return

if __name__ == "__main__":
    main()
