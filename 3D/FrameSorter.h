#ifndef __FRAMESORTER_H
#define __FRAMESORTER_H

#include "Model.h"
#include "Camera.h"

class CModel;
class CFrameSorter: public CObject {
  DEFRTTI
public:
  virtual ~CFrameSorter() {}

  virtual void Clear()             = 0;
  virtual bool Add(CModel *pModel) = 0;
  virtual void Sort()              = 0;
  virtual bool Render()            = 0;
  virtual bool Present();
};

class CSimpleSorter: public CFrameSorter {
  DEFRTTI
public:
  CSimpleSorter()          {}
  virtual ~CSimpleSorter() {}

  virtual void Clear()     {}
  virtual bool Add(CModel *pModel);
  virtual void Sort()      {}
  virtual bool Render()    { return true; }
  virtual bool Present()   { return true; }
};

struct TSortPriority {
  CModel *m_pModel;
  int     m_iPriority;

  inline void Init(CModel *pModel, CStrConst sPriorityVar);

  static inline bool Lt(TSortPriority const &kP0, TSortPriority const &kP1) { return kP0.m_iPriority < kP1.m_iPriority; }
};

struct TSortDistancePriority {
  CModel *m_pModel;
  float   m_fDistance;
  int     m_iPriority;

  inline void Init(CModel *pModel, CStrConst sPriorityVar);

  static inline bool Lt(TSortDistancePriority const &kDP0, TSortDistancePriority const &kDP1) { if (kDP0.m_fDistance == kDP1.m_fDistance) return kDP0.m_iPriority < kDP1.m_iPriority; return kDP0.m_fDistance < kDP1.m_fDistance; }
};

// More sort priority classes can be added in the above manner to implement application specific sorting logic. 
// Do not forget to add IMPRTTI_NOCREATE_T(CPrioritySorter<CustomPriority>, CFrameSorter) in a .cpp somewhere

template<class T = TSortDistancePriority>
class CPrioritySorter: public CFrameSorter {
  DEFRTTI
public:
  typedef T TModelData;

public:
  CArray<TModelData> m_arrModels;
  CStrConst m_sPriorityVar;

  CPrioritySorter(CStrConst sPriorityVar);
  virtual ~CPrioritySorter();

  virtual void Clear();
  virtual bool Add(CModel *pModel);
  virtual void Sort();
  virtual bool Render();
};

class CGroupSorter: public CFrameSorter {
  DEFRTTI
public:
  struct TGroupData {
    CFrameSorter *m_pSorter;
    TGroupData() { m_pSorter = 0; }
    ~TGroupData() { delete m_pSorter; }
  };

  CArray<TGroupData> m_arrGroups;

  CGroupSorter(int iGroups = 16);
  virtual ~CGroupSorter();

  virtual void SetGroupSorter(CFrameSorter *pSorter, int iGroup = -1);

  virtual void Clear();
  virtual bool Add(CModel *pModel);
  virtual void Sort();
  virtual bool Render();

  virtual int GetModelGroup(CModel *pModel);
};

// Implementation -------------------------------------------------------------

// TSortPriority

void TSortPriority::Init(CModel *pModel, CStrConst sPriorityVar)
{
  m_pModel = pModel;
  if (!pModel->GetApplyVars()->GetInt(sPriorityVar, m_iPriority))
    m_iPriority = 0;
}
 
// TSortDistancePriority 
void TSortDistancePriority::Init(CModel *pModel, CStrConst sPriorityVar)
{
  m_pModel = pModel;
  if (pModel->m_pBound && CGraphics::Get()->m_pCamera) {
    CPoint3D kCamPos;
    kCamPos.m_vPoint = CGraphics::Get()->m_pCamera->m_XForm.GetTranslation();
    m_fDistance = kCamPos.GetDistance(pModel->m_pBound);
  } else
    m_fDistance = Util::F_INFINITY;
  if (!pModel->GetApplyVars()->GetInt(sPriorityVar, m_iPriority))
    m_iPriority = 0;
}

// CPrioritySorter

template<class T>
CPrioritySorter<T>::CPrioritySorter(CStrConst sPriorityVar)
{
  m_sPriorityVar = sPriorityVar;
}

template<class T>
CPrioritySorter<T>::~CPrioritySorter()
{
}

template<class T>
void CPrioritySorter<T>::Clear()
{
  m_arrModels.SetCount(0);
}

template<class T>
bool CPrioritySorter<T>::Add(CModel *pModel)
{
  m_arrModels.SetCount(m_arrModels.m_iCount + 1);
  m_arrModels[m_arrModels.m_iCount - 1].Init(pModel, m_sPriorityVar);
  return true;
}

template<class T>
void CPrioritySorter<T>::Sort()
{
  Util::QSort<CArray<TModelData>, TModelData, TModelData>(m_arrModels, m_arrModels.m_iCount);
}

template<class T>
bool CPrioritySorter<T>::Render()
{
  int i;
  for (i = 0; i < m_arrModels.m_iCount; i++)
    m_arrModels[i].m_pModel->DoRender();
  return true;
}

#endif