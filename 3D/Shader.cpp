#include "stdafx.h"
#include "Shader.h"
#include "File.h"
#include "VarUtil.h"
#include "State.h"
#include "Model.h"

#include <D3DCompiler.h>

// CInputDesc -------------------------------------------------------------

int CInputDesc::TInputElement::GetSize()
{
  static int arrTypeSizes[] = { 0, 4, 4, 4 };
  COMPILE_ASSERT(ARRSIZE(arrTypeSizes) == T_LAST_TYPE);
  ASSERT(m_Type > T_UNKNOWN && m_Type < T_LAST_TYPE);
  return arrTypeSizes[m_Type] * m_btElements;
}


CInputDesc &CInputDesc::operator =(const CInputDesc &id)
{
  m_uiHash = id.m_uiHash;
  m_Elements.SetCount(id.m_Elements.m_iCount);
  for (int i = 0; i < m_Elements.m_iCount; i++)
    m_Elements[i] = id.m_Elements[i];
  return *this;
}

bool CInputDesc::operator ==(const CInputDesc &id) const
{
  if (m_uiHash != id.m_uiHash && m_uiHash && id.m_uiHash)
    return false;
  if (m_Elements.m_iCount != id.m_Elements.m_iCount)
    return false;
  for (int i = 0; i < m_Elements.m_iCount; i++)
    if (m_Elements[i] != id.m_Elements[i])
      return false;
  return true;
}

int CInputDesc::GetElementOffset(int iElement)
{
  int iOffset = 0;
  for (int i = 0; i < iElement; i++)
    iOffset += m_Elements[i].GetSize();
  return iOffset;
}

CInputDesc::TInputElement *CInputDesc::GetElementInfo(CStrAny sSemantic, BYTE btSemIndex, int *pInfoIndex)
{
  int iIndex;
  if (!pInfoIndex)
    pInfoIndex = &iIndex;

  for (*pInfoIndex = 0; *pInfoIndex < m_Elements.m_iCount; (*pInfoIndex)++)
    if (m_Elements[*pInfoIndex].m_sSemantic == sSemantic && m_Elements[*pInfoIndex].m_btIndex == btSemIndex)
      return &m_Elements[*pInfoIndex];

  *pInfoIndex = -1;
  return 0;
}

int CInputDesc::AddElement(CStrAny sSem, EElemType kType, BYTE btSemIndex, BYTE btElements)
{
  int iInd = m_Elements.m_iCount;
  m_Elements.SetCount(iInd + 1);
  m_Elements[iInd].m_sSemantic = sSem;
  m_Elements[iInd].m_Type = kType;
  m_Elements[iInd].m_btIndex = btSemIndex;
  m_Elements[iInd].m_btElements = btElements;
  return iInd;
}

bool CInputDesc::IsSuperSet(CInputDesc *pInputDesc)
{
  int iElem, iThisElem; 
  TInputElement *pElem, *pThisElem;
  for (iElem = 0; iElem < pInputDesc->m_Elements.m_iCount; iElem++) {
    pElem = &pInputDesc->m_Elements[iElem];
    pThisElem = GetElementInfo(pElem->m_sSemantic, pElem->m_btIndex, &iThisElem);
    if (!pThisElem)
      return false;
    if (pThisElem->m_btElements < pElem->m_btElements)
      return false;
  }
  return true;
}

ID3D11InputLayout *CInputDesc::CreateLayout(CTechnique *pTech)
{
  if (!m_Elements.m_iCount)
    return 0;

  ASSERT(IsSuperSet(pTech->m_pInputDesc));

  CAutoDeletePtr<D3D11_INPUT_ELEMENT_DESC> pLayout(new D3D11_INPUT_ELEMENT_DESC[m_Elements.m_iCount]);
  UINT uiOffset = 0;

  for (int i = 0; i < m_Elements.m_iCount; i++) {
    pLayout[i].SemanticName = m_Elements[i].m_sSemantic.m_pBuf;
    pLayout[i].SemanticIndex = m_Elements[i].m_btIndex;
    static DXGI_FORMAT arrFormats[4][3] = {
      { DXGI_FORMAT_R32_UINT,          DXGI_FORMAT_R32_SINT,          DXGI_FORMAT_R32_FLOAT          },
      { DXGI_FORMAT_R32G32_UINT,       DXGI_FORMAT_R32G32_SINT,       DXGI_FORMAT_R32G32_FLOAT       },
      { DXGI_FORMAT_R32G32B32_UINT,    DXGI_FORMAT_R32G32B32_SINT,    DXGI_FORMAT_R32G32B32_FLOAT    },
      { DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_FLOAT },
    };
    pLayout[i].Format = arrFormats[m_Elements[i].m_btElements - 1][m_Elements[i].m_Type - T_UINT];
    pLayout[i].InputSlot = 0;
    pLayout[i].AlignedByteOffset = uiOffset;
    pLayout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    pLayout[i].InstanceDataStepRate = 0;

    uiOffset += m_Elements[i].GetSize();
  }

  ID3D11InputLayout *pInputLayout = 0;
  HRESULT res;
  TResReporter kErrorReport(res);

  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  res = CGraphics::Get()->m_pDevice->CreateInputLayout(pLayout, m_Elements.m_iCount, 
    pTech->m_pVSBlob->GetBufferPointer(), pTech->m_pVSBlob->GetBufferSize(), &pInputLayout);

  return pInputLayout;
}

