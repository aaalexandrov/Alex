#pragma once

#include "namespace.h"
#include "utl.h"

NAMESPACE_BEGIN(util)

int32_t ReadUnicodePoint(uint8_t const *&utf8, uint8_t const *utf8End);
int32_t ReadUnicodePointReverse(uint8_t const *utf8, uint8_t const *&utf8End);
size_t WriteUnicodePoint(int32_t codePoint, uint8_t *&utf8, uint8_t const *utf8End);


struct StringRepository;
struct StrId {
	enum InitParam {
		DontAddToRepository,
		AddToRepository,
	};

#if defined(_DEBUG)
	static constexpr InitParam _addToRepositoryDefault = AddToRepository;
#else
	static constexpr InitParam _addToRepositoryDefault = DontAddToRepository;
#endif

	explicit StrId(char const *c, size_t charCount, InitParam addStr = _addToRepositoryDefault);
	explicit StrId(char const *c, InitParam addStr = _addToRepositoryDefault) : StrId(c, c ? strlen(c) : 0u, addStr) {}
	explicit StrId() : StrId(nullptr, 0u, DontAddToRepository) {}
	explicit StrId(std::string s, InitParam addStr = _addToRepositoryDefault);

	uint32_t GetId() const { return _id; }

	bool operator==(StrId other) const { return _id == other._id; }
	bool operator!=(StrId other) const { return !operator==(other); }

	bool operator<(StrId other) const { return _id < other._id; }
	bool operator<=(StrId other) const { return _id <= other._id; }
	bool operator>(StrId other) const { return _id > other._id; }
	bool operator>=(StrId other) const { return _id >= other._id; }

	operator bool() const { return operator!=(StrId()); }
	bool operator!() const { return !operator bool(); }

	std::string GetString() const;
	void RemoveStringGlobal() const;
private:
	uint32_t _id;

	static StringRepository &GetStrRepo();
};

NAMESPACE_END(util)

NAMESPACE_BEGIN(std)

template<>
struct hash<util::StrId> { size_t operator() (util::StrId key) const { return key.GetId(); } };

NAMESPACE_END(std)

NAMESPACE_BEGIN(util)

struct StringRepository {
	void AddToRepository(StrId id, std::string str);
	std::string GetFromRepository(StrId id);
	void RemoveFromRepository(StrId id);

	std::recursive_mutex _mutex;
	std::unordered_map<StrId, std::string> _strRepo;
};

inline StringRepository &StrId::GetStrRepo() 
{
	static StringRepository s_repo;
	return s_repo;
}

NAMESPACE_END(util)

