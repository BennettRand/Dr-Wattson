#!/usr/bin/python2
import psycopg2
import time
import os

char_lookup = {-1: 0xFF,
                0: 0xC0,
                1: 0xF9,
                2: 0xA4,
                3: 0xB0,
                4: 0x99,
                5: 0x92,
                6: 0x82,
                7: 0xF8,
                8: 0x80,
                9: 0x98,
                 }

def encode7Seg(num):
	num = round(num)
	if num > 9999:
		num = 9999

	res = [0xFF]*4
	res[0] = char_lookup[num%10]
	if num > 10:
		res[1] = char_lookup[int((num/10)%10)]
	if num > 100:
		res[2] = char_lookup[int((num/100)%10)]
	if num > 1000:
		res[3] = char_lookup[int((num/1000)%10)]
	return res

conn = psycopg2.connect(database = "wattson", host = "localhost", user = "root", password = "means swim of stream")

try:
	fpga = os.open('/dev/fpga', os.O_RDWR)
	while (True) :
		cur = conn.cursor()
		buttons = os.read(fpga, 2)
		query = ""
		if (bytearray(buttons)[0] & 0x01):
			query = "SELECT SUM(p_1) + SUM(p_2) AS Power FROM sample WHERE til IN (SELECT MAX(til) FROM sample GROUP BY device_mac) and (til > now() - INTERVAL '10 seconds');"
		else:
			query = "SELECT v_2 FROM sample WHERE til IN (SELECT MAX(til) FROM sample GROUP BY device_mac) and (til > now() - INTERVAL '10 seconds');"
		cur.execute(query)
		data = cur.fetchone()
		if (data != None) and len(data) != 0:
			sum = encode7Seg(data[0])
			print data
		else:
			sum = [0xBF]*4
		os.write(fpga, "".join(chr(val) for val in sum))
		cur.close()
		conn.commit()
		time.sleep(1)
except KeyboardInterrupt:
	conn.close()
	os.close(fpga)
conn.close()
