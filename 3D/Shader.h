#ifndef __SHADER_H
#define __SHADER_H

#include "Graphics.h"
#include "Buffer.h"
#include "Matrix.h"
#include "State.h"
#include <D3D11Shader.h>

class CTechnique;
class CInputDesc {
  DEFREFCOUNT
public:
  enum EElemType {
    T_UNKNOWN = 0,
    T_UINT,
    T_INT,
    T_FLOAT,
    T_LAST_TYPE,
  };
  
  struct TInputElement {
    CStrConst m_sSemantic;
    EElemType m_Type;
    BYTE      m_btIndex, m_btElements;

    int GetSize();
    inline bool operator ==(const TInputElement &ie) const { return m_Type == ie.m_Type && m_btIndex == ie.m_btIndex && m_btElements == ie.m_btElements && m_sSemantic == ie.m_sSemantic; }
    inline bool operator !=(const TInputElement &ie) const { return !operator ==(ie); }

    inline size_t GetHash() const { return m_sSemantic.GetHash() + m_Type + ((size_t) m_btIndex << 8) + ((size_t) m_btElements << 16); }
  };
public:
  CArray<TInputElement> m_Elements;
  mutable size_t m_uiHash;

  CInputDesc()                     { m_uiHash = 0; }
  CInputDesc(const CInputDesc &id) { operator =(id); }
  ~CInputDesc()                    {}

  CInputDesc &operator =(const CInputDesc &id);
  bool operator ==(const CInputDesc &id) const;

  int AddElement(CStrConst sSem, EElemType kType, BYTE btSemIndex, BYTE btElements);

  int GetElementCount() { return m_Elements.m_iCount; }
  int GetElementOffset(int iElement);
  int GetSize() { return GetElementOffset(m_Elements.m_iCount); }

  TInputElement *GetElementInfo(CStrConst sSemantic, BYTE btSemIndex = 0, int *pInfoIndex = 0);
  
  bool IsSuperSet(CInputDesc *pInputDesc);

  ID3D11InputLayout *CreateLayout(CTechnique *pTech);

  size_t GetHash() const;
};

class CConstantTemplate: public CVarTemplate {
  DEFRTTI
public:
  CStrConst m_sName, m_sCacheName;
  int       m_iSize;

  CConstantTemplate();
  virtual ~CConstantTemplate();

  virtual bool Init(CTechnique *pTechnique, ID3D11ShaderReflectionConstantBuffer *pCBReflect);
  virtual void Done();
};

class CConstantCache {
public:
  CConstantTemplate *m_pTemplate;
  CVarObj           *m_pVars;
  bool               m_bOwnVars;
  CArray<BYTE, 0>    m_arrBuffer;
  UINT               m_uiFrameUpdated;

  CConstantCache();
  ~CConstantCache();

  bool Init(CConstantTemplate *pTemplate, CVarObj *pVars);
  void Done();
};

class CConstantBuffer {
public:
  CConstantTemplate     *m_pTemplate;
  CVarObj               *m_pVars;
  CSmartPtr<CD3DBuffer>  m_pBuffer;
  CConstantCache        *m_pLastCache;
  UINT                   m_uiFrameUpdated;

  CConstantBuffer();
  ~CConstantBuffer();

  bool Init(CConstantTemplate *pTemplate, CVarObj *pVars);
  void Done();

  bool IsValid()  { return m_pBuffer && m_pTemplate; }
  bool IsMapped() { return !!(m_pBuffer->m_uiFlags & CResource::RF_MAPPED); }

  bool Map();
  void Unmap();

  bool SetFrom(const CVarObj &kVars);
  bool SetFrom(CConstantCache *pCache);
  static bool SetMatrixVar(CMatrixVar *pDst, CMatrixVar const *pSrc, CStrPart sSuffix);
};

class CModel;
class CTechnique {
public:
  enum EShaderType {
    ST_VERTEX = 0,
    ST_PIXEL,
  };

  enum ECacheLocation {
    CL_TECHNIQUE,
    CL_MATERIAL,
    CL_MODEL,
  };

  enum EResourceType {
    RT_UNKNOWN = 0,
    RT_SAMPLER,
    RT_TEXTURE,
    RT_CONSTANT_BUFFER,
  };

