#ifndef XM_SIEVE_H
#define XM_SIEVE_H

#include "uint256.h"
#include "prime.h"



typedef unsigned long sieve_word_t;

// Sieve of Eratosthenes for proof-of-work mining
//
// Includes the sieve extension feature from jhPrimeminer by jh000
//
// A layer of the sieve determines whether the CC1 or CC2 chain members near the
// origin fixed_multiplier * candidate_multiplier * 2^k are known to be
// composites.
//
// The default sieve is composed of layers 1 .. nChainLength.
//
// An extension i is composed of layers i .. i + nChainLength. The candidates
// indexes from the extensions are multiplied by 2^i. The first half of the
// candidates are covered by the default sieve and previous extensions.
//
// The larger numbers in the extensions have a slightly smaller probability of
// being primes and take slightly longer to test but they can be calculated very
// efficiently because the layers overlap.
class CSieveOfEratosthenes
{
    unsigned int nSieveSize; // size of the sieve
    unsigned int nSievePercentage; // weave up to a percentage of primes
    unsigned int nSieveExtensions; // extend the sieve a given number of times
    unsigned int nBits; // target of the prime chain to search for

    // final set of candidates for probable primality checking
    sieve_word_t *vfCandidates;
    sieve_word_t *vfCompositeBiTwin;
    sieve_word_t *vfCompositeCunningham1;
    sieve_word_t *vfCompositeCunningham2;

    // extended sets
    sieve_word_t *vfExtendedCandidates;
    sieve_word_t *vfExtendedCompositeBiTwin;
    sieve_word_t *vfExtendedCompositeCunningham1;
    sieve_word_t *vfExtendedCompositeCunningham2;

    static const unsigned int nWordBits = 8 * sizeof(sieve_word_t);
    unsigned int nCandidatesWords;
    unsigned int nCandidatesBytes;

    unsigned int nPrimeSeq; // prime sequence number currently being processed
    unsigned int nCandidateCount; // cached total count of candidates
    uint64_t nCandidateMultiplier; // current candidate for power test
    unsigned int nCandidateIndex; // internal candidate index
    bool fCandidateIsExtended; // is the current candidate in the extended part
    unsigned int nCandidateActiveExtension; // which extension is active

    unsigned int nChainLength; // target chain length
    unsigned int nSieveLayers; // sieve layers
    unsigned int nPrimes; // number of times to weave the sieve

	unsigned int nStart;
    
	unsigned int nMultiplierBytes;
	unsigned int *vCunningham1Multipliers;
	unsigned int *vCunningham2Multipliers;
	sieve_word_t *vfCompositeLayerCC1;
	sieve_word_t *vfCompositeLayerCC2;



    unsigned int GetWordNum(unsigned int nBitNum) 
	{
        return nBitNum / nWordBits;
    }

    sieve_word_t GetBitMask(unsigned int nBitNum) 
	{
        return (sieve_word_t)1 << (nBitNum % nWordBits);
    }

