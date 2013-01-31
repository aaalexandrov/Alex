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
	  case CBNFGrammar::RID_Value:
			return CompileValue(pNode);
		case CBNFGrammar::RID_Operand:
			return CompileOperand(pNode);
		case CBNFGrammar::RID_Power:
			return CompilePower(pNode);
		case CBNFGrammar::RID_Mult:
			return CompileMult(pNode);
		case CBNFGrammar::RID_Sum:
			return CompileSum(pNode);
		case CBNFGrammar::RID_Expression:
			return CompileExpression(pNode);
		case CBNFGrammar::RID_LValue:
			return CompileLValue(pNode);
		case CBNFGrammar::RID_Assignment:
			return CompileAssignment(pNode);
		case CBNFGrammar::RID_Program:
			return CompileProgram(pNode);
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
		case CToken::TT_NUMBER: 
			bRes = Parse::Str2Float(fVal, pNode->m_pToken->m_sToken);
			ASSERT(bRes);
			kInstr.SetPushValue(CValue(fVal));
			break;
		case CToken::TT_STRING:
			sVal = CStrAny(pNode->m_pToken->m_sToken, ST_CONST);
			kInstr.SetPushValue(CValue(sVal.GetHeader()));
			break;
		case CToken::TT_VARIABLE:
			sVal = CStrAny(pNode->m_pToken->m_sToken, ST_CONST);
			kInstr.SetPushValue(CValue(sVal.GetHeader()));
			m_pCode->Append(kInstr);
			kInstr.SetResolveVar();
			break;
		default:
			ASSERT("Unknown value token type");
			break;
	}
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
	err = CompileNode(pNode->m_arrChildren[0]);
	if (err != IERR_OK)
		return err;
	if (pNode->m_arrChildren.m_iCount <= 1)
		return IERR_OK;
	err = CompileNode(pNode->m_arrChildren[1]);
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
	int iValStart, iCount, i;
	EInterpretError err;
	for (iValStart = 0; iValStart < pNode->m_arrChildren.m_iCount; ++iValStart)
		if (pNode->m_arrChildren[iValStart]->m_pToken->m_eType == CToken::TT_ASSIGN) {
			++iValStart;
			break;
		}
	ASSERT(iValStart < pNode->m_arrChildren.m_iCount);
	iCount = Util::Min(iValStart - 1, pNode->m_arrChildren.m_iCount - iValStart);
	ASSERT(iCount > 0);
	for (i = iValStart; i < iValStart + iCount; ++i) {
		err = CompileNode(pNode->m_arrChildren[i]);
		if (err != IERR_OK)
			return err;
	}
	for (i = iCount - 1; i >= 0; --i) {
		err = CompileNode(pNode->m_arrChildren[i]);
		if (err != IERR_OK)
			return err;
	}
	CInstruction kInstr;
	kInstr.SetAssign();
	for (i = 0; i < iCount; ++i) 
		m_pCode->Append(kInstr);
  return IERR_OK;
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

EInterpretError CCompileChain::Compile(CStrAny sCode)
{
  EInterpretError err;

  err = m_kTokenizer.Tokenize(sCode);
  if (err != IERR_OK)
    return err;

	CAutoDeletePtr<CBNFGrammar::CNode> pParsed;
	if (!m_kGrammar.Parse(m_kTokenizer.m_lstTokens, pParsed.m_pPtr))
		return IERR_PARSING_FAILED;

  err = m_kCompiler.Compile(pParsed);
  if (err != IERR_OK)
    return err;

  return IERR_OK;
}
