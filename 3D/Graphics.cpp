#include "stdafx.h"
#include "Graphics.h"

#include <dxerr.h>
#include <D3DCompiler.h>

#include "Shader.h"
#include "Convex.h"
#include "Camera.h"
#include "State.h"
#include "FrameSorter.h"

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXErr.lib")
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "D3DX11.lib")

void ShowDXError(HRESULT res)
{
  CStr sError, sDesc, sMsg;
  sError = DXGetErrorString(res);
  sDesc = DXGetErrorDescription(res);
  sMsg = sError + CStrPart(": ") + sDesc;
  MessageBox(0, sMsg, "DX Error", MB_OK);
}

// CGraphics -------------------------------------------------------------------------

CGraphics *CGraphics::s_pGraphics = 0;

CGraphics::CGraphics(): m_Lock(2000)
{
  s_pGraphics = this;
  m_pSwapChain = 0;
  m_pDevice = 0;
  m_pDeviceContext = 0;
  m_DeviceFeatureLevel = (D3D_FEATURE_LEVEL) 0;
  m_pRTView = 0;
  m_pDepthView = 0;
  m_hWnd = 0;
  m_bFullscreen = false;
  m_ePixelFormat = DXGI_FORMAT_UNKNOWN;
  m_pCamera = 0;
  m_uiFrameID = 0;
  m_pFrameSorter = 0;
  ResetCounts();
}

CGraphics::~CGraphics()
{
  s_pGraphics = 0;
}

bool CGraphics::Init(HWND hWnd, bool bFullscreen, DXGI_FORMAT ePixelFormat)
{
  m_hWnd = hWnd;
  m_bFullscreen = bFullscreen;
  m_ePixelFormat = ePixelFormat;
  ResetCounts();

  UINT uiWidth, uiHeight;
  RECT rc;
  GetClientRect(m_hWnd, &rc);
  uiWidth = rc.right - rc.left;
  uiHeight = rc.bottom - rc.top;

  if (!InitDevice())
    return false;
  if (!InitSwapChain(uiWidth, uiHeight))
    return false;
  if (!InitBackBuffers())
    return false;

  if (!m_pFrameSorter)
    SetSorter(new CSimpleSorter());

  return true;
}

bool CGraphics::InitDevice()
{
  HRESULT res;

  UINT uiDevFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;

#ifdef _DEBUG
  uiDevFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  res = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, uiDevFlags, 0, 0, D3D11_SDK_VERSION, &m_pDevice, &m_DeviceFeatureLevel, &m_pDeviceContext);
  if (FAILED(res))
    return false;

  return true;
}

bool CGraphics::InitSwapChain(UINT uiWidth, UINT uiHeight)
{
  HRESULT res;
  DXGI_SWAP_CHAIN_DESC scd;

  CScopeLock kLock(&m_Lock);

  CAutoReleasePtr<IDXGIDevice> pDXGIDevice;
  res = m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void **) &pDXGIDevice.m_pPtr);
  if (FAILED(res))
    return false;

  CAutoReleasePtr<IDXGIAdapter> pDXGIAdapter;
  res = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **) &pDXGIAdapter.m_pPtr);
  if (FAILED(res))
    return false;

  CAutoReleasePtr<IDXGIFactory> pDXGIFactory;
  res = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void **) &pDXGIFactory.m_pPtr);
  if (FAILED(res))
    return false;

  if (m_bFullscreen) {
    DXGI_MODE_DESC md;
    md.Width = uiWidth;
    md.Height = uiHeight;
    md.Format = m_ePixelFormat;
    md.RefreshRate.Numerator = 0;
    md.RefreshRate.Denominator = 0;
    md.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    md.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    CAutoReleasePtr<IDXGIOutput> pDXGIOutput;
    res = pDXGIAdapter->EnumOutputs(0, &pDXGIOutput.m_pPtr);
    if (FAILED(res))
      return false;
    pDXGIOutput->FindClosestMatchingMode(&md, &scd.BufferDesc, m_pDevice);
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 2;
    scd.OutputWindow = m_hWnd;
    scd.Windowed = false;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  } else {
    scd.BufferDesc.Width = uiWidth;
    scd.BufferDesc.Height = uiHeight;
    scd.BufferDesc.RefreshRate.Numerator = 0;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferDesc.Format = m_ePixelFormat;
    scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    scd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 1;
    scd.OutputWindow = m_hWnd;
    scd.Windowed = true;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  }

  res = pDXGIFactory->CreateSwapChain(m_pDevice, &scd, &m_pSwapChain);
  if (FAILED(res))
    return false;

  res = pDXGIFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_PRINT_SCREEN);
  if (FAILED(res))
    return false;

  return true;
}

