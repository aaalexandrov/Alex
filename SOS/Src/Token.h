#ifndef __TOKEN_H
#define __TOKEN_H

#include "Error.h"

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
		TT_FUNCTION,
		TT_END,
		TT_RETURN,
		TT_IF,
		TT_THEN,
		TT_ELSE,
		TT_WHILE,
		TT_DO,
		TT_TRUE,
		TT_FALSE,
		TT_NOT,
		TT_AND,
		TT_OR,

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
