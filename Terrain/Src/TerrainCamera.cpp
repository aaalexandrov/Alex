#include "stdafx.h"
#include "TerrainCamera.h"
#include "Graphics.h"
#include "Terrain.h"
#include "Model.h"

extern CTerrain *g_pTerrain;
extern bool g_bUpdateLOD;

CFreeCamera::CFreeCamera(CCamera *pCam, bool bOwnCam)
{
  m_pCamera = pCam;
  m_bOwnCamera = bOwnCam;
  m_bFreeMode = false;
  m_bWalkMode = false;

  m_hashKeys.Add(TKeyInfo('W'));
  m_hashKeys.Add(TKeyInfo('S'));
  m_hashKeys.Add(TKeyInfo('A'));
  m_hashKeys.Add(TKeyInfo('D'));
  m_hashKeys.Add(TKeyInfo('Q'));
  m_hashKeys.Add(TKeyInfo('E'));
  m_hashKeys.Add(TKeyInfo('R'));
  m_hashKeys.Add(TKeyInfo('F'));
  m_hashKeys.Add(TKeyInfo('T'));
  m_hashKeys.Add(TKeyInfo('G'));
  m_hashKeys.Add(TKeyInfo('Z'));
  m_hashKeys.Add(TKeyInfo('C'));
  m_hashKeys.Add(TKeyInfo('X'));
  m_hashKeys.Add(TKeyInfo('V'));
  m_hashKeys.Add(TKeyInfo('M'));
  m_hashKeys.Add(TKeyInfo('L'));
  m_hashKeys.Add(TKeyInfo('J'));
  m_hashKeys.Add(TKeyInfo('K'));
  m_hashKeys.Add(TKeyInfo('I'));
  m_hashKeys.Add(TKeyInfo('O'));
  m_hashKeys.Add(TKeyInfo('P'));
  m_hashKeys.Add(TKeyInfo(VK_ADD));
  m_hashKeys.Add(TKeyInfo(VK_SUBTRACT));

  CInput::Get()->SetEventListener(this);

  m_vMouseLast = m_vMouseNow = CInput::Get()->GetMousePos();
}

CFreeCamera::~CFreeCamera()
{
  CInput::Get()->RemoveEventListener(this);
}

float CFreeCamera::GetKeyPressedSeconds(int iKey, CTime kNow)
{
  float fPressed;
  TKeyHash::TIter it;
  it = m_hashKeys.Find(iKey);
  ASSERT(!!it);
  if (!it)
    return 0;
  if ((*it).bPressed) {
    fPressed = kNow.SecondsSince((*it).kPressedTime, true);
    (*it).kPressedTime = kNow;
  } else {
    fPressed = (*it).kPressedTime.Seconds(true);
    (*it).kPressedTime.m_qwTime = 0;
  }
  return fPressed;
}

bool CFreeCamera::HasBeenPressed(int iKey)
{
  TKeyHash::TIter it;
  it = m_hashKeys.Find(iKey);
  ASSERT(!!it);
  if (!it)
    return false;
  bool bRes = (*it).bPressed && !(*it).bPressConsumed;
  (*it).bPressConsumed = true;
  return bRes;
}

