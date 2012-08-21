#include "stdafx.h"
#include "Terrain.h"
#include "File.h"
#include "Shape.h"
#include "GeomUtil.h"
#include "Camera.h"

// CTerrain::CPatchModelBuilder -----------------------------------------------
CTerrain::CPatchModelBuilder::CPatchModelBuilder(CPatch *pPatch)
{
  Init(pPatch);
}

CTerrain::CPatchModelBuilder::~CPatchModelBuilder()
{
  Done();
}

void CTerrain::CPatchModelBuilder::Init(CPatch *pPatch)
{
  ASSERT(!pPatch->IsProgressiveGeom());
  m_pPatch = pPatch;
  m_pMesh = 0;
  m_bBuildMinLOD = !m_pPatch->m_pMinLODModel;
  m_pOrgModel = pPatch->m_pModel;
  ASSERT(m_pOrgModel);
}

void CTerrain::CPatchModelBuilder::Done()
{
  SAFE_DELETE(m_pMesh);
  m_pLODGeometry = 0;
  for (int i = 0; i < m_arrMaterialGeometries.m_iCount; i++)
    m_arrMaterialGeometries[i] = 0;
  m_arrMaterialGeometries.SetCount(0);
  m_pMinLODGeometry = 0;
}

bool CTerrain::CPatchModelBuilder::InitMesh()
{
  ASSERT(!m_pMesh);
  m_pPatch->GetUsedMaterials(m_kUsedMaterials);
  m_pMesh = new CMesh();
  if (m_pPatch->m_arrCollapses.m_iCount) {
    m_pPatch->RestoreMeshFromData(*m_pMesh, m_pOrgModel->m_pGeometry);
    m_uiMaterialCollapses = m_pPatch->m_uiMaterialCollapses;
    return false;
  } else {
   static CStrAny sTEXCOORD(ST_CONST, "TEXCOORD");

   m_pMesh->Init(m_pOrgModel->m_pGeometry, sTEXCOORD, 1);
   return true;
  }
}

void CTerrain::CPatchModelBuilder::BuildModels()
{
  bool bNeedToSimplify;
  UINT  uiVertices;
  CAVLTree<int>::TIter it;

  bNeedToSimplify = InitMesh();
  if (bNeedToSimplify) {
    uiVertices = m_pMesh->m_pOrgGeom->GetVBVertexCount();
    m_pMesh->Simplify(uiVertices / 16 + CPatch::EDGE_INDICES, m_pPatch->m_pTerrain->m_fMaxLODError, true);
    m_uiMaterialCollapses = m_pMesh->GetMaxCollapses();
    m_pMesh->Simplify(uiVertices / 32 + CPatch::EDGE_INDICES, m_pPatch->m_pTerrain->m_fMaxLODError, false);
    m_pMesh->BuildReverseReorder(CPatch::EDGE_INDICES);
  }
  m_pLODGeometry = m_pMesh->BuildProgressiveGeometry(-1, CPatch::EDGE_INDICES, -1);
  for (it = m_kUsedMaterials; it; ++it) {
    m_arrMaterialGeometries.SetCount(m_arrMaterialGeometries.m_iCount + 1);
    m_arrMaterialGeometries[m_arrMaterialGeometries.m_iCount - 1] = m_pMesh->BuildProgressiveGeometry(*it, CPatch::EDGE_INDICES, m_uiMaterialCollapses);
  }
  if (m_bBuildMinLOD)
    m_pMinLODGeometry = BuildMinLODGeometry(m_pLODGeometry);
}

CGeometry *CTerrain::CPatchModelBuilder::BuildMinLODGeometry(CProgressiveGeometry *pProgGeom)
{
  CGeometry *pGeom;
  BYTE *pSrc, *pDst;
  UINT uiMapFlags;
  bool bRes;

  UINT uiActiveVert = pProgGeom->m_uiVertices;
  pProgGeom->SetActiveVertices(pProgGeom->m_uiMinVertices);

  uiMapFlags = (pProgGeom->m_pVB->m_uiFlags & CResource::RF_KEEPSYSTEMCOPY) ? CResource::RMF_SYSTEM_ONLY : 0;
  pSrc = pProgGeom->m_pVB->Map(0, uiMapFlags);

  pGeom = new CGeometry();
  bRes = pGeom->Init(pProgGeom->m_pInputDesc, CGeometry::PT_TRIANGLELIST, 
                     pProgGeom->m_uiVertices, pProgGeom->m_uiIndices + CPatch::EDGE_INDICES * 3, 
                     pSrc, 0, CResource::RF_KEEPSYSTEMCOPY, CResource::RF_KEEPSYSTEMCOPY);
  ASSERT(bRes);

  pProgGeom->m_pVB->Unmap();
  pGeom->m_uiIndices = pProgGeom->m_uiIndices;
  if (pProgGeom->m_pBound)
    pGeom->m_pBound = pProgGeom->m_pBound->Clone();

  uiMapFlags = (pProgGeom->m_pIB->m_uiFlags & CResource::RF_KEEPSYSTEMCOPY) ? CResource::RMF_SYSTEM_ONLY : 0;
  pSrc = pProgGeom->m_pIB->Map(0, uiMapFlags);
  pDst = pGeom->m_pIB->Map();

  memcpy(pDst, pSrc, pProgGeom->m_uiIndices * sizeof(WORD));

  pGeom->m_pIB->Unmap();
  pProgGeom->m_pIB->Unmap();

  pProgGeom->SetActiveVertices(uiActiveVert);

  return pGeom;
}

