#ifndef __COMPILER_H
#define __COMPILER_H

#include "BNFGrammar.h"
#include "Execution.h"

class CCompiler {
public:
  CSmartPtr<CFragment> m_pCode;

  CCompiler();
  ~CCompiler();

  void Clear();

  EInterpretError Compile(CBNFGrammar::CNode *pNode);

	EInterpretError CompileProgram(CBNFGrammar::CNode *pNode);
	EInterpretError CompileValue(CBNFGrammar::CNode *pNode);
	EInterpretError CompileVariable(CBNFGrammar::CNode *pNode);
	EInterpretError CompileFunctionDef(CBNFGrammar::CNode *pNode);
	EInterpretError CompileFunctionCall(CBNFGrammar::CNode *pNode);
	EInterpretError CompileReturn(CBNFGrammar::CNode *pNode);
	EInterpretError CompileOperand(CBNFGrammar::CNode *pNode);
	EInterpretError CompilePower(CBNFGrammar::CNode *pNode);
	EInterpretError CompileMult(CBNFGrammar::CNode *pNode);
	EInterpretError CompileSum(CBNFGrammar::CNode *pNode);
	EInterpretError CompileExpression(CBNFGrammar::CNode *pNode);
	EInterpretError CompileLValue(CBNFGrammar::CNode *pNode);
	EInterpretError CompileAssignment(CBNFGrammar::CNode *pNode);
	EInterpretError CompileIf(CBNFGrammar::CNode *pNode);
	EInterpretError CompileOperator(CBNFGrammar::CNode *pNode);

	EInterpretError CompileNode(CBNFGrammar::CNode *pNode);
};

class CCompileChain {
public:
  CTokenizer     m_kTokenizer;
  CBNFGrammar    m_kGrammar;
  CCompiler      m_kCompiler;

	void Clear();

  EInterpretError Compile(CStrAny sCode);
};

#endif