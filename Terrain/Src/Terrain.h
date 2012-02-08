#ifndef __TERRAIN_H
#define __TERRAIN_H

#include "Matrix.h"
#include "Mesh.h"
#include "Model.h"
#include "Threads.h"

class CFileBase;
class CMaterial;
class CGeometry;
class CAABB;

class CTerrain {
public:
  static const int PATCH_SIZE = 64;
  static const int LOD_PATCH_SIZE = 4;

  struct TPoint {
    float m_fHeight;
    int   m_iMaterial;

    void Set(float fHeight, int iMat) { m_fHeight = fHeight; m_iMaterial = iMat; }
  };

  struct TMaterialData {
    CSmartPtr<CMaterial> m_pMaterial;
    CVector<3, BYTE> m_clrAverage;
  };

  class CPatchModelBuilder;
  class CLODPatch;

  class CPatch {
  public:
    static const int EDGE_INDICES = (PATCH_SIZE - 1) * 4;

    struct TTerrainVertex {
      CVector<3> m_vPos;
      CVector<2> m_vTexCoord;
      float      m_fMaterialID;
    };

    class CEdgedGeom: public CProgressiveGeometry::CChangeCallback {
    public:
      class CEdgeInfo {
      public:
        virtual UINT Vertex2Edge(UINT uiVertexIndex) = 0;
      };

      struct TTriangleSubst {
        UINT m_uiIndex;
        WORD m_wOrgValue[3];

        inline void Init(WORD *pIndices, UINT uiIndex) { m_uiIndex = uiIndex; for (int i = 0; i < ARRSIZE(m_wOrgValue); i++) m_wOrgValue[i] = pIndices[m_uiIndex + i]; }
        inline void Revert(WORD *pIndices) { for (int i = 0; i < ARRSIZE(m_wOrgValue); i++) pIndices[m_uiIndex + i] = m_wOrgValue[i]; }
      };

      typedef CAVLTree<UINT> TTriangleMap;
    public:
      CGeometry *m_pGeometry;
      CEdgeInfo *m_pEdgeInfo;
      CArray<TTriangleSubst> m_arrTriangleSubstitutions;
      CArray<WORD> m_arrOverwrittenIndices;
      TTriangleMap m_mapEdgeTriangles;

      CEdgedGeom(CGeometry *pGeometry, CEdgeInfo *pEdgeInfo);
      ~CEdgedGeom();

      void InitEdgeTriangles();

      void OverwriteTriangle(WORD *pStartInd);
      virtual void IndicesChanged(CProgressiveGeometry *pGeometry, WORD wOldIndex, WORD wCurIndex, WORD *pIndices, UINT uiChanges, UINT *pIndicesOfIndices);

      bool CheckTriangleForEdge(WORD *pTriInd, CBitArray<EDGE_INDICES> const *pActive, UINT &uiEdgeInd0, UINT &uiEdgeInd1, UINT &uiOtherEdgeInd, BYTE &btOtherIndOfs);
      UINT AppendEdgeFan(WORD *pIndices, UINT uiAppendInd, UINT uiEdgeTriInd, CBitArray<EDGE_INDICES> const &kActive, UINT uiEdgeInd0, UINT uiEdgeInd1, UINT uiOtherEdgeInd, WORD wOtherInd);
      void ActivateEdges(WORD *&pIndices, CBitArray<EDGE_INDICES> const &kActive);
      void DeactivateEdges(WORD *&pIndices);

      inline bool IsSplittableEdge(WORD wInd0, WORD wInd1);
      inline UINT FirstEdgeIndexBetween(UINT uiEdgeInd0, UINT uiEdgeInd1, CBitArray<EDGE_INDICES> const *pActive);
    };

    class CPatchGeom;
    class CEdgeMap: public CEdgedGeom::CEdgeInfo {
    public:
      WORD                 m_wEdgeIndices[EDGE_INDICES];
      CHashKV<WORD, WORD>  m_hashReverseEdgeIndices;

      CEdgeMap();

      bool Init(CMesh &kMesh);
      bool Init(WORD *pEdgeIndices);

      bool IsInitialized() { return !!m_hashReverseEdgeIndices.m_iCount; }

      void SetGridIndex(UINT uiIndex, CMesh &kMesh);

