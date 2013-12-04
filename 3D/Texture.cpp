#include "stdafx.h"
#include "Texture.h"
#include "Graphics.h"
#include "File.h"
#include <D3DX11.h>

// CSampler ---------------------------------------------------------------

bool CSampler::Init(const TDesc &kDesc)
{
  HRESULT res;
  TResReporter kErrorReport(res);

  Done();
  D3D11_SAMPLER_DESC sd;
  if (kDesc.bAnisotropic)
    sd.Filter = D3D11_ENCODE_ANISOTROPIC_FILTER(false);
  else
    sd.Filter = D3D11_ENCODE_BASIC_FILTER(kDesc.bMinLinear ? D3D11_FILTER_TYPE_LINEAR : D3D11_FILTER_TYPE_POINT,
                                          kDesc.bMagLinear ? D3D11_FILTER_TYPE_LINEAR : D3D11_FILTER_TYPE_POINT,
                                          kDesc.bMipLinear ? D3D11_FILTER_TYPE_LINEAR : D3D11_FILTER_TYPE_POINT,
                                          false);
  sd.AddressU = (D3D11_TEXTURE_ADDRESS_MODE) kDesc.uiAddressU;
  sd.AddressV = (D3D11_TEXTURE_ADDRESS_MODE) kDesc.uiAddressV;
  sd.AddressW = (D3D11_TEXTURE_ADDRESS_MODE) kDesc.uiAddressW;
  sd.MipLODBias = kDesc.fMipLODBias;
  sd.MaxAnisotropy = kDesc.uiMaxAnisotropy;
  sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  sd.BorderColor[0] = sd.BorderColor[1] = sd.BorderColor[2] = sd.BorderColor[3] = 0;
  sd.MinLOD = 0;
  sd.MaxLOD = D3D11_FLOAT32_MAX;

  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  res = CGraphics::Get()->m_pDevice->CreateSamplerState(&sd, &m_pSampler);
  if (FAILED(res))
    return false;
  m_Desc = kDesc;

  return true;
}

// Sampler pointer var

CVarRTTIRegisterer<CSampler *> g_RegVarSamplerPtr;

// CTexture -------------------------------------------------------------------

CRTTIRegisterer<CTexture> g_RegTexture;

UINT CTexture::s_uiTotalMemory = 0, 
     CTexture::s_uiDeviceMemory = 0;

CTexture::CTexture()
{
  m_pTexture = 0;
  m_pStagingTexture = 0;
  m_pTextureView = 0;
  m_eFormat = DXGI_FORMAT_UNKNOWN;
  m_iWidth = m_iHeight = m_iMipLevels = 0;
}

