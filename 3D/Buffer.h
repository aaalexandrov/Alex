#ifndef __BUFFER_H
#define __BUFFER_H

#include "Base.h"

class CResource: public CObject {
	DEFRTTI(CResource, CObject, false)
  DEFREFCOUNT
public:
  enum EType {
    RT_INVALID = 0,
    RT_FIRSTTYPE,
    RT_INDEX = RT_FIRSTTYPE,
    RT_VERTEX,
    RT_SHADERCONSTANT,
    RT_TEXTURE2D,
    RT_LASTTYPE = RT_TEXTURE2D,
  };

  enum EFlags {
    RF_IMMUTABLE = 1,
    RF_DYNAMIC = 2,
    RF_KEEPSYSTEMCOPY = 4,
    RF_CANGENERATEMIPS = 8,
    RF_MAPPED = 16,
  };

  enum EMapFlags {
    RMF_NO_OVERWRITE = 1,
    RMF_SYSTEM_ONLY = 2,
    RMF_FORCE_PHYSICAL = 4,
  };
public:
  EType  m_eType;
  UINT   m_uiFlags;

  CResource()          { m_eType = RT_INVALID; }
  virtual ~CResource() { ASSERT(m_eType == RT_INVALID); }

  virtual bool     Init(EType eType, UINT uiFlags);
  virtual void     Done()  { m_eType = RT_INVALID; }
                   
  virtual UINT     GetSubresources() = 0;
  virtual UINT     GetSize(UINT uiSubresource) = 0;

  virtual uint8_t *Map(UINT uiSubresource = 0, UINT uiMapFlags = 0, UINT uiOffset = 0, UINT uiSize = 0) = 0;
  virtual void     Unmap(UINT uiSubresource = -1) = 0;

  virtual uint8_t *GetMappedPtr() = 0;
  virtual UINT     GetMemorySize(UINT *pDeviceMemory) = 0;
};

struct ID3D11Resource;
class CD3DResource: public CResource {
	DEFRTTI(CD3DResource, CResource, false)
public:
  uint8_t *m_pSystemCopy;
  UINT     m_uiMappedSubresource;
  UINT     m_uiMappedOffset, m_uiMappedSize;
  UINT     m_uiMapFlags;
  uint8_t *m_pMappedBuffer;

  CD3DResource();
  virtual ~CD3DResource() {}

  virtual bool Init(EType eType, UINT uiFlags);
  virtual void Done();

  virtual bool ShouldHaveSystemCopy();
  virtual bool CreateSystemCopy(uint8_t *pContents);
  virtual bool GetResourceParams(bool bStaging, D3D11_USAGE &eUsage, UINT &uiCPUAccessFlags);
  virtual UINT GetSubresourceOffset(UINT uiSubresource);

  virtual UINT GetSubresources() = 0;
  virtual UINT GetSize(UINT uiSubresource) = 0;

  virtual uint8_t *Map(UINT uiSubresource = 0, UINT uiMapFlags = 0, UINT uiOffset = 0, UINT uiSize = 0);
  virtual void     Unmap(UINT uiSubresource = -1);

  virtual uint8_t *GetMappedPtr();
  virtual UINT     GetMemorySize(UINT *pDeviceMemory);

  virtual ID3D11Resource *GetD3DResource() = 0;
  virtual ID3D11Resource *GetStagingResource() = 0;
  virtual bool GetMappedBox(D3D11_BOX &kBox, UINT &uiRowSize, UINT &uiRowPitch, UINT &uiDepthPitch, bool bInBlocks) = 0; 

  static  void CopyRegion(UINT uiRowSize, UINT uiRows, UINT uiSlices,
                          uint8_t *pDst, UINT uiDstRowPitch, UINT uiDstDepthPitch, 
                          uint8_t *pSrc, UINT uiSrcRowPitch, UINT uiSrcDepthPitch);
};

struct ID3D11Buffer;
class CD3DBuffer: public CD3DResource {
  DEFRTTI(CD3DBuffer, CD3DResource, true)
public:
  static UINT s_uiTotalMemory, s_uiDeviceMemory;

  ID3D11Buffer *m_pD3DBuffer, *m_pStagingBuffer;
  UINT m_uiBufferSize;

  CD3DBuffer();
  virtual ~CD3DBuffer() { Done(); }

  virtual bool Init(EType eType, UINT uiFlags, uint8_t *pContents, UINT uiSize);
  virtual void Done();

  virtual UINT GetSubresources()               { return 1;                }
  virtual UINT GetSize(UINT uiSubresource)     { ASSERT(!uiSubresource); return m_uiBufferSize; }

  virtual ID3D11Resource *GetD3DResource()     { return m_pD3DBuffer;     }
  virtual ID3D11Resource *GetStagingResource() { return m_pStagingBuffer; }
  virtual bool GetMappedBox(D3D11_BOX &kBox, UINT &uiRowSize, UINT &uiRowPitch, UINT &uiDepthPitch, bool bInBlocks);
};

#endif