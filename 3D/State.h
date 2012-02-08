#ifndef __STATE_H
#define __STATE_H

#include "Var.h"
#include <D3D11.h>

class CStateVarObj: public CVarObj {
  DEFRTTI_NOCREATE
public:
  struct TStr2Int {
    CStrConst sName;
    int iVal;
  };

public:
  CVarObj *m_pVars;

  CStateVarObj()          {}
  virtual ~CStateVarObj() {}

  virtual bool Init();
  virtual void Done();

  virtual CIter *GetIter(const CStrBase &sVar = CStrPart()) const;
	virtual CBaseVar *FindVar(const CStrBase &sVar) const;
  virtual bool ReplaceVar(const CStrBase &sVar, CBaseVar *pSrc, bool bAdding = false);

  virtual bool SetVar(const CStrBase &sVar, const CBaseVar &vSrc);
  virtual bool SetVar(CVarObj::CIter *pIt, const CBaseVar &vSrc);
  virtual bool ApplyVars(CVarObj *pVars, bool bCommit);

  virtual CBaseVar const *TranslateValue(CBaseVar const *pValue);
  virtual TStr2Int *GetDict(int &iCount) = 0;
  virtual void ReleaseState() = 0;
  virtual bool StateReleased() = 0;
  virtual bool Commit(bool bOnlyIfChanged) = 0;

  static inline int GetStrIndex(CStrBase const *pStr, TStr2Int *pDict, int iCount);
  static inline CBaseVar const *TranslateStr2Int(CBaseVar const *pValue, TStr2Int *pDict, int iCount);
  static inline bool EqualVars(CBaseVar const *pVar0, CBaseVar const *pVar1);
};

class CRasterizerState: public CStateVarObj {
  DEFRTTI
public:
  D3D11_RASTERIZER_DESC m_RSDesc;
  ID3D11RasterizerState *m_pRS;

  CRasterizerState()          {}
  virtual ~CRasterizerState() { Done(); }

  virtual bool Init();

  virtual TStr2Int *GetDict(int &iCount);
  virtual void ReleaseState();
  virtual bool StateReleased();
  virtual bool Commit(bool bOnlyIfChanged);

  static inline TStr2Int *GetDict_s(int &iCount);
};

class CBlendState: public CStateVarObj {
  DEFRTTI
public:
  D3D11_BLEND_DESC m_BSDesc;
  ID3D11BlendState *m_pBS;
  CVector<4> m_vBlendFactor;
  UINT m_uiSampleMask;

  CBlendState()          {}
  virtual ~CBlendState() { Done(); }

  virtual bool Init();

  virtual TStr2Int *GetDict(int &iCount);
  virtual void ReleaseState();
  virtual bool StateReleased();
  virtual bool Commit(bool bOnlyIfChanged);

  static inline TStr2Int *GetDict_s(int &iCount);
};

class CDepthStencilState: public CStateVarObj {
  DEFRTTI
public:
  D3D11_DEPTH_STENCIL_DESC m_DSDesc;
  ID3D11DepthStencilState *m_pDS;
  UINT m_uiStencilRef;

  CDepthStencilState()          {}
  virtual ~CDepthStencilState() { Done(); }

  virtual bool Init();

  virtual TStr2Int *GetDict(int &iCount);
  virtual void ReleaseState();
  virtual bool StateReleased();
  virtual bool Commit(bool bOnlyIfChanged);

  static inline TStr2Int *GetDict_s(int &iCount);
};

class CStateCache: public CVarObj {
public:
  enum EStateType {
    ST_RASTERIZER,
    ST_BLEND,
    ST_DEPTHSTENCIL,
  };
public:
  CStateVarObj *m_pStates[3];
  CStateCache  *m_pParent;

  CStateCache();
  ~CStateCache();

  bool Init(bool bRasterizer, bool bBlend, bool bDepthStencil, CStateCache *pParent, CVarObj *pDefStates);
  void Done();

  CStateCache *GetContainer(EStateType eStateType);

  virtual CIter *GetIter(const CStrBase &sVar = CStrPart()) const                     { ASSERT(0); return 0;     }

  virtual CBaseVar *FindVar(const CStrBase &sVar) const                               { ASSERT(0); return 0;     }
  virtual bool ReplaceVar(const CStrBase &sVar, CBaseVar *pSrc, bool bAdding = false) { ASSERT(0); return false; }

  virtual bool GetVar(const CStrBase &sVar, CBaseVar &vDst) const;
  virtual bool SetVar(const CStrBase &sVar, const CBaseVar &vSrc);
};

IMPLEMENT_BASE_SET(CRasterizerState *)
IMPLEMENT_BASE_SET(CBlendState *)
IMPLEMENT_BASE_SET(CDepthStencilState *)

IMPLEMENT_NO_SET(CRasterizerState)
IMPLEMENT_NO_SET(CBlendState)
IMPLEMENT_NO_SET(CDepthStencilState)

#endif