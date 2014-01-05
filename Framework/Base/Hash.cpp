#include "stdafx.h"
#include "Hash.h"
#include <stdio.h>

// Prime numbers to be used as hash table size. 
// Those are the the largest primes, not exceeding the members of a 
// geometric progression with a base of 32 and a multiplier of 1.25.
const int g_iHashSizes[68] = {
        31,        37,        47,        61,        73,        89,       113,       149,
       181,       233,       283,       359,       449,       563,       701,       883,
      1103,      1373,      1721,      2143,      2689,      3361,      4201,      5237,
      6563,      8191,     10253,     12821,     16007,     20029,     25037,     31277,
     39119,     48889,     61129,     76403,     95507,    119389,    149239,    186551,
    233183,    291491,    364349,    455443,    569323,    711653,    889519,   1111949,
   1389943,   1737433,   2171777,   2714749,   3393437,   4241779,   5302237,   6627793,
   8284739,  10355869,  12944923,  16181153,  20226421,  25283051,  31603829,  39504793,
  49380967,  61726243,  77157793,  96447251,
};

// Here's the routine to generate the sequence up to whatever threshold is needed

void PrimeSizes(int iThresh)
{
  CArray<int> iPrimes;
  int i, iCandidate, iNextThresh, iRoot;
  bool bFound;
  iNextThresh = 32;
  iPrimes.Append(2);
  iCandidate = 2;
  while (iNextThresh < iThresh) {
    do {
      ++iCandidate;
      if (iCandidate > iNextThresh) {
        printf("%10d", iPrimes[iPrimes.m_iCount - 1]);
        iNextThresh = iNextThresh + iNextThresh / 4;
      }
      iRoot = (int) sqrt((double) iCandidate);
      bFound = true;
      for (i = 0; i < iPrimes.m_iCount && iPrimes.m_pArray[i] <= iRoot; i++)
        if (!(iCandidate % iPrimes.m_pArray[i])) {
          bFound = false;
          break;
        }
    } while (!bFound);
    iPrimes.Append(iCandidate);
  }
}
