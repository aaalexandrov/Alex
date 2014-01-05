#include "stdafx.h"
#include "Mesh.h"
#include "Matrix.h"
#include "Shape.h"
#include "Model.h"

// CMesh::TVertex -------------------------------------------------------------

void CMesh::TVertex::AddTriangle(TTriangle *pTri) 
{ 
  if (!m_pTriangles) 
    m_pTriangles = new CTriangleArray();  
  ASSERT(GetTriangleIndex(pTri) < 0);
  m_pTriangles->Append(pTri); 
}

void CMesh::TVertex::RemoveTriangle(TTriangle *pTri)
{
  int iInd = GetTriangleIndex(pTri);
  ASSERT(iInd >= 0);
  m_pTriangles->At(iInd) = m_pTriangles->At(m_pTriangles->m_iCount - 1);
  m_pTriangles->SetCount(m_pTriangles->m_iCount - 1);
}

int CMesh::TVertex::GetTriangleIndex(TTriangle *pTri)
{
  int i;
  for (i = m_pTriangles->m_iCount - 1; i >= 0; i--)
    if (m_pTriangles->At(i) == pTri) 
      return i;
  return -1;
}

bool CMesh::TVertex::CheckMaterialAtEdge(TVertex *pVert0, TVertex *pVert1)
{
  // Parameters specify a new edge that will form after the removal of the current vertex
  // This function finds intersections of triangle edges that fan out of the vertex being removed
  // with the new edge, and makes sure interpolated material stays the same at the intersection points
  // This makes sure the vertex removal will not cause material change in the area of any of the changed triangles
  int iTri, i;
  TTriangle *pTri;
  TVertex *pOther[2];
  CVector<2> vEdge0, vEdge1, vThis, vOther;
  float fTEdge, fTOther, fME0, fME1, fMOther, fME, fMOtherMid;

  vEdge0.Set(pVert0->m_vCoord);
  vEdge1.Set(pVert1->m_vCoord);
  vThis.Set(m_vCoord);
  fME0 = (pVert0->m_iMaterial == m_iMaterial);
  fME1 = (pVert1->m_iMaterial == m_iMaterial);
  for (iTri = 0; iTri < m_pTriangles->m_iCount; iTri++) {
    pTri = m_pTriangles->At(iTri);
    pTri->GetOtherVertices(this, pOther[0], pOther[1]);
    for (i = 0; i < 2; i++) {
      vOther.Set(pOther[i]->m_vCoord);
      if (!IntersectSegments(vEdge0, vEdge1, vThis, vOther, fTEdge, fTOther))
        continue;
      fMOther = (pOther[i]->m_iMaterial == m_iMaterial);
      fME = fME0 + (fME1 - fME0) * fTEdge;
      fMOtherMid = 1 + (fMOther - 1) * fTOther;
      if (!IsEqual(fME, fMOtherMid))
        return false;
    }
  }
  return true;
}

// CMesh::TEdge ---------------------------------------------------------------

CMesh::TEdge::TEdge(TVertex *pVert0, TVertex *pVert1) 
{ 
  if (pVert0 > pVert1) 
    Util::Swap(pVert0, pVert1); 
  m_pVertices[0] = pVert0; 
  m_pVertices[1] = pVert1; 
  m_pTriangles[0] = m_pTriangles[1] = 0; 
  m_fEval[0] = m_fEval[1] = -1; 
  m_iHeapInd[0] = m_iHeapInd[1] = -1;
}

void CMesh::TEdge::AddTriangle(TTriangle *pTri) 
{ 
  ASSERT(!m_pTriangles[0] || !m_pTriangles[1]);
  if (!m_pTriangles[0])
    m_pTriangles[0] = pTri;
  else {
    m_pTriangles[1] = pTri;
#ifdef _DEBUG
//    CheckTrianglesFacing();
#endif
  }
  m_fEval[0] = m_fEval[1] = -1;
}

void CMesh::TEdge::RemoveTriangle(TTriangle *pTri)
{
  ASSERT(m_pTriangles[0] == pTri || m_pTriangles[1] == pTri);
  if (m_pTriangles[0] == pTri) {
    m_pTriangles[0] = m_pTriangles[1];
    m_pTriangles[1] = 0;
  } else 
    m_pTriangles[1] = 0;
  m_fEval[0] = m_fEval[1] = -1;
}

void CMesh::TEdge::CheckTrianglesFacing()
{
  TVertex *pVert0, *pVert1;
  int i;

  if (!m_pTriangles[1])
    return;
  for (i = 0; i < 3; i++) {
    if (m_pTriangles[0]->m_pVertices[i] != m_pVertices[0] && m_pTriangles[0]->m_pVertices[i] != m_pVertices[1])
      pVert0 = m_pTriangles[0]->m_pVertices[i];
    if (m_pTriangles[1]->m_pVertices[i] != m_pVertices[0] && m_pTriangles[1]->m_pVertices[i] != m_pVertices[1])
      pVert1 = m_pTriangles[1]->m_pVertices[i];
  }
  CVector<3> vEdge, vNorm0, vNorm1, vSide0, vSide1;

  vEdge = m_pVertices[1]->m_vCoord - m_pVertices[0]->m_vCoord;
  vSide0 = pVert0->m_vCoord - m_pVertices[0]->m_vCoord;
  vSide1 = pVert1->m_vCoord - m_pVertices[0]->m_vCoord;
  vEdge.z() = 0;
  vSide0.z() = 0;
  vSide1.z() = 0;
  vNorm0 = vEdge ^ vSide0;
  vNorm1 = vEdge ^ vSide1;

  float fNormDot = vNorm0.Normalize() % vNorm1.Normalize();
  ASSERT(fNormDot < -0.05f);
}

