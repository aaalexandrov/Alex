#pragma once

#include "namespace.h"
#include "mem.h"
#include <string>
#include <typeindex>

NAMESPACE_BEGIN(util)

template <typename Class>
struct TypeInfoBind;

template <typename Class>
struct RemoveConstVolatile {
  using Type = Class;
};

template <typename Class>
struct RemoveConstVolatile<Class*> {
  using Type = typename RemoveConstVolatile<Class>::Type*;
};

template <typename Class>
struct RemoveConstVolatile<const Class> {
  using Type = typename RemoveConstVolatile<Class>::Type;
};

template <typename Class>
struct RemoveConstVolatile<volatile Class> {
  using Type = typename RemoveConstVolatile<Class>::Type;
};

template <typename Class>
struct RemoveConstVolatile<volatile const Class> {
  using Type = typename RemoveConstVolatile<Class>::Type;
};

template <typename Class>
struct GetTypeBind {
  using Info = typename TypeInfoBind<typename RemoveConstVolatile<Class>::Type>::Info;
};


struct TypeInfo {
  using TypeId = std::type_index;

  virtual char const *GetName() = 0;
  virtual TypeId SelfId() = 0;
  virtual size_t GetSize() = 0;
  virtual size_t GetAlignment() = 0;

  bool IsSame(TypeInfo *other) { return SelfId() == other->SelfId(); }
  virtual bool IsBase(TypeInfo *other) = 0;
  bool IsSameOrBase(TypeInfo *other) { return IsSame(other) || IsBase(other); }
  virtual bool IsPointer() = 0;
  virtual TypeInfo *PointedToType() = 0;

  virtual void CallForBases(std::function<void(TypeInfo&)> func) = 0;

  std::vector<TypeInfo*> GetBases()
  {
    std::vector<TypeInfo*> bases;
    CallForBases([&bases](TypeInfo &base) { bases.push_back(&base); });
    return bases;
  }

  bool IsAssignableTo(TypeInfo *other)
  {
    if (IsSame(other))
      return true;
    if (IsPointer() && other->IsPointer()) {
      TypeInfo *pointedTo = PointedToType();
      TypeInfo *otherPointedTo = other->PointedToType();
      if (pointedTo->IsBase(otherPointedTo))
        return true;
    }
    return false;
  }

  template <typename Class>
  static inline TypeInfo *Get();
};

template <typename Class, typename... Bases>
struct TypeInfoImpl;

template <typename Class, typename... Bases>
struct TypeInfoImpl : TypeInfo {
  using Type = Class;
  using BasesTuple = std::tuple<Bases...>;

  template <typename Other>
  using IsBaseT = std::disjunction<typename GetTypeBind<Bases>::Info::template IsSameOrBaseT<Other>...>;

  template <typename Other>
  using IsSameOrBaseT = std::disjunction<std::is_same<TypeInfoImpl<Class, Bases...>, typename GetTypeBind<Other>::Info>, IsBaseT<Other>>;

  static TypeId SelfIdStatic() { return TypeId(typeid(Class)); }
  static char const *GetNameStatic() { return typeid(Class).name(); }
  static bool IsPointerStatic() { return std::is_pointer<Class>::value; }

  template <typename Cls>
  struct PointedToGetter { static TypeInfo *Get() { return nullptr; } };

  template <typename Cls>
  struct PointedToGetter<Cls*> { static TypeInfo *Get() { return TypeInfo::Get<Cls>(); } };

  static TypeInfo *PointedToTypeStatic() { return PointedToGetter<Class>::Get(); }

  char const *GetName() override { return GetNameStatic(); }
  TypeId SelfId() override { return SelfIdStatic(); }
  bool IsPointer() override { return IsPointerStatic(); }
  TypeInfo *PointedToType() override { return PointedToTypeStatic(); }

  size_t GetSize() override { return SizeAlign<Class>::SizeOf(); }
  size_t GetAlignment() override { return SizeAlign<Class>::AlignOf(); }

  static bool IsBaseStatic(TypeId id)
  {
    bool isBase = false;
    using expander = int[];
    static_cast<void>(expander { 0, (isBase |= (id == GetTypeBind<Bases>::Info::SelfIdStatic() || GetTypeBind<Bases>::Info::IsBaseStatic(id)), 0)... });
    return isBase;
  }

  bool IsBase(TypeInfo *other) override
  {
    return IsBaseStatic(other->SelfId());
  }

  void CallForBases(std::function<void(TypeInfo&)> func) override
  {
    using expander = int[];
    auto funcVal = [func](TypeInfo& type) { func(type); return 0; };
    static_cast<void>(expander { 0, funcVal(*TypeRegistry::Instance.GetTypeInfo<Bases>())... });
  }
};

template <typename Class>
struct TypeInfoBind<Class*> {
  using Info = typename TypeInfoImpl<typename GetTypeBind<Class>::Info::Type*>;
};


struct TypeRegistry {
  static TypeRegistry Instance;

  static TypeRegistry &Get() { return Instance; }

  template <typename Class>
  void Register()
  {
    auto index = GetTypeBind<Class>::Info::SelfIdStatic();
    ASSERT(_types.find(index) == _types.end());
    _types.insert(std::make_pair(index, std::make_unique<GetTypeBind<Class>::Info>()));
  }

  template <typename Class>
  TypeInfo *GetTypeInfo() 
  { 
    auto index = GetTypeBind<Class>::Info::SelfIdStatic();
    auto found = _types.find(index);
    if (found != _types.end()) 
      return found->second.get();
    auto &&added = _types.insert(std::make_pair(index, std::make_unique<GetTypeBind<Class>::Info>()));
    return added.first->second.get();
  }

  std::unordered_map<TypeInfo::TypeId, std::unique_ptr<TypeInfo>> _types;
};

template <typename Class>
TypeInfo *TypeInfo::Get()
{
  return TypeRegistry::Get().GetTypeInfo<Class>();
}

template <typename Class>
struct TypeInfoRegisterer {
  TypeInfoRegisterer()
  {
    TypeRegistry::Get().Register<Class>();
  }
};


#define CAT_(A, B) A ## B
#define CAT(A, B) CAT_(A, B)

#define RTTI_BIND(T, ...)  template <> struct ::util::TypeInfoBind<T> { using Info = TypeInfoImpl<T, ##__VA_ARGS__>; };
#define RTTI_REGISTER(T)   namespace { static ::util::TypeInfoRegisterer<T> CAT(Register_, __LINE__); }

RTTI_BIND(void)
RTTI_BIND(bool)

RTTI_BIND(signed char)
RTTI_BIND(signed short)
RTTI_BIND(signed int)
RTTI_BIND(signed long)
RTTI_BIND(signed long long)

RTTI_BIND(unsigned char)
RTTI_BIND(unsigned short)
RTTI_BIND(unsigned int)
RTTI_BIND(unsigned long)
RTTI_BIND(unsigned long long)

RTTI_BIND(float)
RTTI_BIND(double)
RTTI_BIND(long double)

RTTI_BIND(std::string)

NAMESPACE_END(util)