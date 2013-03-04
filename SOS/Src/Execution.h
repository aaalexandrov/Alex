#ifndef __EXECUTION_H
#define __EXECUTION_H

#include "Variable.h"
#include "Error.h"

class CExecution;
class CInstruction {
public:
  enum EType {
		IT_NOP,

    IT_MOVE_VALUE,
		IT_GET_GLOBAL,
		IT_SET_GLOBAL,
		IT_CREATE_TABLE,
		IT_GET_TABLE_VALUE,
		IT_SET_TABLE_VALUE,

    IT_CONCAT,
    IT_NEGATE,
		IT_ADD,
    IT_SUBTRACT,
    IT_MULTIPLY,
    IT_DIVIDE,
    IT_POWER,

		IT_COMPARE_EQ,
		IT_COMPARE_NOT_EQ,
		IT_COMPARE_LESS,
		IT_COMPARE_LESS_EQ,
		IT_NOT,

		IT_MOVE_AND_JUMP_IF_FALSE,
		IT_MOVE_AND_JUMP_IF_TRUE,
		IT_CALL,
		IT_RETURN,

    IT_LAST
  };

	static const short INVALID_OPERAND = (short) 0x8000;
public:
  EType m_eType;
	short m_nDest, m_nSrc0, m_nSrc1;

  CInstruction() { SetNop(); }
	CInstruction(CInstruction const &kInstr) { Set(kInstr); }
  ~CInstruction() {}

	void Set(CInstruction const &kInstr) { m_eType = kInstr.m_eType; m_nDest = kInstr.m_nDest; m_nSrc0 = kInstr.m_nSrc0; m_nSrc1 = kInstr.m_nSrc1; }
	void Set(EType eType, short nDest = INVALID_OPERAND, short nSrc0 = INVALID_OPERAND, short nSrc1 = INVALID_OPERAND) { m_eType = eType; m_nDest = nDest; m_nSrc0 = nSrc0; m_nSrc1 = nSrc1; }

  void SetNop() { Set(IT_NOP); }
  void SetMoveValue(short nDest, short nSrc0) { Set(IT_MOVE_VALUE, nDest, nSrc0); }
  void SetGetGlobal(short nDest, short nSrc0) { Set(IT_GET_GLOBAL, nDest, nSrc0); }
  void SetSetGlobal(short nDest, short nSrc0) { Set(IT_SET_GLOBAL, nDest, nSrc0); }
	void SetGetTableValue(short nDest, short nSrc0, short nSrc1) { Set(IT_GET_TABLE_VALUE, nDest, nSrc0, nSrc1); }
	void SetSetTableValue(short nDest, short nSrc0, short nSrc1) { Set(IT_SET_TABLE_VALUE, nDest, nSrc0, nSrc1); }
	void SetCreateTable(short nDest) { Set(IT_CREATE_TABLE, nDest); }
  void SetConcat(short nDest, short nSrc0, short nSrc1) { Set(IT_CONCAT, nDest, nSrc0, nSrc1); }
	void SetNegate(short nDest, short nSrc0) { Set(IT_NEGATE, nDest, nSrc0); }
  void SetAdd(short nDest, short nSrc0, short nSrc1) { Set(IT_ADD, nDest, nSrc0, nSrc1); }
  void SetSubtract(short nDest, short nSrc0, short nSrc1) { Set(IT_SUBTRACT, nDest, nSrc0, nSrc1); }
  void SetMultiply(short nDest, short nSrc0, short nSrc1) { Set(IT_MULTIPLY, nDest, nSrc0, nSrc1); }
  void SetDivide(short nDest, short nSrc0, short nSrc1) { Set(IT_DIVIDE, nDest, nSrc0, nSrc1); }
  void SetPower(short nDest, short nSrc0, short nSrc1) { Set(IT_POWER, nDest, nSrc0, nSrc1); }
  void SetCompareNotEq(short nDest, short nSrc0, short nSrc1) { Set(IT_COMPARE_NOT_EQ, nDest, nSrc0, nSrc1); }
	void SetCompareEq(short nDest, short nSrc0, short nSrc1) { Set(IT_COMPARE_EQ, nDest, nSrc0, nSrc1); }
	void SetCompareLess(short nDest, short nSrc0, short nSrc1) { Set(IT_COMPARE_LESS, nDest, nSrc0, nSrc1); }
	void SetCompareLessEq(short nDest, short nSrc0, short nSrc1) { Set(IT_COMPARE_LESS_EQ, nDest, nSrc0, nSrc1); }
	void SetNot(short nDest, short nSrc0) { Set(IT_NOT, nDest, nSrc0); }
	void SetMoveAndJumpIfFalse(short nDest, short nSrc0, short nSrc1) { Set(IT_MOVE_AND_JUMP_IF_FALSE, nDest, nSrc0, nSrc1); } 
	void SetMoveAndJumpIfTrue(short nDest, short nSrc0, short nSrc1) { Set(IT_MOVE_AND_JUMP_IF_TRUE, nDest, nSrc0, nSrc1); }
	void SetCall(short nDest, short nSrc0, short nSrc1) { Set(IT_CALL, nDest, nSrc0, nSrc1); }
	void SetReturn(short nDest, short nSrc0) { Set(IT_RETURN, nDest, nSrc0); }

