#include "stdafx.h"
#include "FrameSorter.h"
#include "Graphics.h"

// CFrameSorter ---------------------------------------------------------------
CRTTIRegisterer<CFrameSorter> g_RegFrameSorter;

bool CFrameSorter::Present()
{
  bool bRes;
  Sort();
  bRes = Render();
  Clear();
  return bRes;
}

// CSimpleSorter --------------------------------------------------------------
CRTTIRegisterer<CSimpleSorter> g_RegSimpleSorter;

bool CSimpleSorter::Add(CModel *pModel)
{
  bool bRes = pModel->DoRender();
  return bRes;
}

// CPrioritySorter ------------------------------------------------------------
CRTTIRegisterer<CPrioritySorter<TSortPriority> > g_RegPrioritySorterSortPriority;
CRTTIRegisterer<CPrioritySorter<TSortDistancePriority> > g_RegPrioritySorterSortDistancePriority;


// CGroupSorter ---------------------------------------------------------------
CRTTIRegisterer<CGroupSorter> g_RegGroupSorter;

CGroupSorter::CGroupSorter(int iGroups): m_arrGroups(iGroups)
{
}

CGroupSorter::~CGroupSorter()
{
}

void CGroupSorter::SetGroupSorter(CFrameSorter *pSorter, int iGroup)
{
  if (iGroup < 0)
    iGroup = m_arrGroups.m_iCount;

  if (iGroup >= m_arrGroups.m_iCount)
    m_arrGroups.SetCount(iGroup + 1);

  delete m_arrGroups[iGroup].m_pSorter;

  m_arrGroups[iGroup].m_pSorter = pSorter;
}

void CGroupSorter::Clear()
{
  int i;
  for (i = 0; i < m_arrGroups.m_iCount; i++)
    if (m_arrGroups[i].m_pSorter)
      m_arrGroups[i].m_pSorter->Clear();
}

bool CGroupSorter::Add(CModel *pModel)
{
  bool bRes;
  int iGroup = GetModelGroup(pModel);
  if (iGroup >= 0 && iGroup < m_arrGroups.m_iCount && m_arrGroups[iGroup].m_pSorter)
    bRes = m_arrGroups[iGroup].m_pSorter->Add(pModel);
  else {
    ASSERT(!"Trying to render object with invalid rendering group");
    bRes = false;
  }
  return bRes;
}

void CGroupSorter::Sort()
{
  int i;
  for (i = 0; i < m_arrGroups.m_iCount; i++) 
    if (m_arrGroups[i].m_pSorter)
      m_arrGroups[i].m_pSorter->Sort();
}

bool CGroupSorter::Render()
{
  int i;
  bool bRes = true;
  for (i = 0; i < m_arrGroups.m_iCount; i++) 
    if (m_arrGroups[i].m_pSorter)
      bRes &= m_arrGroups[i].m_pSorter->Render();
  return bRes;
}

int CGroupSorter::GetModelGroup(CModel *pModel)
{
  static const CStrAny sSorterGroup(ST_CONST, "SorterGroup");
  int iGroup;

  if (!pModel->GetApplyVars()->GetInt(sSorterGroup, iGroup))
    iGroup = -1;

  return iGroup;
}
