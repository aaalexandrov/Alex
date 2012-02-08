#include "stdafx.h"
#include "State.h"
#include "Graphics.h"

// CStateVarObj ---------------------------------------------------------------
IMPRTTI_NOCREATE(CStateVarObj, CVarObj)

bool CStateVarObj::Init()
{
  m_pVars = new CVarHash();
  return true;
}

void CStateVarObj::Done()
{
  ReleaseState();
  SAFE_DELETE(m_pVars);
}

CVarObj::CIter *CStateVarObj::GetIter(const CStrBase &sVar) const
{
  return m_pVars->GetIter(sVar);
}

CBaseVar *CStateVarObj::FindVar(const CStrBase &sVar) const
{
  return m_pVars->FindVar(sVar);
}

bool CStateVarObj::ReplaceVar(const CStrBase &sVar, CBaseVar *pSrc, bool bAdding)
{
  ASSERT(0);
  return false;
}

bool CStateVarObj::SetVar(const CStrBase &sVar, const CBaseVar &vSrc)
{
  CAutoDeletePtr<CVarObj::CIter> pIt(m_pVars->GetIter(sVar));
  if (!pIt)
    return false;

  return SetVar(pIt, vSrc);
}

bool CStateVarObj::SetVar(CVarObj::CIter *pIt, const CBaseVar &vSrc)
{
  CBaseVar const *pVal;
  bool bRes;
  CAutoReleasePtr<CBaseVar const> pNewVal;

  pVal = TranslateValue(&vSrc);
  if (pVal != &vSrc)
    pNewVal.m_pPtr = pVal;

  if (EqualVars(pVal, pIt->GetValue()))
    return true;

  bRes = pIt->GetValue()->SetVar(*pVal);

  if (bRes)
    ReleaseState();

  return bRes;
}

bool CStateVarObj::ApplyVars(CVarObj *pVars, bool bCommit)
{
  CAutoDeletePtr<CVarObj::CIter> pIt(m_pVars->GetIter());
  bool bRes = true;

  while (*pIt) {
    CBaseVar *pSrcVar = pVars->FindVar(pIt->GetName());
    if (pSrcVar) 
      bRes &= SetVar(pIt, *pSrcVar);
    pIt->Next();
  }
  if (bCommit && bRes)
    bRes = Commit(true);
  return bRes;
}

CBaseVar const *CStateVarObj::TranslateValue(CBaseVar const *pValue)
{
  TStr2Int *pDict;
  int iCount;
  pDict = GetDict(iCount);
  return TranslateStr2Int(pValue, pDict, iCount);
}

int CStateVarObj::GetStrIndex(CStrBase const *pStr, TStr2Int *pDict, int iCount)
{
  int i;
  for (i = 0; i < iCount; i++)
    if (*pStr == pDict[i].sName)
      return i;
  return -1;
}

inline CBaseVar const *CStateVarObj::TranslateStr2Int(CBaseVar const *pValue, TStr2Int *pDict, int iCount)
{
  int i;
  CVarValueBase<CStr> const *pStrVal = Cast<CVarValueBase<CStr> >(pValue);
  if (pStrVal)
    for (i = 0; i < iCount; i++)
      if (pStrVal->GetValue() == pDict[i].sName)
        return new CVar<int>(pDict[i].iVal);
  return pValue;
}

inline bool CStateVarObj::EqualVars(CBaseVar const *pVar0, CBaseVar const *pVar1)
{
  ASSERT(pVar0 && pVar1);
  if (pVar0->GetRTTI()->IsKindOf(&CVarValueBase<int>::s_RTTI) && pVar1->GetRTTI()->IsKindOf(&CVarValueBase<int>::s_RTTI)) 
    return ((CVarValueBase<int> *) pVar0)->GetValue() == ((CVarValueBase<int> *) pVar1)->GetValue();

  if (pVar0->GetRTTI()->IsKindOf(&CVarValueBase<float>::s_RTTI) && pVar1->GetRTTI()->IsKindOf(&CVarValueBase<float>::s_RTTI)) 
    return ((CVarValueBase<float> *) pVar0)->GetValue() == ((CVarValueBase<float> *) pVar1)->GetValue();

  if (pVar0->GetRTTI()->IsKindOf(&CVarValueBase<BYTE>::s_RTTI) && pVar1->GetRTTI()->IsKindOf(&CVarValueBase<BYTE>::s_RTTI)) 
    return ((CVarValueBase<BYTE> *) pVar0)->GetValue() == ((CVarValueBase<BYTE> *) pVar1)->GetValue();

  if (pVar0->GetRTTI()->IsKindOf(&CVarValueBase<CVector<4> >::s_RTTI) && pVar1->GetRTTI()->IsKindOf(&CVarValueBase<CVector<4> >::s_RTTI)) 
    return ((CVarValueBase<CVector<4> > *) pVar0)->GetValue() == ((CVarValueBase<CVector<4> > *) pVar1)->GetValue();

  return false;
}


