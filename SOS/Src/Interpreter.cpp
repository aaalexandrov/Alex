#include "stdafx.h"
#include "Interpreter.h"

CInterpreter::CInterpreter() : m_kExecution(this, 0), m_pGlobalEnvironment(NEW(CValueTable, (&m_kValueRegistry)))
{
  CFunctionLibrary::Init(*this);
  SetCollectionThreshold();
}

CInterpreter::~CInterpreter()
{
}

EInterpretError CInterpreter::Execute(CValue const &kCode, CArray<CValue> &arrParams)
{
  EInterpretError err;
  if (kCode.m_btType == CValue::VT_CLOSURE) {
    err = m_kExecution.Execute(kCode.m_pClosure, arrParams);
    if (err != IERR_OK)
      return err;
    m_kExecution.GetReturnValues(arrParams);
  } else
    if (kCode.m_btType == CValue::VT_NATIVE_FUNC) {
      err = m_kExecution.Execute(kCode.m_pNativeFunc, arrParams);
      if (err != IERR_OK)
        return err;
    }
  return IERR_OK;
}

EInterpretError CInterpreter::CollectGarbage()
{
  m_kValueRegistry.CollectGarbage(this);
  SetCollectionThreshold();
  return IERR_OK;
}

EInterpretError CInterpreter::CheckForCollection()
{
  EInterpretError err = IERR_OK;
  if (GetMemoryUsed() >= m_uiCollectionThreshold) {
    err = CollectGarbage();
    ASSERT(GetMemoryUsed() < m_uiCollectionThreshold);
  }
  return err;
}
