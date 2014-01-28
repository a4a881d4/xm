import struct
import hashlib
from ctypes import *
import math
import threading
import xmStep1

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
	def __init__(self,inQ,outQ):
		threading.Thread.__init__(self)
		self.inQ=inQ
		self.outQ=outQ
		self.prime = CDLL('work/libprime.so')
		self.prime.init()

	def run(self):
		while(1):
			(block,mul)=self.inQ.get()
			m = hashlib.sha256()
			m.update(block[0:80])
			hash1 = m.digest()
			m = hashlib.sha256()
			m.update(hash1)
			hash2 = m.digest()
			bHash = struct.unpack('32B',hash2)
			cHash = (c_char * 32).from_buffer(bHash)
			stop = c_bool( True )
			nFix = c_ulonglong(mul)
			xmStep2.Weave(cHash,nFix,0,byref(stop))
			while(1):
				nTry = xmStep2.getNext()
				if nTry!=0:
					self.outQ.put((block,mul,int(nTry)))
				else:
					break
				
			
