#include "stdafx.h"
#include <Windows.h>
#include <crtdbg.h>

#include "Graphics.h"
#include "Shader.h"
#include "VarUtil.h"
#include "File.h"
#include "Model.h"
#include "Matrix.h"
#include "Camera.h"
#include "Input.h"
#include "Timing.h"
#include "Font.h"
#include "Random.h"
#include "FrameSorter.h"

#include "TerrainCamera.h"
#include "Terrain.h"

const char g_chClassName[] = "TerrainWindClass";
HWND g_hWnd = 0;
HINSTANCE g_hInstance = 0;

CGraphics  *g_pGraphics = 0;
CTechnique *g_pTech = 0;
CSampler   *g_pSamplerDiff = 0;
CSampler   *g_pSamplerNorm = 0;
CCamera    *g_pCamera = 0;
CFreeCamera *g_pFreeCamera = 0;
CFont      *g_pFont = 0;
RECT        g_rcPrevWindow;
CTerrain   *g_pTerrain = 0;

CSmartPtr<CTexture> g_pTexture, g_pTexture1, g_pTextureNorm;
CSmartPtr<CModel> g_pModel;
CSmartPtr<CMaterial> g_pMaterial, g_pMaterial1;

bool g_bUpdateLOD = true;

void AddText()
{
  static int iFrame = 0;
  static CTime kLastTimes[256];
  CTime kNow = CTime::GetCurrent();

  float fFPS = ARRSIZE(kLastTimes) / (kNow - kLastTimes[iFrame]).Seconds(true);
  kLastTimes[iFrame] = kNow;
  iFrame = (iFrame + 1) % ARRSIZE(kLastTimes);

  char chBuf[256];
  sprintf(chBuf, "FPS: %03.2f", fFPS);

  CRect<> rcView;
  float fNear, fFar;
  CGraphics::Get()->GetViewport(rcView, fNear, fFar);
  g_pFont->AddStr(CStrPart(chBuf), CVector<2>::Get(10, rcView.GetHeight() - 8));

  sprintf(chBuf, "DP: %d, Tri: %d, Vert: %d", CGraphics::Get()->m_uiDrawPrimitiveCount, CGraphics::Get()->m_uiTriangleCount, CGraphics::Get()->m_uiVertexCount);
  g_pFont->AddStr(CStrPart(chBuf), CVector<2>::Get(180, rcView.GetHeight() - 8));

  sprintf(chBuf, "Buffers - Total: %dK, Dev: %dK, Texture - Total: %dK, Dev: %dK", 
          CD3DBuffer::s_uiTotalMemory / 1024, CD3DBuffer::s_uiDeviceMemory / 1024, CTexture::s_uiTotalMemory / 1024, CTexture::s_uiDeviceMemory / 1024);
  g_pFont->AddStr(CStrPart(chBuf), CVector<2>::Get(10, rcView.GetHeight() - 8 - g_pFont->m_iHeight));
}

struct TTerrainVertex {
  CVector<3> vPos;
  CVector<2> vTexCoord;
  float      fMaterialID;
};

