#include "stdafx.h"
#include "Execution.h"

// CInstruction ---------------------------------------------------------------

CValue2String::TValueString CInstruction::s_arrIT2Str[IT_LAST] = {
  VAL2STR(IT_NOP),
	VAL2STR(IT_NEGATE),
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
	VAL2STR(IT_RETURN),
	VAL2STR(IT_POP_ALL),
	VAL2STR(IT_POP_TO_MARKER),
	VAL2STR(IT_JUMP_IF_FALSE),
	VAL2STR(IT_COMPARE_EQ),
	VAL2STR(IT_COMPARE_LESS),
	VAL2STR(IT_NOT),
	VAL2STR(IT_AND),
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
		case IT_NEGATE:
			return ExecNegate(pExecution);
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
		case IT_RETURN:
			return ExecReturn(pExecution);
		case IT_POP_ALL:
			return ExecPopAll(pExecution);
		case IT_POP_TO_MARKER:
			return ExecPopToMarker(pExecution);
		case IT_JUMP_IF_FALSE:
			return ExecJumpIfFalse(pExecution);
		case IT_COMPARE_EQ:
			return ExecCompareEq(pExecution);
		case IT_COMPARE_LESS:
			return ExecCompareLess(pExecution);
		case IT_NOT:
			return ExecNot(pExecution);
		case IT_AND:
			return ExecAnd(pExecution);
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

EInterpretError CInstruction::ExecNegate(CExecution *pExecution)
{
	if (pExecution->m_kStack.m_iCount < 1)
		return IERR_NOT_ENOUGH_OPERANDS;
	CValue &kVal = pExecution->m_kStack.Last();
	if (kVal.m_btType != CValue::VT_FLOAT)
		return IERR_OPERAND_TYPE;
	kVal.Set(-kVal.m_fValue);
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
      CStrAny s = CStrAny(kVal2.m_pStrValue) + CStrAny(kVal1.m_pStrValue);
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
    kVal2.Set(kVal2.m_fValue - kVal1.m_fValue);
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
    kVal2.Set(kVal2.m_fValue / kVal1.m_fValue);
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
    kVal2.Set(powf(kVal1.m_fValue, kVal2.m_fValue));
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
		if (kVal2.m_btType != CValue::VT_MARKER) {
			*kVal1.m_pReference = kVal2;
			pExecution->m_kStack.SetCount(pExecution->m_kStack.m_iCount - 2);
		} else {
			kVal1.m_pReference->SetNone();
			pExecution->m_kStack.SetCount(pExecution->m_kStack.m_iCount - 1);
		}
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
  CValue::THash::TIter it = pExecution->m_pEnvironment->m_Hash.Find(kVal.m_pStrValue);
	if (!it && pExecution->m_pGlobalEnvironment)
		it = pExecution->m_pGlobalEnvironment->m_Hash.Find(kVal.m_pStrValue);
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
  CValue::THash::TIter it = pExecution->m_pEnvironment->m_Hash.Find(kVal.m_pStrValue);
	if (!it && pExecution->m_pGlobalEnvironment)
		it = pExecution->m_pGlobalEnvironment->m_Hash.Find(kVal.m_pStrValue);
  if (!it) {
    pExecution->m_pEnvironment->m_Hash.Add(CValue::THash::Elem(kVal.m_pStrValue, CValue()));
    it = pExecution->m_pEnvironment->m_Hash.Find(kVal.m_pStrValue);
  }
  if (it)
    kVal.Set(&(*it).m_Val);
  else
    kVal.SetNone();
  return IERR_OK;
}

EInterpretError CInstruction::ExecCall(CExecution *pExecution)
{
	if (pExecution->m_kStack.m_iCount < 2)
		return IERR_NOT_ENOUGH_OPERANDS;
	CValue &kFrag = pExecution->m_kStack.Last();
	if (kFrag.m_btType != CValue::VT_FRAGMENT)
		return IERR_OPERAND_TYPE;
	int i;
	CArray<CValue> arrInputs;
	CFragment *pFrag = kFrag.GetFragment();
	for (i = pExecution->m_kStack.m_iCount - 2; i >= 0 && pExecution->m_kStack[i].m_btType != CValue::VT_MARKER; --i) {
		if (arrInputs.m_iCount < pFrag->m_arrInputs.m_iCount)
			arrInputs.Append(pExecution->m_kStack[i]);
	}
	if (i < 0)
		return IERR_NOT_ENOUGH_OPERANDS;
	ASSERT(pExecution->m_kStack[i].m_btType == CValue::VT_MARKER);
	ASSERT(pFrag->GetRefCount() > 1);
	pExecution->m_kStack.SetCount(i);
	CExecution kExecution;
	EInterpretError err = kExecution.Execute(pFrag, arrInputs, pExecution->GetGlobalEnvironment());
	if (err != IERR_OK)
		return err;
	for (i = 0; i < kExecution.m_kStack.m_iCount; ++i)
		pExecution->m_kStack.Append(kExecution.m_kStack[i]);

	return IERR_OK;
}

EInterpretError CInstruction::ExecReturn(CExecution *pExecution)
{
	pExecution->m_pNextInstruction = 0;
	return IERR_OK;
}

EInterpretError CInstruction::ExecPopAll(CExecution *pExecution)
{
	pExecution->ClearStack();
	return IERR_OK;
}

EInterpretError CInstruction::ExecPopToMarker(CExecution *pExecution)
{
	int i;
	for (i = pExecution->m_kStack.m_iCount - 1; i > 0 && pExecution->m_kStack[i].m_btType != CValue::VT_MARKER; --i);
	pExecution->m_kStack.SetCount(i);
	return IERR_OK;
}

EInterpretError CInstruction::ExecJumpIfFalse(CExecution *pExecution)
{
	if (pExecution->m_kStack.m_iCount < 2)
		return IERR_NOT_ENOUGH_OPERANDS;
  CValue &kVal1 = pExecution->m_kStack.Last();
  CValue &kVal2 = pExecution->m_kStack.PreLast();
	if (kVal1.m_btType != CValue::VT_FLOAT || kVal2.m_btType != CValue::VT_BOOL)
		return IERR_OPERAND_TYPE;
	if (!kVal2.m_bValue) {
		int iJumpIndex = (int) kVal1.m_fValue;
		if (iJumpIndex >= 0 && iJumpIndex < pExecution->m_pCode->m_arrCode.m_iCount)
			pExecution->m_pNextInstruction = &pExecution->m_pCode->m_arrCode[iJumpIndex];
		else
			pExecution->m_pNextInstruction = 0;
	}
	pExecution->m_kStack.SetCount(pExecution->m_kStack.m_iCount - 2);
	return IERR_OK;
}

EInterpretError CInstruction::ExecCompareEq(CExecution *pExecution)
{
	if (pExecution->m_kStack.m_iCount < 2)
		return IERR_NOT_ENOUGH_OPERANDS;
  CValue &kVal1 = pExecution->m_kStack.Last();
  CValue &kVal2 = pExecution->m_kStack.PreLast();
	kVal2.Set(kVal1 == kVal2);
	pExecution->m_kStack.SetCount(pExecution->m_kStack.m_iCount - 1);
	return IERR_OK;
}

EInterpretError CInstruction::ExecCompareLess(CExecution *pExecution)
{
	if (pExecution->m_kStack.m_iCount < 2)
		return IERR_NOT_ENOUGH_OPERANDS;
  CValue &kVal1 = pExecution->m_kStack.Last();
  CValue &kVal2 = pExecution->m_kStack.PreLast();
	if (kVal1.m_btType != kVal2.m_btType)
		return IERR_OPERAND_TYPE;
	if (kVal1.m_btType == CValue::VT_FLOAT)
		kVal2.Set(kVal2.m_fValue < kVal1.m_fValue);
	else
		if (kVal1.m_btType == CValue::VT_STRING)
			kVal2.Set(kVal2.GetStr() < kVal1.GetStr());
		else
			return IERR_OPERAND_TYPE;
	pExecution->m_kStack.SetCount(pExecution->m_kStack.m_iCount - 1);
  return IERR_OK;
}

EInterpretError CInstruction::ExecNot(CExecution *pExecution)
{
	if (pExecution->m_kStack.m_iCount < 1)
		return IERR_NOT_ENOUGH_OPERANDS;
	CValue &kVal1 = pExecution->m_kStack.Last();
	if (kVal1.m_btType != CValue::VT_BOOL)
		return IERR_OPERAND_TYPE;
	kVal1.m_bValue = !kVal1.m_bValue;
	return IERR_OK;
}

EInterpretError CInstruction::ExecAnd(CExecution *pExecution)
{
	if (pExecution->m_kStack.m_iCount < 2)
		return IERR_NOT_ENOUGH_OPERANDS;
  CValue &kVal1 = pExecution->m_kStack.Last();
  CValue &kVal2 = pExecution->m_kStack.PreLast();
	if (kVal1.m_btType != CValue::VT_BOOL || kVal2.m_btType != CValue::VT_BOOL)
		return IERR_OPERAND_TYPE;
	kVal2.m_bValue &= kVal1.m_bValue;
	pExecution->m_kStack.SetCount(pExecution->m_kStack.m_iCount - 1);
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

CExecution::CExecution()
{
	m_pEnvironment = new CValueTable();
	m_pGlobalEnvironment = 0;
}

CExecution::~CExecution()
{
	delete m_pEnvironment;
}

void CExecution::ClearStack()
{
	m_kStack.SetCount(0);
}

CValueTable *CExecution::GetGlobalEnvironment()
{
	if (m_pGlobalEnvironment)
		return m_pGlobalEnvironment;
	return m_pEnvironment;
}

EInterpretError CExecution::Execute(CFragment *pCode, CArray<CValue> &arrParams, CValueTable *pGlobalEnvironment)
{
  m_pCode = pCode;
  m_pNextInstruction = m_pCode->GetFirstInstruction();
  m_kStack.Clear();
	m_pGlobalEnvironment = pGlobalEnvironment;
  for (int i = 0; i < Util::Min(m_pCode->m_arrInputs.m_iCount, arrParams.m_iCount); ++i)
    m_pEnvironment->m_Hash.Add(CValue::THash::Elem(m_pCode->m_arrInputs[i].GetHeader(), arrParams[i]));
  return m_pCode->Execute(this);
}
