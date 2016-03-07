#include "stdafx.h"
#include "PostGrammar.h"

CGrammarTransform::CGrammarTransform() : m_pTransformed(0)
{
}

CGrammarTransform::~CGrammarTransform()
{
  Clear();
}

void CGrammarTransform::Clear()
{
  m_pTransformed = 0;
}

EInterpretError CGrammarTransform::Transform(CBNFGrammar::CNode *&pNode)
{
  EInterpretError err = TransformNode(pNode);
  m_pTransformed = err == IERR_OK ? pNode : 0;
  return err;
}

EInterpretError CGrammarTransform::TransformNode(CBNFGrammar::CNode *&pNode)
{
  EInterpretError err = IERR_OK;

  switch (pNode->m_pRule->m_iID) {
    case CBNFGrammar::RID_For:
      err = TransformFor(pNode);
      break;
  }
  if (err != IERR_OK)
    return err;

  for (int i = 0; i < pNode->m_arrChildren.m_iCount; ++i) {
    err = Transform(pNode->m_arrChildren[i]);
    if (err != IERR_OK)
      return err;
  }

  return IERR_OK;
}


EInterpretError CGrammarTransform::TransformFor(CBNFGrammar::CNode *&pNode)
{
  return IERR_OK;
}