void CFreeCamera::Update(CTime kTime)
{
  float fTime;
  CVector<3> vPos = m_pCamera->m_XForm.GetTranslation();
  CVector<3> vDelta, vRot;
  bool bPressed;
  bool bMatrixChanged = false;

  bPressed = HasBeenPressed('K');
  if (bPressed) {
    m_bWalkMode = !m_bWalkMode;
    bMatrixChanged = true;
  }

  float fTransScale = m_bWalkMode ? 5.0f : 45.0f;
  static const float fRotScale = PI / 2 * 1.5f;
  static const float fMouseScale = 0.005f;

  fTime = GetKeyPressedSeconds('W', kTime) - GetKeyPressedSeconds('S', kTime);
  vDelta.z() = fTime * fTransScale;

  fTime = GetKeyPressedSeconds('D', kTime) - GetKeyPressedSeconds('A', kTime);
  vDelta.x() = fTime * fTransScale;

  fTime = GetKeyPressedSeconds('R', kTime) - GetKeyPressedSeconds('F', kTime);
  vDelta.y() = fTime * fTransScale;

  fTime = GetKeyPressedSeconds('E', kTime) - GetKeyPressedSeconds('Q', kTime);
  vRot.y() = fTime * fRotScale;

  fTime = GetKeyPressedSeconds('G', kTime) - GetKeyPressedSeconds('T', kTime);
  vRot.x() = fTime * fRotScale;

  fTime = GetKeyPressedSeconds('C', kTime) - GetKeyPressedSeconds('Z', kTime);
  vRot.z() = fTime * fRotScale;

  CVector<2, int> vMouseDelta = m_vMouseNow - m_vMouseLast;
  vRot.y() += vMouseDelta.x() * fMouseScale;
  vRot.x() += vMouseDelta.y() * fMouseScale;

  m_vMouseLast = m_vMouseNow;

/*  static float fVertDelta = 0;
  fTime = GetKeyPressedSeconds(VK_ADD, kTime) - GetKeyPressedSeconds(VK_SUBTRACT, kTime);
  CTerrain::CPatch *pPatch = g_pTerrain->GetPatch(0, 0);
  fVertDelta += fTime * 500;
  float fGoal = Bound<float>(pPatch->m_uiActiveVertices + fVertDelta, (float) pPatch->m_uiMinVerticesToSet, (float) pPatch->m_arrMaterialModels[0]->m_pModel->m_pGeometry->GetVBVertexCount());
  pPatch->SetActiveVertices((UINT) fGoal);
  fVertDelta = fGoal - pPatch->m_uiActiveVertices;
*/

  bPressed = HasBeenPressed('V');
  if (bPressed) {
    static CStrAny sFillMode(ST_CONST, "FillMode");
    int iMode = D3D11_FILL_SOLID;
    CGraphics::Get()->m_Globals.GetInt(sFillMode, iMode);
    if (iMode == D3D11_FILL_SOLID)
      iMode = D3D11_FILL_WIREFRAME;
    else
      iMode = D3D11_FILL_SOLID;
    CGraphics::Get()->m_Globals.SetInt(sFillMode, iMode);
  }

  bPressed = HasBeenPressed('M');
  if (bPressed) 
    m_bFreeMode = !m_bFreeMode;

  bPressed = HasBeenPressed('K');
  if (bPressed)
    m_bWalkMode = !m_bWalkMode;

  bPressed = HasBeenPressed('L');
  if (bPressed)
    g_bUpdateLOD = !g_bUpdateLOD;

  bPressed = HasBeenPressed('J');
  if (bPressed) {
    for (int i = 0; i < g_pTerrain->m_arrPatches.m_iCount; i++)
      g_pTerrain->m_arrPatches[i]->m_pGeom->UpdateEdges();
  }

  bPressed = HasBeenPressed('I');
  if (bPressed)
    g_pTerrain->m_iForceModel = 0;

  bPressed = HasBeenPressed('O');
  if (bPressed)
    g_pTerrain->m_iForceModel = 1;

  bPressed = HasBeenPressed('P');
  if (bPressed)
    g_pTerrain->m_iForceModel = 2;

  bPressed = HasBeenPressed('X');

  if (bPressed) {
    if (m_bFreeMode) {
      m_pCamera->m_XForm.SetTranslation(CVector<3>::Get(0, 0, 10));
      CMatrix<3, 3> mRot;
      mRot.SetRotation(0, PI);
      m_pCamera->m_XForm.SetRotation(mRot);
    } else {
      m_pCamera->m_XForm.SetTranslation(CVector<3>::Get(0, 0, 10));
      CMatrix<3, 3> mRot, mRot1, mRot2;
      mRot.SetRotation(0, PI / 2);
      mRot1.SetRotation(2, 3 * PI / 4);
      mRot2.SetMul(mRot, mRot1);
      m_pCamera->m_XForm.SetRotation(mRot2);
    }
    bMatrixChanged = true;
  } else {
    CMatrix<> mTemp[2], mDelta;
    int iCur = 0;
    mTemp[iCur] = *m_pCamera->m_XForm.m_pMatrix;

    if (vDelta.x() || vDelta.y() || vDelta.z()) {
      if (m_bFreeMode) {
        mDelta.SetTranslation(vDelta);
        mTemp[!iCur].SetMul(mDelta, mTemp[iCur]);
        iCur = !iCur;
      } else {
        CVector<3> vForward, vLeft, vRotDelta;
        vForward.Set(mTemp[iCur].GetRow(2));
        vForward.z() = 0;
        vForward.Normalize();
        vLeft.x() = -vForward.y();
        vLeft.y() = vForward.x();
        vLeft.z() = 0;
        vRotDelta = vForward * vDelta.z();
        vRotDelta += vLeft * vDelta.x();
        vRotDelta.z() += vDelta.y();
        mTemp[iCur](3, 0) += vRotDelta.x();
        mTemp[iCur](3, 1) += vRotDelta.y();
        mTemp[iCur](3, 2) += vRotDelta.z();
      }
      bMatrixChanged = true;
    }

    if (vRot.x()) {
      mDelta.SetRotation(0, vRot.x());
      mTemp[!iCur].SetMul(mDelta, mTemp[iCur]);
      iCur = !iCur;
      bMatrixChanged = true;
    }

    if (vRot.y()) {
      if (m_bFreeMode) {
        mDelta.SetRotation(1, vRot.y());
        mTemp[!iCur].SetMul(mDelta, mTemp[iCur]);
      } else {
        mDelta.SetRotation(2, vRot.y());
        CVector<3> vTrans;
        CMatrix<4, 4> mTransInv, mTrans;
        vTrans.Set(mTemp[iCur].GetRow(3));
        mTransInv.SetTranslation(-vTrans);
        mTrans.SetTranslation(vTrans);
        CMatrix<4, 4> mTmp, mTmp1;
        mTmp.SetMul(mDelta, mTrans);
        mTmp1.SetMul(mTransInv, mTmp);
        mTemp[!iCur].SetMul(mTemp[iCur], mTmp1);
      }
      iCur = !iCur;
      bMatrixChanged = true;
    }

    if (vRot.z()) {
      mDelta.SetRotation(2, vRot.z());
      if (m_bFreeMode)
        mTemp[!iCur].SetMul(mDelta, mTemp[iCur]);
      else {
        mDelta.SetRotation(1, vRot.z());
        mTemp[!iCur].SetMul(mDelta, mTemp[iCur]);
      }
      iCur = !iCur;
      bMatrixChanged = true;
    }

    if (bMatrixChanged)
      *m_pCamera->m_XForm.m_pMatrix = mTemp[iCur];
  }

  if (bMatrixChanged && m_bWalkMode) {
    CVector<3> vTrans = m_pCamera->m_XForm.GetTranslation();
    vTrans.z() = g_pTerrain->GetHeight(vTrans.x(), vTrans.y()) + 2;
    m_pCamera->m_XForm.SetTranslation(vTrans);
  }

  if (bMatrixChanged)
    m_pCamera->Update();
}

