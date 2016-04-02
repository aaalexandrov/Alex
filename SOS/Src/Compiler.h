#ifndef __COMPILER_H
#define __COMPILER_H

#include "BNFGrammar.h"
#include "PostGrammar.h"
#include "Execution.h"

class CCompiler {
public:
	struct TLocalInfo {
		short m_nIndex, m_nContext;

		TLocalInfo(short nIndex, short nContext): m_nIndex(nIndex), m_nContext(nContext) {}
	};

	class CLocalTracker {
	public:
		typedef CHashKV<CStrAny, TLocalInfo, CStrAny, CStrAny> THash;

		CArray<int> m_arrLocals;
		int m_iLocals, m_iVars;
		THash m_hashLocals;
		short m_nCurContext;

		CLocalTracker(): m_nCurContext(0), m_iLocals(0), m_iVars(0) {}

		void Clear();

		short GetFreeLocal(short nCount = 1);
		short AllocLocal(short nCount = 1);
		void ReleaseLocal(short nBase, short nCount = 1);
		void LockTemporary(short nIndex);
		void ReleaseTemporary(short nIndex);

		bool RegisterVar(CStrAny sVar, short nIndex);
    THash::TIter FindVar(CStrAny sVar);
		short AllocVar(CStrAny sVar);
		short GetVar(CStrAny sVar);

		void StartContext();
		void EndContext();
	};
public:
	typedef CHashKV<CValue, short, CValue, CValue>  TConstantHash;

  CInterpreter *m_pInterpreter;
  CClosure *m_pClosure;
	TConstantHash m_hashConst;
	CLocalTracker m_kLocals;
	CCompiler *m_pParent;

  CCompiler(CCompiler *pParent);
  ~CCompiler();

  CFragment *GetCode() { return m_pClosure->m_pFragment; }

  void Clear();

	short GetConstantIndex(CValue const &kValue);
	short GetOrCaptureVar(CStrAny sVar);
	void SortCaptures();
	void UpdateLocalNumber();

	void CompilationFinished();

  EInterpretError Compile(CInterpreter *pInterpreter, CBNFGrammar::CNode *pNode);

	EInterpretError CompileProgram(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileConstant(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileVariable(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileFunctionDef(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileFunctionCall(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileOperand(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileDotIndex(CBNFGrammar::CNode *pNode, short &nDest);
  EInterpretError CompileTable(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileReturn(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompilePower(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileMult(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileSum(CBNFGrammar::CNode *pNode, short &nDest);
  EInterpretError CompileConcat(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileComparison(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileNot(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileAnd(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileOr(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileLocals(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileLValue(CBNFGrammar::CNode *pNode, short &nValue, short &nTable, bool &bGlobal);
	EInterpretError CompileAssignment(CBNFGrammar::CNode *pNode, short &nDest);
	EInterpretError CompileIf(CBNFGrammar::CNode *pNode, short &nDest);
  EInterpretError CompileWhile(CBNFGrammar::CNode *pNode, short &nDest);
  EInterpretError CompileDo(CBNFGrammar::CNode *pNode, short &nDest);

	EInterpretError CompileNode(CBNFGrammar::CNode *pNode, short &nDest);

	EInterpretError CompileConstResult(CValue const &kValue, short &nDest);
};

class CCompileChain {
public:
  CTokenizer        m_kTokenizer;
  CBNFGrammar       m_kGrammar;
  CGrammarTransform m_kGrammarTransform;
  CCompiler         m_kCompiler;

  CCompileChain(): m_kGrammarTransform(&m_kTokenizer), m_kCompiler(0) {}

	void Clear();

  EInterpretError Compile(CInterpreter *pInterpreter, CStrAny sCode);
};

#endif