      virtual UINT Vertex2Edge(UINT uiVertexIndex);
    };

    class CPatchGeom: public CObject {
      DEFRTTI
      DEFREFCOUNT
    public:
      CPatch *m_pPatch;

      CPatchGeom()          {}
      virtual ~CPatchGeom() {}

      virtual bool Init(CPatch *pPatch) { m_pPatch = pPatch; return true; }
      virtual void Done()               { m_pPatch = 0; }

      virtual void UpdateEdges() = 0;
      virtual bool IsEdgeIndexActive(UINT uiEdgeInd, UINT uiActiveVert = -1) = 0;
      virtual void UpdateLOD(CCamera *pCamera) = 0;

      virtual void SetMinLOD(bool bSetAdjacent) = 0;
      virtual bool IsMinLOD() = 0;
      virtual CModel *GetMinLODModel() = 0;
      virtual void SetModelVar(CStrConst sVar, CBaseVar const &vSrc) = 0;

      virtual bool Render() = 0;

      inline CTerrain *GetTerrain() { return m_pPatch->m_pTerrain; }
      bool DetermineActiveEdgeIndices(CBitArray<EDGE_INDICES> &kActive, bool bAdjacentMinDetail);
      void UpdateAdjacentEdges();
    };

    class CFullGeom: public CPatchGeom {
      DEFRTTI
    public:
      struct TMatModel {
        CSmartPtr<CModel> m_pModel;
        int m_iMaterial;

        TMatModel(CModel *pModel, int iMaterial): m_pModel(pModel), m_iMaterial(iMaterial) {}
      };
    public:
      CSmartPtr<CModel>   m_pLODModel;
      CEdgedGeom *m_pLODEdgedGeom;
      CArray<TMatModel *> m_arrMaterialModels;
      bool m_bUseLODModel, m_bBordersUpdated;

      CFullGeom();
      virtual ~CFullGeom();

      virtual bool Init(CPatch *pPatch, bool bInitMaterialModels);
      virtual void Done();

      virtual bool InitMaterialModels(UINT uiReservedVertices);
      virtual bool InitMaterialModel(int iMaterial, UINT uiReservedVertices);
      virtual bool InitMinLODModel(CModel *pLODModel);
      virtual void DoneMaterialModels();

      virtual void UpdateEdges();
      virtual bool IsEdgeIndexActive(UINT uiEdgeInd, UINT uiActiveVert = -1);
      virtual void UpdateLOD(CCamera *pCamera);

      virtual void SetMinLOD(bool bSetAdjacent);
      virtual bool IsMinLOD();
      virtual CModel *GetMinLODModel() { ASSERT(m_pPatch->m_pMinLODModel == m_pLODModel); return m_pLODModel; }
      virtual void SetModelVar(CStrConst sVar, CBaseVar const &vSrc);

      virtual bool Render();

      void UpdateBorders(bool bAdjacentMinDetail);
    };

    class CProgGeom: public CPatchGeom {
      DEFRTTI
    public:
      struct TMaterialModel {
        CSmartPtr<CModel> m_pModel;
        int m_iMaterial;
        CEdgedGeom *m_pEdgedGeom;

        TMaterialModel(CModel *pModel, int iMaterial, CProgGeom *pProgGeom);
        ~TMaterialModel();
      };
    public:
      TMaterialModel           *m_pLODMaterialModel;
      CArray<TMaterialModel *>  m_arrMaterialModels;
      UINT                      m_uiActiveVertices, m_uiMinActiveVertices, m_uiMinVerticesToSet, m_uiMinLODVertices, m_uiMaxVertices;
      bool                      m_bBordersUpdated, m_bUseLODModel;

      CProgGeom();
      virtual ~CProgGeom();

      virtual bool Init(CPatch *pPatch, CPatchModelBuilder *pBuilder);
      virtual void Done();

      virtual void UpdateEdges();
      virtual bool IsEdgeIndexActive(UINT uiEdgeInd, UINT uiActiveVert = -1);
      virtual void UpdateLOD(CCamera *pCamera);

      virtual void SetMinLOD(bool bSetAdjacent);
      virtual bool IsMinLOD();
      virtual CModel *GetMinLODModel() { return m_pLODMaterialModel->m_pModel; }
      virtual void SetModelVar(CStrConst sVar, CBaseVar const &vSrc);

