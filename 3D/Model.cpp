#include "stdafx.h"
#include "Model.h"
#include "VarUtil.h"
#include "Shape.h"
#include "FrameSorter.h"

// CMaterial -------------------------------------------------------------------------

IMPRTTI(CMaterial, CVarObj)

CMaterial::CMaterial()
{
  m_pTechnique = 0;
  m_pParams = 0;
  m_pPerMaterialCache = 0;
  m_bUseGlobals = true;
}

CMaterial::~CMaterial()
{
  Done();
}

bool CMaterial::Init(CTechnique *pTech, bool bUseGlobals, CVarObj *pInitParams, bool bOwnParams)
{
  ASSERT(!IsValid());
  m_pTechnique = pTech;
  m_bUseGlobals = bUseGlobals;
  m_pParams = bOwnParams ? pInitParams : new CVarHash();
  if (!m_pTechnique || !m_pParams)
    return false;
  if (!InitConstantCache())
    return false;
  if (!InitStates())
    return false;
  if (!bOwnParams && pInitParams) {
    CVarObj::CIter *pIt;
    for (pIt = pInitParams->GetIter(); *pIt; pIt->Next()) {
      if (m_pPerMaterialCache && m_pPerMaterialCache->m_pTemplate->m_sCacheName == pIt->GetName())
        continue;
      CMatrixVar *pMatVar = Cast<CMatrixVar>(pIt->GetValue());
      if (pMatVar)
        SetMatrixVar(pIt->GetName(), pMatVar);
      else
        m_pParams->SetVar(pIt->GetName(), *pIt->GetValue());
    }
    delete pIt;
  }
  return true;
}

bool CMaterial::InitConstantCache()
{
  static const CStrConst scbPerMaterial("cbPerMaterial");

  ASSERT(!m_pPerMaterialCache);
  CConstantBuffer *pCB = m_pTechnique->GetConstantBuffer(scbPerMaterial);
  if (!pCB) 
    return true;

  m_pPerMaterialCache = new CConstantCache();
  if (!m_pPerMaterialCache->Init(pCB->m_pTemplate, m_pParams))
    return false;

  return true;
}

bool CMaterial::InitStates()
{
  bool bCacheRast, bCacheBlend, bCacheDepth;

  bCacheRast = (m_pTechnique->GetStateLocation(CStateCache::ST_RASTERIZER) == CTechnique::CL_MATERIAL);
  bCacheBlend = (m_pTechnique->GetStateLocation(CStateCache::ST_BLEND) == CTechnique::CL_MATERIAL);
  bCacheDepth = (m_pTechnique->GetStateLocation(CStateCache::ST_DEPTHSTENCIL) == CTechnique::CL_MATERIAL);

  if (!m_StateCache.Init(bCacheRast, bCacheBlend, bCacheDepth, &m_pTechnique->m_StateCache, m_pTechnique->m_pDefStates))
    return false;

  return true;
}

void CMaterial::Done()
{
  m_StateCache.Done();
  SAFE_DELETE(m_pPerMaterialCache);
  SAFE_DELETE(m_pParams);
  m_pTechnique = 0;
}

bool CMaterial::IsValid()
{
  return m_pTechnique && m_pParams;
}

bool CMaterial::SetMatrixVar(CStrConst sVarMatrix, CMatrixVar const *pMatVar, CVarObj *pModelParams)
{
  return m_pTechnique->SetMatrixVar(GetApplyVars(pModelParams), sVarMatrix, pMatVar);
}

void CMaterial::SetVarUpdated()
{
  if (m_pPerMaterialCache)
    m_pPerMaterialCache->m_uiFrameUpdated = CGraphics::Get()->m_uiFrameID;
}

CVarObj *CMaterial::GetApplyVars(CVarObj *pModelParams)
{
  ASSERT(IsValid());
  if (!pModelParams && !m_bUseGlobals)
    return m_pParams;
  m_vApplyVars.ClearBaseVars();
  if (pModelParams)
    m_vApplyVars.AppendBaseVar(pModelParams, false);
  m_vApplyVars.AppendBaseVar(m_pParams, false);
  if (m_bUseGlobals)
    m_vApplyVars.AppendBaseVar(&CGraphics::Get()->m_Globals, false);

  return &m_vApplyVars;
}

