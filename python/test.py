import math
import prime
ps = prime.primes(100)
print ps

a=1
c=0
step=31
for x in ps:
	a*=x
	b=math.log(a)/math.log(2)
	cl=c	
	c=int(b)
	if( cl<step and c >=step ):
		print "x=%d b=%f" % (x,b)
		step+=32

c = [23,47,61]
a=1
for x in ps:
	a*=x
	if x in c:
		b=math.log(a)/math.log(2)
		print "x=%d a=%d b=%f" % (x,a,b)
		a=1