// CMesh::TTriangle -----------------------------------------------------------

CMesh::TTriangle::TTriangle(TVertex *pVert0, TVertex *pVert1, TVertex *pVert2)
{
  ASSERT(pVert0 != pVert1 && pVert1 != pVert2 && pVert2 != pVert0);
  if (pVert0 > pVert1)
    Util::Swap(pVert0, pVert1);
  if (pVert1 > pVert2)
    Util::Swap(pVert1, pVert2);
  if (pVert0 > pVert1)
    Util::Swap(pVert0, pVert1);
  m_pVertices[0] = pVert0;
  m_pVertices[1] = pVert1;
  m_pVertices[2] = pVert2;
}

void CMesh::TTriangle::GetOtherVertices(TVertex *pVert, TVertex *&pOther0, TVertex *&pOther1)
{
  ASSERT(pVert == m_pVertices[0] || pVert == m_pVertices[1] || pVert == m_pVertices[2]);
  if (pVert == m_pVertices[0]) {
    pOther0 = m_pVertices[1];
    pOther1 = m_pVertices[2];
  } else
    if (pVert == m_pVertices[1]) {
      pOther0 = m_pVertices[0];
      pOther1 = m_pVertices[2];
    } else {
      pOther0 = m_pVertices[0];
      pOther1 = m_pVertices[1];
    }
}

CVector<3> CMesh::TTriangle::GetNormal()
{
  CVector<3> v0, v1, vNorm;
  v0 = m_pVertices[1]->m_vCoord - m_pVertices[0]->m_vCoord;
  v1 = m_pVertices[2]->m_vCoord - m_pVertices[0]->m_vCoord;
  vNorm = v0 ^ v1;
  return vNorm;
}

bool CMesh::TTriangle::CheckSwapInversion(TVertex *pCurrent, TVertex *pNew)
{
  ASSERT(pCurrent == m_pVertices[0] || pCurrent == m_pVertices[1] || pCurrent == m_pVertices[2]);
  ASSERT(pNew != m_pVertices[0] && pNew != m_pVertices[1] && pNew != m_pVertices[2]);
  TVertex *pVert0, *pVert1;
  GetOtherVertices(pCurrent, pVert0, pVert1);
  CVector<3> vNormCur, vNormNew, vEdge, vCurSide, vNewSide;
  vEdge = pVert1->m_vCoord - pVert0->m_vCoord;
  vCurSide = pCurrent->m_vCoord - pVert0->m_vCoord;
  vNormCur = vEdge ^ vCurSide;
  vNormCur.Normalize();
  vNewSide = pNew->m_vCoord - pVert0->m_vCoord;
  vNormNew = vEdge ^ vNewSide;
  float fLenNew = vNormNew.Length();
  float fSideLen;
  fSideLen = Util::Max(vEdge.Length(), Util::Max(vNewSide.Length(), (pNew->m_vCoord - pVert1->m_vCoord).Length()));
  // fLenNew = 2 * New Triangle Area. Check if the ratio between the area and length of the longest side isn't too big, i.e. the triangle will get too elongated
  if (fSideLen * fSideLen / fLenNew >= MAX_ELONGATION)
    return true;
  if (fLenNew < 0.5)
    return true;
  vNormNew *= 1 / fLenNew;
  return vNormCur % vNormNew < 0.05 || Util::Sign(vNormCur.z()) != Util::Sign(vNormNew.z());
}

void CMesh::TTriangle::RecordCollapse(CArray<TCollapsedTriangle> &arrCollapsedTriangles)
{
  int iCol;
  iCol = arrCollapsedTriangles.m_iCount;
  arrCollapsedTriangles.SetCount(iCol + 1);
  arrCollapsedTriangles[iCol].m_iIndex = m_iIndex;
/*  for (int iVert = 0; iVert < 3; iVert++) 
    arrCollapsedTriangles[iCol].m_iMaterials[iVert] = m_pVertices[iVert]->m_iMaterial;
*/
}

// CMesh ----------------------------------------------------------------------

CMesh::CMesh()
{
  m_bConsiderMaterials = true;
}

CMesh::~CMesh()
{
  DoneInputs();
}

