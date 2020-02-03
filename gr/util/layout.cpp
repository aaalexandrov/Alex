#include "layout.h"

NAMESPACE_BEGIN(util)

size_t LayoutElement::GetOffset(std::vector<rttr::variant> const &indices)
{
	size_t offset = 0;
	LayoutElement *elem = this;
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

LayoutStruct::LayoutStruct(std::vector<std::pair<std::shared_ptr<LayoutElement>, std::string>> fields, size_t size)
{
	size_t offset = 0;
	for (size_t i = 0; i < fields.size(); ++i) {
		_fields.push_back({ fields[i].first, fields[i].second, offset });
		offset += _fields.back()._element->GetSize();
		_nameIndices.insert(std::make_pair(_fields.back()._name, i));
	}
	_padding = size ? size - GetSize() : 0;
}

size_t LayoutStruct::GetStructFieldIndex(std::string name)
{
	auto it = _nameIndices.find(name);
	return it != _nameIndices.end() ? it->second : ~0ull;
}

NAMESPACE_END(util)