size_t CInputDesc::GetHash() const
{
  if (!m_uiHash) {
    m_uiHash = m_Elements.m_iCount;
    for (int i = 0; i < m_Elements.m_iCount; i++) {
      m_uiHash += m_Elements[i].GetHash();
      m_uiHash = Util::RotateBits(m_uiHash, 7);
    }
  }
  return m_uiHash;
}

// CConstantTemplate ---------------------------------------------------------
IMPRTTI(CConstantTemplate, CVarTemplate)

CConstantTemplate::CConstantTemplate()
{
  m_iSize = 0;
}

CConstantTemplate::~CConstantTemplate()
{
  Done();
}

bool CConstantTemplate::Init(CTechnique *pTechnique, ID3D11ShaderReflectionConstantBuffer *pCBReflect)
{
  HRESULT res;
  TResReporter kErrorReport(res);
  D3D11_SHADER_BUFFER_DESC cbd;

  if (!CVarTemplate::Init())
    return false;

  res = pCBReflect->GetDesc(&cbd);
  if (FAILED(res)) 
    return false;

  m_sName = cbd.Name;
  m_iSize = cbd.Size;
  for (UINT uiVar = 0; uiVar < cbd.Variables; uiVar++) {
    ID3D11ShaderReflectionVariable *pVarReflect;
    pVarReflect = pCBReflect->GetVariableByIndex(uiVar);
    ASSERT(pVarReflect);
    if (!pVarReflect)
      continue;
    D3D11_SHADER_VARIABLE_DESC svd;
    res = pVarReflect->GetDesc(&svd);
    if (FAILED(res))
      return false;
    ID3D11ShaderReflectionType *pTypeReflect = pVarReflect->GetType();
    ASSERT(pTypeReflect);
    if (!pTypeReflect)
      continue;
    D3D11_SHADER_TYPE_DESC std;
    res = pTypeReflect->GetDesc(&std);
    if (FAILED(res)) 
      return false;

    CBaseVar *pVar = 0;
    const CRTTI *pVarRTTI = 0;
    switch (std.Class) {
      case D3D10_SVC_SCALAR:
        if (std.Type == D3D10_SVT_FLOAT) 
          pVarRTTI = &CVarRef<float>::s_RTTI;
        break;
      case D3D10_SVC_VECTOR:
        if (std.Type == D3D10_SVT_FLOAT)
          pVarRTTI = GetVectorVarRTTI(std.Columns, true);
        break;
      case D3D10_SVC_MATRIX_ROWS:
        if (std.Type == D3D10_SVT_FLOAT) {
//          pVarRTTI = GetMatrixVarRTTI(std.Rows, std.Columns, true);
          pVar = new CMatrixVar(std.Rows, /*std.Columns*/ 4, 0, 0);
        }
        break;
      case D3D10_SVC_MATRIX_COLUMNS:
        if (std.Type == D3D10_SVT_FLOAT) {
//          pVarRTTI = GetMatTVarRTTI(std.Rows, std.Columns, true);
          pVar = new CMatrixVar(/*std.Rows*/ 4, std.Columns, CMatrixVar::MVF_TRANSPOSED, 0);
        }
        break;
      default:
        break;
    }
    if (pVarRTTI)
      pVar = (CBaseVar *) pVarRTTI->CreateInstance();
    ASSERT(pVar);
    if (pVar) {
      pVar->SetRef((void *) svd.StartOffset);
      if (!m_pVars)
        m_pVars = new CVarHash();
      m_pVars->ReplaceVar(CStrAny(ST_WHOLE, svd.Name), pVar);
      if (svd.DefaultValue) {
        ASSERT(!"Default values not implemented");
      }
    }
  }

  m_sCacheName = pTechnique->m_sName + CStrAny(ST_WHOLE, "@") + m_sName;

  return true;
}

void CConstantTemplate::Done()
{
  m_sName = CStrAny();
  m_iSize = 0;
  CVarTemplate::Done();
}

// CConstantBuffer -----------------------------------------------------------
CConstantBuffer::CConstantBuffer()
{
  m_pTemplate = 0;
  m_pVars = 0;
  m_pLastCache = 0;
  m_uiFrameUpdated = 0;
}

