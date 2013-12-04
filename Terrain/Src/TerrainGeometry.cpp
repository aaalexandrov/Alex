#include "stdafx.h"
#include "Terrain.h"
#include "Shape.h"
#include "Camera.h"
#include "GeomUtil.h"

// CTerrain::CPatch::CEdgeMap -------------------------------------
CTerrain::CPatch::CEdgeMap::CEdgeMap()
{
}

bool CTerrain::CPatch::CEdgeMap::Init(CMesh &kMesh)
{
  int i;
  for (i = 0; i < EDGE_INDICES; i++) 
    SetGridIndex(i, kMesh);
  return true;
}

bool CTerrain::CPatch::CEdgeMap::Init(uint16_t *pEdgeIndices)
{
  int i;
  for (i = 0; i < EDGE_INDICES; i++) {
    m_wEdgeIndices[i] = pEdgeIndices[i];
    m_hashReverseEdgeIndices.Add(TKeyValue<uint16_t, uint16_t>(m_wEdgeIndices[i], i));
  }
  return true;
}

void CTerrain::CPatch::CEdgeMap::SetGridIndex(UINT uiIndex, CMesh &kMesh)
{
  CVector<2, int> vCoord = EdgeIndex2Grid(uiIndex);
  m_wEdgeIndices[uiIndex] = kMesh.m_arrReverseReorder[vCoord.y() * PATCH_SIZE + vCoord.x() + EDGE_INDICES];
  m_hashReverseEdgeIndices.Add(TKeyValue<uint16_t, uint16_t>(m_wEdgeIndices[uiIndex], uiIndex));
}

UINT CTerrain::CPatch::CEdgeMap::Vertex2Edge(UINT uiVertexIndex)
{
  CHashKV<uint16_t, uint16_t>::TIter it;
  it = m_hashReverseEdgeIndices.Find(uiVertexIndex);
  if (!it)
    return -1;
  return (*it).m_Val;
}

// CTerrain::CPatch::CEdgedGeom -----------------------------------------------

CTerrain::CPatch::CEdgedGeom::CEdgedGeom(CGeometry *pGeometry, CEdgeInfo *pEdgeInfo)
{
  m_pGeometry = pGeometry;
  m_pEdgeInfo = pEdgeInfo;

  CProgressiveGeometry *pProgGeom = Cast<CProgressiveGeometry>(m_pGeometry);
  if (pProgGeom) {
    ASSERT(!pProgGeom->m_pChangeCallback);
    pProgGeom->m_pChangeCallback = this;
  }

  InitEdgeTriangles();
}

CTerrain::CPatch::CEdgedGeom::~CEdgedGeom()
{
  CProgressiveGeometry *pProgGeom = Cast<CProgressiveGeometry>(m_pGeometry);
  if (pProgGeom)
    pProgGeom->m_pChangeCallback = 0;
}

void CTerrain::CPatch::CEdgedGeom::InitEdgeTriangles()
{
  int i;

  m_mapEdgeTriangles.Clear();

  uint16_t *pIndices = (uint16_t *) m_pGeometry->m_pIB->Map();

  UINT uiIndices = m_pGeometry->m_uiIndices;

  for (i = 0; i < (int) uiIndices; i += 3) {
    UINT uiEdgeInd0, uiEdgeInd1, uiOtherEdgeInd;
    uint8_t btOtherIndOfs;
    ASSERT(pIndices[i] >= EDGE_INDICES && pIndices[i+1] >= EDGE_INDICES && pIndices[i+2] >= EDGE_INDICES);
    if (!CheckTriangleForEdge(pIndices + i, 0, uiEdgeInd0, uiEdgeInd1, uiOtherEdgeInd, btOtherIndOfs))
      continue;
    m_mapEdgeTriangles.Add(i);
  }

  m_pGeometry->m_pIB->Unmap();
}

void CTerrain::CPatch::CEdgedGeom::OverwriteTriangle(uint16_t *pStartInd)
{
  m_arrOverwrittenIndices.Append(pStartInd[0]);
  m_arrOverwrittenIndices.Append(pStartInd[1]);
  m_arrOverwrittenIndices.Append(pStartInd[2]);
}

