#ifndef __TOKEN_H
#define __TOKEN_H

#include "Error.h"

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

class CToken {
public:

  enum ETokenClass {
    TC_UNKNOWN,

    TC_IDENTIFIER,
    TC_KEYWORD,
    TC_LITERAL,
    TC_OPERATOR,
    TC_SEPARATOR,

    TC_LAST
  };

  enum ETokenType {
    TT_UNKNOWN,

    // Identifiers
    TT_VARIABLE,
    TT_KEYWORD,

    // Literals
    TT_NUMBER,
    TT_STRING,

    // Operators
    TT_PLUS, TT_OPERATOR_BASE = TT_PLUS,
    TT_MINUS,
    TT_MULTIPLY,
    TT_DIVIDE,
		TT_POWER,
		TT_NEGATE,
    TT_ASSIGN,

    // Separators
    TT_OPENBRACE, TT_OPERATOR_LAST = TT_OPENBRACE,
    TT_CLOSEBRACE,

    TT_LAST
  };

  static const int OPERATOR_NUM = TT_OPERATOR_LAST - TT_OPERATOR_BASE;

public:
  CStrAny     m_sToken;
  ETokenClass m_eClass;
  ETokenType  m_eType;

  CToken(CStrAny const &sToken, ETokenClass eClass, ETokenType eType);
  ~CToken();

  CStrAny ToString();
  static CStrAny ClassToString(ETokenClass eClass);
  static CStrAny TypeToString(ETokenType eType);

  static CValue2String::TValueString s_arrTC2Str[TC_LAST], s_arrTT2Str[TT_LAST];
  static CValue2String s_kTC2Str, s_kTT2Str;
};

class CTokenizer {
public:
  CStrAny         m_sInput;
  CList<CToken *> m_lstTokens;

  CTokenizer();
  ~CTokenizer();

	void Clear();

  EInterpretError Tokenize(CStrAny const &sInput);

  bool Valid() const;

  void Dump(CList<CToken *> *pList = 0);

public:
  static CToken  s_TemplateTokens[CToken::TT_LAST];

  static CToken const *GetTemplateToken(CStrAny const &sToken);
  static CToken const *GetTemplateToken(CToken::ETokenType eType);
};

#endif