bool CMaterial::Apply(CVarObj *pModelParams, CStateCache *pStateCache)
{
  ASSERT(IsValid());
  bool bRes;
  bRes = m_pTechnique->Apply(GetApplyVars(pModelParams), pStateCache);
  return bRes;
}

bool CMaterial::SetVar(const CStrBase &sVar, const CBaseVar &vSrc) 
{ 
  bool bRes;
  CMatrixVar const *pMatVar = Cast<CMatrixVar>(&vSrc);
  if (pMatVar)
    bRes = SetMatrixVar(sVar, pMatVar);
  else
    bRes = m_pParams->SetVar(sVar, vSrc); 
  SetVarUpdated();
  return bRes;
}

// CGeometry -------------------------------------------------------------------------

IMPRTTI(CGeometry, CObject)

CGeometry::CGeometry()
{
  m_ePrimitiveType = PT_NONE;
  m_uiVertices = 0;
  m_uiIndices = 0;
  m_pBound = 0;
}

CGeometry::~CGeometry() 
{
  Done(); 
}

bool CGeometry::Init(CInputDesc *pInputDesc, EPrimitiveType ePrimitiveType, 
                     UINT uiVertices, UINT uiIndices, void *pVertices, WORD *pIndices, 
                     UINT uiVBFlags, UINT uiIBFlags, CRTTI const *pBoundRTTI)
{
  SetInputDesc(pInputDesc);
  m_ePrimitiveType = ePrimitiveType;
  SetVertices(uiVertices, pVertices, uiVBFlags);
  SetIndices(uiIndices, pIndices, uiIBFlags);
  SetBoundType(pBoundRTTI, pVertices, pIndices);

  return IsValid();
}

void CGeometry::Done()
{
  m_pIB = 0;
  m_pVB = 0;
  m_pInputDesc = 0;
  m_uiVertices = 0;
  m_uiIndices = 0;
  SAFE_DELETE(m_pBound);
}

void CGeometry::SetInputDesc(CInputDesc *pInputDesc)
{
  Done();
  m_pInputDesc = pInputDesc;
}

void CGeometry::SetVertices(UINT uiVertices, void *pVertices, UINT uiFlags)
{
  ASSERT(m_pInputDesc);

  m_uiVertices = uiVertices;

  if (uiVertices > 0) {
    m_pVB = new CD3DBuffer();
    bool bRes = m_pVB->Init(CResource::RT_VERTEX, uiFlags, (BYTE *) pVertices, m_uiVertices * m_pInputDesc->GetSize());
    ASSERT(bRes);
  } else
    m_pVB = 0;
}

void CGeometry::SetIndices(UINT uiIndices, WORD *pIndices, UINT uiFlags)
{
  ASSERT(m_pInputDesc);

  m_uiIndices = uiIndices;

  if (uiIndices > 0) {
    m_pIB = new CD3DBuffer();
    bool bRes = m_pIB->Init(CResource::RT_INDEX, uiFlags, (BYTE *) pIndices, m_uiIndices * sizeof(WORD));
    ASSERT(bRes);
  } else
    m_pIB = 0;
}

void CGeometry::SetBoundType(CRTTI const *pBoundRTTI, void *pVertices, WORD *pIndices)
{
  SAFE_DELETE(m_pBound);
  if (pBoundRTTI) {
    ASSERT(pBoundRTTI->IsKindOf(&CShape3D::s_RTTI));
    m_pBound = (CShape3D *) pBoundRTTI->CreateInstance();
    ASSERT(m_uiVertices);
    CVector<3> *pPoints;
    if (pVertices)
      pPoints = (CVector<3> *) pVertices;
    else 
      pPoints = (CVector<3> *) m_pVB->Map();

    WORD *pInd;
    if (!pIndices && m_uiIndices)
      pInd = (WORD *) m_pIB->Map();
    else
      pInd = pIndices;

    static const CStrConst sPOSITION("POSITION");
    int iPosIndex, iStride;
    CInputDesc::TInputElement *pElem = m_pInputDesc->GetElementInfo(sPOSITION, 0, &iPosIndex);
    ASSERT(pElem && pElem->m_btElements >= 3);

    pPoints = (CVector<3> *) ((BYTE *) pPoints + m_pInputDesc->GetElementOffset(iPosIndex));
    iStride = m_pInputDesc->GetSize();

    if (!pInd)
      m_pBound->Init(pPoints, iStride, m_uiVertices);
    else { // In the case of indexed geometry, the bounding shape is built incrementally to avoid copying the used vertex coordinates to a temporary buffer
      for (UINT i = 0; i < m_uiIndices; i++)
        m_pBound->IncludePoint(Util::IndexStride(pPoints, iStride, pInd[i]));
    }

    if (pIndices != pInd)
      m_pIB->Unmap();

    if (!pVertices)
      m_pVB->Unmap();
  }
}

