#ifndef __COMPILE_H
#define __COMPILE_H

#include "Grammar.h"
#include "Execution.h"

class CCompiler {
public:
  CFragment *m_pCode;

  CCompiler();
  ~CCompiler();

  void Clear();

  EInterpretError Compile(CGrammarParser::TOperatorNode *pNode);

  EInterpretError CompileExpression(CGrammarParser::TOperatorNode *pExpression);

  EInterpretError CompileNumber(CGrammarParser::TOperatorNode *pNode);
  EInterpretError CompileString(CGrammarParser::TOperatorNode *pNode);
  EInterpretError CompileVariable(CGrammarParser::TOperatorNode *pNode);
  EInterpretError CompileLValue(CGrammarParser::TOperatorNode *pNode);
  EInterpretError CompileOperator(CGrammarParser::TOperatorNode *pNode);
};

class CCompileChain {
public:
  CTokenizer     m_kTokenizer;
  CGrammarParser m_kGrammar;
  CCompiler      m_kCompiler;

  EInterpretError Compile(CStrAny sCode);
};

#endif