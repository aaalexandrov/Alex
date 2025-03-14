#include "stdafx.h"
#include "Execution.h"
#include "Interpreter.h"

// CInstruction ---------------------------------------------------------------

CValue2String::TValueString CInstruction::s_arrIT2Str[IT_LAST] = {
	VAL2STR(IT_NOP),
  VAL2STR(IT_MOVE_VALUE),
	VAL2STR(IT_GET_GLOBAL),
	VAL2STR(IT_SET_GLOBAL),
	VAL2STR(IT_CREATE_TABLE),
	VAL2STR(IT_GET_TABLE_VALUE),
	VAL2STR(IT_SET_TABLE_VALUE),
  VAL2STR(IT_CONCAT),
  VAL2STR(IT_NEGATE),
	VAL2STR(IT_ADD),
  VAL2STR(IT_SUBTRACT),
  VAL2STR(IT_MULTIPLY),
  VAL2STR(IT_DIVIDE),
  VAL2STR(IT_POWER),
	VAL2STR(IT_COMPARE_EQ),
	VAL2STR(IT_COMPARE_NOT_EQ),
	VAL2STR(IT_COMPARE_LESS),
	VAL2STR(IT_COMPARE_LESS_EQ),
	VAL2STR(IT_NOT),
	VAL2STR(IT_MOVE_AND_JUMP_IF_FALSE),
	VAL2STR(IT_MOVE_AND_JUMP_IF_TRUE),
	VAL2STR(IT_CALL),
	VAL2STR(IT_RETURN),
	VAL2STR(IT_CAPTURE_VARIABLES),
};

CValue2String CInstruction::s_IT2Str(s_arrIT2Str, ARRSIZE(s_arrIT2Str));

CValue *CInstruction::GetOperand(CExecution *pExecution, short nOperand) const
{
	ASSERT(nOperand != INVALID_OPERAND);
	if (nOperand == INVALID_OPERAND)
		return 0;
	CArray<CValue> *pSrc;
	if (nOperand < 0) {
		pSrc = &pExecution->m_pClosure->m_pFragment->m_arrConst;
		nOperand = -nOperand - 1;
	} else
		pSrc = &pExecution->m_arrLocal;
	ASSERT(nOperand >= 0 && nOperand < pSrc->m_iCount);
	if (nOperand >= pSrc->m_iCount)
		return 0;
	return &pSrc->At(nOperand);
}

EInterpretError CInstruction::Execute(CExecution *pExecution) const
{
  switch (m_eType) {
		case IT_NOP:
			return ExecNop(pExecution);
    case IT_MOVE_VALUE:
			return ExecMoveValue(pExecution);
		case IT_GET_GLOBAL:
			return ExecGetGlobal(pExecution);
		case IT_SET_GLOBAL:
			return ExecSetGlobal(pExecution);
		case IT_CREATE_TABLE:
			return ExecCreateTable(pExecution);
		case IT_GET_TABLE_VALUE:
			return ExecGetTableValue(pExecution);
		case IT_SET_TABLE_VALUE:
			return ExecSetTableValue(pExecution);
    case IT_CONCAT:
			return ExecConcat(pExecution);
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
		case IT_COMPARE_EQ:
			return ExecCompareEq(pExecution);
		case IT_COMPARE_NOT_EQ:
			return ExecCompareNotEq(pExecution);
		case IT_COMPARE_LESS:
			return ExecCompareLess(pExecution);
		case IT_COMPARE_LESS_EQ:
			return ExecCompareLessEq(pExecution);
		case IT_NOT:
			return ExecNot(pExecution);
		case IT_MOVE_AND_JUMP_IF_FALSE:
			return ExecMoveAndJumpIfFalse(pExecution);
		case IT_MOVE_AND_JUMP_IF_TRUE:
			return ExecMoveAndJumpIfTrue(pExecution);
		case IT_CALL:
			return ExecCall(pExecution);
		case IT_RETURN:
			return ExecReturn(pExecution);
    case IT_CAPTURE_VARIABLES:
      return ExecCaptureVariables(pExecution);
  	default:
      ASSERT(!"Invalid instruction");
      return IERR_INVALID_INSTRUCTION;
  }
}

