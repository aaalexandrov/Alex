#include "stdafx.h"
#include "GeomUtil.h"

// CGeometryMerge -------------------------------------------------------------

CGeometryMerge::CGeometryMerge(CInputDesc *pInputDesc, bool bSkipUnusedVertices)
{
  m_pInputDesc = pInputDesc;
  m_bSkipUnusedVertices = bSkipUnusedVertices;
  m_pRemap = 0;
  for (int i = 0; i < m_pInputDesc->m_Elements.m_iCount; i++)
    m_arrElementXForms.Append(0);
}

CGeometryMerge::~CGeometryMerge()
{
}

void CGeometryMerge::SetIndexRemap(CIndexRemap *pRemap)
{
  m_pRemap = pRemap;
}

void CGeometryMerge::SetVertexElementTransform(int iElement, CMatrix<4, 4> *pXForm)
{
  m_arrElementXForms[iElement] = pXForm;
}

void CGeometryMerge::AddGeometry(CGeometry *pGeom, UINT uiVertices, UINT uiIndices)
{
  ASSERT(pGeom->m_ePrimitiveType == CGeometry::PT_TRIANGLELIST);
  ASSERT(*pGeom->m_pInputDesc == *m_pInputDesc);
  int iBaseVert, iBaseInd, i, iVertStride;
  BYTE *pSrcVert;
  WORD *pSrcInd;
  UINT uiMapFlags;
  CIndexRemap kRemap, *pRemap;

  if (m_pRemap)
    pRemap = m_pRemap;
  else
    pRemap = &kRemap;

  uiMapFlags = (pGeom->m_pVB->m_uiFlags & CResource::RF_KEEPSYSTEMCOPY) ? CResource::RMF_SYSTEM_ONLY : 0;
  pSrcVert = pGeom->m_pVB->Map(0, uiMapFlags);

  uiMapFlags = (pGeom->m_pIB->m_uiFlags & CResource::RF_KEEPSYSTEMCOPY) ? CResource::RMF_SYSTEM_ONLY : 0;
  pSrcInd = (WORD *) pGeom->m_pIB->Map(0, uiMapFlags);

  if (!uiVertices)
    uiVertices = pGeom->m_uiVertices;
  if (!uiIndices)
    uiIndices = pGeom->m_uiIndices;
  iVertStride = m_pInputDesc->GetSize();
  iBaseVert = m_arrVertices.m_iCount;
  iBaseInd = iBaseVert / iVertStride;

  if (m_bSkipUnusedVertices) { // Compacting vertices so that unused ones aren't copied to the new geometry
    CAVLTreeKV<int, int> avlRemap;
    CAVLTreeKV<int, int>::TIter it;
    for (i = 0; i < (int) uiIndices; i++) {
      int iRemapped = pRemap->RemapIndex(pSrcInd[i]);
      ASSERT(iRemapped < (int) uiVertices);
      avlRemap.AddUnique(TKeyValue<int, int>(pRemap->RemapIndex(pSrcInd[i]), 0));
    }

    m_arrVertices.SetMaxCount(m_arrVertices.m_iCount + avlRemap.m_iCount * iVertStride);

    for (it = avlRemap, i = 0; it; ++it, i++) {
      (*it).m_Val = i;
      ASSERT(i * iVertStride == m_arrVertices.m_iCount - iBaseVert);
      m_arrVertices.SetCount(m_arrVertices.m_iCount + iVertStride);
      TransformVertex(&m_arrVertices[iBaseVert + i * iVertStride], pSrcVert + (*it).m_Key * iVertStride);
    }

    m_arrIndices.SetMaxCount(m_arrIndices.m_iCount + uiIndices);
    for (i = 0; i < (int) uiIndices; i++) {
      it = avlRemap.Find(pRemap->RemapIndex(pSrcInd[i]));
      ASSERT(!!it);
      m_arrIndices.Append((*it).m_Val + iBaseInd);
    }
  } else {
    m_arrVertices.SetCount(m_arrVertices.m_iCount + uiVertices * iVertStride);
    for (i = 0; i < (int) uiVertices; i++)
      TransformVertex(&m_arrVertices[iBaseVert + i * iVertStride], pSrcVert + i * iVertStride);

    m_arrIndices.SetMaxCount(m_arrIndices.m_iCount + uiIndices);
    for (i = 0; i < (int) uiIndices; i++) 
      m_arrIndices.Append(pRemap->RemapIndex(pSrcInd[i]) + iBaseInd);
  }

  pGeom->m_pIB->Unmap();

  pGeom->m_pVB->Unmap();
}

CGeometry *CGeometryMerge::CreateGeometry(CRTTI const *pGeomRTTI, UINT uiVBFlags, UINT uiIBFlags, UINT uiExtraVertices, UINT uiExtraIndices, CRTTI const *pBoundRTTI)
{
  if (!pGeomRTTI)
    pGeomRTTI = &CGeometry::s_RTTI;
  ASSERT(pGeomRTTI->IsKindOf(&CGeometry::s_RTTI));
  if (!pGeomRTTI->IsKindOf(&CGeometry::s_RTTI))
    return 0;
  CGeometry *pGeom = (CGeometry *) pGeomRTTI->CreateInstance();
  bool bRes = pGeom->Init(m_pInputDesc, CGeometry::PT_TRIANGLELIST, 
                          m_arrVertices.m_iCount / m_pInputDesc->GetSize() + uiExtraVertices, m_arrIndices.m_iCount + uiExtraIndices, 
                          0, 0, uiVBFlags, uiIBFlags, pBoundRTTI);
  ASSERT(bRes);
  if (!bRes) {
    SAFE_DELETE(pGeom);
    return 0;
  }

  BYTE *pBuf;
  pBuf = pGeom->m_pVB->Map();
  memcpy(pBuf, m_arrVertices.m_pArray, m_arrVertices.m_iCount);
  pGeom->m_pVB->Unmap();
  pGeom->m_uiVertices -= uiExtraVertices;

  pBuf = pGeom->m_pIB->Map();
  memcpy(pBuf, m_arrIndices.m_pArray, m_arrIndices.m_iCount * sizeof(WORD));
  pGeom->m_pIB->Unmap();
  pGeom->m_uiIndices -= uiExtraVertices;

  return pGeom;
}

void CGeometryMerge::TransformVertex(BYTE *pDstVertex, BYTE *pSrcVertex)
{
  int iElem, i;

  for (iElem = 0; iElem < m_pInputDesc->m_Elements.m_iCount; iElem++) {
    CInputDesc::TInputElement &kElem = m_pInputDesc->m_Elements[iElem];
    int iSize = kElem.GetSize();
    if (m_arrElementXForms[iElem]) {
      CVector<4> vRes, vElem = { 0, 0, 0, 1 };
      ASSERT(kElem.m_Type == CInputDesc::T_FLOAT);
      for (i = 0; i < kElem.m_btElements; i++)
        vElem[i] = ((float *) pSrcVertex)[i]; 
      vRes = vElem * (*m_arrElementXForms[iElem]);
      for (i = 0; i < kElem.m_btElements; i++)
        ((float *) pDstVertex)[i] = vRes[i];
    } else
      memcpy(pDstVertex, pSrcVertex, iSize);
    pDstVertex += iSize;
    pSrcVertex += iSize;
  }
}