// CRasterizerState -----------------------------------------------------------
IMPRTTI(CRasterizerState, CStateVarObj)
IMP_VAR_RTTI(CRasterizerState *)
IMP_VAR_RTTI(CRasterizerState)

bool CRasterizerState::Init()
{
  static CStrConst sFillMode("FillMode");
  static CStrConst sCullMode("CullMode");
  static CStrConst sFrontCCW("FrontCounterClockwise");
  static CStrConst sDepthBias("DepthBias");
  static CStrConst sSlopeScaledDepthBias("SlopeScaledDepthBias");
  static CStrConst sDepthBiasClamp("DepthBiasClamp");
  static CStrConst sDepthClipEnable("DepthClipEnable");
  static CStrConst sScissorEnable("ScissorEnable");
  static CStrConst sMultisampleEnable("MultisampleEnable");
  static CStrConst sAntialiasedLineEnable("AntialiasedLineEnable");

  if (!CStateVarObj::Init())
    return false;

  m_RSDesc.FillMode = D3D11_FILL_SOLID;
  m_RSDesc.CullMode = D3D11_CULL_BACK;
  m_RSDesc.FrontCounterClockwise = false;
  m_RSDesc.DepthBias = 0;
  m_RSDesc.SlopeScaledDepthBias = 0;
  m_RSDesc.DepthBiasClamp = 0;
  m_RSDesc.DepthClipEnable = true;
  m_RSDesc.ScissorEnable = false;
  m_RSDesc.MultisampleEnable = false;
  m_RSDesc.AntialiasedLineEnable = false;

  CBaseVar *pVar;

  COMPILE_ASSERT(sizeof(m_RSDesc.FillMode) == sizeof(int)); 
  pVar = new CVarRef<int>(*(int *) &m_RSDesc.FillMode);
  m_pVars->ReplaceVar(sFillMode, pVar, true);

  COMPILE_ASSERT(sizeof(m_RSDesc.CullMode) == sizeof(int));
  pVar = new CVarRef<int>(*(int *) &m_RSDesc.CullMode);
  m_pVars->ReplaceVar(sCullMode, pVar, true);

  pVar = new CVarRef<int>(m_RSDesc.FrontCounterClockwise);
  m_pVars->ReplaceVar(sFrontCCW, pVar, true);

  pVar = new CVarRef<int>(m_RSDesc.DepthBias);
  m_pVars->ReplaceVar(sDepthBias, pVar, true);

  pVar = new CVarRef<float>(m_RSDesc.DepthBiasClamp);
  m_pVars->ReplaceVar(sDepthBiasClamp, pVar, true);

  pVar = new CVarRef<float>(m_RSDesc.SlopeScaledDepthBias);
  m_pVars->ReplaceVar(sSlopeScaledDepthBias, pVar, true);

  pVar = new CVarRef<int>(m_RSDesc.DepthClipEnable);
  m_pVars->ReplaceVar(sDepthClipEnable, pVar, true);

  pVar = new CVarRef<int>(m_RSDesc.ScissorEnable);
  m_pVars->ReplaceVar(sScissorEnable, pVar, true);

  pVar = new CVarRef<int>(m_RSDesc.MultisampleEnable);
  m_pVars->ReplaceVar(sMultisampleEnable, pVar, true);

  pVar = new CVarRef<int>(m_RSDesc.AntialiasedLineEnable);
  m_pVars->ReplaceVar(sAntialiasedLineEnable, pVar, true);

  m_pRS = 0;

  return true;
}

