#ifndef __RANDOM_H
#define __RANDOM_H

#include "Base.h"
#include <time.h>

class CRandom: public CObject {
  DEFRTTI
public:
  static const UINT MAX_NUMBER = 65536;
  UINT m_uiSeed;

  CRandom(): m_uiSeed(0) {}

  void Randomize()    { time_t kTime; m_uiSeed = time(&kTime) % MAX_NUMBER; }
  UINT GetMax() const { return MAX_NUMBER; }
  UINT Generate()     { m_uiSeed = (36473 * m_uiSeed + 20939) % MAX_NUMBER; return m_uiSeed; }

  int   Get()                                  { return (int) Generate(); }
  int   GetRanged(int iMin, int iMax)          { return MapToRange(Generate(), iMin, iMax); }
  float GetFloat()                             { return MapToRange(Generate(), 0.0f, 1.0f); }
  float GetFloatRanged(float fMin, float fMax) { return MapToRange(Generate(), fMin, fMax); }
  
  static inline int   MapToRange(UINT uiRandom, int iMin, int iMax);
  static inline float MapToRange(UINT uiRandom, float fMin, float fMax);

  static UINT GeneratePseudo(UINT uiX, UINT uiY) { return (15559 * uiX * uiY + 28603 * uiX + 32413 * uiY + 45131) % MAX_NUMBER; }
};

class CNoise2D: public CObject {
  DEFRTTI
public:
  static const int MAX_OCTAVES = 16;
  float m_fOctaveWeights[MAX_OCTAVES];

  CNoise2D();

  void Init(float *pOctaveWeights, bool bNormalize); // pOctaveWeights points to an array of no more than MAX_OCTAVES floats. If the array is shorter, it needs to end with a negative weight

  float Get(float fX, float fY);

  static float SampleOctave(float fX, float fY);
};

// Implementation -------------------------------------------------------------

int CRandom::MapToRange(UINT uiRandom, int iMin, int iMax)
{
  return (int) (uiRandom * (iMax - iMin) / (MAX_NUMBER - 1)) + iMin;
}

float CRandom::MapToRange(UINT uiRandom, float fMin, float fMax)
{
  return uiRandom * (fMax - fMin) / (MAX_NUMBER - 1) + fMin;
}

#endif
