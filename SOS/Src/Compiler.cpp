#include "stdafx.h"
#include "Compiler.h"
#include "Interpreter.h"

class CInstructionStream {
public:
	CCompiler &m_kCompiler;
	short m_nFirst, m_nAccum, m_nPrevious;

	CInstructionStream(CCompiler &kCompiler): m_kCompiler(kCompiler), m_nFirst(CInstruction::INVALID_OPERAND), m_nAccum(CInstruction::INVALID_OPERAND), m_nPrevious(CInstruction::INVALID_OPERAND) {}
	~CInstructionStream() { ReleaseLocals(); }

	void ReleaseLocals() 
	{
		ReleaseLocal(m_nFirst); 
		if (m_nAccum != m_nFirst) 
			ReleaseLocal(m_nAccum); 
		m_nFirst = m_nAccum = m_nPrevious = CInstruction::INVALID_OPERAND;
	}

	void ReleaseLocal(short nLocal) 
	{ 
		if (nLocal >= 0) 
			m_kCompiler.m_kLocals.ReleaseTemporary(nLocal); 
	} 

	short GetAccumulator() 
	{ 
		if (m_nAccum < 0 || m_kCompiler.m_kLocals.m_arrLocals[m_nAccum] > 1)
			m_nAccum = m_kCompiler.m_kLocals.AllocLocal();
		return m_nAccum;
	}

	EInterpretError AddOperation(CInstruction::EType eIT, CBNFGrammar::CNode *pNode, short &nDest, bool bFinal) 
	{
		CInstruction kInstr;
		EInterpretError err;
		if (m_nFirst == CInstruction::INVALID_OPERAND) {
			if (bFinal) {
				err = m_kCompiler.CompileNode(pNode, nDest);
				m_nPrevious = nDest;
				return err;
			} else {
				m_nFirst = -2;
				err = m_kCompiler.CompileNode(pNode, m_nFirst);
				m_nPrevious = m_nFirst;
				m_kCompiler.m_kLocals.LockTemporary(m_nFirst);
				m_nAccum = m_nFirst;
				return err;
			}
		} else {
			short nIndex = -2;
			if (pNode) {
				err = m_kCompiler.CompileNode(pNode, nIndex);
				if (err != IERR_OK)
					return err;
			} else
				nIndex = CInstruction::INVALID_OPERAND;
			if (bFinal) {
				if (nDest < 0) 
					nDest = GetAccumulator();
				kInstr.Set(eIT, nDest, m_nPrevious, nIndex);
				m_nPrevious = nDest;
			} else {
				kInstr.Set(eIT, GetAccumulator(), m_nPrevious, nIndex);
				m_nPrevious = GetAccumulator();
			}
			m_kCompiler.m_pCode->Append(kInstr);
			return IERR_OK;
		}
	}
};

class CLocalChecker {
public:
	CCompiler *m_pCompiler;
	int m_iStartTemps;

	CLocalChecker(CCompiler *pCompiler): m_pCompiler(pCompiler), m_iStartTemps(pCompiler->m_kLocals.m_iLocals - pCompiler->m_kLocals.m_iVars) {}
	~CLocalChecker() { CheckLocalCount(); }

	void CheckLocalCount() { ASSERT(m_pCompiler->m_kLocals.m_iLocals - m_pCompiler->m_kLocals.m_iVars == m_iStartTemps); }
	void CancelTracking() { m_iStartTemps = m_pCompiler->m_kLocals.m_iLocals - m_pCompiler->m_kLocals.m_iVars; }
};

// CCompiler::CLocalTracker ---------------------------------------------------

void CCompiler::CLocalTracker::Clear()
{
	m_arrLocals.Clear();
	m_hashLocals.Clear();
	m_nCurContext = 0;
	m_iLocals = 0;
	m_iVars = 0;
}

short CCompiler::CLocalTracker::GetFreeLocal(short nCount)
{
	ASSERT(nCount);
	int i = 0, iIndex = 0;
	bool bNotFree;
	do {
		bNotFree = false;
		for (i = 0; i < Util::Min<int>(nCount, m_arrLocals.m_iCount - iIndex); ++i)
			if (m_arrLocals[iIndex + i]) {
				bNotFree = true;
				iIndex += i + 1;
				break;
			}
	} while (bNotFree);
	if (iIndex + nCount > m_arrLocals.m_iCount) {
		int iOldCount = m_arrLocals.m_iCount;
		m_arrLocals.SetCount(iIndex + nCount);
		for (int j = iOldCount; j < m_arrLocals.m_iCount; ++j)
			m_arrLocals[j] = 0;
	}
	ASSERT(iIndex + nCount - 1 == (short) (iIndex + nCount - 1));
	return (short) iIndex;
}

short CCompiler::CLocalTracker::AllocLocal(short nCount)
{
	short nFree = GetFreeLocal(nCount);
	for (short i = 0; i < nCount; ++i) 
		if (!m_arrLocals[nFree + i]++)
	    ++m_iLocals;
	return nFree;
}

void CCompiler::CLocalTracker::ReleaseLocal(short nBase, short nCount)
{
	ASSERT(nCount);
	ASSERT(m_iLocals >= nCount);
	for (short i = 0; i < nCount; ++i) {
		ASSERT(m_arrLocals[nBase + i] > 0);
		if (!--m_arrLocals[nBase + i])
			--m_iLocals;
	}
}

void CCompiler::CLocalTracker::LockTemporary(short nIndex)
{
	if (nIndex < 0)
		return;
	ASSERT(m_arrLocals[nIndex] >= 0);
	if (!m_arrLocals[nIndex]++)
	  ++m_iLocals;
}

