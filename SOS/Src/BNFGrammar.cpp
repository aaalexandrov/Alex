#include "stdafx.h"
#include "BNFGrammar.h"

// CBNFGrammar ----------------------------------------------------------------

CValue2String::TValueString CBNFGrammar::s_arrRID2Str[] = {
	VAL2STR(RID_Program),
	VAL2STR(RID_Constant),
	VAL2STR(RID_Variable),
	VAL2STR(RID_FunctionDef),
	VAL2STR(RID_FunctionCall),
	VAL2STR(RID_ParamList),
	VAL2STR(RID_Operand),
  VAL2STR(RID_DotIndex),
	VAL2STR(RID_Table),
	VAL2STR(RID_Return),
	VAL2STR(RID_Power),
	VAL2STR(RID_Mult),
	VAL2STR(RID_Sum),
  VAL2STR(RID_Concat),
	VAL2STR(RID_Comparison),
	VAL2STR(RID_Not),
	VAL2STR(RID_And),
	VAL2STR(RID_Or),
	VAL2STR(RID_Locals),
	VAL2STR(RID_LValue),
	VAL2STR(RID_Assignment),
	VAL2STR(RID_If),
	VAL2STR(RID_While),
	VAL2STR(RID_Do),
};

CValue2String CBNFGrammar::s_RID2Str(s_arrRID2Str, ARRSIZE(s_arrRID2Str));

CBNFGrammar::CBNFGrammar()
{
	m_pParsed = 0;
	InitRules();
}

