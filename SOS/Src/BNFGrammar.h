#ifndef __BNFGRAMMAR_H
#define __BNFGRAMMAR_H

#include "BNF.h"

class CBNFGrammar: public CBNFParser {
public:
	enum ERuleID {
		RID_Value = 1,
		RID_FunctionDef,
		RID_FunctionCall,
		RID_Operand,
		RID_Power,
		RID_Mult,
		RID_Sum,
		RID_Expression,
		RID_LValue,
		RID_Assignment,
		RID_Operator,
		RID_Program = -1
	};

public:
	CNode *m_pParsed;

	CBNFGrammar();
	~CBNFGrammar() { Clear(); }

	void InitRules();

	void Clear();

	bool Parse(CList<CToken *> &lstTokens);

	void Dump() { Dump(m_pParsed); }
	void Dump(CNode *pParsed, int iIndent = 0);
};

#endif