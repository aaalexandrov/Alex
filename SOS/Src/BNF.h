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
		O_NoOutput,
		O_NoRenaming,
		O_Output,
	};

	static const int MAX_CHILD_RULES = 8;

	class CNode;

	class CRule {
	public:
		EOutput m_eOutput;
		int m_iID;

		CRule(): m_eOutput(O_NoRenaming), m_iID(0) {}
		virtual ~CRule() {}

		static inline EOutput CombineOutput(EOutput eOutput1, EOutput eOutput2) { return Util::Min(eOutput1, eOutput2); }

		virtual void AddToHash(CHash<CRule const *> &hashRules) const { hashRules.AddUnique(this); }

		virtual CRule *Set(EOutput eOutput) { m_eOutput = eOutput; return this; }
		virtual CRule *SetID(int iID) { m_iID = iID; return this; }

		virtual CRule *Set(ERepeat eRepeat) { ASSERT(0); return this; }
		virtual CRule *Set(ESequence eSequence) { ASSERT(0); return this; }
		virtual CRule *AddChild(CRule const *pChild) { ASSERT(0); return this; }

		virtual bool Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent, EOutput eOutput) const = 0;
	};

	class CTerminal: public CRule {
	public:
		CToken::ETokenType m_eToken;

		CTerminal(CToken::ETokenType eToken): m_eToken(eToken) {}

		virtual bool Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent, EOutput eOutput) const;
	};

	class CNonTerminal: public CRule {
	public:
		ERepeat m_eRepeat;
		ESequence m_eSequence;
		CRule const *m_pChildren[MAX_CHILD_RULES];

		CNonTerminal();
		virtual ~CNonTerminal();

		virtual void AddToHash(CHash<CRule const *> &hashRules) const;

		virtual CRule *Set(ERepeat eRepeat) { m_eRepeat = eRepeat; return this; }
		virtual CRule *Set(ESequence eSequence) { m_eSequence = eSequence; return this; }
		virtual CRule *AddChild(CRule const *pChild);

		virtual bool Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent, EOutput eOutput) const;
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
	~CBNFParser() { DeleteRules(); }

	void DeleteRules();

	void SetRule(CRule const &kRootRule) { delete m_pRootRule; m_pRootRule = &kRootRule; }

	bool Parse(CList<CToken *> &lstTokens, CNode *&pParsed);

	static CRule *NewNT() { return new CNonTerminal(); }
	static CRule *NewT(CToken::ETokenType eToken) { return new CTerminal(eToken); }
};

#endif