bool InitModels()
{
  if (!g_pGraphics->LoadTechnique("Src/Plain.vfx"))
    return false;

  if (!g_pGraphics->LoadTechnique("Src/Terrain.vfx"))
    return false;

  if (!g_pGraphics->LoadTechnique("Src/TerrainLOD.vfx"))
    return false;

  g_pTech = g_pGraphics->GetTechnique("Terrain");
  if (!g_pTech)
    return false;

  g_pTexture = new CTexture();
//  if (!g_pTexture->Init("Data/earth.bmp"))
  if (!g_pTexture->Init("Data/Bumpy Textured Leaves.jpg", 0, CResource::RF_KEEPSYSTEMCOPY))
//  if (!g_pTexture->Init("Data/Fall Leaf Pile.jpg"))
    return false;

  g_pTexture1 = new CTexture();
  if (!g_pTexture1->Init("Data/Fall Leaf Pile.jpg", 0, CResource::RF_KEEPSYSTEMCOPY))
    return false;

  char chNormBuf[2] = { 0, 0 };
  g_pTextureNorm = new CTexture();
  if (!g_pTextureNorm->Init(1, 1, DXGI_FORMAT_R8G8_SNORM, 1, (BYTE *) chNormBuf, 2, 0))
    return false;

  g_pFont = new CFont("Arial", 20);
  if (!g_pFont->IsValid())
    return false;

  AddText();

  g_pSamplerDiff = g_pGraphics->GetSampler(CSampler::TDesc());
  if (!g_pSamplerDiff)
    return false;

  g_pSamplerNorm = g_pGraphics->GetSampler(CSampler::TDesc(true, true, true, false, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP));
  if (!g_pSamplerNorm)
    return false;

  TTerrainVertex arrVerts[] = {
    { { -1, -1, 0 }, { 0, 0 }, 0 },
    { {  1, -1, 0 }, { 1, 0 }, 0 },
    { {  1,  1, 0 }, { 1, 1 }, 0 },
    { { -1,  1, 0 }, { 0, 1 }, 0 },
  };

  WORD arrInds[] = {
    0, 1, 2, 
    0, 2, 3,
  };

  CSmartPtr<CGeometry> pGeom(new CGeometry());
  pGeom->Init(g_pTech->m_pInputDesc, CGeometry::PT_TRIANGLELIST, ARRSIZE(arrVerts), ARRSIZE(arrInds), arrVerts, arrInds);

  CSmartPtr<CMaterial> pMat(new CMaterial());
  pMat->Init(g_pTech, true, 0);

  g_pModel = new CModel();
  g_pModel->Init(pGeom, pMat, 0, false);

  g_pMaterial = pMat;

  static const CStrConst sg_mWorld("g_mWorld");
  static const CStrConst sg_mDiffTransform("g_mDiffTransform");
  static const CStrConst sg_mNormTransform("g_mNormTransform");
  static const CStrConst sg_cMaterialSpecular("g_cMaterialSpecular");
  static const CStrConst sg_cMaterialDiffuse("g_cMaterialDiffuse");
  static const CStrConst sg_cMaterialAmbient("g_cMaterialAmbient");
  static const CStrConst sg_fLODDistance("g_fLODDistance");
  static const CStrConst sg_txDiffuse("g_txDiffuse");
  static const CStrConst sg_txFar("g_txFar");
  static const CStrConst sg_txNormals("g_txNormals");
  static const CStrConst sg_sDiffuse("g_sDiffuse");
  static const CStrConst sg_sNormals("g_sNormals");
  static const CStrConst sg_fMaterialID("g_fMaterialID");

  CMatrixVar vMat(4, 4);
  CMatrix<4, 4> *pWorld = (CMatrix<4, 4> *) vMat.m_pVal;
  pWorld->SetDiagonal();

  g_pModel->SetVar(sg_mWorld, vMat);
  g_pModel->UpdateBound();

  CMatrixVar vTexTransform(3, 2);
  CMatrix<3, 2> *pTexTransform = (CMatrix<3, 2> *) vTexTransform.m_pVal;
  pTexTransform->SetDiagonal(0.5f);
  pMat->SetVar(sg_mDiffTransform, vTexTransform);
  g_pModel->SetVar(sg_mDiffTransform, vTexTransform);

  CTerrain::SetNormTransform(*pTexTransform);
  g_pModel->SetVar(sg_mNormTransform, vTexTransform);

  CVar<CVector<4> > vVec4;
  vVec4.Val().Set(1, 1, 1, 20);
  pMat->SetVar(sg_cMaterialSpecular, vVec4);

  vVec4.Val().Set(1, 1, 1, 1);
  pMat->SetVar(sg_cMaterialDiffuse, vVec4);

  CVar<CVector<3> > vVec3;
  vVec3.Val().Set(1, 1, 1);
  pMat->SetVar(sg_cMaterialAmbient, vVec3);

  pMat->SetFloat(sg_fLODDistance, 300);

  CVar<CTexture *> vDiffVar;
  vDiffVar.Val() = g_pTexture;
//  vDiffVar.Val() = g_pFont->m_pTexture;
  pMat->SetVar(sg_txDiffuse, vDiffVar);

  CVar<CTexture *> vFarVar;
  vFarVar.Val() = g_pTexture;
  g_pModel->SetVar(sg_txFar, vFarVar);

  CVar<CTexture *> vNormVar;
  vNormVar.Val() = g_pTextureNorm;
  g_pModel->SetVar(sg_txNormals, vNormVar);

  CVar<CSampler *> vSampDiffVar;
  vSampDiffVar.Val() = g_pSamplerDiff;
  pMat->SetVar(sg_sDiffuse, vSampDiffVar);

  CVar<CSampler *> vSampNormVar;
  vSampNormVar.Val() = g_pSamplerNorm;
  pMat->SetVar(sg_sNormals, vSampNormVar);

  g_pModel->SetFloat(sg_fMaterialID, 0);

  g_pMaterial1 = new CMaterial();
  g_pMaterial1->Init(g_pMaterial->m_pTechnique, true, g_pMaterial->GetApplyVars(0));

  g_pMaterial1->SetVar(sg_txDiffuse, CVar<CTexture *>(g_pTexture1));

  return true;
}