    void ProcessMultiplier(sieve_word_t *vfComposites, const unsigned int nMinMultiplier, const unsigned int nMaxMultiplier, const std::vector<unsigned int>& vPrimes, unsigned int *vMultipliers, unsigned int nLayerSeq)
	{
    // Wipe the part of the array first
    	if (nMinMultiplier < nMaxMultiplier)
        	memset(vfComposites + GetWordNum(nMinMultiplier), 0, (nMaxMultiplier - nMinMultiplier + nWordBits - 1) / nWordBits * sizeof(sieve_word_t));

    	for (unsigned int nPrimeSeq = 1; nPrimeSeq < nPrimes; nPrimeSeq++)
    	{
        	const unsigned int nPrime = vPrimes[nPrimeSeq];
        	unsigned int nVariableMultiplier = vMultipliers[nPrimeSeq * nSieveLayers + nLayerSeq];
        	if (nVariableMultiplier < nMinMultiplier)
            	nVariableMultiplier += (nMinMultiplier - nVariableMultiplier + nPrime - 1) / nPrime * nPrime;
        	const unsigned int nRotateBits = nPrime % nWordBits;
        	sieve_word_t lBitMask = GetBitMask(nVariableMultiplier);
        	for (; nVariableMultiplier < nMaxMultiplier; nVariableMultiplier += nPrime)
        	{
            	vfComposites[GetWordNum(nVariableMultiplier)] |= lBitMask;
            	lBitMask = (lBitMask << nRotateBits) | (lBitMask >> (nWordBits - nRotateBits));
        	}
        	vMultipliers[nPrimeSeq * nSieveLayers + nLayerSeq] = nVariableMultiplier;
    	}
	}

	
public:
    CSieveOfEratosthenes(unsigned int nSieveSize, unsigned int nSievePercentage, unsigned int nSieveExtensions, unsigned int nChainLength)
    {
        this->nSieveSize = nSieveSize;
        this->nSievePercentage = nSievePercentage;
        this->nSieveExtensions = nSieveExtensions;
        this->nChainLength = nChainLength;
        nPrimeSeq = 0;
        nCandidateCount = 0;
        nCandidateMultiplier = 0;
        nCandidateIndex = 0;
        fCandidateIsExtended = false;
        nCandidateActiveExtension = 0;
        nCandidatesWords = (nSieveSize + nWordBits - 1) / nWordBits;
        nCandidatesBytes = nCandidatesWords * sizeof(sieve_word_t);
        vfCandidates = (sieve_word_t *)malloc(nCandidatesBytes);
        vfCompositeBiTwin = (sieve_word_t *)malloc(nCandidatesBytes);
        vfCompositeCunningham1 = (sieve_word_t *)malloc(nCandidatesBytes);
        vfCompositeCunningham2 = (sieve_word_t *)malloc(nCandidatesBytes);
        vfExtendedCandidates = (sieve_word_t *)malloc(nSieveExtensions * nCandidatesBytes);
        vfExtendedCompositeBiTwin = (sieve_word_t *)malloc(nSieveExtensions * nCandidatesBytes);
        vfExtendedCompositeCunningham1 = (sieve_word_t *)malloc(nSieveExtensions * nCandidatesBytes);
        vfExtendedCompositeCunningham2 = (sieve_word_t *)malloc(nSieveExtensions * nCandidatesBytes);
        nSieveLayers = nChainLength + nSieveExtensions;

        // Process only a set percentage of the primes
        // Most composites are still found
        const unsigned int nTotalPrimes = vPrimes.size();
        nPrimes = (uint64)nTotalPrimes * nSievePercentage / 100;
		nMultiplierBytes = nPrimes * nSieveLayers * sizeof(unsigned int);
	    vCunningham1Multipliers = (unsigned int *)malloc(nMultiplierBytes);
	    vCunningham2Multipliers = (unsigned int *)malloc(nMultiplierBytes);
		vfCompositeLayerCC1 = (sieve_word_t *)malloc(nCandidatesBytes);
	    vfCompositeLayerCC2 = (sieve_word_t *)malloc(nCandidatesBytes);

    }

    ~CSieveOfEratosthenes()
    {
        free(vfCandidates);
        free(vfCompositeBiTwin);
        free(vfCompositeCunningham1);
        free(vfCompositeCunningham2);
        free(vfExtendedCandidates);
        free(vfExtendedCompositeBiTwin);
        free(vfExtendedCompositeCunningham1);
        free(vfExtendedCompositeCunningham2);
		free(vCunningham1Multipliers);
		free(vCunningham2Multipliers);
		free(vfCompositeLayerCC1);
		free(vfCompositeLayerCC2);
    }

    // Get total number of candidates for power test
    unsigned int GetCandidateCount()
    {
        if (nCandidateCount)
            return nCandidateCount;

        unsigned int nCandidates = 0;
#ifdef __GNUC__
        for (unsigned int i = 0; i < nCandidatesWords; i++)
            nCandidates += __builtin_popcountl(vfCandidates[i]);
        for (unsigned int j = 0; j < nSieveExtensions; j++)
            for (unsigned int i = nCandidatesWords / 2; i < nCandidatesWords; i++)
                nCandidates += __builtin_popcountl(vfExtendedCandidates[j * nCandidatesWords + i]);
#else
        for (unsigned int i = 0; i < nCandidatesWords; i++)
        {
            sieve_word_t lBits = vfCandidates[i];
            for (unsigned int j = 0; j < nWordBits; j++)
            {
                nCandidates += (lBits & 1);
                lBits >>= 1;
            }
        }
        for (unsigned int j = 0; j < nSieveExtensions; j++)
        {
            for (unsigned int i = nCandidatesWords / 2; i < nCandidatesWords; i++)
            {
                sieve_word_t lBits = vfExtendedCandidates[j * nCandidatesWords + i];
                for (unsigned int j = 0; j < nWordBits; j++)
                {
                    nCandidates += (lBits & 1);
                    lBits >>= 1;
                }
            }
        }
#endif
        nCandidateCount = nCandidates;
        return nCandidates;
    }

