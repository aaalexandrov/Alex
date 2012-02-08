#ifndef __GEOMUTIL_H
#define __GEOMUTIL_H

#include "Model.h"
#include "Transform.h"

class CGeometryMerge {
public:
  class CIndexRemap {
  public:
    virtual WORD RemapIndex(WORD wIndex) { return wIndex; }
  };
public:
  CArray<BYTE> m_arrVertices;
  CArray<WORD> m_arrIndices;
  CSmartPtr<CInputDesc> m_pInputDesc;
  CArray<CMatrix<4, 4> *> m_arrElementXForms;
  bool m_bSkipUnusedVertices;
  CIndexRemap *m_pRemap;

  CGeometryMerge(CInputDesc *pInputDesc, bool bSkipUnusedVertices);
  ~CGeometryMerge();

  // Neither the index remap object nor the transform matrices passed to the functions below are owned by CGeometryMerge. 
  // The caller is responsible for disposing of them when they are no longer needed.
  void SetIndexRemap(CIndexRemap *pRemap);
  void SetVertexElementTransform(int iElement, CMatrix<4, 4> *pXForm);
  void AddGeometry(CGeometry *pGeom, UINT uiVertices = 0, UINT uiIndices = 0);

  CGeometry *CreateGeometry(CRTTI const *pGeomRTTI = 0, UINT uiVBFlags = 0, UINT uiIBFlags = 0, UINT uiExtraVertices = 0, UINT uiExtraIndices = 0, CRTTI const *pBoundRTTI = 0);

  void TransformVertex(BYTE *pDstVertex, BYTE *pSrcVertex);
};

#endif