CStateVarObj::TStr2Int *CRasterizerState::GetDict_s(int &iCount)
{
  static CStrConst sSolid("Solid");
  static CStrConst sWireframe("Wireframe");
  static CStrConst sFront("Front");
  static CStrConst sBack("Back");
  static CStrConst sNone("None");

  static TStr2Int arrStr2Rast[] = {
    { sSolid, D3D11_FILL_SOLID },
    { sWireframe, D3D11_FILL_WIREFRAME },
    { sFront, D3D11_CULL_FRONT },
    { sBack, D3D11_CULL_BACK },
    { sNone, D3D11_CULL_NONE },
  };

  iCount = ARRSIZE(arrStr2Rast);
  return arrStr2Rast;
}

CStateVarObj::TStr2Int *CRasterizerState::GetDict(int &iCount) 
{ 
  return GetDict_s(iCount); 
}

void CRasterizerState::ReleaseState()
{
  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  SAFE_RELEASE(m_pRS);
}

bool CRasterizerState::StateReleased()
{
  return !m_pRS;
}

bool CRasterizerState::Commit(bool bOnlyIfChanged)
{
  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  if (!m_pRS) {
    HRESULT res;
    TResReporter kErrorReport(res);

    res = CGraphics::Get()->m_pDevice->CreateRasterizerState(&m_RSDesc, &m_pRS);
    if (FAILED(res))
      return false;
  } else
    if (bOnlyIfChanged)
      return true;
  CGraphics::Get()->m_pDeviceContext->RSSetState(m_pRS);
  return true;
}

// CBlendState ----------------------------------------------------------------
IMPRTTI(CBlendState, CStateVarObj)
IMP_VAR_RTTI(CBlendState *)
IMP_VAR_RTTI(CBlendState)

bool CBlendState::Init()
{
  static CStrConst sAlphaToCoverageEnable("AlphaToCoverageEnable");
  static CStrConst sIndependentBlendEnable("IndependentBlendEnable");
  static CStrConst sBlendEnable("BlendEnable");
  static CStrConst sSrcBlend("SrcBlend");
  static CStrConst sDestBlend("DestBlend");
  static CStrConst sBlendOp("BlendOp");
  static CStrConst sSrcBlendAlpha("SrcBlendAlpha");
  static CStrConst sDestBlendAlpha("DestBlendAlpha");
  static CStrConst sBlendOpAlpha("BlendOpAlpha");
  static CStrConst sRenderTargetWriteMask("RenderTargetWriteMask");
  static CStrConst sBlendFactor("BlendFactor");
  static CStrConst sSampleMask("SampleMask");

  if (!CStateVarObj::Init())
    return false;

  int i;
  CBaseVar *pVar;

  m_BSDesc.AlphaToCoverageEnable = false;
  m_BSDesc.IndependentBlendEnable = false;
  for (i = 0; i < ARRSIZE(m_BSDesc.RenderTarget); i++) {
    m_BSDesc.RenderTarget[i].BlendEnable = false;
    m_BSDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
    m_BSDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
    m_BSDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
    m_BSDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
    m_BSDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
    m_BSDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    m_BSDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  }

  pVar = new CVarRef<int>(m_BSDesc.AlphaToCoverageEnable);
  m_pVars->ReplaceVar(sAlphaToCoverageEnable, pVar, true);

  pVar = new CVarRef<int>(m_BSDesc.IndependentBlendEnable);
  m_pVars->ReplaceVar(sIndependentBlendEnable, pVar, true);

  for (i = 0; i < ARRSIZE(m_BSDesc.RenderTarget); i++) {
    pVar = new CVarRef<int>(m_BSDesc.RenderTarget[i].BlendEnable);
    m_pVars->ReplaceVar(sBlendEnable + CStr(i), pVar, true);

    COMPILE_ASSERT(sizeof(D3D11_BLEND) == sizeof(int));
    pVar = new CVarRef<int>(*(int *) &m_BSDesc.RenderTarget[i].SrcBlend);
    m_pVars->ReplaceVar(sSrcBlend + CStr(i), pVar, true);

    pVar = new CVarRef<int>(*(int *) &m_BSDesc.RenderTarget[i].DestBlend);
    m_pVars->ReplaceVar(sDestBlend + CStr(i), pVar, true);

    COMPILE_ASSERT(sizeof(D3D11_BLEND_OP) == sizeof(int));
    pVar = new CVarRef<int>(*(int *) &m_BSDesc.RenderTarget[i].BlendOp);
    m_pVars->ReplaceVar(sBlendOp + CStr(i), pVar, true);

    pVar = new CVarRef<int>(*(int *) &m_BSDesc.RenderTarget[i].SrcBlendAlpha);
    m_pVars->ReplaceVar(sSrcBlendAlpha + CStr(i), pVar, true);

    pVar = new CVarRef<int>(*(int *) &m_BSDesc.RenderTarget[i].DestBlendAlpha);
    m_pVars->ReplaceVar(sDestBlendAlpha + CStr(i), pVar, true);

    pVar = new CVarRef<int>(*(int *) &m_BSDesc.RenderTarget[i].BlendOpAlpha);
    m_pVars->ReplaceVar(sBlendOpAlpha + CStr(i), pVar, true);

    pVar = new CVarRef<BYTE>(m_BSDesc.RenderTarget[i].RenderTargetWriteMask);
    m_pVars->ReplaceVar(sRenderTargetWriteMask + CStr(i), pVar, true);
  }

  m_vBlendFactor.Set(1, 1, 1, 1);
  m_uiSampleMask = 0xffffffff;

  pVar = new CVarRef<CVector<4> >(m_vBlendFactor); 
  m_pVars->ReplaceVar(sBlendFactor, pVar, true);

  pVar = new CVarRef<int>(*(int *) &m_uiSampleMask);
  m_pVars->ReplaceVar(sSampleMask, pVar, true);

  m_pBS = 0;

  return true;
}

