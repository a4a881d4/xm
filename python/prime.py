def primes(n): 
    if n==2: return [2]
    elif n<2: return []
    s=range(3,n+1,2)
    mroot = n ** 0.5
    half=(n+1)/2-1
    i=0
    m=3
    while m <= mroot:
        if s[i]:
            j=(m*m-3)/2
            s[j]=0
            while j<half:
                s[j]=0
                j+=m
        i=i+1
        m=2*i+3
    return [2]+[x for x in s if x]
"""
def primes(n): # simple Sieve of Eratosthenes 
   odds = range(3, n+1, 2)
   sieve = set(sum([range(q*q, n+1, q+q) for q in odds],[]))
   return [2] + [p for p in odds if p not in sieve]
"""
if __name__=="__main__":
	ps = primes(100000)
	print ps[0:10]
	ps = ps[0:len(ps)/10]
	a=1.
	for p in ps:
		if p>61:
			a=a*(p-8.)/p
	print a*1000000

