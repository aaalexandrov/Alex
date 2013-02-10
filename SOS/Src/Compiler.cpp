#include "stdafx.h"
#include "Compiler.h"

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
	m_pCode = 0;
}

EInterpretError CCompiler::Compile(CBNFGrammar::CNode *pNode)
{
  Clear();

  m_pCode = new CFragment();
  EInterpretError res = CompileNode(pNode);

  if (res != IERR_OK)
    m_pCode = 0;

  return res;
}

EInterpretError CCompiler::CompileNode(CBNFGrammar::CNode *pNode)
{
	switch (pNode->m_pRule->m_iID) {
		case CBNFGrammar::RID_Program:
			return CompileProgram(pNode);
	  case CBNFGrammar::RID_Value:
			return CompileValue(pNode);
	  case CBNFGrammar::RID_Variable:
			return CompileVariable(pNode);
		case CBNFGrammar::RID_FunctionDef:
			return CompileFunctionDef(pNode);
		case CBNFGrammar::RID_FunctionCall:
			return CompileFunctionCall(pNode);
		case CBNFGrammar::RID_Return:
			return CompileReturn(pNode);
		case CBNFGrammar::RID_Operand:
			return CompileOperand(pNode);
		case CBNFGrammar::RID_Power:
			return CompilePower(pNode);
		case CBNFGrammar::RID_Mult:
			return CompileMult(pNode);
		case CBNFGrammar::RID_Sum:
			return CompileSum(pNode);
		case CBNFGrammar::RID_Comparison:
			return CompileComparison(pNode);
		case CBNFGrammar::RID_Not:
			return CompileNot(pNode);
		case CBNFGrammar::RID_And:
			return CompileAnd(pNode);
		case CBNFGrammar::RID_Or:
			return CompileOr(pNode);
		case CBNFGrammar::RID_Expression:
			return CompileExpression(pNode);
		case CBNFGrammar::RID_LValue:
			return CompileLValue(pNode);
		case CBNFGrammar::RID_Assignment:
			return CompileAssignment(pNode);
		case CBNFGrammar::RID_Operator:
			return CompileOperator(pNode);
		case CBNFGrammar::RID_If:
			return CompileIf(pNode);
		case CBNFGrammar::RID_While:
			return CompileWhile(pNode);
		default:
			ASSERT(0);
			return IERR_COMPILE_FAILED;
	}
}

EInterpretError CCompiler::CompileValue(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Value);
	ASSERT(!pNode->m_arrChildren.m_iCount);
	CInstruction kInstr;
	float fVal;
	CStrAny sVal;
	bool bRes;
	switch (pNode->m_pToken->m_eType) {
	  case CToken::TT_TRUE:
			kInstr.SetPushValue(CValue(true));
			break;
		case CToken::TT_FALSE:
			kInstr.SetPushValue(CValue(false));
			break;
		case CToken::TT_NUMBER: 
			bRes = Parse::Str2Float(fVal, pNode->m_pToken->m_sToken);
			ASSERT(bRes);
			kInstr.SetPushValue(CValue(fVal));
			break;
		case CToken::TT_STRING:
			sVal = CStrAny(pNode->m_pToken->m_sToken, ST_CONST);
			kInstr.SetPushValue(CValue(sVal.GetHeader()));
			break;
		default:
			ASSERT("Unknown value token type");
			break;
	}
	m_pCode->Append(kInstr);
  return IERR_OK;
}

EInterpretError CCompiler::CompileVariable(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Variable);
	ASSERT(!pNode->m_arrChildren.m_iCount && pNode->m_pToken->m_eType == CToken::TT_VARIABLE);
	CInstruction kInstr;
	CStrAny sVal;
	sVal = CStrAny(pNode->m_pToken->m_sToken, ST_CONST);
	kInstr.SetPushValue(CValue(sVal.GetHeader()));
	m_pCode->Append(kInstr);
	kInstr.SetResolveVar();
	m_pCode->Append(kInstr);
  return IERR_OK;
}

