#ifndef __POSTGRAMMAR_H
#define __POSTGRAMMAR_H

#include "BNFGrammar.h"
#include "Error.h"
#include "Token.h"

class CGrammarTransform {
public:
  CBNFGrammar::CNode *m_pTransformed;
  CTokenizer *m_pTokenizer;

  CGrammarTransform(CTokenizer *pTokenizer);
  ~CGrammarTransform();

  void Clear();

  EInterpretError Transform(CBNFGrammar::CNode *&pNode);

  EInterpretError TransformNode(CBNFGrammar::CNode *&pNode);

  EInterpretError TransformFor(CBNFGrammar::CNode *&pNode);

  CBNFParser::CNode *NewNode(CToken const *pToken, CBNFGrammar::ERuleID eRID) { return NEW(CBNFParser::CNode, (pToken, eRID)); }
  CBNFParser::CNode *NewTemplate(CToken::ETokenType eTT, CBNFGrammar::ERuleID eRID) { return NewNode(CTokenizer::GetTemplateToken(eTT), eRID); }
  CBNFParser::CNode *NewConstant(CStrAny sToken, CToken::ETokenType eTT) { return NewNode(m_pTokenizer->GetTempToken(sToken, eTT), CBNFGrammar::RID_Constant); }
};

#endif // __POSTGRAMMAR_H
