class Serial:
	ser = "NONE"
	baud = 0
	f = {}
	def __init__(self,s,b):
		self.ser = s
		self.baud = b
		self.f = open('./fake_serial', 'w')
		self.f.write(self.ser+" at "+str(self.baud)+" opened.\n\r")
		return
	def write(self,s):
		for a in s:
			self.f.write(hex(ord(a))+" ")
		return len(s)
	def close(self):
		self.f.write("\n\r"+self.ser+" at "+str(self.baud)+" closed.\n\r")
		self.f.close()
		return