  struct TResourceBind {
    CStrConst     m_sName;
    EResourceType m_eType;
    UINT          m_uiBindPoint, m_uiBindCount;
    EShaderType   m_eShader;
    union {
      CConstantBuffer *m_pBuffer;
    };
  };

  struct TInputDescLayout {
    CSmartPtr<CInputDesc>  m_pDesc;
    ID3D11InputLayout     *m_pLayout;

    TInputDescLayout(CInputDesc *pDesc, CTechnique *pTech);
    ~TInputDescLayout();

    static inline size_t Hash(const TInputDescLayout *pIDL) { return pIDL->m_pDesc->GetHash(); }
    static inline size_t Hash(const CInputDesc *pDesc)         { return pDesc->GetHash(); }

    static inline bool Eq(const TInputDescLayout *pIDL1, const TInputDescLayout *pIDL2) { return pIDL1 == pIDL2; }
    static inline bool Eq(const CInputDesc *pDesc, const TInputDescLayout *pIDL)        { return pDesc == pIDL->m_pDesc || *pDesc == *pIDL->m_pDesc; }
  };

  typedef CHash<TInputDescLayout *, CInputDesc *, TInputDescLayout, TInputDescLayout> THashInputLayout;
public:
  CStrConst m_sSrcFile, m_sVSEntry, m_sPSEntry, m_sName;
  ID3D10Blob *m_pVSBlob, *m_pPSBlob;
  ID3D11VertexShader *m_pVS;
  ID3D11PixelShader *m_pPS;
  CSmartPtr<CInputDesc> m_pInputDesc;
  CArray<CConstantTemplate *> m_arrConstantTemplates;
  CArray<CConstantBuffer *> m_arrConstantBuffers;
  CStateCache m_StateCache;
  UINT m_uiStateCacheFrameID[3];

  CArray<TResourceBind>     m_arrBinds;
  CVarObj *m_pDefStates, *m_pInitVars;
  THashInputLayout m_hashLayouts;

  CTechnique(CStrConst sSrcFile, CStrConst sVSEntry, CStrConst sPSEntry, CStrConst sName);
  CTechnique(CStr sVarFile);
  ~CTechnique();

  bool Init();
  bool InitShaders();
  bool InitInputLayout(ID3D11ShaderReflection *pVSReflect);
  bool InitConstantBuffers(ID3D11ShaderReflection *pSReflect);
  bool InitStates();
  bool InitBinds(EShaderType eShader, ID3D11ShaderReflection *pSReflect);
  void Done();

  bool IsValid();

  CConstantTemplate *GetConstantTemplate(CStrConst sName);
  CConstantBuffer *GetConstantBuffer(CStrConst sName);
  ID3D11InputLayout *GetInputLayout(CInputDesc *pDesc);
  bool SetMatrixVar(CVarObj *pVars, CStrConst sVarMatrix, CMatrixVar const *pMatVar);

  ECacheLocation GetStateLocation(CStateCache::EStateType eStateType);
  bool ApplyStateType(CStateCache::EStateType eStateType, CStateCache *pStateCache, CVarObj *pVars);

  bool ApplyState(CVarObj *pVars, CStateCache *pStateCache);
  bool ApplyConstants(CVarObj *pModelVars);
  bool Apply(CVarObj *pModelVars, CStateCache *pStateCache);

  static inline size_t Hash(const CTechnique *pTech) { return CStrConst::Hash(pTech->m_sName); }
  static inline size_t Hash(CStrConst sTech) { return CStrConst::Hash(sTech); }
  static inline bool Eq(const CTechnique *pTech1, const CTechnique *pTech2) { return pTech1 == pTech2; }
  static inline bool Eq(CStrConst sName, const CTechnique *pTech) { return sName == pTech->m_sName; }
};

/*
class CMatrixFamilyVar: public CMatrixVar {
  DEFRTTI_NOCREATE
public:
  struct TFamilyVar {
    CMatrixVar *m_pVar;
    CStrPart m_sSuffix;

    TFamilyVar() { m_pVar = 0; }
  };
public:
  TFamilyVar m_Family[4];

  CMatrixFamilyVar(int iRows, int iCols, UINT uiFlags, Num *pVal): CMatrixVar(iRows, iCols, uiFlags, pVal) {}
  bool AddFamilyVar(CMatrixVar *pVar, CStrPart sSuffix);

  virtual bool SetVar(const CBaseVar &vSrc);
  virtual CBaseVar *Clone() const;
};
*/
#endif