    // Scan for the next candidate multiplier (variable part)
    // Return values:
    //   True - found next candidate; nVariableMultiplier has the candidate
    //   False - scan complete, no more candidate and reset scan
    bool GetNextCandidateMultiplier(uint64_t& nVariableMultiplier, unsigned int& nCandidateType)
    {
        sieve_word_t *vfActiveCandidates;
        sieve_word_t *vfActiveCompositeTWN;
        sieve_word_t *vfActiveCompositeCC1;

        if (fCandidateIsExtended)
        {
            vfActiveCandidates = vfExtendedCandidates + nCandidateActiveExtension * nCandidatesWords;
            vfActiveCompositeTWN = vfExtendedCompositeBiTwin + nCandidateActiveExtension * nCandidatesWords;
            vfActiveCompositeCC1 = vfExtendedCompositeCunningham1 + nCandidateActiveExtension * nCandidatesWords;
        }
        else
        {
            vfActiveCandidates = vfCandidates;
            vfActiveCompositeTWN = vfCompositeBiTwin;
            vfActiveCompositeCC1 = vfCompositeCunningham1;
        }

        // Acquire the current word from the bitmap
        sieve_word_t lBits = vfActiveCandidates[GetWordNum(nCandidateIndex)];

        for(;;)
        {
            nCandidateIndex++;
            if (nCandidateIndex >= nSieveSize)
            {
                // Check if extensions are available
                if (!fCandidateIsExtended && nSieveExtensions > 0)
                {
                    fCandidateIsExtended = true;
                    nCandidateActiveExtension = 0;
                    nCandidateIndex = nSieveSize / 2;
                }
                else if (fCandidateIsExtended && nCandidateActiveExtension + 1 < nSieveExtensions)
                {
                    nCandidateActiveExtension++;
                    nCandidateIndex = nSieveSize / 2;
                }
                else
                {
                    // Out of candidates
                    fCandidateIsExtended = false;
                    nCandidateActiveExtension = 0;
                    nCandidateIndex = 0;
                    nCandidateMultiplier = 0;
                    return false;
                }

                // Fix the pointers
                if (fCandidateIsExtended)
                {
                    vfActiveCandidates = vfExtendedCandidates + nCandidateActiveExtension * nCandidatesWords;
                    vfActiveCompositeTWN = vfExtendedCompositeBiTwin + nCandidateActiveExtension * nCandidatesWords;
                    vfActiveCompositeCC1 = vfExtendedCompositeCunningham1 + nCandidateActiveExtension * nCandidatesWords;
                }
                else
                {
                    vfActiveCandidates = vfCandidates;
                    vfActiveCompositeTWN = vfCompositeBiTwin;
                    vfActiveCompositeCC1 = vfCompositeCunningham1;
                }
            }

            if (nCandidateIndex % nWordBits == 0)
            {
                // Update the current word
                lBits = vfActiveCandidates[GetWordNum(nCandidateIndex)];

                // Check if any bits are set
                if (lBits == 0)
                {
                    // Skip an entire word
                    nCandidateIndex += nWordBits - 1;
                    continue;
                }
            }

            if (lBits & GetBitMask(nCandidateIndex))
            {
            	if (fCandidateIsExtended)
                    nCandidateMultiplier = ((uint64_t)nCandidateIndex+nStart) * (2 << nCandidateActiveExtension);
                else
                    nCandidateMultiplier = nCandidateIndex+nStart;
                nVariableMultiplier = nCandidateMultiplier;
                if (~vfActiveCompositeTWN[GetWordNum(nCandidateIndex)] & GetBitMask(nCandidateIndex))
                    nCandidateType = PRIME_CHAIN_BI_TWIN;
                else if (~vfActiveCompositeCC1[GetWordNum(nCandidateIndex)] & GetBitMask(nCandidateIndex))
                    nCandidateType = PRIME_CHAIN_CUNNINGHAM1;
                else
                    nCandidateType = PRIME_CHAIN_CUNNINGHAM2;
				
                return true;
            }
        }
    }

