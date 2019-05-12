#include "rtti.h"

NAMESPACE_BEGIN(util)

TypeRegistry TypeRegistry::Instance;

RTTI_REGISTER(void)
RTTI_REGISTER(bool)

RTTI_REGISTER(signed char)
RTTI_REGISTER(signed short)
RTTI_REGISTER(signed int)
RTTI_REGISTER(signed long)
RTTI_REGISTER(signed long long)

RTTI_REGISTER(unsigned char)
RTTI_REGISTER(unsigned short)
RTTI_REGISTER(unsigned int)
RTTI_REGISTER(unsigned long)
RTTI_REGISTER(unsigned long long)

RTTI_REGISTER(float)
RTTI_REGISTER(double)
RTTI_REGISTER(long double)

RTTI_REGISTER(std::string)



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

struct CreatorBase {
  virtual ~CreatorBase() {}

  template <typename Class, typename... Args>
  Class *Create(Args... args) 
  {
    auto requiredCreator = dynamic_cast<CreatorTyped<Class>*>(this);
    if (!requiredCreator)
      return nullptr;

    return requiredCreator->Create(args...);
  }
};

template <typename Class>
struct CreatorTyped : CreatorBase {
  virtual Class *CreatePacked(ArgsPack &&args) = 0;

  template <typename... Args>
  Class *Create(Args... args) 
  {
    return CreatePacked(ArgsPackImpl<Args...>(args...));
  }
};

template <typename Class, typename... Args>
struct Creator : CreatorTyped<Class> {
  virtual Class *CreatePacked(ArgsPack &&args) 
  {
    auto requiredArgs = dynamic_cast<ArgsPackImpl<Args...>*>(&args);
    if (!requiredArgs)
      return nullptr;

    return requiredArgs->Call<Class*>(CreateInstance);
  }

  static Class *CreateInstance(Args... args) 
  {
    return new Class(args...);
  }
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

struct Z {
  int _i;

  virtual TypeInfo *GetType() { return TypeInfo::Get<Z>(); }
};

struct A : Z {
  std::string _s;

  A() {}
  A(std::string s) : _s(s) {}

  TypeInfo *GetType() override { return TypeInfo::Get<A>(); }
};

struct B : A {
  int _b;

  TypeInfo *GetType() override { return TypeInfo::Get<B>(); }
};

struct C {
  virtual TypeInfo *GetType() { return TypeInfo::Get<C>(); }
};


RTTI_BIND(Z)
RTTI_BIND(A, Z)
RTTI_BIND(B, A)
RTTI_BIND(C)

//RTTI_REGISTER(Z)
//RTTI_REGISTER(A)
//RTTI_REGISTER(B)
//RTTI_REGISTER(C)

//RTTI_BIND(B*);
//RTTI_REGISTER(B*);

//RTTI_BIND(A*);
//RTTI_REGISTER(A*);

template <typename Class, class Enable = void>
struct DefOrSpec {
  int def;
};

template <typename Class>
struct DefOrSpec<Class, typename std::enable_if<std::is_same<Class, bool>::value>::type> {
  bool special;
};


void Test()
{
  DefOrSpec<char> dc;
  dc.def = 5;
  DefOrSpec<bool> db;
  db.special = false;

  auto na = GetTypeBind<A volatile const * volatile const>::Info::GetNameStatic();
  auto nb = GetTypeBind<B volatile const *  const>::Info::GetNameStatic();

  type_info const &tv = typeid(void);

  auto pack = ArgsPackImpl<>();
  auto packInt = ArgsPackImpl<int>(5);

  bool called = false;
  auto fCalled = [&called]() { called = true; };
  pack.Call<void>(fCalled);

  int ii = 66;
  auto iiCalled = [&ii](int i) { ii = i; };
  packInt.Call<void>(iiCalled);

  auto fInc = [](int i) { return i + 1; };
  Func *func = new FuncImpl<int, int>(fInc);
  auto iii = func->Call<int>(5, 6);

  Func *createB = new FuncImpl<B*>([]() { return new B(); });
  B const* b = createB->Call<B const*>();
  A const *ba = createB->Call<A const *>();
  auto bc = createB->Call<C*>();

  TypeDescription<A> desc;
  desc.AddMember("_i", &A::_i);
  desc.AddMember("_s", &A::_s);

  A aInst;
  aInst._i = 78;

  auto memI = desc.GetMember("_i");
  auto offsI = memI->GetOffset();
  auto sizeI = memI->GetType()->GetSize();

  auto mi = memI->GetClassValue<int>(aInst);
  memI->SetClassValue(aInst, 5);

  auto memS = desc.GetMember("_s");
  auto offsS = memS->GetOffset();
  auto sizeS = memS->GetType()->GetSize();
  auto memZ = desc.GetMember("somethingInvalid");

  BufferDescription descBuf;
  descBuf.AddMember<std::string>("_s", MemberOffset(&A::_s));
  descBuf.AddMember<int>("_i", MemberOffset(&A::_i));
  descBuf.GetMember("_s")->SetBufferValue<std::string>(&aInst, "kekek");
  descBuf.GetMember("_i")->SetBufferValue(&aInst, 66);
  auto offsValS = descBuf.GetMember("_s")->GetBufferValue<std::string>(&aInst);
  auto offsValI = descBuf.GetMember("_i")->GetBufferValue<int>(&aInst);

  CreatorBase *c1 = new Creator<A>();

  CreatorBase *c2 = new Creator<A, std::string>();

  int *i = c1->Create<int>();

  A *a11 = c1->Create<A>();
  A *a12 = c1->Create<A, std::string>("asd");

  A *a21 = c2->Create<A>();
  A *a22 = c2->Create<A, std::string>("asd");

  bool zb = TypeInfoBind<B>::Info::IsSameOrBaseT<Z>::value;
  bool zz = TypeInfoBind<B>::Info::IsSameOrBaseT<A>::value;
  bool kk = TypeInfoBind<A>::Info::IsSameOrBaseT<B>::value;
  bool k1 = TypeInfoBind<A>::Info::IsSameOrBaseT<A>::value;

  auto tiA = TypeInfo::Get<A>();
  auto tiB = TypeInfo::Get<B>();
  auto tiC = TypeInfo::Get<C>();
  auto tiZ = TypeInfo::Get<Z>();

  auto nA = tiA->GetName();
  auto nB = tiB->GetName();
  auto nC = tiC->GetName();
  auto nZ = tiZ->GetName();

  auto basesB = tiB->GetBases();
  auto z1 = tiA->IsSameOrBase(tiB);
  auto z2 = tiB->IsSameOrBase(tiA);
  auto z3 = tiB->IsSameOrBase(tiZ);
  auto z4 = tiB->IsSameOrBase(tiC);

//  bool b1 = GetTypeInfo<A>().IsA(a11);
//  bool b2 = GetTypeInfo<B>().IsA(a22);
}

NAMESPACE_END(util)