void CCompiler::CLocalTracker::ReleaseTemporary(short nIndex)
{
	if (nIndex < 0)
		return;
	ASSERT(m_arrLocals[nIndex] > 0);
	ASSERT(m_iLocals >= 1);
	if (!--m_arrLocals[nIndex])
	  --m_iLocals;
}

bool CCompiler::CLocalTracker::RegisterVar(CStrAny sVar, short nIndex)
{
	ASSERT(m_arrLocals[nIndex] > 0);
	THash::TIter it = FindVar(sVar);
	if (it && (*it).m_Val.m_nContext == m_nCurContext) // local redefinition in the same context
		return false;
	ASSERT(m_arrLocals[nIndex] == 1);
	m_hashLocals.Add(THash::Elem(sVar, TLocalInfo(nIndex, m_nCurContext)));
	++m_iVars;
	return true;
}

CCompiler::CLocalTracker::THash::TIter CCompiler::CLocalTracker::FindVar(CStrAny sVar)
{
	THash::TIter it = m_hashLocals.Find(sVar);
	THash::TIter itRecent = it;
	while (it && (*it).m_Key == sVar) {
		ASSERT(m_arrLocals[(*it).m_Val.m_nIndex] > 0);
		if ((*it).m_Val.m_nContext > (*itRecent).m_Val.m_nContext)
			itRecent = it;
		++it;
	}
	return itRecent;
}

short CCompiler::CLocalTracker::AllocVar(CStrAny sVar)
{
	THash::TIter itRecent = FindVar(sVar);
	short nIndex;
	if (itRecent && (*itRecent).m_Val.m_nContext == m_nCurContext) { // local redefinition in the same context
		nIndex = CInstruction::INVALID_OPERAND;
	} else {
		nIndex = AllocLocal();
		m_hashLocals.Add(THash::Elem(sVar, TLocalInfo(nIndex, m_nCurContext)));
		ASSERT(m_arrLocals[nIndex] == 1);
		++m_iVars;
	}
	return nIndex;
}

short CCompiler::CLocalTracker::GetVar(CStrAny sVar)
{
	THash::TIter itRecent = FindVar(sVar);
	short nIndex;
	if (itRecent) {
		nIndex = (*itRecent).m_Val.m_nIndex;
		ASSERT(m_arrLocals[nIndex] > 0);
	} else 
		nIndex = CInstruction::INVALID_OPERAND;
	return nIndex;
}

void CCompiler::CLocalTracker::StartContext()
{
	++m_nCurContext;
}

void CCompiler::CLocalTracker::EndContext()
{
	ASSERT(m_nCurContext);
	THash::TIter it(&m_hashLocals);
	while (it) {
		ASSERT((*it).m_Val.m_nContext <= m_nCurContext);
		if ((*it).m_Val.m_nContext >= m_nCurContext) {
			THash::TIter it1(it);
			++it;
			ReleaseLocal((*it1).m_Val.m_nIndex);
			m_hashLocals.Remove(it1);
			--m_iVars;
		} else
			++it;
	}
	--m_nCurContext;
}

// CCompiler ------------------------------------------------------------------

CCompiler::CCompiler()
{
}

CCompiler::~CCompiler()
{
  Clear();
}

void CCompiler::Clear()
{
  m_pInterpreter = 0;
  m_pCode = 0;
	m_hashConst.Clear();
	m_kLocals.Clear();
}

short CCompiler::GetConstantIndex(CValue const &kValue)
{
	TConstantHash::TIter it = m_hashConst.Find(kValue);
	if (it) {
		ASSERT(m_pCode->m_arrConst[-(*it).m_Val - 1] == kValue);
		return (*it).m_Val;
	}
	short nIndex = -m_pCode->m_arrConst.m_iCount - 1;
	m_pCode->m_arrConst.Append(kValue);
	m_hashConst.Add(TConstantHash::Elem(kValue, nIndex));
	return nIndex;
}

void CCompiler::UpdateLocalNumber()
{
  m_pCode->m_nLocalCount = m_kLocals.m_arrLocals.m_iCount;
}

EInterpretError CCompiler::Compile(CInterpreter *pInterpreter, CBNFGrammar::CNode *pNode)
{
  Clear();

  m_pInterpreter = pInterpreter;
  m_pCode = new CFragment(&m_pInterpreter->m_kValueRegistry);
	short nDest = CInstruction::INVALID_OPERAND;
  EInterpretError res = CompileNode(pNode, nDest);
	UpdateLocalNumber();

  if (res != IERR_OK)
    m_pCode = 0;

  return res;
}

