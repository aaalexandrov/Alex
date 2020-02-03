#pragma once

#include "namespace.h"
#include "rttr/rttr_enable.h"
#include <memory>

NAMESPACE_BEGIN(util)

struct LayoutElement : public std::enable_shared_from_this<LayoutElement> {
	enum class Kind {
		Value,
		Array,
		Struct,
	};

	virtual Kind GetKind() = 0;
	virtual size_t GetSize() = 0;

	virtual rttr::type GetValueType() { return rttr::type::get<void>(); }

	virtual LayoutElement *GetArrayElement() { return nullptr; }
	virtual size_t GetArrayCount() { return 0; }

	virtual size_t GetStructFieldCount() { return 0; }
	virtual LayoutElement *GetStructFieldElement(size_t fieldIndex) { return nullptr; }
	virtual std::string GetStructFieldName(size_t fieldIndex) { return {}; }
	virtual size_t GetStructFieldOffset(size_t fieldIndex) { return ~0ull; }
	virtual size_t GetStructFieldIndex(std::string name) { return ~0ull; }

	virtual size_t GetOffset(size_t index) = 0;
	size_t GetOffset(std::string name) { return GetOffset(GetStructFieldIndex(name)); }
	virtual LayoutElement *GetElement(size_t index) = 0;
	LayoutElement *GetElement(std::string name) { return GetElement(GetStructFieldIndex(name)); }
	size_t GetOffset(std::vector<rttr::variant> const &indices);

	size_t GetMemberOffset() { return 0; }
	template<typename First, typename... Rest>
	size_t GetMemberOffset(First first, Rest... rest)
	{
		LayoutElement *element = GetElement(first);
		if (!element)
			return ~0ull;
		size_t restOffs = element->GetMemberOffset(rest...);
		if (restOffs == ~0ull)
			return ~0ull;
		size_t firstOffs = GetOffset(first);
		return firstOffs + restOffs;
	}

public:
	size_t _padding = 0;
	intptr_t _userData = 0;
};

struct LayoutValue : public LayoutElement {
	LayoutValue(rttr::type type, size_t size = 0) : _type(type) { _padding = size ? size - GetSize() : 0; }

	Kind GetKind() override { return Kind::Value; }
	size_t GetSize() override { return _type.get_sizeof() + _padding; }
	size_t GetOffset(size_t index) override { return ~0ull; }
	LayoutElement *GetElement(size_t index) override { return nullptr; }

	rttr::type GetValueType() override { return _type; }

public:
	rttr::type _type;
};

struct LayoutArray : public LayoutElement {
	LayoutArray(std::shared_ptr<LayoutElement> element, size_t arrayCount, size_t size = 0) 
		: _element(element), _arrayCount(arrayCount) { _padding = size ? size - GetSize() : 0; }

	Kind GetKind() override { return Kind::Array; }
	size_t GetSize() override { return _element->GetSize() * _arrayCount + _padding; }
	size_t GetOffset(size_t index) override { return index < _arrayCount ? _element->GetSize() * index : ~0ull; }
	LayoutElement *GetElement(size_t index) override { return index < _arrayCount ? _element.get() : nullptr; }

	LayoutElement *GetArrayElement() override { return _element.get(); }
	size_t GetArrayCount() override { return _arrayCount; }

public:
	std::shared_ptr<LayoutElement> _element;
	size_t _arrayCount = 0;
};

struct LayoutStruct : public LayoutElement {
	LayoutStruct(std::vector<std::pair<std::shared_ptr<LayoutElement>, std::string>> fields, size_t size = 0);

	Kind GetKind() override { return Kind::Struct; }
	size_t GetSize() override { return _fields.back()._offset + _fields.back()._element->GetSize() + _padding; }
	size_t GetOffset(size_t index) override { return index < _fields.size() ? _fields[index]._offset : ~0ull; }
	LayoutElement *GetElement(size_t index) override { return index < _fields.size() ? _fields[index]._element.get() : nullptr; }

	size_t GetStructFieldCount() override { return _fields.size(); }
	LayoutElement *GetStructFieldElement(size_t fieldIndex) override { return _fields[fieldIndex]._element.get(); }
	std::string GetStructFieldName(size_t fieldIndex) override { return _fields[fieldIndex]._name; }
	size_t GetStructFieldOffset(size_t fieldIndex) override { return _fields[fieldIndex]._offset; }
	size_t GetStructFieldIndex(std::string name) override;

public:
	struct Field { 
		std::shared_ptr<LayoutElement> _element;
		std::string _name;
		size_t _offset;
	};

	std::vector<Field> _fields;
	std::unordered_map<std::string, size_t> _nameIndices;
};

NAMESPACE_END(util)

