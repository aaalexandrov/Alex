#ifndef __MODEL_H
#define __MODEL_H

#include "Graphics.h"
#include "Buffer.h"
#include "Shader.h"
#include "VarUtil.h"

class CShape3D;

class CMaterial: public CVarObj {
  DEFRTTI
  DEFREFCOUNT
public:
  CTechnique *m_pTechnique;
  CVarObj    *m_pParams;
  CVarMerge   m_vApplyVars;
  CConstantCache *m_pPerMaterialCache;
  CStateCache m_StateCache;
  bool        m_bUseGlobals;

  CMaterial();
  virtual ~CMaterial();

  virtual bool Init(CTechnique *pTech, bool bUseGlobals, CVarObj *pInitParams, bool bOwnParams = false);
  virtual bool InitConstantCache();
  virtual bool InitStates();
  virtual void Done();

  virtual bool IsValid();

  virtual bool SetMatrixVar(CStrConst sVarMatrix, CMatrixVar const *pMatVar, CVarObj *pModelParams = 0);
  virtual void SetVarUpdated();
  virtual CVarObj *GetApplyVars(CVarObj *pModelParams);
  virtual bool Apply(CVarObj *pModelParams, CStateCache *pStateCache);

  virtual CIter *GetIter(const CStrBase &sVar = CStrPart()) const                     { ASSERT(0); return 0;     }

  virtual CBaseVar *FindVar(const CStrBase &sVar) const                               { ASSERT(0); return 0;     }
  virtual bool ReplaceVar(const CStrBase &sVar, CBaseVar *pSrc, bool bAdding = false) { ASSERT(0); return false; }

  virtual bool GetVar(const CStrBase &sVar, CBaseVar &vDst) const { return m_pParams->GetVar(sVar, vDst); }
  virtual bool SetVar(const CStrBase &sVar, const CBaseVar &vSrc);
};

class CGeometry: public CObject {
  DEFRTTI
  DEFREFCOUNT
public:
  enum EPrimitiveType {
    PT_NONE,
    PT_POINTLIST,
    PT_LINELIST,
    PT_LINESTRIP,
    PT_TRIANGLELIST,
    PT_TRIANGLESTRIP,
  };
public:
  CSmartPtr<CInputDesc>     m_pInputDesc;
  CSmartPtr<CD3DBuffer>     m_pIB, m_pVB;
  EPrimitiveType            m_ePrimitiveType;
  UINT                      m_uiVertices, m_uiIndices;
  CShape3D                 *m_pBound;

  CGeometry();
  virtual ~CGeometry();

  virtual bool Init(CInputDesc *pInputDesc, EPrimitiveType ePrimitiveType, 
                    UINT uiVertices, UINT uiIndices, void *pVertices = 0, WORD *pIndices = 0, 
                    UINT uiVBFlags = 0, UINT uiIBFlags = 0, CRTTI const *pBoundRTTI = 0);
  virtual void Done();

  virtual bool IsValid();

  virtual void SetInputDesc(CInputDesc *pInputDesc);
  virtual void SetVertices(UINT uiVertices, void *pVertices = 0, UINT uiFlags = 0);
  virtual void SetIndices(UINT uiIndices, WORD *pIndices = 0, UINT uiFlags = 0);
  virtual void SetBoundType(CRTTI const *pBoundRTTI, void *pVertices = 0, WORD *pIndices = 0);
  virtual UINT GetVBVertexCount();
  virtual UINT GetIBIndexCount();

  virtual bool Apply();
  virtual bool Render(UINT uiIndices = -1, UINT uiStartIndex = 0, UINT uiBaseVertex = 0);

  static inline bool IsTriangleDegenerate(WORD *pIndices) { return pIndices[0] == pIndices[1] || pIndices[0] == pIndices[2] || pIndices[1] == pIndices[2]; }
  static inline D3D11_PRIMITIVE_TOPOLOGY GetD3DPrimitiveTopology(EPrimitiveType ePT);
};

