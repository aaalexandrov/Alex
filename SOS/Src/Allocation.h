#ifndef __ALLOCATION_H
#define __ALLOCATION_H

#include "Mem.h"
#include "Allocator.h"

#ifdef _DEBUG
struct TSosAllocator: public TDefAllocator {};
#else
struct TSosAllocator: public TTrackingAllocator<TDefAllocator> {};
#endif

class CStrHeader;
template <>
struct TSpecifyAllocator<CStrHeader> { typedef TSosAllocator Type; };

class CValue;
template <>
struct TSpecifyAllocator<CValue> { typedef TSosAllocator Type; };

class CInstruction;
template <>
struct TSpecifyAllocator<CInstruction> { typedef TSosAllocator Type; };

class CValueTable;
template <>
struct TSpecifyAllocator<CValueTable> { typedef TSosAllocator Type; };

struct TCaptureVar;
template <>
struct TSpecifyAllocator<TCaptureVar> { typedef TSosAllocator Type; };

class CFragment;
template <>
struct TSpecifyAllocator<CFragment> { typedef TSosAllocator Type; };

class CClosure;
template <>
struct TSpecifyAllocator<CClosure> { typedef TSosAllocator Type; };

#endif