      virtual void SetActiveVertices(int iVertices);

      virtual bool Render();

      bool InitProgressiveMaterials(CPatchModelBuilder *pBuilder);
      bool InitProgressiveMaterial(int iMaterial, CProgressiveGeometry *pProgGeom);

      void DoneModels();

      inline UINT GetMinActiveVertices() { return m_bUseLODModel ? m_uiMinLODVertices : m_uiMinActiveVertices; }
      inline TMaterialModel *GetMaterialModel(int iMaterialInd);

      int GetValidVertexCount(int iVertices);
      void SetGeomVertices(int iVertices, int iMaterialInd);
      void UpdateBorders(bool bAdjacentMinDetail);

      bool RenderMaterialModel(int iMaterialInd, bool bExtraActive, CBitArray<EDGE_INDICES> const &kActive);
    };

  public:
    CTerrain                 *m_pTerrain;
    CVector<2, int>           m_vPatchIndex; // in number of patches
    CVector<2, int>           m_vGridOrigin;
    CVector<2>                m_vWorldOrigin;
    TPoint                    m_Points[PATCH_SIZE][PATCH_SIZE];
    float                     m_fMinHeight, m_fMaxHeight;
    CSmartPtr<CTexture>       m_pTexNormals, m_pTexFar;
    CSmartPtr<CModel>         m_pModel, m_pMinLODModel;
    CSmartPtr<CPatchGeom>     m_pGeom;

    bool                      m_bBuilding;
    CArray<WORD>              m_arrCollapses, m_arrCollapsedTriangles;
    UINT                      m_uiMaterialCollapses, m_uiRealCollapses;
    CEdgeMap                  m_EdgeMap;

    CPatch(int iX, int iY, CTerrain *pTerrain);
    ~CPatch();

    inline bool PointInside(int iXGrid, int iYGrid) 
    { 
      return iXGrid >= m_vGridOrigin.x() && iXGrid < m_vGridOrigin.x() + PATCH_SIZE && 
             iYGrid >= m_vGridOrigin.y() && iYGrid < m_vGridOrigin.y() + PATCH_SIZE;
    }

    inline bool PointInside(float fX, float fY)
    {
      return fX >= m_vWorldOrigin.x() && fX <= m_vWorldOrigin.x() + (PATCH_SIZE - 1) * m_pTerrain->m_fGrid2World &&
             fY >= m_vWorldOrigin.y() && fY <= m_vWorldOrigin.y() + (PATCH_SIZE - 1) * m_pTerrain->m_fGrid2World;
    }

    inline TPoint &GetPoint(int iXGrid, int iYGrid) 
    { 
      ASSERT(PointInside(iXGrid, iYGrid)); 
      return m_Points[iYGrid - m_vGridOrigin.y()][iXGrid - m_vGridOrigin.x()]; 
    }

    float GetHeight(float fX, float fY);
    void GetGridNormal(int iX, int iY, CVector<3> &vNormal);

    CAABB GetAABB(bool bLocal);
    void GetUsedMaterials(CAVLTree<int> &kUsedMats);

    bool Init();

    bool Save(CFileBase *pFile);
    bool Load(CFileBase *pFile);
    bool SaveMeshData(CFileBase *pFile);
    bool LoadMeshData(CFileBase *pFile);
    bool SaveMinLODMesh(CFileBase *pFile);
    bool LoadMinLODMesh(CFileBase *pFile);
    CStr GetSaveFileName();
    bool HasMeshData() { return !!m_arrCollapses.m_iCount; }

    bool InitModel(UINT uiReservedVertices);
    bool InitIB(CGeometry *pGeom, int iMaterial, UINT uiReservedVertices);
    bool CheckMinimalGeom();
    void SetGeom(CPatchGeom *pGeom);
    bool SetFullGeom(bool bInitMaterialModels);
    bool SetProgGeom(CPatchModelBuilder *pBuilder);

    bool InitEdgeVertices(TTerrainVertex *pVertices);
    static UINT Grid2EdgeIndex(int iX, int iY);
    static CVector<2, int> EdgeIndex2Grid(UINT uiIndex);
    static inline bool EdgeIndicesAdjacent(UINT uiEdgeInd0, UINT uiEdgeInd1);
    static inline bool EdgeIndicesOnSameSide(UINT uiEdgeInd0, UINT uiEdgeInd1);
    static inline bool EdgeIndexOnCorner(UINT uiEdgeInd);

