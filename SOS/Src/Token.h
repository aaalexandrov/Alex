#ifndef __TOKEN_H
#define __TOKEN_H

#include "Error.h"

class CToken {
public:

  enum ETokenType {
    TT_UNKNOWN,

    // Identifiers
    TT_VARIABLE,
		TT_LOCAL,
		TT_FUNCTION,
		TT_END,
		TT_RETURN,
		TT_IF,
		TT_THEN,
		TT_ELSE,
		TT_WHILE,
		TT_FOR,
		TT_DO,
		TT_TRUE,
		TT_FALSE,
		TT_NOT,
		TT_AND,
		TT_OR,
		TT_NIL,

    // Literals
    TT_NUMBER,
    TT_STRING,

    // Operators
		TT_EQUAL, TT_OPERATOR_BASE = TT_EQUAL,
		TT_NOT_EQUAL,
		TT_LESS_EQUAL,
		TT_GREAT_EQUAL,
		TT_LESS,
		TT_GREAT,
    TT_CONCAT,
    TT_PLUS,
    TT_MINUS,
    TT_MULTIPLY,
    TT_DIVIDE,
		TT_POWER,
		TT_NEGATE,
    TT_ASSIGN,

    // Separators
    TT_OPENBRACE, TT_OPERATOR_LAST = TT_OPENBRACE,
    TT_CLOSEBRACE,
    TT_OPENBRACKET,
    TT_CLOSEBRACKET,
    TT_OPENCURLY,
    TT_CLOSECURLY,
    TT_COMMA,
    TT_DOT,

    TT_LAST
  };

  static const int OPERATOR_NUM = TT_OPERATOR_LAST - TT_OPERATOR_BASE;

public:
  CStrAny     m_sToken;
  ETokenType  m_eType;

  CToken(CStrAny const &sToken, ETokenType eType);
  ~CToken();

  CStrAny ToString() const;
  static CStrAny TypeToString(ETokenType eType);

  static CValue2String::TValueString s_arrTT2Str[TT_LAST];
  static CValue2String s_kTT2Str;
};

class CTokenizer {
public:
  CStrAny         m_sInput;
  CList<CToken *> m_lstTokens;
  CList<CToken *> m_lstTemp;
  int             m_iTempIndex;

  CTokenizer();
  ~CTokenizer();

	void Clear();

  EInterpretError Tokenize(CStrAny const &sInput);

  CToken *ReadToken(CStrAny &sInp);

  bool Valid() const;

  CToken *GetTempToken(CStrAny sToken, CToken::ETokenType eTT = CToken::TT_UNKNOWN);
  CStrAny GetTempName(CStrAny sPrefix);
  CToken *GetTempVar(CStrAny sPrefix) { return GetTempToken(GetTempName(sPrefix), CToken::TT_VARIABLE); }

  void Dump(CList<CToken *> *pList = 0);

public:
  static CToken  s_TemplateTokens[CToken::TT_LAST];

  static CToken const *GetTemplateToken(CStrAny const &sToken, bool bMatchKeyword);
  static CToken const *GetTemplateToken(CToken::ETokenType eType);
};

#endif
