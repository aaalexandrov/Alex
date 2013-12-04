#ifndef __MESH_H
#define __MESH_H

#include "Hash.h"

class CD3DBuffer;
class CGeometry;
class CProgressiveGeometry;
class CMesh {
public:
  struct TTriangle;
  class CTriangleArray: public CArray<TTriangle *> {
    DEFREFCOUNT
  };

  struct TVertex {
    CVector<3> m_vCoord;
    int m_iIndex;
    int m_iMaterial;
    bool m_bOnMaterialBorder;
    CSmartPtr<CTriangleArray> m_pTriangles;

    TVertex()  { m_bOnMaterialBorder = false; }

    void AddTriangle(TTriangle *pTri);
    void RemoveTriangle(TTriangle *pTri);
    int GetTriangleIndex(TTriangle *pTri);

    bool CheckMaterialAtEdge(TVertex *pVert0, TVertex *pVert1);
  };

  struct TEdgeRef;
  struct TEdge {
    TVertex *m_pVertices[2];
    TTriangle *m_pTriangles[2];
    float m_fEval[2];
    int m_iHeapInd[2];

    TEdge(TVertex *pVert0, TVertex *pVert1);

    void AddTriangle(TTriangle *pTri);
    void RemoveTriangle(TTriangle *pTri);
    void CheckTrianglesFacing();

    void UpdateHeapInd(bool bSecond, int iInd) { m_iHeapInd[bSecond] = iInd; }

    static inline size_t Hash(TEdge const *pEdge) { return (size_t) pEdge->m_pVertices[0] + (size_t) pEdge->m_pVertices[1]; }
    static inline bool Eq(TEdge const *pEdge0, TEdge const *pEdge1) { return pEdge0->m_pVertices[0] == pEdge1->m_pVertices[0] && pEdge0->m_pVertices[1] == pEdge1->m_pVertices[1]; }
  };

  struct TCollapsedTriangle;
  struct TTriangle {
    TVertex *m_pVertices[3];
    int m_iIndex;

    TTriangle(TVertex *pVert0, TVertex *pVert1, TVertex *pVert2);

    void GetOtherVertices(TVertex *pVert, TVertex *&pOther0, TVertex *&pOther1);
    CVector<3> GetNormal();
    bool CheckSwapInversion(TVertex *pCurrent, TVertex *pNew);
    void RecordCollapse(CArray<TCollapsedTriangle> &arrCollapsedTriangles);

    static inline size_t Hash(TTriangle const *pTri) { return (size_t) pTri->m_pVertices[0] + (size_t) pTri->m_pVertices[1] + (size_t) pTri->m_pVertices[2]; }
    static inline size_t Eq(TTriangle const *pTri0, TTriangle const *pTri1) { return pTri0->m_pVertices[0] == pTri1->m_pVertices[0] && pTri0->m_pVertices[1] == pTri1->m_pVertices[1] && pTri0->m_pVertices[2] == pTri1->m_pVertices[2]; }
  };

  typedef CHash<TVertex *> TVertexHash;
  typedef CHash<TEdge *, TEdge *, TEdge, TEdge> TEdgeHash;
  typedef CHash<TTriangle *, TTriangle *, TTriangle, TTriangle> TTriangleHash;

  struct TCollapse {
    int m_iVertexIndex, m_iCollapseToIndex;
  };

  struct TCollapsedTriangle {
    int m_iIndex;
//    int m_iMaterials[3];
  };

  typedef CHeap<TEdgeRef, TEdgeRef, TEdgeRef> TEdgeRefHeap;

  struct TEdgeRef {
    TEdge *m_pEdge;
    bool m_bSecond;
    TEdgeRefHeap *m_pHeap;

    TEdgeRef() { m_pEdge = 0; m_bSecond = false; m_pHeap = 0;  }
    TEdgeRef(TEdge *pEdge, bool bSecond, TEdgeRefHeap *pHeap) { m_pEdge = pEdge; m_bSecond = bSecond; m_pHeap = pHeap; }
    
