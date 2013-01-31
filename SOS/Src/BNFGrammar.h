#ifndef __BNFGRAMMAR_H
#define __BNFGRAMMAR_H

#include "BNF.h"

class CBNFGrammar: public CBNFParser {
public:
	enum ERuleID {
		RID_Value = 1,
		RID_Operand,
		RID_Power,
		RID_Mult,
		RID_Sum,
		RID_Expression,
		RID_LValue,
		RID_Assignment,
		RID_Program = -1
	};

public:
	CBNFGrammar();

	void InitRules();

	void Dump(CNode *pParsed, int iIndent = 0);
};

#endif