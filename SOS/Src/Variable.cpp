#include "stdafx.h"
#include "Variable.h"
#include "Execution.h"

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
  // CArray<CValueTable *> arrValues(m_hashValues.m_iCount - m_iMarked);
  THashValues::TIter itVal;
  itVal = m_hashValues;
  while (itVal) {
    CValueTable *pValueTable = *itVal;
    ++itVal;
    if (pValueTable->m_btMark != m_btLastMark) // Deletion will also remove the table from the hash
      delete pValueTable;
  }
}

// CFragment ------------------------------------------------------------------

EInterpretError CFragment::Execute(CExecution *pExecution)
{
  while (pExecution->m_pNextInstruction) {
    CInstruction *pInstruction = pExecution->m_pNextInstruction;
    pExecution->m_pNextInstruction = GetNextInstruction(pInstruction);
    EInterpretError res = pInstruction->Execute(pExecution);
    if (res != IERR_OK)
      return res;
  }
  return IERR_OK;
}

CInstruction *CFragment::GetNextInstruction(CInstruction *pInstruction) const 
{ 
  ASSERT(pInstruction >= m_arrCode.m_pArray && pInstruction < m_arrCode.m_pArray + m_arrCode.m_iCount); 
  if (pInstruction < m_arrCode.m_pArray + m_arrCode.m_iCount - 1)
    return pInstruction + 1;
  return 0;
}

void CFragment::Dump()
{
  for (int i = 0; i < m_arrCode.m_iCount; ++i) 
    printf("%s\n", m_arrCode[i].ToStr().m_pBuf);
}
