#include "stdafx.h"
#include "Interpreter.h"

// Execution commands ---------------------------------------------------------

IMPRTTI_NOCREATE(CCmd, CObject)
IMPRTTI_NOCREATE(CCmdPushLiteral, CCmd)

EInterpretError CCmdPushLiteral::Execute(TExecutionStack &kStack, CVarObj &kContext)
{
	kStack.Push(new CStackLiteral(m_pVar));
	return IERR_OK;
}

IMPRTTI_NOCREATE(CCmdPushVar, CCmd)

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

IMPRTTI(CCmdPlus, CCmd)

EInterpretError CCmdPlus::Execute(TExecutionStack &kStack, CVarObj &kContext)
{
  if (kStack.m_iCount < 2)
    return IERR_NOT_ENOUGH_OPERANDS;

  CStackValue *pVal1, *pVal2;

  pVal1 = kStack.Pop();
  pVal2 = kStack.Pop();

	if (pVal1->GetVar()->GetRTTI()->IsKindOf(&CVar<CStrAny>::s_RTTI) || pVal2->GetVar()->GetRTTI()->IsKindOf(&CVar<CStrAny>::s_RTTI)) {
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

IMPRTTI(CCmdMinus, CCmd)

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

IMPRTTI(CCmdMultiply, CCmd)

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

IMPRTTI(CCmdDivide, CCmd)

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

IMPRTTI(CCmdNegate, CCmd)

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

IMPRTTI(CCmdPower, CCmd)

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

IMPRTTI(CCmdAssign, CCmd)

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
