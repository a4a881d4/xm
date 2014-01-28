#include <stdlib.h>
#include "Csieve.h"
std::vector<unsigned int> vPrimes;
unsigned int nSieveSize = 1000000;
unsigned int nSievePercentage = 10;
unsigned int nSieveExtensions = 9;
CSieveOfEratosthenes *ps;
uint256 h;
uint64_t nFix;

void *init() 
{
	GeneratePrimeTable();
	ps = new CSieveOfEratosthenes(nSieveSize, nSievePercentage, nSieveExtensions, 7);
	return (void *)ps;
}

void Weave( void *hash, uint64_t nfix, uint32_t start, bool *pStop )
{
	h = uint256( (unsigned char *)hash );
	nFix = nfix;
	ps->Weave( h, nFix, start, *pStop );
}

uint64_t getNext(uint64_t mul,unsigned int type)
{
	while( ps->GetNextCandidateMultiplier(mul,type) ) {
			unsigned int len = PrimeChainTest(h, nFix, mul, type);
			if( len !=0 ) {
				printf("%s:%d - %lld\n",(type==PRIME_CHAIN_CUNNINGHAM1)? "1CC" : ((type==PRIME_CHAIN_CUNNINGHAM2)? "2CC" : "TWN"),len,mul);
				if( len>=6 )
					return mul;
			}
		}
	return 0;
}

#ifdef SELFTEST 
int main()
{
	init();
	
	uint256 tH = uint256("0xABCDEF123abcdef12345678909832180000011111111");
	bool stop = false;
	uint64_t nfix = 210ULL*11*13*17;
	uint32_t start;	
	for( start=0;start<nSieveSize*1000;start+=nSieveSize )
	{
		Weave( (void *)tH.getPn(), nfix, start, &stop );
		uint64_t mul;
		unsigned int type;
		uint64_t c;
		while( (c=getNext(mul,type))!=0 ) {
			printf("find %lld\n",c);
		}
		printf("end of round\n");
	}
	return 0;
}
#endif