bool CMesh::Init(CGeometry *pOrgGeom, CStrAny sMatSemantic, uint8_t btMatSemanticIndex)
{
  ASSERT(pOrgGeom && pOrgGeom->m_ePrimitiveType == CGeometry::PT_TRIANGLELIST);
  UINT uiVertices, uiIndices, i;

  m_pOrgGeom = pOrgGeom;

  uiVertices = pOrgGeom->GetVBVertexCount();
  uiIndices = pOrgGeom->GetIBIndexCount();
  ASSERT(uiVertices && uiIndices);
  CAutoDeleteArrayPtr<TVertex *> pVertices(new TVertex *[uiVertices]);

  static CStrAny sPOSITION(ST_CONST, "POSITION");

  int iPosIndex, iPosOffset, iVertSize;
  int iMatIndex, iMatOffset;
  CInputDesc::TInputElement *pPosElem, *pMatElem;

  pPosElem = pOrgGeom->m_pInputDesc->GetElementInfo(sPOSITION, 0, &iPosIndex);
  ASSERT(pPosElem && pPosElem->m_Type == CInputDesc::T_FLOAT && pPosElem->m_btElements == 3);
  iPosOffset = pOrgGeom->m_pInputDesc->GetElementOffset(iPosIndex);
  iVertSize = pOrgGeom->m_pInputDesc->GetSize();

  if (sMatSemantic.Length()) {
    pMatElem = pOrgGeom->m_pInputDesc->GetElementInfo(sMatSemantic, btMatSemanticIndex, &iMatIndex);
    ASSERT(pMatElem && pMatElem->m_Type == CInputDesc::T_FLOAT);
    iMatOffset = pOrgGeom->m_pInputDesc->GetElementOffset(iMatIndex);
  } else
    iMatOffset = -1;

  UINT uiVertMapFlags = (pOrgGeom->m_pVB->m_uiFlags & CResource::RF_KEEPSYSTEMCOPY) ? CResource::RMF_SYSTEM_ONLY : 0;
  uint8_t *pVertex = pOrgGeom->m_pVB->Map(0, uiVertMapFlags);
  ASSERT(pVertex);

  int iMatID = 0;
  for (i = 0; i < uiVertices; i++) {
    CVector<3> *pPos = (CVector<3> *) (pVertex + iPosOffset);
    if (iMatOffset >= 0)
      iMatID = (int) *(float *) (pVertex + iMatOffset);
    pVertices[i] = AddVertex(*pPos, iMatID);
    pVertex += iVertSize;
  }

  pOrgGeom->m_pVB->Unmap();

  UINT uiIndMapFlags = (pOrgGeom->m_pIB->m_uiFlags & CResource::RF_KEEPSYSTEMCOPY) ? CResource::RMF_SYSTEM_ONLY : 0;
  uint16_t *pIndex = (uint16_t *) pOrgGeom->m_pIB->Map(0, uiIndMapFlags);
  ASSERT(pIndex);

  for (i = 0; i < uiIndices; i += 3) {
    ASSERT(pIndex[0] < uiVertices && pIndex[1] < uiVertices && pIndex[2] < uiVertices);
    AddTriangle(pVertices[pIndex[0]], pVertices[pIndex[1]], pVertices[pIndex[2]]);
    pIndex += 3;
  }

  pOrgGeom->m_pIB->Unmap();

  return true;
}

CD3DBuffer *CMesh::BuildVB(UINT uiNoReorderVertexCount)
{
  ASSERT(!m_pVB);

  if (uiNoReorderVertexCount >= (UINT) m_arrCollapses.m_iCount) 
    return m_pOrgGeom->m_pVB;

  CD3DBuffer *pVB = new CD3DBuffer();

  pVB->Init(CResource::RT_VERTEX, m_pOrgGeom->m_pVB->m_uiFlags, 0, m_pOrgGeom->m_pVB->GetSize(0));

  int i, iVertSize;
  uint8_t *pSrcVertex, *pDstVertex;
  UINT uiSrcMapFlags = (m_pOrgGeom->m_pVB->m_uiFlags & CResource::RF_KEEPSYSTEMCOPY) ? CResource::RMF_SYSTEM_ONLY : 0;

  iVertSize = m_pOrgGeom->m_pInputDesc->GetSize();
  pDstVertex = pVB->Map();
  pSrcVertex = m_pOrgGeom->m_pVB->Map(0, uiSrcMapFlags);

  ASSERT(pVB->GetSize(0) == iVertSize * m_arrReverseReorder.m_iCount);

  for (i = 0; i < m_arrReverseReorder.m_iCount; i++) 
    memcpy(pDstVertex + m_arrReverseReorder[i] * iVertSize, pSrcVertex + i * iVertSize, iVertSize);

  m_pOrgGeom->m_pVB->Unmap();
  pVB->Unmap();

  return pVB;
}

CD3DBuffer *CMesh::BuildIB(int iMaterial, CArray<uint8_t> &arrUsedVertices)
{
  int i, j;
  uint16_t *pSrcIndices;
  CArray<uint16_t> arrDstIndices(m_arrCollapsedTriangles.m_iCount * 3);

  UINT uiSrcMapFlags = (m_pOrgGeom->m_pIB->m_uiFlags & CResource::RF_KEEPSYSTEMCOPY) ? CResource::RMF_SYSTEM_ONLY : 0;
  pSrcIndices = (uint16_t *) m_pOrgGeom->m_pIB->Map(0, uiSrcMapFlags);

  arrUsedVertices.SetCount(m_arrCollapses.m_iCount);
  memset(arrUsedVertices.m_pArray, 0, arrUsedVertices.m_iCount * sizeof(uint8_t));

  for (i = 0; i < m_arrCollapsedTriangles.m_iCount; i++) {
    TCollapsedTriangle &kColTri = m_arrCollapsedTriangles[m_arrCollapsedTriangles.m_iCount - 1 - i];
    int iSrcTriBase = kColTri.m_iIndex * 3;
    if (iMaterial >= 0 && m_arrVertexMaterials[pSrcIndices[iSrcTriBase]] != iMaterial &&
        m_arrVertexMaterials[pSrcIndices[iSrcTriBase + 1]] != iMaterial &&
        m_arrVertexMaterials[pSrcIndices[iSrcTriBase + 2]] != iMaterial)
      continue;
    for (j = 0; j < 3; j++) {
      int iInd = m_arrReverseReorder[pSrcIndices[iSrcTriBase + j]];
      arrDstIndices.Append(iInd);
      arrUsedVertices[iInd] = 1;
    }
  }

  m_pOrgGeom->m_pIB->Unmap();

  CD3DBuffer *pIB = new CD3DBuffer();
  pIB->Init(CResource::RT_INDEX, m_pOrgGeom->m_pIB->m_uiFlags, (uint8_t *) arrDstIndices.m_pArray, arrDstIndices.m_iCount * sizeof(uint16_t));

  return pIB;
}

