#include "stdafx.h"
#include "State.h"
#include "Graphics.h"

// CStateVarObj ---------------------------------------------------------------
CRTTIRegisterer<CStateVarObj> g_RegStateVarObj;

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

CVarObj::CIter *CStateVarObj::GetIter(const CStrAny &sVar) const
{
  return m_pVars->GetIter(sVar);
}

CBaseVar *CStateVarObj::FindVar(const CStrAny &sVar) const
{
  return m_pVars->FindVar(sVar);
}

bool CStateVarObj::ReplaceVar(const CStrAny &sVar, CBaseVar *pSrc, bool bAdding)
{
  ASSERT(0);
  return false;
}

bool CStateVarObj::SetVar(const CStrAny &sVar, const CBaseVar &vSrc)
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
  CAutoReleasePtr<CBaseVar const> pNewVal(nullptr);

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

int CStateVarObj::GetStrIndex(CStrAny const *pStr, TStr2Int *pDict, int iCount)
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
  CVarValueBase<CStrAny> const *pStrVal = Cast<CVarValueBase<CStrAny> >(pValue);
  if (pStrVal)
    for (i = 0; i < iCount; i++)
      if (pStrVal->GetValue() == pDict[i].sName)
        return new CVar<int>(pDict[i].iVal);
  return pValue;
}

inline bool CStateVarObj::EqualVars(CBaseVar const *pVar0, CBaseVar const *pVar1)
{
  ASSERT(pVar0 && pVar1);
  if (pVar0->GetRTTI()->IsKindOf(CVarValueBase<int>::GetRTTI_s()) && pVar1->GetRTTI()->IsKindOf(CVarValueBase<int>::GetRTTI_s())) 
    return ((CVarValueBase<int> *) pVar0)->GetValue() == ((CVarValueBase<int> *) pVar1)->GetValue();

  if (pVar0->GetRTTI()->IsKindOf(CVarValueBase<float>::GetRTTI_s()) && pVar1->GetRTTI()->IsKindOf(CVarValueBase<float>::GetRTTI_s())) 
    return ((CVarValueBase<float> *) pVar0)->GetValue() == ((CVarValueBase<float> *) pVar1)->GetValue();

  if (pVar0->GetRTTI()->IsKindOf(CVarValueBase<uint8_t>::GetRTTI_s()) && pVar1->GetRTTI()->IsKindOf(CVarValueBase<uint8_t>::GetRTTI_s())) 
    return ((CVarValueBase<uint8_t> *) pVar0)->GetValue() == ((CVarValueBase<uint8_t> *) pVar1)->GetValue();

  if (pVar0->GetRTTI()->IsKindOf(CVarValueBase<CVector<4> >::GetRTTI_s()) && pVar1->GetRTTI()->IsKindOf(CVarValueBase<CVector<4> >::GetRTTI_s())) 
    return ((CVarValueBase<CVector<4> > *) pVar0)->GetValue() == ((CVarValueBase<CVector<4> > *) pVar1)->GetValue();

  return false;
}


// CRasterizerState -----------------------------------------------------------
CRTTIRegisterer<CRasterizerState> g_RegRasterizerState;
CVarRTTIRegisterer<CRasterizerState *> g_RegVarRasterizerStatePtr;
CVarRTTIRegisterer<CRasterizerState> g_RegVarRasterizerState;

