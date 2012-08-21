#include "stdafx.h"
#include "Font.h"
#include "Model.h"
#include "Matrix.h"
//#include <D3DX11.h>

const float CFont::INVALID_LENGTH = -100000.0f;

CFont::CFont(CStrAny sTypeface, int iSizePt)
{
  m_sTypeface = sTypeface;
  m_iSizePt = iSizePt;
  Init();
}

CFont::~CFont()
{
  Done();
}

bool CFont::Init()
{
  HDC hDC = CreateCompatibleDC(0);
  int iXdpi, iYdpi;
  iXdpi = GetDeviceCaps(hDC, LOGPIXELSX);
  iYdpi = GetDeviceCaps(hDC, LOGPIXELSY);

  LOGFONT lf;
  lf.lfHeight = m_iSizePt * iYdpi / 72;
  lf.lfWidth = 0;
  lf.lfEscapement = 0;
  lf.lfOrientation = 0;
  lf.lfWeight = FW_DONTCARE;
  lf.lfItalic = false;
  lf.lfUnderline = false;
  lf.lfStrikeOut = false;
  lf.lfCharSet = ANSI_CHARSET;
  lf.lfOutPrecision = OUT_TT_PRECIS;
  lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  lf.lfQuality = ANTIALIASED_QUALITY;
  lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
  strcpy(lf.lfFaceName, m_sTypeface.m_pBuf);

  HFONT hFont = CreateFontIndirect(&lf);
  SelectObject(hDC, hFont);

  TEXTMETRIC tm;
  bool bRes;
  bRes = !!GetTextMetrics(hDC, &tm);
  m_iAscent = tm.tmAscent;
  m_iDescent = tm.tmDescent;
  m_iHeight = tm.tmHeight;
  m_iInternalLead = tm.tmInternalLeading;
  m_iExternalLead = tm.tmExternalLeading;
  m_iAverageWidth = tm.tmAveCharWidth;
//  m_iMaxWidth = tm.tmMaxCharWidth;
  m_iFirstChar = tm.tmFirstChar;

  int iChars = tm.tmLastChar - tm.tmFirstChar + 1;
  float fCols, fRows;

  ABC abc[MAX_CHARS];
  bRes = !!GetCharABCWidths(hDC, tm.tmFirstChar, tm.tmLastChar, abc + tm.tmFirstChar);

  int i;
  m_iMaxWidth = 0;
  for (i = 0; i < tm.tmFirstChar; i++)
    m_Chars[i].ch = 0;
  while (i <= tm.tmLastChar) {
    m_Chars[i].ch = i;
    m_Chars[i].iA = abc[i].abcA;
    m_Chars[i].iB = abc[i].abcB;
    m_Chars[i].iC = abc[i].abcC;
    m_iMaxWidth = Util::Max(m_iMaxWidth, m_Chars[i].iB);
    i++;
  }
  while (i < MAX_CHARS) {
    m_Chars[i].ch = 0;
    i++;
  }

  int iKernPairs;
  KERNINGPAIR *pKernPairs;
  iKernPairs = GetKerningPairs(hDC, 0, 0);
  pKernPairs = new KERNINGPAIR[iKernPairs];
  GetKerningPairs(hDC, iKernPairs, pKernPairs);

  for (i = 0; i < iKernPairs; i++) 
    m_KerningPairs.Add(TKerningPair((char) pKernPairs[i].wFirst, (char) pKernPairs[i].wSecond, pKernPairs[i].iKernAmount));

  delete [] pKernPairs;

  fRows = sqrt(iChars * m_iMaxWidth / (float) m_iHeight);
  fCols = fRows * m_iHeight / (float) m_iMaxWidth;
  m_iCellCols = (int) ceil(fCols);
  m_iCellRows = (int) ceil(fRows);

  if (m_iCellCols > m_iCellRows) {
    if (m_iCellCols * (m_iCellRows - 1) >= iChars)
      m_iCellRows--;
  } else
    if ((m_iCellCols - 1) * m_iCellRows >= iChars) 
      m_iCellCols--;

  ASSERT(m_iCellRows * m_iCellCols >= iChars);

  BITMAPINFO bmi;
  bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth = m_iCellCols * m_iMaxWidth; 
  bmi.bmiHeader.biHeight = -m_iCellRows * m_iHeight; 
  bmi.bmiHeader.biPlanes = 1; 
  bmi.bmiHeader.biBitCount = 24; 
  bmi.bmiHeader.biCompression = BI_RGB; 
  bmi.bmiHeader.biSizeImage = 0; 
  bmi.bmiHeader.biXPelsPerMeter = (int) (iXdpi / 2.54f * 100); 
  bmi.bmiHeader.biYPelsPerMeter = (int) (iYdpi / 2.54f * 100); 
  bmi.bmiHeader.biClrUsed = 0; 
  bmi.bmiHeader.biClrImportant = 0; 

  HBITMAP hBmp = CreateDIBSection(hDC, &bmi, 0, 0, 0, 0);
  SelectObject(hDC, hBmp);

  HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0)); 
  SelectObject(hDC, hBrush);

  Rectangle(hDC, 0, 0, m_iCellCols * m_iMaxWidth, m_iCellRows * m_iHeight);

  SetTextColor(hDC, RGB(255, 255, 255));
  SetBkColor(hDC, RGB(0, 0, 0));
  SetBkMode(hDC, TRANSPARENT);
  SetTextAlign(hDC, TA_TOP | TA_LEFT | TA_NOUPDATECP);

  for (int iRow = 0; iRow < m_iCellRows; iRow++)
    for (int iCol = 0; iCol < m_iCellCols; iCol++) {
      RECT rc;
      char chBuf[2] = { m_iFirstChar + iRow * m_iCellCols + iCol, 0 };
      if (!m_Chars[(BYTE) chBuf[0]].ch)
        continue;
      rc.left = iCol * m_iMaxWidth - m_Chars[(BYTE) chBuf[0]].iA;
      rc.top = iRow * m_iHeight;
      rc.right = rc.left + m_iMaxWidth;
      rc.bottom = rc.top + m_iHeight;
//      DrawText(hDC, chBuf, 1, &rc, DT_LEFT | DT_TOP);
      TextOut(hDC, rc.left, rc.top, chBuf, 1);
    }

  bRes = !!GdiFlush();

  bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth = m_iCellCols * m_iMaxWidth; 
  bmi.bmiHeader.biHeight = -m_iCellRows * m_iHeight; 
  bmi.bmiHeader.biPlanes = 1; 
  bmi.bmiHeader.biBitCount = 32; 
  bmi.bmiHeader.biCompression = BI_RGB; 
  bmi.bmiHeader.biSizeImage = 0; 
  bmi.bmiHeader.biXPelsPerMeter = (int) (iXdpi / 2.54f * 100); 
  bmi.bmiHeader.biYPelsPerMeter = (int) (iYdpi / 2.54f * 100); 
  bmi.bmiHeader.biClrUsed = 0; 
  bmi.bmiHeader.biClrImportant = 0; 
  int iSize = bmi.bmiHeader.biWidth * abs(bmi.bmiHeader.biHeight) * bmi.bmiHeader.biBitCount / 8;
  BYTE *pBuf = new BYTE[iSize];
  bRes = !!GetDIBits(hDC, hBmp, 0, abs(bmi.bmiHeader.biHeight), pBuf, &bmi, DIB_RGB_COLORS);

  for (i = 0; i < iSize; i += 4)
