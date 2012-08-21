#ifndef __GRAMMAR_H
#define __GRAMMAR_H

#include "Token.h"

class CGrammarParser {
public:
  enum ENodeType {
    NT_UNKNOWN,
    
    NT_OPERAND,
    NT_NEGATION,
    NT_POWER,
    NT_MULTIPLICATION,
    NT_SUM,
    NT_LVALUE,
    NT_ASSIGN,

    NT_LAST
  };

  static const int MAX_OPERANDS = 2;
  struct TOperatorNode {
    TOperatorNode *m_pOperand[MAX_OPERANDS];
    CToken *m_pToken;
    ENodeType m_eType;

    TOperatorNode(CToken *pToken, ENodeType eType): m_pToken(pToken), m_eType(eType) { for (int i = 0; i < MAX_OPERANDS; i++) m_pOperand[i] = 0; }
    ~TOperatorNode() { for (int i = 0; i < MAX_OPERANDS; i++) delete m_pOperand[i]; }
  };

public:
  CList<CToken *> *m_pLstTokens;
  TOperatorNode   *m_pParseTree;

  CGrammarParser();
  ~CGrammarParser();

  void Clear();

  EInterpretError Parse(CList<CToken *> *pLstTokens);
  void DumpParseTree(TOperatorNode *pOpNode = 0, int iIndent = 0);

  TOperatorNode *ParseOperand(CList<CToken *>::TNode *&pFirstToken);
  TOperatorNode *ParseNegation(CList<CToken *>::TNode *&pFirstToken);
  TOperatorNode *ParsePower(CList<CToken *>::TNode *&pFirstToken);
  TOperatorNode *ParseMultiplication(CList<CToken *>::TNode *&pFirstToken);
  TOperatorNode *ParseMultSuffix(CList<CToken *>::TNode *&pFirstToken, TOperatorNode *pLeftOp);
  TOperatorNode *ParseSum(CList<CToken *>::TNode *&pFirstToken);
  TOperatorNode *ParseSumSuffix(CList<CToken *>::TNode *&pFirstToken, TOperatorNode *pLeftOp);
  TOperatorNode *ParseLvalue(CList<CToken *>::TNode *&pFirstToken);
  TOperatorNode *ParseAssignment(CList<CToken *>::TNode *&pFirstToken);
  TOperatorNode *ParseExpression(CList<CToken *>::TNode *&pFirstToken);

public:
  static CValue2String::TValueString s_arrNT2Str[NT_LAST];
  static CValue2String s_kNT2Str;
};

#endif
