#include "stdafx.h"
#include "Interpreter.h"

// Execution commands ---------------------------------------------------------

CRTTIRegisterer<CCmd> g_RegCmd;
CRTTIRegisterer<CCmdPushLiteral> g_RegCmdPushLiteral;

EInterpretError CCmdPushLiteral::Execute(TExecutionStack &kStack, CVarObj &kContext)
{
	kStack.Push(new CStackLiteral(m_pVar));
	return IERR_OK;
}

CRTTIRegisterer<CCmdPushVar> g_RegCmdPushVar;

EInterpretError CCmdPushVar::Execute(TExecutionStack &kStack, CVarObj &kContext)
{
  CVarObj::CIter *pIt;
  pIt = kContext.GetIter(m_sVarName);
  if (!pIt) {
    kContext.ReplaceVar(m_sVarName, new CDummyVar(), true);
    pIt = kContext.GetIter(m_sVarName);
  }
  ASSERT(pIt);
  kStack.Push(new CStackVar(pIt));
  return IERR_OK;
}

CRTTIRegisterer<CCmdPlus> g_RegCmdPlus;

EInterpretError CCmdPlus::Execute(TExecutionStack &kStack, CVarObj &kContext)
{
  if (kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;

  CStackValue *pVal1, *pVal2;

  pVal1 = kStack.Pop();
  pVal2 = kStack.Pop();

	if (pVal1->GetVar()->GetRTTI()->IsKindOf(CVar<CStrAny>::GetRTTI_s()) || pVal2->GetVar()->GetRTTI()->IsKindOf(CVar<CStrAny>::GetRTTI_s())) {
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

CRTTIRegisterer<CCmdMinus> g_RegCmdMinus;

EInterpretError CCmdMinus::Execute(TExecutionStack &kStack, CVarObj &kContext)
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

CRTTIRegisterer<CCmdMultiply> g_RegCmdMultiply;

EInterpretError CCmdMultiply::Execute(TExecutionStack &kStack, CVarObj &kContext)
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

CRTTIRegisterer<CCmdDivide> g_RegCmdDivide;

EInterpretError CCmdDivide::Execute(TExecutionStack &kStack, CVarObj &kContext)
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

CRTTIRegisterer<CCmdNegate> g_RegCmdNegate;

EInterpretError CCmdNegate::Execute(TExecutionStack &kStack, CVarObj &kContext)
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

CRTTIRegisterer<CCmdPower> g_RegCmdPower;

EInterpretError CCmdPower::Execute(TExecutionStack &kStack, CVarObj &kContext)
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

CRTTIRegisterer<CCmdAssign> g_RegCmdAssign;

EInterpretError CCmdAssign::Execute(TExecutionStack &kStack, CVarObj &kContext)
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

// Interpreter ----------------------------------------------------------------

CInterpreter::CInterpreter()
{
}

CInterpreter::~CInterpreter()
{
	m_kStack.DeleteAll();
}
