#include "stdafx.h"
#include "BNFGrammar.h"

// CBNFGrammar ----------------------------------------------------------------

CValue2String::TValueString CBNFGrammar::s_arrRID2Str[] = {
	VAL2STR(RID_Program),
	VAL2STR(RID_Constant),
	VAL2STR(RID_Variable),
	VAL2STR(RID_FunctionDef),
	VAL2STR(RID_FunctionCall),
	VAL2STR(RID_Operand),
	VAL2STR(RID_Table),
	VAL2STR(RID_Return),
	VAL2STR(RID_Power),
	VAL2STR(RID_Mult),
	VAL2STR(RID_Sum),
	VAL2STR(RID_Comparison),
	VAL2STR(RID_Not),
	VAL2STR(RID_And),
	VAL2STR(RID_Or),
	VAL2STR(RID_LValue),
	VAL2STR(RID_Assignment),
	VAL2STR(RID_If),
	VAL2STR(RID_While),
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
	CRule *pConstant = NewNT()->SetID(RID_Constant);
	CRule *pVariable = NewNT()->SetID(RID_Variable);
	CRule *pFunctionDef = NewNT()->SetID(RID_FunctionDef);
	CRule *pFunctionCall = NewNT()->SetID(RID_FunctionCall);
	CRule *pOperand = NewNT()->SetID(RID_Operand);
	CRule *pTable = NewNT()->SetID(RID_Table);
	CRule *pReturn = NewNT()->SetID(RID_Return);
	CRule *pPower = NewNT()->SetID(RID_Power);
	CRule *pMult = NewNT()->SetID(RID_Mult);
	CRule *pSum = NewNT()->SetID(RID_Sum);
	CRule *pComparison = NewNT()->SetID(RID_Comparison);
	CRule *pNot = NewNT()->SetID(RID_Not);
	CRule *pAnd = NewNT()->SetID(RID_And);
	CRule *pOr = NewNT()->SetID(RID_Or);
	CRule *pLValue = NewNT()->SetID(RID_LValue);
	CRule *pAssignment = NewNT()->SetID(RID_Assignment);
	CRule *pIf = NewNT()->SetID(RID_If);
	CRule *pWhile = NewNT()->SetID(RID_While);

	CRule *pIndexable = NewNT();
	CRule *pIndex = NewNT();
	CRule *pFunctionArgs = NewNT();
	CRule *pTableKey = NewNT();
	CRule *pTableValue = NewNT();
	CRule *pExpression = NewNT();
	CRule *pExpressionList = NewNT();
	CRule *pOperator = NewNT();

	pConstant->Set(S_Alternative)->
		AddChild(NewT(CToken::TT_TRUE))->
		AddChild(NewT(CToken::TT_FALSE))->
		AddChild(NewT(CToken::TT_NUMBER))->
		AddChild(NewT(CToken::TT_STRING))->
		AddChild(pTable);

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

	pTableKey->Set(S_Alternative)->
		AddChild(NewT(CToken::TT_VARIABLE))->
		AddChild(pConstant);

	pTableValue->Set(O_Output)->
		AddChild(NewNT()->Set(R_ZeroOne)->
		  AddChild(pTableKey)->
			AddChild(NewT(CToken::TT_ASSIGN)->Set(O_NoOutput)))->
		AddChild(pExpression);

	pTable->
		AddChild(NewT(CToken::TT_OPENCURLY)->Set(O_NoOutput))->
		AddChild(NewNT()->Set(R_ZeroOne)->Set(O_Output)->
		  AddChild(pTableValue)->
			AddChild(NewNT()->Set(R_ZeroInfinity)->
			  AddChild(NewT(CToken::TT_COMMA)->Set(O_NoOutput))->
				AddChild(pTableValue))->
			AddChild(NewNT()->Set(R_ZeroOne)->
			  AddChild(NewT(CToken::TT_COMMA)->Set(O_NoOutput))))->
		AddChild(NewT(CToken::TT_CLOSECURLY)->Set(O_NoOutput));

	pReturn->Set(O_Output)->
		AddChild(NewT(CToken::TT_RETURN)->Set(O_NoOutput))->
		AddChild(NewNT()->Set(R_ZeroOne)->
		  AddChild(pExpressionList));

	pIndexable->Set(S_Alternative)->
		AddChild(pFunctionCall)->
		AddChild(pVariable)->
		AddChild(NewNT()->
		  AddChild(NewT(CToken::TT_OPENBRACE)->Set(O_NoOutput))->
			AddChild(pExpression)->
			AddChild(NewT(CToken::TT_CLOSEBRACE)->Set(O_NoOutput)));

	pIndex->
		AddChild(NewT(CToken::TT_OPENBRACKET)->Set(O_NoOutput))->
		AddChild(pExpression)->
		AddChild(NewT(CToken::TT_CLOSEBRACKET)->Set(O_NoOutput));

	pOperand->Set(S_Alternative)->
		AddChild(pConstant)->
		AddChild(NewNT()->
		  AddChild(pIndexable)->
			AddChild(NewNT()->Set(R_ZeroInfinity)->
			  AddChild(pIndex)));

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

	pComparison->
		AddChild(pSum)->
		AddChild(NewNT()->Set(R_ZeroOne)->
		  AddChild(NewNT()->Set(S_Alternative)->
			  AddChild(NewT(CToken::TT_EQUAL))->
        AddChild(NewT(CToken::TT_NOT_EQUAL))->
        AddChild(NewT(CToken::TT_LESS_EQUAL))->
        AddChild(NewT(CToken::TT_GREAT_EQUAL))->
        AddChild(NewT(CToken::TT_LESS))->
        AddChild(NewT(CToken::TT_GREAT)))->
			AddChild(pSum));

	pNot->
		AddChild(NewNT()->Set(R_ZeroOne)->
		  AddChild(NewT(CToken::TT_NOT)))->
		AddChild(pComparison);

	pAnd->
		AddChild(pNot)->
		AddChild(NewNT()->Set(R_ZeroInfinity)->
		  AddChild(NewT(CToken::TT_AND)->Set(O_NoOutput))->
			AddChild(pNot));

	pOr->
		AddChild(pAnd)->
		AddChild(NewNT()->Set(R_ZeroInfinity)->
		  AddChild(NewT(CToken::TT_OR)->Set(O_NoOutput))->
			AddChild(pAnd));

	pExpression->Set(S_Alternative)->
		AddChild(pOr)->
		AddChild(pFunctionDef);

	pExpressionList->
		AddChild(pExpression)->
		AddChild(NewNT()->Set(R_ZeroInfinity)->
		  AddChild(NewT(CToken::TT_COMMA)->Set(O_NoOutput))->
			AddChild(pExpression));

	pLValue->Set(S_Alternative)->
		AddChild(NewNT()->
		  AddChild(pIndexable)->
			AddChild(pIndex)->
			AddChild(NewNT()->Set(R_ZeroInfinity)->
			  AddChild(pIndex)))->
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

	pWhile->
		AddChild(NewT(CToken::TT_WHILE)->Set(O_NoOutput))->
		AddChild(pExpression)->
		AddChild(NewT(CToken::TT_DO)->Set(O_NoOutput))->
		AddChild(NewNT()->Set(R_ZeroInfinity)->Set(O_Output)->
		  AddChild(pOperator))->
		AddChild(NewT(CToken::TT_END)->Set(O_NoOutput));

	pOperator->Set(S_Alternative)->
		AddChild(pIf)->
		AddChild(pWhile)->
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