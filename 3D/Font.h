#ifndef __FONT_H
#define __FONT_H

#include "Graphics.h"

class CModel;
class CFont {
public:
  struct TCharInfo {
    char ch;
    int iA, iB, iC;
  };

  struct TKerningPair {
    union {
      char ch[2];
      WORD wChars;
    };
    int iKerning;

    TKerningPair(char ch1, char ch2, int iKern) { ch[0] = ch1; ch[1] = ch2; iKerning = iKern; }

    static inline size_t Hash(const TKerningPair &kPair) { return kPair.wChars; }
    static inline size_t Hash(WORD wChars)               { return wChars; }
    static inline bool Eq(const TKerningPair &kPair1, const TKerningPair &kPair2) { return kPair1.wChars == kPair2.wChars; }
    static inline bool Eq(WORD wChars, const TKerningPair &kPair) { return wChars == kPair.wChars; }
  };

  typedef CHash<TKerningPair, WORD, TKerningPair, TKerningPair> THashKerning;

  static const int MAX_CHARS = 256;
  static const int INIT_BUFFER_CHARS = 4096;
  static const float INVALID_LENGTH;
public:
  CSmartPtr<CTexture> m_pTexture;
  CStrAny m_sTypeface;
  int m_iSizePt;
  int m_iAscent, m_iDescent, m_iHeight, m_iExternalLead, m_iInternalLead, m_iAverageWidth, m_iMaxWidth;
  int m_iCellRows, m_iCellCols, m_iFirstChar;
  TCharInfo m_Chars[MAX_CHARS];
  THashKerning m_KerningPairs;

  CSmartPtr<CModel> m_pTextModel;

  CFont(CStrAny sTypeface, int iSizePt);
  ~CFont();

  bool Init();
  bool InitModel();
  void Done();

  bool IsValid();

  CRect<> GetCharRect(char ch);
  int GetKerning(char ch1, char ch2);
  
  // Position is in pixels and indicates the starting position along the baseline from which to start placing the letters
  // Return value indicates the distance the position has shifted along the baseline
  // chPrevious is used to shift the starting position according to the kerning value
  float AddStr(CStrAny &sStr, const CVector<2> &vPos, char chPrevious = 0); 

  bool RenderModel();
  void ResetModel();
};

#endif