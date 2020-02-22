#pragma once

#include "namespace.h"

NAMESPACE_BEGIN(util)

int32_t ReadUnicodePoint(uint8_t const *&utf8, uint8_t const *utf8End);
size_t WriteUnicodePoint(int32_t codePoint, uint8_t *&utf8, uint8_t const *utf8End);

NAMESPACE_END(util)