UINT CGeometry::GetVBVertexCount()
{
  return m_pVB->GetSize(0) / m_pInputDesc->GetSize();
}

UINT CGeometry::GetIBIndexCount()
{
  return m_pIB->GetSize(0) / sizeof(WORD);
}

bool CGeometry::IsValid()
{
  return m_pInputDesc && m_pVB && m_pIB && m_ePrimitiveType != PT_NONE && m_uiVertices && m_uiIndices;
}

bool CGeometry::Apply()
{
  ID3D11DeviceContext *pDC = CGraphics::Get()->m_pDeviceContext;

  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  pDC->IASetIndexBuffer(m_pIB->m_pD3DBuffer, DXGI_FORMAT_R16_UINT, 0);
  UINT uiStride, uiOffset;
  uiStride = m_pInputDesc->GetSize();
  uiOffset = 0;
  pDC->IASetVertexBuffers(0, 1, &m_pVB->m_pD3DBuffer, &uiStride, &uiOffset);
  pDC->IASetPrimitiveTopology(GetD3DPrimitiveTopology(m_ePrimitiveType));

  return true;
}

bool CGeometry::Render(UINT uiIndices, UINT uiStartIndex, UINT uiBaseVertex)
{
  ASSERT(uiStartIndex <= uiIndices);
  if (uiIndices == (UINT) -1)
    uiIndices = m_uiIndices - uiStartIndex;

  CGraphics::Get()->m_uiDrawPrimitiveCount++;
  ASSERT(m_ePrimitiveType == PT_TRIANGLELIST);
  CGraphics::Get()->m_uiTriangleCount += uiIndices / 3;
  CGraphics::Get()->m_uiVertexCount += m_uiVertices;

  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  CGraphics::Get()->m_pDeviceContext->DrawIndexed(uiIndices, uiStartIndex, uiBaseVertex);
  return true;
}

D3D11_PRIMITIVE_TOPOLOGY CGeometry::GetD3DPrimitiveTopology(EPrimitiveType ePT)
{
  switch (ePT) {
    case PT_NONE:
      return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    case PT_POINTLIST:
      return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    case PT_LINELIST:
      return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    case PT_LINESTRIP:
      return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case PT_TRIANGLELIST:
      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case PT_TRIANGLESTRIP:
      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    default:
      ASSERT(!"Unknown primitive type");
      return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
  }
}

// CProgressiveGeometry -------------------------------------------------------------

IMPRTTI(CProgressiveGeometry, CGeometry)

CProgressiveGeometry::CProgressiveGeometry()
{
  m_pCollapses = 0;
  m_uiCollapses = 0;
  m_uiCurCollapse = 0;
  m_uiMinVertices = 0;
  m_uiFlags = 0;
  m_pChangeCallback = 0;
}

CProgressiveGeometry::~CProgressiveGeometry()
{
  DoneProgressive();
}

void CProgressiveGeometry::Done()
{
  DoneProgressive();
  CGeometry::Done();
}

void CProgressiveGeometry::DoneProgressive()
{
  SAFE_DELETE(m_pCollapses);
  m_uiCollapses = 0;
  m_uiCurCollapse = 0;
  m_uiMinVertices = 0;
}

void CProgressiveGeometry::SetCollapses(UINT uiCollapses, UINT *pCollapses, bool bExplicitVertexCount)
{
  DoneProgressive();
  if (uiCollapses) {
    m_uiCollapses = uiCollapses;
    m_pCollapses = new UINT[m_uiCollapses];
    if (pCollapses)
      memcpy(m_pCollapses, pCollapses, m_uiCollapses * sizeof(UINT));
    m_uiCurCollapse = 0;
    if (bExplicitVertexCount)
      m_uiFlags |= PGF_EXPLICIT_VERTEX_COUNT;
    else
      m_uiFlags &= ~PGF_EXPLICIT_VERTEX_COUNT;

    m_uiMinVertices = CalcMinVertices();
  }
}