CConstantBuffer::~CConstantBuffer()
{
  Done();
}

bool CConstantBuffer::Init(CConstantTemplate *pTemplate, CVarObj *pVars)
{
  HRESULT res;
  TResReporter kErrorReport(res);

  m_pTemplate = pTemplate;
  if (pVars)
    m_pVars = pVars;
  else
    m_pVars = pTemplate->m_pVars;

  m_pBuffer = new CD3DBuffer();
  bool bRes = m_pBuffer->Init(CResource::RT_SHADERCONSTANT, CResource::RF_DYNAMIC, 0, m_pTemplate->m_iSize);

  return bRes;
}

void CConstantBuffer::Done()
{
  m_pBuffer = 0;
  m_pTemplate = 0;
  // CConstantBuffer does not own the vars, so just forget the reference
  m_pVars = 0;
  m_pLastCache = 0;
}

bool CConstantBuffer::Map()
{
  BYTE *pMapped = m_pBuffer->Map();
  if (!pMapped)
    return false;

  m_pTemplate->Remap(pMapped, 0, m_pVars);

  return true;
}

void CConstantBuffer::Unmap()
{
  BYTE *pMapped = m_pBuffer->GetMappedPtr();
  ASSERT(pMapped);

  m_pTemplate->Remap(0, pMapped, m_pVars);

  m_pBuffer->Unmap();
}

bool CConstantBuffer::SetFrom(const CVarObj &kVars)
{
  static const CStrAny scbPerFrame(ST_CONST, "cbPerFrame");
  if (m_pTemplate->m_sName == scbPerFrame && !m_pLastCache && m_uiFrameUpdated >= CGraphics::Get()->m_uiFrameID)
    return true;

  ASSERT(!(m_pBuffer->m_uiFlags & CResource::RF_MAPPED));
  if (!Map())
    return false;

  bool bRes = true;
  CVarObj::CIter *pIt;
  for (pIt = m_pVars->GetIter(); *pIt; pIt->Next()) {
    bool bSet;
    CBaseVar *pSrc = kVars.FindVar(pIt->GetName());
    if (pSrc)
      bSet = pIt->GetValue()->SetVar(*pSrc);
    else {
      CMatrixVar *pSrcMat, *pDstMat;
      pDstMat = Cast<CMatrixVar>(pIt->GetValue());
      if (pDstMat) {
        int i = pIt->GetName().Length() - 1;
        while (i > 0 && (pIt->GetName()[i] == 'I') || (pIt->GetName()[i] == 'T'))
          i--;
        if (i > 0 && pIt->GetName()[i] == '_')
          i--;
        CStrAny sSuffix = pIt->GetName().SubStr(i + 1, -1);
        bSet = false;
        if (!!sSuffix && sSuffix[0] == '_')
          sSuffix >>= 1;
        if (!!sSuffix) {
          CStrAny sName(pIt->GetName().SubStr(0, i + 1));
          pSrcMat = Cast<CMatrixVar>(kVars.FindVar(sName));
          if (pSrcMat) 
            bSet = SetMatrixVar(pDstMat, pSrcMat, sSuffix);
        }
      } else
        bSet = false;
    }
    ASSERT(bSet);
    bRes &= bSet;
  }
  delete pIt;

  Unmap();

  m_pLastCache = 0;
  m_uiFrameUpdated = CGraphics::Get()->m_uiFrameID;

  return bRes;
}

bool CConstantBuffer::SetFrom(CConstantCache *pCache)
{
  if (pCache == m_pLastCache && m_uiFrameUpdated >= pCache->m_uiFrameUpdated)
    return true;

  if (!m_pBuffer->Map())
    return false;
  memcpy(m_pBuffer->GetMappedPtr(), pCache->m_arrBuffer.m_pArray, pCache->m_arrBuffer.m_iCount);
  m_pBuffer->Unmap();

  m_pLastCache = pCache;
  m_uiFrameUpdated = CGraphics::Get()->m_uiFrameID;

  return true;
}

bool CConstantBuffer::SetMatrixVar(CMatrixVar *pDst, CMatrixVar const *pSrc, CStrAny sSuffix)
{
  CMatrix<4, 4> m[2];
  int r, c, iNext = 1;

  pSrc->GetMatrix(m[0]);
  // pad with zeroes and 1 only on main diagonal
  r = (pSrc->m_iCols < 4) ? 0 : pSrc->m_iRows;
  while (r < 4) {
    c = (r < pSrc->m_iRows) ? pSrc->m_iCols : 0;
    while (c < 4) {
      m[0](r, c) = (r == c);
      c++;
    }
    r++;
  }

  while (!!sSuffix) {
    switch (sSuffix[0]) {
      case 'T':
        m[!iNext].GetTransposed(m[iNext]);
        break;
      case 'I':
        m[!iNext].GetInverse(m[iNext]);
        break;
    }
    sSuffix >>= 1;
    iNext = !iNext;
  }

  bool bRes;
  bRes = pDst->SetMatrix(m[!iNext]);

  return bRes;
}