    bool InitMinMaxHeight();
    bool InitParams(CModel *pModel, int iMaterial, int iMaterialInd);
    bool InitNormalsTexture();
    bool InitFarTexture();

    CRect<int> GetTexRectPixel();
    void SetTexTransform(CMatrix<3, 2> &mXForm);
    inline CLODPatch *GetLODPatch();
    inline float GetMinLODDistance() { if (!m_pMinLODModel) return Util::F_INFINITY; return Vertices2Dist(m_pMinLODModel->m_pGeometry->m_uiVertices); }

    bool IsProgressiveGeom() { return m_pGeom->GetRTTI()->IsKindOf(&CProgGeom::s_RTTI); }
    void GetAdjacentPatches(CPatch *&pLeft, CPatch *&pUp, CPatch *&pRight, CPatch *&pDown);

    void RecordMeshData(CMesh &kMesh, UINT uiMaterialCollapses);
    void RestoreMeshFromData(CMesh &kMesh, CGeometry *pOrgGeom);

    void SetMaterialModels(CPatchModelBuilder *pBuilder);

    void UpdateLOD(CCamera *pCamera);

    bool Render();
  };

  class CLODPatch {
  public:
    CTerrain *m_pTerrain;
    CPatch *m_pPatches[LOD_PATCH_SIZE][LOD_PATCH_SIZE];
    CSmartPtr<CModel> m_pModel;
    CSmartPtr<CTexture> m_pTexNormals, m_pTexFar;
    float m_fMinHeight, m_fMaxHeight;
    bool m_bActive;
    float m_fNoUpdateDistance;

    CLODPatch(CTerrain *pTerrain, int iXPatch, int iYPatch);
    ~CLODPatch();

    bool Init();
    bool InitMinMaxHeight();
    bool InitTextures();
    bool InitTextureContent();
    bool InitPatches(int iXPatch, int iYPatch);
    bool InitDistance();
    bool AddPatchDistance(CPatch *pPatch);

    bool InitModel();
    bool CanInitModel();

    CAABB GetAABB(bool bLocal);

    bool CanActivate();
    void LowerPatchGeoms();

    void UpdateLOD(CCamera *pCamera);

    bool Render();
  };

public:
  CRect<int>            m_rcGrid;
  CRect<>               m_rcWorld;
  float                 m_fGrid2World;
  CVector<2, int>       m_vPatchDim;
  CArray<TMaterialData> m_arrMaterials;
  CArray<CPatch *>      m_arrPatches;
  CVector<2, int>       m_vLODPatchDim;
  CArray<CLODPatch *>   m_arrLODPatches;
  float                 m_fMaxLODError, m_fLODDistance;
  CSmartPtr<CMaterial>  m_pLODMaterial;
  int                   m_iForceModel;
  CStr                  m_sSaveFileRoot;
  CCamera              *m_pCamera;

  CTerrain(int iGridWidth, int iGridHeight, float fGrid2World, CMaterial *pBaseMat, CStr sSaveFileRoot);
  ~CTerrain();

  CPatch *GetPatchByIndex(int iXPatch, int iYPatch);
  CPatch *GetPatch(int iXGrid, int iYGrid);
  CPatch *GetPatch(float fX, float fY);

  CLODPatch *GetLODPatchByIndex(int iXLOD, int iYLOD);
  CLODPatch *GetLODPatchByPatchIndex(int iXPatch, int iYPatch);

  const TPoint *GetPoint(int iXGrid, int iYGrid);
  void SetPoint(int iX, int iY, float fHeight, int iMaterial);

  float GetHeight(float fX, float fY);

  int AddMaterial(CMaterial *pMaterial);
  CMaterial *GetMaterial(int iMaterial);
  bool InitPatches();
  void DonePatches();
  void SetCamera(CCamera *pCamera);

  bool Save(CFileBase *pFile);
  bool Load(CFileBase *pFile);
  CStr GetSaveFileName();

  void PatchInitDone(CPatch *pPatch);
  void UpdateLOD(CCamera *pCamera = 0);

  bool Render();