    TEdgeRef &operator =(TEdgeRef const &kEdgeRef) { m_pEdge = kEdgeRef.m_pEdge; m_bSecond = kEdgeRef.m_bSecond; ASSERT(!m_pHeap || m_pHeap == kEdgeRef.m_pHeap); m_pHeap = kEdgeRef.m_pHeap; UpdateHeapInd(); return *this; }
    inline void UpdateHeapInd() { if (!m_pHeap) return; int iInd = this - m_pHeap->m_pArray; if (iInd >= 0 && iInd < m_pHeap->m_iCount) m_pEdge->UpdateHeapInd(m_bSecond, iInd); }

    static bool Lt(TEdgeRef const &kEdgeRef0, TEdgeRef const &kEdgeRef1) { return kEdgeRef0.m_pEdge->m_fEval[kEdgeRef0.m_bSecond] < kEdgeRef1.m_pEdge->m_fEval[kEdgeRef1.m_bSecond]; }
  };

  typedef CHeap<TEdgeRef, TEdgeRef, TEdgeRef> TEdgeRefHeap;

public:
  static const int MAX_ELONGATION = 8;
  
  TVertexHash m_hashVertices;
  TTriangleHash m_hashTriangles;
  TEdgeHash m_hashEdges;
  CArray<TCollapse> m_arrCollapses;
  CArray<TCollapsedTriangle> m_arrCollapsedTriangles;
  CArray<int> m_arrVertexMaterials;
  CArray<int> m_arrReverseReorder;
  CSmartPtr<CGeometry> m_pOrgGeom;
  CSmartPtr<CD3DBuffer> m_pVB;
  TEdgeRefHeap m_heapEdgeRefs;
  bool m_bConsiderMaterials;

  CMesh();
  ~CMesh();

  bool Init(CGeometry *pOrgGeom, CStrAny sMatSemantic, uint8_t btMatSemanticIndex);

  CProgressiveGeometry *BuildProgressiveGeometry(int iMaterial, UINT uiNoReorderVertexCount, UINT uiMaxCollapses); // The first uiNoReorderVertexCount will not be reordered and will appear at their original order at the start of the resulting progressive mesh VB
  CD3DBuffer *BuildVB(UINT uiNoReorderVertexCount);
  CD3DBuffer *BuildIB(int iMaterial, CArray<uint8_t> &arrUsedVertices);
  void BuildCollapseBuffer(bool bExplicitVertexCount, UINT uiMaxCollapses, CArray<uint8_t> const &arrUsedVertices, CProgressiveGeometry *pProgGeom);
  void BuildReverseReorder(UINT uiNoReorderVertexCount);
  UINT GetMaxCollapses();

  TVertex *AddVertex(CVector<3> const &vCoord, int iMaterial);
  TTriangle *AddTriangle(TVertex *pVert0, TVertex *pVert1, TVertex *pVert2, int iIndex = -1);
  void RemoveTriangle(TTriangle *pTri);
  void RemoveTriangleEdge(TTriangle *pTri, int iEdge);
  void InitOutputs();
  void InitBorders();
  void InitEdgeEvaluation();
  void DoneInputs();
  void RecordCollapse(TEdge *pEdge, bool bRemoveSecond);
  void RecordRemaining();

  void Simplify(int iVertices2Remain, float fMaxError, bool bConsiderMaterials);

  TEdge *GetEdge(TVertex *pVert0, TVertex *pVert1, bool bCreate = false);
  void EvaluateEdge(TEdge *pEdge, bool bRemoveSecond);
  void CollapseEdge(TEdge *pEdge, bool bRemoveSecond);
  bool SelectEdge(TEdge *&pEdge, bool &bRemoveSecond, float fMaxError);

  bool CheckEdges();
  
  static void GetBaricentricCoordinates(CVector<2> const &vP, CVector<2> const &v0, CVector<2> const &v1, CVector<2> const &v2,
                                        float &fA0, float &fA1, float &fA2);
  static bool IntersectSegments(CVector<2> const &vA0, CVector<2> const &vA1, CVector<2> const &vB0, CVector<2> const &vB1, float &fTA, float &fTB);
};

#endif