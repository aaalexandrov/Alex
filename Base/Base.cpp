#include "stdafx.h"
#include "Base.h"
#include "Util.h"
#include <string.h>

// RTTI -----------------------------------------------------------------------

int CRTTI::s_iClasses = 0;
bool CRTTI::s_bSorted = true;
const CRTTI *CRTTI::s_pRTTIs[CRTTI::MAX_CLASSES];

inline bool CRTTI::Lt(const CRTTI *pRTTI1, const CRTTI *pRTTI2)
{
	return strcmp(pRTTI1->m_pClassName, pRTTI2->m_pClassName) < 0;
}

inline bool CRTTI::Lt(const char *pClassName, const CRTTI *pRTTI)
{
	return strcmp(pClassName, pRTTI->m_pClassName) < 0;
}

inline bool CRTTI::Lt(const CRTTI *pRTTI, const char *pClassName)
{
	return strcmp(pRTTI->m_pClassName, pClassName) < 0;
}

void CRTTI::Add(const CRTTI *pRTTI)
{
	ASSERT(s_iClasses < MAX_CLASSES);
	s_pRTTIs[s_iClasses++] = pRTTI;
	if (!s_bSorted || s_iClasses < 2)
		return;
	s_bSorted = !Lt(s_pRTTIs[s_iClasses - 1], s_pRTTIs[s_iClasses - 2]);
}

const CRTTI *CRTTI::Find(const char *pClassName)
{
	if (!s_bSorted) {
		Util::QSort<const CRTTI *[MAX_CLASSES], const CRTTI *, CRTTI>(s_pRTTIs, s_iClasses);
		s_bSorted = true;
	}
	int i;
	i = Util::BinSearch<const CRTTI *[MAX_CLASSES], const char *, CRTTI>(s_pRTTIs, s_iClasses, pClassName);
	if (i >= 0) {
		ASSERT(!strcmp(s_pRTTIs[i]->m_pClassName, pClassName));
		return s_pRTTIs[i];
	}
	return 0;
}


CRTTI::CRTTI(char *pClassName, FObjectCreate fnCreate, const CRTTI *pParent)
{
	m_pClassName = pClassName;
	m_fnCreate = fnCreate;
	m_pParent = pParent;
  m_uiClassData = 0;
	Add(this);
}

CRTTI CObject::s_RTTI("CObject", CObject::CreateInstance, 0);

class CTest: public CObject {
public:
};

template<class T, class B>
class CRTTI1 {
public:
  typedef CTest   Type;
  typedef CObject Base;

  char const *m_pClassName;

  CRTTI1(char const *pClassName);
  inline Type *CreateInstance() const { return new Type(); }

  CRTTI1<Base> const *GetBase() const {  }

  inline static CRTTI1 const *Get() {  }
};
