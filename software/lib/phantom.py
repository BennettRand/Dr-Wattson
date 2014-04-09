import sys

timestmp = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30]
measures = [0, 0, 0, 0, 0, 0, 0, 0.25, 0.75, 1, 1.5, 1.25, 1, 1, 1.1, 1, 0.9, 1, 0.9, 0.9, 0.8, 0.9, 0.5, 0.25, 0, 0, 0, 0, 0, 0]
samples = zip(timestmp,measures)

def median(aray):
	srtd = sorted(aray)
	alen = len(srtd)
	return 0.5*( srtd[(alen-1)//2] + srtd[alen//2])

def quartile(a, q):
	med = median(a)
	
	
	if q == 0:
		return min(a)
	elif q == 1:
		bot = [x for x in a if x < med]
		return median(bot)
	elif q == 2:
		return med
	elif q == 3:
		top = [x for x in a if x > med]
		return median(bot)
	elif q == 4:
		return max(a)
	else:
		raise "q > 4"
		
def get_phantom(a):
	q1 = quartile([x[1] for x in a],2)
	
	m = [x if x[1] <= q1 else (x[0],0) for x in a]
	
	tot = 0
	
	print m
	
	for i in xrange(1,len(m)):
		tot += (m[i-1][1]+m[i][1]/2)*(m[i][0]+m[i-1][0]/2)
		
	return tot

def main():
	print samples
	print get_phantom(samples)
	return

if __name__ == "__main__":
	main()