	CValue *GetOperand(CExecution *pExecution, short nOperand) const;
	CValue *GetDest(CExecution *pExecution) const { ASSERT(m_nDest >= 0); return GetOperand(pExecution, m_nDest); }
	CValue *GetSrc0(CExecution *pExecution) const { return GetOperand(pExecution, m_nSrc0); }
	CValue *GetSrc1(CExecution *pExecution) const { return GetOperand(pExecution, m_nSrc1); }

  EInterpretError Execute(CExecution *pExecution) const;

  EInterpretError ExecNop(CExecution *pExecution) const;
  EInterpretError ExecMoveValue(CExecution *pExecution) const;
  EInterpretError ExecGetGlobal(CExecution *pExecution) const;
  EInterpretError ExecSetGlobal(CExecution *pExecution) const;
	EInterpretError ExecGetTableValue(CExecution *pExecution) const;
	EInterpretError ExecSetTableValue(CExecution *pExecution) const;
	EInterpretError ExecCreateTable(CExecution *pExecution) const;
  EInterpretError ExecConcat(CExecution *pExecution) const;
	EInterpretError ExecNegate(CExecution *pExecution) const;
  EInterpretError ExecAdd(CExecution *pExecution) const;
  EInterpretError ExecSubtract(CExecution *pExecution) const;
  EInterpretError ExecMultiply(CExecution *pExecution) const;
  EInterpretError ExecDivide(CExecution *pExecution) const;
  EInterpretError ExecPower(CExecution *pExecution) const;
	EInterpretError ExecCompareEq(CExecution *pExecution) const;
  EInterpretError ExecCompareNotEq(CExecution *pExecution) const;
  EInterpretError ExecCompareLess(CExecution *pExecution) const;
	EInterpretError ExecCompareLessEq(CExecution *pExecution) const;
	EInterpretError ExecNot(CExecution *pExecution) const;
	EInterpretError ExecMoveAndJumpIfFalse(CExecution *pExecution) const;
	EInterpretError ExecMoveAndJumpIfTrue(CExecution *pExecution) const;
	EInterpretError ExecCall(CExecution *pExecution) const;
	EInterpretError ExecReturn(CExecution *pExecution) const;

  CStrAny GetOperandStr(short nOperand) const;
	CStrAny GetOperandConstStr(CFragment *pFragment, short nOperand) const;
	CStrAny ToStr(CFragment *pFragment) const;

  CInstruction &operator =(CInstruction const &kInstr) { Set(kInstr); return *this; }

  static CValue2String::TValueString s_arrIT2Str[IT_LAST];
  static CValue2String s_IT2Str;
};

class CExecution {
public:
  CArray<CValue>          m_arrLocal;
	short                   m_nReturnBase, m_nReturnCount;
	CSmartPtr<CValueTable>  m_pGlobalEnvironment;
  CFragment              *m_pCode;
  CInstruction           *m_pNextInstruction;

	CExecution();
	~CExecution();

	EInterpretError Execute(CFragment *pCode, CArray<CValue> &arrParams, CValueTable *pGlobalEnvironment);
};

#endif