EInterpretError CCompiler::CompileNode(CBNFGrammar::CNode *pNode, short &nDest)
{
	EInterpretError err;
	CLocalChecker kChecker(this);
	switch (pNode->m_pRule->m_iID) {
		case CBNFGrammar::RID_Program:
			err = CompileProgram(pNode, nDest);
			break;
	  case CBNFGrammar::RID_Constant:
			err = CompileConstant(pNode, nDest);
			break;
	  case CBNFGrammar::RID_Variable:
			err = CompileVariable(pNode, nDest);
			break;
		case CBNFGrammar::RID_FunctionDef:
			err = CompileFunctionDef(pNode, nDest);
			break;
		case CBNFGrammar::RID_FunctionCall:
			err = CompileFunctionCall(pNode, nDest);
			break;
		case CBNFGrammar::RID_Operand:
			err = CompileOperand(pNode, nDest);
			break;
		case CBNFGrammar::RID_DotIndex:
			err = CompileDotIndex(pNode, nDest);
			break;
		case CBNFGrammar::RID_Table:
			err = CompileTable(pNode, nDest);
			break;
		case CBNFGrammar::RID_Return:
			err = CompileReturn(pNode, nDest);
			break;
		case CBNFGrammar::RID_Power:
			err = CompilePower(pNode, nDest);
			break;
		case CBNFGrammar::RID_Mult:
			err = CompileMult(pNode, nDest);
			break;
		case CBNFGrammar::RID_Sum:
			err = CompileSum(pNode, nDest);
			break;
		case CBNFGrammar::RID_Concat:
			err = CompileConcat(pNode, nDest);
			break;
		case CBNFGrammar::RID_Comparison:
			err = CompileComparison(pNode, nDest);
			break;
		case CBNFGrammar::RID_Not:
			err = CompileNot(pNode, nDest);
			break;
		case CBNFGrammar::RID_And:
			err = CompileAnd(pNode, nDest);
			break;
		case CBNFGrammar::RID_Or:
			err = CompileOr(pNode, nDest);
			break;
//		case CBNFGrammar::RID_LValue:
//			return CompileLValue(pNode, nDest);
		case CBNFGrammar::RID_Locals: 
			err = CompileLocals(pNode, nDest);
			break;
		case CBNFGrammar::RID_Assignment:
			err = CompileAssignment(pNode, nDest);
			break;
		case CBNFGrammar::RID_If:
			err = CompileIf(pNode, nDest);
			break;
		case CBNFGrammar::RID_While:
			err = CompileWhile(pNode, nDest);
			break;
		default:
			ASSERT(0);
			err = IERR_COMPILE_FAILED;
			break;
	}
	if (err != IERR_OK) // Don't track local numbers in case of an error
		kChecker.CancelTracking();
	return err;
}

EInterpretError CCompiler::CompileConstResult(CValue const &kValue, short &nDest)
{
	short nIndex = GetConstantIndex(kValue);
	if (nDest < 0)
		nDest = nIndex;
	else {
  	CInstruction kInstr;
		kInstr.SetMoveValue(nDest, nIndex);
	  m_pCode->Append(kInstr);
	}
  return IERR_OK;
}


EInterpretError CCompiler::CompileConstant(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Constant);
	ASSERT(!pNode->m_arrChildren.m_iCount);
	float fVal;
	CStrAny sVal;
	bool bRes;
	CValue kVal;
	switch (pNode->m_pToken->m_eType) {
	  case CToken::TT_NIL:
			kVal.SetNone();
			break;
	  case CToken::TT_TRUE:
			kVal.Set(true);
			break;
		case CToken::TT_FALSE:
			kVal.Set(false);
			break;
		case CToken::TT_NUMBER: 
			bRes = Parse::Str2Float(fVal, pNode->m_pToken->m_sToken);
			ASSERT(bRes);
			kVal.Set(fVal);
			break;
		case CToken::TT_STRING:
			sVal = CStrAny(pNode->m_pToken->m_sToken, ST_CONST);
			kVal.Set(sVal.GetHeader());
			break;
		default:
			ASSERT("Unknown value token type");
			break;
	}

	return CompileConstResult(kVal, nDest);
}

EInterpretError CCompiler::CompileVariable(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Variable);
	ASSERT(!pNode->m_arrChildren.m_iCount && pNode->m_pToken->m_eType == CToken::TT_VARIABLE);
	CInstruction kInstr;
	CStrAny sVar(pNode->m_pToken->m_sToken, ST_CONST);
	short nIndex = m_kLocals.GetVar(sVar);
	if (nIndex != CInstruction::INVALID_OPERAND) { // a local
		ASSERT(nIndex >= 0);
		if (nDest < 0 || nDest == nIndex)
			nDest = nIndex;
		else {
			kInstr.SetMoveValue(nDest, nIndex);
			m_pCode->Append(kInstr);
		}
	} else { // a global
		short nNameConst = GetConstantIndex(CValue(sVar.GetHeader()));
		if (nDest < 0) 
			nDest = m_kLocals.GetFreeLocal();
		kInstr.SetGetGlobal(nDest, nNameConst);
		m_pCode->Append(kInstr);
	}
  return IERR_OK;
}

EInterpretError CCompiler::CompileFunctionDef(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_FunctionDef);
	ASSERT(pNode->m_arrChildren.m_iCount == 2);
	int i;
	EInterpretError err;
	CCompiler kCompiler;
	kCompiler.m_pCode = new CFragment(&m_pInterpreter->m_kValueRegistry);
	
	for (i = 0; i < pNode->m_arrChildren[0]->m_arrChildren.m_iCount; ++i) {
		ASSERT(pNode->m_arrChildren[0]->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_VARIABLE);
		CStrAny sVar(pNode->m_arrChildren[0]->m_arrChildren[i]->m_pToken->m_sToken, ST_CONST);
		short nIndex = kCompiler.m_kLocals.AllocVar(sVar);
		if (nIndex == CInstruction::INVALID_OPERAND)
			return IERR_DUPLICATE_VARIABLE;
		ASSERT(nIndex == i);
	}
	kCompiler.m_pCode->m_nParamCount = pNode->m_arrChildren[0]->m_arrChildren.m_iCount;

	for (i = 0; i < pNode->m_arrChildren[1]->m_arrChildren.m_iCount; ++i) {
		short nDestOperator = CInstruction::INVALID_OPERAND;
		err = kCompiler.CompileNode(pNode->m_arrChildren[1]->m_arrChildren[i], nDestOperator);
		if (err != IERR_OK)
			return err;
	}

	err = CompileConstResult(CValue(kCompiler.m_pCode), nDest);
	kCompiler.UpdateLocalNumber();

	return err;
}