// CTerrain::CMeshBuildThread -------------------------------------------------
IMPRTTI_NOCREATE(CTerrain::CMeshBuildThread, CThread)

CTerrain::CMeshBuildThread::CMeshBuildThread(CTerrain *pTerrain):
  CThread()
{
  m_pTerrain = pTerrain;
}

CTerrain::CMeshBuildThread::~CMeshBuildThread()
{
}

bool CTerrain::CMeshBuildThread::Start()
{
  if (!Init(m_pTerrain))
    return false;

  return true;
}

UINT CTerrain::CMeshBuildThread::Run(void *pParam)
{
  while (m_pTerrain->m_bBuildingActive) {
    m_pTerrain->m_Lock.Lock();
    if (!m_pTerrain->m_lstBuildRequests.m_iCount) {
      m_pTerrain->m_Lock.Unlock();
      Yield(50);
      continue;
    }
    CPatchModelBuilder *pBuilder = m_pTerrain->m_lstBuildRequests.Pop();
    m_pTerrain->m_Lock.Unlock();

    pBuilder->BuildModels();

    m_pTerrain->m_Lock.Lock();
    m_pTerrain->m_lstBuildReady.PushTail(pBuilder);
    m_pTerrain->m_Lock.Unlock();
  }

  return 0;
}

// CTerrain::CLODPatch --------------------------------------------------------

CTerrain::CLODPatch::CLODPatch(CTerrain *pTerrain, int iXPatch, int iYPatch)
{
  m_pTerrain = pTerrain;
  m_fMinHeight = Util::F_INFINITY;
  m_fMaxHeight = Util::F_NEG_INFINITY;
  m_fNoUpdateDistance = Util::F_INFINITY;

  bool bRes = InitPatches(iXPatch, iYPatch);
  ASSERT(bRes);
}

CTerrain::CLODPatch::~CLODPatch()
{
}

bool CTerrain::CLODPatch::Init()
{
  if (!InitTextureContent())
    return false;

  if (!InitMinMaxHeight())
    return false;

  return true;
}

bool CTerrain::CLODPatch::InitTextures()
{
  int iSize;

  iSize = PATCH_SIZE * LOD_PATCH_SIZE;
  ASSERT(!(iSize & (iSize - 1))); // iSize is a power of 2

  m_pTexNormals = new CTexture();
  if (!m_pTexNormals->Init(iSize, iSize, DXGI_FORMAT_R8G8_SNORM, 0, 0, 0, CResource::RF_KEEPSYSTEMCOPY))
    return false;

  m_pTexFar = new CTexture();
  if (!m_pTexFar->Init(iSize, iSize, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 0, 0, CResource::RF_KEEPSYSTEMCOPY))
    return false;

  return true;
}

bool CTerrain::CLODPatch::InitTextureContent()
{
  if (!m_pTexNormals->GenerateMips())
    return false;
  if (!m_pTexFar->GenerateMips())
    return false;

  return true;
}

bool CTerrain::CLODPatch::InitMinMaxHeight()
{
  int x, y;

  m_fMinHeight = Util::F_INFINITY;
  m_fMaxHeight = Util::F_NEG_INFINITY;
  for (y = 0; y < LOD_PATCH_SIZE; y++)
    for (x = 0; x < LOD_PATCH_SIZE; x++) {
      if (!m_pPatches[y][x])
        continue;
      if (m_fMinHeight > m_pPatches[y][x]->m_fMinHeight)
        m_fMinHeight = m_pPatches[y][x]->m_fMinHeight;
      if (m_fMaxHeight < m_pPatches[y][x]->m_fMaxHeight)
        m_fMaxHeight = m_pPatches[y][x]->m_fMaxHeight;
    }

  return true;
}

