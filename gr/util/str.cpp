#include "str.h"

NAMESPACE_BEGIN(util)

int32_t ReadUnicodePoint(uint8_t const *&utf8, uint8_t const *utf8End)
{
	ptrdiff_t maxLen = utf8End - utf8;
	if (maxLen <= 0)
		return 0;
	int32_t result;
	if (!(utf8[0] & 0x80)) {
		result = utf8[0];
		++utf8;
		return result;
	}
	if ((utf8[0] & 0xe0) == 0xc0) {
		if (maxLen < 2 || (utf8[1] & 0xc0) != 0x80)
			return 0;
		result = (static_cast<uint32_t>(utf8[0] & 0x1f) << 6) | (utf8[1] & 0x3f);
		utf8 += 2;
		return result;
	}
	if ((utf8[0] & 0xf0) == 0xe0) {
		if (maxLen < 3 || (utf8[1] & 0xc0) != 0x80 || (utf8[2] & 0xc0) != 0x80)
			return 0;
		result = (static_cast<uint32_t>(utf8[0] & 0x0f) << 12) | (static_cast<uint32_t>(utf8[1] & 0x3f) << 6) | (utf8[2] & 0x3f);
		utf8 += 3;
		return result;
	}
	if ((utf8[0] & 0xf8) == 0xf0) {
		if (maxLen < 4 || (utf8[1] & 0xc0) != 0x80 || (utf8[2] & 0xc0) != 0x80 || (utf8[3] & 0xc0) != 0x80)
			return 0;
		result = (static_cast<uint32_t>(utf8[0] & 0x07) << 18) | (static_cast<uint32_t>(utf8[1] & 0x3f) << 12) | (static_cast<uint32_t>(utf8[2] & 0x3f) << 6) | (utf8[3] & 0x3f);
		utf8 += 4;
		return result;
	}
	return 0;
}

int32_t ReadUnicodePointReverse(uint8_t const *utf8, uint8_t const *&utf8End)
{
	uint8_t const *pointStart = utf8End;
	while (pointStart > utf8 && (pointStart[-1] & 0xc0) == 0x80 && utf8End - pointStart <= 3)
		--pointStart;
	int32_t cp = ReadUnicodePoint(pointStart, utf8End);
	if (pointStart != utf8End) {
		ASSERT(!cp);
		return 0;
	}
	utf8End = pointStart;
	return cp;
}

size_t WriteUnicodePoint(int32_t codePoint, uint8_t *&utf8, uint8_t const *utf8End)
{
	ptrdiff_t maxLen = utf8End - utf8;
	if (codePoint < 0 || maxLen < 0 || utf8 && !maxLen)
		return 0;
	if (codePoint < 0x80) {
		if (utf8) {
			*utf8++ = static_cast<char>(codePoint);
		}
		return 1;
	}
	if (codePoint < 0x800) {
		if (utf8) {
			if (maxLen < 2)
				return 0;
			*utf8++ = static_cast<uint8_t>(0xc0 | (codePoint >> 6));
			*utf8++ = static_cast<uint8_t>(0x80 | (codePoint & 0x3f));
		}
		return 2;
	}
	if (codePoint < 0x10000) {
		if (utf8) {
			if (maxLen < 3)
				return 0;
			*utf8++ = static_cast<uint8_t>(0xe0 | (codePoint >> 12));
			*utf8++ = static_cast<uint8_t>(0x80 | ((codePoint >> 6) & 0x3f));
			*utf8++ = static_cast<uint8_t>(0x80 | (codePoint & 0x3f));
		}
		return 3;
	}
	if (codePoint < 0x200000) {
		if (utf8) {
			if (maxLen < 4)
				return 0;
			*utf8++ = static_cast<uint8_t>(0xf0 | (codePoint >> 18));
			*utf8++ = static_cast<uint8_t>(0x80 | ((codePoint >> 12) & 0x3f));
			*utf8++ = static_cast<uint8_t>(0x80 | ((codePoint >> 6) & 0x3f));
			*utf8++ = static_cast<uint8_t>(0x80 | (codePoint & 0x3f));
		}
		return 4;
	}
	return 0;
}


StrId::StrId(char const *c, size_t charCount, InitParam addStr)
	: _id(HashFNV<32>::Fnv1<char>(c, charCount))
{
	if (addStr == InitParam::AddToRepository)
		GetStrRepo().AddToRepository(*this, std::string(c, charCount));
}

StrId::StrId(std::string s, InitParam addStr)
	: _id(HashFNV<32>::Fnv1(s))
{
	if (addStr == InitParam::AddToRepository)
		GetStrRepo().AddToRepository(*this, s);
}

std::string StrId::GetString() const 
{ 
	return GetStrRepo().GetFromRepository(*this); 
}

void StrId::RemoveStringGlobal() const 
{ 
	GetStrRepo().RemoveFromRepository(*this); 
}


void StringRepository::AddToRepository(StrId id, std::string str)
{
	if (!id) {
		ASSERT(str == std::string());
		return;
	}
	std::lock_guard lock(_mutex);
	auto it = _strRepo.find(id);
	if (it != _strRepo.end()) {
		ASSERT(str == it->second);
		return;
	}
	_strRepo.insert(std::make_pair(id, str));
}

std::string StringRepository::GetFromRepository(StrId id)
{
	if (!id)
		return std::string();
	std::lock_guard lock(_mutex);
	auto it = _strRepo.find(id);
	if (it == _strRepo.end())
		return std::string();
	return it->second;
}

void StringRepository::RemoveFromRepository(StrId id)
{
	std::lock_guard lock(_mutex);
	_strRepo.erase(id);
}

NAMESPACE_END(util)

