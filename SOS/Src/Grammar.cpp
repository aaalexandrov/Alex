#include "stdafx.h"
#include "Grammar.h"
#include "Token.h"

CValue2String::TValueString CGrammarParser::s_arrNT2Str[NT_LAST] = {
  VAL2STR(NT_UNKNOWN),
  VAL2STR(NT_OPERAND),
  VAL2STR(NT_NEGATION),
  VAL2STR(NT_POWER),
  VAL2STR(NT_MULTIPLICATION),
  VAL2STR(NT_SUM),
  VAL2STR(NT_LVALUE),
  VAL2STR(NT_ASSIGN),
  VAL2STR(NT_OPERATORLIST),
  VAL2STR(NT_BLOCK), 
  VAL2STR(NT_VARLIST),
  VAL2STR(NT_ARGLIST),
  VAL2STR(NT_FRAGMENT),
};

CValue2String CGrammarParser::s_kNT2Str(s_arrNT2Str, ARRSIZE(s_arrNT2Str));

CGrammarParser::CGrammarParser(): m_pLstTokens(0), m_pParseTree(0) 
{
}

CGrammarParser::~CGrammarParser()
{
  Clear();
}

void CGrammarParser::Clear()
{
  m_pLstTokens = 0;
  SAFE_DELETE(m_pParseTree);
  m_lstErrors.Clear();
}

EInterpretError CGrammarParser::Parse(CList<CToken *> *pLstTokens)
{
  Clear();
  m_pLstTokens = pLstTokens;
  m_pParseTree = ParseExpression(m_pLstTokens->m_pHead);
  if (!m_pParseTree)
    return IERR_PARSING_FAILED;
  return IERR_OK;
}

