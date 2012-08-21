#include "stdafx.h"
#include "Compile.h"

// CCompiler ------------------------------------------------------------------

CCompiler::CCompiler()
{
  m_pCode = 0;
}

CCompiler::~CCompiler()
{
  Clear();
}

void CCompiler::Clear()
{
  SAFE_DELETE(m_pCode);
}

EInterpretError CCompiler::Compile(CGrammarParser::TOperatorNode *pNode)
{
  Clear();

  m_pCode = new CFragment();
  EInterpretError res = CompileExpression(pNode);

  if (res != IERR_OK)
    SAFE_DELETE(m_pCode);

  return res;
}

EInterpretError CCompiler::CompileExpression(CGrammarParser::TOperatorNode *pExpression)
{
  if (!pExpression)
    return IERR_OK;
  ASSERT(pExpression->m_pToken->m_eClass == CToken::TC_OPERATOR || 
         pExpression->m_pToken->m_eClass == CToken::TC_IDENTIFIER || 
         pExpression->m_pToken->m_eClass == CToken::TC_LITERAL);

  if (pExpression->m_pToken->m_eType == CToken::TT_NUMBER)
    return CompileNumber(pExpression);
  if (pExpression->m_pToken->m_eType == CToken::TT_STRING)
    return CompileString(pExpression);
  if (pExpression->m_pToken->m_eType == CToken::TT_VARIABLE)
    if (pExpression->m_eType == CGrammarParser::NT_LVALUE)
      return CompileLValue(pExpression);
    else 
      return CompileVariable(pExpression);
  if (pExpression->m_pToken->m_eClass == CToken::TC_OPERATOR)
    return CompileOperator(pExpression);

  return IERR_INVALID_TOKEN;
}

EInterpretError CCompiler::CompileNumber(CGrammarParser::TOperatorNode *pNode)
{
  ASSERT(pNode->m_pToken->m_eType == CToken::TT_NUMBER);
  CInstruction kInstr;
  char chBuf[64];

  memcpy(chBuf, pNode->m_pToken->m_sToken.m_pBuf, pNode->m_pToken->m_sToken.Length());
  chBuf[pNode->m_pToken->m_sToken.Length()] = 0;
  kInstr.SetPushValue(CValue((float) atof(chBuf)));
  m_pCode->Append(kInstr);

  return IERR_OK;
}

EInterpretError CCompiler::CompileString(CGrammarParser::TOperatorNode *pNode)
{
  ASSERT(pNode->m_pToken->m_eType == CToken::TT_STRING);
  CInstruction kInstr;

  CStrAny sStr(ST_CONST, pNode->m_pToken->m_sToken.m_pBuf, pNode->m_pToken->m_sToken.Length());
  kInstr.SetPushValue(CValue(sStr.GetHeader()));
  m_pCode->Append(kInstr);

  return IERR_OK;
}

EInterpretError CCompiler::CompileVariable(CGrammarParser::TOperatorNode *pNode)
{
  ASSERT(pNode->m_pToken->m_eType == CToken::TT_VARIABLE && pNode->m_eType == CGrammarParser::NT_OPERAND);
  CInstruction kInstr;

  CStrAny sStr(ST_CONST, pNode->m_pToken->m_sToken.m_pBuf, pNode->m_pToken->m_sToken.Length());
  kInstr.SetPushValue(CValue(sStr.GetHeader()));
  m_pCode->Append(kInstr);

  kInstr.SetResolveValue();
  m_pCode->Append(kInstr);

  return IERR_OK;
}

EInterpretError CCompiler::CompileLValue(CGrammarParser::TOperatorNode *pNode)
{
  ASSERT(pNode->m_pToken->m_eType == CToken::TT_VARIABLE && pNode->m_eType == CGrammarParser::NT_LVALUE);
  CInstruction kInstr;

  CStrAny sStr(ST_CONST, pNode->m_pToken->m_sToken.m_pBuf, pNode->m_pToken->m_sToken.Length());
  kInstr.SetPushValue(CValue(sStr.GetHeader()));
  m_pCode->Append(kInstr);

  kInstr.SetResolveRef();
  m_pCode->Append(kInstr);

  return IERR_OK;
}

EInterpretError CCompiler::CompileOperator(CGrammarParser::TOperatorNode *pNode)
{
  ASSERT(pNode->m_pToken->m_eClass == CToken::TC_OPERATOR);
  CInstruction kInstr;

  for (int i = CGrammarParser::MAX_OPERANDS - 1; i >= 0; --i) {
    if (!pNode->m_pOperand[i])
      continue;
    CompileExpression(pNode->m_pOperand[i]);
  }
  switch(pNode->m_pToken->m_eType) {
    case CToken::TT_PLUS:
      kInstr.SetAdd();
      break;
    case CToken::TT_MINUS:
      kInstr.SetSubtract();
      break;
    case CToken::TT_MULTIPLY:
      kInstr.SetMultiply();
      break;
    case CToken::TT_DIVIDE:
      kInstr.SetDivide();
      break;
    case CToken::TT_POWER:
      kInstr.SetPower();
      break;
    case CToken::TT_NEGATE:
      kInstr.SetPushValue(CValue(0.0f));
      m_pCode->Append(kInstr);
      kInstr.SetSubtract();
      break;
    case CToken::TT_ASSIGN:
      kInstr.SetAssign();
      break;
    default:
      ASSERT(!"Trying to compile unknown operator");
      return IERR_INVALID_OPERATOR;
  }
  m_pCode->Append(kInstr);

  return IERR_OK;
}

// CCompileChain --------------------------------------------------------------

EInterpretError CCompileChain::Compile(CStrAny sCode)
{
  EInterpretError err;

  err = m_kTokenizer.Tokenize(sCode);
  if (err != IERR_OK)
    return err;

  err = m_kGrammar.Parse(&m_kTokenizer.m_lstTokens);
  if (err != IERR_OK)
    return err;

  err = m_kCompiler.Compile(m_kGrammar.m_pParseTree);
  if (err != IERR_OK)
    return err;

  return IERR_OK;
}
