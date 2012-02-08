#include "stdafx.h"
#include "Terrain.h"
#include "Model.h"
#include "Shape.h"
#include "File.h"

// CTerrain::CPatch -----------------------------------------------------------

CTerrain::CPatch::CPatch(int iX, int iY, CTerrain *pTerrain)
{
  m_pTerrain = pTerrain;
  m_vPatchIndex.Set(iX, iY);
  m_vGridOrigin.Set(iX * (PATCH_SIZE - 1), iY * (PATCH_SIZE - 1));
  m_vWorldOrigin.Set(m_vGridOrigin.x() * m_pTerrain->m_fGrid2World, m_vGridOrigin.y() * m_pTerrain->m_fGrid2World);
  memset(m_Points, 0, sizeof(m_Points));
  m_fMinHeight = Util::F_INFINITY;
  m_fMaxHeight = Util::F_NEG_INFINITY;

  m_bBuilding = false;
}

CTerrain::CPatch::~CPatch()
{
}

float CTerrain::CPatch::GetHeight(float fX, float fY)
{
  float fXGrid, fYGrid;
  int iXGrid, iYGrid;
  float h00, h01, h10, h11, h0, h1, h;
  float wx, wy;

  ASSERT(PointInside(fX, fY));
  fXGrid = fX / m_pTerrain->m_fGrid2World;
  fYGrid = fY / m_pTerrain->m_fGrid2World;
  iXGrid = (int) fXGrid;
  iYGrid = (int) fYGrid;
  wx = fXGrid - iXGrid;
  wy = fYGrid - iYGrid;
  iXGrid -= m_vGridOrigin.x();
  iYGrid -= m_vGridOrigin.y();

  if (iXGrid == PATCH_SIZE - 1)
    iXGrid--;
  if (iYGrid == PATCH_SIZE - 1)
    iYGrid--;

  h00 = m_Points[iYGrid][iXGrid].m_fHeight;
  h01 = m_Points[iYGrid][iXGrid + 1].m_fHeight;
  h10 = m_Points[iYGrid + 1][iXGrid].m_fHeight;
  h11 = m_Points[iYGrid + 1][iXGrid + 1].m_fHeight;
  h0 = Util::Lerp(h00, h01, wx);
  h1 = Util::Lerp(h10, h11, wx);
  h = Util::Lerp(h0, h1, wy);

  return h;
}

void CTerrain::CPatch::GetGridNormal(int iX, int iY, CVector<3> &vNormal)
{
  ASSERT(iX >= 0 && iX < PATCH_SIZE);
  ASSERT(iY >= 0 && iY < PATCH_SIZE);

  float fHx0, fHx1, fHy0, fHy1, fdx, fdy;
  fdx = 2;
  fdy = 2;
  if (iX > 0)
    fHx0 = m_Points[iY][iX - 1].m_fHeight;
  else
    if (m_vPatchIndex.x())
      fHx0 = m_pTerrain->m_arrPatches[m_vPatchIndex.y() * m_pTerrain->m_vPatchDim.x() + m_vPatchIndex.x() - 1]->m_Points[iY][PATCH_SIZE - 2].m_fHeight;
    else {
      fHx0 = m_Points[iY][iX].m_fHeight;
      fdx = 1;
    }
  if (iX < PATCH_SIZE - 1)
    fHx1 = m_Points[iY][iX + 1].m_fHeight;
  else
    if (m_vPatchIndex.x() < m_pTerrain->m_vPatchDim.x() - 1)
      fHx1 = m_pTerrain->m_arrPatches[m_vPatchIndex.y() * m_pTerrain->m_vPatchDim.x() + m_vPatchIndex.x() + 1]->m_Points[iY][1].m_fHeight;
    else {
      fHx1 = m_Points[iY][iX].m_fHeight;
      fdx = 1;
    }

  if (iY > 0)
    fHy0 = m_Points[iY - 1][iX].m_fHeight;
  else
    if (m_vPatchIndex.y())
      fHy0 = m_pTerrain->m_arrPatches[(m_vPatchIndex.y() - 1) * m_pTerrain->m_vPatchDim.x() + m_vPatchIndex.x()]->m_Points[PATCH_SIZE - 2][iX].m_fHeight;
    else {
      fHy0 = m_Points[iY][iX].m_fHeight;
      fdy = 1;
    }
  if (iY < PATCH_SIZE - 1)
    fHy1 = m_Points[iY + 1][iX].m_fHeight;
  else
    if (m_vPatchIndex.y() < m_pTerrain->m_vPatchDim.y() - 1)
      fHy1 = m_pTerrain->m_arrPatches[(m_vPatchIndex.y() + 1) * m_pTerrain->m_vPatchDim.x() + m_vPatchIndex.x()]->m_Points[1][iX].m_fHeight;
    else {
      fHy1 = m_Points[iY][iX].m_fHeight;
      fdy = 1;
    }

  vNormal = CVector<3>::Get(fdx, 0, fHx1 - fHx0) ^ CVector<3>::Get(0, fdy, fHy1 - fHy0);
  vNormal.Normalize();
  ASSERT(vNormal.z() > 0);
}