EInterpretError CCompiler::CompileFunctionCall(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_FunctionCall);
	ASSERT(pNode->m_arrChildren.m_iCount >= 2);
	EInterpretError err;
	CInstruction kInstr;

	short nBase, nReturnCount, nTotalCount, i;
  if (nDest == CInstruction::INVALID_OPERAND)
		nReturnCount = 0;
	else
		if (nDest < 0)
			nReturnCount = -nDest - 1;
		else
			nReturnCount = 1;
	nTotalCount = nReturnCount;
	for (i = 1; i < pNode->m_arrChildren.m_iCount; ++i) {
		short nCount;
		if (pNode->m_arrChildren[i]->m_pRule->m_iID == CBNFGrammar::RID_ParamList)
			nCount = (short) pNode->m_arrChildren[i]->m_arrChildren.m_iCount + 1;
		else
			nCount = 1;
		nTotalCount = Util::Max(nTotalCount, nCount);
	}
	if (nTotalCount == 1 && nDest >= 0)
		nBase = nDest;
	else
	  nBase = m_kLocals.AllocLocal(nTotalCount);
	ASSERT(nBase >= 0);

	err = CompileNode(pNode->m_arrChildren[0], nBase);
	if (err != IERR_OK)
		return err;

	for (short iCall = 1; iCall < pNode->m_arrChildren.m_iCount; ++iCall) {

		if (pNode->m_arrChildren[iCall]->m_pRule->m_iID != CBNFGrammar::RID_ParamList) { // Indexing
			short nIndexDest = -2;
			err = CompileNode(pNode->m_arrChildren[iCall], nIndexDest);
			if (err != IERR_OK)
				return err;
			kInstr.SetGetTableValue(nBase, nBase, nIndexDest);
			m_pCode->Append(kInstr);
			continue;
		}

		short nParamCount = pNode->m_arrChildren[iCall]->m_arrChildren.m_iCount;
		for (i = 0; i < nParamCount; ++i) {
			short nParamDest = nBase + 1 + i;
			err = CompileNode(pNode->m_arrChildren[iCall]->m_arrChildren[i], nParamDest);
			if (err != IERR_OK)
				return err;
		}

		kInstr.SetCall(nBase, nParamCount + 1, iCall == pNode->m_arrChildren.m_iCount - 1 ? nReturnCount : 1);
		m_pCode->Append(kInstr);
	}

	if (nBase != nDest) {
	  m_kLocals.ReleaseLocal(nBase, nTotalCount);
		if (nDest >= 0) {
			kInstr.SetMoveValue(nDest, nBase);
			m_pCode->Append(kInstr);
		}
		else
			if (nDest != CInstruction::INVALID_OPERAND)
				nDest = nBase;
	}

	return IERR_OK;
}

EInterpretError CCompiler::CompileOperand(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Operand);
	ASSERT(pNode->m_arrChildren.m_iCount > 1);
	EInterpretError err;
	CInstructionStream kStream(*this);

	for (int i = 0; i < pNode->m_arrChildren.m_iCount; ++i) {
		err = kStream.AddOperation(CInstruction::IT_GET_TABLE_VALUE, pNode->m_arrChildren[i], nDest, i >= pNode->m_arrChildren.m_iCount - 1);
		if (err != IERR_OK)
			return err;
	}

	return IERR_OK;
}

EInterpretError CCompiler::CompileDotIndex(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_DotIndex);
	ASSERT(!pNode->m_arrChildren.m_iCount);
  ASSERT(pNode->m_pToken->m_eType == CToken::TT_VARIABLE);

  CStrAny sVal(pNode->m_pToken->m_sToken, ST_CONST);
	CValue kVal(sVal.GetHeader());

	return CompileConstResult(kVal, nDest);
}

EInterpretError CCompiler::CompileTable(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Table);
	EInterpretError err;
	CInstruction kInstr;
	int i, iNextKey = 1;

	short nResult = m_kLocals.AllocLocal();

	kInstr.SetCreateTable(nResult);
	m_pCode->Append(kInstr);

	for (i = 0; i < pNode->m_arrChildren.m_iCount; ++i) {
		ASSERT(pNode->m_arrChildren[i]->m_arrChildren.m_iCount == 1 || pNode->m_arrChildren[i]->m_arrChildren.m_iCount == 2);
		short nValue, nKey;
		if (pNode->m_arrChildren[i]->m_arrChildren.m_iCount == 1) {
			nValue = -2;
			err = CompileNode(pNode->m_arrChildren[i]->m_arrChildren[0], nValue);
			if (err != IERR_OK)
				return err;
			nKey = GetConstantIndex(CValue((float) (iNextKey++)));
		} else {
			nValue = -2;
			err = CompileNode(pNode->m_arrChildren[i]->m_arrChildren[1], nValue);
			if (err != IERR_OK)
				return err;
			CBNFGrammar::CNode *pKeyNode = pNode->m_arrChildren[i]->m_arrChildren[0];
			if (pKeyNode->m_pRule->m_iID == CBNFGrammar::RID_Table && pKeyNode->m_pToken->m_eType == CToken::TT_VARIABLE) {
				CStrAny sKey(pKeyNode->m_pToken->m_sToken, ST_CONST);
				nKey = GetConstantIndex(CValue(sKey.GetHeader()));
			} else {
				nKey = -2;
				err = CompileNode(pNode->m_arrChildren[i]->m_arrChildren[0], nKey);
				if (err != IERR_OK)
					return err;
			}
		}
		kInstr.SetSetTableValue(nKey, nResult, nValue);
		m_pCode->Append(kInstr);
	}

	m_kLocals.ReleaseLocal(nResult);
	if (nDest < 0)
		nDest = nResult;
	else {
		kInstr.SetMoveValue(nDest, nResult);
		m_pCode->Append(kInstr);
	}

	return IERR_OK;
}

