#include "stdafx.h"
#include "Variable.h"
#include "Execution.h"

// CValue ---------------------------------------------------------------------

CValue2String::TValueString CValue::s_arrVT2Str[VT_LAST] = {
	VAL2NAME(VT_NONE, "nil"),
  VAL2NAME(VT_BOOL, "bool"),
	VAL2NAME(VT_FLOAT, "float"),
	VAL2NAME(VT_STRING, "string"),
	VAL2NAME(VT_TABLE, "table"),
	VAL2NAME(VT_FRAGMENT, "fragment"),
	VAL2NAME(VT_NATIVE_FUNC, "native_func"),
};

CValue2String CValue::s_VT2Str(s_arrVT2Str, ARRSIZE(s_arrVT2Str));


// CValueRegistry -------------------------------------------------------------

void CValueRegistry::Clear()
{
  m_hashValues.DeleteAll();
}

void CValueRegistry::CollectGarbage(CInterpreter *pInterpreter)
{
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
	printf("Instructions: %d, Constants: %d, Locals: %d, Parameters: %d\n", m_arrCode.m_iCount, m_arrConst.m_iCount, m_nLocalCount, m_nParamCount);
	CStrAny s;
	for (int i = 0; i < m_arrConst.m_iCount; ++i) {
		s = m_arrConst[i].GetStr(true);
		printf("%04d: %s\n", i, s.m_pBuf);
	}
  for (int i = 0; i < m_arrCode.m_iCount; ++i) {
		s = m_arrCode[i].ToStr(this);
    printf("%04d: %s\n", i, s.m_pBuf);
	}
}
