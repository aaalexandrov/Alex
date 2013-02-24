#include "stdafx.h"
#include "Execution.h"

// CInstruction ---------------------------------------------------------------

CValue2String::TValueString CInstruction::s_arrIT2Str[IT_LAST] = {
	VAL2STR(IT_NOP),
  VAL2STR(IT_MOVE_VALUE),
	VAL2STR(IT_GET_GLOBAL),
	VAL2STR(IT_SET_GLOBAL),
	VAL2STR(IT_CREATE_TABLE),
	VAL2STR(IT_GET_TABLE_VALUE),
	VAL2STR(IT_SET_TABLE_VALUE),
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
	VAL2STR(IT_AND),
	VAL2STR(IT_MOVE_AND_JUMP_IF_FALSE),
	VAL2STR(IT_MOVE_AND_JUMP_IF_TRUE),
	VAL2STR(IT_CALL),
	VAL2STR(IT_RETURN),
};

CValue2String CInstruction::s_IT2Str(s_arrIT2Str, ARRSIZE(s_arrIT2Str));

CValue *CInstruction::GetOperand(CExecution *pExecution, short nOperand) const
{
	ASSERT(nOperand != INVALID_OPERAND);
	if (nOperand == INVALID_OPERAND) 
		return 0;
	CArray<CValue> *pSrc;
	if (nOperand < 0) {
		pSrc = &pExecution->m_pCode->m_arrConst;
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
		case IT_AND: 
			return ExecAnd(pExecution);
		case IT_MOVE_AND_JUMP_IF_FALSE: 
			return ExecMoveAndJumpIfFalse(pExecution);
		case IT_MOVE_AND_JUMP_IF_TRUE: 
			return ExecMoveAndJumpIfTrue(pExecution);
		case IT_CALL: 
			return ExecCall(pExecution);
		case IT_RETURN: 
			return ExecReturn(pExecution);
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
	CValue::THash::TIter it = pExecution->m_pGlobalEnvironment->m_Hash.Find(*pSrc);
	if (it)
		*pDst = (*it).m_Val;
	else {
		pDst->ReleaseValue();
		pDst->SetNone();
	}
	return IERR_OK;
}

EInterpretError CInstruction::ExecSetGlobal(CExecution *pExecution) const
{
	CValue *pDst, *pSrc;
	pDst = GetOperand(pExecution, m_nDest);
	pSrc = GetSrc0(pExecution);
	if (!pDst || !pSrc)
		return IERR_INVALID_OPERAND;
	CValue::THash::TIter it = pExecution->m_pGlobalEnvironment->m_Hash.Find(*pDst);
	if (it)
		(*it).m_Val = *pSrc;
	else 
		pExecution->m_pGlobalEnvironment->m_Hash.Add(CValue::THash::Elem(*pDst, *pSrc));
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
	CValue::THash::TIter it = pSrc0->m_pTableValue->m_Hash.Find(*pSrc1);
	if (it)
		*pDst = (*it).m_Val;
	else {
		pDst->ReleaseValue();
		pDst->SetNone();
	}
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
	CValue::THash::TIter it = pSrc0->m_pTableValue->m_Hash.Find(*pDst);
	if (it)
		(*it).m_Val = *pSrc1;
	else 
		pSrc0->m_pTableValue->m_Hash.Add(CValue::THash::Elem(*pDst, *pSrc1));
	return IERR_OK;
}

EInterpretError CInstruction::ExecCreateTable(CExecution *pExecution) const
{
	CValue *pDst;
	pDst = GetDest(pExecution);
	if (!pDst)
		return IERR_INVALID_OPERAND;
	pDst->ReleaseValue();
	pDst->Set(new CValueTable());
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
	if (pSrc0->m_btType == CValue::VT_STRING) {
		CStrAny sRes(pSrc0->m_pStrValue);
		sRes += CStrAny(pSrc1->m_pStrValue);
		sRes.AssureInRepository();
		pDst->ReleaseValue();
		pDst->Set(sRes.GetHeader());
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
	pDst->ReleaseValue();
	pDst->Set(*pSrc0 == *pSrc1);
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
	pDst->ReleaseValue();
	pDst->Set(*pSrc0 != *pSrc1);
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
		pDst->ReleaseValue();
		pDst->Set(pSrc0->GetStr() < pSrc1->GetStr());
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
		pDst->ReleaseValue();
		pDst->Set(pSrc0->GetStr() <= pSrc1->GetStr());
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
	if (pSrc->m_btType != CValue::VT_BOOL)
		return IERR_OPERAND_TYPE;
	pDst->ReleaseValue();
	pDst->Set(!pSrc->m_bValue);
	return IERR_OK;
}

EInterpretError CInstruction::ExecAnd(CExecution *pExecution) const
{
	CValue *pDst, *pSrc0, *pSrc1;
	pDst = GetDest(pExecution);
	pSrc0 = GetSrc0(pExecution);
	pSrc1 = GetSrc1(pExecution);
	if (!pDst || !pSrc0 || !pSrc1)
		return IERR_INVALID_OPERAND;
	if (pSrc0->m_btType != CValue::VT_BOOL || pSrc1->m_btType != CValue::VT_BOOL)
		return IERR_OPERAND_TYPE;
	pDst->ReleaseValue();
	pDst->Set(pSrc0->m_bValue && pSrc1->m_bValue);
	return IERR_OK;
}

EInterpretError CInstruction::ExecMoveAndJumpIfFalse(CExecution *pExecution) const
{
	CValue *pDst, *pSrc;
	pDst = pExecution->m_arrLocal.PtrAt(m_nDest);
	pSrc = GetSrc0(pExecution);
	if (!pSrc)
		return IERR_INVALID_OPERAND;
	if (pSrc->m_btType == CValue::VT_NONE || pSrc->m_btType == CValue::VT_BOOL && !pSrc->m_bValue) {
		if (pDst)
			*pDst = *pSrc;
	  pExecution->m_pNextInstruction = pExecution->m_pCode->m_arrCode.PtrAt(m_nSrc1);
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
	if (!(pSrc->m_btType == CValue::VT_NONE || pSrc->m_btType == CValue::VT_BOOL && !pSrc->m_bValue)) {
		if (pDst)
			*pDst = *pSrc;
	  pExecution->m_pNextInstruction = pExecution->m_pCode->m_arrCode.PtrAt(m_nSrc1);
	}
	return IERR_OK;
}

EInterpretError CInstruction::ExecCall(CExecution *pExecution) const
{
	if (m_nDest < 0 || m_nSrc0 < 1 || m_nSrc1 < 0 || m_nDest + Util::Max(m_nSrc0, m_nSrc1) > pExecution->m_arrLocal.m_iCount)
		return IERR_INVALID_OPERAND;
	if (pExecution->m_arrLocal[m_nDest].m_btType != CValue::VT_FRAGMENT)
		return IERR_OPERAND_TYPE;

	short i;
	CExecution kExecution;
	CArray<CValue> arrParams(m_nSrc0 - 1);
	for (i = 1; i < m_nSrc0; ++i)
		arrParams.Append(pExecution->m_arrLocal[m_nDest + i]);
	EInterpretError err = kExecution.Execute(pExecution->m_arrLocal[m_nDest].m_pFragment, arrParams, pExecution->m_pGlobalEnvironment);
	if (err != IERR_OK)
		return err;
	for (i = 0; i < Util::Min(m_nSrc1, kExecution.m_nReturnCount); ++i)
		pExecution->m_arrLocal[m_nDest + i] = kExecution.m_arrLocal[kExecution.m_nReturnBase + i];
	while (i < m_nSrc1) {
		pExecution->m_arrLocal[m_nDest + i].ReleaseValue();
		pExecution->m_arrLocal[m_nDest + i].SetNone();
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
	return pFragment->m_arrConst[-nOperand - 1].GetStr();
}

CStrAny CInstruction::ToStr(CFragment *pFragment) const
{
  CStrAny sRes = CStrAny(ST_WHOLE, "Instruction: ") + s_IT2Str.GetStr(m_eType);
	sRes += CStrAny(ST_WHOLE, " Params: ") + GetOperandStr(m_nDest) + CStrAny(ST_WHOLE, ", ") + GetOperandStr(m_nSrc0) + CStrAny(ST_WHOLE, ", ") + GetOperandStr(m_nSrc1);
	sRes += CStrAny(ST_WHOLE, " Constants: ") + GetOperandConstStr(pFragment, m_nDest) + CStrAny(ST_WHOLE, ", ") + GetOperandConstStr(pFragment, m_nSrc0) + CStrAny(ST_WHOLE, ", ") + GetOperandConstStr(pFragment, m_nSrc1);
  return sRes;
}

// CExecution -----------------------------------------------------------------

CExecution::CExecution()
{
	m_pGlobalEnvironment = 0;
}

CExecution::~CExecution()
{
}

EInterpretError CExecution::Execute(CFragment *pCode, CArray<CValue> &arrParams, CValueTable *pGlobalEnvironment)
{
	m_nReturnCount = 0;
	m_pCode = pCode;
  m_pNextInstruction = m_pCode->GetFirstInstruction();
	m_arrLocal.SetCount(pCode->m_nLocalCount);
	if (pGlobalEnvironment)
	  m_pGlobalEnvironment = pGlobalEnvironment;
	else
		m_pGlobalEnvironment = new CValueTable();
	ASSERT(pCode->m_nLocalCount >= pCode->m_nParamCount);
  for (int i = 0; i < Util::Min<int>(m_pCode->m_nParamCount, arrParams.m_iCount); ++i)
		m_arrLocal[i] = arrParams[i];
  return m_pCode->Execute(this);
}
