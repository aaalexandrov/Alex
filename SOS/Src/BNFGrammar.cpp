#include "stdafx.h"
#include "BNFGrammar.h"

// CBNFGrammar ----------------------------------------------------------------

CBNFGrammar::CBNFGrammar()
{
	InitRules();
}

void CBNFGrammar::InitRules()
{
	CRule *pValue = NewNT()->SetID(RID_Value);
	CRule *pOperand = NewNT()->SetID(RID_Operand);
	CRule *pPower = NewNT()->SetID(RID_Power);
	CRule *pMult = NewNT()->SetID(RID_Mult);
	CRule *pSum = NewNT()->SetID(RID_Sum);
	CRule *pExpression = NewNT()->SetID(RID_Expression);
	CRule *pLValue = NewNT()->SetID(RID_LValue);
	CRule *pAssignment = NewNT()->SetID(RID_Assignment);
	CRule *pProgram = NewNT()->SetID(RID_Program);

	pValue->Set(S_Alternative)->
		AddChild(NewT(CToken::TT_VARIABLE))->
		AddChild(NewT(CToken::TT_NUMBER))->
		AddChild(NewT(CToken::TT_STRING));

	pOperand->Set(S_Alternative)->
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

	pExpression->
		AddChild(pSum);

	pLValue->
		AddChild(NewT(CToken::TT_VARIABLE));

	pAssignment->
		AddChild(pLValue)->
		AddChild(NewNT()->Set(R_ZeroInfinity)->
		AddChild(NewT(CToken::TT_COMMA)->Set(O_NoOutput))->
			AddChild(pLValue))->
		AddChild(NewT(CToken::TT_ASSIGN))->
		AddChild(pExpression)->
		AddChild(NewNT()->Set(R_ZeroInfinity)->
		AddChild(NewT(CToken::TT_COMMA)->Set(O_NoOutput))->
			AddChild(pExpression));

	pProgram->Set(R_ZeroInfinity)->
		AddChild(pAssignment);

	SetRule(*pProgram);
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
		case RID_Program:
			sRule = "Program";
			break;
		default:
			ASSERT("Unknown rule ID");
			sRule = CStrAny(ST_WHOLE, "Unknown") + CStrAny(ST_STR, pParsed->m_pRule->m_iID);
			break;
	}
	
	printf("Rule: %s\n", sRule.m_pBuf);

	for (int i = 0; i < pParsed->m_arrChildren.m_iCount; ++i)
		Dump(pParsed->m_arrChildren[i], iIndent + 2);
}