bool CProgressiveGeometry::SetActiveVertices(UINT uiVertices, WORD **pMappedIndices)
{
  WORD *pIndices = 0;

  if (!pMappedIndices)
    pMappedIndices = &pIndices;

  while (uiVertices < m_uiVertices)
    if (!CollapseVertex(*pMappedIndices)) {
      ASSERT(m_uiCurCollapse < m_uiCollapses || m_uiVertices == m_uiMinVertices || !m_pCollapses);
      break;
    }
  while (uiVertices > m_uiVertices)
    if (!UncollapseVertex(uiVertices, *pMappedIndices)) {
      ASSERT(m_uiCurCollapse || m_uiVertices == GetVBVertexCount());
      break;
    }

  if (pMappedIndices == &pIndices && *pMappedIndices)
    m_pIB->Unmap();

  return uiVertices == m_uiVertices;
}

UINT CProgressiveGeometry::CalcMinVertices()
{
  UINT uiCollapse, uiMinVertices;
  
  uiMinVertices = GetVBVertexCount();
  uiCollapse = 0;
  while (uiCollapse < m_uiCollapses) {
    if (m_uiFlags & PGF_EXPLICIT_VERTEX_COUNT) 
      uiMinVertices = m_pCollapses[uiCollapse + 2];
    else 
      if (m_pCollapses[uiCollapse] == uiMinVertices)
        uiMinVertices--;
    uiCollapse = GetNextCollapse(uiCollapse);
  }
  return uiMinVertices;
}

UINT CProgressiveGeometry::GetPrevCollapse(UINT uiCollapse)
{
  ASSERT(uiCollapse > 0);
  ASSERT(m_pCollapses[uiCollapse - 1] == INVALID_INDEX);
  uiCollapse--;
  while (uiCollapse > 0 && m_pCollapses[uiCollapse - 1] != INVALID_INDEX)
    uiCollapse--;
  return uiCollapse;
}

UINT CProgressiveGeometry::GetPrevCollapseVertices(UINT uiCollapse)
{
  ASSERT(m_uiFlags & PGF_EXPLICIT_VERTEX_COUNT);
  if (uiCollapse > 0) {
    UINT uiPrevCollapse = GetPrevCollapse(uiCollapse);
    return m_pCollapses[uiPrevCollapse + 2];
  } else
    return GetVBVertexCount();
}

UINT CProgressiveGeometry::GetNextCollapse(UINT uiCollapse)
{
  ASSERT(uiCollapse < m_uiCollapses);
  ASSERT(!uiCollapse || m_pCollapses[uiCollapse - 1] == INVALID_INDEX);
  while (m_pCollapses[uiCollapse] != INVALID_INDEX)
    uiCollapse++;
  return uiCollapse + 1;
}

bool CProgressiveGeometry::CollapseVertex(WORD *&pIndices)
{
  if (m_uiCurCollapse >= m_uiCollapses)
    return false;

  UINT uiOrgVertex, uiNewVertex, uiReplaceIndex, uiNewVertices, uiFirstReplace;
  uiOrgVertex = m_pCollapses[m_uiCurCollapse];
  uiNewVertex = m_pCollapses[m_uiCurCollapse + 1];
  if (m_uiFlags & PGF_EXPLICIT_VERTEX_COUNT) {
    uiNewVertices = m_pCollapses[m_uiCurCollapse + 2];
    uiReplaceIndex = m_uiCurCollapse + 3;
    m_uiVertices = uiNewVertices;
  } else {
    uiReplaceIndex = m_uiCurCollapse + 2;
    if (uiOrgVertex == m_uiVertices - 1)
      m_uiVertices--;
  }

  ASSERT(uiNewVertex < m_uiVertices);

  uiFirstReplace = uiReplaceIndex;

  if (!pIndices)
    pIndices = (WORD *) m_pIB->Map();

  while (m_pCollapses[uiReplaceIndex] != INVALID_INDEX) {
    UINT uiIndexOfIndex = m_pCollapses[uiReplaceIndex];
    ASSERT(uiIndexOfIndex < m_uiIndices);
    ASSERT(pIndices[uiIndexOfIndex] == uiOrgVertex);
    pIndices[uiIndexOfIndex] = uiNewVertex;
    uiReplaceIndex++;
  }

  m_uiCurCollapse = uiReplaceIndex + 1;
  ASSERT(m_uiCurCollapse <= m_uiCollapses);

  while (m_uiIndices > 0 && IsTriangleDegenerate(pIndices + m_uiIndices - 3))
    m_uiIndices -= 3;

  if (m_pChangeCallback)
    m_pChangeCallback->IndicesChanged(this, uiOrgVertex, uiNewVertex, pIndices, uiReplaceIndex - uiFirstReplace, m_pCollapses + uiFirstReplace);

  return true;
}

