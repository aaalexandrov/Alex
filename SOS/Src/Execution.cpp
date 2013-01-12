#include "stdafx.h"
#include "Execution.h"

// CInstruction ---------------------------------------------------------------

CValue2String::TValueString CInstruction::s_arrIT2Str[IT_LAST] = {
  VAL2STR(IT_NOP),
  VAL2STR(IT_PUSH_VALUE),
  VAL2STR(IT_ADD),
  VAL2STR(IT_SUBTRACT),
  VAL2STR(IT_MULTIPLY),
  VAL2STR(IT_DIVIDE),
  VAL2STR(IT_POWER),
  VAL2STR(IT_ASSIGN),
  VAL2STR(IT_RESOLVE_VAR),
  VAL2STR(IT_RESOLVE_REF),
	VAL2STR(IT_CALL),
};

CValue2String CInstruction::s_IT2Str(s_arrIT2Str, ARRSIZE(s_arrIT2Str));

void CInstruction::ReleaseData()
{
  if (HasValue())
    GetValue().~CValue();
  m_eType = IT_NOP;
}

EInterpretError CInstruction::Execute(CExecution *pExecution)
{
  switch (m_eType) {
    case IT_PUSH_VALUE:
      return ExecPushValue(pExecution);
    case IT_ADD:
      return ExecAdd(pExecution);
    case IT_SUBTRACT:
      return ExecSubtract(pExecution);
    case IT_MULTIPLY:
      return ExecMultiply(pExecution);
    case IT_DIVIDE:
      return ExecDivide(pExecution);
    case IT_POWER:
      return ExecPower(pExecution);
    case IT_ASSIGN:
      return ExecAssign(pExecution);
    case IT_RESOLVE_VAR:
      return ExecResolveValue(pExecution);
    case IT_RESOLVE_REF:
      return ExecResolveRef(pExecution);
		case IT_CALL:
			return ExecCall(pExecution);
    default:
      ASSERT(!"Invalid instruction");
      return IERR_INVALID_INSTRUCTION;
  }
}

int CInstruction::GetSize()
{
  int iSize = sizeof(m_eType);
  if (HasValue())
    iSize += sizeof(CValue);
  return iSize;
}

EInterpretError CInstruction::ExecPushValue(CExecution *pExecution)
{
  pExecution->m_kStack.Append(GetValue());
  return IERR_OK;
}