class CProgressiveGeometry: public CGeometry {
  DEFRTTI
public:
  static const UINT INVALID_INDEX = (UINT) -1;
  enum EFlags {
    PGF_EXPLICIT_VERTEX_COUNT = 1,
  };

  class CChangeCallback {
  public:
    // This callback is called after the changes have been made
    virtual void IndicesChanged(CProgressiveGeometry *pGeometry, WORD wOldIndex, WORD wCurIndex, WORD *pIndices, UINT uiChanges, UINT *pIndicesOfIndices) = 0;
  };
public:
  // Individual collapse data is a sequence of: 
  //   index of the vertex being collapsed, 
  //   index of the vertex it's being collapsed to, 
  //   index of the last valid vertex after the collapse (in case PGF_EXPLICIT_VERTEX_COUNT flag is specified, otherwise this field is omitted)
  //   indices of indices that that need to be changed from the collapsed to the replacement value, 
  //   terminating INVALID_INDEX
  // Collapses are a sequence of individual collapse data, with the first one corresponding to the last vertex in the vertex buffer
  UINT *m_pCollapses; 
  UINT  m_uiCollapses, m_uiCurCollapse;
  UINT  m_uiMinVertices;
  UINT  m_uiFlags;
  CChangeCallback *m_pChangeCallback;

  CProgressiveGeometry();
  virtual ~CProgressiveGeometry();

  virtual void Done();
  void DoneProgressive();

  virtual void SetCollapses(UINT uiCollapses, UINT *pCollapses, bool bExplicitVertexCount);
  virtual bool SetActiveVertices(UINT uiVertices, WORD **pMappedIndices = 0);

  UINT CalcMinVertices();
  inline UINT GetPrevCollapse(UINT uiCollapse);
  inline UINT GetPrevCollapseVertices(UINT uiCollapse);
  inline UINT GetNextCollapse(UINT uiCollapse);
  bool CollapseVertex(WORD *&pIndices);
  bool UncollapseVertex(UINT uiVertThreshold, WORD *&pIndices);
};

class CModel: public CVarObj {
  DEFRTTI
  DEFREFCOUNT
public:
  CSmartPtr<CGeometry>  m_pGeometry;
  CSmartPtr<CMaterial>  m_pMaterial;
  ID3D11InputLayout    *m_pLayout;
  CShape3D             *m_pBound;
  CVarObj              *m_pParams;
  CConstantCache       *m_pPerObjectCache;
  CStateCache           m_StateCache;

  CModel();
  virtual ~CModel();

  virtual bool Init(CGeometry *pGeom, CMaterial *pMaterial, CVarObj *pInitParams, bool bOwnParams, CRTTI const *pBoundRTTI = 0);
  virtual bool InitConstantCache();
  virtual bool InitStates();
  virtual void Done();

  virtual bool IsValid();

  virtual bool SetMatrixVar(CStrConst sVarMatrix, CMatrixVar const *pMatVar);
  virtual void SetVarUpdated(bool bMaterialUpdated);
  virtual CVarObj *GetApplyVars() { return m_pMaterial->GetApplyVars(m_pParams); }
  virtual bool UpdateBound();
  virtual bool Apply();
  virtual bool DoRender();
  virtual bool Render();

  virtual CIter *GetIter(const CStrBase &sVar = CStrPart()) const                     { ASSERT(0); return 0;     }

  virtual CBaseVar *FindVar(const CStrBase &sVar) const                               { ASSERT(0); return 0;     }
  virtual bool ReplaceVar(const CStrBase &sVar, CBaseVar *pSrc, bool bAdding = false) { ASSERT(0); return false; }

  virtual bool GetVar(const CStrBase &sVar, CBaseVar &vDst) const { return m_pParams->GetVar(sVar, vDst); }
  virtual bool SetVar(const CStrBase &sVar, const CBaseVar &vSrc);
};

#endif