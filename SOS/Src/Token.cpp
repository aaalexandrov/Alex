#include "stdafx.h"
#include "Token.h"

using namespace Parse;

// Parsing helpers -------------------------------------------------------------

CStrAny g_sWhitespace(ST_WHOLE, " \t\n\r");
CStrAny g_sNumber(ST_WHOLE, "0123456789.");
CStrAny g_sStringDelimiter(ST_WHOLE, "\"'");

// CValue2String ---------------------------------------------------------------

CStrAny CValue2String::GetStr(int iVal)
{
  if (iVal >= 0 && iVal < m_iCount && m_pTable[iVal].m_iVal == iVal)
    return m_pTable[iVal].m_sStr;
  for (int i = 0; i < m_iCount; ++i)
    if (m_pTable[i].m_iVal == iVal)
      return m_pTable[i].m_sStr;
  ASSERT(!"String not found for value");
  return CStrAny(ST_WHOLE, "<Unrecognized>");
}

// CToken ----------------------------------------------------------------------

CValue2String::TValueString CToken::s_arrTT2Str[TT_LAST] = {
  VAL2STR(TT_UNKNOWN),
  VAL2STR(TT_VARIABLE),
	VAL2STR(TT_LOCAL),
	VAL2STR(TT_FUNCTION),
	VAL2STR(TT_END),
	VAL2STR(TT_RETURN),
	VAL2STR(TT_IF),
	VAL2STR(TT_THEN),
	VAL2STR(TT_ELSE),
	VAL2STR(TT_WHILE),
	VAL2STR(TT_FOR),
	VAL2STR(TT_DO),
	VAL2STR(TT_TRUE),
	VAL2STR(TT_FALSE),
	VAL2STR(TT_NOT),
	VAL2STR(TT_AND),
	VAL2STR(TT_OR),
	VAL2STR(TT_NIL),
  VAL2STR(TT_NUMBER),
  VAL2STR(TT_STRING),
	VAL2STR(TT_EQUAL),
	VAL2STR(TT_NOT_EQUAL),
	VAL2STR(TT_LESS_EQUAL),
	VAL2STR(TT_GREAT_EQUAL),
	VAL2STR(TT_LESS),
	VAL2STR(TT_GREAT),
  VAL2STR(TT_CONCAT),
  VAL2STR(TT_PLUS),
  VAL2STR(TT_MINUS),
  VAL2STR(TT_MULTIPLY),
  VAL2STR(TT_DIVIDE),
	VAL2STR(TT_POWER),
	VAL2STR(TT_NEGATE),
  VAL2STR(TT_ASSIGN),
  VAL2STR(TT_OPENBRACE),
  VAL2STR(TT_CLOSEBRACE),
  VAL2STR(TT_OPENBRACKET),
  VAL2STR(TT_CLOSEBRACKET),
  VAL2STR(TT_OPENCURLY),
  VAL2STR(TT_CLOSECURLY),
  VAL2STR(TT_COMMA),
  VAL2STR(TT_DOT),
};

CValue2String CToken::s_kTT2Str(s_arrTT2Str, ARRSIZE(s_arrTT2Str));

CToken::CToken(CStrAny const &sToken, ETokenType eType)
{
  m_sToken = sToken;
  m_eType = eType;
}

CToken::~CToken()
{
}

CStrAny CToken::ToString() const
{
  CStrAny s(ST_WHOLE, "Token: ");
  s += m_sToken;
  s += CStrAny(ST_WHOLE, " Type: ");
  s += TypeToString(m_eType);
  return s;
}

CStrAny CToken::TypeToString(ETokenType eType)
{
  return s_kTT2Str.GetStr(eType);
}

// CTokenizer ------------------------------------------------------------------

CTokenizer::CTokenizer()
{
  m_iTempIndex = 0;
}

CTokenizer::~CTokenizer()
{
	Clear();
}

void CTokenizer::Clear()
{
  m_iTempIndex = 0;
  m_sInput.Clear();
  m_lstTokens.DeleteAll();
  m_lstTemp.DeleteAll();
}

CToken *CTokenizer::GetTempToken(CStrAny sToken, CToken::ETokenType eTT)
{
  CToken *pTemp;
  if (eTT == CToken::TT_UNKNOWN) {
    CStrAny sInp(sToken, ST_PART);
    pTemp = ReadToken(sInp);
    ASSERT(!sInp); // The read token consumed the whole string
  }
  else
    pTemp = NEW(CToken, (sToken, eTT));
  m_lstTemp.Push(pTemp);
  return pTemp;
}