CStateVarObj::TStr2Int *CBlendState::GetDict_s(int &iCount)
{
  static CStrConst sZero("Zero");
  static CStrConst sOne("One");
  static CStrConst sSrcColor("SrcColor");
  static CStrConst sInvSrcColor("InvSrcColor");
  static CStrConst sSrcAlpha("SrcAlpha");
  static CStrConst sInvSrcAlpha("InvSrcAlpha");
  static CStrConst sDestColor("DestColor");
  static CStrConst sInvDestColor("InvDestColor");
  static CStrConst sDestAlpha("DestAlpha");
  static CStrConst sInvDestAlpha("InvDestAlpha");
  static CStrConst sSrcAlphaSat("SrcAlphaSat");
  static CStrConst sBlendFactor("BlendFactor");
  static CStrConst sInvBlendFactor("InvBlendFactor");
  static CStrConst sSrc1Color("Src1Color");
  static CStrConst sInvSrc1Color("InvSrc1Color");
  static CStrConst sSrc1Alpha("Src1Alpha");
  static CStrConst sInvSrc1Alpha("InvSrc1Alpha");

  static CStrConst sAdd("Add");
  static CStrConst sSubtract("Subtract");
  static CStrConst sRevSubtract("RevSubtract");
  static CStrConst sMin("Min");
  static CStrConst sMax("Max");

  static TStr2Int arrStr2Blend[] = {
    { sZero, D3D11_BLEND_ZERO },
    { sOne, D3D11_BLEND_ONE },
    { sSrcColor, D3D11_BLEND_SRC_COLOR },
    { sInvSrcColor, D3D11_BLEND_INV_SRC_COLOR },
    { sSrcAlpha, D3D11_BLEND_SRC_ALPHA },
    { sInvSrcAlpha, D3D11_BLEND_INV_SRC_ALPHA },
    { sDestAlpha, D3D11_BLEND_DEST_ALPHA },
    { sInvDestAlpha, D3D11_BLEND_INV_DEST_ALPHA },
    { sDestColor, D3D11_BLEND_DEST_COLOR },
    { sInvDestColor, D3D11_BLEND_INV_DEST_COLOR },
    { sSrcAlphaSat, D3D11_BLEND_SRC_ALPHA_SAT },
    { sBlendFactor, D3D11_BLEND_BLEND_FACTOR },
    { sInvBlendFactor, D3D11_BLEND_INV_BLEND_FACTOR },
    { sSrc1Color, D3D11_BLEND_SRC1_COLOR },
    { sInvSrc1Color, D3D11_BLEND_INV_SRC1_COLOR },
    { sSrc1Alpha, D3D11_BLEND_SRC1_ALPHA },
    { sInvSrc1Alpha, D3D11_BLEND_INV_SRC1_ALPHA },

    { sAdd, D3D11_BLEND_OP_ADD },
    { sSubtract, D3D11_BLEND_OP_SUBTRACT },
    { sRevSubtract, D3D11_BLEND_OP_REV_SUBTRACT },
    { sMin, D3D11_BLEND_OP_MIN },
    { sMax, D3D11_BLEND_OP_MAX },
  };

  iCount = ARRSIZE(arrStr2Blend);
  return arrStr2Blend;
}