bool CTerrain::CLODPatch::InitPatches(int iXPatch, int iYPatch)
{
  int x, y;

  for (y = 0; y < LOD_PATCH_SIZE; y++)
    for (x = 0; x < LOD_PATCH_SIZE; x++)
      m_pPatches[y][x] = m_pTerrain->GetPatchByIndex(iXPatch + x, iYPatch + y);

  return true;
}

bool CTerrain::CLODPatch::InitDistance()
{
  int i;
  CPatch *pPatch;
  CVector<2, int> vIndex;
  bool bRes = true;

  m_fNoUpdateDistance = 0;
  vIndex = m_pPatches[0][0]->m_vPatchIndex;

  for (i = 0; i < LOD_PATCH_SIZE; i++) {
    pPatch = m_pTerrain->GetPatchByIndex(vIndex.x() - 1, vIndex.y() + i);
    bRes &= AddPatchDistance(pPatch);
    pPatch = m_pTerrain->GetPatchByIndex(vIndex.x() + LOD_PATCH_SIZE, vIndex.y() + i);
    bRes &= AddPatchDistance(pPatch);
    pPatch = m_pTerrain->GetPatchByIndex(vIndex.x() + i, vIndex.y() - 1);
    bRes &= AddPatchDistance(pPatch);
    pPatch = m_pTerrain->GetPatchByIndex(vIndex.x() + i, vIndex.y() + LOD_PATCH_SIZE);
    bRes &= AddPatchDistance(pPatch);
  }

  return bRes;
}

bool CTerrain::CLODPatch::AddPatchDistance(CPatch *pPatch)
{
  float fDist;
  CAABB kAABB;

  if (!pPatch)
    return true;
  fDist = pPatch->GetMinLODDistance();
  ASSERT(fDist != Util::F_INFINITY);
  if (fDist == Util::F_INFINITY) {
    m_fNoUpdateDistance = Util::F_INFINITY;
    return false;
  }
  kAABB = pPatch->GetAABB(false);
  fDist += (kAABB.m_vMax - kAABB.m_vMin).Length();
  if (fDist > m_fNoUpdateDistance)
    m_fNoUpdateDistance = fDist;

  return true;
}

class CEdgeRemap: public CGeometryMerge::CIndexRemap {
public:
  CTerrain::CPatch::CEdgedGeom::CEdgeInfo *m_pEdgeInfo;

  CEdgeRemap() { m_pEdgeInfo = 0; }

  void SetEdgeInfo(CTerrain::CPatch::CEdgedGeom::CEdgeInfo *pEdgeInfo) { m_pEdgeInfo = pEdgeInfo; }

  WORD RemapIndex(WORD wIndex) 
  { 
    UINT uiVertInd;
    uiVertInd = m_pEdgeInfo->Vertex2Edge(wIndex);
    if (uiVertInd == -1)
      return wIndex;
    return (WORD) uiVertInd;
  }
};

