#ifndef __BNFGRAMMAR_H
#define __BNFGRAMMAR_H

#include "BNF.h"

class CBNFGrammar: public CBNFParser {
public:
	enum ERuleID {
		RID_Program = -1,
		RID_Value = 1,
		RID_Variable,
		RID_FunctionDef,
		RID_FunctionCall,
		RID_Return,
		RID_Operand,
		RID_Power,
		RID_Mult,
		RID_Sum,
		RID_Expression,
		RID_LValue,
		RID_Assignment,
		RID_If,
		RID_Operator,
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

	static CValue2String::TValueString s_arrRID2Str[];
	static CValue2String s_RID2Str;
};

#endif