void CTerrain::CPatch::CEdgedGeom::IndicesChanged(CProgressiveGeometry *pGeometry, uint16_t wOldIndex, uint16_t wCurIndex, uint16_t *pIndices, UINT uiChanges, UINT *pIndicesOfIndices)
{
  TTriangleMap::TIter itTri;
  UINT uiChange, uiTri;

  if (m_pEdgeInfo->Vertex2Edge(wOldIndex) == -1 && m_pEdgeInfo->Vertex2Edge(wCurIndex) == -1)
    return;
  for (uiChange = 0; uiChange < uiChanges; uiChange++) {
    uiTri = pIndicesOfIndices[uiChange] - (pIndicesOfIndices[uiChange] % 3);
    itTri = m_mapEdgeTriangles.Find(uiTri);
    UINT uiEdgeInd0, uiEdgeInd1, uiOtherEdgeInd;
    uint8_t btOtherIndOfs;
    if (!CProgressiveGeometry::IsTriangleDegenerate(pIndices + uiTri) &&
        CheckTriangleForEdge(pIndices + uiTri, 0, uiEdgeInd0, uiEdgeInd1, uiOtherEdgeInd, btOtherIndOfs)) {
      if (!itTri)
        m_mapEdgeTriangles.Add(uiTri);
    } else {
      ASSERT(CProgressiveGeometry::IsTriangleDegenerate(pIndices + uiTri) || 
             !IsSplittableEdge(pIndices[uiTri], pIndices[uiTri + 1]) &&
             !IsSplittableEdge(pIndices[uiTri + 1], pIndices[uiTri + 2]) &&
             !IsSplittableEdge(pIndices[uiTri + 2], pIndices[uiTri]));
      if (itTri)
        m_mapEdgeTriangles.Remove(itTri);
    }
  }
}

bool CTerrain::CPatch::CEdgedGeom::CheckTriangleForEdge(uint16_t *pTriInd, CBitArray<EDGE_INDICES> const *pActive, UINT &uiEdgeInd0, UINT &uiEdgeInd1, UINT &uiOtherEdgeInd, uint8_t &btOtherIndOfs)
{
  UINT uiEdgeInd[3];
  int iNotFound, i, iNegIndex;

  iNotFound = 0;
  iNegIndex = -1;
  for (i = 0; i < 3 && iNotFound < 2; i++) {
    uiEdgeInd[i] = m_pEdgeInfo->Vertex2Edge(pTriInd[i]);
    if (uiEdgeInd[i] == -1) {
      iNegIndex = i;
      iNotFound++;
    }
  }
  if (iNotFound >= 2)
    return false;

  int iInd1, iInd2;
  UINT uiActive;
  if (iNegIndex < 0) {
    for (iNegIndex = 0; iNegIndex < 3; iNegIndex++) {
      if (EdgeIndexOnCorner(uiEdgeInd[iNegIndex])) // Corner vertex, the opposite side is not eligible to be split in a fan
        continue;
      iInd1 = uiEdgeInd[(iNegIndex + 1) % 3];
      iInd2 = uiEdgeInd[(iNegIndex + 2) % 3];
      if (!EdgeIndicesOnSameSide(iInd1, iInd2))
        continue;
      uiActive = FirstEdgeIndexBetween(iInd1, iInd2, pActive); 
      if (uiActive != (UINT) -1)
        break;
    }
    if (iNegIndex >= 3)
      return false;
  } else {
    iInd1 = uiEdgeInd[(iNegIndex + 1) % 3];
    iInd2 = uiEdgeInd[(iNegIndex + 2) % 3];
    if (!EdgeIndicesOnSameSide(iInd1, iInd2))
      return false;
    uiActive = FirstEdgeIndexBetween(iInd1, iInd2, pActive);
    if (uiActive == (UINT) -1)
      return false;
  }

  uiEdgeInd0 = iInd1;
  uiEdgeInd1 = iInd2;
  uiOtherEdgeInd = uiEdgeInd[iNegIndex];
  btOtherIndOfs = (uint8_t) iNegIndex;

  return true;
}