// CConstantCache ------------------------------------------------------------
IMP_VAR_RTTI(CConstantCache *)
IMPLEMENT_BASE_SET(CConstantCache *)

CConstantCache::CConstantCache()
{
  m_pTemplate = 0;
  m_pVars = 0;
  m_bOwnVars = false;
  m_uiFrameUpdated = 0;
}

CConstantCache::~CConstantCache()
{
  Done();
}

bool CConstantCache::Init(CConstantTemplate *pTemplate, CVarObj *pVars)
{
  m_pTemplate = pTemplate;
  m_arrBuffer.SetCount(m_pTemplate->m_iSize);
  if (pVars) 
    m_pVars = pVars;
  else
    m_pVars = new CVarHash();
  m_bOwnVars = !pVars;
  m_pTemplate->MakeVarCopy(m_pVars, m_arrBuffer.m_pArray, 0, false);
  CVar<CConstantCache *> *pVarCache = new CVar<CConstantCache *>();
  pVarCache->Val() = this;
  m_pVars->ReplaceVar(m_pTemplate->m_sCacheName, pVarCache);
  m_uiFrameUpdated = CGraphics::Get()->m_uiFrameID;
  return true;
}

void CConstantCache::Done()
{
  if (m_bOwnVars) 
    delete m_pVars;
  else {
    m_pTemplate->ClearVarCopy(m_pVars, m_arrBuffer.m_pArray, m_arrBuffer.m_iCount);
    m_pVars->ReplaceVar(m_pTemplate->m_sCacheName, 0);
  }
  m_pVars = 0;
  m_arrBuffer.SetCount(0);
  m_pTemplate = 0;
}

// CTechnique ----------------------------------------------------------------

CTechnique::TInputDescLayout::TInputDescLayout(CInputDesc *pDesc, CTechnique *pTech)
{
  m_pDesc = pDesc;
  m_pLayout = m_pDesc->CreateLayout(pTech);
}

CTechnique::TInputDescLayout::~TInputDescLayout() 
{   
  CScopeLock kLock(&CGraphics::Get()->m_Lock); 
  SAFE_RELEASE(m_pLayout); 
}

CTechnique::CTechnique(CStrAny sSrcFile, CStrAny sVSEntry, CStrAny sPSEntry, CStrAny sName)
{
  m_sSrcFile = sSrcFile;
  m_sVSEntry = sVSEntry;
  m_sPSEntry = sPSEntry;
  m_sName = sName;
  m_pVSBlob = 0;
  m_pPSBlob = 0;
  m_pVS = 0;
  m_pPS = 0;
  m_pDefStates = 0;
  m_pInitVars = 0;
  Init();
}

CTechnique::CTechnique(CStrAny sVarFile)
{
  static CStrAny sHLSLFile(ST_CONST, "HLSLFile");
  static CStrAny sVS(ST_CONST, "VertexShader");
  static CStrAny sPS(ST_CONST, "PixelShader");
  static CStrAny sTech(ST_CONST, "Technique");
  static CStrAny sStates(ST_CONST, "States");

  CFileBase *pFile = CFileSystem::Get()->OpenFile(sVarFile, CFileBase::FOF_READ);
  CVarHash *pVars = new CVarHash();

  if (pFile) {
    sVarFile = pFile->GetFullName();
    CVarFile kVarFile(pFile);
    kVarFile.Read(pVars);
  }

  pVars->GetStr(sHLSLFile, m_sSrcFile);
  CStrAny sDrive, sDir;
  CFileSystem::ParsePath(m_sSrcFile, &sDrive, &sDir, 0, 0);
  if (!sDrive && !sDir) {
    CFileSystem::ParsePath(sVarFile, &sDrive, &sDir, 0, 0);
    m_sSrcFile = sDrive + sDir + m_sSrcFile;
  }
  pVars->GetStr(sVS, m_sVSEntry);
  pVars->GetStr(sPS, m_sPSEntry);
  pVars->GetStr(sTech, m_sName);
  m_pVSBlob = 0;
  m_pPSBlob = 0;
  m_pVS = 0;
  m_pPS = 0;
  m_pDefStates = pVars->GetContext(sStates);
  m_pInitVars = pVars;
  Init();
}

CTechnique::~CTechnique()
{
  Done();
}