    // Get progress percentage of the sieve
    unsigned int GetProgressPercentage()
	{
	    return std::min(100u, (((nPrimeSeq >= vPrimes.size())? nSieveSize : vPrimes[nPrimeSeq]) * 100 / nSieveSize));
	}
	
	unsigned int int_invert(unsigned int a, unsigned int nPrime)
	{
	    // Extended Euclidean algorithm to calculate the inverse of a in finite field defined by nPrime
	    int rem0 = nPrime, rem1 = a % nPrime, rem2;
	    int aux0 = 0, aux1 = 1, aux2;
	    int quotient, inverse;

	    while (1)
	    {
	        if (rem1 <= 1)
	        {
	            inverse = aux1;
	            break;
	        }

	        rem2 = rem0 % rem1;
	        quotient = rem0 / rem1;
	        aux2 = -quotient * aux1 + aux0;

	        if (rem2 <= 1)
	        {
	            inverse = aux2;
	            break;
	        }

	        rem0 = rem1 % rem2;
	        quotient = rem1 / rem2;
	        aux0 = -quotient * aux2 + aux1;

	        if (rem0 <= 1)
	        {
	            inverse = aux0;
	            break;
	        }

	        rem1 = rem2 % rem0;
	        quotient = rem2 / rem0;
	        aux1 = -quotient * aux0 + aux2;
	    }

	    return (inverse + nPrime) % nPrime;
	}