//    Util::Swap(pBuf[i], pBuf[i + 2]);
    pBuf[i / 4] = pBuf[i];

  m_pTexture = new CTexture();
  bRes = m_pTexture->Init(bmi.bmiHeader.biWidth, abs(bmi.bmiHeader.biHeight), DXGI_FORMAT_R8_UNORM, 1, pBuf, bmi.bmiHeader.biWidth /* * bmi.bmiHeader.biBitCount / 8*/, 0);

/*
  ID3D11Resource *pTexRes = 0;
  m_pTexture->m_pTextureView->GetResource(&pTexRes);
  D3DX11SaveTextureToFile(CGraphics::Get()->m_pDeviceContext, pTexRes, D3DX11_IFF_BMP, "font.bmp");
  SAFE_RELEASE(pTexRes);
*/

  delete [] pBuf;
  DeleteObject(hBrush);
  DeleteObject(hBmp);
  DeleteObject(hFont);
  DeleteDC(hDC);

  if (!InitModel())
    return false;

  return true;
}

bool CFont::InitModel()
{
  static CStrAny sPlain(ST_CONST, "Plain");
  static CStrAny sg_mWorld(ST_CONST, "g_mWorld");
  static CStrAny sg_mView(ST_CONST, "g_mView");
  static CStrAny sg_mProj(ST_CONST, "g_mProj");
  static CStrAny sg_mTexTransform(ST_CONST, "g_mTexTransform");
  static CStrAny sg_cMaterialDiffuse(ST_CONST, "g_cMaterialDiffuse");
  static CStrAny sg_txDiffuse(ST_CONST, "g_txDiffuse");
  static CStrAny sg_sDiffuse(ST_CONST, "g_sDiffuse");

  ASSERT(!m_pTextModel);
  bool bRes;
  CTechnique *pTech = CGraphics::Get()->GetTechnique(sPlain);
  if (!pTech)
    return false;
  CSmartPtr<CGeometry> pGeom(new CGeometry());
  bRes = pGeom->Init(pTech->m_pInputDesc, CGeometry::PT_TRIANGLELIST, INIT_BUFFER_CHARS * 4, INIT_BUFFER_CHARS * 6, 0, 0, 
                     CResource::RF_DYNAMIC | CResource::RF_KEEPSYSTEMCOPY, CResource::RF_DYNAMIC | CResource::RF_KEEPSYSTEMCOPY);
  if (!bRes) 
    return false;
  CSmartPtr<CMaterial> pMaterial(new CMaterial());
  bRes = pMaterial->Init(pTech, true, 0);
  if (!bRes)
    return false;
  CSmartPtr<CModel> pModel(new CModel());
  bRes = pModel->Init(pGeom, pMaterial, 0, false);
  if (!bRes) 
    return false;

  m_pTextModel = pModel;

  CMatrix<4, 4> mIdentity;
  mIdentity.SetDiagonal();
  CMatrixVar vMat(4, 4, CMatrixVar::MVF_OWNVALUES, &mIdentity(0, 0));
  m_pTextModel->SetVar(sg_mProj, vMat);
  m_pTextModel->SetVar(sg_mView, vMat);
  m_pTextModel->SetVar(sg_mWorld, vMat);
  CMatrix<3, 2> mTexIdentity;
  mTexIdentity.SetDiagonal();
  CMatrixVar vTexMat(3, 2, CMatrixVar::MVF_OWNVALUES, &mTexIdentity(0, 0));
  m_pTextModel->SetVar(sg_mTexTransform, vTexMat);
  CVar<CVector<4> > vVec4;
  vVec4.Val().Set(1, 1, 1, 1);
  m_pTextModel->SetVar(sg_cMaterialDiffuse, vVec4);

  CVar<CTexture *> vTexVar;
  vTexVar.Val() = m_pTexture;
  m_pTextModel->SetVar(sg_txDiffuse, vTexVar);

  CVar<CSampler *> vSampVar;
  vSampVar.Val() = CGraphics::Get()->GetSampler(CSampler::TDesc());
  m_pTextModel->SetVar(sg_sDiffuse, vSampVar);

  ResetModel();

  return true;
}