EInterpretError CInstruction::ExecNop(CExecution *pExecution) const
{
	return IERR_OK;
}

EInterpretError CInstruction::ExecMoveValue(CExecution *pExecution) const
{
	CValue *pDst, *pSrc;
	pDst = GetDest(pExecution);
	pSrc = GetSrc0(pExecution);
	if (!pDst || !pSrc)
		return IERR_INVALID_OPERAND;
	*pDst = *pSrc;
	return IERR_OK;
}

EInterpretError CInstruction::ExecGetGlobal(CExecution *pExecution) const
{
	CValue *pDst, *pSrc;
	pDst = GetDest(pExecution);
	pSrc = GetSrc0(pExecution);
	if (!pDst || !pSrc)
		return IERR_INVALID_OPERAND;
  GetTableValue(*pExecution->GetGlobalEnvironment(), *pSrc, *pDst);
	return IERR_OK;
}

EInterpretError CInstruction::ExecSetGlobal(CExecution *pExecution) const
{
	CValue *pDst, *pSrc;
	pDst = GetOperand(pExecution, m_nDest);
	pSrc = GetSrc0(pExecution);
	if (!pDst || !pSrc)
		return IERR_INVALID_OPERAND;
  SetTableValue(*pExecution->GetGlobalEnvironment(), *pDst, *pSrc);
	return IERR_OK;
}

EInterpretError CInstruction::ExecGetTableValue(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetDest(pExecution);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
	if (pSrc0->m_btType != CValue::VT_TABLE)
		return IERR_OPERAND_TYPE;
  GetTableValue(*pSrc0->m_pTableValue, *pSrc1, *pDst);
	return IERR_OK;
}

EInterpretError CInstruction::ExecSetTableValue(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetOperand(pExecution, m_nDest);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
	if (pSrc0->m_btType != CValue::VT_TABLE)
		return IERR_OPERAND_TYPE;
  SetTableValue(*pSrc0->m_pTableValue, *pDst, *pSrc1);
	return IERR_OK;
}

EInterpretError CInstruction::ExecCreateTable(CExecution *pExecution) const
{
	CValue *pDst;
	pDst = GetDest(pExecution);
	if (!pDst)
		return IERR_INVALID_OPERAND;
	pDst->ReleaseValue();
  pDst->Set(NEW(CValueTable, (&pExecution->m_pInterpreter->m_kValueRegistry)));
	return IERR_OK;
}

EInterpretError CInstruction::ExecConcat(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetDest(pExecution);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
	CStrAny sRes(pSrc0->GetStr(false));
	sRes += CStrAny(pSrc1->GetStr(false));
	sRes.AssureInRepository();
	pDst->ReleaseValue();
	pDst->Set(sRes.GetHeader());
	return IERR_OK;
}

EInterpretError CInstruction::ExecNegate(CExecution *pExecution) const
{
	CValue *pDst, *pSrc;
	pDst = GetDest(pExecution);
	pSrc = GetSrc0(pExecution);
	if (!pDst || !pSrc)
		return IERR_INVALID_OPERAND;
	if (pSrc->m_btType != CValue::VT_FLOAT)
		return IERR_OPERAND_TYPE;
	pDst->ReleaseValue();
	pDst->Set(-pSrc->m_fValue);
	return IERR_OK;
}

EInterpretError CInstruction::ExecAdd(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetDest(pExecution);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
	if (pSrc0->m_btType != pSrc1->m_btType)
		return IERR_OPERAND_TYPE;
	if (pSrc0->m_btType == CValue::VT_FLOAT) {
		pDst->ReleaseValue();
		pDst->Set(pSrc0->m_fValue + pSrc1->m_fValue);
		return IERR_OK;
	}
	return IERR_OPERAND_TYPE;
}

EInterpretError CInstruction::ExecSubtract(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetDest(pExecution);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
	if (pSrc0->m_btType != CValue::VT_FLOAT || pSrc1->m_btType != CValue::VT_FLOAT)
		return IERR_OPERAND_TYPE;
	pDst->ReleaseValue();
	pDst->Set(pSrc0->m_fValue - pSrc1->m_fValue);
	return IERR_OK;
}

