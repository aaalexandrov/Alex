#include "stdafx.h"
#include "Expression.h"

using namespace Parse;

// Operator execution ----------------------------------------------------------

typedef EInterpretError (*FOperatorExecute)(TExecutionStack &kStack);

EInterpretError OperatorPlus(TExecutionStack &kStack)
{
  if (kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;

  CStackValue *pVal1, *pVal2;

  pVal1 = kStack.Pop();
  pVal2 = kStack.Pop();

  if (IsKindOf<CVar<CStrAny> >(pVal1->GetVar()) || IsKindOf<CVar<CStrAny> >(pVal2->GetVar())) {
    CStrAny s1, s2;
    pVal1->GetVar()->GetStr(s1);
    pVal2->GetVar()->GetStr(s2);
    kStack.Push(new CStackConstant(new CVar<CStrAny>(s2 + s1)));
  } else {
    float f1, f2;
    pVal1->GetVar()->GetFloat(f1);
    pVal2->GetVar()->GetFloat(f2);
    kStack.Push(new CStackConstant(new CVar<float>(f2 + f1)));
  }
  delete pVal1;
  delete pVal2;

  return IERR_OK;
}

EInterpretError OperatorMinus(TExecutionStack &kStack)
{
  if (kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;

  CStackValue *pVal1, *pVal2;

  pVal1 = kStack.Pop();
  pVal2 = kStack.Pop();

  float f1, f2;
  pVal1->GetVar()->GetFloat(f1);
  pVal2->GetVar()->GetFloat(f2);
  kStack.Push(new CStackConstant(new CVar<float>(f2 - f1)));

  delete pVal1;
  delete pVal2;

  return IERR_OK;
}

EInterpretError OperatorMultiply(TExecutionStack &kStack)
{
  if (kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;

  CStackValue *pVal1, *pVal2;

  pVal1 = kStack.Pop();
  pVal2 = kStack.Pop();

  float f1, f2;
  pVal1->GetVar()->GetFloat(f1);
  pVal2->GetVar()->GetFloat(f2);
  kStack.Push(new CStackConstant(new CVar<float>(f2 * f1)));

  delete pVal1;
  delete pVal2;

  return IERR_OK;
}

EInterpretError OperatorDivide(TExecutionStack &kStack)
{
  if (kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;

  CStackValue *pVal1, *pVal2;

  pVal1 = kStack.Pop();
  pVal2 = kStack.Pop();

  float f1, f2;
  pVal1->GetVar()->GetFloat(f1);
  pVal2->GetVar()->GetFloat(f2);
  kStack.Push(new CStackConstant(new CVar<float>(f2 / f1)));

  delete pVal1;
  delete pVal2;

  return IERR_OK;
}

EInterpretError OperatorAssign(TExecutionStack &kStack)
{
  if (kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;

  CStackValue *pVal;
  EInterpretError err;

  pVal = kStack.Pop();
  err = kStack.Head()->SetVar(pVal->GetVar()->Clone());

  delete pVal;

  return err;
}

EInterpretError OperatorNegate(TExecutionStack &kStack)
{
  if (kStack.m_iCount < 1)
    return IERR_NOT_ENOUGH_OPERANDS;

  CStackValue *pVal;

	pVal = kStack.Pop();
  float f;
	pVal->GetVar()->GetFloat(f);
	kStack.Push(new CStackConstant(new CVar<float>(-f)));

	delete pVal;

	return IERR_OK;
}

EInterpretError OperatorPower(TExecutionStack &kStack)
{
  if (kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;

  CStackValue *pVal1, *pVal2;

  pVal1 = kStack.Pop();
  pVal2 = kStack.Pop();

  float f1, f2;
  pVal1->GetVar()->GetFloat(f1);
  pVal2->GetVar()->GetFloat(f2);
  kStack.Push(new CStackConstant(new CVar<float>(pow(f2, f1))));

  delete pVal1;
  delete pVal2;

  return IERR_OK;
}


// Parsing helpers -------------------------------------------------------------

class COperatorInfo {
public:
	CToken::ETokenType m_eType;
  int m_iPriority;
	bool m_bLeftAssociative;
	FOperatorExecute m_fnExecute;
	CRTTI *m_pCmdRTTI;

	COperatorInfo(CToken::ETokenType eType, int iPriority, bool bLeftAssociative, FOperatorExecute fnExecute, CRTTI *pCmdRTTI):
	  m_eType(eType), m_iPriority(iPriority), m_bLeftAssociative(bLeftAssociative), m_fnExecute(fnExecute), m_pCmdRTTI(pCmdRTTI) {}

	inline bool Cmp(const COperatorInfo &kInfo) const { if (m_bLeftAssociative) return m_iPriority <= kInfo.m_iPriority; else return m_iPriority < kInfo.m_iPriority; }
};

COperatorInfo g_Operators[CToken::OPERATOR_NUM + 1] = {
	COperatorInfo(CToken::TT_PLUS,          0, true,  OperatorPlus,     &CCmdPlus::s_RTTI),
  COperatorInfo(CToken::TT_MINUS,         0, true,  OperatorMinus,    &CCmdMinus::s_RTTI),
	COperatorInfo(CToken::TT_MULTIPLY,     10, true,  OperatorMultiply, &CCmdMultiply::s_RTTI),
	COperatorInfo(CToken::TT_DIVIDE,       10, true,  OperatorDivide,   &CCmdDivide::s_RTTI),
	COperatorInfo(CToken::TT_POWER,        15, false, OperatorPower,    &CCmdPower::s_RTTI),
	COperatorInfo(CToken::TT_NEGATE,       20, true,  OperatorNegate,   &CCmdNegate::s_RTTI),
	COperatorInfo(CToken::TT_ASSIGN,      -10, false, OperatorAssign,   &CCmdAssign::s_RTTI),

	COperatorInfo(CToken::TT_OPENBRACE, -1000, true,  0,                0)
};

inline const COperatorInfo *GetOperatorInfo(CToken::ETokenType eType)
{
	ASSERT(eType >= CToken::TT_OPERATOR_BASE && eType <= CToken::TT_OPERATOR_LAST);
	return &g_Operators[eType - CToken::TT_OPERATOR_BASE];
}

// CExpression -----------------------------------------------------------------

CExpression::CExpression()
{
	m_eStatus = IERR_UNINITIALIZED;
}

CExpression::~CExpression()
{
	Clear();
}

void CExpression::Clear()
{
  m_Tokenizer.Clear();
  m_lstPostfix.Clear();
	m_lstCommands.DeleteAll();
	m_eStatus = IERR_UNINITIALIZED;
}

EInterpretError CExpression::Init(const CStrAny &sExpression)
{
	EInterpretError err;
	err = m_Tokenizer.Tokenize(sExpression);
	if (err != IERR_OK) {
		m_eStatus = err;
		return err;
	}

	err = BuildPostfix();
	if (err != IERR_OK) {
		m_eStatus = err;
		return err;
	}

	err = BuildCommands();
	if (err != IERR_OK) {
		m_eStatus = err;
		return err;
	}

	m_eStatus = IERR_OK;
	return IERR_OK;
}


EInterpretError CExpression::BuildPostfix()
{
  CList<CToken *> kStack;

	ASSERT(!m_lstPostfix.m_iCount);
	CList<CToken *>::TNode *pNode;
	const COperatorInfo *pOpInfo;
	for (pNode = m_Tokenizer.m_lstTokens.m_pHead; pNode; pNode = pNode->pNext) {
		switch (pNode->Data->m_eClass) {
			case CToken::TC_IDENTIFIER:
				m_lstPostfix.PushTail(pNode->Data);
				break;
			case CToken::TC_LITERAL:
				m_lstPostfix.PushTail(pNode->Data);
				break;
			case CToken::TC_OPERATOR:
				pOpInfo = GetOperatorInfo(pNode->Data->m_eType);
				while (kStack.m_iCount && pOpInfo->Cmp(*GetOperatorInfo(kStack.Tail()->m_eType)))
          m_lstPostfix.PushTail(kStack.PopTail());
				kStack.PushTail(pNode->Data);
				break;
			case CToken::TC_SEPARATOR:
				if (pNode->Data->m_eType == CToken::TT_OPENBRACE)
					kStack.PushTail(pNode->Data);
				else {
					ASSERT(pNode->Data->m_eType == CToken::TT_CLOSEBRACE);
					while (kStack.m_iCount && kStack.Tail()->m_eType != CToken::TT_OPENBRACE)
						m_lstPostfix.PushTail(kStack.PopTail());
          if (kStack.m_iCount)
						kStack.PopTail();
					else {
						// ERROR: close brace without matching opening brace
            m_lstPostfix.Clear();
						return IERR_NO_OPEN_BRACE;
					}
				}
				break;
		}
	}
	while (kStack.m_iCount) {
		if (kStack.Tail()->m_eClass != CToken::TC_OPERATOR) {
			// ERROR: opening brace without a matching closing brace
			m_lstPostfix.Clear();
			return IERR_NO_CLOSE_BRACE;
		}
		m_lstPostfix.PushTail(kStack.PopTail());
	}

	return IERR_OK;
}

EInterpretError CExpression::BuildCommands()
{
  CList<CToken *> kStack;

	ASSERT(!m_lstCommands.m_iCount);
	CList<CToken *>::TNode *pNode;
	const COperatorInfo *pOpInfo, *pCurOpInfo;
	for (pNode = m_Tokenizer.m_lstTokens.m_pHead; pNode; pNode = pNode->pNext) {
		switch (pNode->Data->m_eClass) {
			case CToken::TC_IDENTIFIER:
				m_lstCommands.PushTail(new CCmdPushVar(pNode->Data->m_sToken));
				break;
			case CToken::TC_LITERAL: {
				CBaseVar *pVar;
				switch (pNode->Data->m_eType) {
			    case CToken::TT_STRING:
						pVar = new CVar<CStrAny>();
						break;
					case CToken::TT_NUMBER:
						pVar = new CVar<float>();
						break;
				}
				pVar->SetStr(pNode->Data->m_sToken);
				m_lstCommands.PushTail(new CCmdPushLiteral(pVar));
				break;
      }
			case CToken::TC_OPERATOR:
				pOpInfo = GetOperatorInfo(pNode->Data->m_eType);
				while (kStack.m_iCount && pOpInfo->Cmp(*(pCurOpInfo = GetOperatorInfo(kStack.Head()->m_eType)))) {
					m_lstCommands.PushTail((CCmd *) pCurOpInfo->m_pCmdRTTI->CreateInstance());
					kStack.Pop();
				}
				kStack.Push(pNode->Data);
				break;
			case CToken::TC_SEPARATOR:
				if (pNode->Data->m_eType == CToken::TT_OPENBRACE)
					kStack.Push(pNode->Data);
				else {
					ASSERT(pNode->Data->m_eType == CToken::TT_CLOSEBRACE);
					while (kStack.m_iCount && kStack.Head()->m_eType != CToken::TT_OPENBRACE) 
						m_lstCommands.PushTail((CCmd *) GetOperatorInfo(kStack.Pop()->m_eType)->m_pCmdRTTI->CreateInstance());
          if (kStack.m_iCount)
						kStack.Pop();
					else {
						// ERROR: close brace without matching opening brace
            m_lstCommands.Clear();
						return IERR_NO_OPEN_BRACE;
					}
				}
				break;
		}
	}
	while (kStack.m_iCount) {
		if (kStack.Head()->m_eClass != CToken::TC_OPERATOR) {
			// ERROR: opening brace without a matching closing brace
			m_lstCommands.Clear();
			return IERR_NO_CLOSE_BRACE;
		}
		m_lstCommands.PushTail((CCmd *) GetOperatorInfo(kStack.Pop()->m_eType)->m_pCmdRTTI->CreateInstance());
	}

	return IERR_OK;
}

bool CExpression::Valid() const
{
  return m_eStatus == IERR_OK;
}

EInterpretError CExpression::Execute(TExecutionStack &kStack, CVarObj &kContext)
{
	EInterpretError err;
	CList<CCmd *>::TNode *pNode;
	err = IERR_OK;
	ASSERT(m_eStatus == IERR_OK);
	for (pNode = m_lstCommands.m_pHead; pNode && err == IERR_OK; pNode = pNode->pNext) {
    err = pNode->Data->Execute(kStack, kContext);
	}
	return err;
}

EInterpretError CExpression::Evaluate(CInterpreter *pInterpreter)
{
  CList<CStackValue *> kStack;

  CList<CToken *>::TNode *pNode;
  CStackValue *pVal;
  EInterpretError err = IERR_OK;

  for (pNode = m_lstPostfix.m_pHead; pNode; pNode = pNode->pNext) {
		switch (pNode->Data->m_eClass) {
			case CToken::TC_IDENTIFIER: {
        CVarObj::CIter *pIt;
        pIt = pInterpreter->m_Vars.GetIter(pNode->Data->m_sToken);
			  if (!pIt) {
			    pInterpreter->m_Vars.ReplaceVar(pNode->Data->m_sToken, new CDummyVar(), true);
			    pIt = pInterpreter->m_Vars.GetIter(pNode->Data->m_sToken);
			  }
			  ASSERT(pIt && !!*pIt);
				pVal = new CStackVar(pIt);
			  kStack.Push(pVal);
				break;
			}
			case CToken::TC_LITERAL: {
        CBaseVar *pVar;
			  switch(pNode->Data->m_eType) {
			    case CToken::TT_NUMBER:
			      pVar = new CVar<float>();
			      break;
          case CToken::TT_STRING:
            pVar = new CVar<CStrAny>();
            break;
          default:
            ASSERT(!"Unknown literal type!");
            break;
			  }
			  pVar->SetStr(pNode->Data->m_sToken);
			  pVal = new CStackConstant(pVar);
			  kStack.Push(pVal);
				break;
			}
			case CToken::TC_OPERATOR: {
			  FOperatorExecute fnOp;
				fnOp = GetOperatorInfo(pNode->Data->m_eType)->m_fnExecute;
			  err = fnOp(kStack);
			  if (err != IERR_OK)
			    pNode = 0;
				break;
			}
		}
  }

  kStack.DeleteAll();

  return err;
}

void CExpression::Dump(CList<CToken *> *pList)
{
	if (!pList) {
		printf("Tokens: ");
		Dump(&m_Tokenizer.m_lstTokens);
		printf("Postfix: ");
		Dump(&m_lstPostfix);
		return;
	}
  CList<CToken*>::TNode *pNode;
  printf("<start of token list>\n");
  for (pNode = pList->m_pHead; pNode; pNode = pNode->pNext) {
    CStrAny s = pNode->Data->ToString();
    s += CStrAny(ST_WHOLE, "\n");
    printf(s.m_pBuf);
  }
  printf("<end of token list>\n");
}