bool CTexture::Init(CStrAny sFilename, int iMipLevels, UINT uiFlags)
{
  Done();

  CAutoDeletePtr<CFile> pFile(CFileSystem::Get()->OpenFile(sFilename, CFile::FOF_READ));
  if (!pFile)
    return false;

  int iSize = (int) pFile->GetSize();
  CAutoDeletePtr<char> pBuf(new char[iSize]);
  CFile::ERRCODE ec = pFile->Read(pBuf, iSize);
  if (ec)
    return false;

  if (!CD3DResource::Init(CResource::RT_TEXTURE2D, uiFlags))
    return false;

  HRESULT res;
  TResReporter kErrorReport(res);

  D3DX11_IMAGE_INFO ii;
  res = D3DX11GetImageInfoFromMemory(pBuf, iSize, 0, &ii, 0);
  if (FAILED(res))
    return false;

  if (!iMipLevels)
    iMipLevels = -Util::MostSignificantBitSet(Util::Max(ii.Width, ii.Height)) - 1;

  if (iMipLevels == -1)
    iMipLevels = 1; 
  else
    if (iMipLevels > 0)
      iMipLevels = Util::Max<int>(iMipLevels, ii.MipLevels);

  if (iMipLevels < 0)
    m_uiFlags |= RF_CANGENERATEMIPS;

  D3DX11_IMAGE_LOAD_INFO ili;

  ili.Width = ii.Width;
  ili.Height = ii.Height;
  ili.Depth = 1;
  ili.FirstMipLevel = 0;
  ili.MipLevels = abs(iMipLevels);
  GetResourceParams(false, ili.Usage, ili.CpuAccessFlags);
  if (m_uiFlags & RF_CANGENERATEMIPS) {
    ili.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    ili.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
  } else {
    ili.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    ili.MiscFlags = 0;
  }
  ili.Format = ii.Format;
  ili.Filter = D3DX11_FILTER_NONE;
  ili.MipFilter = D3DX11_FILTER_LINEAR;
  ili.pSrcInfo = &ii;

  CScopeLock kLock(&CGraphics::Get()->m_Lock);

  res = D3DX11CreateTextureFromMemory(CGraphics::Get()->m_pDevice, pBuf, iSize, &ili, 0, (ID3D11Resource **) &m_pTexture, 0);
  if (FAILED(res))
    return false;

  res = CGraphics::Get()->m_pDevice->CreateShaderResourceView(m_pTexture, 0, &m_pTextureView);
  if (FAILED(res))
    return false;

  D3D11_USAGE eUsage;
  UINT uiCPUAccessFlags;
  if (GetResourceParams(true, eUsage, uiCPUAccessFlags)) {
    m_pStagingTexture = CreateD3DTexture(ii.Width, ii.Height, ii.Format, abs(iMipLevels), 0, 0, eUsage, uiCPUAccessFlags, !!(m_uiFlags & RF_CANGENERATEMIPS));
    if (!m_pStagingTexture)
      return false;
    CGraphics::Get()->m_pDeviceContext->CopyResource(m_pStagingTexture, m_pTexture);
  }

  m_eFormat = ii.Format;
  m_iWidth = ii.Width;
  m_iHeight = ii.Height;
  m_iMipLevels = abs(iMipLevels);

  if (!CreateSystemCopy(0))
    return false;

  return true;
}

bool CTexture::Init(int iWidth, int iHeight, DXGI_FORMAT eFormat, int iMipLevels, uint8_t *pData, int iRowPitch, UINT uiFlags)
{
  D3D11_USAGE eUsage;
  UINT uiCPUAccessFlags;
  HRESULT res;
  TResReporter kErrorReport(res);

  Done();

  if (!iMipLevels)
    iMipLevels = -Util::MostSignificantBitSet(Util::Max(iWidth, iHeight)) - 1;

  if (iMipLevels == -1)
    iMipLevels = 1;

  if (iMipLevels < 0)
    uiFlags |= RF_CANGENERATEMIPS;

  if (!CD3DResource::Init(CResource::RT_TEXTURE2D, uiFlags))
    return false;

  GetResourceParams(false, eUsage, uiCPUAccessFlags);

  {
    CScopeLock kLock(&CGraphics::Get()->m_Lock);

    m_pTexture = CreateD3DTexture(iWidth, iHeight, eFormat, iMipLevels, pData, iRowPitch, eUsage, uiCPUAccessFlags, !!(m_uiFlags & RF_CANGENERATEMIPS));

    if (!m_pTexture)
      return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC rvd;
    rvd.Format = eFormat;
    rvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    rvd.Texture2D.MipLevels = abs(iMipLevels);
    rvd.Texture2D.MostDetailedMip = 0;
    res = CGraphics::Get()->m_pDevice->CreateShaderResourceView((ID3D11Resource *) m_pTexture, &rvd, &m_pTextureView);
    if (FAILED(res))
      return false;

    if (iMipLevels < 0) 
      CGraphics::Get()->m_pDeviceContext->GenerateMips(m_pTextureView);

    if (GetResourceParams(true, eUsage, uiCPUAccessFlags)) {
      m_pStagingTexture = CreateD3DTexture(iWidth, iHeight, eFormat, iMipLevels, pData, iRowPitch, eUsage, uiCPUAccessFlags, !!(m_uiFlags & RF_CANGENERATEMIPS));
      if (!m_pStagingTexture)
        return false;
      CGraphics::Get()->m_pDeviceContext->CopyResource(m_pStagingTexture, m_pTexture);
    }
  }

  m_eFormat = eFormat;
  m_iWidth = iWidth;
  m_iHeight = iHeight;
  m_iMipLevels = abs(iMipLevels);

  if (!CreateSystemCopy((iMipLevels > 0) ? pData : 0))
    return false;

  RecordMemory(true);

  return true;
}

