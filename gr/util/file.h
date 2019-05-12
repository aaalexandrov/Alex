#pragma once

#include "namespace.h"
#include <vector>

NAMESPACE_BEGIN(util)

std::vector<uint8_t> ReadFile(std::string const &path);

NAMESPACE_END(util)