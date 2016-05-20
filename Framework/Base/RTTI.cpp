#include "stdafx.h"
#include "RTTI.h"
#include "Base.h"
#include "Util.h"
#include <string.h>

// RTTI -----------------------------------------------------------------------

CObject *CObject::CreateInstance_s()
{
  return NEW(CObject, ());
}

CObject *CRTTI::CreateInstance_s()
{
  return NEW(Type, ());
}

CRTTIRegisterer<CObject> g_RegObject;

void CRTTIHolder::Add(CRTTI const *pRTTI)
{
	ASSERT(m_iClasses < MAX_CLASSES);
	m_pRTTIs[m_iClasses++] = pRTTI;
	if (!m_bSorted || m_iClasses < 2)
		return;
	m_bSorted = !Lt(m_pRTTIs[m_iClasses - 1], m_pRTTIs[m_iClasses - 2]);
}

CRTTI const *CRTTIHolder::Find(char const *pClassName)
{
	if (!m_bSorted) {
		Util::QSort<CRTTI const *[MAX_CLASSES], CRTTI const *, CRTTIHolder>(m_pRTTIs, m_iClasses);
		m_bSorted = true;
	}
	int i;
	i = Util::BinSearch<CRTTI const *[MAX_CLASSES], char const *, CRTTIHolder>(m_pRTTIs, m_iClasses, pClassName);
	if (i >= 0) {
		ASSERT(!strcmp(m_pRTTIs[i]->GetName(), pClassName));
		return m_pRTTIs[i];
	}
	return 0;
}

/*
class CTest;
template <>
class CRTTITpl<CTest, CObject, true>: public CRTTITpl<CTest, CObject, false> {
public:
  static  CObject *CreateInstance_s();
  virtual CObject *CreateInstance() const         { return CreateInstance_s(); }
};

class CTest: public CObject {
  DEFRTTI1(CTest, CObject, true)
public:
  CTest(int i) {}
};

CObject *CRTTITpl<CTest, CObject, true>::CreateInstance_s()             { return NEW(CTest, (0)); }

CRTTIRegisterer<CTest> g_RegTest;

void RTTITest()
{
  CTest kTest(0);

  ASSERT(!strcmp(kTest.GetRTTI()->GetClassName(), "CTest"));
  ASSERT(!strcmp(kTest.GetRTTI()->GetBase()->GetClassName(), "CObject"));
  CTest *pTest = (CTest *) CTest::GetRTTI_s()->CreateInstance();
  DEL(pTest);
}

struct TRTTITest {
  TRTTITest() { RTTITest(); }
} g_RTTITest;

*/
