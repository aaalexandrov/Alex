#include "stdafx.h"
#include "BNFGrammar.h"

// CBNFGrammar ----------------------------------------------------------------

CValue2String::TValueString CBNFGrammar::s_arrRID2Str[] = {
	VAL2STR(RID_Program),
	VAL2STR(RID_Value),
	VAL2STR(RID_Variable),
	VAL2STR(RID_FunctionDef),
	VAL2STR(RID_FunctionCall),
	VAL2STR(RID_Return),
	VAL2STR(RID_Operand),
	VAL2STR(RID_Power),
	VAL2STR(RID_Mult),
	VAL2STR(RID_Sum),
	VAL2STR(RID_Expression),
	VAL2STR(RID_LValue),
	VAL2STR(RID_Assignment),
	VAL2STR(RID_If),
	VAL2STR(RID_Operator),
};

CValue2String CBNFGrammar::s_RID2Str(s_arrRID2Str, ARRSIZE(s_arrRID2Str));

CBNFGrammar::CBNFGrammar()
{
	m_pParsed = 0;
	InitRules();
}

void CBNFGrammar::InitRules()
{
	CRule *pProgram = NewNT()->SetID(RID_Program);
	CRule *pValue = NewNT()->SetID(RID_Value);
	CRule *pVariable = NewNT()->SetID(RID_Variable);
	CRule *pFunctionDef = NewNT()->SetID(RID_FunctionDef);
	CRule *pFunctionCall = NewNT()->SetID(RID_FunctionCall);
	CRule *pReturn = NewNT()->SetID(RID_Return);
	CRule *pOperand = NewNT()->SetID(RID_Operand);
	CRule *pPower = NewNT()->SetID(RID_Power);
	CRule *pMult = NewNT()->SetID(RID_Mult);
	CRule *pSum = NewNT()->SetID(RID_Sum);
	CRule *pExpression = NewNT()->SetID(RID_Expression);
	CRule *pLValue = NewNT()->SetID(RID_LValue);
	CRule *pAssignment = NewNT()->SetID(RID_Assignment);
	CRule *pIf = NewNT()->SetID(RID_If);
	CRule *pOperator = NewNT()->SetID(RID_Operator);

	CRule *pFunctionArgs = NewNT();
	CRule *pExpressionList = NewNT();

	pValue->Set(S_Alternative)->
		AddChild(NewT(CToken::TT_NUMBER))->
		AddChild(NewT(CToken::TT_STRING))->
		AddChild(pVariable);

	pVariable->
		AddChild(NewT(CToken::TT_VARIABLE));

	pFunctionDef->Set(O_Output)->
		AddChild(NewT(CToken::TT_FUNCTION)->Set(O_NoOutput))->
		AddChild(NewT(CToken::TT_OPENBRACE)->Set(O_NoOutput))->
		AddChild(NewNT()->Set(R_ZeroOne)->Set(O_Output)->
		  AddChild(NewT(CToken::TT_VARIABLE))->
			AddChild(NewNT()->Set(R_ZeroInfinity)->
			  AddChild(NewT(CToken::TT_COMMA)->Set(O_NoOutput))->
				AddChild(NewT(CToken::TT_VARIABLE))))->
		AddChild(NewT(CToken::TT_CLOSEBRACE)->Set(O_NoOutput))->
		AddChild(NewNT()->Set(R_ZeroInfinity)->Set(O_Output)->
		  AddChild(pOperator))->
		AddChild(NewT(CToken::TT_END)->Set(O_NoOutput));

	pFunctionArgs->Set(O_Output)->
		AddChild(NewT(CToken::TT_OPENBRACE)->Set(O_NoOutput))->
		AddChild(NewNT()->Set(R_ZeroOne)->
			AddChild(pExpressionList))->
		AddChild(NewT(CToken::TT_CLOSEBRACE)->Set(O_NoOutput));

	pFunctionCall->
		AddChild(NewNT()->Set(S_Alternative)->
		  AddChild(pVariable)->
			AddChild(pFunctionDef))->
		AddChild(pFunctionArgs)->
		AddChild(NewNT()->Set(R_ZeroInfinity)->
		  AddChild(pFunctionArgs));

	pReturn->Set(O_Output)->
		AddChild(NewT(CToken::TT_RETURN)->Set(O_NoOutput))->
		AddChild(NewNT()->Set(R_ZeroOne)->
		  AddChild(pExpressionList));

	pOperand->Set(S_Alternative)->
		AddChild(pFunctionCall)->
		AddChild(pValue)->
		AddChild(NewNT()->
		  AddChild(NewT(CToken::TT_OPENBRACE)->Set(O_NoOutput))->
			AddChild(pExpression)->
			AddChild(NewT(CToken::TT_CLOSEBRACE)->Set(O_NoOutput)));

	pPower->
		AddChild(pOperand)->
		AddChild(NewNT()->Set(R_ZeroOne)->
		  AddChild(NewT(CToken::TT_POWER)->Set(O_NoOutput))->
			AddChild(pPower));

	pMult->
		AddChild(pPower)->
		AddChild(NewNT()->Set(R_ZeroInfinity)->
		  AddChild(NewNT()->Set(S_Alternative)->
			  AddChild(NewT(CToken::TT_MULTIPLY))->
				AddChild(NewT(CToken::TT_DIVIDE)))->
			AddChild(pPower));

	pSum->
		AddChild(NewNT()->Set(R_ZeroOne)->Set(S_Alternative)->
		  AddChild(NewT(CToken::TT_PLUS))->
			AddChild(NewT(CToken::TT_MINUS)))->
		AddChild(pMult)->
		AddChild(NewNT()->Set(R_ZeroInfinity)->
		  AddChild(NewNT()->Set(S_Alternative)->
			  AddChild(NewT(CToken::TT_PLUS))->
				AddChild(NewT(CToken::TT_MINUS)))->
			AddChild(pMult));

	pExpression->Set(S_Alternative)->
		AddChild(pSum)->
		AddChild(pFunctionDef);

	pExpressionList->
		AddChild(pExpression)->
		AddChild(NewNT()->Set(R_ZeroInfinity)->
		  AddChild(NewT(CToken::TT_COMMA)->Set(O_NoOutput))->
			AddChild(pExpression));

	pLValue->
		AddChild(NewT(CToken::TT_VARIABLE));

	pAssignment->
		AddChild(NewNT()->Set(O_Output)->
			AddChild(pLValue)->
			AddChild(NewNT()->Set(R_ZeroInfinity)->
  			AddChild(NewT(CToken::TT_COMMA)->Set(O_NoOutput))->
				AddChild(pLValue)))->
		AddChild(NewT(CToken::TT_ASSIGN)->Set(O_NoOutput))->
		AddChild(NewNT()->Set(O_Output)->
			AddChild(pExpressionList));

	pIf->
		AddChild(NewT(CToken::TT_IF)->Set(O_NoOutput))->
		AddChild(pExpression)->
		AddChild(NewT(CToken::TT_THEN)->Set(O_NoOutput))->
		AddChild(NewNT()->Set(R_ZeroInfinity)->Set(O_Output)->
		  AddChild(pOperator))->
		AddChild(NewNT()->Set(R_ZeroOne)->
		  AddChild(NewT(CToken::TT_ELSE)->Set(O_NoOutput))->
			AddChild(NewNT()->Set(R_ZeroInfinity)->Set(O_Output)->
			  AddChild(pOperator)))->
		AddChild(NewT(CToken::TT_END)->Set(O_NoOutput));

	pOperator->Set(S_Alternative)->
		AddChild(pIf)->
		AddChild(pReturn)->
		AddChild(pAssignment)->
		AddChild(pFunctionCall);

	pProgram->Set(R_ZeroInfinity)->
		AddChild(pOperator);

	SetRule(*pProgram);
}

void CBNFGrammar::Clear()
{
	SAFE_DELETE(m_pParsed);
}

bool CBNFGrammar::Parse(CList<CToken *> &lstTokens)
{
	Clear();
	bool bRes = CBNFParser::Parse(lstTokens, m_pParsed);
	return bRes;
}

void CBNFGrammar::Dump(CNode *pParsed, int iIndent)
{
	if (!pParsed)
		return;
	CStrAny sIndent(ST_STR, ' ', iIndent);
	printf(sIndent.m_pBuf);
	if (!pParsed->m_arrChildren.m_iCount) {
		CStrAny sToken = pParsed->m_pToken->ToString();
		printf("%s, ", sToken.m_pBuf);
	}

	CStrAny sRule = s_RID2Str.GetStr(pParsed->m_pRule->m_iID);
	printf("Rule: %s\n", sRule.m_pBuf);

	for (int i = 0; i < pParsed->m_arrChildren.m_iCount; ++i)
		Dump(pParsed->m_arrChildren[i], iIndent + 2);
}