UINT CMesh::GetMaxCollapses()
{
  int i;
  for (i = m_arrCollapses.m_iCount - 1; i >= 0; i++)
    if (m_arrCollapses[i].m_iCollapseToIndex > 0)
      break;
  return i + 1;
}

// CIndexChain

class CIndexChain { // maintains chains of all index buffer positions that reference a particular vertex
public:
  CArray<int> m_arrChains;
  CArray<int> m_arrVertexChain;
  CAVLTree<int> m_avlVertices;

  void Init(uint16_t *pIndices, int iIndexCount, int iVertexCount, bool bTrackVertices);
  void MergeChains(int iVertex, int iCollapseTo, int iIndexCount);
  int GetLastActiveVertex();
};

void CIndexChain::Init(uint16_t *pIndices, int iIndexCount, int iVertexCount, bool bTrackVertices)
{
  int i;
  m_arrChains.SetCount(iIndexCount);
  m_arrVertexChain.SetCount(iVertexCount);
  memset(m_arrVertexChain.m_pArray, -1, m_arrVertexChain.m_iCount * sizeof(int));
  for (i = iIndexCount - 1; i >= 0; i--) {
    m_arrChains[i] = m_arrVertexChain[pIndices[i]];
    m_arrVertexChain[pIndices[i]] = i;
    if (bTrackVertices)
      m_avlVertices.AddUnique(pIndices[i]);
  }
}

void CIndexChain::MergeChains(int iVertex, int iCollapseTo, int iIndexCount)
{
  int *pDst, iNextVert, iNextColl;
  pDst = &m_arrVertexChain[iCollapseTo];
  iNextVert = m_arrVertexChain[iVertex];
  iNextColl = m_arrVertexChain[iCollapseTo];
  while (iNextVert >= 0 && iNextColl >= 0 && iNextVert < iIndexCount && iNextColl < iIndexCount) {
    if (iNextVert < iNextColl) {
      *pDst = iNextVert;
      pDst = &m_arrChains[iNextVert];
      iNextVert = *pDst;
    } else {
      *pDst = iNextColl;
      pDst = &m_arrChains[iNextColl];
      iNextColl = *pDst;
    }
  }
  if (iNextVert >= iIndexCount)
    iNextVert = -1;
  if (iNextColl >= iIndexCount)
    iNextColl = -1;
  if (iNextVert >= 0)
    *pDst = iNextVert;
  else
    *pDst = iNextColl;
  m_arrVertexChain[iVertex] = -1;
  m_avlVertices.RemoveValue(iVertex);
}

int CIndexChain::GetLastActiveVertex()
{
  CAVLTree<int>::TIter it(&m_avlVertices, 1);
  ASSERT(it);
  if (it)
    return *it + 1;
  return 0;
}

// CMesh

void CMesh::BuildCollapseBuffer(bool bExplicitVertexCount, UINT uiMaxCollapses, CArray<uint8_t> const &arrUsedVertices, CProgressiveGeometry *pProgGeom)
{
  int iCollapse, iPos;
  CArray<UINT> arrCollapses;
  CArray<uint16_t> arrTriIndices;

  UINT uiSrcMapFlags = (pProgGeom->m_pIB->m_uiFlags & CResource::RF_KEEPSYSTEMCOPY) ? CResource::RMF_SYSTEM_ONLY : 0;
  uint8_t *pSrcIndices = pProgGeom->m_pIB->Map(0, uiSrcMapFlags);
  
  arrTriIndices.SetCount(pProgGeom->GetIBIndexCount());
  memcpy(arrTriIndices.m_pArray, pSrcIndices, arrTriIndices.m_iCount * sizeof(uint16_t));
  
  pProgGeom->m_pIB->Unmap();

  CIndexChain kChain;
  kChain.Init(arrTriIndices.m_pArray, arrTriIndices.m_iCount, pProgGeom->GetVBVertexCount(), bExplicitVertexCount);

  for (iCollapse = 0; iCollapse < (int) Util::Min<UINT>(m_arrCollapses.m_iCount, uiMaxCollapses); iCollapse++) {
    if (m_arrCollapses[iCollapse].m_iCollapseToIndex < 0)
      break;
    int iVertIndex, iCollapseToIndex, iBaseIndex;
    iVertIndex = m_arrReverseReorder[m_arrCollapses[iCollapse].m_iVertexIndex];
    if (!arrUsedVertices[iVertIndex])
      continue;
    iCollapseToIndex = m_arrReverseReorder[m_arrCollapses[iCollapse].m_iCollapseToIndex];
    ASSERT(arrUsedVertices[iCollapseToIndex]);
    iBaseIndex = arrCollapses.m_iCount;
    arrCollapses.Append(iVertIndex);
    arrCollapses.Append(iCollapseToIndex);

    if (bExplicitVertexCount) // Reserve space for maximum vertex index
      arrCollapses.SetCount(arrCollapses.m_iCount + 1);

    iPos = kChain.m_arrVertexChain[iVertIndex];
    ASSERT(iPos >= 0);
    while (iPos >= 0 && iPos < arrTriIndices.m_iCount) {
      arrCollapses.Append(iPos);
      ASSERT(arrTriIndices[iPos] == iVertIndex);
      arrTriIndices[iPos] = iCollapseToIndex;
      ASSERT(iPos < kChain.m_arrChains[iPos] || kChain.m_arrChains[iPos] < 0);
      iPos = kChain.m_arrChains[iPos];
    }

    // Shrink index buffer 
    while (arrTriIndices.m_iCount && CGeometry::IsTriangleDegenerate(&arrTriIndices[arrTriIndices.m_iCount - 3]))
      arrTriIndices.SetCount(arrTriIndices.m_iCount - 3);

    kChain.MergeChains(iVertIndex, iCollapseToIndex, arrTriIndices.m_iCount);

    if (bExplicitVertexCount) 
      arrCollapses[iBaseIndex + 2] = kChain.GetLastActiveVertex();

    arrCollapses.Append(CProgressiveGeometry::INVALID_INDEX);
  }

  pProgGeom->SetCollapses(arrCollapses.m_iCount, arrCollapses.m_pArray, bExplicitVertexCount);
}

