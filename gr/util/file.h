#pragma once

#include "namespace.h"
#include <vector>

NAMESPACE_BEGIN(util)

std::vector<uint8_t> ReadFile(std::string const &path);

std::string GetPathDir(std::string path);
std::string GetPathFilenameExt(std::string path);
std::string GetPathExt(std::string path);

NAMESPACE_END(util)