void CTexture::Done() 
{ 
  m_eFormat = DXGI_FORMAT_UNKNOWN;
  m_iWidth = m_iHeight = m_iMipLevels = 0;

  CGraphics::Get()->m_Lock.Lock();
  RecordMemory(false);
  SAFE_RELEASE(m_pTextureView);
  SAFE_RELEASE(m_pStagingTexture);
  SAFE_RELEASE(m_pTexture);
  CGraphics::Get()->m_Lock.Unlock();
  CD3DResource::Done();
}

void CTexture::RecordMemory(bool bAdd)
{
  UINT uiMem, uiDevMem;
  CScopeLock lock(&CGraphics::Get()->m_Lock);
  uiMem = GetMemorySize(&uiDevMem);
  if (bAdd) {
    s_uiTotalMemory += uiMem;
    s_uiDeviceMemory += uiDevMem;
  } else {
    s_uiTotalMemory -= uiMem;
    s_uiDeviceMemory -= uiDevMem;
  }
}

bool CTexture::GenerateMips()
{
  ASSERT(m_uiFlags & RF_CANGENERATEMIPS);
  if (!(m_uiFlags & RF_CANGENERATEMIPS))
    return false;
  CScopeLock kLock(&CGraphics::Get()->m_Lock);
  CGraphics::Get()->m_pDeviceContext->GenerateMips(m_pTextureView);
  if (m_pStagingTexture) {
    CGraphics::Get()->m_pDeviceContext->CopyResource(m_pStagingTexture, m_pTexture);
    return true;
  }
  if (m_pSystemCopy && !TransferD3DToSystemCopy())
    return false;
  return true;
}

UINT CTexture::GetSize(UINT uiSubresource)
{
  ASSERT(uiSubresource < GetSubresources());
  int iBPP, iBlockSize, iBlockPitch;
  if (!GetFormatParams(m_eFormat, iBPP, iBlockSize))
    return 0;
  iBlockPitch = GetBlockPitch(m_iWidth, uiSubresource, iBPP, iBlockSize);
  return GetLevelSize(m_iHeight, uiSubresource, iBlockPitch, iBlockSize);
}

UINT CTexture::GetBlockPitch(UINT uiSubresource)
{
  ASSERT(uiSubresource < GetSubresources());
  int iBPP, iBlockSize, iBlockPitch;
  if (!GetFormatParams(m_eFormat, iBPP, iBlockSize))
    return 0;
  iBlockPitch = GetBlockPitch(m_iWidth, uiSubresource, iBPP, iBlockSize);
  return iBlockPitch;
}

uint8_t *CTexture::MapRect(UINT uiSubresource, UINT uiMapFlags, CRect<int> const &rcRect)
{
  ASSERT(uiSubresource < GetSubresources());
  int iBPP, iBlockSize, iBlockPitch;
  CRect<int> rcMap;
  UINT uiStart, uiEnd;

  if (!GetFormatParams(m_eFormat, iBPP, iBlockSize))
    return 0;
  iBlockPitch = GetBlockPitch(m_iWidth, uiSubresource, iBPP, iBlockSize);
  rcMap.Set(0, 0, (m_iWidth >> uiSubresource) - 1, (m_iHeight >> uiSubresource) - 1);
  rcMap = rcMap.GetIntersection(rcRect);
  ASSERT(!rcMap.IsEmpty());
  if (rcMap.IsEmpty())
    return 0;
  ASSERT(!(rcMap.m_vMin.x() % iBlockSize) && !(rcMap.m_vMin.y() % iBlockSize) && !(rcMap.m_vMax.x() % iBlockSize) && !(rcMap.m_vMax.y() % iBlockSize));
  uiStart = rcMap.m_vMin.x() * iBlockSize * iBPP / 8 + rcMap.m_vMin.y() * iBlockPitch / iBlockSize;
  uiEnd = (rcMap.m_vMax.x() + 1) * iBlockSize * iBPP / 8 + rcMap.m_vMax.y() * iBlockPitch / iBlockSize;
  return Map(uiSubresource, uiMapFlags, uiStart, uiEnd - uiStart);
}