bool CTechnique::Init()
{
  if (!InitShaders())
    return false;

  HRESULT res;
  TResReporter kErrorReport(res);
  CAutoReleasePtr<ID3D11ShaderReflection> pVSReflect, pPSReflect;
  res = D3DReflect(m_pVSBlob->GetBufferPointer(), m_pVSBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void **) &pVSReflect.m_pPtr);
  if (FAILED(res))
    return false;

  res = D3DReflect(m_pPSBlob->GetBufferPointer(), m_pPSBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void **) &pPSReflect.m_pPtr);
  if (FAILED(res))
    return false;

  if (!InitInputLayout(pVSReflect))
    return false;

  if (!InitConstantBuffers(pVSReflect))
    return false;
  if (!InitConstantBuffers(pPSReflect))
    return false;

  if (!InitBinds(ST_VERTEX, pVSReflect))
    return false;
  if (!InitBinds(ST_PIXEL, pPSReflect))
    return false;

/*  if (!InitConstantCache())
    return false;
*/

  if (!InitStates())
    return false;

  return true;
}

bool CTechnique::InitShaders()
{
  HRESULT res;
  TResReporter kErrorReport(res);

  CAutoDeletePtr<CFileBase> pFile(CFileSystem::Get()->OpenFile(m_sSrcFile, CFile::FOF_READ));
  if (!pFile || !pFile->IsValid())
    return false;
  int iSize = (int) pFile->GetSize();
  CAutoDeletePtr<char> pBuf(new char[iSize]);
  CFileBase::ERRCODE ec = pFile->Read(pBuf, iSize);
  if (ec)
    return false;

  CAutoReleasePtr<ID3D10Blob> pErrBlob;
  res = D3DCompile(pBuf, iSize, m_sSrcFile.m_pBuf, 0, 0, m_sVSEntry.m_pBuf, "vs_4_0", 
                   D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, 0, 
                   &m_pVSBlob, &pErrBlob.m_pPtr);

  if (FAILED(res)) {
    if (pErrBlob) 
      MessageBox(0, (char *) pErrBlob->GetBufferPointer(), "Vertex Shader Compile Error", MB_OK);
    return false;
  }

  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  res = CGraphics::Get()->m_pDevice->CreateVertexShader(m_pVSBlob->GetBufferPointer(), m_pVSBlob->GetBufferSize(), 0, &m_pVS);

  if (FAILED(res)) 
    return false;

  SAFE_RELEASE(pErrBlob.m_pPtr);

  res = D3DCompile(pBuf, iSize, m_sSrcFile.m_pBuf, 0, 0, m_sPSEntry.m_pBuf, "ps_4_0", 
                   D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY, 0, 
                   &m_pPSBlob, &pErrBlob.m_pPtr);

  if (FAILED(res)) {
    if (pErrBlob) 
      MessageBox(0, (char *) pErrBlob->GetBufferPointer(), "Pixel Shader Compile Error", MB_OK);
    return false;
  }

  res = CGraphics::Get()->m_pDevice->CreatePixelShader(m_pPSBlob->GetBufferPointer(), m_pPSBlob->GetBufferSize(), 0, &m_pPS);

  if (FAILED(res)) 
    return false;

  return true;
}

bool CTechnique::InitInputLayout(ID3D11ShaderReflection *pVSReflect)
{
  HRESULT res;
  TResReporter kErrorReport(res);

  D3D11_SHADER_DESC kVSDesc;

  res = pVSReflect->GetDesc(&kVSDesc);
  if (FAILED(res))
    return false;

  m_pInputDesc = new CInputDesc();

  CAutoDeletePtr<D3D11_INPUT_ELEMENT_DESC> pLayout(new D3D11_INPUT_ELEMENT_DESC[kVSDesc.InputParameters]);
  UINT uiOffset = 0;

  for (UINT i = 0; i < kVSDesc.InputParameters; i++) {
    D3D11_SIGNATURE_PARAMETER_DESC pd;
    res = pVSReflect->GetInputParameterDesc(i, &pd);
    if (FAILED(res))
      return false;
    int iMSBSet = 0;
    if (pd.Mask & 8)
      iMSBSet = 3;
    else
      if (pd.Mask & 4)
        iMSBSet = 2;
      else
        if (pd.Mask & 2)
          iMSBSet = 1;
    static CInputDesc::EElemType arrTypes[3] = { CInputDesc::T_UINT, CInputDesc::T_INT, CInputDesc::T_FLOAT };
    m_pInputDesc->AddElement(CStrAny(ST_CONST, pd.SemanticName), arrTypes[pd.ComponentType - D3D10_REGISTER_COMPONENT_UINT32], pd.SemanticIndex, iMSBSet + 1);
  }

  return true;
}

