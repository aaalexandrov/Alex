#ifndef __EXPRESSION_H
#define __EXPRESSION_H

#include "Token.h"

class CInterpreter;

class CExpression {
public:
	EInterpretError m_eStatus;
  CTokenizer      m_Tokenizer;
	CList<CToken *> m_lstPostfix;
	CList<CCmd *>   m_lstCommands;

  CExpression();
  ~CExpression();

  EInterpretError Init(const CStrAny &sExpression);
	void Clear();

	EInterpretError BuildCommands();

  bool Valid() const;

	EInterpretError Execute(TExecutionStack &kStack, CVarObj &kContext);

	EInterpretError BuildPostfix();
  EInterpretError Evaluate(CInterpreter *pInterpreter);

  void Dump(CList<CToken *> *pList = 0);
};

#endif
