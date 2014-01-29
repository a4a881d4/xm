#include "uint256.h"
#include "prime.h"

class CPrimalityTestParams
{
public:
    // GMP variables
    mpz_t mpzE;
    mpz_t mpzR;
    mpz_t mpzRplusOne;
    
    // GMP C++ variables
    mpz_class mpzOriginMinusOne;
    mpz_class mpzOriginPlusOne;
    mpz_class N;

    // Values specific to a round
    unsigned int nCandidateType;

    // Results
    unsigned int nChainLength;

    CPrimalityTestParams()
    {
        nChainLength = 0;
        mpz_init(mpzE);
        mpz_init(mpzR);
        mpz_init(mpzRplusOne);
    }

    ~CPrimalityTestParams()
    {
        mpz_clear(mpzE);
        mpz_clear(mpzR);
        mpz_clear(mpzRplusOne);
    }
};

// Check Fermat probable primality test (2-PRP): 2 ** (n-1) = 1 (mod n)
// true: n is probable prime
// false: n is composite; set fractional length in the nLength output
static bool FermatProbablePrimalityTestFast(const mpz_class& n, CPrimalityTestParams& testParams)
{
    // Faster GMP version
    mpz_t& mpzE = testParams.mpzE;
    mpz_t& mpzR = testParams.mpzR;

    mpz_sub_ui(mpzE, n.get_mpz_t(), 1);
    
    mpz_powm(mpzR, mpzTwo.get_mpz_t(), mpzE, n.get_mpz_t());
    if (mpz_cmp_ui(mpzR, 1) == 0)
        return true;
    else
        return false;
}

// Test probable primality of n = 2p +/- 1 based on Euler, Lagrange and Lifchitz
// fSophieGermain:
//   true:  n = 2p+1, p prime, aka Cunningham Chain of first kind
//   false: n = 2p-1, p prime, aka Cunningham Chain of second kind
// Return values
//   true: n is probable prime
//   false: n is composite; set fractional length in the nLength output
static bool EulerLagrangeLifchitzPrimalityTestFast(const mpz_class& n, bool fSophieGermain, CPrimalityTestParams& testParams)
{
    // Faster GMP version
    mpz_t& mpzE = testParams.mpzE;
    mpz_t& mpzR = testParams.mpzR;
    mpz_t& mpzRplusOne = testParams.mpzRplusOne;

    mpz_sub_ui(mpzE, n.get_mpz_t(), 1);
    mpz_tdiv_q_2exp(mpzE, mpzE, 1);
    mpz_powm(mpzR, mpzTwo.get_mpz_t(), mpzE, n.get_mpz_t());
    unsigned int nMod8 = mpz_get_ui(n.get_mpz_t()) % 8;
    bool fPassedTest = false;
    if (fSophieGermain && (nMod8 == 7)) // Euler & Lagrange
        fPassedTest = !mpz_cmp_ui(mpzR, 1);
    else if (fSophieGermain && (nMod8 == 3)) // Lifchitz
    {
        mpz_add_ui(mpzRplusOne, mpzR, 1);
        fPassedTest = !mpz_cmp(mpzRplusOne, n.get_mpz_t());
    }
    else if ((!fSophieGermain) && (nMod8 == 5)) // Lifchitz
    {
        mpz_add_ui(mpzRplusOne, mpzR, 1);
        fPassedTest = !mpz_cmp(mpzRplusOne, n.get_mpz_t());
    }
    else if ((!fSophieGermain) && (nMod8 == 1)) // LifChitz
        fPassedTest = !mpz_cmp_ui(mpzR, 1);
    else
    {   printf("EulerLagrangeLifchitzPrimalityTest() : invalid n %% 8 = %d, %s", nMod8, (fSophieGermain? "first kind" : "second kind"));
		return false;
	}
    
    if (fPassedTest)
    {
        return true;
    }
    return false;
}