UINT CTerrain::CPatch::CEdgedGeom::AppendEdgeFan(uint16_t *pIndices, UINT uiAppendInd, UINT uiEdgeTriInd, CBitArray<EDGE_INDICES> const &kActive, UINT uiEdgeInd0, UINT uiEdgeInd1, UINT uiOtherEdgeInd, uint16_t wOtherInd)
{
  UINT uiAdded, uiActive;
  int iSubstInd;
  int iDelta, iCur;
  uint16_t wPrev, *pStartInd;

  if (uiOtherEdgeInd != (UINT) -1) {
    if (EdgeIndexOnCorner(uiEdgeInd0)) {
      if (FirstEdgeIndexBetween(uiOtherEdgeInd, uiEdgeInd0, &kActive) != (UINT) -1) {
        uiActive = FirstEdgeIndexBetween(uiEdgeInd0, uiEdgeInd1, &kActive);
        if (uiActive != (UINT) -1) {
          uiAdded = AppendEdgeFan(pIndices, uiAppendInd, uiEdgeTriInd, kActive, uiActive, uiEdgeInd1, -1, wOtherInd);
          uiAdded += AppendEdgeFan(pIndices, uiAppendInd + uiAdded, -1, kActive, uiOtherEdgeInd, uiEdgeInd0, -1, (uint16_t) uiActive);
          return uiAdded;
        }
      }
    } else {
      if (EdgeIndexOnCorner(uiEdgeInd1)) {
        if (FirstEdgeIndexBetween(uiEdgeInd1, uiOtherEdgeInd, &kActive) != (UINT) -1) {
          uiActive = FirstEdgeIndexBetween(uiEdgeInd1, uiEdgeInd0, &kActive);
          if (uiActive != (UINT) -1) {
            uiAdded = AppendEdgeFan(pIndices, uiAppendInd, uiEdgeTriInd, kActive, uiEdgeInd0, uiActive, -1, wOtherInd);
            uiAdded += AppendEdgeFan(pIndices, uiAppendInd + uiAdded, -1, kActive, uiEdgeInd1, uiOtherEdgeInd, -1, (uint16_t) uiActive);
            return uiAdded;
          }
        }
      }
    }
  }

  uiAdded = 0;
  if (!uiEdgeInd0 && uiEdgeInd1 >= 3 * (PATCH_SIZE - 1))
    uiEdgeInd0 = EDGE_INDICES;
  else
    if (!uiEdgeInd1 && uiEdgeInd0 >= 3 * (PATCH_SIZE - 1))
      uiEdgeInd1 = EDGE_INDICES;

  pStartInd = pIndices + uiAppendInd;
  iDelta = Util::Sign((int) uiEdgeInd1 - (int) uiEdgeInd0);
  wPrev = (uint16_t) uiEdgeInd0;
  for (iCur = (int) uiEdgeInd0 + iDelta; iCur != (int) uiEdgeInd1; iCur += iDelta) {
    if (!kActive.GetBit(iCur))
      continue;
    OverwriteTriangle(pStartInd + uiAdded);
    pStartInd[uiAdded] = wPrev;
    pStartInd[uiAdded + 1] = (uint16_t) iCur;
    pStartInd[uiAdded + 2] = wOtherInd;
    wPrev = (uint16_t) iCur;
    uiAdded += 3;
  }
  if (uiEdgeTriInd == (UINT) -1) { 
    // Append last triangle at the end
    pStartInd += uiAdded;
    OverwriteTriangle(pStartInd);
    uiAdded += 3;
  } else { // Replace original edge triangle
    iSubstInd = m_arrTriangleSubstitutions.m_iCount;
    m_arrTriangleSubstitutions.SetCount(iSubstInd + 1);
    m_arrTriangleSubstitutions[iSubstInd].Init(pIndices, uiEdgeTriInd);
    pStartInd = pIndices + uiEdgeTriInd;
  }

  pStartInd[0] = wPrev;
  pStartInd[1] = (uint16_t) (uiEdgeInd1 % EDGE_INDICES);
  pStartInd[2] = wOtherInd;

  return uiAdded;
}

void CTerrain::CPatch::CEdgedGeom::ActivateEdges(uint16_t *&pIndices, CBitArray<EDGE_INDICES> const &kActive)
{
  int i;
  UINT uiAppendIndex;
  TTriangleMap::TIter it;

  if (!pIndices)
    pIndices = (uint16_t *) m_pGeometry->m_pIB->Map();

  UINT uiIndices = m_pGeometry->m_uiIndices;

  uiAppendIndex = uiIndices;
  for (it = m_mapEdgeTriangles; it; ++it) {
    UINT uiEdgeInd0, uiEdgeInd1, uiOtherEdgeInd;
    uint8_t btOtherIndOfs;

    i = *it;
    if (!CheckTriangleForEdge(pIndices + i, &kActive, uiEdgeInd0, uiEdgeInd1, uiOtherEdgeInd, btOtherIndOfs))
      continue;
    uint16_t wOtherInd = pIndices[i + btOtherIndOfs];
    ASSERT(wOtherInd >= EDGE_INDICES);
    uiAppendIndex += AppendEdgeFan(pIndices, uiAppendIndex, i, kActive, uiEdgeInd0, uiEdgeInd1, uiOtherEdgeInd, wOtherInd);
  }
  ASSERT(uiAppendIndex <= m_pGeometry->GetIBIndexCount());

  m_pGeometry->m_uiIndices = uiAppendIndex;
}

void CTerrain::CPatch::CEdgedGeom::DeactivateEdges(uint16_t *&pIndices)
{
  //ASSERT(m_bBordersUpdated);
  int i;

  if (!m_arrTriangleSubstitutions.m_iCount && !m_arrOverwrittenIndices.m_iCount)
    return;

  if (!pIndices)
    pIndices = (uint16_t *) m_pGeometry->m_pIB->Map();

  for (i = 0; i < m_arrTriangleSubstitutions.m_iCount; i++) 
    m_arrTriangleSubstitutions[i].Revert(pIndices);

  m_arrTriangleSubstitutions.SetCount(0);

  if (m_arrOverwrittenIndices.m_iCount) {
    memcpy(pIndices + m_pGeometry->m_uiIndices - m_arrOverwrittenIndices.m_iCount, 
           m_arrOverwrittenIndices.m_pArray, m_arrOverwrittenIndices.m_iCount * sizeof(uint16_t));
    m_pGeometry->m_uiIndices -= m_arrOverwrittenIndices.m_iCount;
    m_arrOverwrittenIndices.SetCount(0);
  }
}