EInterpretError CCompiler::CompileFunctionDef(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_FunctionDef);
	int i;
	EInterpretError err;
	CCompiler kCompiler;
	kCompiler.m_pCode = new CFragment();
	
	ASSERT(pNode->m_arrChildren.m_iCount == 2);
	for (i = 0; i < pNode->m_arrChildren[0]->m_arrChildren.m_iCount; ++i) {
		ASSERT(pNode->m_arrChildren[0]->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_VARIABLE);
		CStrAny sVar(pNode->m_arrChildren[0]->m_arrChildren[i]->m_pToken->m_sToken, ST_CONST);
		kCompiler.m_pCode->m_arrInputs.Append(sVar);
	}

	for (i = 0; i < pNode->m_arrChildren[1]->m_arrChildren.m_iCount; ++i) {
		err = kCompiler.CompileNode(pNode->m_arrChildren[1]->m_arrChildren[i]);
		if (err != IERR_OK)
			return err;
	}

	CInstruction kInstr;
	kInstr.SetPushValue(CValue(kCompiler.m_pCode.m_pPtr));
	m_pCode->Append(kInstr);

	return IERR_OK;
}

EInterpretError CCompiler::CompileFunctionCall(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_FunctionCall);
	ASSERT(pNode->m_arrChildren.m_iCount == 2);
	EInterpretError err;
	CInstruction kInstr;

  kInstr.SetPushValue(CValue(CValue::VT_MARKER));
	m_pCode->Append(kInstr);

	for (int i = pNode->m_arrChildren[1]->m_arrChildren.m_iCount - 1; i >= 0; --i) {
		err = CompileNode(pNode->m_arrChildren[1]->m_arrChildren[i]);
		if (err != IERR_OK)
			return err;
	}

	err = CompileNode(pNode->m_arrChildren[0]);
	if (err != IERR_OK)
		return err;

	kInstr.SetCall();
	m_pCode->Append(kInstr);

	return IERR_OK;
}

EInterpretError CCompiler::CompileReturn(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Return);
	CInstruction kInstr;
	kInstr.SetPopAll();
	m_pCode->Append(kInstr);
	for (int i = pNode->m_arrChildren.m_iCount - 1; i >= 0; --i) {
		EInterpretError err = CompileNode(pNode->m_arrChildren[i]);
		if (err != IERR_OK)
			return err;
	}
	kInstr.SetReturn();
	m_pCode->Append(kInstr);
	return IERR_OK;
}

EInterpretError CCompiler::CompileOperand(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Operand);
	ASSERT(!"This shouldn't happen, renaming only rules shouldn't generate nodes");
	return IERR_COMPILE_FAILED;
}

EInterpretError CCompiler::CompilePower(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Power);
	ASSERT(pNode->m_arrChildren.m_iCount == 2);
	EInterpretError err;
	err = CompileNode(pNode->m_arrChildren[1]);
	if (err != IERR_OK)
		return err;
	err = CompileNode(pNode->m_arrChildren[0]);
	if (err != IERR_OK)
		return err;
	CInstruction kInstr;
	kInstr.SetPower();
	m_pCode->Append(kInstr);
  return IERR_OK;
}

EInterpretError CCompiler::CompileMult(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Mult);
	ASSERT(pNode->m_arrChildren.m_iCount >= 3 && pNode->m_arrChildren.m_iCount % 2 == 1);
	EInterpretError err;
	err = CompileNode(pNode->m_arrChildren[0]);
	if (err != IERR_OK)
		return err;
	for (int i = 1; i < pNode->m_arrChildren.m_iCount; i += 2) {
		err = CompileNode(pNode->m_arrChildren[i + 1]);
		if (err != IERR_OK)
			return err;
		CInstruction kInstr;
		ASSERT(pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_MULTIPLY || pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_DIVIDE);
		if (pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_MULTIPLY)
			kInstr.SetMultiply();
		else
			kInstr.SetDivide();
		m_pCode->Append(kInstr);
	}
  return IERR_OK;
}