CAABB CTerrain::CPatch::GetAABB(bool bLocal)
{
  CAABB kAABB;

  ASSERT(m_fMinHeight <= m_fMaxHeight);
  if (bLocal)
    kAABB.m_vMin.Set(0, 0, m_fMinHeight);
  else
    kAABB.m_vMin.Set(m_vWorldOrigin.x(), m_vWorldOrigin.y(), m_fMinHeight);
  kAABB.m_vMax.x() = kAABB.m_vMin.x() + (PATCH_SIZE - 1) * m_pTerrain->m_fGrid2World;
  kAABB.m_vMax.y() = kAABB.m_vMin.y() + (PATCH_SIZE - 1) * m_pTerrain->m_fGrid2World;
  kAABB.m_vMax.z() = m_fMaxHeight;

  return kAABB;
}

void CTerrain::CPatch::GetUsedMaterials(CAVLTree<int> &kUsedMats)
{
  int x, y;
  for (y = 0; y < PATCH_SIZE; y++)
    for (x = 0; x < PATCH_SIZE; x++) 
      kUsedMats.AddUnique(m_Points[y][x].m_iMaterial);
}

bool CTerrain::CPatch::Init()
{
  UINT uiReservedVerts;

  if (!InitMinMaxHeight())
    return false;
  
  if (!InitNormalsTexture())
    return false;

  if (!InitFarTexture())
    return false;

  bool bInitProgressive = false;
/*  if (m_pTerrain->GetPatch(0, 0) == this || true)
    uiReservedVerts = EDGE_INDICES;
  else 
    uiReservedVerts = 0;
*/

  uiReservedVerts = EDGE_INDICES;
  if (!InitModel(uiReservedVerts))
    return false;

/*  if (m_pTerrain->GetPatch(0, 0) == this) {
    if (!InitProgressiveModel())
      return false;
    return true;
  }
*/

  if (bInitProgressive) {
    CPatchModelBuilder kBuilder(this);
    kBuilder.BuildModels();
    if (!SetProgGeom(&kBuilder))
      return false;
  } else {
    if (!SetFullGeom(false))
      return false;
  }

  return true;
}

bool CTerrain::CPatch::Save(CFileBase *pFile)
{
  CAutoDeletePtr<CFileBase> pPatchFile;

  if (!pFile) {
    CStr sName = GetSaveFileName();
    if (sName) 
      pFile = CFileSystem::Get()->OpenFile(sName, CFileBase::FOF_READ | CFileBase::FOF_WRITE | CFileBase::FOF_CREATE | CFileBase::FOF_TRUNCATE);
    if (!pFile)
      return false;
    pPatchFile.m_pPtr = pFile;
  }

  if (pFile->Write(m_vPatchIndex))
    return false;

  if (pFile->Write(m_Points))
    return false;

  if (pFile->Write(m_fMinHeight) || pFile->Write(m_fMaxHeight))
    return false;

  if (!SaveMeshData(pFile))
    return false;

  if (!SaveMinLODMesh(pFile))
    return false;

  return true;
}

bool CTerrain::CPatch::Load(CFileBase *pFile)
{
  CVector<2, int> vPatchIndex;
  CAutoDeletePtr<CFileBase> pPatchFile;

  if (!pFile) {
    CStr sName = GetSaveFileName();
    if (sName) 
      pFile = CFileSystem::Get()->OpenFile(sName, CFileBase::FOF_READ);
    if (!pFile)
      return false;
    pPatchFile.m_pPtr = pFile;
  }

  if (pFile->Read(vPatchIndex) || vPatchIndex != m_vPatchIndex)
    return false;

  if (pFile->Read(m_Points))
    return false;

  if (pFile->Read(m_fMinHeight) || pFile->Read(m_fMaxHeight))
    return false;

/*  if (!LoadMeshData(pFile))
    return false;
*/
  LoadMeshData(pFile);

  LoadMinLODMesh(pFile);

  bool bInitProgressive = false;
  if (bInitProgressive && HasMeshData()) {
    if (!InitModel(EDGE_INDICES))
      return false;
    CPatchModelBuilder kBuilder(this);
    kBuilder.BuildModels();
    if (!SetProgGeom(&kBuilder))
      return false;
  } else {
    if (!SetFullGeom(false))
      return false;
  }

  return true;
}