void CBNFGrammar::InitRules()
{
	CRule *pProgram = NewNT()->SetID(RID_Program)->SetOutput(true);
	CRule *pConstant = NewNT()->SetID(RID_Constant)->SetOutput(true);
	CRule *pVariable = NewNT()->SetID(RID_Variable)->SetOutput(true);
	CRule *pFunctionDef = NewNT()->SetID(RID_FunctionDef)->SetOutput(true);
	CRule *pFunctionCall = NewNT()->SetID(RID_FunctionCall)->SetOutput(true);
	CRule *pParamList = NewNT()->SetID(RID_ParamList)->SetOutput(true);
	CRule *pOperand = NewNT()->SetID(RID_Operand)->SetOutput(true);
	CRule *pDotIndex = NewNT()->SetID(RID_DotIndex)->SetOutput(true);
  CRule *pTable = NewNT()->SetID(RID_Table)->SetOutput(true);
	CRule *pReturn = NewNT()->SetID(RID_Return)->SetOutput(true);
	CRule *pPower = NewNT()->SetID(RID_Power)->SetOutput(true);
	CRule *pMult = NewNT()->SetID(RID_Mult)->SetOutput(true);
	CRule *pSum = NewNT()->SetID(RID_Sum)->SetOutput(true);
  CRule *pConcat = NewNT()->SetID(RID_Concat)->SetOutput(true);
	CRule *pComparison = NewNT()->SetID(RID_Comparison)->SetOutput(true);
	CRule *pNot = NewNT()->SetID(RID_Not)->SetOutput(true);
	CRule *pAnd = NewNT()->SetID(RID_And)->SetOutput(true);
	CRule *pOr = NewNT()->SetID(RID_Or)->SetOutput(true);
	CRule *pLocals = NewNT()->SetID(RID_Locals)->SetOutput(true);
	CRule *pLValue = NewNT()->SetID(RID_LValue)->SetOutput(true);
	CRule *pAssignment = NewNT()->SetID(RID_Assignment)->SetOutput(true);
	CRule *pIf = NewNT()->SetID(RID_If)->SetOutput(true);
	CRule *pWhile = NewNT()->SetID(RID_While)->SetOutput(true);
	CRule *pFor = NewNT()->SetID(RID_For)->SetOutput(true);
	CRule *pDo = NewNT()->SetID(RID_Do)->SetOutput(true);

	CRule *pIndexable = NewNT();
	CRule *pIndex = NewNT();
	CRule *pTableKey = NewNT();
	CRule *pTableValue = NewNT();
	CRule *pIdentifierList = NewNT();
	CRule *pExpression = NewNT();
	CRule *pExpressionList = NewNT();
	CRule *pOperator = NewNT();

	pConstant->Set(S_Alternative)->
    AddChild(NewT(CToken::TT_NIL)->SetOutput(true))->
		AddChild(NewT(CToken::TT_TRUE)->SetOutput(true))->
		AddChild(NewT(CToken::TT_FALSE)->SetOutput(true))->
		AddChild(NewT(CToken::TT_NUMBER)->SetOutput(true))->
		AddChild(NewT(CToken::TT_STRING)->SetOutput(true))->
		AddChild(pTable);

	pVariable->
		AddChild(NewT(CToken::TT_VARIABLE)->SetOutput(true));

	pIdentifierList->
	  AddChild(NewT(CToken::TT_VARIABLE)->SetOutput(true))->
		AddChild(NewZeroPlus(NewNT()->
		  AddChild(NewT(CToken::TT_COMMA))->
			AddChild(NewT(CToken::TT_VARIABLE)->SetOutput(true))));

	pFunctionDef->SetAllowRenaming(true)->
		AddChild(NewT(CToken::TT_FUNCTION))->
		AddChild(NewT(CToken::TT_OPENBRACE))->
		AddChild(NewOptional(NewNT()->SetAllowRenaming(true)->
			AddChild(pIdentifierList)))->
		AddChild(NewT(CToken::TT_CLOSEBRACE))->
		AddChild(NewNT()->SetAllowRenaming(true)->
		  AddChild(NewZeroPlus(pOperator)))->
		AddChild(NewT(CToken::TT_END));

	pParamList->SetAllowRenaming(true)->
		AddChild(NewT(CToken::TT_OPENBRACE))->
		AddChild(NewOptional(pExpressionList))->
		AddChild(NewT(CToken::TT_CLOSEBRACE));

	pFunctionCall->
		AddChild(NewNT()->Set(S_Alternative)->
		  AddChild(NewNT()->
			  AddChild(pVariable)->
				AddChild(NewZeroPlus(pIndex)))->
			AddChild(pFunctionDef))->
		AddChild(pParamList)->
		AddChild(NewZeroPlus(NewNT()->
		  AddChild(NewZeroPlus(pIndex))->
		  AddChild(pParamList)));

	pTableKey->Set(S_Alternative)->
		AddChild(NewT(CToken::TT_VARIABLE)->SetOutput(true))->
		AddChild(pConstant);

	pTableValue->SetAllowRenaming(true)->
		AddChild(NewOptional(NewNT()->
		  AddChild(pTableKey)->
			AddChild(NewT(CToken::TT_ASSIGN))))->
		AddChild(pExpression);

	pTable->
		AddChild(NewT(CToken::TT_OPENCURLY))->
		AddChild(NewOptional(NewNT()->SetAllowRenaming(true)->
		  AddChild(pTableValue)->
			AddChild(NewZeroPlus(NewNT()->
			  AddChild(NewT(CToken::TT_COMMA))->
				AddChild(pTableValue)))->
			AddChild(NewOptional(NewT(CToken::TT_COMMA)))))->
		AddChild(NewT(CToken::TT_CLOSECURLY));

	pReturn->SetAllowRenaming(true)->
		AddChild(NewT(CToken::TT_RETURN))->
		AddChild(NewOptional(pExpressionList));

	pIndexable->Set(S_Alternative)->
		AddChild(pFunctionCall)->
		AddChild(pVariable)->
		AddChild(NewNT()->
		  AddChild(NewT(CToken::TT_OPENBRACE))->
			AddChild(pExpression)->
			AddChild(NewT(CToken::TT_CLOSEBRACE)));

	pDotIndex->
    AddChild(NewT(CToken::TT_DOT))->
    AddChild(NewT(CToken::TT_VARIABLE)->SetOutput(true));

  pIndex->Set(S_Alternative)->
    AddChild(NewNT()->
		  AddChild(NewT(CToken::TT_OPENBRACKET))->
		  AddChild(pExpression)->
		  AddChild(NewT(CToken::TT_CLOSEBRACKET)))->
    AddChild(pDotIndex);

	pOperand->Set(S_Alternative)->
		AddChild(pConstant)->
		AddChild(NewNT()->
		  AddChild(pIndexable)->
			AddChild(NewZeroPlus(pIndex)));

	pPower->
		AddChild(pOperand)->
    AddChild(NewOptional(NewNT()->
		  AddChild(NewT(CToken::TT_POWER))->
			AddChild(pPower)));

	pMult->
		AddChild(pPower)->
		AddChild(NewZeroPlus(NewNT()->
		  AddChild(NewNT()->Set(S_Alternative)->
			  AddChild(NewT(CToken::TT_MULTIPLY)->SetOutput(true))->
				AddChild(NewT(CToken::TT_DIVIDE)->SetOutput(true)))->
			AddChild(pPower)));

	pSum->
		AddChild(NewNT()->Set(S_Alternative)->
		  AddChild(NewT(CToken::TT_PLUS)->SetOutput(true))->
			AddChild(NewT(CToken::TT_MINUS)->SetOutput(true))->
      AddChild(NewEmpty()))->
		AddChild(pMult)->
		AddChild(NewZeroPlus(NewNT()->
		  AddChild(NewNT()->Set(S_Alternative)->
			  AddChild(NewT(CToken::TT_PLUS)->SetOutput(true))->
				AddChild(NewT(CToken::TT_MINUS)->SetOutput(true)))->
			AddChild(pMult)));

  pConcat->
    AddChild(pSum)->
    AddChild(NewZeroPlus(NewNT()->
      AddChild(NewT(CToken::TT_CONCAT))->
      AddChild(pSum)));

	pComparison->
		AddChild(pConcat)->
		AddChild(NewOptional(NewNT()->
		  AddChild(NewNT()->Set(S_Alternative)->
			  AddChild(NewT(CToken::TT_EQUAL)->SetOutput(true))->
        AddChild(NewT(CToken::TT_NOT_EQUAL)->SetOutput(true))->
        AddChild(NewT(CToken::TT_LESS_EQUAL)->SetOutput(true))->
        AddChild(NewT(CToken::TT_GREAT_EQUAL)->SetOutput(true))->
        AddChild(NewT(CToken::TT_LESS)->SetOutput(true))->
        AddChild(NewT(CToken::TT_GREAT)->SetOutput(true)))->
			AddChild(pConcat)));

	pNot->
		AddChild(NewZeroPlus(NewT(CToken::TT_NOT)->SetOutput(true)))->
		AddChild(pComparison);

	pAnd->
		AddChild(pNot)->
		AddChild(NewZeroPlus(NewNT()->
		  AddChild(NewT(CToken::TT_AND))->
			AddChild(pNot)));

	pOr->
		AddChild(pAnd)->
		AddChild(NewZeroPlus(NewNT()->
		  AddChild(NewT(CToken::TT_OR))->
			AddChild(pAnd)));

	pExpression->Set(S_Alternative)->
		AddChild(pOr)->
		AddChild(pFunctionDef);

	pExpressionList->
		AddChild(pExpression)->
		AddChild(NewZeroPlus(NewNT()->
		  AddChild(NewT(CToken::TT_COMMA))->
			AddChild(pExpression)));

	pLocals->
		AddChild(NewT(CToken::TT_LOCAL))->
		AddChild(NewNT()->SetAllowRenaming(true)->
		  AddChild(pIdentifierList))->
		AddChild(NewOptional(NewNT()->SetAllowRenaming(true)->
			AddChild(NewT(CToken::TT_ASSIGN))->
			AddChild(pExpressionList)));

	pLValue->Set(S_Alternative)->
		AddChild(NewNT()->
		  AddChild(pIndexable)->
      AddChild(NewOnePlus(pIndex)))->
		AddChild(NewT(CToken::TT_VARIABLE)->SetOutput(true));

	pAssignment->
		AddChild(NewNT()->SetAllowRenaming(true)->
			AddChild(pLValue)->
			AddChild(NewZeroPlus(NewNT()->
  			AddChild(NewT(CToken::TT_COMMA))->
				AddChild(pLValue))))->
		AddChild(NewT(CToken::TT_ASSIGN))->
		AddChild(NewNT()->SetAllowRenaming(true)->
			AddChild(pExpressionList));

	pIf->
		AddChild(NewT(CToken::TT_IF))->
		AddChild(pExpression)->
		AddChild(NewT(CToken::TT_THEN))->
		AddChild(NewNT()->SetAllowRenaming(true)->
		  AddChild(NewZeroPlus(pOperator)))->
		AddChild(NewOptional(NewNT()->
		  AddChild(NewT(CToken::TT_ELSE))->
			AddChild(NewNT()->SetAllowRenaming(true)->
			  AddChild(NewZeroPlus(pOperator)))))->
		AddChild(NewT(CToken::TT_END));

	pWhile->
		AddChild(NewT(CToken::TT_WHILE))->
		AddChild(pExpression)->
		AddChild(NewT(CToken::TT_DO))->
		AddChild(NewNT()->SetAllowRenaming(true)->
		  AddChild(NewZeroPlus(pOperator)))->
		AddChild(NewT(CToken::TT_END));

	pFor->
		AddChild(NewT(CToken::TT_FOR))->
		AddChild(NewNT()->Set(S_Alternative)->
      AddChild(NewNT()->
        AddChild(NewT(CToken::TT_VARIABLE)->SetOutput(true))->
        AddChild(NewT(CToken::TT_COMMA))->
        AddChild(NewT(CToken::TT_VARIABLE)->SetOutput(true))->
        AddChild(NewT(CToken::TT_ASSIGN)->SetOutput(true))->
        AddChild(pExpression))->
      AddChild(NewNT()->
        AddChild(NewT(CToken::TT_VARIABLE)->SetOutput(true))->
        AddChild(NewT(CToken::TT_ASSIGN)->SetOutput(true))->
        AddChild(pExpression)->
        AddChild(NewT(CToken::TT_COMMA))->
        AddChild(pExpression)->
        AddChild(NewOptional(NewNT()->SetAllowRenaming(true)->
          AddChild(NewT(CToken::TT_COMMA))->
          AddChild(pExpression)))))->
		AddChild(NewT(CToken::TT_DO))->
		AddChild(NewNT()->SetAllowRenaming(true)->
		  AddChild(NewZeroPlus(pOperator)))->
		AddChild(NewT(CToken::TT_END));

  pDo->
    AddChild(NewT(CToken::TT_DO))->
		AddChild(NewNT()->SetAllowRenaming(true)->
		  AddChild(NewZeroPlus(pOperator)))->
		AddChild(NewT(CToken::TT_END));

	pOperator->Set(S_Alternative)->
		AddChild(pIf)->
		AddChild(pWhile)->
		AddChild(pDo)->
		AddChild(pReturn)->
		AddChild(pLocals)->
		AddChild(pAssignment)->
		AddChild(pFunctionCall);

	pProgram->
		AddChild(NewZeroPlus(pOperator));

	SetRule(*pProgram);
}

void CBNFGrammar::Clear()
{
  if (m_pParsed)
  {
    DEL(m_pParsed);
    m_pParsed = 0;
  }
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
	printf("%s", sIndent.m_pBuf);
	if (!pParsed->m_arrChildren.m_iCount && pParsed->m_pToken) {
		CStrAny sToken = pParsed->m_pToken->ToString();
		printf("%s, ", sToken.m_pBuf);
	}

	CStrAny sRule = s_RID2Str.GetStr(pParsed->m_pRule->m_iID);
	printf("Rule: %s\n", sRule.m_pBuf);

	for (int i = 0; i < pParsed->m_arrChildren.m_iCount; ++i)
		Dump(pParsed->m_arrChildren[i], iIndent + 2);
}
