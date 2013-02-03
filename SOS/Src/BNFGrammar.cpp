#include "stdafx.h"
#include "BNFGrammar.h"

// CBNFGrammar ----------------------------------------------------------------

CBNFGrammar::CBNFGrammar()
{
	m_pParsed = 0;
	InitRules();
}

void CBNFGrammar::InitRules()
{
	CRule *pValue = NewNT()->SetID(RID_Value);
	CRule *pFunctionDef = NewNT()->SetID(RID_FunctionDef);
	CRule *pFunctionCall = NewNT()->SetID(RID_FunctionCall);
	CRule *pOperand = NewNT()->SetID(RID_Operand);
	CRule *pPower = NewNT()->SetID(RID_Power);
	CRule *pMult = NewNT()->SetID(RID_Mult);
	CRule *pSum = NewNT()->SetID(RID_Sum);
	CRule *pExpression = NewNT()->SetID(RID_Expression);
	CRule *pLValue = NewNT()->SetID(RID_LValue);
	CRule *pAssignment = NewNT()->SetID(RID_Assignment);
	CRule *pOperator = NewNT()->SetID(RID_Operator);
	CRule *pProgram = NewNT()->SetID(RID_Program);

	CRule *pFunctionArgs = NewNT();
	CRule *pExpressionList = NewNT();

	pValue->Set(S_Alternative)->
		AddChild(NewT(CToken::TT_VARIABLE))->
		AddChild(NewT(CToken::TT_NUMBER))->
		AddChild(NewT(CToken::TT_STRING));

	pFunctionDef->
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
		  AddChild(NewT(CToken::TT_VARIABLE))->
			AddChild(pFunctionDef))->
		AddChild(pFunctionArgs)->
		AddChild(NewNT()->Set(R_ZeroInfinity)->
		  AddChild(pFunctionArgs));

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
		AddChild(pLValue)->
		AddChild(NewNT()->Set(R_ZeroInfinity)->
		AddChild(NewT(CToken::TT_COMMA)->Set(O_NoOutput))->
			AddChild(pLValue))->
		AddChild(NewT(CToken::TT_ASSIGN))->
		AddChild(pExpressionList);

	pOperator->Set(S_Alternative)->
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
	CStrAny sRule;
	switch (pParsed->m_pRule->m_iID) {
	  case RID_Value:
			sRule = "Value";
			break;
		case RID_FunctionDef:
			sRule = "FunctionDef";
			break;
		case RID_FunctionCall:
			sRule = "FunctionCall";
			break;
		case RID_Operand:
			sRule = "Operand";
			break;
		case RID_Power:
			sRule = "Power";
			break;
		case RID_Mult:
			sRule = "Mult";
			break;
		case RID_Sum:
			sRule = "Sum";
			break;
		case RID_Expression:
			sRule = "Expression";
			break;
		case RID_LValue:
			sRule = "LValue";
			break;
		case RID_Assignment:
			sRule = "Assignment";
			break;
		case RID_Operator:
			sRule = "Operator";
			break;
		case RID_Program:
			sRule = "Program";
			break;
		default:
			ASSERT(!"Unknown rule");
			sRule = CStrAny(ST_WHOLE, "Unknown rule ID ") + CStrAny(ST_STR, pParsed->m_pRule->m_iID);
			break;
	}
	
	printf("Rule: %s\n", sRule.m_pBuf);

	for (int i = 0; i < pParsed->m_arrChildren.m_iCount; ++i)
		Dump(pParsed->m_arrChildren[i], iIndent + 2);
}