EInterpretError CInstruction::ExecMultiply(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetDest(pExecution);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
	if (pSrc0->m_btType != CValue::VT_FLOAT || pSrc1->m_btType != CValue::VT_FLOAT)
		return IERR_OPERAND_TYPE;
	pDst->ReleaseValue();
	pDst->Set(pSrc0->m_fValue * pSrc1->m_fValue);
	return IERR_OK;
}

EInterpretError CInstruction::ExecDivide(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetDest(pExecution);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
	if (pSrc0->m_btType != CValue::VT_FLOAT || pSrc1->m_btType != CValue::VT_FLOAT)
		return IERR_OPERAND_TYPE;
	pDst->ReleaseValue();
	pDst->Set(pSrc0->m_fValue / pSrc1->m_fValue);
	return IERR_OK;
}

EInterpretError CInstruction::ExecPower(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetDest(pExecution);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
	if (pSrc0->m_btType != CValue::VT_FLOAT || pSrc1->m_btType != CValue::VT_FLOAT)
		return IERR_OPERAND_TYPE;
	pDst->ReleaseValue();
	pDst->Set(powf(pSrc0->m_fValue, pSrc1->m_fValue));
	return IERR_OK;
}

EInterpretError CInstruction::ExecCompareEq(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetDest(pExecution);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
  bool bRes = *pSrc0 == *pSrc1;
	pDst->ReleaseValue();
	pDst->Set(bRes);
	return IERR_OK;
}

EInterpretError CInstruction::ExecCompareNotEq(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetDest(pExecution);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
  bool bRes = *pSrc0 != *pSrc1;
	pDst->ReleaseValue();
	pDst->Set(bRes);
	return IERR_OK;
}

EInterpretError CInstruction::ExecCompareLess(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetDest(pExecution);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
	if (pSrc0->m_btType != pSrc1->m_btType)
		return IERR_OPERAND_TYPE;
	if (pSrc0->m_btType == CValue::VT_FLOAT) {
		pDst->ReleaseValue();
		pDst->Set(pSrc0->m_fValue < pSrc1->m_fValue);
		return IERR_OK;
	}
	if (pSrc0->m_btType == CValue::VT_STRING) {
    bool bRes = pSrc0->GetStr(false) < pSrc1->GetStr(false);
		pDst->ReleaseValue();
		pDst->Set(bRes);
		return IERR_OK;
	}
	return IERR_OPERAND_TYPE;
}

EInterpretError CInstruction::ExecCompareLessEq(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetDest(pExecution);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
	if (pSrc0->m_btType != pSrc1->m_btType)
		return IERR_OPERAND_TYPE;
	if (pSrc0->m_btType == CValue::VT_FLOAT) {
		pDst->ReleaseValue();
		pDst->Set(pSrc0->m_fValue <= pSrc1->m_fValue);
		return IERR_OK;
	}
	if (pSrc0->m_btType == CValue::VT_STRING) {
    bool bRes = pSrc0->GetStr(false) <= pSrc1->GetStr(false);
    pDst->ReleaseValue();
		pDst->Set(bRes);
		return IERR_OK;
	}
	return IERR_OPERAND_TYPE;
}

EInterpretError CInstruction::ExecNot(CExecution *pExecution) const
{
	CValue *pDst, *pSrc;
	pDst = GetDest(pExecution);
	pSrc = GetSrc0(pExecution);
	if (!pDst || !pSrc)
		return IERR_INVALID_OPERAND;
	pDst->ReleaseValue();
  pDst->Set(pSrc->m_btType == CValue::VT_NONE || (pSrc->m_btType == CValue::VT_BOOL && !pSrc->m_bValue));
	return IERR_OK;
}

