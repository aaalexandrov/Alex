#include "stdafx.h"
#include "Buffer.h"

#include "Graphics.h"

// CResource --------------------------------------------------------------------

IMPRTTI_NOCREATE(CResource, CObject)

bool CResource::Init(EType eType, UINT uiFlags)
{
  m_eType = eType;
  m_uiFlags = uiFlags;
  return true;
}

// CD3DResource ---------------------------------------------------------------

IMPRTTI_NOCREATE(CD3DResource, CResource)

CD3DResource::CD3DResource()
{
  m_eType = RT_INVALID;
  m_pSystemCopy = 0;
  m_uiMappedSubresource = 0;
  m_uiMappedOffset = 0;
  m_uiMappedSize = 0;
  m_uiMapFlags = 0;
  m_pMappedBuffer = 0;
}

bool CD3DResource::Init(EType eType, UINT uiFlags)
{
  ASSERT(m_eType == RT_INVALID && !m_pSystemCopy);

  if (!CResource::Init(eType, uiFlags))
    return false;

  return true;
}

void CD3DResource::Done()
{
  SAFE_DELETE(m_pSystemCopy);
  CResource::Done();
}

bool CD3DResource::ShouldHaveSystemCopy()
{
  return !!(m_uiFlags & RF_KEEPSYSTEMCOPY)/* && (m_uiFlags & (RF_IMMUTABLE | RF_DYNAMIC))*/;
}

bool CD3DResource::CreateSystemCopy(BYTE *pContents)
{
  int i, iSubResCount;
  UINT uiSize;

  if (!ShouldHaveSystemCopy())
    return true;

  iSubResCount = GetSubresources();
  uiSize = 0;
  for (i = 0; i < iSubResCount; i++)
    uiSize += GetSize(i);
  m_pSystemCopy = new BYTE[uiSize];
  if (pContents)
    memcpy(m_pSystemCopy, pContents, uiSize);

  return true;
}

bool CD3DResource::GetResourceParams(bool bStaging, D3D11_USAGE &eUsage, UINT &uiCPUAccessFlags)
{
  if (bStaging) {
    eUsage = D3D11_USAGE_STAGING;
    uiCPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
    return false;
    //return (m_uiFlags & RF_KEEPSYSTEMCOPY) && !(m_uiFlags & (RF_IMMUTABLE | RF_DYNAMIC));
  }
  
  if (m_uiFlags & RF_IMMUTABLE) {
    eUsage = D3D11_USAGE_IMMUTABLE;
    uiCPUAccessFlags = 0;
  } else
    if (m_uiFlags & RF_DYNAMIC) {
      eUsage = D3D11_USAGE_DYNAMIC;
      uiCPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    } else {
      eUsage = D3D11_USAGE_DEFAULT;
      uiCPUAccessFlags = 0;
    }

  return true;
}

UINT CD3DResource::GetSubresourceOffset(UINT uiSubresource)
{
  UINT uiSub, uiOffs = 0;
  for (uiSub = 0; uiSub < uiSubresource; uiSub++)
    uiOffs += GetSize(uiSub);
  return uiOffs;
}

