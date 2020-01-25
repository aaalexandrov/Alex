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

struct Func;

struct TypeInfo {
  using TypeId = std::type_index;

  virtual char const *GetName() const = 0;
  virtual TypeId SelfId() const = 0;
  virtual size_t GetSize() const = 0;
  virtual size_t GetAlignment() const = 0;

  bool IsSame(TypeInfo const *other) const { return SelfId() == other->SelfId(); }
  virtual bool IsBase(TypeInfo const *other) const = 0;
  bool IsSameOrBase(TypeInfo const *other) const { return IsSame(other) || IsBase(other); }
  virtual bool IsPointer() const = 0;
  virtual TypeInfo const *PointedToType() const = 0;

  virtual void CallForBases(std::function<void(TypeInfo const &)> func) const = 0;

  std::vector<TypeInfo const*> GetBases() const
  {
    std::vector<TypeInfo const*> bases;
    CallForBases([&bases](TypeInfo const &base) { bases.push_back(&base); });
    return bases;
  }

  bool IsAssignableTo(TypeInfo const *other) const
  {
    if (IsSame(other))
      return true;
    if (IsPointer() && other->IsPointer()) {
      TypeInfo const *pointedTo = PointedToType();
      TypeInfo const *otherPointedTo = other->PointedToType();
      if (pointedTo->IsBase(otherPointedTo))
        return true;
    }
    return false;
  }

  template <typename Class>
  static inline TypeInfo const *Get();

	Func const *GetConstructor() const { return _ctor.get(); }
	void SetConstructor(Func *ctor) { _ctor.reset(ctor); }
protected:
	std::unique_ptr<Func> _ctor;
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
  struct PointedToGetter { static TypeInfo const *Get() { return nullptr; } };

  template <typename Cls>
  struct PointedToGetter<Cls*> { static TypeInfo const *Get() { return TypeInfo::Get<Cls>(); } };

  static TypeInfo const *PointedToTypeStatic() { return PointedToGetter<Class>::Get(); }

  char const *GetName() const override { return GetNameStatic(); }
  TypeId SelfId() const override { return SelfIdStatic(); }
  bool IsPointer() const override { return IsPointerStatic(); }
  TypeInfo const *PointedToType() const override { return PointedToTypeStatic(); }

  size_t GetSize() const override { return SizeAlign<Class>::SizeOf(); }
  size_t GetAlignment() const override { return SizeAlign<Class>::AlignOf(); }

  static bool IsBaseStatic(TypeId id)
  {
    bool isBase = false;
    using expander = int[];
    static_cast<void>(expander { 0, (isBase |= (id == GetTypeBind<Bases>::Info::SelfIdStatic() || GetTypeBind<Bases>::Info::IsBaseStatic(id)), 0)... });
    return isBase;
  }

  bool IsBase(TypeInfo const *other) const override
  {
    return IsBaseStatic(other->SelfId());
  }

  void CallForBases(std::function<void(TypeInfo const&)> func) const override
  {
    using expander = int[];
    auto funcVal = [func](TypeInfo const& type) { func(type); return 0; };
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
  TypeInfo &Register()
  {
    auto index = GetTypeBind<Class>::Info::SelfIdStatic();
    ASSERT(_types.find(index) == _types.end());
    auto it = _types.insert(std::make_pair(index, std::make_unique<GetTypeBind<Class>::Info>())).first;
		return *it->second;
  }

  template <typename Class>
  TypeInfo const *GetTypeInfo() 
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
TypeInfo const *TypeInfo::Get()
{
  return TypeRegistry::Get().GetTypeInfo<Class>();
}

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
	virtual TypeInfo const *ResultType() const = 0;
	virtual bool CompatibleArgs(ArgsPack &args) const = 0;

	template <typename Res, typename... Args>
	Res Call(Args... args) const
	{
		if (!ResultType()->IsAssignableTo(TypeInfo::Get<Res>()))
			return Res();
		ArgsPackImpl<Args...> args(args...);
		if (!CompatibleArgs(args))
			return Res();
		auto funcTyped = static_cast<FuncImpl<Res, Args...> const*>(this);
		return args.Call(funcTyped->_func);
	}
};

template <typename Res, typename... Args>
struct FuncImpl : Func {
	FuncImpl(std::function<Res(Args...)> func) : _func(func) {}

	virtual TypeInfo const *ResultType() const { return TypeInfo::Get<Res>(); }
	virtual bool CompatibleArgs(ArgsPack &args) const { return dynamic_cast<ArgsPackImpl<Args...>*>(&args); }

	std::function<Res(Args...)> _func;
};

template <typename Class>
struct TypeInfoRegisterer {
  TypeInfoRegisterer()
  {
    TypeRegistry::Get().Register<Class>();
  }

	template<typename... Args>
	TypeInfoRegisterer(std::function<Class*(Args... args)> ctor) 
	{
		TypeRegistry::Get().Register<Class>();
		TypeRegistry::Get().GetTypeInfo<Class>()->SetConstructor(new FuncImpl(ctor));
	}
};


#define CAT_(A, B) A ## B
#define CAT(A, B) CAT_(A, B)

#define STATIC_INIT1(FUNC, STRUCT)  \
	static void FUNC();               \
	namespace {                       \
		static struct STRUCT {     			\
				STRUCT() { FUNC(); }        \
		} CAT(STRUCT, _Instance);       \
	}                                 \
	static void FUNC()

#define STATIC_INIT STATIC_INIT1(CAT(InitFunc_, __LINE__), CAT(InitStruct, __LINE__))

#define RTTI_BIND(T, ...)  template <> struct ::util::TypeInfoBind<T> { using Info = TypeInfoImpl<T, ##__VA_ARGS__>; };
#define RTTI_REGISTER(T)   namespace { static inline ::util::TypeInfoRegisterer<T> CAT(Register_, __LINE__); }

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