bool CTerrain::CPatch::CEdgedGeom::IsSplittableEdge(uint16_t wInd0, uint16_t wInd1)
{
  UINT uiEdgeInd0, uiEdgeInd1;
  uiEdgeInd0 = m_pEdgeInfo->Vertex2Edge(wInd0);
  if (uiEdgeInd0 == -1)
    return false;
  uiEdgeInd1 = m_pEdgeInfo->Vertex2Edge(wInd1);
  if (uiEdgeInd1 == -1)
    return false;
  if (!EdgeIndicesOnSameSide(uiEdgeInd0, uiEdgeInd1))
    return false;
  if (EdgeIndicesAdjacent(uiEdgeInd0, uiEdgeInd1))
    return false;
  return true;
}

UINT CTerrain::CPatch::CEdgedGeom::FirstEdgeIndexBetween(UINT uiEdgeInd0, UINT uiEdgeInd1, CBitArray<EDGE_INDICES> const *pActive)
{
  int iDelta, iCur;
  if (!uiEdgeInd0 && uiEdgeInd1 >= (PATCH_SIZE - 1) * 3)
    uiEdgeInd0 = EDGE_INDICES;
  else
    if (!uiEdgeInd1 && uiEdgeInd0 >= (PATCH_SIZE - 1) * 3)
      uiEdgeInd1 = EDGE_INDICES;
  iDelta = Util::Sign((int) uiEdgeInd1 - (int) uiEdgeInd0);
  if (!pActive) {
    if ((int) uiEdgeInd0 + iDelta == (int) uiEdgeInd1)
      return -1;
    return uiEdgeInd0 + iDelta;
  }
  for (iCur = (int) uiEdgeInd0 + iDelta; iCur != (int) uiEdgeInd1; iCur += iDelta)
    if (pActive->GetBit(iCur))
      return iCur;
  return -1;
}

// CTerrain::CPatch::CPatchGeom -----------------------------------------------
CRTTIRegisterer<CTerrain::CPatch::CPatchGeom> g_RegTerrainPatchPatchGeom;

bool CTerrain::CPatch::CPatchGeom::DetermineActiveEdgeIndices(CBitArray<EDGE_INDICES> &kActive, bool bAdjacentMinDetail)
{
  int i, iInd;
  CPatch *pUp, *pLeft, *pRight, *pDown;
  bool bActive, bExtraActive = false;
  UINT uiAdjActiveVert = bAdjacentMinDetail ? 0 : -1;
  m_pPatch->GetAdjacentPatches(pLeft, pUp, pRight, pDown);
  ASSERT(EdgeIndexOnCorner(Grid2EdgeIndex(0, 0)));
  kActive.SetBit(Grid2EdgeIndex(0, 0));
  ASSERT(EdgeIndexOnCorner(Grid2EdgeIndex(0, PATCH_SIZE - 1)));
  kActive.SetBit(Grid2EdgeIndex(0, PATCH_SIZE - 1));
  ASSERT(EdgeIndexOnCorner(Grid2EdgeIndex(PATCH_SIZE - 1, PATCH_SIZE - 1)));
  kActive.SetBit(Grid2EdgeIndex(PATCH_SIZE - 1, PATCH_SIZE - 1));
  ASSERT(EdgeIndexOnCorner(Grid2EdgeIndex(PATCH_SIZE - 1, 0)));
  kActive.SetBit(Grid2EdgeIndex(PATCH_SIZE - 1, 0));
  for (i = 1; i < PATCH_SIZE - 1; i++) {
    iInd = Grid2EdgeIndex(i, 0);
    if (m_pPatch->m_pGeom->IsEdgeIndexActive(iInd))
      kActive.SetBit(iInd);
    else {
      bActive = pUp && pUp->m_pGeom->IsEdgeIndexActive(Grid2EdgeIndex(i, PATCH_SIZE - 1), uiAdjActiveVert);
      kActive.SetBit(iInd, bActive);
      bExtraActive |= bActive;
    }
    iInd = Grid2EdgeIndex(PATCH_SIZE - 1, i);
    if (m_pPatch->m_pGeom->IsEdgeIndexActive(iInd))
      kActive.SetBit(iInd);
    else {
      bActive = pRight && pRight->m_pGeom->IsEdgeIndexActive(Grid2EdgeIndex(0, i), uiAdjActiveVert);
      kActive.SetBit(iInd, bActive);
      bExtraActive |= bActive;
    }
    iInd = Grid2EdgeIndex(i, PATCH_SIZE - 1);
    if (m_pPatch->m_pGeom->IsEdgeIndexActive(iInd))
      kActive.SetBit(iInd);
    else {
      bActive = pDown && pDown->m_pGeom->IsEdgeIndexActive(Grid2EdgeIndex(i, 0), uiAdjActiveVert);
      kActive.SetBit(iInd, bActive);
      bExtraActive |= bActive;
    }
    iInd = Grid2EdgeIndex(0, i);
    if (m_pPatch->m_pGeom->IsEdgeIndexActive(iInd))
      kActive.SetBit(iInd);
    else {
      bActive = pLeft && pLeft->m_pGeom->IsEdgeIndexActive(Grid2EdgeIndex(PATCH_SIZE - 1, i), uiAdjActiveVert);
      kActive.SetBit(iInd, bActive);
      bExtraActive |= bActive;
    }
  }
  return bExtraActive;
}

