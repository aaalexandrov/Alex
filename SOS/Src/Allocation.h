#ifndef __ALLOCATION_H
#define __ALLOCATION_H

#include "Mem.h"

#ifdef _DEBUG
struct TSosAllocator: public TDefAllocator {};
#else
struct TSosAllocator: public TTrackingAllocator<TDefAllocator> {};
#endif

class CStrHeader;
template <>
struct TGetAllocator<CStrHeader> { typedef TSosAllocator Type; };

class CValue;
template <>
struct TGetAllocator<CValue> { typedef TSosAllocator Type; };

class CValueTable;
template <>
struct TGetAllocator<CValueTable> { typedef TSosAllocator Type; };

class CFragment;
template <>
struct TGetAllocator<CFragment> { typedef TSosAllocator Type; };

#endif