bool CTerrain::CPatch::SaveMeshData(CFileBase *pFile)
{
  if (!HasMeshData())
    return true;

  if (pFile->Write(m_uiMaterialCollapses))
    return false;
  if (pFile->Write(m_uiRealCollapses))
    return false;

  if (pFile->Write(m_arrCollapses))
    return false;

  if (pFile->Write(m_arrCollapsedTriangles))
    return false;

  return true;
}

bool CTerrain::CPatch::LoadMeshData(CFileBase *pFile)
{
  ASSERT(!HasMeshData());

  if (pFile->Read(m_uiMaterialCollapses))
    return false;
  if (pFile->Read(m_uiRealCollapses))
    return false;

  if (pFile->Read(m_arrCollapses))
    return false;

  if (pFile->Read(m_arrCollapsedTriangles))
    return false;

  return true;
}

bool CTerrain::CPatch::SaveMinLODMesh(CFileBase *pFile)
{
  if (!m_EdgeMap.IsInitialized() || !m_pMinLODModel)
    return true;
  if (pFile->Write(m_EdgeMap.m_wEdgeIndices))
    return false;

  UINT uiMapFlags;
  BYTE *pBuf;
  CFileBase::ERRCODE err;
  CGeometry *pGeom;
  
  pGeom = m_pMinLODModel->m_pGeometry;
  uiMapFlags = (pGeom->m_pIB->m_uiFlags & CResource::RF_KEEPSYSTEMCOPY) ? CResource::RMF_SYSTEM_ONLY : 0;
  pBuf = pGeom->m_pIB->Map(0, uiMapFlags);
  err = pFile->WriteBuf(pBuf, pGeom->m_uiIndices * sizeof(WORD));
  pGeom->m_pIB->Unmap();
  if (err)
    return false;

  uiMapFlags = (pGeom->m_pVB->m_uiFlags & CResource::RF_KEEPSYSTEMCOPY) ? CResource::RMF_SYSTEM_ONLY : 0;
  pBuf = pGeom->m_pVB->Map(0, uiMapFlags);
  err = pFile->WriteBuf(pBuf, pGeom->m_uiVertices * pGeom->m_pInputDesc->GetSize());
  pGeom->m_pVB->Unmap();
  if (err)
    return false;

  return true;
}

bool CTerrain::CPatch::LoadMinLODMesh(CFileBase *pFile)
{
  ASSERT(!m_EdgeMap.IsInitialized() && !m_pMinLODModel);

  WORD wEdgeIndices[EDGE_INDICES];
  if (pFile->Read(wEdgeIndices))
    return false;
  if (!m_EdgeMap.Init(wEdgeIndices))
    return false;

  CAutoDeletePtr<BYTE> pIB, pVB;
  int iIBSize, iVBSize;
  CSmartPtr<CGeometry> pGeom;
  CInputDesc *pDesc;
  BYTE *pBuf;

  if (pFile->ReadBuf(*(void **) &pIB.m_pPtr, iIBSize))
    return false;
  if (pFile->ReadBuf(*(void **) &pVB.m_pPtr, iVBSize))
    return false;

  pDesc = m_pTerrain->m_pLODMaterial->m_pTechnique->m_pInputDesc;
  pGeom = new CGeometry();
  if (!pGeom->Init(pDesc, CGeometry::PT_TRIANGLELIST, iVBSize / pDesc->GetSize(), 
                   iIBSize / sizeof(WORD) + EDGE_INDICES * 3, pVB, 0, 
                   CResource::RF_KEEPSYSTEMCOPY, CResource::RF_KEEPSYSTEMCOPY))
    return false;
  pBuf = pGeom->m_pIB->Map();
  memcpy(pBuf, pIB, iIBSize);
  pGeom->m_pIB->Unmap();
  pGeom->m_uiIndices = iIBSize / sizeof(WORD);
  pGeom->m_pBound = GetAABB(true).Clone();

  m_pMinLODModel = new CModel();
  m_pMinLODModel->Init(pGeom, m_pTerrain->m_pLODMaterial, 0, false);
  InitParams(m_pMinLODModel, -1, 0);

  return true;
}

CStr CTerrain::CPatch::GetSaveFileName()
{
  if (!m_pTerrain->m_sSaveFileRoot)
    return "";
  return m_pTerrain->m_sSaveFileRoot + CStrPart("/Patch(") + CStr(m_vPatchIndex.x()) + CStrPart(",") + CStr(m_vPatchIndex.y()) + CStrPart(").ter");
}