void CMesh::BuildReverseReorder(UINT uiNoReorderVertexCount)
{
  int i;

  RecordRemaining();
  m_arrReverseReorder.SetCount(m_arrCollapses.m_iCount);
  int iSkipped = 0;
  for (i = 0; i < m_arrCollapses.m_iCount; i++) {
    int iInd = m_arrCollapses[i].m_iVertexIndex;
    if ((UINT) iInd < uiNoReorderVertexCount) {
      m_arrReverseReorder[iInd] = iInd;
      iSkipped++;
    } else
      m_arrReverseReorder[iInd] = m_arrCollapses.m_iCount - 1 - i + iSkipped;
  }
  ASSERT(iSkipped == uiNoReorderVertexCount);
}

CProgressiveGeometry *CMesh::BuildProgressiveGeometry(int iMaterial, UINT uiNoReorderVertexCount, UINT uiMaxCollapses)
{
  CProgressiveGeometry *pProgGeom = new CProgressiveGeometry();
  CArray<uint8_t> arrUsedVertices;

  if (uiNoReorderVertexCount > (UINT) (m_arrCollapses.m_iCount + m_hashVertices.m_iCount))
    uiNoReorderVertexCount = m_arrCollapses.m_iCount + m_hashVertices.m_iCount;

  if (!m_arrReverseReorder.m_iCount)
    BuildReverseReorder(uiNoReorderVertexCount);
  
  pProgGeom->m_pInputDesc = m_pOrgGeom->m_pInputDesc;
  if (!m_pVB)
    m_pVB = BuildVB(uiNoReorderVertexCount);
  pProgGeom->m_pVB = m_pVB;
  pProgGeom->m_pIB = BuildIB(iMaterial, arrUsedVertices);
  pProgGeom->m_ePrimitiveType = CGeometry::PT_TRIANGLELIST;
  pProgGeom->m_uiVertices = pProgGeom->GetVBVertexCount();
  pProgGeom->m_uiIndices = pProgGeom->GetIBIndexCount();
  if (m_pOrgGeom->m_pBound)
    pProgGeom->m_pBound = m_pOrgGeom->m_pBound->Clone();

  BuildCollapseBuffer(uiNoReorderVertexCount || iMaterial >= 0, uiMaxCollapses, arrUsedVertices, pProgGeom);

  return pProgGeom;
}

CMesh::TVertex *CMesh::AddVertex(CVector<3> const &vCoord, int iMaterial)
{
  TVertex *pVertex = new TVertex();
  pVertex->m_vCoord = vCoord;
  pVertex->m_iIndex = m_hashVertices.m_iCount;
  pVertex->m_iMaterial = iMaterial;
  m_hashVertices.Add(pVertex);
  ASSERT(m_arrVertexMaterials.m_iCount == pVertex->m_iIndex);
  m_arrVertexMaterials.Append(iMaterial);
  return pVertex;
}

CMesh::TTriangle *CMesh::AddTriangle(TVertex *pVert0, TVertex *pVert1, TVertex *pVert2, int iIndex)
{
  ASSERT(m_hashVertices.Find(pVert0) && m_hashVertices.Find(pVert1) && m_hashVertices.Find(pVert2));

#ifdef _DEBUG
  {
  TTriangle kTestTri(pVert0, pVert1, pVert2);
  TTriangleHash::TIter it = m_hashTriangles.Find(&kTestTri);
  ASSERT(!it);

  CVector<3> vNormal = kTestTri.GetNormal();
  float fArea = vNormal.Length();
  ASSERT(fArea > 0.25);
  float fEdgeLen = Util::Max((pVert0->m_vCoord - pVert1->m_vCoord).Length(), 
                             Util::Max((pVert1->m_vCoord - pVert2->m_vCoord).Length(), 
                                       (pVert2->m_vCoord - pVert0->m_vCoord).Length()));
  ASSERT(fEdgeLen * fEdgeLen / fArea < MAX_ELONGATION);
  }
#endif

  TTriangle *pTri = new TTriangle(pVert0, pVert1, pVert2);
  if (iIndex >= 0)
    pTri->m_iIndex = iIndex;
  else
    pTri->m_iIndex = m_hashTriangles.m_iCount;
  m_hashTriangles.Add(pTri);

  pVert0->AddTriangle(pTri);
  pVert1->AddTriangle(pTri);
  pVert2->AddTriangle(pTri);

  TEdge *pEdge0, *pEdge1, *pEdge2;
  pEdge0 = GetEdge(pVert0, pVert1, true);
  pEdge0->AddTriangle(pTri);
  pEdge1 = GetEdge(pVert1, pVert2, true);
  ASSERT(pEdge1 != pEdge0);
  pEdge1->AddTriangle(pTri);
  pEdge2 = GetEdge(pVert2, pVert0, true);
  ASSERT(pEdge2 != pEdge1 && pEdge2 != pEdge0);
  pEdge2->AddTriangle(pTri);

  return pTri;
}

