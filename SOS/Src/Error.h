#ifndef __ERROR_H
#define __ERROR_H

// Errors ---------------------------------------------------------------------

enum EInterpretError {
	IERR_OK,
	IERR_UNKNOWN,

	IERR_INVALID_OPERAND,
  IERR_OPERAND_TYPE,
  IERR_INVALID_INSTRUCTION,

	IERR_UNKNOWN_TOKEN,
	IERR_PARSING_FAILED,

	IERR_COMPILE_FAILED,
	IERR_DUPLICATE_VARIABLE,
	IERR_TOO_MANY_INSTRUCTIONS,
};

class CValue2String {
public:
  struct TValueString {
    int     m_iVal;
    CStrAny m_sStr;
  };

public:
  TValueString *m_pTable;
  int m_iCount;

  CValue2String(TValueString *pTable, int iCount): m_pTable(pTable), m_iCount(iCount) {}
  CStrAny GetStr(int iVal);
};

#define VAL2STR(V) { V, CStrAny(ST_WHOLE, #V) }

extern CValue2String g_IERR2Str;

#endif