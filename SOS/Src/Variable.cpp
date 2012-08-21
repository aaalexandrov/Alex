#include "stdafx.h"
#include "Variable.h"

// CEnvRegistry ---------------------------------------------------------------

void CEnvRegistry::Clear()
{
  m_hashEnvironments.Clear();
  m_hashValues.DeleteAll();
  m_btLastMark = 0;
}

void CEnvRegistry::MarkAndSweep()
{
  THashEnv::TIter itEnv;

  // Mark
  m_btLastMark++;
  if (!m_btLastMark)
    m_btLastMark = 1;
  m_iMarked = 0;
  for (itEnv = m_hashEnvironments; itEnv; ++itEnv) 
    itEnv->Mark(m_btLastMark);

  ASSERT(m_iMarked <= m_hashValues.m_iCount);
  // Do we need to sweep?
  if (m_iMarked == m_hashValues.m_iCount)
    return;

  // Sweep
  CArray<CValueTable *> arrValues(m_hashValues.m_iCount - m_iMarked);
  THashValues::TIter itVal;
  itVal = m_hashValues;
  while (itVal) {
    CValueTable *pValueTable = *itVal;
    ++itVal;
    if (pValueTable->m_btMark != m_btLastMark) // Deletion will also remove the table from the hash
      delete pValueTable;
  }
}