bool CFreeCamera::OnInputEvent(CInput::CEvent *pEvent)
{
  TKeyHash::TIter it;
  switch (pEvent->m_eEvent) {
    case CInput::ET_KEYDOWN:
//      PRINT("Key down: %c\n", pEvent->m_iKey);
      it = m_hashKeys.Find(pEvent->m_iKey);
      if (!!it) {
        ASSERT(!(*it).bPressed);
        (*it).bPressed = true;
        (*it).bPressConsumed = false;
        (*it).kPressedTime = pEvent->m_Time - (*it).kPressedTime;
      }
      break;
    case CInput::ET_KEYUP:
//      PRINT("Key up: %c\n", pEvent->m_iKey);
      it = m_hashKeys.Find(pEvent->m_iKey);
      if (!!it) {
        ASSERT((*it).bPressed);
        (*it).bPressed = false;
        (*it).bPressConsumed = false;
        (*it).kPressedTime = pEvent->m_Time - (*it).kPressedTime;
      }
      break;
    case CInput::ET_MOUSEDOWN:
      if (pEvent->m_iKey == VK_RBUTTON) {
        CInput::Get()->SetMouseCapture(true);
        m_vMouseLast = pEvent->m_vPos - m_vMouseNow + m_vMouseLast;
        m_vMouseNow = pEvent->m_vPos;
      }
      break;
    case CInput::ET_MOUSEUP:
      if (pEvent->m_iKey == VK_RBUTTON) {
        CInput::Get()->SetMouseCapture(false);
        m_vMouseNow = pEvent->m_vPos;
      }
      break;
    case CInput::ET_MOUSEMOVE:
      if (CInput::Get()->GetMouseCapture()) 
        m_vMouseNow = pEvent->m_vPos;
      break;
  }
  return true;
}