bool CTerrain::CPatch::InitModel(UINT uiReservedVertices)
{
  UINT uiVertices;
  int x, y;

  CMaterial *pMaterial = m_pTerrain->m_pLODMaterial;
  CGeometry *pGeom = new CGeometry();
  uiVertices = Util::Sqr(PATCH_SIZE) + uiReservedVertices;
  pGeom->Init(pMaterial->m_pTechnique->m_pInputDesc, CGeometry::PT_TRIANGLELIST, uiVertices, 0, 0, 0, CResource::RF_KEEPSYSTEMCOPY, CResource::RF_KEEPSYSTEMCOPY);

  TTerrainVertex *pOrgVert, *pVert; 
  float fScale = m_pTerrain->m_fGrid2World;
  pOrgVert = (TTerrainVertex *) pGeom->m_pVB->Map();
  pVert = pOrgVert + uiReservedVertices;

  for (y = 0; y < PATCH_SIZE; y++)
    for (x = 0; x < PATCH_SIZE; x++) {
      pVert->m_vPos = CVector<3>::Get(x * fScale, y * fScale, m_Points[y][x].m_fHeight);
      pVert->m_vTexCoord.Set(pVert->m_vPos.x(), pVert->m_vPos.y());
      pVert->m_fMaterialID = (float) m_Points[y][x].m_iMaterial;

      pVert++;
    }

  if (uiReservedVertices == EDGE_INDICES)
    InitEdgeVertices(pOrgVert);

  pGeom->SetBoundType(&CAABB::s_RTTI, pOrgVert);

  pGeom->m_pVB->Unmap();

  InitIB(pGeom, -1, uiReservedVertices);

  m_pModel = new CModel();
  m_pModel->Init(pGeom, pMaterial, 0, false);

  InitParams(m_pModel, -1, 0);

  return true;
}

bool CTerrain::CPatch::InitIB(CGeometry *pGeom, int iMaterial, UINT uiReservedVertices)
{
  int x, y;
  UINT uiMaxIndices;
  WORD *pInd, *pOrgInd;

  uiMaxIndices = Util::Sqr(PATCH_SIZE - 1) * 6;
  pOrgInd = new WORD[uiMaxIndices];

  pInd = pOrgInd;
  for (y = 0; y < PATCH_SIZE - 1; y++)
    for (x = 0; x < PATCH_SIZE - 1; x++) {
      WORD wBase;
      bool bAddTri;

      wBase = y * PATCH_SIZE + x + uiReservedVertices;

      bAddTri = iMaterial < 0 || m_Points[y][x].m_iMaterial == iMaterial || m_Points[y][x + 1].m_iMaterial == iMaterial ||
                m_Points[y + 1][x].m_iMaterial == iMaterial;

      if (bAddTri) {
        pInd[0] = wBase;
        pInd[1] = wBase + 1;
        pInd[2] = wBase + PATCH_SIZE;
        pInd += 3;
      }

      bAddTri = iMaterial < 0 || m_Points[y][x + 1].m_iMaterial == iMaterial ||
                m_Points[y + 1][x].m_iMaterial == iMaterial || m_Points[y + 1][x + 1].m_iMaterial == iMaterial;

      if (bAddTri) {
        pInd[0] = wBase + 1;
        pInd[1] = wBase + PATCH_SIZE + 1;
        pInd[2] = wBase + PATCH_SIZE;
        pInd += 3;
      }
    }

  ASSERT(pInd - pOrgInd <= (int) uiMaxIndices && pInd - pOrgInd > 0);
  pGeom->SetIndices(pInd - pOrgInd, pOrgInd, CResource::RF_KEEPSYSTEMCOPY);

  delete pOrgInd;

  return true;
}

bool CTerrain::CPatch::CheckMinimalGeom()
{
  CFullGeom *pFullGeom;

  if (!m_pMinLODModel) // Do not switch to minimal geom if the mesh data is still building
    return true;
  pFullGeom = Cast<CFullGeom>(m_pGeom);
  if (!pFullGeom)
    return false;
  if (pFullGeom->m_arrMaterialModels.m_iCount)
    return false;

  return true;
}

void CTerrain::CPatch::SetGeom(CPatchGeom *pGeom)
{
  m_pGeom = pGeom;
  m_pGeom->UpdateLOD(m_pTerrain->m_pCamera);
}

