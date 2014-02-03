import struct
import hashlib
from ctypes import *
import math
import threading
import time

switch=c_bool(True)
Chain7=0.
Chain6=0.
startTime=time.time()

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
			
def testSearch(blockbin,target):
	block=bytearray(blockbin[0:80])
	hHash = CDLL('work/libsha256.so')

	buf = bytearray(128+32+4+4+8+8)
	for i in range(0,80):
		buf[i] = blockbin[i]

	struct.pack_into('II',buf,128+32,1000000,target)

	buff2 = (c_char * len(buf)).from_buffer(buf)
	"""
	struct work {
		char pdata[128];
		char phash[32];
		uint32_t max;
		uint32_t target;
		uint64_t mulfactor;
		uint64_t nHashesDone;
	};
	"""
	ret = hHash.scanhash_sse2_64(byref(buff2))
	#print "ret=%d" % ret
	if ret!=-1:
		buf = bytearray(buff2)

		#hashout = buf[128:128+32]
		#ihash = unpack256(hashout)
		mul = struct.unpack("q",buf[128+32+8:128+32+16])
		#print "from c"
		#print ihash
		#check(ihash)
		#print "return mul"
		#print mul
		#check(mul[0])
		blockbin,off = packInt(blockbin,76,ret&0xffffffff)
		#m = hashlib.sha256()
		#m.update(block)
		#hash1 = m.digest()
		#m = hashlib.sha256()
		#m.update(hash1)
		#hash2 = m.digest()
		#ihash = unpack256(hash2)
		#print "calc myself"
		#print ihash
		#check(ihash)
		return (ret,mul[0])
	lastnonce=struct.unpack("I",buf[76:80])
	newnonce=lastnonce[0]+1000000
	blockbin,off = packInt(blockbin,76,newnonce)
	return (ret,0)

class xmStep1(threading.Thread):
	def __init__(self,inQ,outQ):
		threading.Thread.__init__(self)
		self.inQ=inQ
		self.outQ=outQ
		self.hHash = CDLL('work/libsha256.so')

	def search(self,blockbin,target):
		block=bytearray(blockbin[0:80])
		buf = bytearray(128+32+4+4+8+8)
		for i in range(0,80):
			buf[i] = blockbin[i]

		struct.pack_into('II',buf,128+32,1000000,target)

		buff2 = (c_char * len(buf)).from_buffer(buf)
		ret = self.hHash.scanhash_sse2_64(byref(buff2))
		if ret!=-1:
			buf = bytearray(buff2)
			mul = struct.unpack("q",buf[128+32+8:128+32+16])
			blockbin,off = packInt(blockbin,76,ret&0xffffffff)
			return (ret,mul[0])
		lastnonce=struct.unpack("I",buf[76:80])
		newnonce=lastnonce[0]+1000000
		blockbin,off = packInt(blockbin,76,uint32(newnonce))
		return (ret,0)

	def run(self):
		global switch
		while(1):
			while( not self.outQ.empty() ):
				self.outQ.get_nowait()
			print time.ctime()+" new block"
			switch=c_bool(True)
			block=bytearray(self.inQ.get())
			target=8
			index=0
			(nonce,mul)=self.search(block,target)
			while( self.inQ.empty() ):
				if nonce!=-1:
					print "q len %d" % self.outQ.qsize()
					if( self.outQ.qsize()>16 ):
						time.sleep(0.1*self.outQ.qsize())
					b=bytearray(len(block))			
					b[:]=block
					self.outQ.put((mul,b,index))
					switch=c_bool(False)
					index+=1
				
				#target=target+1
				lastnonce=struct.unpack("I",block[76:80])
				newnonce=lastnonce[0]+1
				block,off = packInt(block,76,newnonce)
				(nonce,mul)=self.search(block,target)
		
