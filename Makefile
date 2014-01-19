work/test:src/seiveTest.cpp src/prime.cpp
	g++ -g -o work/test  src/seiveTest.cpp src/prime.cpp -l crypto -l ssl -l gmp

