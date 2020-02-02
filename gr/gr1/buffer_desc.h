#pragma once

#include "util/namespace.h"
#include "rttr/rttr_enable.h"
#include <memory>

NAMESPACE_BEGIN(gr1)

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

	uintptr_t GetUserData() { return _userData; }
	void SetUserData(uintptr_t userData) { _userData = userData; }
public:
	size_t _padding = 0;
	uintptr_t _userData = 0;
};

struct LayoutValue : public LayoutElement {
	Kind GetKind() override { return Kind::Value; }
	size_t GetSize() override { return _type.get_sizeof() + _padding; }

	rttr::type GetValueType() override { return _type; }

public:
	rttr::type _type;
};

struct LayoutArray : public LayoutElement {
	Kind GetKind() override { return Kind::Array; }
	size_t GetSize() override { return _element->GetSize() * _arrayCount + _padding; }

	virtual LayoutElement *GetArrayElement() override { return _element.get(); }
	virtual size_t GetArrayCount() override { return _arrayCount; }

public:
	std::shared_ptr<LayoutElement> _element;
	size_t _arrayCount = 0;
};

struct LayoutStruct : public LayoutElement {
	Kind GetKind() override { return Kind::Struct; }
	size_t GetSize() override { return GetFieldsSize(_fields.size()) + _padding; }

	size_t GetStructFieldCount() override { return _fields.size(); }
	LayoutElement *GetStructFieldElement(size_t fieldIndex) override { return _fields[fieldIndex]._element.get(); }
	std::string GetStructFieldName(size_t fieldIndex) override { return _fields[fieldIndex]._name; }

public:
	size_t GetFieldsSize(size_t beforeIndex)
	{
		size_t size = 0;
		for (size_t i = 0; i < beforeIndex; ++i)
			size += _fields[i]._element->GetSize();
		return size;
	}

	struct Field { 
		std::shared_ptr<LayoutElement> _element;
		std::string _name;
	};

	std::vector<Field> _fields;
};

struct BufferElement {
  rttr::type _type;
  size_t _offset;
  uint32_t _arraySize;
  uint32_t _location;

  void *GetPtr(void *buffer) const { return static_cast<uint8_t*>(buffer) + _offset; }

  template <typename Class>
  Class *GetPointer(void *buffer) const
  {
    ASSERT(_type == rttr::type::get<Class>());
    return reinterpret_cast<Class*>(GetPtr(buffer));
  }
};

struct BufferDesc;
using BufferDescPtr = std::shared_ptr<BufferDesc>;

struct BufferDesc {
  static BufferDescPtr Create() { return std::make_shared<BufferDesc>(); }

  BufferElement const *GetElement(std::string name) const
  {
    auto found = _elements.find(name);
    if (found == _elements.end())
      return nullptr;
    return &found->second;
  }

  void AddElement(std::string name, BufferElement &elem)
  {
    _size = std::max(_size, elem._offset + elem._type.get_sizeof());
    _elements.emplace(name, elem);
  }

  std::unordered_map<std::string, BufferElement> _elements;
  size_t _size = 0;
};

NAMESPACE_END(gr1)