bool InitTerrainHeightsSine()
{
  ASSERT(g_pTerrain->m_rcGrid.GetWidth() == g_pTerrain->m_rcGrid.GetHeight());
  int iSize = g_pTerrain->m_rcGrid.GetWidth() + 1;
  for (int y = 0; y < iSize; y++)
    for (int x = 0; x < iSize; x++) {
      float fZ = sin(x / (float) (iSize - 1) * PI * 8) * cos(y / (float) (iSize - 1) * PI * 8) * 5;
      g_pTerrain->SetPoint(x, y, fZ, fZ > 0);
    }
  return true;
}

bool InitTerrainHeightsNoise()
{
  ASSERT(g_pTerrain->m_rcGrid.GetWidth() == g_pTerrain->m_rcGrid.GetHeight());
  int iSize = g_pTerrain->m_rcGrid.GetWidth() + 1;
  CNoise2D kNoise;
  float fWeights[] = {0, 0, 0.5, 1, 1, 1, 2, 1, 0, 0, -1};
  kNoise.Init(fWeights, true);

  for (int y = 0; y < iSize; y++)
    for (int x = 0; x < iSize; x++) {
      float fZ = kNoise.Get((float) x + 1000, (float) y + 1000);
      ASSERT(fZ >= 0 && fZ <= 1);
      fZ = Util::Lerp(-25.0f, 25.0f, fZ);
      g_pTerrain->SetPoint(x, y, fZ, fZ > 0);
    }
  return true;
}

bool InitTerrain(int iTerrainSize)
{
  int iSize;
  CFileBase *pFile;
  CRect<int> rcGrid;
  float fGrid2World;

  static CStrConst sTerFile("Data/Terrain.ter");

  if (iTerrainSize > 0)
    iSize = (CTerrain::PATCH_SIZE - 1) * iTerrainSize + 1;
  else
    iSize = (CTerrain::PATCH_SIZE - 1) * 16 + 1;

  pFile = CFileSystem::Get()->OpenFile(sTerFile, CFileBase::FOF_READ);
  if (!pFile || pFile->Read(rcGrid) || pFile->Read(fGrid2World) ||
      (iTerrainSize > 0 && (rcGrid.GetWidth() != iSize - 1 || rcGrid.GetHeight() != iSize - 1))) {
    rcGrid.Set(0, 0, iSize - 1, iSize - 1);
    fGrid2World = 1;
    SAFE_DELETE(pFile);
  } else
    pFile->SetPos(0);

  g_pTerrain = new CTerrain(rcGrid.GetWidth() + 1, rcGrid.GetHeight() + 1, fGrid2World, g_pMaterial, "Data/");
  g_pTerrain->AddMaterial(g_pMaterial1);

  g_pTerrain->SetCamera(g_pFreeCamera->m_pCamera);

  if (!pFile || !g_pTerrain->Load(0)) {
    delete pFile;

    if (!InitTerrainHeightsNoise())
      return false;

    if (!g_pTerrain->InitPatches())
      return false;

    pFile = CFileSystem::Get()->OpenFile(sTerFile, CFileBase::FOF_READ | CFileBase::FOF_WRITE | CFileBase::FOF_CREATE | CFileBase::FOF_TRUNCATE);
    if (!pFile)
      return false;
    if (!g_pTerrain->Save(0)) {
      delete pFile;
      return false;
    }
  }

  delete pFile;

  return true;
}

bool InitCameraParams()
{
  CRect<> rcViewport;
  float fMinZ, fMaxZ;

  g_pGraphics->GetViewport(rcViewport, fMinZ, fMaxZ);

  bool bRes = g_pCamera->Init(rcViewport.GetWidth() / rcViewport.GetHeight(), PI / 2, 0.2f, 2000, g_pCamera->m_XForm.m_pMatrix);
  return bRes;
}

bool InitScene()
{
  static CStrConst sg_vLightDirection("g_vLightDirection");
  static CStrConst sg_cLightSpecular("g_cLightSpecular");
  static CStrConst sg_cLightDiffuse("g_cLightDiffuse");
  static CStrConst sg_cLightAmbient("g_cLightAmbient");

  g_pCamera = new CCamera();
  g_pCamera->m_XForm.SetTranslation(CVector<3>::Get(0, 0, 10));
  CMatrix<3, 3> mRot, mRot1, mRot2;
  mRot.SetRotation(0, PI / 2);
  mRot1.SetRotation(2, 3 * PI / 4);
  mRot2.SetMul(mRot, mRot1);
  g_pCamera->m_XForm.SetRotation(mRot2);
  InitCameraParams();
  g_pCamera->SetVars();

  g_pGraphics->SetCamera(g_pCamera);

  CVar<CVector<3> > vVec3;
  vVec3.Val().Set(1, 1, 1);
  vVec3.Val().Normalize();
  g_pGraphics->m_Globals.SetVar(sg_vLightDirection, vVec3);

  vVec3.Val().Set(0.3f, 0.3f, 0.3f);
  g_pGraphics->m_Globals.SetVar(sg_cLightSpecular, vVec3);

  g_pGraphics->m_Globals.SetVar(sg_cLightDiffuse, vVec3);

  g_pGraphics->m_Globals.SetVar(sg_cLightAmbient, vVec3);

  g_pFreeCamera = new CFreeCamera(g_pCamera, false);

  return true;
}

