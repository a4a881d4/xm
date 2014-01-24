import socket
import struct
import hashlib
import time
from Queue import Queue
import threading

class proxyRecv(threading.Thread):
	def __init__(self,sock,q):
		threading.Thread.__init__(self)
		self.s=sock
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
				#block.header,block.nNonce,block.primemultiplier = struct.unpack("76BI48B",blockBin)
			if mesType[0]==1:
				ret=struct.unpack("I",s.recv(4))
				print "server return %d",ret
			time.sleep(10)

class proxySock:
	def __init__(self):
		self.HOST = "162.243.41.59"
		self.PORT = 8336
		self.POOLER = "DLsvbQLXQBEWxynZaus7vaHqLny1TTDtkt.Test"
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
		
	def recv(self,num):
		return self.s.recv(num)

if __name__=='__main__':
	import xmStep1
	s = proxySock()
	s.connect()
	workQ=Queue()
	hashQ=Queue()
	recvThread=proxyRecv(s,workQ)
	hashThread=xmStep1.xmStep1(workQ,hashQ)
	recvThread.start()
	hashThread.start()
	while(1):
		(block,mul)=hashQ.get()
		print mul
		



		
		
		
	