bool CTerrain::CLODPatch::InitModel()
{
  int x, y;
  CGeometryMerge kMerge(m_pTerrain->m_arrMaterials[0].m_pMaterial->m_pTechnique->m_pInputDesc, true);
  CEdgeRemap kRemap;
  CMatrix<4, 4> mXForm, mTexXForm;
  CMatrixVar vTexVar(3, 2);
  CVector<3> vLODOrigin, vPatchOrigin;
  int iIndex;
  CGeometry *pGeom;
  CPatch *pPatch;
//  CMatrixVar *pTexVar;
  CModel *pModel;

  static const CStrAny sPOSITION(ST_CONST, "POSITION");
  static const CStrAny sTEXCOORD(ST_CONST, "TEXCOORD");
  static const CStrAny sg_mNormTransform(ST_CONST, "g_mNormTransform");

  vLODOrigin.Set(m_pPatches[0][0]->m_vWorldOrigin);
  vLODOrigin.z() = 0;
  mTexXForm.SetDiagonal();
  kMerge.SetIndexRemap(&kRemap);
  if (kMerge.m_pInputDesc->GetElementInfo(sPOSITION, 0, &iIndex)) 
    kMerge.SetVertexElementTransform(iIndex, &mXForm);
  if (kMerge.m_pInputDesc->GetElementInfo(sTEXCOORD, 0, &iIndex))
    kMerge.SetVertexElementTransform(iIndex, &mTexXForm);
  for (y = 0; y < LOD_PATCH_SIZE; y++)
    for (x = 0; x < LOD_PATCH_SIZE; x++) {
      pPatch = m_pPatches[y][x];
      if (!pPatch)
        continue;
      kRemap.SetEdgeInfo(&pPatch->m_EdgeMap);
      vPatchOrigin.Set(pPatch->m_vWorldOrigin);
      vPatchOrigin.z() = 0;
      mXForm.SetTranslation(vPatchOrigin - vLODOrigin);
      pModel = pPatch->m_pGeom->GetMinLODModel();
      pModel->GetVar(sg_mNormTransform, vTexVar);
/*      pTexVar = Cast<CMatrixVar>(pModel->m_pParams->FindVar(sg_mNormTransform));
      ASSERT(pTexVar);
      if (pTexVar) {
*/
        mTexXForm(0, 0) = vTexVar.At(0, 0);
        mTexXForm(0, 1) = vTexVar.At(0, 1);
        mTexXForm(1, 0) = vTexVar.At(1, 0);
        mTexXForm(1, 1) = vTexVar.At(1, 1);
        mTexXForm(3, 0) = vTexVar.At(2, 0);
        mTexXForm(3, 1) = vTexVar.At(2, 1);
//      }
      pPatch->m_pGeom->SetMinLOD(true);
      kMerge.AddGeometry(pModel->m_pGeometry);
    }

  pGeom = kMerge.CreateGeometry(0, CResource::RF_KEEPSYSTEMCOPY, CResource::RF_KEEPSYSTEMCOPY);
  pGeom->m_pBound = GetAABB(true).Clone();

  m_pModel = new CModel();
  m_pModel->Init(pGeom, m_pTerrain->m_pLODMaterial, 0, false);

  if (!m_pPatches[0][0]->InitParams(m_pModel, -1, -1))
    return false;

  if (!InitDistance())
    return false;

  return true;
}

bool CTerrain::CLODPatch::CanInitModel()
{
  int x, y;
  CVector<2, int> vIndex;
  CPatch *pPatch;

  if (m_pModel)
    return false;

  vIndex = m_pPatches[0][0]->m_vPatchIndex;
  for (y = 0; y < LOD_PATCH_SIZE; y++) {
    for (x = 0; x < LOD_PATCH_SIZE; x++) {
      if (!m_pPatches[y][x])
        continue;
      if (!m_pPatches[y][x]->m_pMinLODModel)
        return false;
    }
    // Neighbouring patches should have progressive geometry too so they can be stitched together with the LOD geometry at minimum level at the borders
    pPatch = m_pTerrain->GetPatchByIndex(vIndex.x() - 1, vIndex.y() + y);
    if (pPatch && !pPatch->m_pMinLODModel)
      return false;
    pPatch = m_pTerrain->GetPatchByIndex(vIndex.x() + LOD_PATCH_SIZE, vIndex.y() + y);
    if (pPatch && !pPatch->m_pMinLODModel)
      return false;
    pPatch = m_pTerrain->GetPatchByIndex(vIndex.x() + y, vIndex.y() - 1);
    if (pPatch && !pPatch->m_pMinLODModel)
      return false;
    pPatch = m_pTerrain->GetPatchByIndex(vIndex.x() + y, vIndex.y() + LOD_PATCH_SIZE);
    if (pPatch && !pPatch->m_pMinLODModel)
      return false;
  }
  return true;
}

CAABB CTerrain::CLODPatch::GetAABB(bool bLocal)
{
  CAABB kAABB;

  ASSERT(m_fMinHeight <= m_fMaxHeight);
  if (bLocal)
    kAABB.m_vMin.Set(CVector<2>::Get(0, 0));
  else
    kAABB.m_vMin.Set(m_pPatches[0][0]->m_vWorldOrigin);
  kAABB.m_vMin.z() = m_fMinHeight;
  kAABB.m_vMax.x() = kAABB.m_vMin.x() + (LOD_PATCH_SIZE * (PATCH_SIZE - 1) + 1) * m_pTerrain->m_fGrid2World;
  kAABB.m_vMax.y() = kAABB.m_vMin.y() + (LOD_PATCH_SIZE * (PATCH_SIZE - 1) + 1) * m_pTerrain->m_fGrid2World;
  kAABB.m_vMax.z() = m_fMaxHeight;

  return kAABB;
}