EInterpretError CCompiler::CompileSum(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Sum);
	ASSERT(pNode->m_arrChildren.m_iCount >= 2);
	EInterpretError err;
	CInstruction kInstr;
	int i = 1;
	if (pNode->m_arrChildren[0]->m_pToken->m_eType == CToken::TT_MINUS) {
		err = CompileNode(pNode->m_arrChildren[1]);
		if (err != IERR_OK)
			return err;
		kInstr.SetNegate();
		m_pCode->Append(kInstr);
		i = 2;
	} else {
		if (pNode->m_arrChildren[0]->m_pToken->m_eType == CToken::TT_PLUS) 
			i = 2;
		err = CompileNode(pNode->m_arrChildren[i - 1]);
		if (err != IERR_OK)
			return err;
	}
	ASSERT((pNode->m_arrChildren.m_iCount - i) % 2 == 0);
	while (i < pNode->m_arrChildren.m_iCount) {
		err = CompileNode(pNode->m_arrChildren[i + 1]);
		if (err != IERR_OK)
			return err;
		ASSERT(pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_PLUS || pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_MINUS);
		if (pNode->m_arrChildren[i]->m_pToken->m_eType == CToken::TT_PLUS)
			kInstr.SetAdd();
		else
			kInstr.SetSubtract();
		m_pCode->Append(kInstr);
		i += 2;
	}
  return IERR_OK;
}

EInterpretError CCompiler::CompileComparison(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Comparison);
	ASSERT(pNode->m_arrChildren.m_iCount == 3);
	EInterpretError err;
	CInstruction kInstr;

	switch (pNode->m_arrChildren[1]->m_pToken->m_eType) {
	  case CToken::TT_EQUAL:
		case CToken::TT_NOT_EQUAL:
			err = CompileNode(pNode->m_arrChildren[0]);
			if (err != IERR_OK)
				return err;
			err = CompileNode(pNode->m_arrChildren[2]);
			if (err != IERR_OK)
				return err;
			kInstr.SetCompareEq();
			m_pCode->Append(kInstr);
			break;
		case CToken::TT_GREAT_EQUAL:
		case CToken::TT_LESS:
			err = CompileNode(pNode->m_arrChildren[0]);
			if (err != IERR_OK)
				return err;
			err = CompileNode(pNode->m_arrChildren[2]);
			if (err != IERR_OK)
				return err;
			kInstr.SetCompareLess();
			m_pCode->Append(kInstr);
			break;
		case CToken::TT_LESS_EQUAL:
		case CToken::TT_GREAT:
			err = CompileNode(pNode->m_arrChildren[2]);
			if (err != IERR_OK)
				return err;
			err = CompileNode(pNode->m_arrChildren[0]);
			if (err != IERR_OK)
				return err;
			kInstr.SetCompareLess();
			m_pCode->Append(kInstr);
      break;
	}
	if (pNode->m_arrChildren[1]->m_pToken->m_eType == CToken::TT_NOT_EQUAL || 
		  pNode->m_arrChildren[1]->m_pToken->m_eType == CToken::TT_GREAT_EQUAL || 
			pNode->m_arrChildren[1]->m_pToken->m_eType == CToken::TT_LESS_EQUAL) {
		kInstr.SetNot();
		m_pCode->Append(kInstr);
	}

  return IERR_OK;
}

EInterpretError CCompiler::CompileNot(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Not);
	ASSERT(pNode->m_arrChildren.m_iCount == 2 && pNode->m_arrChildren[0]->m_pToken->m_eType == CToken::TT_NOT);
	EInterpretError err;
	CInstruction kInstr;

	err = CompileNode(pNode->m_arrChildren[1]);
	if (err != IERR_OK)
		return err;
	kInstr.SetNot();
	m_pCode->Append(kInstr);

  return IERR_OK;
}