BYTE *CD3DResource::Map(UINT uiSubresource, UINT uiMapFlags, UINT uiOffset, UINT uiSize)
{
  UINT uiSubresSize = GetSize(uiSubresource);
  if ((m_uiFlags & RF_MAPPED) || 
      !(m_uiFlags & RF_KEEPSYSTEMCOPY) &&
      ((uiMapFlags & RMF_SYSTEM_ONLY) || !(m_uiFlags & RF_DYNAMIC)) || 
      !(m_uiFlags & RF_DYNAMIC) && (uiMapFlags & RMF_FORCE_PHYSICAL) || 
      uiOffset >= uiSubresSize)
    return 0;
  if (!uiSize || uiSize > uiSubresSize - uiOffset)
    uiSize = uiSubresSize - uiOffset;
  if ((m_uiFlags & RF_KEEPSYSTEMCOPY) && !(uiMapFlags & RMF_FORCE_PHYSICAL) && m_pSystemCopy) {
    m_uiFlags |= RF_MAPPED;
    m_uiMappedSubresource = uiSubresource;
    m_uiMappedOffset = uiOffset + GetSubresourceOffset(uiSubresource);
    m_uiMappedSize = uiSize;
    m_uiMapFlags = uiMapFlags;
    m_pMappedBuffer = 0;
    return m_pSystemCopy + m_uiMappedOffset;
  }
  D3D11_MAP eMap;
  ID3D11Resource *pResource, *pStaging;
  if (!(uiMapFlags & RMF_FORCE_PHYSICAL) && (m_uiFlags & RF_KEEPSYSTEMCOPY) && (pStaging = GetStagingResource())) {
    pResource = pStaging;
    eMap = D3D11_MAP_READ_WRITE;
  } else {
    if (m_uiFlags & RF_DYNAMIC) {
      pResource = GetD3DResource();
      if ((uiMapFlags & RMF_NO_OVERWRITE) && m_eType != RT_SHADERCONSTANT)
        eMap = D3D11_MAP_WRITE_NO_OVERWRITE;
      else
        eMap = D3D11_MAP_WRITE_DISCARD;
    } else {
      ASSERT(0);
      return 0;
    }
  }

  D3D11_MAPPED_SUBRESOURCE ms;
  HRESULT res;
  TResReporter kErrorReport(res);
  { 
    CScopeLock kLock(&CGraphics::Get()->m_Lock);
    res = CGraphics::Get()->m_pDeviceContext->Map(pResource, uiSubresource, eMap, 0, &ms);
  }
  if (FAILED(res))
    return 0;

  m_uiFlags |= RF_MAPPED;
  m_uiMappedSubresource = uiSubresource;
  m_uiMappedOffset = uiOffset;
  m_uiMappedSize = uiSize;
  m_uiMapFlags = uiMapFlags;
  m_pMappedBuffer = (BYTE *) ms.pData;

  return m_pMappedBuffer + uiOffset;
}