bool CTerrain::CLODPatch::CanActivate()
{
  int x, y, iXIndex, iYIndex;
  CPatch *pPatch;

  if (!m_pModel)
    return false;
  iXIndex = m_pPatches[0][0]->m_vPatchIndex.x();
  iYIndex = m_pPatches[0][0]->m_vPatchIndex.y();
  for (y = 0; y < LOD_PATCH_SIZE; y++) {
    for (x = 0; x < LOD_PATCH_SIZE; x++) {
      pPatch = m_pPatches[y][x];
      if (!pPatch)
        continue;
      if (!pPatch->m_pGeom->IsMinLOD())
        return false;
    }

    pPatch = m_pTerrain->GetPatchByIndex(iXIndex - 1, iYIndex + y);
    if (pPatch && !pPatch->m_pGeom->IsMinLOD())
      return false;
    pPatch = m_pTerrain->GetPatchByIndex(iXIndex + LOD_PATCH_SIZE, iYIndex + y);
    if (pPatch && !pPatch->m_pGeom->IsMinLOD())
      return false;
    pPatch = m_pTerrain->GetPatchByIndex(iXIndex + y, iYIndex - 1);
    if (pPatch && !pPatch->m_pGeom->IsMinLOD())
      return false;
    pPatch = m_pTerrain->GetPatchByIndex(iXIndex + y, iYIndex + LOD_PATCH_SIZE);
    if (pPatch && !pPatch->m_pGeom->IsMinLOD())
      return false;
  }

  return true;
}

void CTerrain::CLODPatch::LowerPatchGeoms()
{
  int x, y;
  for (y = 0; y < LOD_PATCH_SIZE; y++)
    for (x = 0; x < LOD_PATCH_SIZE; x++) 
      m_pPatches[y][x]->SetFullGeom(false);
}

void CTerrain::CLODPatch::UpdateLOD(CCamera *pCamera)
{
  int x, y;

  if (m_bActive && m_pModel) {
    CPoint3D kCamPos;
    float fDist;

    kCamPos.m_vPoint = pCamera->m_XForm.GetTranslation();
    fDist = GetAABB(false).GetDistance(&kCamPos);
    if (fDist >= m_fNoUpdateDistance) {
      if (!m_pPatches[0][0]->CheckMinimalGeom()) 
        LowerPatchGeoms();
      return;
    }
  }
  m_bActive = CanActivate();

  for (y = 0; y < LOD_PATCH_SIZE; y++)
    for (x = 0; x < LOD_PATCH_SIZE; x++) {
      if (!m_pPatches[y][x])
        continue;
      m_pPatches[y][x]->UpdateLOD(pCamera);
    }
}

bool CTerrain::CLODPatch::Render()
{
  int x, y;
  bool bRes;

  if (m_bActive)
    return m_pModel->Render();

  bRes = true;
  for (y = 0; y < LOD_PATCH_SIZE; y++)
    for (x = 0; x < LOD_PATCH_SIZE; x++) {
      if (!m_pPatches[y][x])
        continue;
      bRes &= m_pPatches[y][x]->Render();
    }
  return bRes;
}

// CTerrain -------------------------------------------------------------------

CTerrain::CTerrain(int iGridWidth, int iGridHeight, float fGrid2World, CMaterial *pBaseMat, CStrAny sSaveFileRoot): m_Lock(4000)
{
  m_fMaxLODError = 8.0f;
  m_fLODDistance = 300.0f;
  m_fGrid2World = fGrid2World;
  m_rcGrid.Set(0, 0, iGridWidth - 1, iGridHeight - 1);
  m_rcWorld.Set(0, 0, m_rcGrid.m_vMax.x() * m_fGrid2World, m_rcGrid.m_vMax.y() * m_fGrid2World);
  m_vPatchDim.x() = iGridWidth / (PATCH_SIZE - 1) + (iGridWidth % (PATCH_SIZE - 1) > 1);
  m_vPatchDim.y() = iGridHeight / (PATCH_SIZE - 1) + (iGridHeight % (PATCH_SIZE - 1) > 1);
  AddMaterial(pBaseMat);
  m_arrPatches.SetMaxCount(m_vPatchDim.x() * m_vPatchDim.y());
  int x, y;
  for (y = 0; y < m_vPatchDim.y(); y++)
    for (x = 0; x < m_vPatchDim.x(); x++)
      m_arrPatches.Append(new CPatch(x, y, this));

  m_vLODPatchDim.x() = m_vPatchDim.x() / LOD_PATCH_SIZE + !!(m_vPatchDim.x() % LOD_PATCH_SIZE);
  m_vLODPatchDim.y() = m_vPatchDim.y() / LOD_PATCH_SIZE + !!(m_vPatchDim.y() % LOD_PATCH_SIZE);
  for (y = 0; y < m_vPatchDim.y(); y += LOD_PATCH_SIZE)
    for (x = 0; x < m_vPatchDim.x(); x += LOD_PATCH_SIZE)
      m_arrLODPatches.Append(new CLODPatch(this, x, y));
  ASSERT(m_arrLODPatches.m_iCount == m_vLODPatchDim.x() * m_vLODPatchDim.y());

  m_iForceModel = 0;
  m_sSaveFileRoot = sSaveFileRoot;

  m_bBuildingActive = true;
  m_pBuilder = new CMeshBuildThread(this);
  m_pBuilder->Start();
}