bool CTexture::GetMappedBox(D3D11_BOX &kBox, UINT &uiRowSize, UINT &uiRowPitch, UINT &uiDepthPitch, bool bInBlocks)
{
  int iBPP, iBlockSize;
  if (!GetFormatParams(m_eFormat, iBPP, iBlockSize))
    return false;
  uiRowPitch = GetBlockPitch(m_iWidth, m_uiMappedSubresource, iBPP, iBlockSize);
  uiDepthPitch = 0;

  UINT uiOffs = m_uiMappedOffset - GetSubresourceOffset(m_uiMappedSubresource);
  kBox.top = uiOffs / uiRowPitch;
  kBox.left = (uiOffs % uiRowPitch) * 8 / iBPP;
  kBox.bottom = (uiOffs + m_uiMappedSize) / uiRowPitch;
  kBox.right = ((uiOffs + m_uiMappedSize) % uiRowPitch) * 8 / iBPP;
  if (kBox.right)
    kBox.bottom++;
  else 
    kBox.right = m_iWidth >> m_uiMappedSubresource;
  kBox.front = 0;
  kBox.back = 1;

  uiRowSize = ((kBox.right - kBox.left) * iBPP) / 8;
  ASSERT(!(((kBox.right - kBox.left) * iBPP) % 8));

  if (bInBlocks) {
    kBox.left /= iBlockSize;
    kBox.top /= iBlockSize;
    kBox.front /= iBlockSize;
    kBox.right /= iBlockSize;
    kBox.bottom /= iBlockSize;
    kBox.back /= iBlockSize;
  }

  ASSERT(kBox.left < kBox.right && kBox.top < kBox.bottom);
  ASSERT(kBox.right <= Util::Max<UINT>(1, (m_iWidth >> m_uiMappedSubresource)));
  ASSERT(kBox.bottom <= Util::Max<UINT>(1, (m_iHeight >> m_uiMappedSubresource)));

  return true;
}

bool CTexture::CreateSystemCopy(uint8_t *pContents)
{
  if (!CD3DResource::CreateSystemCopy(pContents))
    return false;
  if (!m_pSystemCopy || pContents)
    return true;

  return TransferD3DToSystemCopy();
}

bool CTexture::TransferD3DToSystemCopy()
{
  D3D11_USAGE eUsage;
  UINT uiCPUAccessFlags;
  int iLevel;
  CAutoReleasePtr<ID3D11Texture2D> pStaging;

  ASSERT(m_pSystemCopy);
  GetResourceParams(true, eUsage, uiCPUAccessFlags);
  pStaging.m_pPtr = CreateD3DTexture(m_iWidth, m_iHeight, m_eFormat, m_iMipLevels, 0, 0, eUsage, uiCPUAccessFlags, !!(m_uiFlags & RF_CANGENERATEMIPS));
  if (!pStaging)
    return false;

  CScopeLock kLock(&CGraphics::Get()->m_Lock);
  CGraphics::Get()->m_pDeviceContext->CopyResource(pStaging, m_pTexture);

  uint8_t *pCopy = m_pSystemCopy;
  for (iLevel = 0; iLevel < m_iMipLevels; iLevel++) {
    D3D11_MAPPED_SUBRESOURCE ms;
    HRESULT res;
    TResReporter kErrorReport(res);

    res = CGraphics::Get()->m_pDeviceContext->Map(pStaging, iLevel, D3D11_MAP_READ, 0, &ms);
    if (FAILED(res))
      return false;

    if (!CopyLevelData(m_iWidth, m_iHeight, m_eFormat, iLevel, (uint8_t *) ms.pData, ms.RowPitch, pCopy, 0))
      return false;

    CGraphics::Get()->m_pDeviceContext->Unmap(pStaging, iLevel);

    pCopy += GetSize(iLevel);
  }

  return true;
}