EInterpretError CCompiler::CompileAnd(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_And);
	ASSERT(pNode->m_arrChildren.m_iCount > 1);
	EInterpretError err;
	CInstruction kInstr;

	err = CompileNode(pNode->m_arrChildren[0]);
	if (err != IERR_OK)
		return err;
	kInstr.SetAnd();
	for (int i = 1; i < pNode->m_arrChildren.m_iCount; ++i) {
		err = CompileNode(pNode->m_arrChildren[i]);
		if (err != IERR_OK)
			return err;
		m_pCode->Append(kInstr);
	}

  return IERR_OK;
}

EInterpretError CCompiler::CompileOr(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Or);
	ASSERT(pNode->m_arrChildren.m_iCount > 1);
	EInterpretError err;
	CInstruction kInstr;

	err = CompileNode(pNode->m_arrChildren[0]);
	if (err != IERR_OK)
		return err;
	kInstr.SetNot();
	m_pCode->Append(kInstr);
	for (int i = 1; i < pNode->m_arrChildren.m_iCount; ++i) {
		err = CompileNode(pNode->m_arrChildren[i]);
		if (err != IERR_OK)
			return err;
		kInstr.SetNot();
		m_pCode->Append(kInstr);
		kInstr.SetAnd();
		m_pCode->Append(kInstr);
	}
	kInstr.SetNot();
	m_pCode->Append(kInstr);

  return IERR_OK;
}

EInterpretError CCompiler::CompileExpression(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Expression);
	ASSERT(!"This shouldn't happen, renaming only rules shouldn't generate nodes");
  return IERR_COMPILE_FAILED;
}

EInterpretError CCompiler::CompileLValue(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_LValue);
	ASSERT(!pNode->m_arrChildren.m_iCount);
	CInstruction kInstr;
	CStrAny sVal(pNode->m_pToken->m_sToken, ST_CONST);
	kInstr.SetPushValue(CValue(sVal.GetHeader()));
	m_pCode->Append(kInstr);
	kInstr.SetResolveRef();
	m_pCode->Append(kInstr);
  return IERR_OK;
}

EInterpretError CCompiler::CompileAssignment(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Assignment);
	int i, iCount;
	EInterpretError err;
	CInstruction kInstr;
	ASSERT(pNode->m_arrChildren.m_iCount == 2);
	kInstr.SetPushValue(CValue(CValue::VT_MARKER));
	m_pCode->Append(kInstr);
	iCount = Util::Min(pNode->m_arrChildren[0]->m_arrChildren.m_iCount, pNode->m_arrChildren[1]->m_arrChildren.m_iCount);
	ASSERT(iCount > 0);
	for (i = iCount - 1; i >= 0; --i) {
		err = CompileNode(pNode->m_arrChildren[1]->m_arrChildren[i]);
		if (err != IERR_OK)
			return err;
	}
	kInstr.SetAssign();
	for (i = 0; i < pNode->m_arrChildren[0]->m_arrChildren.m_iCount; ++i) {
		err = CompileNode(pNode->m_arrChildren[0]->m_arrChildren[i]);
		if (err != IERR_OK)
			return err;
		m_pCode->Append(kInstr);
	}
	kInstr.SetPopToMarker();
	m_pCode->Append(kInstr);
  return IERR_OK;
}