void CFont::Done()
{
  m_pTextModel = 0;
  m_pTexture = 0;
}

bool CFont::IsValid()
{
  return m_pTexture && m_pTextModel;
}

CRect<> CFont::GetCharRect(char ch)
{
  CRect<> rc;
  if (m_Chars[(BYTE) ch].ch != ch) 
    return rc.SetEmpty();

  rc.m_vMin.x() = ((ch - m_iFirstChar) % m_iCellCols) / (float) m_iCellCols;
  rc.m_vMin.y() = ((ch - m_iFirstChar) / m_iCellCols) / (float) m_iCellRows;
  rc.m_vMax = rc.m_vMin + CVector<2>::Get((m_Chars[(BYTE) ch].iB + 1) / (float) (m_iCellCols * m_iMaxWidth), 1.0f / m_iCellRows);

  return rc;
}

int CFont::GetKerning(char ch1, char ch2)
{
  THashKerning::TIter it;
  it = m_KerningPairs.Find((WORD) (ch2 << 8) | ch1);
  if (!it)
    return 0;
  return (*it).iKerning;
}

struct TPlainVertex {
  CVector<3> vPos;
  CVector<2> vTex;
};

float CFont::AddStr(CStrAny &sStr, const CVector<2> &vPos, char chPrevious)
{
  ASSERT(IsValid());
  if (!sStr.Length())
    return 0;
  UINT uiVerts, uiInds, uiVertSize;
  uiVerts = sStr.Length() * 4;
  uiInds = sStr.Length() * 6;
  CGeometry *pGeom = m_pTextModel->m_pGeometry;
  uiVertSize = pGeom->m_pInputDesc->GetSize();
  ASSERT(uiVertSize == sizeof(TPlainVertex));
  if (pGeom->m_pVB->GetSize(0) < (pGeom->m_uiVertices + uiVerts) * uiVertSize)
    return INVALID_LENGTH;
  if (pGeom->m_pIB->GetSize(0) < (pGeom->m_uiIndices + uiInds) * sizeof(WORD))
    return INVALID_LENGTH;
  TPlainVertex *pVerts;
  WORD *pInds;
  pVerts = (TPlainVertex *) pGeom->m_pVB->Map(0, CResource::RMF_SYSTEM_ONLY, pGeom->m_uiVertices * uiVertSize, uiVerts * uiVertSize);
  ASSERT(pVerts);
  pInds = (WORD *) pGeom->m_pIB->Map(0, CResource::RMF_SYSTEM_ONLY, pGeom->m_uiIndices * sizeof(WORD), uiInds * sizeof(WORD));
  ASSERT(pInds);

  CVector<3> vCur = { vPos.x(), vPos.y() - m_iAscent, 0.5f }, vBoxSize = { 0, (float) m_iHeight, 0 };

  int i, iChars = 0;
  for (i = 0; i < sStr.Length(); i++) {
    char ch = sStr[i];
    if (!m_Chars[(BYTE) ch].ch)
      continue;
    CRect<> rcInTex = GetCharRect(ch);
    vBoxSize.x() = (float) m_Chars[(BYTE) ch].iB;
    
    vCur.x() += m_Chars[(BYTE) ch].iA + GetKerning(chPrevious, ch);

    pVerts[0].vPos.Set(vCur.x(), vCur.y() + vBoxSize.y(), vCur.z());
    pVerts[0].vTex = rcInTex.m_vMin;

    pVerts[1].vPos = vCur;
    pVerts[1].vTex = rcInTex.GetPoint(0, 1);

    pVerts[2].vPos = vCur + vBoxSize;
    pVerts[2].vTex = rcInTex.GetPoint(1, 0);

    pVerts[3].vPos.Set(vCur.x() + vBoxSize.x(), vCur.y(), vCur.z());
    pVerts[3].vTex = rcInTex.m_vMax;

    pInds[0] = pGeom->m_uiVertices + iChars * 4;
    pInds[1] = pInds[0] + 1;
    pInds[2] = pInds[0] + 2;
    pInds[3] = pInds[0] + 2;
    pInds[4] = pInds[0] + 1;
    pInds[5] = pInds[0] + 3;

    vCur.x() += m_Chars[(BYTE) ch].iB + m_Chars[(BYTE) ch].iC;
    chPrevious = ch;

    pVerts += 4;
    pInds += 6;
    iChars++;
  }

  pGeom->m_pIB->Unmap();
  pGeom->m_pVB->Unmap();
  pGeom->m_uiVertices += iChars * 4;
  pGeom->m_uiIndices += iChars * 6;

  return vCur.x() - vPos.x();
}