EInterpretError CCompiler::CompileReturn(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Return);
	ASSERT(nDest < 0);
	CInstruction kInstr;
	EInterpretError err; 

	short nIndex, nBase, nCount = (short) pNode->m_arrChildren.m_iCount;
	switch (nCount) {
	  case 0:
			nBase = 0;
			break;
		case 1:
			nBase = -2;
			err = CompileNode(pNode->m_arrChildren[0], nBase);
			if (err != IERR_OK)
				return err;
			if (nBase < 0) {
				short nLocal = m_kLocals.GetFreeLocal();
				kInstr.SetMoveValue(nLocal, nBase);
				m_pCode->Append(kInstr);
				nBase = nLocal;
			}
			break;
		default:
			nBase = m_kLocals.AllocLocal(nCount);
			for (short i = 0; i < pNode->m_arrChildren.m_iCount; ++i) {
				nIndex = nBase + i;
				err = CompileNode(pNode->m_arrChildren[i], nIndex);
				if (err != IERR_OK)
					return err;
			}
  		m_kLocals.ReleaseLocal(nBase, nCount);
			break;
	}

	kInstr.SetReturn(nBase, nCount);
	m_pCode->Append(kInstr);

	return IERR_OK;
}

EInterpretError CCompiler::CompilePower(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Power);
	ASSERT(pNode->m_arrChildren.m_iCount == 2);
	EInterpretError err;

	short nX = -2, nY = -2;
	err = CompileNode(pNode->m_arrChildren[0], nX);
	if (err != IERR_OK)
		return err;
	m_kLocals.LockTemporary(nX);
	err = CompileNode(pNode->m_arrChildren[1], nY);
	if (err != IERR_OK)
		return err;
	m_kLocals.ReleaseTemporary(nX);

	if (nDest < 0) 
		nDest = m_kLocals.GetFreeLocal();
	CInstruction kInstr;
	kInstr.SetPower(nDest, nX, nY);
	m_pCode->Append(kInstr);

  return IERR_OK;
}

EInterpretError CCompiler::CompileMult(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Mult);
	ASSERT(pNode->m_arrChildren.m_iCount >= 3 && pNode->m_arrChildren.m_iCount % 2 == 1);
	EInterpretError err;
	CInstructionStream kStream(*this);

	err = kStream.AddOperation(CInstruction::IT_NOP, pNode->m_arrChildren[0], nDest, false);
	if (err != IERR_OK)
		return err;
	for (int i = 1; i < pNode->m_arrChildren.m_iCount; i += 2) {
		CInstruction::EType eIT;
		ASSERT(pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_MULTIPLY || pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_DIVIDE);
		if (pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_MULTIPLY)
			eIT = CInstruction::IT_MULTIPLY;
		else
			eIT = CInstruction::IT_DIVIDE;
		err = kStream.AddOperation(eIT, pNode->m_arrChildren[i + 1], nDest, i >= pNode->m_arrChildren.m_iCount - 2);
		if (err != IERR_OK)
			return err;
	}

  return IERR_OK;
}

EInterpretError CCompiler::CompileConcat(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Concat);
	ASSERT(pNode->m_arrChildren.m_iCount >= 2);
	EInterpretError err;
	CInstructionStream kStream(*this);

	err = kStream.AddOperation(CInstruction::IT_NOP, pNode->m_arrChildren[0], nDest, false);
	if (err != IERR_OK)
		return err;
	for (int i = 1; i < pNode->m_arrChildren.m_iCount; ++i) {
    err = kStream.AddOperation(CInstruction::IT_CONCAT, pNode->m_arrChildren[i], nDest, i >= pNode->m_arrChildren.m_iCount - 1);
		if (err != IERR_OK)
			return err;
	}

  return IERR_OK;
}

EInterpretError CCompiler::CompileSum(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Sum);
	ASSERT(pNode->m_arrChildren.m_iCount >= 2);
	EInterpretError err;
	CInstructionStream kStream(*this);

	int i = 1;
	if (pNode->m_arrChildren[0]->m_pToken->m_eType == CToken::TT_MINUS) {
		i = 2;
		err = kStream.AddOperation(CInstruction::IT_NOP, pNode->m_arrChildren[1], nDest, false);
		if (err != IERR_OK)
			return err;
		err = kStream.AddOperation(CInstruction::IT_NEGATE, 0, nDest, i >= pNode->m_arrChildren.m_iCount);
		if (err != IERR_OK)
			return err;
	} else {
		if (pNode->m_arrChildren[0]->m_pToken->m_eType == CToken::TT_PLUS) 
			i = 2;
		err = kStream.AddOperation(CInstruction::IT_NOP, pNode->m_arrChildren[i - 1], nDest, i >= pNode->m_arrChildren.m_iCount);
		if (err != IERR_OK)
			return err;
	}
	ASSERT((pNode->m_arrChildren.m_iCount - i) % 2 == 0);
	while (i < pNode->m_arrChildren.m_iCount) {
		CInstruction::EType eIT;
		ASSERT(pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_PLUS || pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_MINUS);
		if (pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_PLUS)
			eIT = CInstruction::IT_ADD;
		else
			eIT = CInstruction::IT_SUBTRACT;
		err = kStream.AddOperation(eIT, pNode->m_arrChildren[i + 1], nDest, i >= pNode->m_arrChildren.m_iCount - 2);
		if (err != IERR_OK)
			return err;
		i += 2;
	}
  return IERR_OK;
}