bool CTechnique::InitConstantBuffers(ID3D11ShaderReflection *pSReflect)
{
  HRESULT res;
  TResReporter kErrorReport(res);

  D3D11_SHADER_DESC sd;
  res = pSReflect->GetDesc(&sd);
  if (FAILED(res))
    return false;

  for (UINT i = 0; i < sd.ConstantBuffers; i++) {
    ID3D11ShaderReflectionConstantBuffer *pCBReflect = pSReflect->GetConstantBufferByIndex(i);
    if (!pCBReflect)
      return false;
    D3D11_SHADER_BUFFER_DESC sbd;
    res = pCBReflect->GetDesc(&sbd);
    if (FAILED(res))
      return false;
    if (GetConstantBuffer(CStrAny(ST_WHOLE, sbd.Name)))
      continue;
    CConstantTemplate *pCT = new CConstantTemplate();
    if (!pCT->Init(this, pCBReflect)) {
      delete pCT;
      return false;
    }
    m_arrConstantTemplates.Append(pCT);
    CConstantBuffer *pCB = new CConstantBuffer();
    if (!pCB->Init(pCT, 0)) {
      delete pCB;
      return false;
    }
    m_arrConstantBuffers.Append(pCB);
  }

  return true;
}

bool CTechnique::InitStates()
{
  bool bCacheRast, bCacheBlend, bCacheDepth;

  bCacheRast = (GetStateLocation(CStateCache::ST_RASTERIZER) == CL_TECHNIQUE);
  bCacheBlend = (GetStateLocation(CStateCache::ST_BLEND) == CL_TECHNIQUE);
  bCacheDepth = (GetStateLocation(CStateCache::ST_DEPTHSTENCIL) == CL_TECHNIQUE);

  if (!m_StateCache.Init(bCacheRast, bCacheBlend, bCacheDepth, 0, m_pDefStates))
    return false;

  for (int i = 0; i < ARRSIZE(m_uiStateCacheFrameID); i++)
    m_uiStateCacheFrameID[i] = 0;

  return true;
}

bool CTechnique::InitBinds(EShaderType eShader, ID3D11ShaderReflection *pSReflect)
{
  HRESULT res;
  TResReporter kErrorReport(res);

  D3D11_SHADER_DESC sd;
  res = pSReflect->GetDesc(&sd);
  if (FAILED(res))
    return false;

  for (UINT uiBind = 0; uiBind < sd.BoundResources; uiBind++) {
    D3D11_SHADER_INPUT_BIND_DESC bd;
    res = pSReflect->GetResourceBindingDesc(uiBind, &bd);
    if (FAILED(res))
      return false;

    int iResBind;

    ASSERT(bd.BindCount == 1);
    iResBind = m_arrBinds.m_iCount;
    m_arrBinds.SetCount(iResBind + 1);
    m_arrBinds[iResBind].m_uiBindPoint = bd.BindPoint;
    m_arrBinds[iResBind].m_uiBindCount = bd.BindCount;
    m_arrBinds[iResBind].m_eShader = eShader;
    m_arrBinds[iResBind].m_sName = bd.Name;

    switch (bd.Type) {
      case D3D10_SIT_CBUFFER:
        m_arrBinds[iResBind].m_eType = RT_CONSTANT_BUFFER;
        m_arrBinds[iResBind].m_pBuffer = GetConstantBuffer(m_arrBinds[iResBind].m_sName);
        break;
      case D3D10_SIT_TEXTURE:
        m_arrBinds[iResBind].m_eType = RT_TEXTURE;
        break;
      case D3D10_SIT_SAMPLER:
        m_arrBinds[iResBind].m_eType = RT_SAMPLER;
        break;
      default:
        ASSERT(0);
        return false;
    }
  }

  return true;
}

void CTechnique::Done()
{
  m_hashLayouts.DeleteAll();
  m_pDefStates = 0;
  SAFE_DELETE(m_pInitVars);
  m_StateCache.Done();
  m_arrConstantBuffers.DeleteAll();
  m_arrConstantTemplates.DeleteAll();
  m_arrBinds.SetCount(0);
  m_pInputDesc = 0;

  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  SAFE_RELEASE(m_pPS);
  SAFE_RELEASE(m_pVS);
  SAFE_RELEASE(m_pPSBlob);
  SAFE_RELEASE(m_pVSBlob);
}

bool CTechnique::IsValid()
{
  return m_pVSBlob && m_pPSBlob && m_pVS && m_pPS;
}

CConstantTemplate *CTechnique::GetConstantTemplate(CStrAny sName)
{
  for (int i = 0; i < m_arrConstantTemplates.m_iCount; i++)
    if (m_arrConstantTemplates[i]->m_sName == sName)
      return m_arrConstantTemplates[i];
  return 0;
}