bool CProgressiveGeometry::UncollapseVertex(UINT uiVertThreshold, WORD *&pIndices)
{
  if (!m_uiCurCollapse)
    return false;

  UINT uiOrgVertex, uiNewVertex, uiReplaceIndex, uiNewVertices, uiFirstReplace;
  UINT uiCollapse = GetPrevCollapse(m_uiCurCollapse);
  uiOrgVertex = m_pCollapses[uiCollapse];
  uiNewVertex = m_pCollapses[uiCollapse + 1];
  if (m_uiFlags & PGF_EXPLICIT_VERTEX_COUNT) {
    uiReplaceIndex = uiCollapse + 3;
    uiNewVertices = GetPrevCollapseVertices(uiCollapse);
    if (uiNewVertices > uiVertThreshold)
      return false;
    m_uiVertices = uiNewVertices;
  } else {
    uiReplaceIndex = uiCollapse + 2;
    if (uiOrgVertex == m_uiVertices)
      m_uiVertices++;
  }

  ASSERT(uiOrgVertex < m_uiVertices);

  uiFirstReplace = uiReplaceIndex;

  if (!pIndices)
    pIndices = (WORD *) m_pIB->Map();

  while (m_pCollapses[uiReplaceIndex] != INVALID_INDEX) {
    UINT uiIndexOfIndex = m_pCollapses[uiReplaceIndex];
    if (uiIndexOfIndex >= m_uiIndices)
      m_uiIndices = uiIndexOfIndex + 3 - uiIndexOfIndex % 3;
    ASSERT(pIndices[uiIndexOfIndex] == uiNewVertex);
    pIndices[uiIndexOfIndex] = uiOrgVertex;
    uiReplaceIndex++;
  }

  m_uiCurCollapse = uiCollapse;

  if (m_pChangeCallback)
    m_pChangeCallback->IndicesChanged(this, uiNewVertex, uiOrgVertex, pIndices, uiReplaceIndex - uiFirstReplace, m_pCollapses + uiFirstReplace);

  return true;
}

// CModel ---------------------------------------------------------------------------

IMPRTTI(CModel, CVarObj)

CModel::CModel()
{
  m_pLayout = 0;
  m_pParams = 0;
  m_pBound = 0;
  m_pPerObjectCache = 0;
}

CModel::~CModel()
{
  Done();
}

bool CModel::Init(CGeometry *pGeom, CMaterial *pMaterial, CVarObj *pInitParams, bool bOwnParams, CRTTI const *pBoundRTTI)
{
  Done();
  m_pGeometry = pGeom;
  m_pMaterial = pMaterial;

  m_pLayout = m_pMaterial->m_pTechnique->GetInputLayout(pGeom->m_pInputDesc);
  m_pParams = bOwnParams ? pInitParams : new CVarHash();

  if (!InitConstantCache())
    return false;

  if (!InitStates())
    return false;

  if (!bOwnParams && pInitParams) {
    CVarObj::CIter *pIt = pInitParams->GetIter();
    while (*pIt) {
      CMatrixVar *pMatVar = Cast<CMatrixVar>(pIt->GetValue());
      if (pMatVar)
        SetMatrixVar(pIt->GetName(), pMatVar);
      else
        m_pParams->SetVar(pIt->GetName(), *pIt->GetValue());
      pIt->Next();
    }
    delete pIt;
  }

  if (!pBoundRTTI && pGeom->m_pBound)
    pBoundRTTI = pGeom->m_pBound->GetRTTI();
  ASSERT(!pBoundRTTI || pBoundRTTI->IsKindOf(&CShape3D::s_RTTI));
  if (pBoundRTTI) 
    m_pBound = (CShape3D *) pBoundRTTI->CreateInstance();

  if (!IsValid())
    return false;

  return true;
}