void CD3DResource::Unmap(UINT uiSubresource)
{
  D3D11_BOX kBox;
  UINT uiRowSize, uiRowPitch, uiDepthPitch;

  if (uiSubresource == -1)
    uiSubresource = m_uiMappedSubresource;

  ASSERT(uiSubresource == m_uiMappedSubresource && (m_uiFlags & RF_MAPPED));

  if (!(m_uiFlags & RF_MAPPED) || uiSubresource != m_uiMappedSubresource)
    return;

  if ((m_uiFlags & RF_KEEPSYSTEMCOPY) && !(m_uiMapFlags & RMF_FORCE_PHYSICAL)) {
    ID3D11Resource *pStaging = GetStagingResource();
    if (pStaging) {
      {
        CScopeLock kLock(&CGraphics::Get()->m_Lock);
        CGraphics::Get()->m_pDeviceContext->Unmap(pStaging, uiSubresource);
        if (!(m_uiMapFlags & RMF_SYSTEM_ONLY)) {
          if (!m_uiMappedOffset && m_uiMappedSize == GetSize(m_uiMappedSubresource))
            CGraphics::Get()->m_pDeviceContext->CopyResource(GetD3DResource(), pStaging);
          else {
            if (!GetMappedBox(kBox, uiRowSize, uiRowPitch, uiDepthPitch, false))
              return;
            CGraphics::Get()->m_pDeviceContext->CopySubresourceRegion(GetD3DResource(), m_uiMappedSubresource, 
                                                                      kBox.left, kBox.top, kBox.front, 
                                                                      pStaging, m_uiMappedSubresource, &kBox);
          }
        }
      }
      m_uiFlags &= ~RF_MAPPED;
      m_uiMappedSubresource = 0;
      m_uiMappedOffset = 0;
      m_uiMappedSize = 0;
      m_uiMapFlags = 0;
      m_pMappedBuffer = 0;
      return;
    }
    ASSERT(m_pSystemCopy);
    if (!(m_uiMapFlags & RMF_SYSTEM_ONLY))
      if (m_uiFlags & RF_DYNAMIC) {
        m_uiFlags &= ~RF_MAPPED;
        if (!Map(m_uiMappedSubresource, m_uiMapFlags | RMF_FORCE_PHYSICAL, m_uiMappedOffset, m_uiMappedSize)) {
          ASSERT(0);
          return;
        }
//        memcpy(m_pMappedBuffer + m_uiMappedOffset, m_pSystemCopy + m_uiMappedOffset, m_uiMappedSize);
        if (!GetMappedBox(kBox, uiRowSize, uiRowPitch, uiDepthPitch, true))
          return;
        CopyRegion(uiRowSize, kBox.bottom - kBox.top, kBox.back - kBox.front, 
                   m_pMappedBuffer + m_uiMappedOffset, uiRowPitch, uiDepthPitch,
                   m_pSystemCopy + m_uiMappedOffset, uiRowPitch, uiDepthPitch);
        CScopeLock kLock(&CGraphics::Get()->m_Lock);
        CGraphics::Get()->m_pDeviceContext->Unmap(GetD3DResource(), uiSubresource);
      } else {
        if (!GetMappedBox(kBox, uiRowSize, uiRowPitch, uiDepthPitch, false))
          return;
        CScopeLock kLock(&CGraphics::Get()->m_Lock);
        CGraphics::Get()->m_pDeviceContext->UpdateSubresource(GetD3DResource(), m_uiMappedSubresource, &kBox, 
                                                              m_pSystemCopy + m_uiMappedOffset, uiRowPitch, uiDepthPitch);
      }
  } else
    if (m_pMappedBuffer) {
      CScopeLock kLock(&CGraphics::Get()->m_Lock);
      CGraphics::Get()->m_pDeviceContext->Unmap(GetD3DResource(), uiSubresource);
    }

  m_uiFlags &= ~RF_MAPPED;
  m_uiMappedSubresource = 0;
  m_uiMappedOffset = 0;
  m_uiMappedSize = 0;
  m_uiMapFlags = 0;
  m_pMappedBuffer = 0;
}

BYTE *CD3DResource::GetMappedPtr()
{
  if (!(m_uiFlags & RF_MAPPED))
    return 0;
  if (m_pMappedBuffer)
    return m_pMappedBuffer + m_uiMappedOffset;
  if (m_uiFlags & RF_KEEPSYSTEMCOPY)
    return m_pSystemCopy + m_uiMappedOffset;
  ASSERT(0);
  return 0;
}

UINT CD3DResource::GetMemorySize(UINT *pDeviceMemory)
{
  UINT uiMem, uiSize;
  UINT uiSubRes, uiDeviceBuffers, uiDevMem;

  if (!pDeviceMemory)
    pDeviceMemory = &uiDevMem;
  uiDeviceBuffers = !!GetD3DResource() + !!GetStagingResource();
  uiMem = 0;
  *pDeviceMemory = 0;
  for (uiSubRes = 0; uiSubRes < GetSubresources(); uiSubRes++) {
    uiSize = GetSize(uiSubRes);
    uiMem += (uiDeviceBuffers + !!m_pSystemCopy) * uiSize;
    *pDeviceMemory += uiDeviceBuffers * uiSize;
  }
  return uiMem;
}

void CD3DResource::CopyRegion(UINT uiRowSize, UINT uiRows, UINT uiSlices,
                              BYTE *pDst, UINT uiDstRowPitch, UINT uiDstDepthPitch, 
                              BYTE *pSrc, UINT uiSrcRowPitch, UINT uiSrcDepthPitch)
{
  UINT y, z;
  for (z = 0; z < uiSlices; z++) {
    for (y = 0; y < uiRows; y++) 
      memcpy(pDst + uiDstRowPitch * y, pSrc + uiSrcRowPitch * y, uiRowSize);
    pDst += uiDstDepthPitch;
    pSrc += uiSrcDepthPitch;
  }
}