EInterpretError CCompiler::CompileComparison(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Comparison);
	ASSERT(pNode->m_arrChildren.m_iCount == 3);
	EInterpretError err;
	CInstruction kInstr;

	short nArg[2] = { -2, -2 };
	err = CompileNode(pNode->m_arrChildren[0], nArg[0]);
	if (err != IERR_OK)
		return err;
	m_kLocals.LockTemporary(nArg[0]);
	err = CompileNode(pNode->m_arrChildren[2], nArg[1]);
	if (err != IERR_OK)
		return err;
	m_kLocals.ReleaseTemporary(nArg[0]);
	CInstruction::EType eIT;
	BYTE i = 0;
	switch (pNode->m_arrChildren[1]->m_pToken->m_eType) {
	  case CToken::TT_EQUAL:
			eIT = CInstruction::IT_COMPARE_EQ;
			break;
		case CToken::TT_NOT_EQUAL:
			eIT = CInstruction::IT_COMPARE_NOT_EQ;
			break;
		case CToken::TT_GREAT_EQUAL:
			eIT = CInstruction::IT_COMPARE_LESS_EQ;
			i = 1;
			break;
		case CToken::TT_LESS:
			eIT = CInstruction::IT_COMPARE_LESS;
			break;
		case CToken::TT_LESS_EQUAL:
			eIT = CInstruction::IT_COMPARE_LESS_EQ;
			break;
		case CToken::TT_GREAT:
			eIT = CInstruction::IT_COMPARE_LESS;
			i = 1;
      break;
		default:
			return IERR_COMPILE_FAILED;
	}
	if (nDest < 0) 
		nDest = m_kLocals.GetFreeLocal();
	kInstr.Set(eIT, nDest, nArg[i], nArg[!i]);
	m_pCode->Append(kInstr);

  return IERR_OK;
}

EInterpretError CCompiler::CompileNot(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Not);
	ASSERT(pNode->m_arrChildren.m_iCount > 1);
	EInterpretError err;
	CInstruction kInstr;

	short nValue = -2;
	err = CompileNode(pNode->m_arrChildren.Last(), nValue);
	if (err != IERR_OK)
		return err;
	if (nDest < 0) 
		nDest = m_kLocals.GetFreeLocal();
	for (int i = 0; i < pNode->m_arrChildren.m_iCount - 1; ++i) {
		ASSERT(pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_NOT);
		kInstr.SetNot(nDest, i ? nDest : nValue);
		m_pCode->Append(kInstr);
	}

  return IERR_OK;
}

EInterpretError CCompiler::CompileAnd(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_And);
	ASSERT(pNode->m_arrChildren.m_iCount > 1);
	EInterpretError err;
	CInstruction kInstr;
	CArray<int> arrShortValIndices;
	int i;

	short nCurrent, nResult;
	if (nDest < 0)
		nResult = m_kLocals.AllocLocal();
	else
		nResult = nDest;
	for (i = 0; i < pNode->m_arrChildren.m_iCount - 1; ++i) {
		nCurrent = -2;
		err = CompileNode(pNode->m_arrChildren[i], nCurrent);
		if (err != IERR_OK)
			return err;
		arrShortValIndices.Append(m_pCode->m_arrCode.m_iCount);
		kInstr.SetMoveAndJumpIfFalse(nResult, nCurrent, -1);
		m_pCode->Append(kInstr);
	}
	err = CompileNode(pNode->m_arrChildren.Last(), nResult);
	if (err != IERR_OK)
		return err;
	if (m_pCode->m_arrCode.m_iCount >= (WORD) -1)
		return IERR_TOO_MANY_INSTRUCTIONS;
	short nAddress = (short) m_pCode->m_arrCode.m_iCount;
	for (i = 0; i < arrShortValIndices.m_iCount; ++i)
		m_pCode->m_arrCode[arrShortValIndices[i]].m_nSrc1 = nAddress;
	if (nDest < 0) {
		m_kLocals.ReleaseLocal(nResult);
		nDest = nResult;
	}

  return IERR_OK;
}

EInterpretError CCompiler::CompileOr(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Or);
	ASSERT(pNode->m_arrChildren.m_iCount > 1);
	EInterpretError err;
	CInstruction kInstr;
	CArray<int> arrShortValIndices;
	int i;

	short nCurrent, nResult;
	if (nDest < 0)
		nResult = m_kLocals.AllocLocal();
	else
		nResult = nDest;
	for (i = 0; i < pNode->m_arrChildren.m_iCount - 1; ++i) {
		nCurrent = -2;
		err = CompileNode(pNode->m_arrChildren[i], nCurrent);
		if (err != IERR_OK)
			return err;
		arrShortValIndices.Append(m_pCode->m_arrCode.m_iCount);
		kInstr.SetMoveAndJumpIfTrue(nResult, nCurrent, -1);
		m_pCode->Append(kInstr);
	}
	err = CompileNode(pNode->m_arrChildren.Last(), nResult);
	if (err != IERR_OK)
		return err;
	if (m_pCode->m_arrCode.m_iCount >= (WORD) -1)
		return IERR_TOO_MANY_INSTRUCTIONS;
	short nAddress = (short) m_pCode->m_arrCode.m_iCount;
	for (i = 0; i < arrShortValIndices.m_iCount; ++i)
		m_pCode->m_arrCode[arrShortValIndices[i]].m_nSrc1 = nAddress;
	if (nDest < 0) {
		m_kLocals.ReleaseLocal(nResult);
		nDest = nResult;
	}

  return IERR_OK;
}