bool CFont::RenderModel()
{
  static CStrAny sg_mProj(ST_CONST, "g_mProj");
  bool bRes;

  if (!m_pTextModel->m_pGeometry->m_uiIndices)
    return true;

  CRect<> rcView;
  float fNear, fFar;
  CGraphics::Get()->GetViewport(rcView, fNear, fFar);

  CMatrixVar vProj(4, 4, CMatrixVar::MVF_OWNVALUES, 0);
  CMatrix<4, 4> *pProj = (CMatrix<4, 4> *) vProj.m_pVal;
  pProj->SetOrthographic(rcView.m_vMin.x(), rcView.m_vMin.y(), rcView.m_vMax.x(), rcView.m_vMax.y(), fNear, fFar);
  bRes = m_pTextModel->SetVar(sg_mProj, vProj);
  ASSERT(bRes);

  // Flush vertex & index buffers to hardware
  CGeometry *pGeom = m_pTextModel->m_pGeometry;
  pGeom->m_pVB->Map();
  pGeom->m_pVB->Unmap();

  pGeom->m_pIB->Map();
  pGeom->m_pIB->Unmap();

  bRes = m_pTextModel->Render();
  return bRes;
}

void CFont::ResetModel()
{
  m_pTextModel->m_pGeometry->m_uiIndices = 0;
  m_pTextModel->m_pGeometry->m_uiVertices = 0;
}
