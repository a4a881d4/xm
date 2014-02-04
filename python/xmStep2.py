import struct
import hashlib
from ctypes import *
import math
import threading
import xmStep1
import time
import math

def uint32(x):
	return x & 0xffffffffL

def bytereverse(x):
	return uint32(( ((x) << 24) | (((x) << 8) & 0x00ff0000) | (((x) >> 8) & 0x0000ff00) | ((x) >> 24) ))

def bytearray_to_uint32(x):
	return uint32(((x[3]) << 24) | ((x[2]) << 16)  | ((x[1]) << 8) | x[0])


def pack256(buf,off,sNum):
	s = int(sNum,16)
	for i in range( 0, 8 ):
		s,b = divmod(s,0x100000000)
		struct.pack_into('I',buf,off+i*4,(b))
	return buf,off+8*4
def packInt(buf,off,s):
	struct.pack_into('I',buf,off,(s))
	return buf,off+4

def unpack256(buf):
	s = struct.unpack('8I',buf)
	r=0;
	for i in range(0,8):
		r*=0x100000000
		r+=s[7-i]
	return r

def check(a):
	for x in [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61]:
		print "%d check %d" % (x, a%x )
			


class xmStep2(threading.Thread):
	def __init__(self,inQ,outQ,id):
		threading.Thread.__init__(self)
		self.inQ=inQ
		self.outQ=outQ
		self.prime = CDLL('work/libprime.so')
		self.nSieveExtensions = 9
		self.nSievePercentage = 10
		self.nSieveSize = 3000000
		self.pSclass = self.prime.init(c_uint(self.nSieveSize),c_uint(self.nSievePercentage),c_uint(self.nSieveExtensions),7)
		self.ID=id
		
	def run(self):
		#global xmStep1.switch
		while(1):
			#time.sleep(0.1)
			(mul,block,index)=self.inQ.get()
			#print time.ctime()+" thread %d get work %d" %(self.ID,index)
			m = hashlib.sha256()
			m.update(block[0:80])
			hash1 = m.digest()
			m = hashlib.sha256()
			m.update(hash1)
			hash2 = m.digest()
			bHash = bytearray(hash2)
			cHash = (c_char * 32).from_buffer(bHash)
			stop = c_bool( False )
			nFix = c_ulonglong(mul)
			start = 0
			#print repr(bHash)
			self.prime.Weave(self.pSclass,cHash,nFix,0,byref(xmStep1.switch))
			nRound = float(self.prime.GetCandidateCount(self.pSclass))
			for i in range(0,10):
				nRound *= self.EstimateCandidatePrimeProbability(mul,i)
				xmStep1.Chain[i]+=nRound
			
			while(1):
				nTry = self.prime.getNext(self.pSclass,byref(xmStep1.switch))
				if nTry==0:
					break
				else:
					self.outQ.put((block,mul,long(nTry)))
					
	def EstimateCandidatePrimeProbability(self, nFixed, nChain):
		'''
    // h * q# / r# * s is prime with probability 1/log(h * q# / r# * s),
    //   (prime number theorem)
    //   here s ~ max sieve size / 2,
    //   h ~ 2^255 * 1.5,
    //   r = 7 (primorial multiplier embedded in the hash)
    // Euler product to p ~ 1.781072 * log(p)   (Mertens theorem)
    // If sieve is weaved up to p, a number in a candidate chain is a prime
    // with probability
    //     (1/log(h * q# / r# * s)) / (1/(1.781072 * log(p)))
    //   = 1.781072 * log(p) / (255 * log(2) + log(1.5) + log(q# / r#) + log(s))
    //
    // This model assumes that the numbers on a chain being primes are
    // statistically independent after running the sieve, which might not be
    // true, but nontheless it's a reasonable model of the chances of finding
    // prime chains.
		'''
		sieveSize = float(self.nSieveSize)
		nSieveWeaveOptimalPrime = self.prime.getNSieveWeaveOptimalPrime(self.pSclass)
		nAverageCandidateMultiplier = sieveSize / 2.
		dExtendedSieveWeightedSum = 0.5 * sieveSize
		dExtendedSieveCandidates = sieveSize
		for i in range(0, self.nSieveExtensions):
			dExtendedSieveWeightedSum += 0.75 * float(self.nSieveSize * (2 << i))
			dExtendedSieveCandidates += sieveSize / 2.
		dExtendedSieveAverageMultiplier = dExtendedSieveWeightedSum / dExtendedSieveCandidates
		dLogTwo = math.log(2.)
		dLogOneAndHalf = math.log(1.5)
		up = 1.781072 * math.log(float(nSieveWeaveOptimalPrime))
		down = 255.0 * dLogTwo + dLogOneAndHalf + math.log(float(nFixed)) + math.log(nAverageCandidateMultiplier) + dLogTwo * float(nChain) + math.log(dExtendedSieveAverageMultiplier)

		return up / down
			
				
			
