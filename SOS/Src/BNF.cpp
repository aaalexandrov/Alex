#include "stdafx.h"
#include "BNF.h"

// CBNFParser::CNothing -------------------------------------------------------

bool CBNFParser::CNothing::Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent) const
{
  if (m_bOutput) {
    CNode *pNode = new CNode(pFirstToken ? pFirstToken->Data : 0, m_iID > 0 ? this : kParent.m_pRule);
    kParent.m_arrChildren.Append(pNode);
  }
  return true;
}

// CBNFParser::CTerminal ------------------------------------------------------

bool CBNFParser::CTerminal::Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent) const
{
	if (!pFirstToken || pFirstToken->Data->m_eType != m_eToken)
		return false;
	if (m_bOutput) {
		CNode *pNode = new CNode(pFirstToken->Data, m_iID > 0 ? this : kParent.m_pRule);
		kParent.m_arrChildren.Append(pNode);
	} 
	pFirstToken = pFirstToken->pNext;
	return true;
}

// CBNFParser::CNonTerminal ---------------------------------------------------

CBNFParser::CNonTerminal::CNonTerminal()
	: m_bAllowRenaming(false), m_eSequence(S_Sequence)
{
	for (int i = 0; i < MAX_CHILD_RULES; ++i)
	  m_pChildren[i] = 0;
}

CBNFParser::CNonTerminal::~CNonTerminal()
{
/*
	for (int i = 0; i < MAX_CHILD_RULES && m_pChildren[i]; ++i)
		delete m_pChildren[i];
*/
}

void CBNFParser::CNonTerminal::AddToHash(CHash<CRule const *> &hashRules) const 
{ 
	CHash<CRule const *>::TIter it = hashRules.Find(this);
	if (!it) {
		hashRules.Add(this); 
		for (int i = 0; i < MAX_CHILD_RULES && m_pChildren[i]; ++i)
			m_pChildren[i]->AddToHash(hashRules);
	}
}

CBNFParser::CRule *CBNFParser::CNonTerminal::AddChild(CRule const *pChild)
{
	ASSERT(pChild);
	for (int i = 0; i < MAX_CHILD_RULES; ++i) {
		if (!m_pChildren[i]) {
			m_pChildren[i] = pChild;
			return this;
		}
	}
	ASSERT(!"Trying to add more than maximum allowed child rules in a non-terminal");
	return this;
}

bool CBNFParser::CNonTerminal::Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent) const
{
	CNode *pNode;
	if (m_bOutput) {
		pNode = new CNode(pFirstToken ? pFirstToken->Data : 0, m_iID > 0 ? this : kParent.m_pRule);
		kParent.m_arrChildren.Append(pNode);
	} else
		pNode = &kParent;

  CList<CToken *>::TNode *pPrevFirstToken = pFirstToken;
	int iPrevParsedCount = pNode->m_arrChildren.m_iCount;
	int i;
	for (i = 0; i < MAX_CHILD_RULES && m_pChildren[i]; ++i) {
		if (m_pChildren[i]->Match(pFirstToken, *pNode)) {
			if (m_eSequence == S_Alternative)
				break;
		} else {
			if (m_eSequence == S_Sequence) {
				while (pNode->m_arrChildren.m_iCount > iPrevParsedCount) {
					delete pNode->m_arrChildren.Last();
					--pNode->m_arrChildren.m_iCount;
				}
				break;
			}
		}
	}
	bool bMatchFound = i < MAX_CHILD_RULES && m_pChildren[i];
	if (m_eSequence == S_Sequence)
		bMatchFound = !bMatchFound;
	if (!bMatchFound)	{
		pFirstToken = pPrevFirstToken;
		ASSERT(pNode->m_arrChildren.m_iCount == iPrevParsedCount);
	}

  if (pNode != &kParent) {
		if (!bMatchFound) {
			ASSERT(kParent.m_arrChildren.Last() == pNode && !pNode->m_arrChildren.m_iCount);
			delete pNode;
			--kParent.m_arrChildren.m_iCount;
		} else {
      if (!m_bAllowRenaming && pNode->m_arrChildren.m_iCount == 1) {
  			ASSERT(kParent.m_arrChildren.Last() == pNode);
				kParent.m_arrChildren.Last() = pNode->m_arrChildren[0];
				pNode->m_arrChildren.SetCount(0);
				delete pNode;
			}
		}
	}

	return bMatchFound;
}

// CBNFParser -----------------------------------------------------------------

void CBNFParser::DeleteRules()
{
	if (!m_pRootRule)
		return;
	CHash<CRule const *> hashRules;
	m_pRootRule->AddToHash(hashRules);
	hashRules.DeleteAll();
	m_pRootRule = 0;
}

bool CBNFParser::Parse(CList<CToken *> &lstTokens, CNode *&pParsed)
{
	ASSERT(m_pRootRule);
	pParsed = 0;
	CList<CToken *>::TNode *pFirst = lstTokens.m_pHead;
	pParsed = new CNode(pFirst ? pFirst->Data : 0, m_pRootRule);
	bool bMatch = m_pRootRule->Match(pFirst, *pParsed);
	if (!bMatch || pFirst) {
		delete pParsed;
		pParsed = 0;
		bMatch = false;
	}
	return bMatch;
}

CBNFParser::CRule *CBNFParser::NewOptional(CRule *pContent)
{ 
  return NewNT()->Set(S_Alternative)->
    AddChild(pContent)->
    AddChild(NewEmpty()->SetOutput(pContent->m_bOutput));
}

CBNFParser::CRule *CBNFParser::NewZeroPlus(CRule *pContent)
{
  CRule *pRule = NewNT();
  return pRule->Set(S_Alternative)->
    AddChild(NewNT()->
      AddChild(pContent)->
      AddChild(pRule))->
    AddChild(NewEmpty());
}

CBNFParser::CRule *CBNFParser::NewOnePlus(CRule *pContent)
{
  CRule *pRule = NewNT(); 
  return pRule->
    AddChild(pContent)->
    AddChild(NewNT()->Set(S_Alternative)->
      AddChild(pRule)->
      AddChild(NewEmpty()));
}