EInterpretError CCompiler::CompileLValue(CBNFGrammar::CNode *pNode, short &nValue, short &nTable, bool &bGlobal)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_LValue);
	EInterpretError err;

	if (pNode->m_arrChildren.m_iCount) { // table with indices
		CInstructionStream kStream(*this);
		bGlobal = false;
		nTable = -2;
		for (int i = 0; i < pNode->m_arrChildren.m_iCount - 1; ++i) {
			err = kStream.AddOperation(CInstruction::IT_GET_TABLE_VALUE, pNode->m_arrChildren[i], nTable, i >= pNode->m_arrChildren.m_iCount - 2);
			if (err != IERR_OK)
				return err;
		}
		kStream.ReleaseLocals();
		m_kLocals.LockTemporary(nTable);
		nValue = -2;
		err = CompileNode(pNode->m_arrChildren.Last(), nValue);
		if (err != IERR_OK)
			return err;
		m_kLocals.ReleaseTemporary(nTable);
	} else { // variable
		CStrAny sVar(pNode->m_pToken->m_sToken, ST_CONST);
		short nResult = m_kLocals.GetVar(sVar);
		nTable = CInstruction::INVALID_OPERAND;
		if (nResult != CInstruction::INVALID_OPERAND) { // local
			nValue = nResult;
			bGlobal = false;
		} else { // global
			nValue = GetConstantIndex(CValue(sVar.GetHeader()));
			bGlobal = true;
		}
	}
	return IERR_OK;
}

EInterpretError CCompiler::CompileLocals(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Locals);
	ASSERT(nDest == CInstruction::INVALID_OPERAND);
	ASSERT(pNode->m_arrChildren.m_iCount == 1 || pNode->m_arrChildren.m_iCount == 2);
	int i, iExprCount;
	short nIndex, nReturns;
	EInterpretError err;
	CInstruction kInstr;

	CArray<short> arrValues;
	iExprCount = pNode->m_arrChildren.m_iCount > 1 ? pNode->m_arrChildren[1]->m_arrChildren.m_iCount : 0;
  nReturns = 1;
	for (i = 0; i < pNode->m_arrChildren[0]->m_arrChildren.m_iCount; i += nReturns) {
		if (i < iExprCount) {
			if (i == iExprCount - 1 && pNode->m_arrChildren[1]->m_arrChildren[i]->m_pRule->m_iID == CBNFGrammar::RID_FunctionCall) 
				nReturns = pNode->m_arrChildren[0]->m_arrChildren.m_iCount - i;
			nIndex = -nReturns - 1;
			err = CompileNode(pNode->m_arrChildren[1]->m_arrChildren[i], nIndex);
			if (err != IERR_OK)
				return err;
			for (short nRet = 0; nRet < nReturns; ++nRet) {
				m_kLocals.LockTemporary(nIndex + nRet);
				arrValues.Append(nIndex + nRet);
			}
		} else {
			nIndex = m_kLocals.AllocLocal();
			arrValues.Append(nIndex);
			short nNil = GetConstantIndex(CValue());
			kInstr.SetMoveValue(nIndex, nNil);
			m_pCode->Append(kInstr);
		}
	}

	for (i = 0; i < arrValues.m_iCount; ++i) {
		if (arrValues[i] < 0 || m_kLocals.m_arrLocals[arrValues[i]] > 1) { // A constant or not a temporary we own
			nIndex = m_kLocals.AllocLocal();
			kInstr.SetMoveValue(nIndex, arrValues[i]);
			m_pCode->Append(kInstr);
			m_kLocals.ReleaseTemporary(arrValues[i]);
		} else
			nIndex = arrValues[i];
		CStrAny sVar(pNode->m_arrChildren[0]->m_arrChildren[i]->m_pToken->m_sToken, ST_CONST);
		if (!m_kLocals.RegisterVar(sVar, nIndex))
			return IERR_DUPLICATE_VARIABLE;
	}

	return IERR_OK;
}

EInterpretError CCompiler::CompileAssignment(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Assignment);
	ASSERT(nDest == CInstruction::INVALID_OPERAND);
	ASSERT(pNode->m_arrChildren.m_iCount == 2);
	int i, iCount;
	EInterpretError err;
	CInstruction kInstr;

	CArray<short> arrValues;
	iCount = Util::Min(pNode->m_arrChildren[0]->m_arrChildren.m_iCount, pNode->m_arrChildren[1]->m_arrChildren.m_iCount);
	for (i = 0; i < iCount; ++i) {
		short nVal, nReturns;
		if (i == iCount - 1 && pNode->m_arrChildren[1]->m_arrChildren[i]->m_pRule->m_iID == CBNFGrammar::RID_FunctionCall) 
			nReturns = pNode->m_arrChildren[0]->m_arrChildren.m_iCount - (iCount - 1);
		else
			nReturns = 1;
		nVal = -nReturns - 1;
		err = CompileNode(pNode->m_arrChildren[1]->m_arrChildren[i], nVal);
		if (err != IERR_OK)
			return err;
		for (short nRet = 0; nRet < nReturns; ++nRet) {
			m_kLocals.LockTemporary(nVal + nRet);
			arrValues.Append(nVal + nRet);
		}
	}

	iCount = pNode->m_arrChildren[0]->m_arrChildren.m_iCount;
	for (i = 0; i < iCount; ++i) {
		short nValue, nTable;
		bool bGlobal;
		err = CompileLValue(pNode->m_arrChildren[0]->m_arrChildren[i], nValue, nTable, bGlobal);
		if (err != IERR_OK)
			return err;
		short nRes;
		if (i < arrValues.m_iCount) {
			nRes = arrValues[i];
			m_kLocals.ReleaseTemporary(nRes);
		} else
			nRes = GetConstantIndex(CValue());
		if (bGlobal) { // global
			kInstr.SetSetGlobal(nValue, nRes);
			m_pCode->Append(kInstr);
		} else 
			if (nTable != CInstruction::INVALID_OPERAND) { // table value
				kInstr.SetSetTableValue(nValue, nTable, nRes);
				m_pCode->Append(kInstr);
			} else { // local
				if (nValue != nRes) {
					kInstr.SetMoveValue(nValue, nRes);
					m_pCode->Append(kInstr);
				}
			}
	}

	return IERR_OK;
}

