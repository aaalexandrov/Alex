#ifndef __EXECUTION_H
#define __EXECUTION_H

#include "Variable.h"

typedef CArray<CValue> TValueStack;

class CExecution;
class CInstruction {
public:
  enum EType {
    IT_NOP,

    IT_PUSH_VALUE,

    IT_ADD,
    IT_SUBTRACT,
    IT_MULTIPLY,
    IT_DIVIDE,
    IT_POWER,
    IT_ASSIGN,
    IT_RESOLVE_VAR,
    IT_RESOLVE_REF,
		IT_CALL,

    IT_LAST
  };
public:
  EType m_eType;
  union {
    BYTE m_btValue[sizeof(CValue)];
  };

  CInstruction(): m_eType(IT_NOP) {}
  ~CInstruction() { ReleaseData(); }

  void ReleaseData();

  void SetNop() { ReleaseData(); }
  void SetPushValue(CValue const &kValue) { ReleaseData(); m_eType = IT_PUSH_VALUE; GetValue().Set(kValue); }
  void SetAdd() { ReleaseData(); m_eType = IT_ADD; }
  void SetSubtract() { ReleaseData(); m_eType = IT_SUBTRACT; }
  void SetMultiply() { ReleaseData(); m_eType = IT_MULTIPLY; }
  void SetDivide() { ReleaseData(); m_eType = IT_DIVIDE; }
  void SetPower() { ReleaseData(); m_eType = IT_POWER; }
  void SetAssign() { ReleaseData(); m_eType = IT_ASSIGN; }
  void SetResolveVar() { ReleaseData(); m_eType = IT_RESOLVE_VAR; }
  void SetResolveRef() { ReleaseData(); m_eType = IT_RESOLVE_REF; }
	void SetCall() { ReleaseData(); m_eType = IT_CALL; }

  EInterpretError Execute(CExecution *pExecution);
  int GetSize();

  EInterpretError ExecPushValue(CExecution *pExecution);
  EInterpretError ExecAdd(CExecution *pExecution);
  EInterpretError ExecSubtract(CExecution *pExecution);
  EInterpretError ExecMultiply(CExecution *pExecution);
  EInterpretError ExecDivide(CExecution *pExecution);
  EInterpretError ExecPower(CExecution *pExecution);
  EInterpretError ExecAssign(CExecution *pExecution);
  EInterpretError ExecResolveValue(CExecution *pExecution);
  EInterpretError ExecResolveRef(CExecution *pExecution);
	EInterpretError ExecCall(CExecution *pExecution);

  bool HasValue() const { return m_eType == IT_PUSH_VALUE; }
  CValue &GetValue() { return *(CValue *) &m_btValue; }

  CStrAny ToStr();

  CInstruction &operator =(CInstruction const &kInstr) { ReleaseData(); m_eType = kInstr.m_eType; if (HasValue()) GetValue().Set(const_cast<CInstruction *>(&kInstr)->GetValue()); return *this; }

  static CValue2String::TValueString s_arrIT2Str[IT_LAST];
  static CValue2String s_IT2Str;
};

class CExecution {
public:
  TValueStack   m_kStack;
  CEnvTable     m_kEnvironment;
  CFragment    *m_pCode;
  CInstruction *m_pNextInstruction;

  EInterpretError Execute(CFragment *pCode, CArray<CValue> &arrParams);
};

#endif