void CMesh::RemoveTriangle(TTriangle *pTri)
{
  if (!pTri)
    return;

  ASSERT(m_hashTriangles.Find(pTri));
  pTri->m_pVertices[0]->RemoveTriangle(pTri);
  pTri->m_pVertices[1]->RemoveTriangle(pTri);
  pTri->m_pVertices[2]->RemoveTriangle(pTri);

  RemoveTriangleEdge(pTri, 0);
  RemoveTriangleEdge(pTri, 1);
  RemoveTriangleEdge(pTri, 2);

  m_hashTriangles.RemoveValue(pTri);
  delete pTri;
}

void CMesh::RemoveTriangleEdge(TTriangle *pTri, int iEdge)
{
  TEdge *pEdge = GetEdge(pTri->m_pVertices[iEdge], pTri->m_pVertices[(iEdge + 1) % 3]);
  ASSERT(pEdge);
  pEdge->RemoveTriangle(pTri);
  if (!pEdge->m_pTriangles[0]) {
    m_hashEdges.RemoveValue(pEdge);
    ASSERT(pEdge->m_iHeapInd[0] >= 0 && pEdge->m_iHeapInd[1] >= 0);
    ASSERT(m_heapEdgeRefs[pEdge->m_iHeapInd[0]].m_pEdge == pEdge);
    ASSERT(m_heapEdgeRefs[pEdge->m_iHeapInd[1]].m_pEdge == pEdge);
    m_heapEdgeRefs.Remove(pEdge->m_iHeapInd[0]);
    m_heapEdgeRefs.Remove(pEdge->m_iHeapInd[1]);
    delete pEdge;
  }
}

void CMesh::InitOutputs()
{
  m_arrCollapses.SetCount(0);
  m_arrCollapses.SetMaxCount(m_hashVertices.m_iCount);
  m_arrCollapsedTriangles.SetCount(0);
  m_arrCollapsedTriangles.SetMaxCount(m_hashTriangles.m_iCount);
  m_arrReverseReorder.SetCount(0);
  m_arrReverseReorder.SetMaxCount(m_hashVertices.m_iCount);
}

void CMesh::InitBorders()
{
  TEdgeHash::TIter it;
  for (it = m_hashEdges; it; ++it) 
    if (it->m_pVertices[0]->m_iMaterial != it->m_pVertices[1]->m_iMaterial) {
      it->m_pVertices[0]->m_bOnMaterialBorder = true;
      it->m_pVertices[1]->m_bOnMaterialBorder = true;
    }
}

void CMesh::InitEdgeEvaluation()
{
  TEdgeHash::TIter it;
  for (it = m_hashEdges; it; ++it) 
    for (int i = 0; i < 2; i++)
      EvaluateEdge(*it, !!i);
}

void CMesh::DoneInputs()
{
  m_hashEdges.DeleteAll();
  m_hashTriangles.DeleteAll();
  m_hashVertices.DeleteAll();
}

void CMesh::RecordCollapse(TEdge *pEdge, bool bRemoveSecond)
{
  int i;
  i = m_arrCollapses.m_iCount;
  m_arrCollapses.SetCount(i + 1);
  m_arrCollapses[i].m_iVertexIndex = pEdge->m_pVertices[bRemoveSecond]->m_iIndex;
  m_arrCollapses[i].m_iCollapseToIndex = pEdge->m_pVertices[!bRemoveSecond]->m_iIndex;
  for (i = 0; i < 2; i++) {
    if (!pEdge->m_pTriangles[i])
      continue;
    pEdge->m_pTriangles[i]->RecordCollapse(m_arrCollapsedTriangles);
  }
}

void CMesh::RecordRemaining()
{
  TVertexHash::TIter itVert;
  for (itVert = m_hashVertices; itVert; ++itVert) {
    int i = m_arrCollapses.m_iCount;
    m_arrCollapses.SetCount(i + 1);
    m_arrCollapses[i].m_iVertexIndex = itVert->m_iIndex;
    m_arrCollapses[i].m_iCollapseToIndex = -1;
  }
  TTriangleHash::TIter itTri;
  for (itTri = m_hashTriangles; itTri; ++itTri) 
    itTri->RecordCollapse(m_arrCollapsedTriangles);
  DoneInputs();
}

void CMesh::Simplify(int iVertices2Remain, float fMaxError, bool bConsiderMaterials)
{
  TEdge *pEdge;
  bool bRemoveSecond;

  m_bConsiderMaterials = bConsiderMaterials;
  if (!m_arrCollapses.m_iCount) {
    InitOutputs();
    InitBorders();
  }
  InitEdgeEvaluation();
  while (m_hashVertices.m_iCount > iVertices2Remain) {
    if (!SelectEdge(pEdge, bRemoveSecond, fMaxError))
      break;
    RecordCollapse(pEdge, bRemoveSecond);
    CollapseEdge(pEdge, bRemoveSecond);
//    CheckEdges();
  }
}

CMesh::TEdge *CMesh::GetEdge(TVertex *pVert0, TVertex *pVert1, bool bCreate)
{
  TEdge kEdge(pVert0, pVert1), *pEdge;
  TEdgeHash::TIter it;
  it = m_hashEdges.Find(&kEdge);
  if (it)
    return *it;
  if (!bCreate)
    return 0;
  pEdge = new TEdge(pVert0, pVert1);
  m_hashEdges.Add(pEdge);
  m_heapEdgeRefs.Add(TEdgeRef(pEdge, false, &m_heapEdgeRefs));
  m_heapEdgeRefs.Add(TEdgeRef(pEdge, true, &m_heapEdgeRefs));
  return pEdge;
}

