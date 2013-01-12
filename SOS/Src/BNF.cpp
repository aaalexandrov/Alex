#include "stdafx.h"
#include "BNF.h"

// CBNFParser::CTerminal ------------------------------------------------------

bool CBNFParser::CTerminal::Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent, bool bOutput) const
{
	if (!pFirstToken || pFirstToken->Data->m_eType != m_eToken)
		return false;
	if (bOutput && m_eOutput == O_Output) {
		CNode *pNode = new CNode(pFirstToken->Data, m_bTopLevel ? this : kParent.m_pRule);
		kParent.m_arrChildren.Append(pNode);
	} 
	pFirstToken = pFirstToken->pNext;
	return true;
}

// CBNFParser::CNonTerminal ---------------------------------------------------

CBNFParser::CNonTerminal::CNonTerminal(CRule **pChildren, ERepeat eRepeat, EOutput eOutput, bool bTopLevel)
	: CRule(eOutput, bTopLevel), m_eRepeat(eRepeat)
{
	for (int i = 0; i < MAX_CHILD_RULES; ++i) {
		m_pChildren[i] = pChildren[i];
		if (!pChildren[i])
			break;
	}
}

CBNFParser::CNonTerminal::~CNonTerminal()
{
	for (int i = 0; i < MAX_CHILD_RULES && m_pChildren[i]; ++i)
		delete m_pChildren[i];
}

bool CBNFParser::CNonTerminal::Match(CList<CToken *>::TNode *&pFirstToken, CNode &kParent, bool bOutput) const
{
	CNode *pNode;
	if (m_bTopLevel && pFirstToken) {
		pNode = new CNode(pFirstToken->Data, this);
		kParent.m_arrChildren.Append(pNode);
	} else
		pNode = &kParent;
	bOutput &= m_eOutput == O_Output;
	bool bMatchFound, bAnyMatch = false;
	do {
		CList<CToken *>::TNode *pPrevFirstToken = pFirstToken;
		int iPrevParsedCount = pNode->m_arrChildren.m_iCount;
		int i;
		for (i = 0; i < MAX_CHILD_RULES && m_pChildren[i]; ++i) {
			if (m_pChildren[i]->Match(pFirstToken, *pNode, bOutput)) {
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
		bMatchFound = i < MAX_CHILD_RULES && m_pChildren[i];
		if (m_eSequence == S_Sequence)
			bMatchFound = !bMatchFound;
		if (bMatchFound)
			bAnyMatch = true;
		else {
			pFirstToken = pPrevFirstToken;
			ASSERT(pNode->m_arrChildren.m_iCount == iPrevParsedCount);
		}
	} while (m_eRepeat == R_ZeroInfinity && bMatchFound);
	if (!bAnyMatch && pNode != &kParent) {
		ASSERT(kParent.m_arrChildren.Last() == pNode && !pNode->m_arrChildren.m_iCount);
		delete pNode;
		--kParent.m_arrChildren.m_iCount;
	}
	if (!bAnyMatch && (m_eRepeat == R_ZeroOne || m_eRepeat == R_ZeroInfinity))
		bAnyMatch = true;
	return bAnyMatch;
}

// CBNFParser -----------------------------------------------------------------

bool CBNFParser::Parse(CList<CToken *> &lstTokens, CNode *&pParsed)
{
	ASSERT(!pParsed && m_pRootRule && m_pRootRule->m_bTopLevel && m_pRootRule->m_eOutput == O_Output);
	CList<CToken *>::TNode *pFirst = lstTokens.m_pHead;
	pParsed = new CNode(pFirst ? pFirst->Data : 0, m_pRootRule);
	bool bMatch = m_pRootRule->Match(pFirst, *pParsed, true);
	if (!bMatch) {
		delete pParsed;
		pParsed = 0;
	}
	return bMatch;
}