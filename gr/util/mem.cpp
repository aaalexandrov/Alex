#include "mem.h"

NAMESPACE_BEGIN(util)

void *AlignedAlloc(size_t alignment, size_t size)
{
    alignment = std::max(alignment, alignof(void *));
    uint8_t *mem = (uint8_t *)malloc(size + alignment);
    uintptr_t extra = alignment - (uintptr_t(mem)) % alignment;
    uint8_t *aligned = mem + extra;
    ASSERT(aligned - mem >= sizeof(void *));
    ASSERT(uintptr_t(aligned) % alignment == 0);
    ((void **)aligned)[-1] = mem;
    return aligned;
}

void *AlignedRealloc(void *mem, size_t alignment, size_t size)
{
    if (!size) {
        AlignedFree(mem);
        return nullptr;
    }
    void *newMem = AlignedAlloc(alignment, size);
    if (mem) {
        size_t prevSize = AlignedMemSize(mem);
        memcpy(newMem, mem, std::min(size, prevSize));
    }
    return newMem;
}

void AlignedFree(void *mem)
{
    if (mem)
        free(((void **)mem)[-1]);
}

size_t AlignedMemSize(void *mem)
{
    return mem ? MemSize(((void **)mem)[-1]) : 0;
}

NAMESPACE_END(util)

