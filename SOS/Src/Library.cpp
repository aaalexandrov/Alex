#include "stdafx.h"
#include "Library.h"
#include "Interpreter.h"

EInterpretError CFunctionLibrary::Init(CInterpreter &kInterpreter)
{
  kInterpreter.SetGlobal(CValue(CStrAny(ST_CONST, "print").GetHeader()), CValue(Print));
  kInterpreter.SetGlobal(CValue(CStrAny(ST_CONST, "dump").GetHeader()), CValue(Dump));
  kInterpreter.SetGlobal(CValue(CStrAny(ST_CONST, "next").GetHeader()), CValue(Next));
  kInterpreter.SetGlobal(CValue(CStrAny(ST_CONST, "type").GetHeader()), CValue(Type));
  kInterpreter.SetGlobal(CValue(CStrAny(ST_CONST, "tostring").GetHeader()), CValue(ToString));
  kInterpreter.SetGlobal(CValue(CStrAny(ST_CONST, "tonumber").GetHeader()), CValue(ToNumber));

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

EInterpretError CFunctionLibrary::Next(CExecution &kExecution, CArray<CValue> &arrParams)
{
  if (arrParams.m_iCount < 1 || arrParams[0].m_btType != CValue::VT_TABLE)
    return IERR_INVALID_OPERAND;
  CValue::THash::TIter it;
  if (arrParams.m_iCount < 2 || arrParams[1].m_btType == CValue::VT_NONE)
    it = arrParams[0].m_pTableValue->m_Hash;
  else {
    it = arrParams[0].m_pTableValue->m_Hash.Find(arrParams[1]);
    if (it)
      ++it;
  }

  if (it) {
    arrParams.SetCount(2);
    arrParams[0] = (*it).m_Key;
    arrParams[1] = (*it).m_Val;
  } else
    arrParams.SetCount(0);

  return IERR_OK;
}

EInterpretError CFunctionLibrary::Type(CExecution &kExecution, CArray<CValue> &arrParams)
{
  for (int i = 0; i < arrParams.m_iCount; ++i)
    arrParams[i] = CValue(CValue::s_VT2Str.GetStr(arrParams[i].m_btType).GetHeader());
  return IERR_OK;
}

EInterpretError CFunctionLibrary::ToString(CExecution &kExecution, CArray<CValue> &arrParams)
{
  for (int i = 0; i < arrParams.m_iCount; ++i)
    arrParams[i] = CValue(arrParams[i].GetStr(false).GetHeader());
  return IERR_OK;
}

EInterpretError CFunctionLibrary::ToNumber(CExecution &kExecution, CArray<CValue> &arrParams)
{
  for (int i = 0; i < arrParams.m_iCount; ++i) {
    if (arrParams[i].m_btType == CValue::VT_FLOAT)
      continue;
    if (arrParams[i].m_btType == CValue::VT_STRING) {
      float fVal;
      if (Parse::Str2Float(fVal, CStrAny(arrParams[i].m_pStrValue)))
        arrParams[i] = CValue(fVal);
      else
        arrParams[i].ClearValue();
    } else
      arrParams[i].ClearValue();
  }
  return IERR_OK;
}
