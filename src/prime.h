#ifndef XM_PRIME_H
#define XM_PRIME_H
#include <gmp.h>
#include <gmpxx.h>


extern std::vector<unsigned int> vPrimes;
extern unsigned int nSieveExtensions;
extern unsigned int nSievePercentage;
extern unsigned int nSieveSize;
static const mpz_class mpzOne = 1;
static const mpz_class mpzTwo = 2;
// Generate small prime table
unsigned int PrimeChainTest(uint256& hash, uint64_t nFix, uint64_t nTry, unsigned int nCandidateType);

enum // prime chain type
{
    PRIME_CHAIN_CUNNINGHAM1 = 1u,
    PRIME_CHAIN_CUNNINGHAM2 = 2u,
    PRIME_CHAIN_BI_TWIN     = 3u,
};



inline void mpz_set_uint256(mpz_t r, uint256& u)
{
    mpz_import(r, 32 / sizeof(unsigned long), -1, sizeof(unsigned long), -1, 0, &u);
}
#endif