bool CModel::InitConstantCache()
{
  static const CStrConst scbPerObject("cbPerObject");

  ASSERT(!m_pPerObjectCache);
  CConstantBuffer *pCB = m_pMaterial->m_pTechnique->GetConstantBuffer(scbPerObject);
  if (!pCB)
    return true;

  m_pPerObjectCache = new CConstantCache();
  if (!m_pPerObjectCache->Init(pCB->m_pTemplate, m_pParams))
    return false;

  return true;
}

bool CModel::InitStates()
{
  bool bCacheRast, bCacheBlend, bCacheDepth;

  bCacheRast = (m_pMaterial->m_pTechnique->GetStateLocation(CStateCache::ST_RASTERIZER) == CTechnique::CL_MODEL);
  bCacheBlend = (m_pMaterial->m_pTechnique->GetStateLocation(CStateCache::ST_BLEND) == CTechnique::CL_MODEL);
  bCacheDepth = (m_pMaterial->m_pTechnique->GetStateLocation(CStateCache::ST_DEPTHSTENCIL) == CTechnique::CL_MODEL);

  if (!m_StateCache.Init(bCacheRast, bCacheBlend, bCacheDepth, &m_pMaterial->m_StateCache, m_pMaterial->m_pTechnique->m_pDefStates))
    return false;

  return true;
}

void CModel::Done()
{
  m_pGeometry = 0;
  m_pMaterial = 0;
  // Model doesn't own the layout so it just forgets the reference
  m_pLayout = 0;
  m_StateCache.Done();
  SAFE_DELETE(m_pPerObjectCache);
  SAFE_DELETE(m_pParams);
  SAFE_DELETE(m_pBound);
}

bool CModel::IsValid()
{
  return m_pGeometry && m_pGeometry->IsValid() && m_pMaterial && m_pMaterial->IsValid() && m_pLayout;
}

bool CModel::SetMatrixVar(CStrConst sVarMatrix, CMatrixVar const *pMatVar)
{
  return m_pMaterial->SetMatrixVar(sVarMatrix, pMatVar, m_pParams);
}

void CModel::SetVarUpdated(bool bMaterialUpdated)
{
  if (m_pPerObjectCache)
    m_pPerObjectCache->m_uiFrameUpdated = CGraphics::Get()->m_uiFrameID;
  if (bMaterialUpdated)
    m_pMaterial->SetVarUpdated();
}

bool CModel::UpdateBound()
{
  if (!m_pBound)
    return true;
  static const CStrConst sg_mWorld("g_mWorld");
/*  CMatrixVar *pMatVar = Cast<CMatrixVar>(m_pParams->FindVar(sg_mWorld));
  if (!pMatVar || pMatVar->m_iCols != 4 || pMatVar->m_iRows != 4) {
    ASSERT(0);
    return false;
  }
*/
  CMatrixVar vWorld(4, 4);
  CXForm kXForm((CMatrix<4, 4> *) vWorld.m_pVal);
  if (!m_pParams->GetVar(sg_mWorld, vWorld)) {
    ASSERT(0);
    return false;
  }
//  CXForm kXForm((CMatrix<4, 4> *) pMatVar->m_pVal);
  m_pGeometry->m_pBound->GetTransformed(kXForm, m_pBound);
  // Forget the reference to the matrix var's data so it doesn't get deleted in transform destruction
  kXForm.m_pMatrix = 0;

  return true;
}

bool CModel::Apply()
{
  if (!m_pMaterial->Apply(m_pParams, &m_StateCache))
    return false;
  if (!m_pGeometry->Apply())
    return false;

  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  CGraphics::Get()->m_pDeviceContext->IASetInputLayout(m_pLayout);

  return true;
}

bool CModel::DoRender()
{
  if (!Apply())
    return false;
  if (!m_pGeometry->Render())
    return false;
  return true;
}

bool CModel::Render()
{
/*  if (!UpdateBound())
    return false;
*/
  if (!CGraphics::Get()->CheckVisibility(m_pBound))
    return true;
  if (!CGraphics::Get()->m_pFrameSorter->Add(this))
    return false;
  return true;
}

bool CModel::SetVar(const CStrBase &sVar, const CBaseVar &vSrc)
{
  bool bRes;
  CMatrixVar const *pMatVar = Cast<CMatrixVar>(&vSrc);
  if (pMatVar)
    bRes = SetMatrixVar(sVar, pMatVar);
  else
    bRes = GetApplyVars()->SetVar(sVar, vSrc); 
  SetVarUpdated(false);
  return bRes;
}

