#include "layout.h"

NAMESPACE_BEGIN(util)

size_t LayoutElement::GetOffset(std::vector<rttr::variant> const &indices) const
{
	size_t offset = 0;
	LayoutElement const *elem = this;
	for (size_t i = 0; i < indices.size(); ++i) {
		size_t indexOffs = ~0ull;
		if (indices[i].can_convert<std::string>()) {
			std::string name = indices[i].get_value<std::string>();
			indexOffs = elem->GetOffset(name);
		} else if (indices[i].can_convert<size_t>()) {
			size_t ind = indices[i].get_value<size_t>();
			indexOffs = elem->GetOffset(ind);
		}
		if (indexOffs == ~0ull)
			return ~0ull;
		offset += indexOffs;
	}
	return offset;
}

size_t LayoutElement::GetMultidimensionalArrayCount() const
{
	size_t count = 1;
	LayoutElement const *elem = this;
	while (elem->GetKind() == Kind::Array) {
		count *= elem->GetArrayCount();
		elem = elem->GetArrayElement();
	}
	return count;
}

LayoutElement const *LayoutElement::GetMultidimensionalArrayElement() const
{
	LayoutElement const *elem = this;
	while (elem->GetKind() == Kind::Array) {
		elem = elem->GetArrayElement();
	}
	return elem;
}

bool LayoutElement::operator==(LayoutElement const &other) const
{
	if (this == &other)
		return true;
	if (GetKind() != other.GetKind() || _padding != other._padding)
		return false;
	return IsEqualToSameKind(other);
}

LayoutStruct::LayoutStruct(std::vector<std::pair<std::shared_ptr<LayoutElement>, std::string>> fields, size_t size)
{
	size_t offset = 0;
	for (size_t i = 0; i < fields.size(); ++i) {
		_fields.push_back({ fields[i].first, fields[i].second, offset });
		offset += _fields.back()._element->GetSize();
		_nameIndices.insert(std::make_pair(_fields.back()._name, i));
	}
	ASSERT(!size || size >= GetSize());
	_padding = size ? size - GetSize() : 0;
}

size_t LayoutStruct::GetStructFieldIndex(std::string name) const
{
	auto it = _nameIndices.find(name);
	return it != _nameIndices.end() ? it->second : ~0ull;
}

bool LayoutStruct::IsEqualToSameKind(LayoutElement const &other) const
{
	if (_fields.size() != other.GetStructFieldCount() || _padding != other.GetPadding())
		return false;
	for (size_t i = 0; i < _fields.size(); ++i) {
		if (_fields[i]._name != other.GetStructFieldName(i))
			return false;
		if (*_fields[i]._element != *other.GetStructFieldElement(i))
			return false;
		ASSERT(_fields[i]._offset == other.GetStructFieldOffset(i));
	}
	return true;
}

size_t LayoutStruct::GetHash() const
{
	size_t hash = _padding;
	std::hash<std::string> strHasher;
	for (size_t i = 0; i < _fields.size(); ++i) {
		hash = 31 * hash + strHasher(_fields[i]._name);
		hash = 31 * hash + _fields[i]._element->GetHash();
	}
	return hash;
}

NAMESPACE_END(util)