bool CTerrain::CPatch::SetFullGeom(bool bInitMaterialModels)
{
  bool bRes;
  CFullGeom *pFullGeom;

  if ((bInitMaterialModels || !m_pMinLODModel) && !m_pModel)
    if (!InitModel(EDGE_INDICES))
      return false;

  if (!bInitMaterialModels && m_pMinLODModel)
    m_pModel = 0;

  pFullGeom = Cast<CFullGeom>(m_pGeom);
  if (pFullGeom) {
    if (pFullGeom->m_arrMaterialModels.m_iCount && !bInitMaterialModels) {
      pFullGeom->DoneMaterialModels();
      bRes = true;
    } else
      if (!pFullGeom->m_arrMaterialModels.m_iCount && bInitMaterialModels)
        bRes = pFullGeom->InitMaterialModels(EDGE_INDICES);
      else
        bRes = true;
  } else {
    pFullGeom = new CFullGeom();
    bRes = pFullGeom->Init(this, bInitMaterialModels);
    SetGeom(pFullGeom);
  }

  return bRes;
}

bool CTerrain::CPatch::SetProgGeom(CPatchModelBuilder *pBuilder)
{
  bool bRes;
  CProgGeom *pProgGeom;

  m_bBuilding = false;
  pProgGeom = new CProgGeom();
  bRes = pProgGeom->Init(this, pBuilder);
  SetGeom(pProgGeom);

  return bRes;
}


bool CTerrain::CPatch::InitEdgeVertices(TTerrainVertex *pVertices)
{
  int i;
  CVector<2, int> vCoord;
  for (i = 0; i < EDGE_INDICES; i++) {
    vCoord = EdgeIndex2Grid(i);
    pVertices[i] = pVertices[vCoord.y() * PATCH_SIZE + vCoord.x() + EDGE_INDICES];
    ASSERT(pVertices[i].m_vPos.x() == vCoord.x() && pVertices[i].m_vPos.y() == vCoord.y());
  }
  return true;
}

UINT CTerrain::CPatch::Grid2EdgeIndex(int iX, int iY)
{
  ASSERT(iX >= 0 && iX < PATCH_SIZE);
  ASSERT(iY >= 0 && iY < PATCH_SIZE);
  if (iY == 0)
    return iX;
  if (iX == PATCH_SIZE - 1)
    return iY + PATCH_SIZE - 1;
  if (iY == PATCH_SIZE - 1)
    return PATCH_SIZE - 1 - iX + (PATCH_SIZE - 1) * 2;
  if (iX == 0)
    return PATCH_SIZE - 1 - iY + (PATCH_SIZE - 1) * 3;
  ASSERT(!"Vertex not on edge");
  return -1;
}

CVector<2, int> CTerrain::CPatch::EdgeIndex2Grid(UINT uiIndex)
{
  CVector<2, int> vRes;
  UINT uiSide, uiIndexOnSide;
  uiSide = uiIndex / (PATCH_SIZE - 1);
  uiIndexOnSide = uiIndex % (PATCH_SIZE - 1);
  switch (uiSide) {
    case 0:
      vRes.Set(uiIndexOnSide, 0);
      break;
    case 1:
      vRes.Set(PATCH_SIZE - 1, uiIndexOnSide);
      break;
    case 2:
      vRes.Set(PATCH_SIZE - 1 - uiIndexOnSide, PATCH_SIZE - 1);
      break;
    case 3:
      vRes.Set(0, PATCH_SIZE - 1 - uiIndexOnSide);
      break;
    default:
      ASSERT(!"Edge index out of bounds");
      vRes.SetVal(-1);
      break;
  }
  ASSERT(Grid2EdgeIndex(vRes.x(), vRes.y()) == uiIndex);
  return vRes;
}