CStrAny CTokenizer::GetTempName(CStrAny sPrefix)
{
  CStrAny sName = sPrefix + CStrAny(ST_STR, '#') + CStrAny(ST_STR, m_iTempIndex++);
  return sName.AssureInRepository();
}

EInterpretError CTokenizer::Tokenize(CStrAny const &sInput)
{
  CStrAny sInp(sInput, ST_PART);
  CToken *pToken;
	EInterpretError err = IERR_OK;

  Clear();
  m_sInput = sInput;
  while (true) {
    pToken = ReadToken(sInp);
    if (!pToken)
      break;
    m_lstTokens.PushTail(pToken);
    if (pToken->m_eType == CToken::TT_UNKNOWN)
      err = IERR_UNKNOWN_TOKEN;
  }

  return err;
}

CToken *CTokenizer::ReadToken(CStrAny &sInp)
{
  ASSERT(!sInp.m_bHasHeader && "ReadToken parameter needs to be of type ST_PART");
  CStrAny sToken;
  CToken *pToken;
  CToken const *pTemplate;

  ReadChars(sInp, g_sWhitespace);
  if (!sInp)
    return 0;
  sToken = ReadIdentifier(sInp);
  if (!!sToken) { // Identifier
    pTemplate = GetTemplateToken(sToken, true);
    if (pTemplate)
      pToken = NEW(CToken, (sToken, pTemplate->m_eType)); // Keyword
    else
      pToken = NEW(CToken, (sToken, CToken::TT_VARIABLE));
    return pToken;
  }
  pTemplate = GetTemplateToken(sInp, false);
  if (pTemplate) { // Operator or separator
    sToken.m_pBuf = sInp.m_pBuf;
    sToken.m_iLen = pTemplate->m_sToken.m_iLen;
    sInp >>= sToken.m_iLen;
    pToken = NEW(CToken,(sToken, pTemplate->m_eType));
    return pToken;
  }
  if (g_sNumber.Find(sInp[0]) >= 0) { // Number literal
    sToken = ReadFloat(sInp);
    ASSERT(!!sToken);
    pToken = NEW(CToken, (sToken, CToken::TT_NUMBER));
    return pToken;
  }
  if (g_sStringDelimiter.Find(sInp[0]) >= 0) { // String literal
    char chDelimiter = sInp[0];
    sInp >>= 1;
    sToken = ReadUntilChar(sInp, chDelimiter);
    ASSERT(!!sInp && sInp[0] == chDelimiter);
    sInp >>= 1;
    pToken = NEW(CToken, (sToken, CToken::TT_STRING));
    return pToken;
  }
  sToken.m_pBuf = sInp.m_pBuf;
  sToken.m_iLen = 1;
  sInp >>= 1;
  pToken = NEW(CToken, (sToken, CToken::TT_UNKNOWN));
  return pToken;
}

bool CTokenizer::Valid() const
{
  return !!m_sInput && m_lstTokens.m_iCount > 0;
}

void CTokenizer::Dump(CList<CToken *> *pList)
{
	if (!pList) {
		printf("Tokens: ");
		Dump(&m_lstTokens);
		return;
	}
  CList<CToken*>::TNode *pNode;
  printf("<start of token list>\n");
  for (pNode = pList->m_pHead; pNode; pNode = pNode->pNext) {
    CStrAny s = pNode->Data->ToString();
    s += CStrAny(ST_WHOLE, "\n");
    printf("%s", s.m_pBuf);
  }
  printf("<end of token list>\n");
}

