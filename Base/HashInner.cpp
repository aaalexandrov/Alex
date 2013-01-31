#include "stdafx.h"
#include "HashInner.h"
#include "Hash.h"
#include <time.h>
#include <stdio.h>

/*

class A {
public:
    int i;
    A() {
        i = 0;
    }
    ~A() {
        i = -1;
    }
};

struct TTestHashInner {
	TTestHashInner()
	{
		int i;
		TConDestructor<int>::Construct(&i);
		TConDestructor<int>::Destroy(&i);

		int const *pI;
		TConDestructor<int const*>::Construct(&pI);
		TConDestructor<int const*>::Destroy(&pI);

		BYTE btBuf[sizeof(A)], btBuf1[sizeof(A)];
		TConDestructor<A>::Construct((A*)btBuf);
		TConDestructor<A>::Destroy((A*)btBuf);
		TConDestructor<A>::ConstructCopy((A*)btBuf1, *(A *)btBuf);
		TConDestructor<A>::Destroy((A*)btBuf1);

		CHashInner<int> kH;

		kH.Add(5);
		kH.Add(17);
		kH.Add(18);
		kH.Add(17);

		CHashInner<int>::TIter it;
		it = kH.Find(999);
		it = kH.Find(5);
		it = kH.Find(18);

		kH.RemoveValue(17);
		it = kH.Find(17);

		CArray<int> arrTest(10000000);
		for (int i = 0; i < arrTest.m_iMaxCount; i++)
			arrTest.Append(rand());

		CHash<int> kHash;
		CHashInner<int> kHashI;

		time_t tStart = time(0);

		for (int i = 0; i < arrTest.m_iCount; i++)
			kHash.Add(arrTest[i]);

		double dElapsed = difftime(time(0), tStart);
		printf("Adding to Hash: %g\n", dElapsed);

		tStart = time(0);

		for (int i = 0; i < arrTest.m_iCount; i++)
			kHashI.Add(arrTest[i]);

		dElapsed = difftime(time(0), tStart);
		printf("Adding to HashInner: %g\n", dElapsed);

		ASSERT(kHash.m_iCount == arrTest.m_iCount);
		ASSERT(kHashI.m_iCount == arrTest.m_iCount);

		tStart = time(0);

		for (int i = 0; i < arrTest.m_iCount; i++) {
			CHash<int>::TIter it = kHash.Find(arrTest[i]);
			ASSERT(it);
		}

		dElapsed = difftime(time(0), tStart);
		printf("Searching in Hash: %g\n", dElapsed);

		tStart = time(0);

		for (int i = 0; i < arrTest.m_iCount; i++) {
			CHashInner<int>::TIter it = kHashI.Find(arrTest[i]);
			ASSERT(it);
		}

		dElapsed = difftime(time(0), tStart);
		printf("Searching in HashInner: %g\n", dElapsed);

		tStart = time(0);

		for (int i = 0; i < arrTest.m_iCount; i++) {
			kHash.RemoveValue(arrTest[arrTest.m_iCount - 1 - i]);
		}

		dElapsed = difftime(time(0), tStart);
		printf("Removing from Hash: %g\n", dElapsed);

		tStart = time(0);

		for (int i = 0; i < arrTest.m_iCount; i++) {
			kHashI.RemoveValue(arrTest[arrTest.m_iCount - 1 - i]);
		}

		dElapsed = difftime(time(0), tStart);
		printf("Removing from HashInner: %g\n", dElapsed);

	}
} g_TestHashInner;
 */