CConstantBuffer *CTechnique::GetConstantBuffer(CStrAny sName)
{
  for (int i = 0; i < m_arrConstantBuffers.m_iCount; i++)
    if (m_arrConstantBuffers[i]->m_pTemplate->m_sName == sName)
      return m_arrConstantBuffers[i];
  return 0;
}

ID3D11InputLayout *CTechnique::GetInputLayout(CInputDesc *pDesc)
{
  THashInputLayout::TIter it;
  it = m_hashLayouts.Find(pDesc);
  if (!!it)
    return it->m_pLayout;
  TInputDescLayout *pIDL = new TInputDescLayout(pDesc, this);
  m_hashLayouts.Add(pIDL);
  return pIDL->m_pLayout;
}

bool CTechnique::SetMatrixVar(CVarObj *pVars, CStrAny sVarMatrix, CMatrixVar const *pMatVar)
{
  int i;
  bool bRes;

  bRes = pVars->SetVar(sVarMatrix, *pMatVar);

  CStrAny sAlias;
  static const CStrAny sSuffixes[] = { CStrAny(ST_WHOLE, "_I"), CStrAny(ST_WHOLE, "_T"), CStrAny(ST_WHOLE, "_IT"), CStrAny(ST_WHOLE, "_TI"), };
  CMatrixVar *pDstVar;
  CStrAny sSuff;

  for (i = 0; i < ARRSIZE(sSuffixes); i++) {
    sAlias = sVarMatrix + sSuffixes[i];
    sAlias.AssureInRepository();
    pDstVar = Cast<CMatrixVar>(pVars->FindVar(sAlias));
    if (!pDstVar)
      continue;
    sSuff = sSuffixes[i];
    sSuff >>= 1;
    bRes &= CConstantBuffer::SetMatrixVar(pDstVar, pMatVar, sSuff);
  }

  return bRes;
}

CTechnique::ECacheLocation CTechnique::GetStateLocation(CStateCache::EStateType eStateType)
{
  static const CStrAny sTechnique(ST_CONST, "Technique");
  static const CStrAny sMaterial(ST_CONST, "Material");
  static const CStrAny sModel(ST_CONST, "Model");

  static const CStrAny sRasterizerState(ST_CONST, "RasterizerState");
  static const CStrAny sBlendState(ST_CONST, "BlendState");
  static const CStrAny sDepthStencilState(ST_CONST, "DepthStencilState");

  static const CStrAny sLocationNames[] = { sRasterizerState, sBlendState, sDepthStencilState };

  CStrAny sLoc;
  if (m_pDefStates && m_pDefStates->GetStr(sLocationNames[eStateType], sLoc)) {
    if (sLoc == sTechnique)
      return CL_TECHNIQUE;
    if (sLoc == sMaterial)
      return CL_MATERIAL;
    if (sLoc == sModel)
      return CL_MODEL;
  }
  return CL_MATERIAL;
}

bool CTechnique::ApplyStateType(CStateCache::EStateType eStateType, CStateCache *pStateCache, CVarObj *pVars)
{
  CStateCache *pContainer = pStateCache->GetContainer(eStateType);
  ASSERT(pContainer);
  if (pContainer == &m_StateCache && m_uiStateCacheFrameID[eStateType] < CGraphics::Get()->m_uiFrameID) {
    if (!pContainer->m_pStates[eStateType]->ApplyVars(pVars, false))
      return false;
    m_uiStateCacheFrameID[eStateType] = CGraphics::Get()->m_uiFrameID;
  }
  if (!pContainer->m_pStates[eStateType]->Commit(false))
    return false;

  return true;
}

bool CTechnique::ApplyState(CVarObj *pVars, CStateCache *pStateCache)
{
  for (int i = 0; i < 3; i++) 
    if (!ApplyStateType((CStateCache::EStateType) i, pStateCache, pVars))
      return false;

  return true;
}

bool CTechnique::ApplyConstants(CVarObj *pModelVars)
{
  int i;
  bool bRes;
  CVar<CConstantCache *> vCache;
  for (i = 0; i < m_arrConstantBuffers.m_iCount; i++) {
    if (pModelVars->GetVar(m_arrConstantBuffers[i]->m_pTemplate->m_sCacheName, vCache)) 
      bRes = m_arrConstantBuffers[i]->SetFrom(vCache.Val());
    else
      bRes = m_arrConstantBuffers[i]->SetFrom(*pModelVars);
    if (!bRes)
      return false;
  }
  return true;
}