bool CTerrain::CPatch::InitParams(CModel *pModel, int iMaterial, int iMaterialInd)
{
  static const CStrConst sg_mWorld("g_mWorld");
  static const CStrConst sg_mDiffTransform("g_mDiffTransform");
  static const CStrConst sg_mNormTransform("g_mNormTransform");
  static const CStrConst sg_cMaterialSpecular("g_cMaterialSpecular");
  static const CStrConst sg_cMaterialDiffuse("g_cMaterialDiffuse");
  static const CStrConst sg_cMaterialAmbient("g_cMaterialAmbient");
  static const CStrConst sg_fLODDistance("g_fLODDistance");
  static const CStrConst sg_txDiffuse("g_txDiffuse");
  static const CStrConst sg_txFar("g_txFar");
  static const CStrConst sg_txNormals("g_txNormals");
  static const CStrConst sg_sDiffuse("g_sDiffuse");
  static const CStrConst sg_sNormals("g_sNormals");
  static const CStrConst sg_fMaterialID("g_fMaterialID");
  static const CStrConst sBlendEnable0("BlendEnable0");
  static const CStrConst sBlendState("BlendState");

  CMatrixVar vWorld(4, 4);
  CMatrix<4, 4> *pWorld = (CMatrix<4, 4> *) vWorld.m_pVal;
  pWorld->SetDiagonal();
  pWorld->SetTranslation(CVector<3>::Get(m_vWorldOrigin.x(), m_vWorldOrigin.y(), 0));
  pModel->SetVar(sg_mWorld, vWorld);
  pModel->UpdateBound();
  
  CMatrixVar vTexTransform(3, 2), vTexNew(3, 2);
  CMatrix<3, 2> *pTexTransform, *pTexNew;
  CMatrix<3, 3> mTexMod;
  CMaterial *pMaterial;
  pTexTransform = (CMatrix<3, 2> *) vTexTransform.m_pVal;
  pTexNew = (CMatrix<3, 2> *) vTexNew.m_pVal;

  pMaterial = m_pTerrain->GetMaterial(iMaterial);
  pMaterial->GetVar(sg_mDiffTransform, vTexTransform);

  mTexMod.SetDiagonal();
  mTexMod(2, 0) = m_vWorldOrigin.x();
  mTexMod(2, 1) = m_vWorldOrigin.y();
  pTexNew->SetMul(mTexMod, *pTexTransform);
  
  pModel->SetVar(sg_mDiffTransform, vTexNew);

  if (iMaterialInd >= 0)
    SetTexTransform(*pTexNew);
  else
    pTexNew->SetDiagonal(); // LOD patch composite geometry
  pModel->SetVar(sg_mNormTransform, vTexNew);

/*  if (iMaterial >= 0)
    pModel->SetFloat(sg_fMaterialID, (float) iMaterial);
*/

//  pModel->SetInt(sBlendEnable0, iMaterial >= 0 && iMaterialInd > 0);
  pModel->m_StateCache.SetInt(sBlendEnable0, iMaterial >= 0 && iMaterialInd > 0);

  CVar<CTexture *> vTex;
  vTex.Val() = m_pTexFar;
  pModel->SetVar(sg_txFar, vTex);

  vTex.Val() = m_pTexNormals;
  pModel->SetVar(sg_txNormals, vTex);

/*  CVarRef<CTexture *> *pFarTexVar = new CVarRef<CTexture *>(m_pTexFar.m_pPtr);
  pParams->ReplaceVar(sg_txFar, pFarTexVar);

  CVarRef<CTexture *> *pNormTexVar = new CVarRef<CTexture *>(m_pTexNormals.m_pPtr);
  pParams->ReplaceVar(sg_txNormals, pNormTexVar);
*/

  return true;
}

bool CTerrain::CPatch::InitNormalsTexture()
{
  CLODPatch *pLODPatch = GetLODPatch();
  m_pTexNormals = pLODPatch->m_pTexNormals;

  ASSERT(m_pTexNormals);
  if (!m_pTexNormals)
    return false;

  CRect<int> rcNorm = GetTexRectPixel();

  char *pBuf;
  int iRowPitch, x, y;
  CVector<3> vNormal;

  iRowPitch = m_pTexNormals->GetBlockPitch(0);
  pBuf = (char *) m_pTexNormals->MapRect(0, 0, rcNorm);
  
  for (y = 0; y < PATCH_SIZE; y++)
    for (x = 0; x < PATCH_SIZE; x++) {
      GetGridNormal(x, y, vNormal);
      pBuf[x * 2 + y * iRowPitch] = (char) (vNormal.x() * 127.0f);
      pBuf[x * 2 + y * iRowPitch + 1] = (char) (vNormal.y() * 127.0f);
    }

  m_pTexNormals->Unmap();

  static const CStrConst sg_txNormals("g_txNormals");
  CVar<CTexture *> vTexNorm(m_pTexNormals);
  if (m_pMinLODModel)
    m_pMinLODModel->SetVar(sg_txNormals, vTexNorm);
  if (m_pGeom)
    m_pGeom->SetModelVar(sg_txNormals, vTexNorm);

  return true;
}

