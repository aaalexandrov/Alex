#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include <D3D11.h>
#include "Texture.h"
#include "Threads.h"

// CGraphics ------------------------------------------------------------------

class CRasterizerState;
class CBlendState;
class CDepthStencilState;
class CTechnique;
class CShape3D;
class CCamera;
class CFrameSorter;
class CGraphics {
public:
  static CGraphics *s_pGraphics;
  static inline CGraphics *Get() { return s_pGraphics; }

  typedef CHash<CTechnique *, CStrAny, CTechnique, CTechnique> TTechHash;
  typedef CHash<CSampler *, const CSampler::TDesc &, CSampler, CSampler> TSamplerHash;

  CLock                   m_Lock;
  IDXGISwapChain         *m_pSwapChain;
  ID3D11Device           *m_pDevice;
  ID3D11DeviceContext    *m_pDeviceContext;
  D3D_FEATURE_LEVEL       m_DeviceFeatureLevel;
  ID3D11RenderTargetView *m_pRTView;
  ID3D11DepthStencilView *m_pDepthView;
  HWND                    m_hWnd;
  bool                    m_bFullscreen;
  DXGI_FORMAT             m_ePixelFormat;

  CFrameSorter           *m_pFrameSorter;

  UINT                    m_uiFrameID;

  CVarHash                m_Globals;
  CCamera                *m_pCamera;

  TTechHash               m_Techniques;
  TSamplerHash            m_Samplers;

  UINT                    m_uiDrawPrimitiveCount, m_uiTriangleCount, m_uiVertexCount;

  CGraphics();
  ~CGraphics();

  bool Init(HWND hWnd, bool bFullscreen, DXGI_FORMAT ePixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM);
  bool InitDevice();
  bool InitSwapChain(UINT uiWidth, UINT uiHeight);
  bool InitBackBuffers();

  void Done();
  void DoneDevice();
  void DoneSwapChain();
  void DoneBackBuffers();

  bool Resize(UINT uiWidth, UINT uiHeight, bool bFullscreen);

  bool SetSorter(CFrameSorter *pSorter);

  bool LoadTechnique(CStrAny sVarFile);
  CTechnique *GetTechnique(CStrAny sName);

  CSampler *GetSampler(const CSampler::TDesc &kDesc);

  void GetViewport(CRect<> &rc, float &fMinZ, float &fMaxZ);
  void SetViewport(const CRect<> &rc, float fMinZ, float fMaxZ);
  void Clear(const CVector<4> &vColor, float fDepth, uint8_t btStencil);
  bool Present();
  void ResetCounts();

  void SetCamera(CCamera *pCamera);
  bool CheckVisibility(CShape3D *pBound);
};


// Convenience stuff ----------------------------------------------------------

void ShowDXError(HRESULT res);

struct TResReporter {
  HRESULT &m_Res;

  TResReporter(HRESULT &res): m_Res(res) { m_Res = S_OK; }
  ~TResReporter() { if (FAILED(m_Res)) ShowDXError(m_Res); }
};

#endif