bool Init(char const *pCmdLine)
{
  if (!CTime::InitTiming())
    return false;
  new CInput(g_hWnd);
  new CFileSystem();

  g_pGraphics = new CGraphics();
  g_pGraphics->SetSorter(new CPrioritySorter<>("g_fMaterialID"));
  if (!g_pGraphics->Init(g_hWnd, false))
    return false;

  if (!InitScene())
    return false;

  if (!InitModels())
    return false;

  int iTerrainSize;
  CStrPart sSize, sCmdLine(pCmdLine);

  sSize = Parse::ReadInt(sCmdLine);
  if (!sSize || !Parse::Str2Int(iTerrainSize, sSize))
    iTerrainSize = -1;

  if (!InitTerrain(iTerrainSize))
    return false;

  return true;
}

void Done()
{
  SAFE_DELETE(g_pTerrain);
  SAFE_DELETE(g_pFreeCamera);
  SAFE_DELETE(g_pCamera);
  g_pModel = 0;
  g_pMaterial1 = 0;
  g_pMaterial = 0;
  SAFE_DELETE(g_pFont);
  g_pTextureNorm = 0;
  g_pTexture1 = 0;
  g_pTexture = 0;
  if (g_pGraphics)
    g_pGraphics->Done();
  delete g_pGraphics;
  delete CFileSystem::Get();
  delete CInput::Get();
}

void Render()
{
  g_pFreeCamera->Update(CTime::GetCurrentReal());

  CVector<4> vBlue = { 0, 0, 1, 1 };
  g_pGraphics->Clear(vBlue, 1, 0);
  g_pFont->ResetModel();

  AddText();

  if (g_pModel)
    g_pModel->Render();

  if (g_pTerrain) {
    if (g_bUpdateLOD)
      g_pTerrain->UpdateLOD();
    g_pTerrain->Render();
  }

  g_pFont->RenderModel();

  g_pGraphics->Present();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (CInput::Get()->InputWindowProc(hwnd, uMsg, wParam, lParam))
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  switch (uMsg) {
    case WM_CLOSE:
      DestroyWindow(hwnd);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_PAINT:
      ValidateRect(hwnd, 0);
      break;
    case WM_SYSKEYDOWN:
      if (wParam == VK_RETURN) {
        if (g_pGraphics->m_bFullscreen)
          g_pGraphics->Resize(g_rcPrevWindow.right - g_rcPrevWindow.left, g_rcPrevWindow.bottom - g_rcPrevWindow.top, false);
        else {
          GetClientRect(hwnd, &g_rcPrevWindow);
          g_pGraphics->Resize(1920, 1080, true);
        }
        InitCameraParams();
        break;
      }
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
    case WM_SIZE:
      if (g_pGraphics && g_pGraphics->Resize(LOWORD(lParam), HIWORD(lParam), g_pGraphics->m_bFullscreen))
        InitCameraParams();
      break;
    default:
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF /*| _CRTDBG_CHECK_CRT_DF*/);
#endif

  WNDCLASSEX wc;

  g_hInstance = hInstance;

  wc.cbSize = sizeof(wc);
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = WindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = 0;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = 0; //(HBRUSH) (COLOR_BACKGROUND + 1);
  wc.lpszMenuName = 0;
  wc.lpszClassName = g_chClassName;
  wc.hIconSm = 0;

  if (!RegisterClassEx(&wc)) {
    MessageBox(0, "RegisterClassEx failed", "Terrain Error", MB_OK | MB_ICONERROR);
    return 0;
  }

  g_hWnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW, g_chClassName, "Terrain", WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768, 0, 0, hInstance, 0);
  if (!g_hWnd) {
    MessageBox(0, "CreateWindowEx failed", "Terrain Error", MB_OK | MB_ICONERROR);
    return 0;
  }

  ShowWindow(g_hWnd, nCmdShow);
  UpdateWindow(g_hWnd);

  if (!Init(lpCmdLine)) {
    MessageBox(0, "Init failed", "Terrain Error", MB_OK | MB_ICONERROR);
    return 0;
  }

  MSG msg;

  bool bContinue = true;
  while (bContinue) {
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        bContinue = false;
        break;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    Render();
  }

  Done();

  return (int) msg.wParam;
}