void CMesh::EvaluateEdge(TEdge *pEdge, bool bRemoveSecond)
{
  ASSERT(pEdge);
  TVertex *p2Remain, *p2Remove;

  p2Remain = pEdge->m_pVertices[!bRemoveSecond];
  p2Remove = pEdge->m_pVertices[bRemoveSecond];

  if (m_bConsiderMaterials && 
     (p2Remove->m_bOnMaterialBorder && !p2Remain->m_bOnMaterialBorder || 
      p2Remove->m_iMaterial != p2Remain->m_iMaterial)) {
    pEdge->m_fEval[bRemoveSecond] = Util::F_INFINITY;
    m_heapEdgeRefs.Resort(pEdge->m_iHeapInd[bRemoveSecond]);
    return;
  }

  int i;
  float fEval = -1, fEdgeLen;
  bool bCollapseValid = false;
  CVector<2> v2Remove, v2Remain;
  CVector<3> vEdge;
  v2Remain.Set(p2Remain->m_vCoord);
  v2Remove.Set(p2Remove->m_vCoord);
  vEdge = p2Remain->m_vCoord - p2Remove->m_vCoord;
  fEdgeLen = vEdge.Length();
  ASSERT(!IsEqual(fEdgeLen, 0));
  vEdge *= 1 / fEdgeLen;
  for (i = 0; i < p2Remove->m_pTriangles->m_iCount; i++) {
    TTriangle *pTri = p2Remove->m_pTriangles->At(i);
    TVertex *pVert0, *pVert1;
    pTri->GetOtherVertices(p2Remove, pVert0, pVert1);
    if (pVert0 == p2Remain || pVert1 == p2Remain)
      continue;
    if (pTri->CheckSwapInversion(p2Remove, p2Remain)) {
      bCollapseValid = false;
      break;
    }
    CVector<3> vTriNorm = pTri->GetNormal();
    float fEdgeDiff = abs(vTriNorm % vEdge);
    if (fEdgeDiff > fEval)
      fEval = fEdgeDiff;
    if (!bCollapseValid) {
      float fA0, fA1, fA2;
      CVector<2> v0, v1;
      v0.Set(pVert0->m_vCoord);
      v1.Set(pVert1->m_vCoord);
      GetBaricentricCoordinates(v2Remove, v2Remain, v0, v1, fA0, fA1, fA2);
      if (fA0 >= -0.005f && fA1 >= -0.005f && fA2 >= -0.005f) {
        if (m_bConsiderMaterials) {
          float fM0, fM1, fM2;
          fM0 = p2Remain->m_iMaterial == p2Remove->m_iMaterial;
          fM1 = pVert0->m_iMaterial == p2Remove->m_iMaterial;
          fM2 = pVert1->m_iMaterial == p2Remove->m_iMaterial;
          if (!IsEqual(fM0 * fA0 + fM1 * fA1 + fM2 * fA2, 1)) 
            break; // Collapse is invalid if the interpolated material at the vertex being removed is different from the original material
          // fAn == 0 means the edge opposite to the vertex whose weight fAn is, is colinear with an edge of the newly formed triangle,
          // so it cannot intersect any of the triangle edges that CheckMaterialAtEdge checks for
          if (fA2 > 0 && !p2Remove->CheckMaterialAtEdge(p2Remain, pVert0))
            break;
          if (fA1 > 0 && !p2Remove->CheckMaterialAtEdge(p2Remain, pVert1))
            break;
        }
        bCollapseValid = true;
      }
    }
  }
  if (!bCollapseValid)
    fEval = Util::F_INFINITY;
  ASSERT(fEval >= 0);
  pEdge->m_fEval[bRemoveSecond] = fEval;
  m_heapEdgeRefs.Resort(pEdge->m_iHeapInd[bRemoveSecond]);
}

void CMesh::CollapseEdge(TEdge *pEdge, bool bRemoveSecond)
{
  TVertex *p2Remain, *p2Remove;
  CArray<TVertex *> arrInvalVerts;
  TEdgeHash hashEvaluatedEdges;
  int i, iTri;
  ASSERT(pEdge);
  p2Remain = pEdge->m_pVertices[!bRemoveSecond];
  p2Remove = pEdge->m_pVertices[bRemoveSecond];

  for (i = 0; i < p2Remove->m_pTriangles->m_iCount; i++) {
    TVertex *pOther0, *pOther1;
    p2Remove->m_pTriangles->At(i)->GetOtherVertices(p2Remove, pOther0, pOther1);
    arrInvalVerts.Append(pOther0);
    arrInvalVerts.Append(pOther1);
  }

  ASSERT(pEdge->m_pTriangles[0]);
  // Order of removal is important, if we remove the first one first and it's the only one left, 
  // the edge will get deleted and the attempt to remove the other one will crash
  RemoveTriangle(pEdge->m_pTriangles[1]); 
  RemoveTriangle(pEdge->m_pTriangles[0]);
  ASSERT(!GetEdge(p2Remain, p2Remove));

/*
#ifdef _DEBUG
  {
    for (int i = 0; i < p2Remove->m_pTriangles->m_iCount; i++) {
      TVertex *pVert0, *pVert1;
      TTriangle *pTri = p2Remove->m_pTriangles->At(i);
      pTri->GetOtherVertices(p2Remove, pVert0, pVert1);
      TTriangle kTestTri(p2Remain, pVert0, pVert1);
      TTriangleHash::TIter it = m_hashTriangles.Find(&kTestTri);
      ASSERT(!it);
      if (it)
        TEdge *pE = GetEdge(pVert0, pVert1);
    }
  }
#endif
*/

  while (p2Remove->m_pTriangles->m_iCount > 0) {
    TVertex *pVert0, *pVert1;
    TTriangle *pTri = p2Remove->m_pTriangles->At(p2Remove->m_pTriangles->m_iCount - 1);
    pTri->GetOtherVertices(p2Remove, pVert0, pVert1);
    int iTriIndex = pTri->m_iIndex;
    RemoveTriangle(pTri);
    AddTriangle(p2Remain, pVert0, pVert1, iTriIndex);
  }
  m_hashVertices.RemoveValue(p2Remove);
  delete p2Remove;

  for (i = 0; i < arrInvalVerts.m_iCount; i++) {
    for (iTri = 0; iTri < arrInvalVerts[i]->m_pTriangles->m_iCount; iTri++) {
      TVertex *pOther[2];
      TEdge *pEdge;
      int iVert;
      arrInvalVerts[i]->m_pTriangles->At(iTri)->GetOtherVertices(arrInvalVerts[i], pOther[0], pOther[1]);
      for (iVert = 0; iVert < 2; iVert++) {
        pEdge = GetEdge(arrInvalVerts[i], pOther[iVert]);
        if (hashEvaluatedEdges.Find(pEdge))
          continue;
        EvaluateEdge(pEdge, false);
        EvaluateEdge(pEdge, true);
        hashEvaluatedEdges.Add(pEdge);
      }
    }
  }
}

