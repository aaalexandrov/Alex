#pragma once

#include "namespace.h"
#include "rtti.h"

NAMESPACE_BEGIN(util)

struct TypeMember {
  TypeMember(std::string name) : _name(name) {}

  std::string _name;

  virtual TypeInfo const *GetContainingType() const = 0;
  virtual TypeInfo const *GetType() const = 0;
  virtual size_t GetOffset() const = 0;

  virtual Func const *Getter() const = 0;
  virtual Func const *Setter() const = 0;

  template <typename MemType, typename Class>
  MemType GetClassValue(Class const &obj) const
  {
    return Getter()->Call<MemType>(const_cast<Class*>(&obj));
  }

  template <typename MemType, typename Class>
  void SetClassValue(Class &obj, MemType memValue) const
  {
    Setter()->Call<void>(&obj, memValue);
  }

  template <typename MemType>
  MemType GetBufferValue(void const *obj) const
  {
    return Getter()->Call<MemType>(const_cast<void*>(obj));
  }

  template <typename MemType>
  void SetBufferValue(void *obj, MemType memValue) const
  {
    Setter()->Call<void>(obj, memValue);
  }
};

template <typename MemType>
struct TypeMemberAtOffsetImpl : TypeMember {
  using MemberType = MemType;

  TypeMemberAtOffsetImpl(std::string name, size_t offset)
    : TypeMember(name)
    , _offset(offset)
    , _getter([this](void *obj)->MemType { return GetValue(obj); })
    , _setter([this](void *obj, MemType memValue) { return SetValue(obj, memValue); })
  {}

  size_t _offset;
  FuncImpl<MemType, void*> _getter;
  FuncImpl<void, void*, MemType> _setter;

  TypeInfo const *GetContainingType() const override { return nullptr; }
  TypeInfo const *GetType() const override { return TypeInfo::Get<MemType>(); }
  size_t GetOffset() const override { return _offset; }

  Func const *Getter() const override { return &_getter; }
  Func const *Setter() const override { return &_setter; }

  inline MemType *GetMemberPtr(void *obj) const
  {
    return reinterpret_cast<MemType*>(reinterpret_cast<uint8_t*>(obj) + _offset);
  }

  inline MemType GetValue(void *obj) const
  {
    return *GetMemberPtr(obj);
  }

  inline void SetValue(void *obj, MemType memValue) const
  {
    *GetMemberPtr(obj) = memValue;
  }
};

template <typename Class, typename MemType>
struct TypeMemberImpl : TypeMember {
  using Type = Class;
  using MemberType = MemType;

  TypeMemberImpl(std::string name, MemType Class::*memPtr)
    : TypeMember(name)
    , _member(memPtr)
    , _getter([this](Class *obj)->MemType { return GetValue(obj); })
    , _setter([this](Class *obj, MemType memValue) { return SetValue(obj, memValue); })
  {}

  MemType Class::*_member;
  FuncImpl<MemType, Class*> _getter;
  FuncImpl<void, Class*, MemType> _setter;

  TypeInfo const *GetContainingType() const override { return TypeInfo::Get<Class>(); }
  TypeInfo const *GetType() const override { return TypeInfo::Get<MemType>(); }
  size_t GetOffset() const override { return reinterpret_cast<size_t>(&(static_cast<Class*>(nullptr)->*_member)); }

  Func const *Getter() const override { return &_getter; }
  Func const *Setter() const override { return &_setter; }

  inline MemType GetValue(Class *obj)
  {
    return obj->*_member;
  }

  inline void SetValue(Class *obj, MemType memValue)
  {
    obj->*_member = memValue;
  }
};

struct LayoutDescription {
  TypeMember const *GetMember(std::string const &name) const
  {
    auto found = _members.find(name);
    if (found == _members.end())
      return nullptr;
    return found->second.get();
  }

  std::unordered_map<std::string, std::unique_ptr<TypeMember>> _members;
};

template <typename Class>
struct TypeDescription : LayoutDescription {
  template <typename MemType>
  void AddMember(std::string name, MemType Class::*memPtr)
  {
    auto member = std::make_unique<TypeMemberImpl<Class, MemType>>(name, memPtr);
    ASSERT(_members.find(name) == _members.end());
    _members.emplace(name, std::move(member));
  }
};

struct BufferDescription : LayoutDescription {
  template <typename MemType>
  void AddMember(std::string name, size_t offset)
  {
    auto member = std::make_unique<TypeMemberAtOffsetImpl<MemType>>(name, offset);
    ASSERT(_members.find(name) == _members.end());
    _members.emplace(name, std::move(member));
  }
};


NAMESPACE_END(util)