CTerrain::~CTerrain()
{
  m_bBuildingActive = false;
  m_pBuilder->Wait(5000);
  delete m_pBuilder;
  m_lstBuildReady.DeleteAll();
  m_lstBuildRequests.DeleteAll();

  DonePatches();
}

CTerrain::CPatch *CTerrain::GetPatchByIndex(int iXPatch, int iYPatch)
{
  if (iXPatch < 0 || iXPatch >= m_vPatchDim.x() || iYPatch < 0 || iYPatch >= m_vPatchDim.y())
    return 0;
  return m_arrPatches[iYPatch * m_vPatchDim.x() + iXPatch];
}

CTerrain::CPatch *CTerrain::GetPatch(int iXGrid, int iYGrid)
{
  if (!m_rcGrid.Contains(CVector<2, int>::Get(iXGrid, iYGrid)))
    return 0;
  iXGrid = Util::Min(iXGrid / (PATCH_SIZE - 1), m_vPatchDim.x() - 1);
  iYGrid = Util::Min(iYGrid / (PATCH_SIZE - 1), m_vPatchDim.y() - 1);
  return m_arrPatches[iYGrid * m_vPatchDim.x() + iXGrid];
}

CTerrain::CPatch *CTerrain::GetPatch(float fX, float fY)
{
  if (!m_rcWorld.Contains(CVector<2>::Get(fX, fY)))
    return 0;
  int iXPatch, iYPatch;
  iXPatch = Util::Min((int) (fX / ((PATCH_SIZE - 1) * m_fGrid2World)), m_vPatchDim.x() - 1);
  iYPatch = Util::Min((int) (fY / ((PATCH_SIZE - 1) * m_fGrid2World)), m_vPatchDim.y() - 1);
  ASSERT(iXPatch >= 0 && iXPatch < m_vPatchDim.x() && iYPatch >= 0 && iYPatch < m_vPatchDim.y());
  return m_arrPatches[iYPatch * m_vPatchDim.x() + iXPatch];
}

CTerrain::CLODPatch *CTerrain::GetLODPatchByIndex(int iXLOD, int iYLOD)
{
  if (iXLOD < 0 || iXLOD >= m_vLODPatchDim.x() || iYLOD < 0 || iYLOD >= m_vLODPatchDim.y())
    return 0;
  return m_arrLODPatches[iYLOD * m_vLODPatchDim.x() + iXLOD];
}

CTerrain::CLODPatch *CTerrain::GetLODPatchByPatchIndex(int iXPatch, int iYPatch)
{
  return GetLODPatchByIndex(iXPatch / LOD_PATCH_SIZE, iYPatch / LOD_PATCH_SIZE);
}

const CTerrain::TPoint *CTerrain::GetPoint(int iXGrid, int iYGrid)
{
  CPatch *pPatch = GetPatch(iXGrid, iYGrid);
  if (!pPatch)
    return 0;
  return &pPatch->GetPoint(iXGrid, iYGrid);
}

void CTerrain::SetPoint(int iX, int iY, float fHeight, int iMaterial)
{
  if (!m_rcGrid.Contains(CVector<2, int>::Get(iX, iY)))
    return;
  int iXPatch, iYPatch, iInd;
  CPatch *pPatch, *pPatch1;
  bool bPrevXSet = false;
  iXPatch = Util::Min(iX / (PATCH_SIZE - 1), m_vPatchDim.x() - 1);
  iYPatch = Util::Min(iY / (PATCH_SIZE - 1), m_vPatchDim.y() - 1);
  iInd = iYPatch * m_vPatchDim.x() + iXPatch;
  pPatch = m_arrPatches[iInd];
  pPatch->GetPoint(iX, iY).Set(fHeight, iMaterial);
  if (iXPatch > 0 && iX == pPatch->m_vGridOrigin.x()) {
    pPatch1 = m_arrPatches[iInd - 1];
    pPatch1->GetPoint(iX, iY).Set(fHeight, iMaterial);
    bPrevXSet = true;
  }
  if (iYPatch > 0 && iY == pPatch->m_vGridOrigin.y()) {
    pPatch1 = m_arrPatches[iInd - m_vPatchDim.x()];
    pPatch1->GetPoint(iX, iY).Set(fHeight, iMaterial);
    if (bPrevXSet) {
      pPatch1 = m_arrPatches[iInd - m_vPatchDim.x() - 1];
      pPatch1->GetPoint(iX, iY).Set(fHeight, iMaterial);
    }
  }
}