ID3D11Texture2D *CTexture::CreateD3DTexture(int iWidth, int iHeight, DXGI_FORMAT eFormat, int iMipLevels, uint8_t *pData, int iRowPitch, D3D11_USAGE eUsage, UINT uiCPUAccessFlags, bool bCanGenerateMips)
{
  int iBPP, iBlockSize, i;
  bool bRes, bAutoGenMips;

  // Do not allow explicit pitch combined with data for multiple mip levels
  ASSERT(!(iMipLevels > 1 && iRowPitch && pData));
  if (iMipLevels > 1 && iRowPitch && pData)
    return 0;

  bRes = GetFormatParams(eFormat, iBPP, iBlockSize);
  ASSERT(bRes);
  if (!bRes)
    return 0;
  ASSERT(!(iWidth % iBlockSize));
  ASSERT(!(iHeight % iBlockSize));
  
  if (!iRowPitch)
    iRowPitch = GetBlockPitch(iWidth, 0, iBPP, iBlockSize);

  if (iMipLevels < 0) {
    bAutoGenMips = true;
    iMipLevels = -iMipLevels;
  } else
    bAutoGenMips = false;

  bCanGenerateMips |= bAutoGenMips;

  D3D11_TEXTURE2D_DESC td;
  td.Width = iWidth;
  td.Height = iHeight;
  td.MipLevels = iMipLevels;
  td.ArraySize = 1;
  td.Format = eFormat;
  td.SampleDesc.Count = 1;
  td.SampleDesc.Quality = 0;
  td.Usage = eUsage;
  if (eUsage != D3D11_USAGE_STAGING) {
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    if (bCanGenerateMips)
      td.BindFlags |= D3D11_BIND_RENDER_TARGET;
  }
  else
    td.BindFlags = 0;
  td.CPUAccessFlags = uiCPUAccessFlags;
  td.MiscFlags = (bCanGenerateMips && eUsage != D3D11_USAGE_STAGING) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

  D3D11_SUBRESOURCE_DATA sd[32];
  ASSERT(iMipLevels < ARRSIZE(sd));
  D3D11_SUBRESOURCE_DATA *pSubResData;
  if (eUsage != D3D11_USAGE_STAGING && pData) {
    for (i = 0; i < iMipLevels; i++) {
      sd[i].pSysMem = pData;
      if (pData) {
        if (bAutoGenMips) 
          sd[i].SysMemPitch = iRowPitch; // Can't pass partially empty initialization data. Filling subsequent levels with garbage from the same buffer, it will be overwritten by GenerateMips anyway. 
        else {
          sd[i].SysMemPitch = GetBlockPitch(iWidth, i, iBPP, iBlockSize);
          pData += GetLevelSize(iHeight, i, sd[i].SysMemPitch, iBlockSize);
        }
      } else 
        sd[i].SysMemPitch = 0;
      sd[i].SysMemSlicePitch = 0;
    }
    pSubResData = sd;
  } else
    pSubResData = 0;

  HRESULT res;
  TResReporter kErrorReport(res);
  ID3D11Texture2D *pRes = 0;
  res = CGraphics::Get()->m_pDevice->CreateTexture2D(&td, pSubResData, &pRes);

  return pRes;
}

