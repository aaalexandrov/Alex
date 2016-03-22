#include "stdafx.h"
#include "PostGrammar.h"

CGrammarTransform::CGrammarTransform(CTokenizer *pTokenizer) : m_pTransformed(0), m_pTokenizer(pTokenizer)
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

  switch (pNode->m_iRuleID) {
    case CBNFGrammar::RID_For:
      err = TransformFor(pNode);
      break;
  }
  if (err != IERR_OK)
    return err;

  for (int i = 0; i < pNode->m_arrChildren.m_iCount; ++i) {
    err = TransformNode(pNode->m_arrChildren[i]);
    if (err != IERR_OK)
      return err;
  }

  return IERR_OK;
}

EInterpretError CGrammarTransform::TransformFor(CBNFGrammar::CNode *&pNode)
{
  ASSERT(pNode->m_iRuleID == CBNFGrammar::RID_For);
  ASSERT(pNode->m_arrChildren[1]->m_pToken->m_eType == CToken::TT_ASSIGN || pNode->m_arrChildren[2]->m_pToken->m_eType == CToken::TT_ASSIGN);

  if (pNode->m_arrChildren[1]->m_pToken->m_eType == CToken::TT_ASSIGN) { // for i = start, end [, step] do ... end
    // rewrite that as
    // do
    //   local st# = step
    //   local sgn# = st# < 0 and 1 or -1
    //   local i, e# = start, end * sgn#
    //   while sgn# * i <= e# do
    //     ...
    //     i = i+st#
    //   end
    // end
    CBNFParser::CNode *pDo = NewTemplate(CToken::TT_DO, CBNFGrammar::RID_Do);
    CBNFParser::CNode *pLocalStep = NewTemplate(CToken::TT_LOCAL, CBNFGrammar::RID_Locals);
    CBNFParser::CNode *pLocalSign = NewTemplate(CToken::TT_LOCAL, CBNFGrammar::RID_Locals);
    CBNFParser::CNode *pLocals = NewTemplate(CToken::TT_LOCAL, CBNFGrammar::RID_Locals);
    CBNFParser::CNode *pWhile = NewTemplate(CToken::TT_WHILE, CBNFGrammar::RID_While);

    CToken const *pVarToken = pNode->m_arrChildren[0]->m_pToken;
    pNode->m_arrChildren[0]->m_iRuleID = CBNFGrammar::RID_Do;
    CToken *pEndToken = m_pTokenizer->GetTempVar(CStrAny(ST_STR, "endval"));
    CToken *pStepToken = m_pTokenizer->GetTempVar(CStrAny(ST_STR, "step"));
    CToken *pSignToken = m_pTokenizer->GetTempVar(CStrAny(ST_STR, "sign"));

    CBNFParser::CNode *pStepValue, *pOperators;
    if (pNode->m_arrChildren.m_iCount >= 6) {
      pStepValue = pNode->m_arrChildren[4];
      pOperators = pNode->m_arrChildren[5];
    } else {
      pStepValue = NewConstant(CStrAny(ST_STR, "1"), CToken::TT_NUMBER);
      pOperators = pNode->m_arrChildren[4];
    }
    pOperators->m_iRuleID = CBNFGrammar::RID_While;

    pLocalStep->
      AddChild(NewNode(0, CBNFGrammar::RID_Locals)->
        AddChild(NewNode(pStepToken, CBNFGrammar::RID_Locals)))->
      AddChild(NewNode(0, CBNFGrammar::RID_Locals)->
        AddChild(pStepValue));

    pLocalSign->
      AddChild(NewNode(0, CBNFGrammar::RID_Locals)->
        AddChild(NewNode(pSignToken, CBNFGrammar::RID_Locals)))->
      AddChild(NewNode(0, CBNFGrammar::RID_Locals)->
        AddChild(NewNode(0, CBNFGrammar::RID_Or)->
          AddChild(NewNode(0, CBNFGrammar::RID_And)->
            AddChild(NewNode(0, CBNFGrammar::RID_Comparison)->
              AddChild(NewNode(pStepToken, CBNFGrammar::RID_Variable))->
              AddChild(NewTemplate(CToken::TT_LESS, CBNFGrammar::RID_Comparison))->
              AddChild(NewConstant(CStrAny(ST_STR, "0"), CToken::TT_NUMBER)))->
            AddChild(NewNode(0, CBNFGrammar::RID_Sum)->
              AddChild(NewTemplate(CToken::TT_MINUS, CBNFGrammar::RID_Sum))->
              AddChild(NewConstant(CStrAny(ST_STR, "1"), CToken::TT_NUMBER))))->
          AddChild(NewConstant(CStrAny(ST_STR, "1"), CToken::TT_NUMBER))));

    pLocals->
      AddChild(NewNode(0, CBNFGrammar::RID_Locals)->
        AddChild(pNode->m_arrChildren[0])->
        AddChild(NewNode(pEndToken, CBNFGrammar::RID_Locals)))->
      AddChild(NewNode(0, CBNFGrammar::RID_Locals)->
        AddChild(pNode->m_arrChildren[2])->
        AddChild(NewNode(0, CBNFGrammar::RID_Mult)->
          AddChild(pNode->m_arrChildren[3])->
          AddChild(NewTemplate(CToken::TT_MULTIPLY, CBNFGrammar::RID_Mult))->
          AddChild(NewNode(pSignToken, CBNFGrammar::RID_Variable))));

    pWhile->
      AddChild(NewNode(0, CBNFGrammar::RID_Comparison)->  // Condition
        AddChild(NewNode(0, CBNFGrammar::RID_Mult)->
          AddChild(NewNode(pVarToken, CBNFGrammar::RID_Variable))->
          AddChild(NewTemplate(CToken::TT_MULTIPLY, CBNFGrammar::RID_Mult))->
          AddChild(NewNode(pSignToken, CBNFGrammar::RID_Variable)))->
        AddChild(NewTemplate(CToken::TT_LESS_EQUAL, CBNFGrammar::RID_Comparison))->
        AddChild(NewNode(pEndToken, CBNFGrammar::RID_Variable)))->
      AddChild(pOperators-> // Do
        AddChild(NewNode(0, CBNFGrammar::RID_Assignment)->
          AddChild(NewNode(0, CBNFGrammar::RID_Assignment)->
            AddChild(NewNode(pVarToken, CBNFGrammar::RID_LValue)))->
          AddChild(NewNode(0, CBNFGrammar::RID_Assignment)->
            AddChild(NewNode(0, CBNFGrammar::RID_Sum)->
              AddChild(NewNode(pVarToken, CBNFGrammar::RID_Variable))->
              AddChild(NewTemplate(CToken::TT_PLUS, CBNFGrammar::RID_Sum))->
              AddChild(NewNode(pStepToken, CBNFGrammar::RID_Variable))))));

    pDo->
      AddChild(pLocalStep)->
      AddChild(pLocalSign)->
      AddChild(pLocals)->
      AddChild(pWhile);

    DEL(pNode->m_arrChildren[1]);
    pNode->m_arrChildren.Clear();
    DEL(pNode);
    pNode = pDo;
  } else { // for k,v = table do ... end
    // rewrite as
    // do
    //   local t# = table
    //   local k,v = next(t#)
    //   while k do
    //     ...
    //     k,v = next(t#, k)
    //   end
    // end

    CToken const *pKeyToken = pNode->m_arrChildren[0]->m_pToken;
    CToken const *pValToken = pNode->m_arrChildren[1]->m_pToken;
    pNode->m_arrChildren[0]->m_iRuleID = CBNFGrammar::RID_Locals;
    pNode->m_arrChildren[1]->m_iRuleID = CBNFGrammar::RID_Locals;
    pNode->m_arrChildren[4]->m_iRuleID = CBNFGrammar::RID_While;
    CToken *pTableToken = m_pTokenizer->GetTempVar(CStrAny(ST_STR, "table"));
    CToken *pNextToken = m_pTokenizer->GetTempToken(CStrAny(ST_STR, "next"), CToken::TT_VARIABLE);

    CBNFParser::CNode *pDo = NewTemplate(CToken::TT_DO, CBNFGrammar::RID_Do);

    pDo->
      AddChild(NewTemplate(CToken::TT_LOCAL, CBNFGrammar::RID_Locals)->
        AddChild(NewNode(0, CBNFGrammar::RID_Locals)->
          AddChild(NewNode(pTableToken, CBNFGrammar::RID_Variable)))->
        AddChild(NewNode(0, CBNFGrammar::RID_Locals)->
          AddChild(pNode->m_arrChildren[3])))->
      AddChild(NewTemplate(CToken::TT_LOCAL, CBNFGrammar::RID_Locals)->
        AddChild(NewNode(0, CBNFGrammar::RID_Locals)->
          AddChild(pNode->m_arrChildren[0])->
          AddChild(pNode->m_arrChildren[1]))->
        AddChild(NewNode(0, CBNFGrammar::RID_Locals)->
          AddChild(NewNode(0, CBNFGrammar::RID_FunctionCall)->
            AddChild(NewNode(pNextToken, CBNFGrammar::RID_Variable))->
            AddChild(NewNode(0, CBNFGrammar::RID_ParamList)->
              AddChild(NewNode(pTableToken, CBNFGrammar::RID_Variable))))))->
      AddChild(NewTemplate(CToken::TT_WHILE, CBNFGrammar::RID_While)->
        AddChild(NewNode(pKeyToken, CBNFGrammar::RID_Variable))->
        AddChild(pNode->m_arrChildren[4]->
          AddChild(NewNode(0, CBNFGrammar::RID_Assignment)->
            AddChild(NewNode(0, CBNFGrammar::RID_Assignment)->
              AddChild(NewNode(pKeyToken, CBNFGrammar::RID_LValue))->
              AddChild(NewNode(pValToken, CBNFGrammar::RID_LValue)))->
            AddChild(NewNode(0, CBNFGrammar::RID_Assignment)->
              AddChild(NewNode(0, CBNFGrammar::RID_FunctionCall)->
                AddChild(NewNode(pNextToken, CBNFGrammar::RID_Variable))->
                AddChild(NewNode(0, CBNFGrammar::RID_ParamList)->
                  AddChild(NewNode(pTableToken, CBNFGrammar::RID_Variable))->
                  AddChild(NewNode(pKeyToken, CBNFGrammar::RID_Variable))))))));

    DEL(pNode->m_arrChildren[2]);
    pNode->m_arrChildren.Clear();
    DEL(pNode);
    pNode = pDo;
  }
  return IERR_OK;
}