EInterpretError CCompiler::CompileIf(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_If);
	ASSERT(nDest == CInstruction::INVALID_OPERAND);
	ASSERT(pNode->m_arrChildren.m_iCount == 2 || pNode->m_arrChildren.m_iCount == 3);
	EInterpretError err;
	CInstruction kInstr;
	int iJumpValIndex, i;

	short nCondition = -2;
	err = CompileNode(pNode->m_arrChildren[0], nCondition);
	if (err != IERR_OK)
		return err;
	iJumpValIndex = m_pCode->m_arrCode.m_iCount;
	kInstr.SetMoveAndJumpIfFalse(CInstruction::INVALID_OPERAND, nCondition, -1);
	m_pCode->Append(kInstr);
	m_kLocals.StartContext();
	for (i = 0; i < pNode->m_arrChildren[1]->m_arrChildren.m_iCount; ++i) {
		short nDst = CInstruction::INVALID_OPERAND;
		err = CompileNode(pNode->m_arrChildren[1]->m_arrChildren[i], nDst);
		if (err != IERR_OK)
			return err;
	}
	m_kLocals.EndContext();
	if (pNode->m_arrChildren.m_iCount > 2) {
		short nFalse = GetConstantIndex(CValue(false));
		kInstr.SetMoveAndJumpIfFalse(CInstruction::INVALID_OPERAND, nFalse, -1);
		m_pCode->Append(kInstr);
		m_pCode->m_arrCode[iJumpValIndex].m_nSrc1 = (short) m_pCode->m_arrCode.m_iCount;
		iJumpValIndex = m_pCode->m_arrCode.m_iCount - 1;
		m_kLocals.StartContext();
		for (i = 0; i < pNode->m_arrChildren[2]->m_arrChildren.m_iCount; ++i) {
			short nDst = CInstruction::INVALID_OPERAND;
			err = CompileNode(pNode->m_arrChildren[2]->m_arrChildren[i], nDst);
			if (err != IERR_OK)
				return err;
		}
		m_kLocals.EndContext();
	}
	m_pCode->m_arrCode[iJumpValIndex].m_nSrc1 = (short) m_pCode->m_arrCode.m_iCount;

	return IERR_OK;
}

EInterpretError CCompiler::CompileWhile(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_While);
	ASSERT(pNode->m_arrChildren.m_iCount == 2);
	EInterpretError err;
	CInstruction kInstr;
	int i, iJumpValIndex, iStartIndex;

	iStartIndex = m_pCode->m_arrCode.m_iCount;
	short nCondition = -2;
	err = CompileNode(pNode->m_arrChildren[0], nCondition);
	if (err != IERR_OK)
		return err;
	iJumpValIndex = m_pCode->m_arrCode.m_iCount;
	kInstr.SetMoveAndJumpIfFalse(CInstruction::INVALID_OPERAND, nCondition, -1);
	m_pCode->Append(kInstr);
	m_kLocals.StartContext();
	for (i = 0; i < pNode->m_arrChildren[1]->m_arrChildren.m_iCount; ++i) {
		short nDst = CInstruction::INVALID_OPERAND;
		err = CompileNode(pNode->m_arrChildren[1]->m_arrChildren[i], nDst);
		if (err != IERR_OK)
			return err;
	}
	m_kLocals.EndContext();
	short nFalse = GetConstantIndex(CValue(false));
	kInstr.SetMoveAndJumpIfFalse(CInstruction::INVALID_OPERAND, nFalse, iStartIndex);
	m_pCode->Append(kInstr);
	m_pCode->m_arrCode[iJumpValIndex].m_nSrc1 = m_pCode->m_arrCode.m_iCount;

	return IERR_OK;
}

EInterpretError CCompiler::CompileProgram(CBNFGrammar::CNode *pNode, short &nDest)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Program);
	EInterpretError err;
	for (int i = 0; i < pNode->m_arrChildren.m_iCount; ++i) {
		short nDst = CInstruction::INVALID_OPERAND;
		err = CompileNode(pNode->m_arrChildren[i], nDst);
		if (err != IERR_OK)
			return err;
	}
  return IERR_OK;
}

// CCompileChain --------------------------------------------------------------

void CCompileChain::Clear()
{
	m_kTokenizer.Clear();
	m_kGrammar.Clear();
	m_kCompiler.Clear();
}

EInterpretError CCompileChain::Compile(CInterpreter *pInterpreter, CStrAny sCode)
{
  EInterpretError err;

  err = m_kTokenizer.Tokenize(sCode);
  if (err != IERR_OK)
    return err;

	if (!m_kGrammar.Parse(m_kTokenizer.m_lstTokens))
		return IERR_PARSING_FAILED;

  err = m_kCompiler.Compile(pInterpreter, m_kGrammar.m_pParsed);
  if (err != IERR_OK)
    return err;

  return IERR_OK;
}
