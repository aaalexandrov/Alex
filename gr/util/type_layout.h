#pragma once

#include "namespace.h"
#include "rtti.h"

NAMESPACE_BEGIN(util)

struct ArgsPack {
  virtual ~ArgsPack() {}
};

template<typename... Args>
struct ArgsPackImpl : public ArgsPack {
public:
  ArgsPackImpl(Args... args) : _argsTuple(args...) {}

  template <typename Res>
  Res Call(std::function<Res(Args...)> function)
  {
    return CallExpansion(function, std::index_sequence_for<Args...>{});
  }

  template <>
  void Call(std::function<void(Args...)> function)
  {
    CallExpansion(function, std::index_sequence_for<Args...>{});
  }

private:
  template<typename Res, std::size_t... I>
  Res CallExpansion(std::function<Res(Args...)> function, std::index_sequence<I...>)
  {
    return function(std::get<I>(_argsTuple)...);
  }

  template<std::size_t... I>
  void CallExpansion(std::function<void(Args...)> function, std::index_sequence<I...>)
  {
    function(std::get<I>(_argsTuple)...);
  }

  std::tuple<Args...> _argsTuple;
};

template <typename Res, typename... Args>
struct FuncImpl;

struct Func {
  virtual TypeInfo *ResultType() = 0;
  virtual bool CompatibleArgs(ArgsPack &args) = 0;

  template <typename Res, typename... Args>
  Res Call(Args... args)
  {
    if (!ResultType()->IsAssignableTo(TypeInfo::Get<Res>()))
      return Res();
    ArgsPackImpl<Args...> args(args...);
    if (!CompatibleArgs(args))
      return Res();
    auto funcTyped = static_cast<FuncImpl<Res, Args...>*>(this);
    return args.Call(funcTyped->_func);
  }
};

template <typename Res, typename... Args>
struct FuncImpl : Func {
  FuncImpl(std::function<Res(Args...)> func) : _func(func) {}

  virtual TypeInfo *ResultType() { return TypeInfo::Get<Res>(); }
  virtual bool CompatibleArgs(ArgsPack &args) { return dynamic_cast<ArgsPackImpl<Args...>*>(&args); }

  std::function<Res(Args...)> _func;
};


struct TypeMember {
  TypeMember(std::string name) : _name(name) {}

  std::string _name;

  virtual TypeInfo *GetContainingType() = 0;
  virtual TypeInfo *GetType() = 0;
  virtual size_t GetOffset() = 0;

  virtual Func *Getter() = 0;
  virtual Func *Setter() = 0;

  template <typename MemType, typename Class>
  MemType GetClassValue(Class const &obj)
  {
    return Getter()->Call<MemType>(const_cast<Class*>(&obj));
  }

  template <typename MemType, typename Class>
  void SetClassValue(Class &obj, MemType memValue)
  {
    Setter()->Call<void>(&obj, memValue);
  }

  template <typename MemType>
  MemType GetBufferValue(void const *obj)
  {
    return Getter()->Call<MemType>(const_cast<void*>(obj));
  }

  template <typename MemType>
  void SetBufferValue(void *obj, MemType memValue)
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

  TypeInfo *GetContainingType() override { return nullptr; }
  TypeInfo *GetType() override { return TypeInfo::Get<MemType>(); }
  size_t GetOffset() override { return _offset; }

  Func *Getter() override { return &_getter; }
  Func *Setter() override { return &_setter; }

  inline MemType *GetMemberPtr(void *obj)
  {
    return reinterpret_cast<MemType*>(reinterpret_cast<uint8_t*>(obj) + _offset);
  }

  inline MemType GetValue(void *obj)
  {
    return *GetMemberPtr(obj);
  }

  inline void SetValue(void *obj, MemType memValue)
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

  TypeInfo *GetContainingType() override { return TypeInfo::Get<Class>(); }
  TypeInfo *GetType() override { return TypeInfo::Get<MemType>(); }
  size_t GetOffset() override { return reinterpret_cast<size_t>(&(static_cast<Class*>(nullptr)->*_member)); }

  Func *Getter() override { return &_getter; }
  Func *Setter() override { return &_setter; }

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
  TypeMember *GetMember(std::string const &name)
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
    _members.emplace(std::make_pair(name, std::move(member)));
  }
};

struct BufferDescription : LayoutDescription {
  template <typename MemType>
  void AddMember(std::string name, size_t offset)
  {
    auto member = std::make_unique<TypeMemberAtOffsetImpl<MemType>>(name, offset);
    ASSERT(_members.find(name) == _members.end());
    _members.emplace(std::make_pair(name, std::move(member)));
  }
};


NAMESPACE_END(util)