CToken CTokenizer::s_TemplateTokens[CToken::TT_LAST] = {
  CToken(CStrAny(ST_WHOLE, ""),         CToken::TT_UNKNOWN),
  CToken(CStrAny(ST_WHOLE, ""),         CToken::TT_VARIABLE),
	CToken(CStrAny(ST_WHOLE, "local"),    CToken::TT_LOCAL),
	CToken(CStrAny(ST_WHOLE, "function"), CToken::TT_FUNCTION),
	CToken(CStrAny(ST_WHOLE, "end"),      CToken::TT_END),
	CToken(CStrAny(ST_WHOLE, "return"),   CToken::TT_RETURN),
	CToken(CStrAny(ST_WHOLE, "if"),       CToken::TT_IF),
	CToken(CStrAny(ST_WHOLE, "then"),     CToken::TT_THEN),
	CToken(CStrAny(ST_WHOLE, "else"),     CToken::TT_ELSE),
	CToken(CStrAny(ST_WHOLE, "while"),    CToken::TT_WHILE),
	CToken(CStrAny(ST_WHOLE, "for"),      CToken::TT_FOR),
	CToken(CStrAny(ST_WHOLE, "do"),       CToken::TT_DO),
	CToken(CStrAny(ST_WHOLE, "true"),     CToken::TT_TRUE),
	CToken(CStrAny(ST_WHOLE, "false"),    CToken::TT_FALSE),
	CToken(CStrAny(ST_WHOLE, "not"),      CToken::TT_NOT),
	CToken(CStrAny(ST_WHOLE, "and"),      CToken::TT_AND),
	CToken(CStrAny(ST_WHOLE, "or"),       CToken::TT_OR),
	CToken(CStrAny(ST_WHOLE, "nil"),      CToken::TT_NIL),
  CToken(CStrAny(ST_WHOLE, ""),         CToken::TT_NUMBER),
  CToken(CStrAny(ST_WHOLE, ""),         CToken::TT_STRING),
  CToken(CStrAny(ST_WHOLE, "=="),       CToken::TT_EQUAL),
  CToken(CStrAny(ST_WHOLE, "~="),       CToken::TT_NOT_EQUAL),
  CToken(CStrAny(ST_WHOLE, "<="),       CToken::TT_LESS_EQUAL),
  CToken(CStrAny(ST_WHOLE, ">="),       CToken::TT_GREAT_EQUAL),
  CToken(CStrAny(ST_WHOLE, "<"),        CToken::TT_LESS),
  CToken(CStrAny(ST_WHOLE, ">"),        CToken::TT_GREAT),
  CToken(CStrAny(ST_WHOLE, ".."),       CToken::TT_CONCAT),
  CToken(CStrAny(ST_WHOLE, "+"),        CToken::TT_PLUS),
  CToken(CStrAny(ST_WHOLE, "-"),        CToken::TT_MINUS),
  CToken(CStrAny(ST_WHOLE, "*"),        CToken::TT_MULTIPLY),
  CToken(CStrAny(ST_WHOLE, "/"),        CToken::TT_DIVIDE),
	CToken(CStrAny(ST_WHOLE, "^"),        CToken::TT_POWER),
	CToken(CStrAny(ST_WHOLE, "-"),        CToken::TT_NEGATE),
  CToken(CStrAny(ST_WHOLE, "="),        CToken::TT_ASSIGN),
  CToken(CStrAny(ST_WHOLE, "("),        CToken::TT_OPENBRACE),
  CToken(CStrAny(ST_WHOLE, ")"),        CToken::TT_CLOSEBRACE),
  CToken(CStrAny(ST_WHOLE, "["),        CToken::TT_OPENBRACKET),
  CToken(CStrAny(ST_WHOLE, "]"),        CToken::TT_CLOSEBRACKET),
  CToken(CStrAny(ST_WHOLE, "{"),        CToken::TT_OPENCURLY),
  CToken(CStrAny(ST_WHOLE, "}"),        CToken::TT_CLOSECURLY),
  CToken(CStrAny(ST_WHOLE, ","),        CToken::TT_COMMA),
  CToken(CStrAny(ST_WHOLE, "."),        CToken::TT_DOT),
};

CToken const *CTokenizer::GetTemplateToken(CStrAny const &sToken, bool bMatchKeyword)
{
  for (int i = 0; i < CToken::TT_LAST; i++) {
		if (bMatchKeyword) {
			if (s_TemplateTokens[i].m_sToken == sToken)
				return &s_TemplateTokens[i];
		} else
			if (!!s_TemplateTokens[i].m_sToken && sToken.StartsWith(s_TemplateTokens[i].m_sToken))
				return &s_TemplateTokens[i];
	}
  return 0;
}

CToken const *CTokenizer::GetTemplateToken(CToken::ETokenType eType)
{
  ASSERT(eType >= 0 && eType < CToken::TT_LAST);
	ASSERT(s_TemplateTokens[eType].m_eType == eType);
  return &s_TemplateTokens[eType];
}