// Test Probable Cunningham Chain for: n
// fSophieGermain:
//   true - Test for Cunningham Chain of first kind (n, 2n+1, 4n+3, ...)
//   false - Test for Cunningham Chain of second kind (n, 2n-1, 4n-3, ...)
// Return value:
//   true - Probable Cunningham Chain found (length at least 2)
//   false - Not Cunningham Chain
static bool ProbableCunninghamChainTestFast(const mpz_class& n, bool fSophieGermain, bool fFermatTest, unsigned int& nProbableChainLength, CPrimalityTestParams& testParams)
{
    nProbableChainLength = 0;

    // Fermat test for n first
    if (!FermatProbablePrimalityTestFast(n, testParams))
        return false;

    // Euler-Lagrange-Lifchitz test for the following numbers in chain
    mpz_class &N = testParams.N;
    N = n;
    while (true)
    {
        nProbableChainLength++;
        N <<= 1;
        N += (fSophieGermain? 1 : (-1));
        if (fFermatTest)
        {
            if (!FermatProbablePrimalityTestFast(N, testParams))
                break;
        }
        else
        {
            if (!EulerLagrangeLifchitzPrimalityTestFast(N, fSophieGermain, testParams))
                break;
        }
    }

    return (nProbableChainLength >= 2);
}

// Test probable prime chain for: nOrigin
// Return value:
//   true - Probable prime chain found (one of nChainLength meeting target)
//   false - prime chain too short (none of nChainLength meeting target)
static bool ProbablePrimeChainTestFast(const mpz_class& mpzPrimeChainOrigin, CPrimalityTestParams& testParams)
{
    const unsigned int nCandidateType = testParams.nCandidateType;
    unsigned int& nChainLength = testParams.nChainLength;
    mpz_class& mpzOriginMinusOne = testParams.mpzOriginMinusOne;
    mpz_class& mpzOriginPlusOne = testParams.mpzOriginPlusOne;
    nChainLength = 0;

    // Test for Cunningham Chain of first kind
    if (nCandidateType == PRIME_CHAIN_CUNNINGHAM1)
    {
        mpzOriginMinusOne = mpzPrimeChainOrigin - 1;
        ProbableCunninghamChainTestFast(mpzOriginMinusOne, true, false, nChainLength, testParams);
    }
    else if (nCandidateType == PRIME_CHAIN_CUNNINGHAM2)
    {
        // Test for Cunningham Chain of second kind
        mpzOriginPlusOne = mpzPrimeChainOrigin + 1;
        ProbableCunninghamChainTestFast(mpzOriginPlusOne, false, false, nChainLength, testParams);
    }
    else
    {
        unsigned int nChainLengthCunningham1 = 0;
        unsigned int nChainLengthCunningham2 = 0;
        mpzOriginMinusOne = mpzPrimeChainOrigin - 1;
        if (ProbableCunninghamChainTestFast(mpzOriginMinusOne, true, false, nChainLengthCunningham1, testParams))
        {
            mpzOriginPlusOne = mpzPrimeChainOrigin + 1;
            ProbableCunninghamChainTestFast(mpzOriginPlusOne, false, false, nChainLengthCunningham2, testParams);
            // Figure out BiTwin Chain length
            // BiTwin Chain allows a single prime at the end for odd length chain
            nChainLength =
                (nChainLengthCunningham1 > nChainLengthCunningham2)?
                    (nChainLengthCunningham2 + nChainLengthCunningham2+1) :
                    (nChainLengthCunningham1 + nChainLengthCunningham1);
        }
    }

    return (nChainLength >= 6);
}

unsigned int PrimeChainTest(uint256& hash, uint64_t nFix, uint64_t nTry, unsigned int nCandidateType)
{
	mpz_class mpzChainOrigin;
	mpz_set_uint256(mpzChainOrigin.get_mpz_t(), hash);
	mpzChainOrigin*=nFix;
	mpzChainOrigin*=nTry;
	
	CPrimalityTestParams testParams;
	unsigned int& nChainLength = testParams.nChainLength;
    testParams.nCandidateType=nCandidateType;
    if (ProbablePrimeChainTestFast(mpzChainOrigin, testParams))
    {
        printf("Probable prime chain found =%s!!\n Chain: %s:%d\n", mpzChainOrigin.get_str().c_str()
			, (nCandidateType==PRIME_CHAIN_CUNNINGHAM1)? "1CC" : ((nCandidateType==PRIME_CHAIN_CUNNINGHAM2)? "2CC" : "TWN")
			, nChainLength);
    }
	return nChainLength;
}