bool CTexture::GetFormatParams(DXGI_FORMAT eFormat, int &iBPP, int &iBlockSize)
{
  switch (eFormat) {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:      
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:        
    case DXGI_FORMAT_R32G32B32A32_SINT:
      iBPP = 4 * 32;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R32G32B32_TYPELESS:
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
      iBPP = 3 * 32;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R16G16B16A16_TYPELESS:   
    case DXGI_FORMAT_R16G16B16A16_FLOAT:         
    case DXGI_FORMAT_R16G16B16A16_UNORM:         
    case DXGI_FORMAT_R16G16B16A16_UINT:          
    case DXGI_FORMAT_R16G16B16A16_SNORM:         
    case DXGI_FORMAT_R16G16B16A16_SINT:
      iBPP = 4 * 16;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R32G32_TYPELESS:
    case DXGI_FORMAT_R32G32_FLOAT:               
    case DXGI_FORMAT_R32G32_UINT:                
    case DXGI_FORMAT_R32G32_SINT:
      iBPP = 2 * 32;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:       
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
      iBPP = 32 + 8 + 24;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    case DXGI_FORMAT_R10G10B10A2_UNORM:      
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
      iBPP = 32;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8B8A8_UNORM:             
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:        
    case DXGI_FORMAT_R8G8B8A8_UINT:              
    case DXGI_FORMAT_R8G8B8A8_SNORM:             
    case DXGI_FORMAT_R8G8B8A8_SINT:
      iBPP = 4 * 8;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R16G16_TYPELESS:            
    case DXGI_FORMAT_R16G16_FLOAT:               
    case DXGI_FORMAT_R16G16_UNORM:               
    case DXGI_FORMAT_R16G16_UINT:                
    case DXGI_FORMAT_R16G16_SNORM:               
    case DXGI_FORMAT_R16G16_SINT:
      iBPP = 2 * 16;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R32_TYPELESS:               
    case DXGI_FORMAT_D32_FLOAT:                  
    case DXGI_FORMAT_R32_FLOAT:                  
    case DXGI_FORMAT_R32_UINT:                   
    case DXGI_FORMAT_R32_SINT:
      iBPP = 32;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R24G8_TYPELESS:             
    case DXGI_FORMAT_D24_UNORM_S8_UINT:          
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:      
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
      iBPP = 24 + 8;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R8G8_TYPELESS:              
    case DXGI_FORMAT_R8G8_UNORM:                 
    case DXGI_FORMAT_R8G8_UINT:                  
    case DXGI_FORMAT_R8G8_SNORM:                 
    case DXGI_FORMAT_R8G8_SINT:
      iBPP = 8 + 8;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R16_TYPELESS:               
    case DXGI_FORMAT_R16_FLOAT:                  
    case DXGI_FORMAT_D16_UNORM:                  
    case DXGI_FORMAT_R16_UNORM:                  
    case DXGI_FORMAT_R16_UINT:                   
    case DXGI_FORMAT_R16_SNORM:                  
    case DXGI_FORMAT_R16_SINT:
      iBPP = 16;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R8_TYPELESS:                
    case DXGI_FORMAT_R8_UNORM:                   
    case DXGI_FORMAT_R8_UINT:                    
    case DXGI_FORMAT_R8_SNORM:                   
    case DXGI_FORMAT_R8_SINT:                    
    case DXGI_FORMAT_A8_UNORM:
      iBPP = 8;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R1_UNORM:
      iBPP = 1;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
      iBPP = 32;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_R8G8_B8G8_UNORM:            
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
      ASSERT(0);
      return false;
    case DXGI_FORMAT_BC1_TYPELESS:               
    case DXGI_FORMAT_BC1_UNORM:                  
    case DXGI_FORMAT_BC1_UNORM_SRGB:
      ASSERT(0);
      return false;
    case DXGI_FORMAT_BC2_TYPELESS:               
    case DXGI_FORMAT_BC2_UNORM:                  
    case DXGI_FORMAT_BC2_UNORM_SRGB:
      ASSERT(0);
      return false;
    case DXGI_FORMAT_BC3_TYPELESS:               
    case DXGI_FORMAT_BC3_UNORM:                  
    case DXGI_FORMAT_BC3_UNORM_SRGB:
      ASSERT(0);
      return false;
    case DXGI_FORMAT_BC4_TYPELESS:               
    case DXGI_FORMAT_BC4_UNORM:                  
    case DXGI_FORMAT_BC4_SNORM:
      ASSERT(0);
      return false;
    case DXGI_FORMAT_BC5_TYPELESS:               
    case DXGI_FORMAT_BC5_UNORM:                  
    case DXGI_FORMAT_BC5_SNORM:
      ASSERT(0);
      return false;
    case DXGI_FORMAT_B5G6R5_UNORM:               
    case DXGI_FORMAT_B5G5R5A1_UNORM:
      iBPP = 16;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_B8G8R8A8_UNORM:             
    case DXGI_FORMAT_B8G8R8X8_UNORM:             
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: 
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:          
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:        
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:          
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:       
      iBPP = 32;
      iBlockSize = 1;
      return true;
    case DXGI_FORMAT_BC6H_TYPELESS:              
    case DXGI_FORMAT_BC6H_UF16:                  
    case DXGI_FORMAT_BC6H_SF16:
      ASSERT(0);
      return false;
    case DXGI_FORMAT_BC7_TYPELESS:               
    case DXGI_FORMAT_BC7_UNORM:                  
    case DXGI_FORMAT_BC7_UNORM_SRGB:
      ASSERT(0);
      return false;
    default:
      ASSERT(!"Invalid pixel format");
      return false;
  };
}

