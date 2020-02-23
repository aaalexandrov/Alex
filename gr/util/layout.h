#pragma once

#include "namespace.h"
#include "str.h"
#include "rttr/rttr_enable.h"
#include <memory>

NAMESPACE_BEGIN(util)

struct LayoutElement : public std::enable_shared_from_this<LayoutElement> {
	enum class Kind {
		Value,
		Array,
		Struct,
	};

	virtual Kind GetKind() const = 0;
	virtual size_t GetSize() const = 0;

	virtual rttr::type GetValueType() const { return rttr::type::get<void>(); }

	virtual LayoutElement const *GetArrayElement() const { return nullptr; }
	virtual size_t GetArrayCount() const { return 0; }

	virtual size_t GetStructFieldCount() const { return 0; }
	virtual LayoutElement const *GetStructFieldElement(size_t fieldIndex) const { return nullptr; }
	virtual std::string GetStructFieldName(size_t fieldIndex) const { return {}; }
	virtual StrId GetStructFieldId(size_t fieldIndex) const { return StrId(); }
	virtual size_t GetStructFieldOffset(size_t fieldIndex) const { return ~0ull; }
	virtual size_t GetStructFieldIndex(StrId nameId) const { return ~0ull; }
	virtual std::string GetStructFieldName(StrId nameId) const { return GetStructFieldName(GetStructFieldIndex(nameId)); }

	size_t GetPadding() const { return _padding; }
	void SetPadding(size_t padding) { _padding = padding; }
	bool IsValue() const { return GetKind() == Kind::Value; }
	bool IsArray() const { return GetKind() == Kind::Array; }
	bool IsStruct() const { return GetKind() == Kind::Struct; }
	intptr_t GetUserData() const { return _userData; }
	void SetUserData(intptr_t userData) const { _userData = userData; }

	virtual size_t GetOffset(size_t index) const = 0;
	size_t GetOffset(StrId nameId) const { return GetOffset(GetStructFieldIndex(nameId)); }
	virtual LayoutElement const *GetElement(size_t index) const = 0;
	LayoutElement const *GetElement(StrId nameId) const { return GetElement(GetStructFieldIndex(nameId)); }
	size_t GetOffset(std::vector<rttr::variant> const &indices) const;

	size_t GetMultidimensionalArrayCount() const;
	LayoutElement const *GetMultidimensionalArrayElement() const;

	std::shared_ptr<LayoutElement> AsShared() const { return const_cast<LayoutElement*>(this)->shared_from_this(); }

	std::pair<LayoutElement const*, size_t> GetMemberElementAndOffset() const { return std::make_pair(this, 0); }
	template<typename First, typename... Rest>
	std::pair<LayoutElement const*, size_t> GetMemberElementAndOffset(First first, Rest... rest) const
	{
		LayoutElement const *element = GetElement(first);
		if (!element)
			return std::pair<LayoutElement const*, size_t>(nullptr, ~0ull);
		std::pair<LayoutElement const*, size_t> restElemOffs = element->GetMemberElementAndOffset(rest...);
		if (!restElemOffs.first)
			return restElemOffs;
		size_t firstOffs = GetOffset(first);
		return std::make_pair(restElemOffs.first, firstOffs + restElemOffs.second);
	}

	template<typename DataType, typename... Indices>
	DataType *GetMemberPtr(void *buffer, Indices... indices) const
	{
		std::pair<LayoutElement const*, size_t> elemOffs = GetMemberElementAndOffset(indices...);
		rttr::type dataType = rttr::type::get<DataType>();
		if (!elemOffs.first || elemOffs.first->GetValueType() != dataType && dataType != rttr::type::get<void>())
			return nullptr;
		return reinterpret_cast<DataType*>(reinterpret_cast<uint8_t*>(buffer) + elemOffs.second);
	}

	virtual bool IsEqualToSameKind(LayoutElement const &other) const = 0;
	virtual size_t GetHash() const = 0;

	bool operator==(LayoutElement const &other) const;
	bool operator!=(LayoutElement const &other) const { return !(*this == other); }
protected:
	size_t _padding = 0;
	mutable intptr_t _userData = 0;
};

struct LayoutValue : public LayoutElement {
	LayoutValue(rttr::type type, size_t size = 0) : _type(type) { _padding = size ? size - GetSize() : 0; }

	Kind GetKind() const override { return Kind::Value; }
	size_t GetSize() const override { return _type.get_sizeof() + _padding; }
	size_t GetOffset(size_t index) const override { return ~0ull; }
	LayoutElement const *GetElement(size_t index) const override { return nullptr; }

	rttr::type GetValueType() const override { return _type; }

	bool IsEqualToSameKind(LayoutElement const &other) const override { return GetValueType() == other.GetValueType() && _padding == other.GetPadding(); }
	size_t GetHash() const override { return 31 * _type.get_id() + _padding; }
protected:
	rttr::type _type;
};

struct LayoutArray : public LayoutElement {
	LayoutArray(std::shared_ptr<LayoutElement> element, size_t arrayCount, size_t size = 0) 
		: _element(element), _arrayCount(arrayCount) { _padding = size ? size - GetSize() : 0; }