CStateVarObj::TStr2Int *CBlendState::GetDict(int &iCount) 
{ 
  return GetDict_s(iCount); 
}

void CBlendState::ReleaseState()
{
  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  SAFE_RELEASE(m_pBS);
}

bool CBlendState::StateReleased()
{
  return !m_pBS;
}

bool CBlendState::Commit(bool bOnlyIfChanged)
{
  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  if (!m_pBS) {
    HRESULT res;
    TResReporter kErrorReport(res);

    res = CGraphics::Get()->m_pDevice->CreateBlendState(&m_BSDesc, &m_pBS);
    if (FAILED(res))
      return false;
  } else
    if (bOnlyIfChanged)
      return true;

  CGraphics::Get()->m_pDeviceContext->OMSetBlendState(m_pBS, &m_vBlendFactor.x(), m_uiSampleMask);
  return true;
}

// CDepthStencilState ---------------------------------------------------------
IMPRTTI(CDepthStencilState, CStateVarObj)
IMP_VAR_RTTI(CDepthStencilState *)
IMP_VAR_RTTI(CDepthStencilState)

bool CDepthStencilState::Init()
{
  static CStrConst sDepthEnable("DepthEnable");
  static CStrConst sDepthWriteMask("DepthWriteMask");
  static CStrConst sDepthFunc("DepthFunc");
  static CStrConst sStencilEnable("StencilEnable");
  static CStrConst sStencilReadMask("StencilReadMask");
  static CStrConst sStencilWriteMask("StencilWriteMask");
  static CStrConst sStencilFunc("StencilFunc");
  static CStrConst sStencilDepthFailOp("StencilDepthFailOp");
  static CStrConst sStencilPassOp("StencilPassOp");
  static CStrConst sStencilFailOp("StencilFailOp");
  static CStrConst sStencilFuncBack("StencilFuncBack");
  static CStrConst sStencilDepthFailOpBack("StencilDepthFailOpBack");
  static CStrConst sStencilPassOpBack("StencilPassOpBack");
  static CStrConst sStencilFailOpBack("StencilFailOpBack");
  static CStrConst sStencilRef("StencilRef");

  if (!CStateVarObj::Init())
    return false;

  m_DSDesc.DepthEnable = true;
  m_DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  m_DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
  m_DSDesc.StencilEnable = false;
  m_DSDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
  m_DSDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
  m_DSDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
  m_DSDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
  m_DSDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  m_DSDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  m_DSDesc.BackFace = m_DSDesc.FrontFace;

  CBaseVar *pVar;

  pVar = new CVarRef<int>(m_DSDesc.DepthEnable);
  m_pVars->ReplaceVar(sDepthEnable, pVar, true);

  COMPILE_ASSERT(sizeof(m_DSDesc.DepthWriteMask) == sizeof(int));
  pVar = new CVarRef<int>(*(int *) &m_DSDesc.DepthWriteMask);
  m_pVars->ReplaceVar(sDepthWriteMask, pVar, true);

  COMPILE_ASSERT(sizeof(D3D11_COMPARISON_FUNC) == sizeof(int));
  pVar = new CVarRef<int>(*(int *) &m_DSDesc.DepthFunc);
  m_pVars->ReplaceVar(sDepthFunc, pVar, true);

  pVar = new CVarRef<int>(m_DSDesc.StencilEnable);
  m_pVars->ReplaceVar(sStencilEnable, pVar, true);

  pVar = new CVarRef<BYTE>(m_DSDesc.StencilReadMask);
  m_pVars->ReplaceVar(sStencilReadMask, pVar, true);

  pVar = new CVarRef<BYTE>(m_DSDesc.StencilWriteMask);
  m_pVars->ReplaceVar(sStencilWriteMask, pVar, true);

  pVar = new CVarRef<int>(*(int *) &m_DSDesc.FrontFace.StencilFunc);
  m_pVars->ReplaceVar(sStencilFunc, pVar, true);

  COMPILE_ASSERT(sizeof(D3D11_STENCIL_OP) == sizeof(int));
  pVar = new CVarRef<int>(*(int *) &m_DSDesc.FrontFace.StencilDepthFailOp);
  m_pVars->ReplaceVar(sStencilDepthFailOp, pVar, true);

  pVar = new CVarRef<int>(*(int *) &m_DSDesc.FrontFace.StencilPassOp);
  m_pVars->ReplaceVar(sStencilPassOp, pVar, true);

  pVar = new CVarRef<int>(*(int *) &m_DSDesc.FrontFace.StencilFailOp);
  m_pVars->ReplaceVar(sStencilFailOp, pVar, true);

  pVar = new CVarRef<int>(*(int *) &m_DSDesc.BackFace.StencilFunc);
  m_pVars->ReplaceVar(sStencilFuncBack, pVar, true);

  pVar = new CVarRef<int>(*(int *) &m_DSDesc.BackFace.StencilDepthFailOp);
  m_pVars->ReplaceVar(sStencilDepthFailOpBack, pVar, true);

  pVar = new CVarRef<int>(*(int *) &m_DSDesc.BackFace.StencilPassOp);
  m_pVars->ReplaceVar(sStencilPassOpBack, pVar, true);

  pVar = new CVarRef<int>(*(int *) &m_DSDesc.BackFace.StencilFailOp);
  m_pVars->ReplaceVar(sStencilFailOpBack, pVar, true);

  m_uiStencilRef = 0;

  pVar = new CVar<int>(*(int *) &m_uiStencilRef);
  m_pVars->ReplaceVar(sStencilRef, pVar, true);

  m_pDS = 0;

  return true;
}

