#include "stdafx.h"
#include "Library.h"
#include "Interpreter.h"

EInterpretError CFunctionLibrary::Init(CInterpreter &kInterpreter)
{
  kInterpreter.SetGlobal(CValue(CStrAny(ST_CONST, "print").GetHeader()), CValue(Print));
  kInterpreter.SetGlobal(CValue(CStrAny(ST_CONST, "dump").GetHeader()), CValue(Dump));

  return IERR_OK;
}

EInterpretError CFunctionLibrary::Print(CExecution &kExecution, CArray<CValue> &arrParams)
{
  for (int i = 0; i < arrParams.m_iCount; ++i) {
    char const *pFormat = i < arrParams.m_iCount - 1 ? "%s\t" : "%s";
    fprintf(stdout, pFormat, arrParams[i].GetStr(false).m_pBuf);
  }
  fprintf(stdout, "\n");
  
  arrParams.SetCount(0);

  return IERR_OK;
}

EInterpretError CFunctionLibrary::Dump(CExecution &kExecution, CArray<CValue> &arrParams)
{
  for (int i = 0; i < arrParams.m_iCount; ++i) {
		CValue const &kVal = arrParams[i];
		CStrAny sRes = kVal.GetStr(true);
		fprintf(stdout, "<< %s\n", sRes.m_pBuf);
		if (kVal.m_btType == CValue::VT_FRAGMENT)
			kVal.GetFragment()->Dump();
	}

  arrParams.SetCount(0);

  return IERR_OK;
}