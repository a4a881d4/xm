#include <stdlib.h>
#include "Csieve.h"
std::vector<unsigned int> vPrimes;
unsigned int nSieveSize = 1000000;
unsigned int nSievePercentage = 10;
unsigned int nSieveExtensions = 9;

int main()
{
	GeneratePrimeTable();
	CSieveOfEratosthenes sieve(nSieveSize, nSievePercentage, nSieveExtensions, 7);
	uint256 h = uint256("0xABCDEF123abcdef12345678909832180000011111111");
	bool stop = false;
	uint64_t nFix = 210ULL*11*13*17;
	uint32_t start;	
	for( start=0;start<nSieveSize*1000;start+=nSieveSize )
	{
		sieve.Weave( h, nFix, start, stop );
		uint64_t mul;
		unsigned int type;
		while( sieve.GetNextCandidateMultiplier(mul,type) ) {
			unsigned int len = PrimeChainTest(h, nFix, mul, type);
			if( len !=0 )
				printf("%s:%d - %lld\n",(type==PRIME_CHAIN_CUNNINGHAM1)? "1CC" : ((type==PRIME_CHAIN_CUNNINGHAM2)? "2CC" : "TWN"),len,mul);
		}
	}
	return 0;
}