CStateVarObj::TStr2Int *CDepthStencilState::GetDict_s(int &iCount)
{
  static CStrConst sNever("Never");
  static CStrConst sLess("Less");
  static CStrConst sEqual("Equal");
  static CStrConst sLessEqual("LessEqual");
  static CStrConst sGreater("Greater");
  static CStrConst sNotEqual("NotEqual");
  static CStrConst sGreaterEqual("GreaterEqual");
  static CStrConst sAlways("Always");

  static CStrConst sKeep("Keep");
  static CStrConst sZero("Zero");   
  static CStrConst sReplace("Replace");
  static CStrConst sIncrSat("IncrSat");
  static CStrConst sDecrSat("DecrSat");
  static CStrConst sInvert("Invert"); 
  static CStrConst sIncr("Incr");   
  static CStrConst sDecr("Decr");   

  static TStr2Int arrStr2Stencil[] = {
    { sNever, D3D11_COMPARISON_NEVER },
    { sLess, D3D11_COMPARISON_LESS },
    { sEqual, D3D11_COMPARISON_EQUAL },
    { sLessEqual, D3D11_COMPARISON_LESS_EQUAL  },
    { sGreater, D3D11_COMPARISON_GREATER },
    { sNotEqual, D3D11_COMPARISON_NOT_EQUAL },
    { sGreaterEqual, D3D11_COMPARISON_GREATER_EQUAL },
    { sAlways, D3D11_COMPARISON_ALWAYS },

    { sKeep, D3D11_STENCIL_OP_KEEP },
    { sZero, D3D11_STENCIL_OP_ZERO },
    { sReplace, D3D11_STENCIL_OP_REPLACE },
    { sIncrSat, D3D11_STENCIL_OP_INCR_SAT  },
    { sDecrSat, D3D11_STENCIL_OP_DECR_SAT },
    { sInvert, D3D11_STENCIL_OP_INVERT },
    { sIncr, D3D11_STENCIL_OP_INCR },
    { sDecr, D3D11_STENCIL_OP_DECR },
  };

  iCount = ARRSIZE(arrStr2Stencil);
  return arrStr2Stencil;
}

