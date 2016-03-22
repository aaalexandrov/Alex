#ifndef __BNF_H
#define __BNF_H

#include "Token.h"

class CBNFParser {
public:
	enum ESequence {
		S_Sequence,
		S_Alternative,
	};

	static const int MAX_CHILD_RULES = 8;

	class CNode;

	class CRule {
	public:
    bool m_bOutput;
		int m_iID;

		CRule(): m_bOutput(false), m_iID(0) {}
		virtual ~CRule() {}

		virtual void AddToHash(CHash<CRule const *> &hashRules) const { hashRules.AddUnique(this); }

		virtual CRule *SetOutput(bool bOutput) { m_bOutput = bOutput; return this; }
		virtual CRule *SetID(int iID) { m_iID = iID; return this; }

		virtual CRule *SetAllowRenaming(bool bAllowRenaming) { ASSERT(0); return this; }
		virtual CRule *Set(ESequence eSequence) { ASSERT(0); return this; }
		virtual CRule *AddChild(CRule const *pChild) { ASSERT(0); return this; }

		virtual bool Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent) const = 0;
	};

	class CNothing: public CRule {
  public:
    CNothing() {}

    virtual bool Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent) const;
  };

  class CTerminal: public CRule {
	public:
		CToken::ETokenType m_eToken;

		CTerminal(CToken::ETokenType eToken): m_eToken(eToken) {}

		virtual bool Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent) const;
	};

	class CNonTerminal: public CRule {
	public:
    bool m_bAllowRenaming;
		ESequence m_eSequence;
		CRule const *m_pChildren[MAX_CHILD_RULES];

		CNonTerminal();
		virtual ~CNonTerminal();

		virtual void AddToHash(CHash<CRule const *> &hashRules) const;

    virtual CRule *SetAllowRenaming(bool bAllowRenaming) { m_bAllowRenaming = bAllowRenaming; m_bOutput |= bAllowRenaming; return this; }
		virtual CRule *Set(ESequence eSequence) { m_eSequence = eSequence; return this; }
		virtual CRule *AddChild(CRule const *pChild);

		virtual bool Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent) const;
	};

	class CNode {
	public:
		CToken const *m_pToken;
		int m_iRuleID;
		CArray<CNode *> m_arrChildren;

		CNode(CToken const *pToken, int iRuleID): m_pToken(pToken), m_iRuleID(iRuleID) {}
		CNode(CNode const &kNode): m_pToken(kNode.m_pToken), m_iRuleID(kNode.m_iRuleID), m_arrChildren(Max(1, kNode.m_arrChildren.m_iCount)) { AppendChildren(kNode.m_arrChildren); }
		~CNode() { m_arrChildren.DeleteAll(); }

		void AppendChildren(CArray<CNode *> const &kChildren);

		CNode *AddChild(CNode *pChild) { m_arrChildren.Append(pChild); return this; }
	};

public:
	CRule const *m_pRootRule;

	CBNFParser(): m_pRootRule(0) {}
	~CBNFParser() { DeleteRules(); }

	void DeleteRules();

	void SetRule(CRule const &kRootRule) { DeleteRules(); m_pRootRule = &kRootRule; }

	bool Parse(CList<CToken *> &lstTokens, CNode *&pParsed);

	static CRule *NewNT()                         { return NEW(CNonTerminal, ()); }
	static CRule *NewT(CToken::ETokenType eToken) { return NEW(CTerminal, (eToken)); }
  static CRule *NewEmpty()                      { return NEW(CNothing, ()); }
  static CRule *NewOptional(CRule *pContent);
  static CRule *NewZeroPlus(CRule *pContent);
  static CRule *NewOnePlus(CRule *pContent);
};

#endif
