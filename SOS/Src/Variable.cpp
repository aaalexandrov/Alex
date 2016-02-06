#include "stdafx.h"
#include "Variable.h"
#include "Execution.h"
#include "Interpreter.h"

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

CValueRegistry::CValueRegistry(): m_pUnprocessed(NEW(THashValues, ())), m_pProcessed(NEW(THashValues, ()))
{
}

CValueRegistry::~CValueRegistry()
{
  DeleteValues(*m_pUnprocessed);
  ASSERT(!m_pUnprocessed->m_iCount && !m_pProcessed->m_iCount && !m_lstProcessing.m_iCount);
  DEL(m_pProcessed);
  DEL(m_pUnprocessed);
}

void CValueRegistry::Add(CValue const &kValue)    
{ 
  m_pUnprocessed->Add(kValue); 
}

void CValueRegistry::Remove(CValue const &kValue) 
{ 
  m_pUnprocessed->RemoveValue(kValue); 
}

void CValueRegistry::DeleteValues(THashValues &hashValues)
{
  for (THashValues::TIter it(&hashValues); it; ++it) 
    (*it).DeleteValue();
  hashValues.Clear();
}

void CValueRegistry::MoveToProcessing(CValue const &kValue)
{
  if (kValue.m_btType != CValue::VT_TABLE && kValue.m_btType != CValue::VT_FRAGMENT)
    return;
  THashValues::TIter it = m_pUnprocessed->Find(kValue);
  if (it) {
    m_lstProcessing.PushTail(kValue);
    m_pUnprocessed->Remove(it);
  }
}

void CValueRegistry::Process(CList<CValue>::TNode *pNode)
{
  ASSERT(pNode->Data.m_btType == CValue::VT_TABLE || pNode->Data.m_btType == CValue::VT_FRAGMENT);
  CValue kValue = pNode->Data;
  m_pProcessed->Add(kValue);
  m_lstProcessing.Remove(pNode);
  if (kValue.m_btType == CValue::VT_TABLE) {
    for (CValue::THash::TIter itVal(&kValue.m_pTableValue->m_Hash); itVal; ++itVal) {
      MoveToProcessing((*itVal).m_Key);
      MoveToProcessing((*itVal).m_Val);
    }
  } else 
    if (kValue.m_btType == CValue::VT_FRAGMENT) {
      for (int i = 0; i < kValue.m_pFragment->m_arrConst.m_iCount; ++i)
        MoveToProcessing(kValue.m_pFragment->m_arrConst[i]);
    }
}

void CValueRegistry::CollectGarbage(CInterpreter *pInterpreter)
{
  ASSERT(!m_pProcessed->m_iCount && !m_lstProcessing.m_iCount);
  MoveToProcessing(CValue(pInterpreter->m_pGlobalEnvironment));
  ASSERT(m_lstProcessing.m_iCount);
  while (m_lstProcessing.m_iCount) 
    Process(m_lstProcessing.m_pHead);

  DeleteValues(*m_pUnprocessed);
  Util::Swap(m_pUnprocessed, m_pProcessed);
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