CStateVarObj::TStr2Int *CDepthStencilState::GetDict(int &iCount) 
{ 
  return GetDict_s(iCount); 
}

void CDepthStencilState::ReleaseState()
{
  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  SAFE_RELEASE(m_pDS);
}

bool CDepthStencilState::StateReleased()
{
  return !m_pDS;
}

bool CDepthStencilState::Commit(bool bOnlyIfChanged)
{
  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  if (!m_pDS) {
    HRESULT res;
    TResReporter kErrorReport(res);

    res = CGraphics::Get()->m_pDevice->CreateDepthStencilState(&m_DSDesc, &m_pDS);
    if (FAILED(res))
      return false;
  } else
    if (bOnlyIfChanged)
      return true;

  CGraphics::Get()->m_pDeviceContext->OMSetDepthStencilState(m_pDS, m_uiStencilRef);
  return true;
}

// CStateCache ----------------------------------------------------------------

CStateCache::CStateCache()
{
  for (int i = 0; i < ARRSIZE(m_pStates); i++)
    m_pStates[i] = 0;
  m_pParent = 0;
}

CStateCache::~CStateCache()
{
  Done();
}


bool CStateCache::Init(bool bRasterizer, bool bBlend, bool bDepthStencil, CStateCache *pParent, CVarObj *pDefStates)
{
  ASSERT(!m_pStates[0] && !m_pStates[1] && !m_pStates[2]);
  if (bRasterizer) {
    m_pStates[ST_RASTERIZER] = new CRasterizerState();
    if (!m_pStates[ST_RASTERIZER]->Init())
      return false;
  }
  if (bBlend) {
    m_pStates[ST_BLEND] = new CBlendState();
    if (!m_pStates[ST_BLEND]->Init())
      return false;
  }
  if (bDepthStencil) {
    m_pStates[ST_DEPTHSTENCIL] = new CDepthStencilState();
    if (!m_pStates[ST_DEPTHSTENCIL]->Init())
      return false;
  }
  m_pParent = pParent;

  if (pDefStates)
    for (int i = 0; i < ARRSIZE(m_pStates); i++) 
      if (m_pStates[i])
        m_pStates[i]->ApplyVars(pDefStates, false);

  return true;
}

void CStateCache::Done()
{
  for (int i = 0; i < ARRSIZE(m_pStates); i++)
    SAFE_DELETE(m_pStates[i]);
  m_pParent = 0;
}

CStateCache *CStateCache::GetContainer(EStateType eStateType)
{
  for (CStateCache *pContainer = this; pContainer; pContainer = pContainer->m_pParent)
    if (pContainer->m_pStates[eStateType])
      return pContainer;
  return 0;
}

bool CStateCache::GetVar(const CStrBase &sVar, CBaseVar &vDst) const
{
  for (int i = 0; i < ARRSIZE(m_pStates); i++) {
    if (!m_pStates[i])
      continue;
    CBaseVar *pVar = m_pStates[i]->FindVar(sVar);
    if (!pVar)
      continue;
    return vDst.SetVar(*pVar);
  }
  if (m_pParent)
    return m_pParent->GetVar(sVar, vDst);
  return false;
}

bool CStateCache::SetVar(const CStrBase &sVar, const CBaseVar &vSrc)
{
  for (int i = 0; i < ARRSIZE(m_pStates); i++) {
    if (!m_pStates[i])
      continue;
    CBaseVar *pVar = m_pStates[i]->FindVar(sVar);
    if (!pVar)
      continue;
    bool bRes = pVar->SetVar(vSrc);
    m_pStates[i]->ReleaseState();
    return bRes;
  }
  if (m_pParent)
    return m_pParent->SetVar(sVar, vSrc);
  return false;
}
