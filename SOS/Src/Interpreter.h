#ifndef __INTERPRETER_H
#define __INTERPRETER_H

#include "Error.h"

// Execution stack ------------------------------------------------------------

class CStackValue {
public:
  virtual ~CStackValue() {}
  virtual CBaseVar *GetVar() = 0;
  virtual EInterpretError SetVar(CBaseVar *pVar) = 0;
};

class CStackLiteral: public CStackValue {
public:
  CBaseVar *m_pVar;

  CStackLiteral(CBaseVar *pVar)                      { m_pVar = pVar; }
  virtual CBaseVar *GetVar()                         { return m_pVar; }
  virtual EInterpretError SetVar(CBaseVar *pVar)     { return IERR_NOT_LVALUE; }
};

class CStackConstant: public CStackValue {
public:
  CBaseVar *m_pVar;

  CStackConstant(CBaseVar *pVar)                      { m_pVar = pVar; }
  virtual ~CStackConstant()                           { delete m_pVar; }
  virtual CBaseVar *GetVar()                          { return m_pVar; }
  virtual EInterpretError SetVar(CBaseVar *pVar)      { return IERR_NOT_LVALUE; }
};

class CStackVar: public CStackValue {
public:
  CVarObj::CIter *m_pVarIter;

	CStackVar(CVarObj::CIter *pVarIter)            { m_pVarIter = pVarIter; }
  virtual ~CStackVar()                           { delete m_pVarIter; }
  virtual CBaseVar *GetVar()                     { return m_pVarIter->GetValue(); }
	virtual EInterpretError SetVar(CBaseVar *pVar) { if (m_pVarIter->SetValue(pVar)) return IERR_OK; else return IERR_NOT_LVALUE; }
};

typedef CList<CStackValue *>  TExecutionStack;

// Execution commands ---------------------------------------------------------

class CCmd: public CObject {
  DEFRTTI(CCmd, CObject, false)
public:
	virtual ~CCmd() {}
	virtual EInterpretError Execute(TExecutionStack &kStack, CVarObj &kContext) = 0;
};

class CCmdPushLiteral: public CCmd {
  DEFRTTI(CCmdPushLiteral, CCmd, false)
public:
  CBaseVar *m_pVar;

	CCmdPushLiteral(CBaseVar *pVar) { m_pVar = pVar; }
	virtual ~CCmdPushLiteral()      { delete m_pVar; }

	virtual EInterpretError Execute(TExecutionStack &kStack, CVarObj &kContext);
};

class CCmdPushVar: public CCmd {
  DEFRTTI(CCmdPushVar, CCmd, false)
public:
	CStrAny m_sVarName;

	CCmdPushVar(const CStrAny &sVarName): m_sVarName(sVarName) {}

	virtual EInterpretError Execute(TExecutionStack &kStack, CVarObj &kContext);
};

class CCmdPlus: public CCmd {
	DEFRTTI(CCmdPlus, CCmd, true)
public:
  virtual EInterpretError Execute(TExecutionStack &kStack, CVarObj &kContext);
};

class CCmdMinus: public CCmd {
	DEFRTTI(CCmdMinus, CCmd, true)
public:
  virtual EInterpretError Execute(TExecutionStack &kStack, CVarObj &kContext);
};

class CCmdMultiply: public CCmd {
	DEFRTTI(CCmdMultiply, CCmd, true)
public:
  virtual EInterpretError Execute(TExecutionStack &kStack, CVarObj &kContext);
};

class CCmdDivide: public CCmd {
	DEFRTTI(CCmdDivide, CCmd, true)
public:
  virtual EInterpretError Execute(TExecutionStack &kStack, CVarObj &kContext);
};

class CCmdNegate: public CCmd {
	DEFRTTI(CCmdNegate, CCmd, true)
public:
  virtual EInterpretError Execute(TExecutionStack &kStack, CVarObj &kContext);
};

class CCmdPower: public CCmd {
	DEFRTTI(CCmdPower, CCmd, true)
public:
  virtual EInterpretError Execute(TExecutionStack &kStack, CVarObj &kContext);
};

class CCmdAssign: public CCmd {
	DEFRTTI(CCmdPower, CCmd, true)
public:
  virtual EInterpretError Execute(TExecutionStack &kStack, CVarObj &kContext);
};

// Interpreter ----------------------------------------------------------------

class CInterpreter {
public:
  CVarHash m_Vars;
	TExecutionStack m_kStack;

  CInterpreter();
  ~CInterpreter();
};

#endif