void CGrammarParser::DumpParseTree(TOperatorNode *pOpNode, int iIndent)
{
  if (!pOpNode)
    pOpNode = m_pParseTree;
  CStrAny sToken = pOpNode->m_pToken->ToString();
  CStrAny sIndent(ST_STR, ' ', iIndent);
  printf("%s[%s Node: %s", sIndent.m_pBuf, sToken.m_pBuf, s_kNT2Str.GetStr(pOpNode->m_eType).m_pBuf);
  bool bNewLineAdded = false;
  for (int i = 0; i < MAX_OPERANDS; i++) {
    if (!pOpNode->m_pOperand[i])
      break;
    if (!bNewLineAdded) {
      printf("\n");
      bNewLineAdded = true;
    }
    DumpParseTree(pOpNode->m_pOperand[i], iIndent + 2);
  }
  printf("%s]\n", bNewLineAdded ? sIndent.m_pBuf : "");
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseOperand(CList<CToken *>::TNode *&pFirstToken)
{
  TOperatorNode *pOpNode;
  if (!pFirstToken)
    return 0;
  if (pFirstToken->Data->m_eType == CToken::TT_VARIABLE || pFirstToken->Data->m_eClass == CToken::TC_LITERAL) {
    // Operand ::= Variable
    // Operand ::= Literal
    pOpNode = new TOperatorNode(pFirstToken->Data, NT_OPERAND);
    pFirstToken = pFirstToken->pNext;
    return pOpNode;
  }
  if (pFirstToken->Data->m_eType == CToken::TT_OPENBRACE) {
    // Operand ::= ( Expression )
    CList<CToken *>::TNode *pToken = pFirstToken->pNext;
    pOpNode = ParseExpression(pToken);
    if (pOpNode) {
      if (pToken->Data->m_eType == CToken::TT_CLOSEBRACE) {
        pFirstToken = pToken->pNext;
        return pOpNode;
      } else
        delete pOpNode;
    }
    return 0;
  }
  // Operand ::= Negation
  pOpNode = ParseNegation(pFirstToken);
  return pOpNode;
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseNegation(CList<CToken *>::TNode *&pFirstToken)
{
  TOperatorNode *pOpNode, *pNegNode;
  if (!pFirstToken)
    return 0;
  if (pFirstToken->Data->m_eType == CToken::TT_MINUS || pFirstToken->Data->m_eType == CToken::TT_NEGATE) {
    // Negation ::= - Operand
    CList<CToken *>::TNode *pMinusToken = pFirstToken;
    pFirstToken = pFirstToken->pNext;
    pOpNode = ParseOperand(pFirstToken);
    if (pOpNode) {
      pNegNode = new TOperatorNode(pMinusToken->Data, NT_NEGATION);
      pNegNode->m_pOperand[0] = pOpNode;
      return pNegNode;
    } else 
      pFirstToken = pMinusToken;
  }
  return 0;
}

CGrammarParser::TOperatorNode *CGrammarParser::ParsePower(CList<CToken *>::TNode *&pFirstToken)
{
  TOperatorNode *pOpNode, *pPowerNode, *pRootNode;
  CList<CToken *>::TNode *pPowerToken;
  if (!pFirstToken)
    return 0;
  pOpNode = ParseOperand(pFirstToken);
  if (!pOpNode)
    return 0;
  if (!pFirstToken || pFirstToken->Data->m_eType != CToken::TT_POWER) {
    // Power ::= Operand
    return pOpNode;
  }
  pPowerToken = pFirstToken;
  pFirstToken = pFirstToken->pNext;
  pPowerNode = ParsePower(pFirstToken);
  if (pPowerNode) {
    // Power ::= Operand ^ Power
    pRootNode = new TOperatorNode(pPowerToken->Data, NT_POWER);
    pRootNode->m_pOperand[0] = pOpNode;
    pRootNode->m_pOperand[1] = pPowerNode;
    return pRootNode;
  }
  pFirstToken = pPowerToken;
  return pOpNode;
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseMultiplication(CList<CToken *>::TNode *&pFirstToken)
{
  TOperatorNode *pOpNode;
  if (!pFirstToken)
    return 0;
  pOpNode = ParsePower(pFirstToken);
  if (!pOpNode)
    return 0;
  // Multiplication ::= Power MultSuffix
  return ParseMultSuffix(pFirstToken, pOpNode);
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseMultSuffix(CList<CToken *>::TNode *&pFirstToken, TOperatorNode *pLeftOp)
{
  TOperatorNode *pPowerNode, *pRootNode;
  CToken *pOperator;
  ASSERT(pLeftOp);
  if (!pFirstToken || pFirstToken->Data->m_eType != CToken::TT_MULTIPLY && pFirstToken->Data->m_eType != CToken::TT_DIVIDE) {
    // MultSuffix ::= Nothing
    return pLeftOp;
  }
  pOperator = pFirstToken->Data;
  CList<CToken *>::TNode *pToken = pFirstToken->pNext;
  pPowerNode = ParsePower(pToken);
  if (!pPowerNode) {
    // MultSuffix ::= Nothing
    return pLeftOp;
  }
  // MultSuffix ::= * Power MultSuffix
  pFirstToken = pToken;
  pRootNode = new TOperatorNode(pOperator, NT_MULTIPLICATION);
  pRootNode->m_pOperand[0] = pLeftOp;
  pRootNode->m_pOperand[1] = pPowerNode;
  return ParseMultSuffix(pFirstToken, pRootNode);
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseSum(CList<CToken *>::TNode *&pFirstToken)
{
  TOperatorNode *pOpNode;
  if (!pFirstToken)
    return 0;
  pOpNode = ParseMultiplication(pFirstToken);
  if (!pOpNode)
    return 0;
  // Sum ::= Multiplication SumSuffix
  return ParseSumSuffix(pFirstToken, pOpNode);
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseSumSuffix(CList<CToken *>::TNode *&pFirstToken, TOperatorNode *pLeftOp)
{
  TOperatorNode *pMultNode, *pRootNode;
  CToken *pOperator;
  ASSERT(pLeftOp);
  if (!pFirstToken || pFirstToken->Data->m_eType != CToken::TT_PLUS && pFirstToken->Data->m_eType != CToken::TT_MINUS) {
    // SumSuffix ::= Nothing
    return pLeftOp;
  }
  pOperator = pFirstToken->Data;
  CList<CToken *>::TNode *pToken = pFirstToken->pNext;
  pMultNode = ParseMultiplication(pToken);
  if (!pMultNode) {
    // SumSuffix ::= Nothing
    return pLeftOp;
  }
  // SumSuffix ::= + Multiplication SumSuffix
  pFirstToken = pToken;
  pRootNode = new TOperatorNode(pOperator, NT_SUM);
  pRootNode->m_pOperand[0] = pLeftOp;
  pRootNode->m_pOperand[1] = pMultNode;
  return ParseSumSuffix(pFirstToken, pRootNode);
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseLvalue(CList<CToken *>::TNode *&pFirstToken)
{
  TOperatorNode *pOpNode;
  if (!pFirstToken)
    return 0;
  if (pFirstToken->Data->m_eType == CToken::TT_VARIABLE) {
    // Lvalue ::= Variable
    pOpNode = new TOperatorNode(pFirstToken->Data, NT_LVALUE);
    pFirstToken = pFirstToken->pNext;
    return pOpNode;
  }
  if (pFirstToken->Data->m_eType == CToken::TT_OPENBRACE) {
    CList<CToken *>::TNode *pToken = pFirstToken->pNext;
    pOpNode = ParseLvalue(pToken);
    if (pOpNode) {
      if (pToken && pToken->Data->m_eType == CToken::TT_CLOSEBRACE) {
        // Lvalue ::= (Lvalue)
        pFirstToken = pToken->pNext;
        return pOpNode;
      }
      delete pOpNode;
    }
  }
  return 0;
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseAssignment(CList<CToken *>::TNode *&pFirstToken)
{
  TOperatorNode *pLvalueNode, *pExprNode, *pRootNode;
  if (!pFirstToken)
    return 0;
  CList<CToken *>::TNode *pToken = pFirstToken;
  pLvalueNode = ParseLvalue(pToken);
  if (!pLvalueNode)
    return 0;
  if (pToken && pToken->Data->m_eType == CToken::TT_ASSIGN) {
    CToken *pAssignToken = pToken->Data;
    pToken = pToken->pNext;
    pExprNode = ParseExpression(pToken);
    if (pExprNode) {
      // Assignment ::= Lvalue = Expression
      pFirstToken = pToken;
      pRootNode = new TOperatorNode(pAssignToken, NT_ASSIGN);
      pRootNode->m_pOperand[0] = pLvalueNode;
      pRootNode->m_pOperand[1] = pExprNode;
      return pRootNode;
    }
  }
  delete pLvalueNode;
  return 0;
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseExpression(CList<CToken *>::TNode *&pFirstToken)
{
  TOperatorNode *pOpNode;
  if (!pFirstToken)
    return 0;
  pOpNode = ParseAssignment(pFirstToken);
  if (pOpNode) {
    // Expression ::= Assignment
    return pOpNode;
  }
  pOpNode = ParseSum(pFirstToken);
  if (pOpNode) {
    // Expression ::= Sum
    return pOpNode;
  }
  return 0;
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseOperator(CList<CToken *>::TNode *&pFirstToken)
{
  // Operator ::= Expression
  return ParseExpression(pFirstToken);
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseOperatorList(CList<CToken *>::TNode *&pFirstToken)
{
  TOperatorNode *pFirstNode, *pRestNode, *pNewNode;
  if (!pFirstToken)
    return 0;
  pFirstNode = ParseOperator(pFirstToken);
  if (!pFirstNode) {
    // OperatorList ::= Nothing
    return 0;
  }
  pRestNode = ParseOperatorList(pFirstToken);
  // OperatorList ::= Operator OperatorList
  pNewNode = new TOperatorNode(0, NT_OPERATORLIST);
  pNewNode->m_pOperand[0] = pFirstNode;
  pNewNode->m_pOperand[1] = pRestNode;
  return pNewNode;
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseBlock(CList<CToken *>::TNode *&pFirstToken)
{
  if (!pFirstToken)
    return 0;
  if (pFirstToken->Data->m_eType != CToken::TT_OPENCURLY)
    return 0;
  CList<CToken *>::TNode *pToken = pFirstToken->pNext;
  TOperatorNode *pOperatorList = ParseOperatorList(pToken);
  if (!pToken || pToken->Data->m_eType != CToken::TT_CLOSECURLY) {
    delete pOperatorList;
    return 0;
  }
  // Block ::= { OperatorList }
  pFirstToken = pToken->pNext;
  TOperatorNode *pNewNode = new TOperatorNode(0, NT_BLOCK);
  pNewNode->m_pOperand[0] = pOperatorList;
  return pNewNode;
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseVarList(CList<CToken *>::TNode *&pFirstToken)
{
  if (!pFirstToken)
    return 0;
  if (pFirstToken->Data->m_eType != CToken::TT_VARIABLE)
    return 0;
  // VarList ::= Variable VarListSuffix
  TOperatorNode *pNode = new TOperatorNode(pFirstToken->Data, NT_VARLIST);
  pFirstToken = pFirstToken->pNext;
  pNode->m_pOperand[0] = ParseVarListSuffix(pFirstToken);
  return pNode;
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseVarListSuffix(CList<CToken *>::TNode *&pFirstToken)
{
  if (!pFirstToken)
    return 0;
  if (pFirstToken->Data->m_eType != CToken::TT_COMMA || !pFirstToken->pNext || pFirstToken->pNext->Data->m_eType != CToken::TT_VARIABLE) {
    // VarListSuffix ::= Nothing
    return 0;
  }
  // VarListSuffix ::= , Variable VarListSuffix
  TOperatorNode *pNode = new TOperatorNode(pFirstToken->pNext->Data, NT_VARLIST);
  pFirstToken = pFirstToken->pNext->pNext;
  pNode->m_pOperand[0] = ParseVarListSuffix(pFirstToken);
  return pNode;
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseArgList(CList<CToken *>::TNode *&pFirstToken)
{
  return ParseVarList(pFirstToken);
}

CGrammarParser::TOperatorNode *CGrammarParser::ParseFragment(CList<CToken *>::TNode *&pFirstToken)
{
  if (!pFirstToken)
    return 0;
  if (pFirstToken->Data->m_eType != CToken::TT_OPENBRACKET) 
    return 0;
  CList<CToken *>::TNode *pToken = pFirstToken->pNext;
  TOperatorNode *pNodeArgs = ParseArgList(pToken);
  if (pToken->Data->m_eType != CToken::TT_CLOSEBRACKET) {
    delete pNodeArgs;
    return 0;
  }
  pToken = pToken->pNext;
  TOperatorNode *pNodeBlock = ParseBlock(pToken);
  if (!pNodeBlock) {
    delete pNodeArgs;
    return 0;
  }
  // Fragment ::= [ ArgList ] Block
  pFirstToken = pToken;
  TOperatorNode *pNode = new TOperatorNode(0, NT_FRAGMENT);
  pNode->m_pOperand[0] = pNodeArgs;
  pNode->m_pOperand[1] = pNodeBlock;
  return pNode;
}

CGrammarParser::TOperatorNode *ParseFunctionCall(CList<CToken *>::TNode *&pFirstToken)
{
  return 0;
}