EInterpretError CInstruction::ExecMoveAndJumpIfFalse(CExecution *pExecution) const
{
	CValue *pDst, *pSrc;
	pDst = pExecution->m_arrLocal.PtrAt(m_nDest);
	pSrc = GetSrc0(pExecution);
	if (!pSrc)
		return IERR_INVALID_OPERAND;
	if (pSrc->m_btType == CValue::VT_NONE || (pSrc->m_btType == CValue::VT_BOOL && !pSrc->m_bValue)) {
		if (pDst)
			*pDst = *pSrc;
	  pExecution->m_pNextInstruction = pExecution->m_pClosure->m_pFragment->m_arrCode.PtrAt(m_nSrc1);
	}
	return IERR_OK;
}

EInterpretError CInstruction::ExecMoveAndJumpIfTrue(CExecution *pExecution) const
{
	CValue *pDst, *pSrc;
	pDst = pExecution->m_arrLocal.PtrAt(m_nDest);
	pSrc = GetSrc0(pExecution);
	if (!pSrc)
		return IERR_INVALID_OPERAND;
	if (!(pSrc->m_btType == CValue::VT_NONE || (pSrc->m_btType == CValue::VT_BOOL && !pSrc->m_bValue))) {
		if (pDst)
			*pDst = *pSrc;
	  pExecution->m_pNextInstruction = pExecution->m_pClosure->m_pFragment->m_arrCode.PtrAt(m_nSrc1);
	}
	return IERR_OK;
}

EInterpretError CInstruction::ExecCall(CExecution *pExecution) const
{
	if (m_nDest < 0 || m_nSrc0 < 1 || m_nSrc1 < 0 || m_nDest + Util::Max(m_nSrc0, m_nSrc1) > pExecution->m_arrLocal.m_iCount)
		return IERR_INVALID_OPERAND;
  bool bNativeCall = pExecution->m_arrLocal[m_nDest].m_btType == CValue::VT_NATIVE_FUNC;
	if (pExecution->m_arrLocal[m_nDest].m_btType != CValue::VT_CLOSURE && !bNativeCall)
		return IERR_OPERAND_TYPE;

	short i;
  CExecution kExecution(pExecution->m_pInterpreter, pExecution);
	CArray<CValue> arrParams(m_nSrc0 - 1);
	for (i = 1; i < m_nSrc0; ++i)
		arrParams.Append(pExecution->m_arrLocal[m_nDest + i]);

  CArray<CValue> *pResults;
  short nReturnCount, nReturnBase;
  if (bNativeCall) {
	  EInterpretError err = kExecution.Execute(pExecution->m_arrLocal[m_nDest].m_pNativeFunc, arrParams);
	  if (err != IERR_OK)
		  return err;
    pResults = &arrParams;
    nReturnBase = 0;
    nReturnCount = (short) arrParams.m_iCount;
  } else {
	  EInterpretError err = kExecution.Execute(pExecution->m_arrLocal[m_nDest].m_pClosure, arrParams);
	  if (err != IERR_OK)
		  return err;
    pResults = &kExecution.m_arrLocal;
    nReturnBase = kExecution.m_nReturnBase;
    nReturnCount = kExecution.m_nReturnCount;
  }

  nReturnCount = Util::Min(m_nSrc1, nReturnCount);
	for (i = 0; i < nReturnCount; ++i)
		pExecution->m_arrLocal[m_nDest + i] = pResults->At(nReturnBase + i);
	while (i < m_nSrc1) {
		pExecution->m_arrLocal[m_nDest + i].ClearValue();
		++i;
	}

	return IERR_OK;
}

EInterpretError CInstruction::ExecReturn(CExecution *pExecution) const
{
	if (m_nDest < 0 || m_nSrc0 < 0 || m_nDest + m_nSrc0 > pExecution->m_arrLocal.m_iCount)
		return IERR_INVALID_OPERAND;
	pExecution->m_nReturnBase = m_nDest;
	pExecution->m_nReturnCount = m_nSrc0;
	pExecution->m_pNextInstruction = 0;
	return IERR_OK;
}