bool CGraphics::InitBackBuffers()
{
  HRESULT res;

  CScopeLock kLock(&m_Lock);

  CAutoReleasePtr<ID3D11Texture2D> pBackBuffer;
  res = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &pBackBuffer.m_pPtr);
  if (FAILED(res))
    return false;

  D3D11_TEXTURE2D_DESC t2dd;
  pBackBuffer->GetDesc(&t2dd);

  res = m_pDevice->CreateRenderTargetView(pBackBuffer, 0, &m_pRTView);
  if (FAILED(res))
    return false;

  CAutoReleasePtr<ID3D11Texture2D> pDepthStencil;
  t2dd.MipLevels = 1;
  t2dd.ArraySize = 1;
  t2dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  t2dd.SampleDesc.Count = 1;
  t2dd.SampleDesc.Quality = 0;
  t2dd.Usage = D3D11_USAGE_DEFAULT;
  t2dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  t2dd.CPUAccessFlags = 0;
  t2dd.MiscFlags = 0;

  res = m_pDevice->CreateTexture2D(&t2dd, 0, &pDepthStencil.m_pPtr);
  if (FAILED(res))
    return false;

  res = m_pDevice->CreateDepthStencilView(pDepthStencil, 0, &m_pDepthView);
  if (FAILED(res))
    return false;

  m_pDeviceContext->OMSetRenderTargets(1, &m_pRTView, m_pDepthView);

  SetViewport(CRect<>(0, 0, (float) t2dd.Width, (float) t2dd.Height), 0, 1);

  return true;
}

void CGraphics::Done()
{
  m_Samplers.DeleteAll();
  m_Techniques.DeleteAll();
  SAFE_DELETE(m_pFrameSorter);
  DoneBackBuffers();
  DoneSwapChain();
  DoneDevice();
  m_hWnd = 0;
}

void CGraphics::DoneDevice()
{
  SAFE_RELEASE(m_pDeviceContext);
  SAFE_RELEASE(m_pDevice);
}

void CGraphics::DoneSwapChain()
{
  CScopeLock kLock(&m_Lock);

  if (m_bFullscreen)
    m_pSwapChain->SetFullscreenState(false, 0);
  SAFE_RELEASE(m_pSwapChain);
}

void CGraphics::DoneBackBuffers()
{
  CScopeLock kLock(&m_Lock);

  m_pDeviceContext->ClearState();
  SAFE_RELEASE(m_pDepthView);
  SAFE_RELEASE(m_pRTView);
}

bool CGraphics::SetSorter(CFrameSorter *pSorter)
{
  ASSERT(pSorter);
  delete m_pFrameSorter;
  m_pFrameSorter = pSorter;
  return true;
}