bool CRasterizerState::Init()
{
  static CStrAny sFillMode(ST_CONST, "FillMode");
  static CStrAny sCullMode(ST_CONST, "CullMode");
  static CStrAny sFrontCCW(ST_CONST, "FrontCounterClockwise");
  static CStrAny sDepthBias(ST_CONST, "DepthBias");
  static CStrAny sSlopeScaledDepthBias(ST_CONST, "SlopeScaledDepthBias");
  static CStrAny sDepthBiasClamp(ST_CONST, "DepthBiasClamp");
  static CStrAny sDepthClipEnable(ST_CONST, "DepthClipEnable");
  static CStrAny sScissorEnable(ST_CONST, "ScissorEnable");
  static CStrAny sMultisampleEnable(ST_CONST, "MultisampleEnable");
  static CStrAny sAntialiasedLineEnable(ST_CONST, "AntialiasedLineEnable");

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
  static CStrAny sSolid(ST_CONST, "Solid");
  static CStrAny sWireframe(ST_CONST, "Wireframe");
  static CStrAny sFront(ST_CONST, "Front");
  static CStrAny sBack(ST_CONST, "Back");
  static CStrAny sNone(ST_CONST, "None");

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
CRTTIRegisterer<CBlendState> g_RegBlendState;
CVarRTTIRegisterer<CBlendState *> g_RegVarBlendStatePtr;
CVarRTTIRegisterer<CBlendState> g_RegVarBlendState;

bool CBlendState::Init()
{
  static CStrAny sAlphaToCoverageEnable(ST_CONST, "AlphaToCoverageEnable");
  static CStrAny sIndependentBlendEnable(ST_CONST, "IndependentBlendEnable");
  static CStrAny sBlendEnable(ST_CONST, "BlendEnable");
  static CStrAny sSrcBlend(ST_CONST, "SrcBlend");
  static CStrAny sDestBlend(ST_CONST, "DestBlend");
  static CStrAny sBlendOp(ST_CONST, "BlendOp");
  static CStrAny sSrcBlendAlpha(ST_CONST, "SrcBlendAlpha");
  static CStrAny sDestBlendAlpha(ST_CONST, "DestBlendAlpha");
  static CStrAny sBlendOpAlpha(ST_CONST, "BlendOpAlpha");
  static CStrAny sRenderTargetWriteMask(ST_CONST, "RenderTargetWriteMask");
  static CStrAny sBlendFactor(ST_CONST, "BlendFactor");
  static CStrAny sSampleMask(ST_CONST, "SampleMask");

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
    m_pVars->ReplaceVar(sBlendEnable + CStrAny(ST_STR, i), pVar, true);

    COMPILE_ASSERT(sizeof(D3D11_BLEND) == sizeof(int));
    pVar = new CVarRef<int>(*(int *) &m_BSDesc.RenderTarget[i].SrcBlend);
    m_pVars->ReplaceVar(sSrcBlend + CStrAny(ST_STR, i), pVar, true);

    pVar = new CVarRef<int>(*(int *) &m_BSDesc.RenderTarget[i].DestBlend);
    m_pVars->ReplaceVar(sDestBlend + CStrAny(ST_STR, i), pVar, true);

    COMPILE_ASSERT(sizeof(D3D11_BLEND_OP) == sizeof(int));
    pVar = new CVarRef<int>(*(int *) &m_BSDesc.RenderTarget[i].BlendOp);
    m_pVars->ReplaceVar(sBlendOp + CStrAny(ST_STR, i), pVar, true);

    pVar = new CVarRef<int>(*(int *) &m_BSDesc.RenderTarget[i].SrcBlendAlpha);
    m_pVars->ReplaceVar(sSrcBlendAlpha + CStrAny(ST_STR, i), pVar, true);

    pVar = new CVarRef<int>(*(int *) &m_BSDesc.RenderTarget[i].DestBlendAlpha);
    m_pVars->ReplaceVar(sDestBlendAlpha + CStrAny(ST_STR, i), pVar, true);

    pVar = new CVarRef<int>(*(int *) &m_BSDesc.RenderTarget[i].BlendOpAlpha);
    m_pVars->ReplaceVar(sBlendOpAlpha + CStrAny(ST_STR, i), pVar, true);

    pVar = new CVarRef<uint8_t>(m_BSDesc.RenderTarget[i].RenderTargetWriteMask);
    m_pVars->ReplaceVar(sRenderTargetWriteMask + CStrAny(ST_STR, i), pVar, true);
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
  static CStrAny sZero(ST_CONST, "Zero");
  static CStrAny sOne(ST_CONST, "One");
  static CStrAny sSrcColor(ST_CONST, "SrcColor");
  static CStrAny sInvSrcColor(ST_CONST, "InvSrcColor");
  static CStrAny sSrcAlpha(ST_CONST, "SrcAlpha");
  static CStrAny sInvSrcAlpha(ST_CONST, "InvSrcAlpha");
  static CStrAny sDestColor(ST_CONST, "DestColor");
  static CStrAny sInvDestColor(ST_CONST, "InvDestColor");
  static CStrAny sDestAlpha(ST_CONST, "DestAlpha");
  static CStrAny sInvDestAlpha(ST_CONST, "InvDestAlpha");
  static CStrAny sSrcAlphaSat(ST_CONST, "SrcAlphaSat");
  static CStrAny sBlendFactor(ST_CONST, "BlendFactor");
  static CStrAny sInvBlendFactor(ST_CONST, "InvBlendFactor");
  static CStrAny sSrc1Color(ST_CONST, "Src1Color");
  static CStrAny sInvSrc1Color(ST_CONST, "InvSrc1Color");
  static CStrAny sSrc1Alpha(ST_CONST, "Src1Alpha");
  static CStrAny sInvSrc1Alpha(ST_CONST, "InvSrc1Alpha");

  static CStrAny sAdd(ST_CONST, "Add");
  static CStrAny sSubtract(ST_CONST, "Subtract");
  static CStrAny sRevSubtract(ST_CONST, "RevSubtract");
  static CStrAny sMin(ST_CONST, "Min");
  static CStrAny sMax(ST_CONST, "Max");

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
CRTTIRegisterer<CDepthStencilState> g_RegDepthStencilState;
CVarRTTIRegisterer<CDepthStencilState *> g_RegVarDepthStencilStatePtr;
CVarRTTIRegisterer<CDepthStencilState> g_RegVarDepthStencilState;

bool CDepthStencilState::Init()
{
  static CStrAny sDepthEnable(ST_CONST, "DepthEnable");
  static CStrAny sDepthWriteMask(ST_CONST, "DepthWriteMask");
  static CStrAny sDepthFunc(ST_CONST, "DepthFunc");
  static CStrAny sStencilEnable(ST_CONST, "StencilEnable");
  static CStrAny sStencilReadMask(ST_CONST, "StencilReadMask");
  static CStrAny sStencilWriteMask(ST_CONST, "StencilWriteMask");
  static CStrAny sStencilFunc(ST_CONST, "StencilFunc");
  static CStrAny sStencilDepthFailOp(ST_CONST, "StencilDepthFailOp");
  static CStrAny sStencilPassOp(ST_CONST, "StencilPassOp");
  static CStrAny sStencilFailOp(ST_CONST, "StencilFailOp");
  static CStrAny sStencilFuncBack(ST_CONST, "StencilFuncBack");
  static CStrAny sStencilDepthFailOpBack(ST_CONST, "StencilDepthFailOpBack");
  static CStrAny sStencilPassOpBack(ST_CONST, "StencilPassOpBack");
  static CStrAny sStencilFailOpBack(ST_CONST, "StencilFailOpBack");
  static CStrAny sStencilRef(ST_CONST, "StencilRef");

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

  pVar = new CVarRef<uint8_t>(m_DSDesc.StencilReadMask);
  m_pVars->ReplaceVar(sStencilReadMask, pVar, true);

  pVar = new CVarRef<uint8_t>(m_DSDesc.StencilWriteMask);
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
  static CStrAny sNever(ST_CONST, "Never");
  static CStrAny sLess(ST_CONST, "Less");
  static CStrAny sEqual(ST_CONST, "Equal");
  static CStrAny sLessEqual(ST_CONST, "LessEqual");
  static CStrAny sGreater(ST_CONST, "Greater");
  static CStrAny sNotEqual(ST_CONST, "NotEqual");
  static CStrAny sGreaterEqual(ST_CONST, "GreaterEqual");
  static CStrAny sAlways(ST_CONST, "Always");

  static CStrAny sKeep(ST_CONST, "Keep");
  static CStrAny sZero(ST_CONST, "Zero");   
  static CStrAny sReplace(ST_CONST, "Replace");
  static CStrAny sIncrSat(ST_CONST, "IncrSat");
  static CStrAny sDecrSat(ST_CONST, "DecrSat");
  static CStrAny sInvert(ST_CONST, "Invert"); 
  static CStrAny sIncr(ST_CONST, "Incr");   
  static CStrAny sDecr(ST_CONST, "Decr");   

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

bool CStateCache::GetVar(const CStrAny &sVar, CBaseVar &vDst) const
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

bool CStateCache::SetVar(const CStrAny &sVar, const CBaseVar &vSrc)
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