	void prepare()
	{
		nCandidateIndex = 0;
		nPrimeSeq = 0;
		nCandidateCount = 0;
		nCandidateMultiplier = 0;
		nCandidateIndex = 0;
		fCandidateIsExtended = false;
		nCandidateActiveExtension = 0;

        memset(vfCandidates, 0, nCandidatesBytes);
        memset(vfCompositeBiTwin, 0, nCandidatesBytes);
        memset(vfCompositeCunningham1, 0, nCandidatesBytes);
        memset(vfCompositeCunningham2, 0, nCandidatesBytes);
        
		memset(vfExtendedCandidates, 0, nSieveExtensions * nCandidatesBytes);
        memset(vfExtendedCompositeBiTwin, 0, nSieveExtensions * nCandidatesBytes);
        memset(vfExtendedCompositeCunningham1, 0, nSieveExtensions * nCandidatesBytes);
        memset(vfExtendedCompositeCunningham2, 0, nSieveExtensions * nCandidatesBytes);
 
	    memset(vCunningham1Multipliers, 0xFF, nMultiplierBytes);
	    memset(vCunningham2Multipliers, 0xFF, nMultiplierBytes);
	}
    // Weave the sieve for the next prime in table
    // Return values:
    //   True  - weaved another prime; nComposite - number of composites removed
    //   False - sieve already completed
    bool Weave( uint256& hash, unsigned long nFixedMultiplier, uint32_t start, bool& stop )
	{
		nStart = start;
        prepare();
	    unsigned int nCombinedEndSeq = 1;
	    unsigned int nFixedFactorCombinedMod = 0;

	    for (unsigned int nPrimeSeqLocal = 1; nPrimeSeqLocal < nPrimes; nPrimeSeqLocal++)
	    {
	        if (stop)
	            break;  // new block
	        unsigned int nPrime = vPrimes[nPrimeSeqLocal];
	        if (nPrimeSeqLocal >= nCombinedEndSeq)
	        {
            // Combine multiple primes to produce a big divisor
	            unsigned int nPrimeCombined = 1;
	            while (nPrimeCombined < UINT_MAX / vPrimes[nCombinedEndSeq])
	            {
	                nPrimeCombined *= vPrimes[nCombinedEndSeq];
	                nCombinedEndSeq++;
	            }

	            nFixedFactorCombinedMod = hash.mod32(nPrimeCombined);
	            nFixedFactorCombinedMod = (uint64)nFixedFactorCombinedMod * (nFixedMultiplier % nPrimeCombined) % nPrimeCombined;
	        }

	        unsigned int nFixedFactorMod = nFixedFactorCombinedMod % nPrime;
	        if (nFixedFactorMod == 0)
	        {
	            // Nothing in the sieve is divisible by this prime
							//printf("0");
	            continue;
	        }
					//printf("*");
	        // Find the modulo inverse of fixed factor
	        unsigned int nFixedInverse = int_invert(nFixedFactorMod, nPrime);
	        if (!nFixedInverse) {
	            printf("CSieveOfEratosthenes::Weave(): int_invert of fixed factor failed for prime #%u=%u", nPrimeSeqLocal, vPrimes[nPrimeSeqLocal]);
				return false;
			}
	        unsigned int nTwoInverse = (nPrime + 1) / 2;

	        // Check whether 32-bit arithmetic can be used for nFixedInverse
	        const unsigned int nnStart = nPrime-nStart%nPrime;
	        // Weave the sieve for the prime
	        for (unsigned int nChainSeq = 0; nChainSeq < nSieveLayers; nChainSeq++)
	        {
	        	// Find the first number that's divisible by this prime
	            vCunningham1Multipliers[nPrimeSeqLocal * nSieveLayers + nChainSeq] = (nFixedInverse+nnStart)%nPrime;
	            vCunningham2Multipliers[nPrimeSeqLocal * nSieveLayers + nChainSeq] = (nPrime - nFixedInverse+nnStart)%nPrime;
                // For next number in chain
                nFixedInverse = (uint64)nFixedInverse * nTwoInverse % nPrime;
            }
	    }

	    // Number of elements that are likely to fit in L1 cache
	    // NOTE: This needs to be a multiple of nWordBits
	    const unsigned int nL1CacheElements = 224000;
	    const unsigned int nArrayRounds = (nSieveSize + nL1CacheElements - 1) / nL1CacheElements;

	    // Calculate the number of CC1 and CC2 layers needed for BiTwin candidates
	    const unsigned int nBiTwinCC1Layers = (nChainLength + 1) / 2;
	    const unsigned int nBiTwinCC2Layers = nChainLength / 2;

	    // Only 50% of the array is used in extensions
	    const unsigned int nExtensionsMinMultiplier = nSieveSize / 2;
	    const unsigned int nExtensionsMinWord = nExtensionsMinMultiplier / nWordBits;

	    // Loop over each array one at a time for optimal L1 cache performance
	    for (unsigned int j = 0; j < nArrayRounds; j++)
	    {
	        const unsigned int nMinMultiplier = nL1CacheElements * j;
	        const unsigned int nMaxMultiplier = std::min(nL1CacheElements * (j + 1), nSieveSize);
	        const unsigned int nExtMinMultiplier = std::max(nMinMultiplier, nExtensionsMinMultiplier);
	        const unsigned int nMinWord = nMinMultiplier / nWordBits;
	        const unsigned int nMaxWord = (nMaxMultiplier + nWordBits - 1) / nWordBits;
	        const unsigned int nExtMinWord = std::max(nMinWord, nExtensionsMinWord);
	        if (stop)
	            break;  // new block

	        // Loop over the layers
	        for (unsigned int nLayerSeq = 0; nLayerSeq < nSieveLayers; nLayerSeq++) {
	            if (stop)
	                break;  // new block
	            if (nLayerSeq < nChainLength)
	            {
	                ProcessMultiplier(vfCompositeLayerCC1, nMinMultiplier, nMaxMultiplier, vPrimes, vCunningham1Multipliers, nLayerSeq);
	                ProcessMultiplier(vfCompositeLayerCC2, nMinMultiplier, nMaxMultiplier, vPrimes, vCunningham2Multipliers, nLayerSeq);
	            }
	            else
	            {
	                // Optimize: First halves of the arrays are not needed in the extensions
	                ProcessMultiplier(vfCompositeLayerCC1, nExtMinMultiplier, nMaxMultiplier, vPrimes, vCunningham1Multipliers, nLayerSeq);
	                ProcessMultiplier(vfCompositeLayerCC2, nExtMinMultiplier, nMaxMultiplier, vPrimes, vCunningham2Multipliers, nLayerSeq);
	            }

	            // Apply the layer to the primary sieve arrays
	            if (nLayerSeq < nChainLength)
	            {
	                if (nLayerSeq < nBiTwinCC2Layers)
	                {
	                    for (unsigned int nWord = nMinWord; nWord < nMaxWord; nWord++)
	                    {
	                        vfCompositeCunningham1[nWord] |= vfCompositeLayerCC1[nWord];
	                        vfCompositeCunningham2[nWord] |= vfCompositeLayerCC2[nWord];
	                        vfCompositeBiTwin[nWord] |= vfCompositeLayerCC1[nWord] | vfCompositeLayerCC2[nWord];
	                    }
	                }
	                else if (nLayerSeq < nBiTwinCC1Layers)
	                {
	                    for (unsigned int nWord = nMinWord; nWord < nMaxWord; nWord++)
	                    {
	                        vfCompositeCunningham1[nWord] |= vfCompositeLayerCC1[nWord];
	                        vfCompositeCunningham2[nWord] |= vfCompositeLayerCC2[nWord];
	                        vfCompositeBiTwin[nWord] |= vfCompositeLayerCC1[nWord];
	                    }
	                }
	                else
	                {
	                    for (unsigned int nWord = nMinWord; nWord < nMaxWord; nWord++)
	                    {
	                        vfCompositeCunningham1[nWord] |= vfCompositeLayerCC1[nWord];
	                        vfCompositeCunningham2[nWord] |= vfCompositeLayerCC2[nWord];
	                    }
	                }
	            }
	
	            // Apply the layer to extensions
	            for (unsigned int nExtensionSeq = 0; nExtensionSeq < nSieveExtensions; nExtensionSeq++)
	            {
	                const unsigned int nLayerOffset = nExtensionSeq + 1;
	                if (nLayerSeq >= nLayerOffset && nLayerSeq < nChainLength + nLayerOffset)
	                {
	                    const unsigned int nLayerExtendedSeq = nLayerSeq - nLayerOffset;
	                    sieve_word_t *vfExtCC1 = vfExtendedCompositeCunningham1 + nExtensionSeq * nCandidatesWords;
	                    sieve_word_t *vfExtCC2 = vfExtendedCompositeCunningham2 + nExtensionSeq * nCandidatesWords;
	                    sieve_word_t *vfExtTWN = vfExtendedCompositeBiTwin + nExtensionSeq * nCandidatesWords;
	                    if (nLayerExtendedSeq < nBiTwinCC2Layers)
	                    {
	                        for (unsigned int nWord = nExtMinWord; nWord < nMaxWord; nWord++)
	                        {
	                            vfExtCC1[nWord] |= vfCompositeLayerCC1[nWord];
	                            vfExtCC2[nWord] |= vfCompositeLayerCC2[nWord];
	                            vfExtTWN[nWord] |= vfCompositeLayerCC1[nWord] | vfCompositeLayerCC2[nWord];
	                        }
	                    }
	                    else if (nLayerExtendedSeq < nBiTwinCC1Layers)
	                    {
	                        for (unsigned int nWord = nExtMinWord; nWord < nMaxWord; nWord++)
	                        {
	                            vfExtCC1[nWord] |= vfCompositeLayerCC1[nWord];
	                            vfExtCC2[nWord] |= vfCompositeLayerCC2[nWord];
	                            vfExtTWN[nWord] |= vfCompositeLayerCC1[nWord];
	                        }
	                    }
	                    else
	                    {
	                        for (unsigned int nWord = nExtMinWord; nWord < nMaxWord; nWord++)
	                        {
	                            vfExtCC1[nWord] |= vfCompositeLayerCC1[nWord];
	                            vfExtCC2[nWord] |= vfCompositeLayerCC2[nWord];
	                        }
	                    }
	                }
	            }
	        }

	        // Combine the bitsets
	        // vfCandidates = ~(vfCompositeCunningham1 & vfCompositeCunningham2 & vfCompositeBiTwin)
	        for (unsigned int i = nMinWord; i < nMaxWord; i++)
	            vfCandidates[i] = ~(vfCompositeCunningham1[i] & vfCompositeCunningham2[i] & vfCompositeBiTwin[i]);

	        // Combine the extended bitsets
	        for (unsigned int j = 0; j < nSieveExtensions; j++)
	            for (unsigned int i = nExtMinWord; i < nMaxWord; i++)
	                vfExtendedCandidates[j * nCandidatesWords + i] = ~(
	                    vfExtendedCompositeCunningham1[j * nCandidatesWords + i] &
	                    vfExtendedCompositeCunningham2[j * nCandidatesWords + i] &
	                    vfExtendedCompositeBiTwin[j * nCandidatesWords + i]);
	    }

	    // The sieve has been partially weaved
	    this->nPrimeSeq = nPrimes - 1;
	    return false;
	}
};

#endif