EInterpretError CCompiler::CompileIf(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_If);
	ASSERT(pNode->m_arrChildren.m_iCount == 2 || pNode->m_arrChildren.m_iCount == 3);
	EInterpretError err;
	CInstruction kInstr;
	int iJumpValIndex, i;

	err = CompileNode(pNode->m_arrChildren[0]);
	if (err != IERR_OK)
		return err;
	iJumpValIndex = m_pCode->m_arrCode.m_iCount;
	kInstr.SetPushValue(CValue(0.0f)); // destination address instruction
	m_pCode->Append(kInstr);
	kInstr.SetJumpIfFalse();
	m_pCode->Append(kInstr);
	for (i = 0; i < pNode->m_arrChildren[1]->m_arrChildren.m_iCount; ++i) {
		err = CompileNode(pNode->m_arrChildren[1]->m_arrChildren[i]);
		if (err != IERR_OK)
			return err;
	}
	if (pNode->m_arrChildren.m_iCount > 2) {
		kInstr.SetPushValue(CValue(false)); // FALSE value
		m_pCode->Append(kInstr);
		int iNewJumpValIndex = m_pCode->m_arrCode.m_iCount;
		m_pCode->Append(kInstr); // destination address instruction
		kInstr.SetJumpIfFalse();
		m_pCode->Append(kInstr);
		m_pCode->m_arrCode[iJumpValIndex].GetValue().Set((float) m_pCode->m_arrCode.m_iCount);
		iJumpValIndex = iNewJumpValIndex;
		for (i = 0; i < pNode->m_arrChildren[2]->m_arrChildren.m_iCount; ++i) {
			err = CompileNode(pNode->m_arrChildren[2]->m_arrChildren[i]);
			if (err != IERR_OK)
				return err;
		}
	}
	m_pCode->m_arrCode[iJumpValIndex].GetValue().Set((float) m_pCode->m_arrCode.m_iCount);

	return IERR_OK;
}

EInterpretError CCompiler::CompileWhile(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_While);
	ASSERT(pNode->m_arrChildren.m_iCount == 2);
	EInterpretError err;
	CInstruction kInstr;
	int i, iJumpValIndex, iStartIndex;

	iStartIndex = m_pCode->m_arrCode.m_iCount;
	err = CompileNode(pNode->m_arrChildren[0]);
	if (err != IERR_OK)
		return err;
	iJumpValIndex = m_pCode->m_arrCode.m_iCount;
	kInstr.SetPushValue(CValue(0.0f)); // destination address instruction
	m_pCode->Append(kInstr);
	kInstr.SetJumpIfFalse();
	m_pCode->Append(kInstr);
	for (i = 0; i < pNode->m_arrChildren[1]->m_arrChildren.m_iCount; ++i) {
		err = CompileNode(pNode->m_arrChildren[1]->m_arrChildren[i]);
		if (err != IERR_OK)
			return err;
	}
	kInstr.SetPushValue(CValue(false)); // FALSE value
	m_pCode->Append(kInstr);
	kInstr.SetPushValue(CValue((float) iStartIndex));
	m_pCode->Append(kInstr);
	kInstr.SetJumpIfFalse();
	m_pCode->Append(kInstr);
	m_pCode->m_arrCode[iJumpValIndex].GetValue().Set((float) m_pCode->m_arrCode.m_iCount);

	return IERR_OK;
}

EInterpretError CCompiler::CompileOperator(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Operator);
	ASSERT(!"This shouldn't happen, renaming only rules shouldn't generate nodes");
	return IERR_COMPILE_FAILED;
}

EInterpretError CCompiler::CompileProgram(CBNFGrammar::CNode *pNode)
{
	ASSERT(pNode->m_pRule->m_iID == CBNFGrammar::RID_Program);
	EInterpretError err;
	for (int i = 0; i < pNode->m_arrChildren.m_iCount; ++i) {
		err = CompileNode(pNode->m_arrChildren[i]);
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

EInterpretError CCompileChain::Compile(CStrAny sCode)
{
  EInterpretError err;

  err = m_kTokenizer.Tokenize(sCode);
  if (err != IERR_OK)
    return err;

	if (!m_kGrammar.Parse(m_kTokenizer.m_lstTokens))
		return IERR_PARSING_FAILED;

  err = m_kCompiler.Compile(m_kGrammar.m_pParsed);
  if (err != IERR_OK)
    return err;

  return IERR_OK;
}
