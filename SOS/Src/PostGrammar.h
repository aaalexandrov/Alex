#ifndef __POSTGRAMMAR_H
#define __POSTGRAMMAR_H

#include "BNFGrammar.h"
#include "Error.h"

class CGrammarTransform {
public:
  CBNFGrammar::CNode *m_pTransformed;

  CGrammarTransform();
  ~CGrammarTransform();

  void Clear();

  EInterpretError Transform(CBNFGrammar::CNode *&pNode);

  EInterpretError TransformNode(CBNFGrammar::CNode *&pNode);

  EInterpretError TransformFor(CBNFGrammar::CNode *&pNode);
};

#endif // __POSTGRAMMAR_H
