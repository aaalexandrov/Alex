#ifndef __TEXTURE_H
#define __TEXTURE_H

#include <D3D11.h>
#include "Buffer.h"

// CSampler -------------------------------------------------------------------

class CSampler {
public:
  struct TDesc {
    union {
      UINT uiMode;
      struct {
        UINT bMinLinear : 1;
        UINT bMagLinear : 1;
        UINT bMipLinear : 1;
        UINT bAnisotropic : 1;
        UINT uiAddressU : 3;
        UINT uiAddressV : 3;
        UINT uiAddressW : 3;
        UINT uiMaxAnisotropy : 8;
        UINT uiUnused : 8;
      };
    };
    float fMipLODBias;

    inline TDesc(bool bMinLin = true, bool bMagLin = true, bool bMipLin = true, bool bAniso = false, 
                 UINT uiAddrU = D3D11_TEXTURE_ADDRESS_WRAP, UINT uiAddrV = D3D11_TEXTURE_ADDRESS_WRAP, 
                 UINT uiAddrW = D3D11_TEXTURE_ADDRESS_WRAP, UINT uiMaxAniso = 16, float fMipBias = 0.0f)
    {
      bMinLinear = bMinLin;
      bMagLinear = bMagLin;
      bMipLinear = bMipLin;
      bAnisotropic = bAniso;
      uiAddressU = uiAddrU;
      uiAddressV = uiAddrV;
      uiAddressW = uiAddrW;
      uiMaxAnisotropy = uiMaxAniso;
      uiUnused = 0;
      fMipLODBias = fMipBias;
    }

    inline bool operator ==(const TDesc &kDesc) const { return uiMode == kDesc.uiMode && fMipLODBias == kDesc.fMipLODBias; }
    inline size_t Hash() const { return uiMode | *(UINT *) &fMipLODBias; }
  };
public:
  ID3D11SamplerState *m_pSampler;
  TDesc               m_Desc;

  CSampler()  { m_pSampler = 0; }
  ~CSampler() { Done();         }

  bool Init(const TDesc &kDesc);
  void Done() { SAFE_RELEASE(m_pSampler); }

  bool IsValid() { return !!m_pSampler; }

  static inline bool Eq(const CSampler *pS1, const CSampler *pS2)   { return pS1->m_Desc == pS2->m_Desc; }
  static inline bool Eq(const TDesc &kDesc, const CSampler *pS)     { return kDesc == pS->m_Desc;        }
  static inline size_t Hash(const TDesc &kDesc)                     { return kDesc.Hash();               }
  static inline size_t Hash(CSampler *pS)                           { return pS->m_Desc.Hash();          }
};

IMPLEMENT_BASE_SET(CSampler *)

// CTexture -------------------------------------------------------------------

class CTexture: public CD3DResource {
	DEFRTTI(CTexture, CD3DResource, true)
public:
  static UINT s_uiTotalMemory, s_uiDeviceMemory;

  ID3D11Texture2D          *m_pTexture, 
                           *m_pStagingTexture;
  ID3D11ShaderResourceView *m_pTextureView;
  DXGI_FORMAT               m_eFormat;
  int                       m_iWidth, 
                            m_iHeight, 
                            m_iMipLevels;


  CTexture();
  virtual ~CTexture() { Done(); }

  virtual bool Init(CStrAny sFilename, int iMipLevels, UINT uiFlags);
  virtual bool Init(int iWidth, int iHeight, DXGI_FORMAT eFormat, int iMipLevels, BYTE *pData, int iRowPitch, UINT uiFlags);
  virtual void Done();
  virtual void RecordMemory(bool bAdd);

  virtual bool GenerateMips();

  virtual UINT GetSubresources()               { return m_iMipLevels; }
  virtual UINT GetSize(UINT uiSubresource);

  virtual UINT GetBlockPitch(UINT uiSubresource);

  virtual BYTE *MapRect(UINT uiSubresource, UINT uiMapFlags, CRect<int> const &rcRect);

  virtual ID3D11Resource *GetD3DResource()     { return m_pTexture; }
  virtual ID3D11Resource *GetStagingResource() { return m_pStagingTexture; }
  virtual bool GetMappedBox(D3D11_BOX &kBox, UINT &uiRowSize, UINT &uiRowPitch, UINT &uiDepthPitch, bool bInBlocks); 

  virtual bool IsValid()                       { return !!m_pTextureView; }

  virtual bool CreateSystemCopy(BYTE *pContents);
  virtual bool TransferD3DToSystemCopy();

  static ID3D11Texture2D *CreateD3DTexture(int iWidth, int iHeight, DXGI_FORMAT eFormat, int iMipLevels, BYTE *pData, int iRowPitch, D3D11_USAGE eUsage, UINT uiCPUAccessFlags, bool bCanGenerateMips);

  static bool GetFormatParams(DXGI_FORMAT eFormat, int &iBPP, int &iBlockSize);
  static inline int GetBlockPitch(int iWidth, int iLevel, int iBPP, int iBlockSize);
  static inline int GetLevelSize(int iHeight, int iLevel, int iBlockPitch, int iBlockSize);

  static bool CopyLevelData(int iWidth, int iHeight, DXGI_FORMAT eFormat, int iLevel, BYTE *pSrcData, int iSrcBlockPitch, BYTE *pDstData, int iDstBlockPitch);
};

IMPLEMENT_BASE_SET(CTexture *)


#endif