bool CMesh::SelectEdge(TEdge *&pEdge, bool &bRemoveSecond, float fMaxError)
{
  float fEval;
  pEdge = m_heapEdgeRefs[0].m_pEdge;
  bRemoveSecond = m_heapEdgeRefs[0].m_bSecond;
  fEval = pEdge->m_fEval[bRemoveSecond];
  ASSERT(fEval >= 0);
  return fEval < fMaxError;
}

bool CMesh::CheckEdges()
{
  TEdgeHash::TIter it;
  int i;
  for (it = m_hashEdges; it; ++it) {
    for (i = 0; i < 2; i++)
      if (it->m_pTriangles[i]) {
        TEdge *pEdge0, *pEdge1, *pEdge2;
        pEdge0 = GetEdge(it->m_pTriangles[i]->m_pVertices[0], it->m_pTriangles[i]->m_pVertices[1]);
        pEdge1 = GetEdge(it->m_pTriangles[i]->m_pVertices[1], it->m_pTriangles[i]->m_pVertices[2]);
        pEdge2 = GetEdge(it->m_pTriangles[i]->m_pVertices[2], it->m_pTriangles[i]->m_pVertices[0]);
        ASSERT(*it == pEdge0 || *it == pEdge1 || *it == pEdge2);
      }
  }
  TTriangleHash::TIter itTri;
  for (itTri = m_hashTriangles; itTri; ++itTri) {
    TEdge *pEdge0, *pEdge1, *pEdge2;
    pEdge0 = GetEdge(itTri->m_pVertices[0], itTri->m_pVertices[1]);
    pEdge1 = GetEdge(itTri->m_pVertices[1], itTri->m_pVertices[2]);
    pEdge2 = GetEdge(itTri->m_pVertices[2], itTri->m_pVertices[0]);
    ASSERT(pEdge0 && pEdge0->m_pTriangles[0] == *itTri || pEdge0->m_pTriangles[1] == *itTri);
    ASSERT(pEdge1 && pEdge1->m_pTriangles[0] == *itTri || pEdge1->m_pTriangles[1] == *itTri);
    ASSERT(pEdge2 && pEdge2->m_pTriangles[0] == *itTri || pEdge2->m_pTriangles[1] == *itTri);
  }
  return true;
}

void CMesh::GetBaricentricCoordinates(CVector<2> const &vP, CVector<2> const &v0, CVector<2> const &v1, CVector<2> const &v2,
                                        float &fA0, float &fA1, float &fA2)
{
  CVector<2> vX, vY, vDelta, vXp, vYp;
  vX = v1 - v0;
  vY = v2 - v0;
  vDelta = vP - v0;
  vXp.Set(vX.y(), -vX.x()); // vXp is perpendicular to vX
  vYp.Set(vY.y(), -vY.x()); // vYp is perpendicular to vY
  float fXYp = vX % vYp;
  if (IsEqual(fXYp, 0)) {
    fA0 = fA1 = fA2 = Util::F_NEG_INFINITY;
    return;
  }
  fA1 = (vDelta % vYp) / fXYp;
  fA2 = (vDelta % vXp) / -fXYp; // vXp % vY == -(vX % vYp)
  fA0 = 1 - fA1 - fA2;

  ASSERT(IsEqual(fA0 + fA1 + fA2, 1));
  ASSERT(IsEqual((v0 * fA0 + v1 * fA1 + v2 * fA2 - vP).Length(), 0.0f, 0.05f));
}

bool CMesh::IntersectSegments(CVector<2> const &vA0, CVector<2> const &vA1, CVector<2> const &vB0, CVector<2> const &vB1, float &fTA, float &fTB)
{
  CVector<2> vA, vB, vBP, vAP, vD;
  float fD;

  vA = vA1 - vA0;
  vB = vB1 - vB0;
  vAP = vA.Perpendicular2D();
  vBP = vB.Perpendicular2D();

  fD = vA % vBP;
  if (IsEqual(fD, 0)) // Colinear or one of the vectors is 0, so we return no intersection
    return false;
  vD = vB0 - vA0;
  fTA = (vD % vBP) / fD;
  if (fTA < 0 || fTA > 1)
    return false;
  fTB = (vD % vAP) / fD;
  if (fTB < 0 || fTB > 1)
    return false;

  ASSERT(IsEqual(((vB0 + fTB * vB) - (vA0 + fTA * vA)).Length(), 0));

  return true;
}