	Kind GetKind() const override { return Kind::Array; }
	size_t GetSize() const override { return _element->GetSize() * _arrayCount + _padding; }
	size_t GetOffset(size_t index) const override { return index < _arrayCount ? _element->GetSize() * index : ~0ull; }
	LayoutElement const *GetElement(size_t index) const override { return index < _arrayCount ? _element.get() : nullptr; }

	LayoutElement const *GetArrayElement() const override { return _element.get(); }
	size_t GetArrayCount() const override { return _arrayCount; }

	bool IsEqualToSameKind(LayoutElement const &other) const override { return GetArrayCount() == other.GetArrayCount() && *GetArrayElement() == *other.GetArrayElement() && _padding == other.GetPadding(); }
	size_t GetHash() const override { return 31 * (GetArrayCount() + 31 * GetArrayElement()->GetHash()) + _padding; }
protected:
	std::shared_ptr<LayoutElement> _element;
	size_t _arrayCount = 0;
};

struct LayoutStruct : public LayoutElement {
	LayoutStruct(std::vector<std::pair<std::shared_ptr<LayoutElement>, std::string>> fields = {}, size_t size = 0);

	void AddField(std::string name, std::shared_ptr<LayoutElement> const &elem, size_t offset = ~0ull);

	Kind GetKind() const override { return Kind::Struct; }
	size_t GetSize() const override { return (_fields.size() ? (_fields.back()._offset + _fields.back()._element->GetSize()) : 0) + _padding; }
	size_t GetOffset(size_t index) const override { return index < _fields.size() ? _fields[index]._offset : ~0ull; }
	LayoutElement const *GetElement(size_t index) const override { return index < _fields.size() ? _fields[index]._element.get() : nullptr; }

	size_t GetStructFieldCount() const override { return _fields.size(); }
	LayoutElement const *GetStructFieldElement(size_t fieldIndex) const override { return _fields[fieldIndex]._element.get(); }
	std::string GetStructFieldName(size_t fieldIndex) const override { return _fields[fieldIndex]._name; }
	StrId GetStructFieldId(size_t fieldIndex) const override { return _fields[fieldIndex]._id; }
	size_t GetStructFieldOffset(size_t fieldIndex) const override { return _fields[fieldIndex]._offset; }
	size_t GetStructFieldIndex(StrId nameId) const override;

	bool IsEqualToSameKind(LayoutElement const &other) const override;
	size_t GetHash() const override;
protected:
	struct Field { 
		std::shared_ptr<LayoutElement> _element;
		std::string _name;
		StrId _id;
		size_t _offset;
	};

	std::vector<Field> _fields;
	std::unordered_map<StrId, size_t> _nameIndices;
};

inline std::shared_ptr<LayoutValue> CreateLayoutValue(rttr::type type, size_t size = 0)
{
	return std::make_shared<LayoutValue>(type, size);
}

inline std::shared_ptr<LayoutArray> CreateLayoutArray(std::shared_ptr<LayoutElement> const &elem, size_t size)
{
	return std::make_shared<LayoutArray>(elem, size);
}

template <typename... Sizes>
std::shared_ptr<LayoutArray> CreateLayoutArray(std::shared_ptr<LayoutElement> const &elem, size_t size, Sizes... sizes)
{
	return std::make_shared<LayoutArray>(CreateLayoutArray(elem, sizes...), size);
}

template <typename... Sizes>
std::shared_ptr<LayoutArray> CreateLayoutArray(rttr::type elemType, Sizes... sizes)
{
	return CreateLayoutArray(std::make_shared<LayoutValue>(elemType), sizes...);
}

template <typename Elem>
inline std::shared_ptr<LayoutArray> CreateLayoutArray(std::vector<Elem> const &vec)
{
	return CreateLayoutArray(CreateLayoutValue(rttr::type::get<Elem>()), vec.size());
}

inline std::shared_ptr<LayoutElement> GetLayoutElement(rttr::type type) { return std::make_shared<LayoutValue>(type); }
inline std::shared_ptr<LayoutElement> GetLayoutElement(std::shared_ptr<LayoutElement> elem) { return elem; }
inline std::shared_ptr<LayoutElement> GetLayoutElement(LayoutElement *elem) { return elem->shared_from_this(); }

inline void PushStructElementName(std::vector<std::pair<std::shared_ptr<LayoutElement>, std::string>> &elems) {}

template <typename Name, typename Elem, typename... Rest>
void PushStructElementName(std::vector<std::pair<std::shared_ptr<LayoutElement>, std::string>> &elems, Name name, Elem elem, Rest... rest)
{
	elems.push_back(std::make_pair(GetLayoutElement(elem), std::string(name)));
	PushStructElementName(elems, std::forward<Rest>(rest)...);
}

template <typename... Args>
std::shared_ptr<LayoutStruct> CreateLayoutStruct(Args... args)
{
	std::vector<std::pair<std::shared_ptr<LayoutElement>, std::string>> elems;
	PushStructElementName(elems, std::forward<Args>(args)...);
	return std::make_shared<LayoutStruct>(elems);
}

NAMESPACE_END(util)