void CTerrain::CPatch::CPatchGeom::UpdateAdjacentEdges()
{
  CPatch *pAdj[4];
  int i;

  m_pPatch->GetAdjacentPatches(pAdj[0], pAdj[1], pAdj[2], pAdj[3]);
  for (i = 0; i < 4; i++) {
    if (!pAdj[i])
      continue;
    pAdj[i]->m_pGeom->UpdateEdges();
  }
}

// CTerrain::CPatch::CFullGeom ------------------------------------------------
CRTTIRegisterer<CTerrain::CPatch::CFullGeom> g_RegTerrainPatchFullGeom;

CTerrain::CPatch::CFullGeom::CFullGeom()
{
  m_pLODEdgedGeom = 0;
  m_bBordersUpdated = false;
  m_bUseLODModel = false;
}

CTerrain::CPatch::CFullGeom::~CFullGeom()
{
  Done();
}

bool CTerrain::CPatch::CFullGeom::Init(CPatch *pPatch, bool bInitMaterialModels)
{
  if (!CPatchGeom::Init(pPatch))
    return false;

  ASSERT(m_pPatch->m_pModel || !bInitMaterialModels && m_pPatch->m_pMinLODModel);
  if (m_pPatch->m_pMinLODModel) {
    if (!InitMinLODModel(m_pPatch->m_pMinLODModel))
      return false;
  } else
    m_pLODModel = m_pPatch->m_pModel;

  if (!m_pLODModel)
    return false;

  if (bInitMaterialModels && !InitMaterialModels(EDGE_INDICES))
    return false;

  m_bUseLODModel = !bInitMaterialModels;

  return true;
}

void CTerrain::CPatch::CFullGeom::Done()
{
  if (m_pLODEdgedGeom && m_bBordersUpdated)
    UpdateEdges();
  DoneMaterialModels();
  SAFE_DELETE(m_pLODEdgedGeom);
  m_pLODModel = 0;
  CPatchGeom::Done();
}

bool CTerrain::CPatch::CFullGeom::InitMaterialModels(UINT uiReservedVertices)
{
  CAVLTree<int> kUsedMats;
  CAVLTree<int>::TIter it;

  m_pPatch->GetUsedMaterials(kUsedMats);
  for (it = kUsedMats; it; ++it)
    if (!InitMaterialModel(*it, uiReservedVertices))
      return false;

  return true;
}

bool CTerrain::CPatch::CFullGeom::InitMaterialModel(int iMaterial, UINT uiReservedVertices)
{
  CGeometry *pGeom;
  CModel *pModel;
  CMaterial *pMaterial;

  pMaterial = GetTerrain()->GetMaterial(iMaterial);
  pGeom = new CGeometry();
  pGeom->Init(pMaterial->m_pTechnique->m_pInputDesc, CGeometry::PT_TRIANGLELIST, 0, 0);

  pGeom->m_uiVertices = m_pPatch->m_pModel->m_pGeometry->m_uiVertices;
  pGeom->m_pVB = m_pPatch->m_pModel->m_pGeometry->m_pVB;

  m_pPatch->InitIB(pGeom, iMaterial, uiReservedVertices);

//  pGeom->SetBoundType(&CAABB::s_RTTI);
  pGeom->m_pBound = m_pPatch->GetAABB(true).Clone();

  pModel = new CModel();
  pModel->Init(pGeom, pMaterial, 0, false);

  m_pPatch->InitParams(pModel, iMaterial, m_arrMaterialModels.m_iCount);

  m_arrMaterialModels.Append(new TMatModel(pModel, iMaterial));

  return true;
}

bool CTerrain::CPatch::CFullGeom::InitMinLODModel(CModel *pLODModel)
{
  ASSERT(!m_pLODEdgedGeom);
  m_pLODModel = pLODModel;

  m_bUseLODModel = true;
  m_pLODEdgedGeom = new CEdgedGeom(m_pLODModel->m_pGeometry, &m_pPatch->m_EdgeMap);
  m_bBordersUpdated = false;

  return true;
}

void CTerrain::CPatch::CFullGeom::DoneMaterialModels()
{
  m_arrMaterialModels.DeleteAll();
}

void CTerrain::CPatch::CFullGeom::UpdateEdges()
{
  if (m_pLODEdgedGeom) {
    uint16_t *pIndices = 0;
    m_pLODEdgedGeom->DeactivateEdges(pIndices);
    if (pIndices)
      m_pLODEdgedGeom->m_pGeometry->m_pIB->Unmap();
  }
  m_bBordersUpdated = false;
}