float CTerrain::GetHeight(float fX, float fY)
{
  CPatch *pPatch = GetPatch(fX, fY);
  if (!pPatch)
    return 0.0f;
  return pPatch->GetHeight(fX, fY);
}

int CTerrain::AddMaterial(CMaterial *pMaterial)
{
  static const CStrAny sg_cMaterialAverageColor(ST_CONST, "g_cMaterialAverageColor");
  static const CStrAny sg_fLODDistance(ST_CONST, "g_fLODDistance");
  static const CStrAny sg_txDiffuse(ST_CONST, "g_txDiffuse");
  static const CStrAny sg_fMaterialID(ST_CONST, "g_fMaterialID");

  int iInd = m_arrMaterials.m_iCount;
  m_arrMaterials.SetCount(iInd + 1);
  m_arrMaterials[iInd].m_pMaterial = pMaterial;

  CVar<CVector<3> > vVec3;
  CVar<CTexture *> vTex;

  vVec3.Val().Set(1, 0, 1);

//  pTex = Cast<CVar<CTexture *> >(pMaterial->m_pParams->FindVar(sg_txDiffuse));
  if (pMaterial->GetVar(sg_txDiffuse, vTex)) {  // Read pixel from the least detailed mip map of the diffuse texture
    BYTE *pColor = vTex.Val()->Map(vTex.Val()->m_iMipLevels - 1, CResource::RMF_SYSTEM_ONLY);
    vVec3.Val().Set(pColor[0] / 255.0f, pColor[1] / 255.0f, pColor[2] / 255.0f);
    m_arrMaterials[iInd].m_clrAverage.Set(pColor[0], pColor[1], pColor[2]);
    vTex.Val()->Unmap();
  }

  pMaterial->SetVar(sg_cMaterialAverageColor, vVec3);

//  pMaterial->m_pParams->ReplaceVar(sg_fLODDistance, new CVarRef<float>(m_fLODDistance));
  pMaterial->SetVar(sg_fLODDistance, CVar<float>(m_fLODDistance));
  pMaterial->SetVar(sg_fMaterialID, CVar<float>((float) iInd));

  return iInd;
}

CMaterial *CTerrain::GetMaterial(int iMaterial)
{
  if (iMaterial < 0)
    return m_pLODMaterial;

  return m_arrMaterials[iMaterial].m_pMaterial;
}

bool CTerrain::InitPatches()
{
  if (!InitLODMaterial(m_arrMaterials[0].m_pMaterial))
    return false;

  int i;
  for (i = 0; i < m_arrLODPatches.m_iCount; i++)
    if (!m_arrLODPatches[i]->InitTextures())
      return false;

  for (i = 0; i < m_arrPatches.m_iCount; i++)
    if (!m_arrPatches[i]->Init())
      return false;

  for (i = 0; i < m_arrLODPatches.m_iCount; i++)
    if (!m_arrLODPatches[i]->Init())
      return false;

  return true;
}

void CTerrain::DonePatches()
{
  m_arrPatches.DeleteAll();
  m_arrLODPatches.DeleteAll();
}

void CTerrain::SetCamera(CCamera *pCamera)
{
  m_pCamera = pCamera;
}

bool CTerrain::Save(CFileBase *pFile)
{
  int i;
  CAutoDeletePtr<CFileBase> pPatchFile;

  if (!pFile) {
    CStrAny sName = GetSaveFileName();
    if (!!sName) 
      pFile = CFileSystem::Get()->OpenFile(sName, CFileBase::FOF_READ | CFileBase::FOF_WRITE | CFileBase::FOF_CREATE | CFileBase::FOF_TRUNCATE);
    if (!pFile)
      return false;
    pPatchFile.m_pPtr = pFile;
  }

  if (pFile->Write(m_rcGrid))
    return false;

  if (pFile->Write(m_fGrid2World))
    return false;

  if (pFile->Write(m_arrMaterials.m_iCount))
    return false;

  for (i = 0; i < m_arrPatches.m_iCount; i++) 
    if (!m_arrPatches[i]->Save(0))
      return false;

  return true;
}

