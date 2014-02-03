#include <stdlib.h>
#include <gmp.h>
#include <gmpxx.h>
#include "Csieve.h"

/*
unsigned int nSieveSize = 1000000;
unsigned int nSievePercentage = 10;
unsigned int nSieveExtensions = 0;
*/

int cc = 1;
int cp[10];
extern "C" {
void clean()
{
	int i;
	cc=1;
	for( i=0;i<10;i++ ) cp[i]=0;	
}
int chain( int k )
{
	return cp[k];	
}
int test()
{
	return cc;	
}
void *init(unsigned int nSieveSize, unsigned int nSievePercentage, unsigned int nSieveExtensions, unsigned int nBit) 
{
	clean();
	CSieveOfEratosthenes *ps = new CSieveOfEratosthenes(nSieveSize, nSievePercentage, nSieveExtensions, nBit);
	return (void *)ps;
}
int getNSieveWeaveOptimalPrime( void *pSclass )
{
	CSieveOfEratosthenes *ps = (CSieveOfEratosthenes *) pSclass;
	return ps->getNSieveWeaveOptimalPrime();	
}
int GetCandidateCount( void *pSclass )
{
	CSieveOfEratosthenes *ps = (CSieveOfEratosthenes *) pSclass;
	return ps->GetCandidateCount();	
}
void Weave( void *pSclass, void *hash, uint64_t nfix, uint32_t start, bool *pStop )
{
	CSieveOfEratosthenes *ps = (CSieveOfEratosthenes *) pSclass;
	ps->hash = uint256( (unsigned char *)hash );
	ps->nFix = nfix;
	ps->Weave( ps->hash, ps->nFix, start, *pStop );
}

uint64_t getNext(void *pSclass, bool& stop)
{
	CSieveOfEratosthenes *ps = (CSieveOfEratosthenes *) pSclass;
	uint64_t mul;
	unsigned int type;
	int i;
	while( ps->GetNextCandidateMultiplier(mul,type) ) {
			if(stop)
				break;
			unsigned int len = PrimeChainTest(ps->hash, ps->nFix, mul, type);
			cc++;
			if( len<10 )
				cp[len]++;
			if( len >=6 ) {
				printf("%s:%d - %lld - %d e %d s %d - %lld\n",(type==PRIME_CHAIN_CUNNINGHAM1)? "1CC" : ((type==PRIME_CHAIN_CUNNINGHAM2)? "2CC" : "TWN"),len,mul,ps->nCandidateIndex,ps->nCandidateActiveExtension,ps->nStart,((uint64_t)ps->nCandidateIndex+ps->nStart) * (2 << ps->nCandidateActiveExtension));
					return mul;
			}
			
	}
	/*
	printf("find %d-%d: ",cc,cp[1]);
	for( i=0;i<7;i++ )
		printf("%d-%d:%le ",i,cp[i],(double)cp[i]/(double)cc);
	printf("\n");
	if( cc > 0x1000000 ) clean();
	*/
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
