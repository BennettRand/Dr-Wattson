import sys
import json

def multicommit(arr):
	return ",".join(["("+",".join(str(x))+")" for x in arr])
	
def one_row(arr):
	return "("+",".join([str(x) for x in arr])+")"
	
def main(argc = len(sys.argv), args = sys.argv):
	f = open("collected.json")
	
	data = json.load(f)
	
	for d in data:
		keys = data[d]['power'][0].keys()
		print ",".join([str(p) for p in data[d]['power']])
	
	return

if __name__ == "__main__":
	main()