// CD3DBuffer -----------------------------------------------------------------

IMPRTTI(CD3DBuffer, CD3DResource)

UINT CD3DBuffer::s_uiTotalMemory = 0, 
     CD3DBuffer::s_uiDeviceMemory = 0;

CD3DBuffer::CD3DBuffer()
{
  m_pD3DBuffer = 0;
  m_pStagingBuffer = 0;
}

bool CD3DBuffer::Init(EType eType, UINT uiFlags, BYTE *pContents, UINT uiSize)
{
  ASSERT(eType == RT_INDEX || eType == RT_VERTEX || eType == RT_SHADERCONSTANT);
  ASSERT(!m_pD3DBuffer && m_eType == RT_INVALID);

  if (m_eType == RT_SHADERCONSTANT) {
    uiSize = uiSize + (16 - uiSize % 16) * (uiSize % 16 > 0);
    uiSize = Min<UINT>(uiSize, D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT);
  }
  if (!CD3DResource::Init(eType, uiFlags)) 
    return false;

  m_uiBufferSize = uiSize;

  D3D11_BUFFER_DESC bd;
  bd.ByteWidth = m_uiBufferSize;
  static D3D11_BIND_FLAG arrBinds[] = { D3D11_BIND_INDEX_BUFFER, D3D11_BIND_VERTEX_BUFFER, D3D11_BIND_CONSTANT_BUFFER };
  ASSERT(m_eType - RT_FIRSTTYPE < ARRSIZE(arrBinds));
  bd.BindFlags = arrBinds[m_eType - RT_FIRSTTYPE];
  GetResourceParams(false, bd.Usage, bd.CPUAccessFlags);
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA srd;
  if (pContents) {
    srd.pSysMem = pContents;
    srd.SysMemPitch = 0;
    srd.SysMemSlicePitch = 0;
  }

  HRESULT res;
  TResReporter kErrorReport(res);
  {
    CScopeLock kLock(&CGraphics::Get()->m_Lock);
    res = CGraphics::Get()->m_pDevice->CreateBuffer(&bd, pContents ? &srd : 0, &m_pD3DBuffer);
    if (FAILED(res))
      return false;

    if (GetResourceParams(true, bd.Usage, bd.CPUAccessFlags)) {
      bd.BindFlags = 0;
      res = CGraphics::Get()->m_pDevice->CreateBuffer(&bd, pContents ? &srd : 0, &m_pStagingBuffer);
      if (FAILED(res))
        return false;
    }
  }

  if (!CreateSystemCopy(pContents))
    return false;

  CGraphics::Get()->m_Lock.Lock();
  UINT uiDevMem;
  s_uiTotalMemory += GetMemorySize(&uiDevMem);
  s_uiDeviceMemory += uiDevMem;
  CGraphics::Get()->m_Lock.Unlock();

  return true;
}

void CD3DBuffer::Done()
{
  CGraphics::Get()->m_Lock.Lock();

  UINT uiDevMem;
  s_uiTotalMemory -= GetMemorySize(&uiDevMem);
  s_uiDeviceMemory -= uiDevMem;

  SAFE_RELEASE(m_pD3DBuffer);
  SAFE_RELEASE(m_pStagingBuffer);
  CGraphics::Get()->m_Lock.Unlock();
  CD3DResource::Done();
}

bool CD3DBuffer::GetMappedBox(D3D11_BOX &kBox, UINT &uiRowSize, UINT &uiRowPitch, UINT &uiDepthPitch, bool bInBlocks)
{
  UINT uiOffs = m_uiMappedOffset - GetSubresourceOffset(m_uiMappedSubresource);
  kBox.left = uiOffs;
  kBox.right = uiOffs + m_uiMappedSize;
  kBox.top = kBox.front = 0;
  kBox.bottom = kBox.back = 1;

  uiRowSize = m_uiMappedSize;
  uiRowPitch = uiDepthPitch = 0;
  return true;
}
