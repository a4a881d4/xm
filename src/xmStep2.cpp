#include <stdlib.h>
#include <gmp.h>
#include <gmpxx.h>
#include "Csieve.h"

/*
unsigned int nSieveSize = 1000000;
unsigned int nSievePercentage = 10;
unsigned int nSieveExtensions = 0;
*/

extern "C" {
void *init(unsigned int nSieveSize, unsigned int nSievePercentage, unsigned int nSieveExtensions, unsigned int nBit) 
{
	CSieveOfEratosthenes *ps = new CSieveOfEratosthenes(nSieveSize, nSievePercentage, nSieveExtensions, nBit);
	return (void *)ps;
}

void Weave( void *pSclass, void *hash, uint64_t nfix, uint32_t start, bool *pStop )
{
	CSieveOfEratosthenes *ps = (CSieveOfEratosthenes *) pSclass;
	ps->hash = uint256( (unsigned char *)hash );
	ps->nFix = nfix;
	ps->Weave( ps->hash, ps->nFix, start, *pStop );
}

uint64_t getNext(void *pSclass)
{
	CSieveOfEratosthenes *ps = (CSieveOfEratosthenes *) pSclass;
	uint64_t mul;
	unsigned int type;
	int cc = 0;
	int cp = 0;
	while( ps->GetNextCandidateMultiplier(mul,type) ) {
			unsigned int len = PrimeChainTest(ps->hash, ps->nFix, mul, type);
			cc++;
			if( len >=6 ) {
				printf("%s:%d - %lld - %d e %d s %d - %lld\n",(type==PRIME_CHAIN_CUNNINGHAM1)? "1CC" : ((type==PRIME_CHAIN_CUNNINGHAM2)? "2CC" : "TWN"),len,mul,ps->nCandidateIndex,ps->nCandidateActiveExtension,ps->nStart,((uint64_t)ps->nCandidateIndex+ps->nStart) * (2 << ps->nCandidateActiveExtension));
					return mul;
			}
			if( len>0 )
				cp++;
	}
	//printf("find %d:%d\n",cc,cp);
	return 0;
}
}
#ifdef SELFTEST 
int main()
{
	init();
	
	uint256 tH = uint256("0xABCDEF123abcdef12345678909832180000011111111");
	bool stop = false;
	uint64_t nfix = 210ULL*11*13*17;
	uint32_t start;	
	for( start=0;start<ps->nSieveSize*1000;start+=ps->nSieveSize )
	{
		Weave( (void *)tH.getPn(), nfix, start, &stop );
		uint64_t mul;
		unsigned int type;
		uint64_t c;
		while( (c=getNext())!=0 ) {
			printf("find %lld\n",c);
		}
		printf("end of round\n");
	}
	return 0;
}
#endif
