#include "stdafx.h"
#include "Interpreter.h"

EInterpretError CInterpreter::Execute(CValue const &kCode, CArray<CValue> &arrParams)
{
  EInterpretError err;
  if (kCode.m_btType == CValue::VT_FRAGMENT) {
    err = m_kExecution.Execute(kCode.m_pFragment, arrParams);
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