bool CGraphics::Resize(UINT uiWidth, UINT uiHeight, bool bFullscreen)
{
  // Resize is made to be called when handling WM_SIZE or when switching fullscreen / windowed
  // Switching however triggers WM_SIZE to be sent by DXGI, so we record the new size in static variables and use them to resize the swapchain 
  // in case the size is different. It can be different because it comes from the window's last size as recorded by DXGI.
  struct TRecurseGuard {
    inline TRecurseGuard()       { Active() = true;  }
    inline ~TRecurseGuard()      { Active() = false; }
    inline static bool &Active() { static bool bActive = false; return bActive; }
  };

  static UINT uiNewWidth, uiNewHeight;

  if (TRecurseGuard::Active()) {
    uiNewWidth = uiWidth;
    uiNewHeight = uiHeight;
    return false;
  }
  TRecurseGuard kGuard;

  CScopeLock kLock(&m_Lock);

  uiNewWidth = uiWidth;
  uiNewHeight = uiHeight;
  bool bRes;
  DoneBackBuffers();
  if (bFullscreen != m_bFullscreen) {
    DoneSwapChain();
    m_bFullscreen = bFullscreen;
    InitSwapChain(uiWidth, uiHeight);
    if (uiNewWidth != uiWidth || uiNewHeight != uiHeight) 
      m_pSwapChain->ResizeBuffers(m_bFullscreen ? 2 : 1, uiNewWidth, uiNewHeight, m_ePixelFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
  } else
    m_pSwapChain->ResizeBuffers(m_bFullscreen ? 2 : 1, uiWidth, uiHeight, m_ePixelFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
  bRes = InitBackBuffers();
  return bRes;
}

bool CGraphics::LoadTechnique(CStr sVarFile)
{
  CTechnique *pTech = new CTechnique(sVarFile);
  if (!pTech->IsValid()) {
    delete pTech;
    return false;
  }

  m_Techniques.Add(pTech);

  return true;
}

CTechnique *CGraphics::GetTechnique(CStrConst sName)
{
  TTechHash::TIter it;
  it = m_Techniques.Find(sName);
  if (!it)
    return 0;
  return *it;
}

CSampler *CGraphics::GetSampler(const CSampler::TDesc &kDesc)
{
  TSamplerHash::TIter it;
  it = m_Samplers.Find(kDesc);
  if (it)
    return *it;
  CSampler *pSamp = new CSampler();
  if (!pSamp->Init(kDesc)) {
    delete pSamp;
    return 0;
  }
  m_Samplers.Add(pSamp);
  return pSamp;
}

void CGraphics::ResetCounts()
{
  m_uiDrawPrimitiveCount = 0;
  m_uiTriangleCount = 0;
  m_uiVertexCount = 0;
}

void CGraphics::GetViewport(CRect<> &rc, float &fMinZ, float &fMaxZ)
{
  D3D11_VIEWPORT vp;
  UINT uiCount = 1;

  CScopeLock kLock(&m_Lock);

  m_pDeviceContext->RSGetViewports(&uiCount, &vp);
  rc.Set(vp.TopLeftX, vp.TopLeftY, vp.TopLeftX + vp.Width, vp.TopLeftY + vp.Height);
  fMinZ = vp.MinDepth;
  fMaxZ = vp.MaxDepth;
}

void CGraphics::SetViewport(const CRect<> &rc, float fMinZ, float fMaxZ)
{
  D3D11_VIEWPORT vp;
  vp.TopLeftX = rc.m_vMin.x();
  vp.TopLeftY = rc.m_vMin.y();
  vp.Width = rc.GetWidth();
  vp.Height = rc.GetHeight();
  vp.MinDepth = fMinZ;
  vp.MaxDepth = fMaxZ;

  CScopeLock kLock(&m_Lock);

  m_pDeviceContext->RSSetViewports(1, &vp);
}

void CGraphics::Clear(const CVector<4> &vColor, float fDepth, BYTE btStencil)
{
  CScopeLock kLock(&m_Lock);

  m_pDeviceContext->ClearRenderTargetView(m_pRTView, (const float*) &vColor);
  m_pDeviceContext->ClearDepthStencilView(m_pDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, fDepth, btStencil); 
}

bool CGraphics::Present()
{
  HRESULT res;

  ResetCounts();
  m_uiFrameID++;

  if (!m_pFrameSorter->Present())
    return false;

  CScopeLock kLock(&m_Lock);

  res = m_pSwapChain->Present(0, 0);
  if (FAILED(res))
    return false;

  return true;
}

void CGraphics::SetCamera(CCamera *pCamera)
{
  if (m_pCamera && !pCamera)
    m_pCamera->UnsetVars(&m_Globals);
  m_pCamera = pCamera;
  if (m_pCamera)
    m_pCamera->SetVars(&m_Globals);
}

bool CGraphics::CheckVisibility(CShape3D *pBound)
{
  if (!m_pCamera || !pBound)
    return true;
  bool bRes = m_pCamera->m_Frustum.Intersects(pBound, true);
  return bRes;
}