bool CTerrain::CPatch::InitFarTexture()
{
  CLODPatch *pLODPatch = GetLODPatch();
  m_pTexFar = pLODPatch->m_pTexFar;

  ASSERT(m_pTexFar);
  if (!m_pTexFar)
    return false;

  CRect<int> rcNorm = GetTexRectPixel();

  BYTE *pBuf;
  int iRowPitch, x, y;

  iRowPitch = m_pTexFar->GetBlockPitch(0);
  pBuf = m_pTexFar->MapRect(0, 0, rcNorm);
  
  for (y = 0; y < PATCH_SIZE; y++)
    for (x = 0; x < PATCH_SIZE; x++) {
      ((CVector<3, BYTE> *) (pBuf + x * 4 + y * iRowPitch))->Set(m_pTerrain->m_arrMaterials[m_Points[y][x].m_iMaterial].m_clrAverage);
      pBuf[x * 4 + y * iRowPitch + 3] = 255;
    }

  m_pTexFar->Unmap();

  static const CStrConst sg_txFar("g_txFar");

  if (m_pMinLODModel)
    m_pMinLODModel->SetVar(sg_txFar, CVar<CTexture *>(m_pTexFar));

  return true;
}

CRect<int> CTerrain::CPatch::GetTexRectPixel()
{
  CRect<int> rcRes;
  CLODPatch *pLODPatch = GetLODPatch();

  rcRes.m_vMin = m_vPatchIndex - pLODPatch->m_pPatches[0][0]->m_vPatchIndex;
  rcRes.m_vMin *= PATCH_SIZE;
  rcRes.m_vMax = rcRes.m_vMin + (PATCH_SIZE - 1);

  return rcRes;
}

void CTerrain::CPatch::SetTexTransform(CMatrix<3, 2> &mXForm)
{
  CVector<2> vMin, vMax;
  CRect<int> rcPixel = GetTexRectPixel();

  vMin.x() = (float) rcPixel.m_vMin.x() + 0.5f;
  vMin.y() = (float) rcPixel.m_vMin.y() + 0.5f;
  vMax.x() = (float) rcPixel.m_vMax.x() - 0.5f + 1;
  vMax.y() = (float) rcPixel.m_vMax.y() - 0.5f + 1;
  vMin /= PATCH_SIZE * LOD_PATCH_SIZE;
  vMax /= PATCH_SIZE * LOD_PATCH_SIZE;

  mXForm(0, 0) = (vMax.x() - vMin.x()) / (PATCH_SIZE - 1);
  mXForm(1, 1) = (vMax.y() - vMin.y()) / (PATCH_SIZE - 1);
  mXForm(2, 0) = vMin.x();
  mXForm(2, 1) = vMin.y();
  mXForm(0, 1) = mXForm(1, 0) = 0;
}

bool CTerrain::CPatch::InitMinMaxHeight()
{
  int x, y;

  m_fMinHeight = Util::F_INFINITY;
  m_fMaxHeight = Util::F_NEG_INFINITY;

  for (y = 0; y < PATCH_SIZE; y++)
    for (x = 0; x < PATCH_SIZE; x++) {
      if (m_fMinHeight > m_Points[y][x].m_fHeight)
        m_fMinHeight = m_Points[y][x].m_fHeight;
      if (m_fMaxHeight < m_Points[y][x].m_fHeight)
        m_fMaxHeight = m_Points[y][x].m_fHeight;
    }

  return true;
}

void CTerrain::CPatch::GetAdjacentPatches(CPatch *&pLeft, CPatch *&pUp, CPatch *&pRight, CPatch *&pDown)
{
  pLeft = m_pTerrain->GetPatchByIndex(m_vPatchIndex.x() - 1, m_vPatchIndex.y());
  pUp = m_pTerrain->GetPatchByIndex(m_vPatchIndex.x(), m_vPatchIndex.y() - 1);
  pRight = m_pTerrain->GetPatchByIndex(m_vPatchIndex.x() + 1, m_vPatchIndex.y());
  pDown = m_pTerrain->GetPatchByIndex(m_vPatchIndex.x(), m_vPatchIndex.y() + 1);
}

void CTerrain::CPatch::RecordMeshData(CMesh &kMesh, UINT uiMaterialCollapses)
{
  int i;

  m_uiMaterialCollapses = uiMaterialCollapses;
  m_uiRealCollapses = -1;

  m_arrCollapses.SetMaxCount(kMesh.m_arrCollapses.m_iCount);
  m_arrCollapses.SetCount(0);
  for (i = 0; i < kMesh.m_arrCollapses.m_iCount; i++) {
    m_arrCollapses.Append(kMesh.m_arrCollapses[i].m_iVertexIndex);
    m_arrCollapses.Append(kMesh.m_arrCollapses[i].m_iCollapseToIndex);
    if (m_uiRealCollapses == -1 && kMesh.m_arrCollapses[i].m_iCollapseToIndex == -1)
      m_uiRealCollapses = i;
    ASSERT(m_arrCollapses[2 * i] == kMesh.m_arrCollapses[i].m_iVertexIndex);
    ASSERT(m_arrCollapses[2 * i + 1] == kMesh.m_arrCollapses[i].m_iCollapseToIndex || kMesh.m_arrCollapses[i].m_iCollapseToIndex == -1);
  }
  ASSERT(m_uiRealCollapses != -1);

  m_arrCollapsedTriangles.SetMaxCount(kMesh.m_arrCollapsedTriangles.m_iCount);
  m_arrCollapsedTriangles.SetCount(0);
  for (i = 0; i < kMesh.m_arrCollapsedTriangles.m_iCount; i++) {
    m_arrCollapsedTriangles.Append(kMesh.m_arrCollapsedTriangles[i].m_iIndex);
    ASSERT(m_arrCollapsedTriangles[i] == kMesh.m_arrCollapsedTriangles[i].m_iIndex);
  }

  ASSERT(!m_pModel || kMesh.m_pOrgGeom == m_pModel->m_pGeometry);
}

