#ifndef __BNF_H
#define __BNF_H

#include "Token.h"

class CBNFParser {
public:
	enum ERepeat {
		R_One,
		R_ZeroOne,
		R_ZeroInfinity,
	};

	enum ESequence {
		S_Sequence,
		S_Alternative,
	};

	enum EOutput {
		O_Output,
		O_NoOutput,
	};

	static const int MAX_CHILD_RULES = 8;

	class CNode;

	class CRule {
	public:
		EOutput m_eOutput;
		bool m_bTopLevel;

		CRule(EOutput eOutput, bool bTopLevel): m_eOutput(eOutput), m_bTopLevel(bTopLevel) {}
		virtual ~CRule() {}

		virtual bool Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent, bool bOutput) const = 0;
	};

	class CTerminal: public CRule {
	public:
		CToken::ETokenType m_eToken;

		CTerminal(CToken::ETokenType eToken, EOutput eOutput = O_Output, bool bTopLevel = false): CRule(eOutput, bTopLevel), m_eToken(eToken) {}

		virtual bool Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent, bool bOutput) const;
	};

	class CNonTerminal: public CRule {
	public:
		ERepeat m_eRepeat;
		ESequence m_eSequence;
		CRule *m_pChildren[MAX_CHILD_RULES];

		CNonTerminal(CRule **pChildren, ERepeat eRepeat = R_One, EOutput eOutput = O_Output, bool bTopLevel = false);
		virtual ~CNonTerminal();

		virtual bool Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent, bool bOutput) const;
	};

	class CNode {
	public:
		CToken *m_pToken;
		CRule const *m_pRule;
		CArray<CNode *> m_arrChildren;

		CNode(CToken *pToken, CRule const *pRule): m_pToken(pToken), m_pRule(pRule) {}
		~CNode() { m_arrChildren.DeleteAll(); }
	};

public:
	CRule const *m_pRootRule;

	CBNFParser(): m_pRootRule(0) {}
	~CBNFParser() { delete m_pRootRule; }

	void SetRule(CRule const &kRootRule) { delete m_pRootRule; m_pRootRule = &kRootRule; }

	bool Parse(CList<CToken *> &lstTokens, CNode *&pParsed);
};

#endif