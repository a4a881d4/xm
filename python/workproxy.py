import socket
import struct
import hashlib
import time
from Queue import Queue,PriorityQueue
import threading
from ctypes import *

class proxyRecv(threading.Thread):
	def __init__(self,proxy,q):
		threading.Thread.__init__(self)
		self.s=proxy
		self.q=q
		
	def run(self):
		while(1):
			message = self.s.recv(1)
			mesType = struct.unpack("B",message)
			print "reciev message "
			print mesType
			if mesType[0]==0:
				blockBin=s.recv(128)
				print "Block:recv"
				self.q.put(blockBin)
			if mesType[0]==1:
				ret=struct.unpack("I",s.recv(4))
				print "server return %d" % ret[0]
				

class proxySock:
	def __init__(self):
		self.HOST = "162.243.41.59"
		self.PORT = 8336
		self.POOLER = "D687YjhqsJLYiXjYRaneBgpzwhdmYDx6Dg.a4"
		self.PASSWD = "0"
		self.VERSION_MAJOR = 0
		self.VERSION_MINOR = 9
		self.thread_num_max = 1
		self.fee_to_pay = 3
		self.miner_id = 0
		self.nSieveExtensions = 9
		self.nSievePercentage = 10
		self.nSieveSize = 5000000
		
			
	def connect(self):
		nameLen = len(self.POOLER)
		digest = struct.unpack("5I",hashlib.sha1(self.PASSWD).digest())
		p1 = digest[0] ^ digest[1] ^ digest[4]
		p2 = digest[2] ^ digest[3] ^ digest[4]
		passstr = struct.pack("II",p1,p2)
		passhash = ''.join( [ "%02x" % ord( x ) for x in passstr ] ).strip()
		passLen = len(passhash)
		fmt="B%dsBBBBBHBBIB%dsBB" % (nameLen,passLen)
		Hello = struct.pack( fmt
			, nameLen
			, self.POOLER
			, 0
			, self.VERSION_MAJOR
			, self.VERSION_MINOR
			, self.thread_num_max
			, self.fee_to_pay
			, self.miner_id
			, self.nSieveExtensions
			, self.nSievePercentage
			, self.nSieveSize
			, passLen
			, passhash
			, 0
			, 0
		)

		self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.s.connect((self.HOST, self.PORT))
		hs = self.s.send(Hello)
		print "conect to %s:%d" % (self.HOST, self.PORT)
		
	def recv(self,num):
		while True:
			self.s.settimeout(300)
			try:
				msg = self.s.recv(num)
				return msg
			except socket.timeout:
				self.connect()
				
	def submit(self,block,nFix,nTry):
		n = nFix*nTry
		b = bytearray(128)
		b[:] = block
		i=81
		while(i<128):
			b[i]=n%256
			n/=256
			i+=1
			if n==0:
				break
		b[80]=i-80
		hs = self.s.send(b)

if __name__=='__main__':
	import xmStep1
	import xmStep2
	hprime = CDLL('work/libprime.so')
		
	s = proxySock()
	s.connect()
	workQ=Queue()
	hashQ=PriorityQueue()
	chainQ=Queue()
	recvThread=proxyRecv(s,workQ)
	hashThread=xmStep1.xmStep1(workQ,hashQ)
	workers=[]
	for i in range(0,4):
		primeThread=xmStep2.xmStep2(hashQ,chainQ,i)
		workers.append(primeThread)
	recvThread.start()
	hashThread.start()
	for primeThread in workers:
		primeThread.start()
	lastcc = int(hprime.test())
	lastcp5 = int(hprime.chain(5))
	lasttime = time.time()
	while(1):
		newcc = int(hprime.test())
		newcp5 = int(hprime.chain(5))
		det=time.time()-lasttime
		detChain6=xmStep1.Chain6-lastChain6
		detChain7=xmStep1.Chain7-lastChain7
		print "Test per second:%f" % ((newcc-lastcc)/det)
		print "Chain 5 per hour:%f" % ((newcp5-lastcp5)/det*3600.)
		print "Est Chain 6 per hour:%f" % (detChain6/det*3600.)
		print "Est Chain 7 per hour:%f" % (detChain7/det*3600.)
		onTime = time.time()-xmStep1.startTime
		lastChain6=xmStep1.Chain6
		lastChain7=xmStep1.Chain7
		print "Est from start Chain 6 per hour:%f" % (lastChain6/onTime*3600.)
		print "Est from start Chain 7 per hour:%f" % (lastChain7/onTime*3600.)
		lastcc=newcc
		lastcp5=newcp5
		lasttime=time.time()
		(block,nFix,nTry)=chainQ.get()
		s.submit(block,nFix,nTry)
		



		
		
		
	