bool CTerrain::CPatch::CFullGeom::IsEdgeIndexActive(UINT uiEdgeInd, UINT uiActiveVert) 
{ 
  if (!m_bUseLODModel && uiActiveVert || !m_pPatch->m_EdgeMap.IsInitialized())
    return true; 
  switch (uiActiveVert) {
    case -1:
      uiActiveVert = m_bUseLODModel ? m_pLODModel->m_pGeometry->m_uiVertices : -1;
      break;
    case 0:
      uiActiveVert = m_pLODModel->m_pGeometry->m_uiVertices;
      break;
  }
  return m_pPatch->m_EdgeMap.m_wEdgeIndices[uiEdgeInd] < uiActiveVert;
}

void CTerrain::CPatch::CFullGeom::UpdateLOD(CCamera *pCamera)
{
  CAABB kAABB;
  CPoint3D kCamPos;
  float fDist;

  bool bWasUsingLOD = m_bUseLODModel;
  if (m_arrMaterialModels.m_iCount) {
    kAABB = m_pPatch->GetAABB(false);

    kCamPos.m_vPoint = pCamera->m_XForm.GetTranslation();
    fDist = kCamPos.GetDistance(&kAABB);

    m_bUseLODModel = fDist > GetTerrain()->m_fLODDistance;
  } else
    m_bUseLODModel = true;

  if (bWasUsingLOD != m_bUseLODModel) {
    UpdateEdges();
    UpdateAdjacentEdges();
  }
}

void CTerrain::CPatch::CFullGeom::SetMinLOD(bool bSetAdjacent)
{
  m_bUseLODModel = true;
  UpdateBorders(bSetAdjacent);
}

bool CTerrain::CPatch::CFullGeom::IsMinLOD()
{
  return m_bUseLODModel;
}

void CTerrain::CPatch::CFullGeom::SetModelVar(CStrAny sVar, CBaseVar const &vSrc)
{
  int i;
  for (i = 0; i < m_arrMaterialModels.m_iCount; i++)
    m_arrMaterialModels[i]->m_pModel->SetVar(sVar, vSrc);
}

bool CTerrain::CPatch::CFullGeom::Render()
{
  int i;
  bool bRes;

  if (m_bUseLODModel) {
    ASSERT(!m_pLODEdgedGeom || m_pLODEdgedGeom->m_pGeometry == m_pLODModel->m_pGeometry);
    if (!m_bBordersUpdated && m_pLODEdgedGeom) {
      CBitArray<EDGE_INDICES> kActive;
      bool bExtraActive;
      uint16_t *pIndices = 0;

      m_pLODEdgedGeom->DeactivateEdges(pIndices);
      bExtraActive = DetermineActiveEdgeIndices(kActive, false);
      if (bExtraActive)
        m_pLODEdgedGeom->ActivateEdges(pIndices, kActive);
      if (pIndices) 
        m_pLODEdgedGeom->m_pGeometry->m_pIB->Unmap();
      m_bBordersUpdated = true;
    }
    return m_pLODModel->Render();
  }

  bRes = true;
  for (i = 0; i < m_arrMaterialModels.m_iCount; i++)
    bRes &= m_arrMaterialModels[i]->m_pModel->Render();

  return bRes;
}

void CTerrain::CPatch::CFullGeom::UpdateBorders(bool bAdjacentMinDetail)
{
  ASSERT(!m_pLODEdgedGeom || m_pLODEdgedGeom->m_pGeometry == m_pLODModel->m_pGeometry);
  if (!m_bBordersUpdated && m_pLODEdgedGeom) {
    CBitArray<EDGE_INDICES> kActive;
    bool bExtraActive;
    uint16_t *pIndices = 0;

    m_pLODEdgedGeom->DeactivateEdges(pIndices);
    bExtraActive = DetermineActiveEdgeIndices(kActive, bAdjacentMinDetail);
    if (bExtraActive)
      m_pLODEdgedGeom->ActivateEdges(pIndices, kActive);
    if (pIndices) 
      m_pLODEdgedGeom->m_pGeometry->m_pIB->Unmap();
    m_bBordersUpdated = true;
  }
}

// CTerrain::CPatch::CProgGeom::TMaterialModel -------------------------------------------

CTerrain::CPatch::CProgGeom::TMaterialModel::TMaterialModel(CModel *pModel, int iMaterial, CProgGeom *pProgGeom)
{
  m_pModel = pModel;
  m_iMaterial = iMaterial;
  m_pEdgedGeom = new CEdgedGeom(m_pModel->m_pGeometry, &pProgGeom->m_pPatch->m_EdgeMap);
}

CTerrain::CPatch::CProgGeom::TMaterialModel::~TMaterialModel()
{
  delete m_pEdgedGeom;
}

// CTerrain::CPatch::CProgGeom ------------------------------------------------

CRTTIRegisterer<CTerrain::CPatch::CProgGeom> g_RegTerrainPatchProgGeom;

CTerrain::CPatch::CProgGeom::CProgGeom()
{
  m_uiActiveVertices = -1;
  m_uiMinActiveVertices = 0;
  m_uiMinVerticesToSet = -1;
  m_uiMinLODVertices = 0;
  m_uiMaxVertices = -1;
  m_bBordersUpdated = false;
  m_pLODMaterialModel = 0;
  m_bUseLODModel = false;
}