int CTexture::GetBlockPitch(int iWidth, int iLevel, int iBPP, int iBlockSize)
{
  iWidth = Util::Max(1, iWidth >> iLevel);
  iWidth = Util::AlignToMultiple(iWidth, iBlockSize);
  int iBits = iWidth * iBlockSize * iBPP;
  return iBits / 8 + !!(iBits % 8);
}

int CTexture::GetLevelSize(int iHeight, int iLevel, int iBlockPitch, int iBlockSize)
{
  iHeight = Max(1, (iHeight >> iLevel));
  int iBlockHeight = iHeight / iBlockSize + !!(iHeight % iBlockSize);
  return iBlockPitch * iBlockHeight;
}

bool CTexture::CopyLevelData(int iWidth, int iHeight, DXGI_FORMAT eFormat, int iLevel, uint8_t *pSrcData, int iSrcBlockPitch, uint8_t *pDstData, int iDstBlockPitch)
{
  int iBPP, iBlockSize;
  int iRowSize, iWidthB, iHeightB;

  if (!GetFormatParams(eFormat, iBPP, iBlockSize) || !pSrcData || !pDstData) {
    ASSERT(0);
    return false;
  }

  if (!iSrcBlockPitch)
    iSrcBlockPitch = GetBlockPitch(iWidth, iLevel, iBPP, iBlockSize);

  if (!iDstBlockPitch)
    iDstBlockPitch = GetBlockPitch(iWidth, iLevel, iBPP, iBlockSize);

  iWidthB = Util::Max(1, iWidth >> iLevel);
  iWidthB = iWidthB / iBlockSize + !!(iWidthB % iBlockSize);

  iRowSize = iWidthB * iBPP * iBlockSize;
  iRowSize = iRowSize / 8 + !!(iRowSize % 8);
  ASSERT(iRowSize <= iSrcBlockPitch);
  ASSERT(iRowSize <= iDstBlockPitch);

  iHeightB = Util::Max(1, iHeight >> iLevel);
  iHeightB = iHeightB / iBlockSize + !!(iHeightB % iBlockSize);

  while (iHeightB) {
    memcpy(pDstData, pSrcData, iRowSize);
    pDstData += iDstBlockPitch;
    pSrcData += iSrcBlockPitch;
    iHeightB--;
  }

  return true;
}

// Texture pointer var

CVarRTTIRegisterer<CTexture *> g_RegVarTexturePtr;