EInterpretError CInstruction::ExecCaptureVariables(CExecution *pExecution) const
{
  CValue *pDst, *pSrc0;
  pDst = GetDest(pExecution);
  pSrc0 = GetSrc0(pExecution);
 	if (!pDst || !pSrc0)
		return IERR_INVALID_OPERAND;
  if (pSrc0->m_btType != CValue::VT_CLOSURE)
    return IERR_OPERAND_TYPE;
  CClosure *pDstClosure = NEW(CClosure, (&pExecution->m_pInterpreter->m_kValueRegistry, pSrc0->m_pClosure->m_pFragment));
  pDstClosure->CaptureVariables(pExecution);
  pDst->Set(pDstClosure);
  return IERR_OK;
}

CStrAny CInstruction::GetOperandStr(short nOperand) const
{
	if (nOperand == INVALID_OPERAND)
		return CStrAny();
	if (nOperand >= 0)
		return CStrAny(ST_WHOLE, "L") + CStrAny(ST_STR, nOperand);
	return CStrAny(ST_WHOLE, "C") + CStrAny(ST_STR, -nOperand - 1);
}

CStrAny CInstruction::GetOperandConstStr(CFragment *pFragment, short nOperand) const
{
	if (nOperand == INVALID_OPERAND || nOperand >= 0)
		return CStrAny();
	return pFragment->m_arrConst[-nOperand - 1].GetStr(true);
}

CStrAny CInstruction::ToStr(CFragment *pFragment) const
{
  CStrAny sRes = CStrAny(ST_WHOLE, "Instruction: ") + s_IT2Str.GetStr(m_eType);
	sRes += CStrAny(ST_WHOLE, " Params: ") + GetOperandStr(m_nDest) + CStrAny(ST_WHOLE, ", ") + GetOperandStr(m_nSrc0) + CStrAny(ST_WHOLE, ", ") + GetOperandStr(m_nSrc1);
	sRes += CStrAny(ST_WHOLE, " Constants: ") + GetOperandConstStr(pFragment, m_nDest) + CStrAny(ST_WHOLE, ", ") + GetOperandConstStr(pFragment, m_nSrc0) + CStrAny(ST_WHOLE, ", ") + GetOperandConstStr(pFragment, m_nSrc1);
  return sRes;
}

// CExecution -----------------------------------------------------------------

CExecution::CExecution(CInterpreter *pInterpreter, CExecution *pCallerExecution) :
  m_pInterpreter(pInterpreter), m_pCallerExecution(pCallerExecution), m_pCalleeExecution(0)
{
  if (m_pCallerExecution) {
    ASSERT(!m_pCallerExecution->m_pCalleeExecution);
    m_pCallerExecution->m_pCalleeExecution = this;
  }
}

CExecution::~CExecution()
{
  if (m_pCallerExecution) {
    ASSERT(m_pCallerExecution->m_pCalleeExecution == this);
    m_pCallerExecution->m_pCalleeExecution = 0;
  }
}

CValueTable *CExecution::GetGlobalEnvironment()
{
  return m_pInterpreter->m_pGlobalEnvironment;
}

void CExecution::GetReturnValues(CArray<CValue> &arrReturns)
{
  arrReturns.SetCount(m_nReturnCount);
  for (int i = 0; i < m_nReturnCount; ++i)
    arrReturns[i] = m_arrLocal[m_nReturnBase + i];
}

EInterpretError CExecution::Execute(CClosure *pClosure, CArray<CValue> &arrParams)
{
	m_nReturnCount = 0;
	m_pClosure = pClosure;
  m_pNextInstruction = m_pClosure->m_pFragment->GetFirstInstruction();
	m_arrLocal.SetCount(m_pClosure->m_pFragment->m_nLocalCount);
	ASSERT(m_pClosure->m_pFragment->m_nLocalCount >= m_pClosure->m_pFragment->m_nParamCount);
  for (int i = 0; i < Util::Min<int>(m_pClosure->m_pFragment->m_nParamCount, arrParams.m_iCount); ++i)
		m_arrLocal[i] = arrParams[i];
  m_pClosure->SetCapturedVariables(this);
  return m_pClosure->m_pFragment->Execute(this);
}

EInterpretError CExecution::Execute(CValue::FnNative *pNativeFunc, CArray<CValue> &arrParams)
{
  ASSERT(pNativeFunc);
  return pNativeFunc(*this, arrParams);
}