  bool InitLODMaterial(CMaterial *pSrcMat);
  static inline void SetNormTransform(CMatrix<3, 2> &mNormXForm);
  static inline float GetMaxResDistance() { return (float) PATCH_SIZE; }
  static inline int Dist2Vertices(float fDistance);
  static inline float Vertices2Dist(int iVertices);

public:
  class CPatchModelBuilder {
  public:
    CPatch *m_pPatch;
    CSmartPtr<CProgressiveGeometry> m_pLODGeometry;
    CArray<CSmartPtr<CProgressiveGeometry> > m_arrMaterialGeometries;
    CSmartPtr<CGeometry> m_pMinLODGeometry;
    CMesh *m_pMesh;
    UINT m_uiMaterialCollapses;
    CAVLTree<int> m_kUsedMaterials;
    bool m_bBuildMinLOD;
    CSmartPtr<CModel> m_pOrgModel;

    CPatchModelBuilder(CPatch *pPatch);
    ~CPatchModelBuilder();

    void Init(CPatch *pPatch);
    void Done();

    bool InitMesh();
    
    void BuildModels();

    static CGeometry *BuildMinLODGeometry(CProgressiveGeometry *pProgGeom);
  };

  class CMeshBuildThread: public CThread {
    DEFRTTI
  public:
    CTerrain *m_pTerrain;

    CMeshBuildThread(CTerrain *pTerrain);
    virtual ~CMeshBuildThread();

    virtual bool Start();

    virtual UINT Run(void *pParam);
  };

  CLock m_Lock;
  CList<CPatchModelBuilder *> m_lstBuildRequests;
  CList<CPatchModelBuilder *> m_lstBuildReady;
  CMeshBuildThread *m_pBuilder;
  volatile bool m_bBuildingActive;
};

// Implementation -------------------------------------------------------------

bool CTerrain::CPatch::EdgeIndicesAdjacent(UINT uiEdgeInd0, UINT uiEdgeInd1)
{
  ASSERT(uiEdgeInd0 != uiEdgeInd1);
  if (uiEdgeInd0 > uiEdgeInd1)
    Util::Swap(uiEdgeInd0, uiEdgeInd1);
  UINT uiDiff = uiEdgeInd1 - uiEdgeInd0;
  return uiDiff == 1 || uiDiff == EDGE_INDICES - 1;
}

bool CTerrain::CPatch::EdgeIndicesOnSameSide(UINT uiEdgeInd0, UINT uiEdgeInd1)
{
  int iSide0, iSide1;
  if (uiEdgeInd0 > uiEdgeInd1)
    Util::Swap(uiEdgeInd0, uiEdgeInd1);
  iSide0 = uiEdgeInd0 / (PATCH_SIZE - 1);
  iSide1 = uiEdgeInd1 / (PATCH_SIZE - 1);
  if (iSide0 == iSide1)
    return true;
  if (uiEdgeInd1 == (iSide0 + 1) * (PATCH_SIZE - 1) || !uiEdgeInd0 && iSide1 == 3)
    return true;
  return false;
}

bool CTerrain::CPatch::EdgeIndexOnCorner(UINT uiEdgeInd)
{
  return !(uiEdgeInd % (PATCH_SIZE - 1));
}

CTerrain::CLODPatch *CTerrain::CPatch::GetLODPatch() 
{ 
  return m_pTerrain->GetLODPatchByPatchIndex(m_vPatchIndex.x(), m_vPatchIndex.y()); 
}

void CTerrain::SetNormTransform(CMatrix<3, 2> &mNormXForm)
{
  mNormXForm(0, 0) = mNormXForm(1, 1) = (1 - 1.0f / PATCH_SIZE) / (PATCH_SIZE - 1);
  mNormXForm(0, 1) = mNormXForm(1, 0) = 0;
  mNormXForm(2, 0) = mNormXForm(2, 1) = 0.5f / PATCH_SIZE;
}

int CTerrain::Dist2Vertices(float fDistance)
{
  return (int) Util::Sqr(PATCH_SIZE * GetMaxResDistance() / fDistance) + CPatch::EDGE_INDICES;
}

float CTerrain::Vertices2Dist(int iVertices)
{
  return PATCH_SIZE * GetMaxResDistance() / sqrt((float) (iVertices - CPatch::EDGE_INDICES));
}

#endif