EInterpretError CInstruction::ExecAdd(CExecution *pExecution)
{
  if (pExecution->m_kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;
  CValue &kVal1 = pExecution->m_kStack.Last();
  CValue &kVal2 = pExecution->m_kStack.PreLast();
  if (kVal1.m_btType != kVal2.m_btType)
    return IERR_OPERAND_TYPE;
  if (kVal1.m_btType == CValue::VT_FLOAT) {
    kVal2.Set(kVal1.m_fValue + kVal2.m_fValue);
    pExecution->m_kStack.m_iCount--;
    return IERR_OK;
  } else
    if (kVal1.m_btType == CValue::VT_STRING) {
      CStrAny s = CStrAny(kVal1.m_pStrValue) + CStrAny(kVal2.m_pStrValue);
      s.AssureInRepository();
      kVal2.Set(s.GetHeader());
      pExecution->m_kStack.m_iCount--;
      return IERR_OK;
    }
  return IERR_OPERAND_TYPE;
}

EInterpretError CInstruction::ExecSubtract(CExecution *pExecution)
{
  if (pExecution->m_kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;
  CValue &kVal1 = pExecution->m_kStack.Last();
  CValue &kVal2 = pExecution->m_kStack.PreLast();
  if (kVal1.m_btType != kVal2.m_btType)
    return IERR_OPERAND_TYPE;
  if (kVal1.m_btType == CValue::VT_FLOAT) {
    kVal2.Set(kVal1.m_fValue - kVal2.m_fValue);
    pExecution->m_kStack.m_iCount--;
    return IERR_OK;
  }
  return IERR_OPERAND_TYPE;
}

EInterpretError CInstruction::ExecMultiply(CExecution *pExecution)
{
  if (pExecution->m_kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;
  CValue &kVal1 = pExecution->m_kStack.Last();
  CValue &kVal2 = pExecution->m_kStack.PreLast();
  if (kVal1.m_btType != kVal2.m_btType)
    return IERR_OPERAND_TYPE;
  if (kVal1.m_btType == CValue::VT_FLOAT) {
    kVal2.Set(kVal1.m_fValue * kVal2.m_fValue);
    pExecution->m_kStack.m_iCount--;
    return IERR_OK;
  }
  return IERR_OPERAND_TYPE;
}

EInterpretError CInstruction::ExecDivide(CExecution *pExecution)
{
  if (pExecution->m_kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;
  CValue &kVal1 = pExecution->m_kStack.Last();
  CValue &kVal2 = pExecution->m_kStack.PreLast();
  if (kVal1.m_btType != kVal2.m_btType)
    return IERR_OPERAND_TYPE;
  if (kVal1.m_btType == CValue::VT_FLOAT) {
    kVal2.Set(kVal1.m_fValue / kVal2.m_fValue);
    pExecution->m_kStack.m_iCount--;
    return IERR_OK;
  }
  return IERR_OPERAND_TYPE;
}

EInterpretError CInstruction::ExecPower(CExecution *pExecution)
{
  if (pExecution->m_kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;
  CValue &kVal1 = pExecution->m_kStack.Last();
  CValue &kVal2 = pExecution->m_kStack.PreLast();
  if (kVal1.m_btType != kVal2.m_btType)
    return IERR_OPERAND_TYPE;
  if (kVal1.m_btType == CValue::VT_FLOAT) {
    kVal2.Set(pow(kVal1.m_fValue, kVal2.m_fValue));
    pExecution->m_kStack.m_iCount--;
    return IERR_OK;
  }
  return IERR_OPERAND_TYPE;
}

EInterpretError CInstruction::ExecAssign(CExecution *pExecution)
{
  if (pExecution->m_kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;
  CValue &kVal1 = pExecution->m_kStack.Last();
  CValue &kVal2 = pExecution->m_kStack.PreLast();
  if (kVal1.m_btType == CValue::VT_REF) {
    (*kVal1.m_pReference) = kVal2;
    pExecution->m_kStack.m_iCount--;
    return IERR_OK;
  }
  return IERR_OPERAND_TYPE;
}

EInterpretError CInstruction::ExecResolveValue(CExecution *pExecution)
{
  if (pExecution->m_kStack.m_iCount < 1)
    return IERR_NOT_ENOUGH_OPERANDS;
  CValue &kVal = pExecution->m_kStack.Last();
  if (kVal.m_btType != CValue::VT_STRING)
    return IERR_OPERAND_TYPE;
  CValue::THash::TIter it = pExecution->m_kEnvironment.m_Hash.Find(kVal.m_pStrValue);
  if (it) 
    kVal = (*it).m_Val;
  else
    kVal.SetNone();
  return IERR_OK;
}

EInterpretError CInstruction::ExecResolveRef(CExecution *pExecution)
{
  if (pExecution->m_kStack.m_iCount < 1)
    return IERR_NOT_ENOUGH_OPERANDS;
  CValue &kVal = pExecution->m_kStack.Last();
  if (kVal.m_btType != CValue::VT_STRING)
    return IERR_OPERAND_TYPE;
  CValue::THash::TIter it = pExecution->m_kEnvironment.m_Hash.Find(kVal.m_pStrValue);
  if (!it) {
    pExecution->m_kEnvironment.m_Hash.Add(CValue::THash::Elem(kVal.m_pStrValue, CValue()));
    it = pExecution->m_kEnvironment.m_Hash.Find(kVal.m_pStrValue);
  }
  if (it) 
    kVal.Set(&(*it).m_Val);
  else
    kVal.SetNone();
  return IERR_OK;
}

EInterpretError CInstruction::ExecCall(CExecution *pExecution)
{
  if (pExecution->m_kStack.m_iCount < 1)
    return IERR_NOT_ENOUGH_OPERANDS;
	CArray<CValue> arrParams;
	int i = pExecution->m_kStack.m_iCount - 1;
	while (i >= 0 && pExecution->m_kStack[i].m_btType != CValue::VT_NONE) {
		arrParams.Append(pExecution->m_kStack[i]);
		pExecution->m_kStack[i].ClearValue();
		--i;
	}
	if (i < 1)
		return IERR_NOT_ENOUGH_OPERANDS;
	CValue &kVal = pExecution->m_kStack[i - 1];
	if (kVal.m_btType != CValue::VT_FRAGMENT)
		return IERR_OPERAND_TYPE;

	CExecution kExecution;
	EInterpretError err = kExecution.Execute(kVal.m_pFragment, arrParams);
	if (err != IERR_OK)
		return err;
	for (i = 0; i < kExecution.m_kStack.m_iCount; ++i)
		pExecution->m_kStack.Append(kExecution.m_kStack[i]);

	return IERR_OK;
}

CStrAny CInstruction::ToStr()
{
  CStrAny sRes = CStrAny(ST_WHOLE, "Instruction: ") + s_IT2Str.GetStr(m_eType);
  if (HasValue())
    sRes += CStrAny(ST_WHOLE, " Value: ") + GetValue().GetStr();
  return sRes;
}

// CExecution -----------------------------------------------------------------

EInterpretError CExecution::Execute(CFragment *pCode, CArray<CValue> &arrParams)
{
  m_pCode = pCode;
  m_pNextInstruction = m_pCode->GetFirstInstruction();
  m_kStack.Clear();
  for (int i = 0; i < Util::Min(m_pCode->m_arrInputs.m_iCount, arrParams.m_iCount); ++i) 
    m_kEnvironment.m_Hash.Add(CValue::THash::Elem(m_pCode->m_arrInputs[i], arrParams[i]));
  return m_pCode->Execute(this);
}
