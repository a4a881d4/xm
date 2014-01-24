work/test:src/seiveTest.cpp src/prime.cpp
	g++ -g -o work/test  src/seiveTest.cpp src/prime.cpp -l crypto -l ssl -l gmp

work/sha256_xmm_amd64.o:src/sha256_xmm_amd64.asm
	yasm -f elf64 -o work/sha256_xmm_amd64.o src/sha256_xmm_amd64.asm

work/sha256_sse2_amd64.o:src/sha256_sse2_amd64.cpp
	g++ -fPIC -c -o work/sha256_sse2_amd64.o src/sha256_sse2_amd64.cpp

work/libsha256.so:work/sha256_sse2_amd64.o work/sha256_xmm_amd64.o
	ld -shared -L /usr/lib/gcc/x86_64-linux-gnu/4.8 -lcrypto -lgmp -lssl -lstdc++ -o work/libsha256.so work/sha256_sse2_amd64.o work/sha256_xmm_amd64.o

so:work/libsha256.so