CTerrain::CPatch::CProgGeom::~CProgGeom()
{
  Done();
}

bool CTerrain::CPatch::CProgGeom::Init(CPatch *pPatch, CPatchModelBuilder *pBuilder)
{
  if (!CPatchGeom::Init(pPatch))
    return false;

  ASSERT(pPatch == pBuilder->m_pPatch);
  if (!InitProgressiveMaterials(pBuilder))
    return false;

  return true;
}

void CTerrain::CPatch::CProgGeom::Done()
{
  DoneModels();
  CPatchGeom::Done();
}

void CTerrain::CPatch::CProgGeom::UpdateEdges()
{
  SetActiveVertices(-1);
}

bool CTerrain::CPatch::CProgGeom::IsEdgeIndexActive(UINT uiEdgeInd, UINT uiActiveVert) 
{ 
  switch (uiActiveVert) {
    case -1:
      uiActiveVert = Util::Max(m_uiActiveVertices, GetMinActiveVertices());
      break;
    case 0:
      uiActiveVert = m_uiMinLODVertices;
      break;
  }
  return m_pPatch->m_EdgeMap.m_wEdgeIndices[uiEdgeInd] < uiActiveVert; 
}

void CTerrain::CPatch::CProgGeom::SetActiveVertices(int iVertices)
{
  int i;

  if (iVertices >= 0) {
    iVertices = GetValidVertexCount(iVertices);
    if (iVertices == m_uiActiveVertices)
      return;
    m_uiActiveVertices = iVertices;
  } else
    if (!m_bBordersUpdated)
      return;

  SetGeomVertices(iVertices, -1);

  for (i = 0; i < m_arrMaterialModels.m_iCount; i++)
    SetGeomVertices(iVertices, i);

  m_bBordersUpdated = false;

  if (iVertices >= 0) 
    UpdateAdjacentEdges();
}

bool CTerrain::CPatch::CProgGeom::Render()
{
  bool bRes, bExtraActive;
  int i;
  CBitArray<EDGE_INDICES> kActive;

  if (!m_bBordersUpdated)
    bExtraActive = DetermineActiveEdgeIndices(kActive, false);
  else
    bExtraActive = false;

  if (m_bUseLODModel)
    bRes = RenderMaterialModel(-1, bExtraActive, kActive);
  else {
    bRes = true;
    for (i = 0; i < m_arrMaterialModels.m_iCount; i++) 
      bRes &= RenderMaterialModel(i, bExtraActive, kActive);
  }
  m_bBordersUpdated = true;

  return bRes;
}

bool CTerrain::CPatch::CProgGeom::InitProgressiveMaterials(CPatchModelBuilder *pBuilder)
{
  ASSERT(!m_pLODMaterialModel);
  ASSERT(!m_arrMaterialModels.m_iCount);

  ASSERT(m_pPatch->m_EdgeMap.IsInitialized());
   
  if (!InitProgressiveMaterial(-1, pBuilder->m_pLODGeometry))
    return false;

  int i;
  CAVLTree<int>::TIter it;
  for (it = pBuilder->m_kUsedMaterials, i = 0; it; ++it, i++)
    if (!InitProgressiveMaterial(*it, pBuilder->m_arrMaterialGeometries[i]))
      return false;

  return true;
}

bool CTerrain::CPatch::CProgGeom::InitProgressiveMaterial(int iMaterial, CProgressiveGeometry *pProgGeom)
{
  CModel *pModel;
  CMaterial *pMaterial;

  pMaterial = GetTerrain()->GetMaterial(iMaterial);

  pModel = new CModel();
  pModel->Init(pProgGeom, pMaterial, 0, false);

  m_pPatch->InitParams(pModel, iMaterial, m_arrMaterialModels.m_iCount);

  if (iMaterial >= 0) {
    m_arrMaterialModels.Append(new TMaterialModel(pModel, iMaterial, this));
    if (m_uiMinVerticesToSet > pProgGeom->m_uiMinVertices)
      m_uiMinVerticesToSet = pProgGeom->m_uiMinVertices;
    if (m_uiMinActiveVertices < pProgGeom->m_uiMinVertices)
      m_uiMinActiveVertices = pProgGeom->m_uiMinVertices;
  } else {
    m_pLODMaterialModel = new TMaterialModel(pModel, -1, this);
    m_uiMinLODVertices = pProgGeom->m_uiMinVertices;
    m_uiMaxVertices = pProgGeom->GetVBVertexCount();
  }

  return true;
}

void CTerrain::CPatch::CProgGeom::DoneModels()
{
  delete m_pLODMaterialModel;
  m_arrMaterialModels.DeleteAll();
}

CTerrain::CPatch::CProgGeom::TMaterialModel *CTerrain::CPatch::CProgGeom::GetMaterialModel(int iMaterialInd)
{
  if (iMaterialInd >= 0)
    return m_arrMaterialModels[iMaterialInd];
  else
    return m_pLODMaterialModel;
}

