#include "stdafx.h"
#include "Random.h"
#include "Util.h"

// CRandom --------------------------------------------------------------------

IMPRTTI(CRandom, CObject)

// CNoise2D -------------------------------------------------------------------

IMPRTTI(CNoise2D, CObject)

CNoise2D::CNoise2D()
{
  for (int i = 0; i < MAX_OCTAVES; i++)
    m_fOctaveWeights[i] = (1 << i) / (float) ((1 << MAX_OCTAVES) - 1);
}

void CNoise2D::Init(float *pOctaveWeights, bool bNormalize)
{
  ASSERT(pOctaveWeights);
  int i;
  float fSum;

  fSum = 0;
  for (i = 0; i < MAX_OCTAVES; i++) {
    m_fOctaveWeights[i] = pOctaveWeights[i];
    if (pOctaveWeights[i] < 0)
      break;
    fSum += pOctaveWeights[i];
  }
  if (bNormalize && !Util::IsEqual(fSum, 0))
    for (i = 0; i < MAX_OCTAVES && m_fOctaveWeights[i] >= 0; i++)
      m_fOctaveWeights[i] /= fSum;
}

float CNoise2D::Get(float fX, float fY)
{
  int i;
  float fValue;

  fValue = 0;
  for (i = 0; i < MAX_OCTAVES; i++) {
    if (m_fOctaveWeights[i] < 0)
      break;
    fValue += m_fOctaveWeights[i] * SampleOctave(fX, fY);
    fX /= 2;
    fY /= 2;
  }

  return fValue;
}

float CNoise2D::SampleOctave(float fX, float fY)
{
  ASSERT(fX >= 0 && fY >= 0);

  int iX, iY;
  float fAx, fAy;
  float fV00, fV01, fV10, fV11, fV0, fV1, fV;

  iX = (int) fX;
  iY = (int) fY;
  fAx = fX - iX;
  fAy = fY - iY;
  fV00 = (float) CRandom::MapToRange(CRandom::GeneratePseudo(iX    , iY    ), 0.0f, 1.0f);
  fV01 = (float) CRandom::MapToRange(CRandom::GeneratePseudo(iX    , iY + 1), 0.0f, 1.0f);
  fV10 = (float) CRandom::MapToRange(CRandom::GeneratePseudo(iX + 1, iY    ), 0.0f, 1.0f);
  fV11 = (float) CRandom::MapToRange(CRandom::GeneratePseudo(iX + 1, iY + 1), 0.0f, 1.0f);
  fV0 = Util::SmoothLerp(fV00, fV01, fAy);
  fV1 = Util::SmoothLerp(fV10, fV11, fAy);
  fV  = Util::SmoothLerp(fV0, fV1, fAx);

  return fV;
}