bool CTerrain::Load(CFileBase *pFile)
{
  int i;
  CAutoDeletePtr<CFileBase> pPatchFile;

  if (!pFile) {
    CStrAny sName = GetSaveFileName();
    if (!!sName) 
      pFile = CFileSystem::Get()->OpenFile(sName, CFileBase::FOF_READ);
    if (!pFile)
      return false;
    pPatchFile.m_pPtr = pFile;
  }

  if (!InitLODMaterial(m_arrMaterials[0].m_pMaterial))
    return false;

  for (i = 0; i < m_arrLODPatches.m_iCount; i++)
    if (!m_arrLODPatches[i]->InitTextures())
      return false;

  CRect<int> rcGrid;
  if (pFile->Read(rcGrid) || rcGrid != m_rcGrid)
    return false;

  float fGrid2World;
  if (pFile->Read(fGrid2World) || fGrid2World != m_fGrid2World)
    return false;

  int iMaterials;
  if (pFile->Read(iMaterials) || iMaterials != m_arrMaterials.m_iCount)
    return false;

  for (i = 0; i < m_arrPatches.m_iCount; i++)
    if (!m_arrPatches[i]->Load(0))
      return false;

  for (i = 0; i < m_arrPatches.m_iCount; i++) {
    if (!m_arrPatches[i]->InitNormalsTexture())
      return false;
    if (!m_arrPatches[i]->InitFarTexture())
      return false;
  }

/*  for (i = 0; i < m_arrLODPatches.m_iCount; i++)
    if (!m_arrLODPatches[i]->Load(0))
      return false;
*/
  for (i = 0; i < m_arrLODPatches.m_iCount; i++) {
    if (!m_arrLODPatches[i]->InitTextureContent())
      return false;
    if (!m_arrLODPatches[i]->InitMinMaxHeight())
      return false;
    if (m_arrLODPatches[i]->CanInitModel())
      if (!m_arrLODPatches[i]->InitModel())
        return false;
  }

  return true;
}

CStrAny CTerrain::GetSaveFileName()
{
  if (!m_sSaveFileRoot) 
    return CStrAny(ST_WHOLE, "");
  return m_sSaveFileRoot + CStrAny(ST_WHOLE, "/Terrain.ter");
}

void CTerrain::PatchInitDone(CPatch *pPatch)
{
  CPatch *pAdj[4];
  CLODPatch *pLODPatch[4];
  int i, j;

  pPatch->GetAdjacentPatches(pAdj[0], pAdj[1], pAdj[2], pAdj[3]);
  for (i = 0; i < 4; i++) {
    if (!pAdj[i])
      continue;
    pLODPatch[i] = pAdj[i]->GetLODPatch();
    for (j = 0; j < i; j++)
      if (pLODPatch[i] == pLODPatch[j])
        break;
    if (j < i)
      continue;
    if (!pLODPatch[i]->CanInitModel())
      continue;
    pLODPatch[i]->InitModel();
/*    if (m_sSaveFileRoot)
      pLODPatch[i]->Save(0);
*/
  }
}

void CTerrain::UpdateLOD(CCamera *pCamera)
{
  int i;

  if (!pCamera)
    pCamera = m_pCamera;
  while (m_lstBuildReady.m_iCount) {
    m_Lock.Lock();
    CPatchModelBuilder *pBuilder = m_lstBuildReady.Pop();
    m_Lock.Unlock();
    bool bSavePatch = !pBuilder->m_pPatch->HasMeshData() || !pBuilder->m_pPatch->m_pMinLODModel;
    pBuilder->m_pPatch->SetMaterialModels(pBuilder);
    if (bSavePatch && !!m_sSaveFileRoot) {
      ASSERT(pBuilder->m_pPatch->HasMeshData());
      pBuilder->m_pPatch->Save(0);
    }
    PatchInitDone(pBuilder->m_pPatch);
    delete pBuilder;
  }

  for (i = 0; i < m_arrLODPatches.m_iCount; i++)
    m_arrLODPatches[i]->UpdateLOD(pCamera);
}

bool CTerrain::Render()
{
  bool bRes = true;
  for (int i = 0; i < m_arrLODPatches.m_iCount; i++)
    bRes &= m_arrLODPatches[i]->Render();

  return bRes;
}

bool CTerrain::InitLODMaterial(CMaterial *pSrcMat)
{
  static CStrAny sTerrainLOD(ST_CONST, "TerrainLOD");

  m_pLODMaterial = new CMaterial();
  CTechnique *pTech = CGraphics::Get()->GetTechnique(sTerrainLOD);
  if (!pTech)
    return false;

  if (!m_pLODMaterial->Init(pTech, true, pSrcMat->GetApplyVars(0)))
    return false;

  return true;
}