int CTerrain::CPatch::CProgGeom::GetValidVertexCount(int iVertices)
{
  if (m_bUseLODModel)
    return Util::Bound<UINT>(iVertices, m_uiMinLODVertices, m_uiMaxVertices);
  return Util::Bound<UINT>(iVertices, m_uiMinVerticesToSet, m_uiMaxVertices);
}

void CTerrain::CPatch::CProgGeom::SetGeomVertices(int iVertices, int iMaterialInd)
{
  CProgressiveGeometry *pProgGeom;
  uint16_t *pIndices;
  TMaterialModel *pMatModel;

  pMatModel = GetMaterialModel(iMaterialInd);
  pProgGeom = Cast<CProgressiveGeometry>(pMatModel->m_pModel->m_pGeometry);
  if (!pProgGeom) 
    return;

  pIndices = (uint16_t *) pProgGeom->m_pIB->Map(0, CResource::RMF_SYSTEM_ONLY);

  if (m_bBordersUpdated)
    pMatModel->m_pEdgedGeom->DeactivateEdges(pIndices);
  if (iVertices >= 0)
    pProgGeom->SetActiveVertices(iVertices, &pIndices);

  pProgGeom->m_pIB->Unmap();
}

void CTerrain::CPatch::CProgGeom::UpdateLOD(CCamera *pCamera)
{
  CAABB kAABB;
  CPoint3D kCamPos;
  float fDist;
  int iTargetVertices;
  bool bWasUsingLOD;

  kAABB = m_pPatch->GetAABB(false);

  kCamPos.m_vPoint = pCamera->m_XForm.GetTranslation();
  fDist = kCamPos.GetDistance(&kAABB);

  bWasUsingLOD = m_bUseLODModel;

  switch (GetTerrain()->m_iForceModel) {
    case 1:
      m_bUseLODModel = true;
      break;
    case 2:
      m_bUseLODModel = false;
      break;
    default:
      m_bUseLODModel = fDist > GetTerrain()->m_fLODDistance;
      break;
  }

  iTargetVertices = Dist2Vertices(fDist);
  iTargetVertices = GetValidVertexCount(iTargetVertices);

  if (bWasUsingLOD != m_bUseLODModel)
    m_uiActiveVertices = -1;
  
  if (abs(iTargetVertices - (int) m_uiActiveVertices) <= (int) m_uiActiveVertices / 32 && 
      !(iTargetVertices >= (int) m_uiMaxVertices && m_uiActiveVertices < m_uiMaxVertices) &&
      !(iTargetVertices <= (int) m_uiMinLODVertices && m_uiActiveVertices > m_uiMinLODVertices))
    return;
  
  SetActiveVertices(iTargetVertices);
}

void CTerrain::CPatch::CProgGeom::SetMinLOD(bool bSetAdjacent)
{
  m_bUseLODModel = true;
  SetActiveVertices(m_uiMinLODVertices);
  UpdateBorders(bSetAdjacent);
}

bool CTerrain::CPatch::CProgGeom::IsMinLOD()
{
  return m_bUseLODModel && m_uiActiveVertices == m_uiMinLODVertices;
}

void CTerrain::CPatch::CProgGeom::SetModelVar(CStrAny sVar, CBaseVar const &vSrc)
{
  int i;
  for (i = 0; i < m_arrMaterialModels.m_iCount; i++)
    m_arrMaterialModels[i]->m_pModel->SetVar(sVar, vSrc);
}

void CTerrain::CPatch::CProgGeom::UpdateBorders(bool bAdjacentMinDetail)
{
  CBitArray<EDGE_INDICES> kActive;
  bool bExtraActive;
  CModel *pModel;
  TMaterialModel *pMatModel;
  int i;

  if (m_bBordersUpdated)
    return;

  bExtraActive = DetermineActiveEdgeIndices(kActive, bAdjacentMinDetail);

  for (i = -1; i < m_arrMaterialModels.m_iCount; i++) {
    pMatModel = GetMaterialModel(i);
    pModel = pMatModel->m_pModel;
    uint16_t *pIndices = (uint16_t *) pModel->m_pGeometry->m_pIB->Map();
    if (bExtraActive)
      pMatModel->m_pEdgedGeom->ActivateEdges(pIndices, kActive);
    pModel->m_pGeometry->m_pIB->Unmap();
  }
  m_bBordersUpdated = true;
}

bool CTerrain::CPatch::CProgGeom::RenderMaterialModel(int iMaterialInd, bool bExtraActive, CBitArray<EDGE_INDICES> const &kActive)
{
  CModel *pModel;
  TMaterialModel *pMatModel;
  
  pMatModel = GetMaterialModel(iMaterialInd);
  pModel = pMatModel->m_pModel;
  if (!m_bBordersUpdated) {
    uint16_t *pIndices = (uint16_t *) pModel->m_pGeometry->m_pIB->Map();
    if (bExtraActive)
      pMatModel->m_pEdgedGeom->ActivateEdges(pIndices, kActive);
    pModel->m_pGeometry->m_pIB->Unmap();
  }

  bool bRes = pModel->Render();

  return bRes;
}
