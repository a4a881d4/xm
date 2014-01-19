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

static void GeneratePrimeTable()
{
    printf("GeneratePrimeTable() : setting nSieveExtensions = %u, nSievePercentage = %u, nSieveSize = %u\n", nSieveExtensions, nSievePercentage, nSieveSize);
    const unsigned nPrimeTableLimit = nSieveSize;
    vPrimes.clear();
    // Generate prime table using sieve of Eratosthenes
    std::vector<bool> vfComposite (nPrimeTableLimit, false);
    for (unsigned int nFactor = 2; nFactor * nFactor < nPrimeTableLimit; nFactor++)
    {
        if (vfComposite[nFactor])
            continue;
        for (unsigned int nComposite = nFactor * nFactor; nComposite < nPrimeTableLimit; nComposite += nFactor)
            vfComposite[nComposite] = true;
    }
    for (unsigned int n = 2; n < nPrimeTableLimit; n++)
        if (!vfComposite[n])
            vPrimes.push_back(n);
    printf("GeneratePrimeTable() : prime table [1, %u] generated with %u primes\n", nPrimeTableLimit, (unsigned int) vPrimes.size());
}

inline void mpz_set_uint256(mpz_t r, uint256& u)
{
    mpz_import(r, 32 / sizeof(unsigned long), -1, sizeof(unsigned long), -1, 0, &u);
}
#endif