bool CTechnique::Apply(CVarObj *pModelVars, CStateCache *pStateCache)
{
  ASSERT(pModelVars);
  bool bRes;
  int i;

  ID3D11DeviceContext *pDC = CGraphics::Get()->m_pDeviceContext;
  CScopeLock kLock(&CGraphics::Get()->m_Lock);
  pDC->VSSetShader(m_pVS, 0, 0);
  pDC->PSSetShader(m_pPS, 0, 0);

/*  if (!ApplyState(pModelVars, true))
    return false;
*/
  if (!ApplyState(pModelVars, pStateCache))
    return false;

  if (!ApplyConstants(pModelVars))
    return false;

  for (i = 0; i < m_arrBinds.m_iCount; i++) {
    ASSERT(m_arrBinds[i].m_eShader == ST_VERTEX || m_arrBinds[i].m_eShader == ST_PIXEL);
    switch (m_arrBinds[i].m_eType) {
      case RT_SAMPLER: {
        CVar<CSampler *> varSampler;
        bRes = pModelVars->GetVar(m_arrBinds[i].m_sName, varSampler);
        if (!(bRes && varSampler.Val() && varSampler.Val()->IsValid())) {
          ASSERT(0);
          break;
        }
        switch (m_arrBinds[i].m_eShader) {
          case ST_VERTEX:
            pDC->VSSetSamplers(m_arrBinds[i].m_uiBindPoint, m_arrBinds[i].m_uiBindCount, &varSampler.Val()->m_pSampler);
            break;
          case ST_PIXEL:
            pDC->PSSetSamplers(m_arrBinds[i].m_uiBindPoint, m_arrBinds[i].m_uiBindCount, &varSampler.Val()->m_pSampler);
            break;
        }
      } break;
      case RT_TEXTURE: {
        CVar<CTexture *> varTex;
        bRes = pModelVars->GetVar(m_arrBinds[i].m_sName, varTex);
        if (!(bRes && varTex.Val() && varTex.Val()->IsValid())) {
          ASSERT(0);
          break;
        }
        switch (m_arrBinds[i].m_eShader) {
          case ST_VERTEX:
            pDC->VSSetShaderResources(m_arrBinds[i].m_uiBindPoint, m_arrBinds[i].m_uiBindCount, &varTex.Val()->m_pTextureView);
            break;
          case ST_PIXEL:
            pDC->PSSetShaderResources(m_arrBinds[i].m_uiBindPoint, m_arrBinds[i].m_uiBindCount, &varTex.Val()->m_pTextureView);
            break;
        }
      } break;
      case RT_CONSTANT_BUFFER:
        if (!m_arrBinds[i].m_pBuffer || !m_arrBinds[i].m_pBuffer->m_pBuffer) {
          ASSERT(0);
          break;
        }
        switch (m_arrBinds[i].m_eShader) {
          case ST_VERTEX:
            pDC->VSSetConstantBuffers(m_arrBinds[i].m_uiBindPoint, m_arrBinds[i].m_uiBindCount, &m_arrBinds[i].m_pBuffer->m_pBuffer->m_pD3DBuffer);
            break;
          case ST_PIXEL:
            pDC->PSSetConstantBuffers(m_arrBinds[i].m_uiBindPoint, m_arrBinds[i].m_uiBindCount, &m_arrBinds[i].m_pBuffer->m_pBuffer->m_pD3DBuffer);
            break;
        }
        break;
      default:
        ASSERT(0);
        return false;
    }
  }

  return true;
}

// CMatrixFamilyVar ----------------------------------------------------------
/*
IMPRTTI_NOCREATE(CMatrixFamilyVar, CMatrixVar);

bool CMatrixFamilyVar::AddFamilyVar(CMatrixVar *pVar, CStrPart sSuffix)
{
  int i;
  for (i = 0; i < ARRSIZE(m_Family); i++) 
    if (!m_Family[i].m_pVar) {
      m_Family[i].m_pVar = pVar;
      m_Family[i].m_sSuffix = sSuffix;
      return true;
    }
  return false;
}

bool CMatrixFamilyVar::SetVar(const CBaseVar &vSrc)
{
  bool bRes;
  int i;
  CMatrixVar const *pSrc = Cast<CMatrixVar>(&vSrc);
  if (!pSrc)
    return false;
  bRes = CMatrixVar::SetVar(vSrc);
  for (i = 0; i < ARRSIZE(m_Family) && m_Family[i].m_pVar; i++)
    bRes &= CConstantBuffer::SetMatrixVar(m_Family[i].m_pVar, pSrc, m_Family[i].m_sSuffix);
  return bRes;
}

CBaseVar *CMatrixFamilyVar::Clone() const
{
  CMatrixFamilyVar *pNewVar = new CMatrixFamilyVar(m_iRows, m_iCols, m_uiFlags, m_pVal);
  for (int i = 0; i < ARRSIZE(m_Family) && m_Family[i].m_pVar; i++)
    pNewVar->AddFamilyVar(m_Family[i].m_pVar, m_Family[i].m_sSuffix);
  return pNewVar;
}
*/