void CTerrain::CPatch::RestoreMeshFromData(CMesh &kMesh, CGeometry *pOrgGeom)
{
  int i, x, y;

  kMesh.m_arrCollapses.SetCount(m_arrCollapses.m_iCount / 2);
  for (i = 0; i < (int) m_uiRealCollapses; i++) {
    kMesh.m_arrCollapses[i].m_iVertexIndex = m_arrCollapses[2 * i];
    kMesh.m_arrCollapses[i].m_iCollapseToIndex = m_arrCollapses[2 * i + 1];
  }
  while (i < m_arrCollapses.m_iCount / 2) {
    kMesh.m_arrCollapses[i].m_iVertexIndex = m_arrCollapses[2 * i];
    kMesh.m_arrCollapses[i].m_iCollapseToIndex = -1;
    i++;
  }

  kMesh.m_arrCollapsedTriangles.SetCount(m_arrCollapsedTriangles.m_iCount);
  for (i = 0; i < m_arrCollapsedTriangles.m_iCount; i++) 
    kMesh.m_arrCollapsedTriangles[i].m_iIndex = m_arrCollapsedTriangles[i];

  ASSERT(m_arrCollapses.m_iCount == (PATCH_SIZE * PATCH_SIZE + EDGE_INDICES) * 2);
  kMesh.m_arrVertexMaterials.SetMaxCount(PATCH_SIZE * PATCH_SIZE + EDGE_INDICES);
  kMesh.m_arrVertexMaterials.SetCount(0);
  for (i = 0; i < EDGE_INDICES; i++) {
    CVector<2, int> vPos;
    vPos = EdgeIndex2Grid(i);
    kMesh.m_arrVertexMaterials.Append(m_Points[vPos.y()][vPos.x()].m_iMaterial);
  }
  for (y = 0; y < PATCH_SIZE; y++)
    for (x = 0; x < PATCH_SIZE; x++)
      kMesh.m_arrVertexMaterials.Append(m_Points[y][x].m_iMaterial);

  kMesh.m_pOrgGeom = pOrgGeom;
  kMesh.BuildReverseReorder(EDGE_INDICES);
}

void CTerrain::CPatch::SetMaterialModels(CPatchModelBuilder *pBuilder)
{
  ASSERT(!IsProgressiveGeom());
   
  if (!m_EdgeMap.IsInitialized())
    m_EdgeMap.Init(*pBuilder->m_pMesh);

  bool bRes = SetProgGeom(pBuilder);
  ASSERT(bRes);

  ASSERT(!!m_pMinLODModel != !!pBuilder->m_pMinLODGeometry);
  if (pBuilder->m_pMinLODGeometry) {
    m_pMinLODModel = new CModel();
    m_pMinLODModel->Init(pBuilder->m_pMinLODGeometry, m_pTerrain->m_pLODMaterial, 0, false);
    InitParams(m_pMinLODModel, -1, 0);
  }

  m_pModel = 0;

  if (!HasMeshData())
    RecordMeshData(*pBuilder->m_pMesh, pBuilder->m_uiMaterialCollapses);
}

void CTerrain::CPatch::UpdateLOD(CCamera *pCamera)
{
  if (!m_bBuilding && !IsProgressiveGeom()) {
    SetFullGeom(true);
    m_bBuilding = true;
    CPatchModelBuilder *pBuilder = new CPatchModelBuilder(this);
    CScopeLock kLock(&m_pTerrain->m_Lock);
    m_pTerrain->m_lstBuildRequests.PushTail(pBuilder);
  }
  m_pGeom->UpdateLOD(pCamera);
}

bool CTerrain::CPatch::Render()
{
  bool bRes;
  bRes = m_pGeom->Render();
  return bRes;
}
