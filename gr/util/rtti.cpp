#include "rtti.h"
#include "type_layout.h"

NAMESPACE_BEGIN(util)

TypeRegistry TypeRegistry::Instance;

//RTTI_REGISTER(void)
//RTTI_REGISTER(bool)
//
//RTTI_REGISTER(signed char)
//RTTI_REGISTER(signed short)
//RTTI_REGISTER(signed int)
//RTTI_REGISTER(signed long)
//RTTI_REGISTER(signed long long)
//
//RTTI_REGISTER(unsigned char)
//RTTI_REGISTER(unsigned short)
//RTTI_REGISTER(unsigned int)
//RTTI_REGISTER(unsigned long)
//RTTI_REGISTER(unsigned long long)
//
//RTTI_REGISTER(float)
//RTTI_REGISTER(double)
//RTTI_REGISTER(long double)
//
//RTTI_REGISTER(std::string)



template <typename Class> struct CreatorTyped;

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

    return requiredArgs->template Call<Class*>(CreateInstance);
  }

  static Class *CreateInstance(Args... args) 
  {
    return new Class(args...);
  }
};


struct Z {
  int _i;

  virtual TypeInfo const *GetType() { return TypeInfo::Get<Z>(); }
};

struct A : Z {
  std::string _s;

  A() {}
  A(std::string s) : _s(s) {}

  TypeInfo const *GetType() override { return TypeInfo::Get<A>(); }
};

struct B : A {
  int _b;

  TypeInfo const *GetType() override { return TypeInfo::Get<B>(); }
};

struct C {
  virtual TypeInfo const *GetType() { return TypeInfo::Get<C>(); }
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

  std::type_info const &tv = typeid(void);

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
  desc.AddMember("_i", (int A::*)&A::_i);
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
  auto offsValS = descBuf.GetMember("_s")->GetBufferValue<std::string>((void*)&aInst);
  auto offsValI = descBuf.GetMember("_i")->GetBufferValue<int>((void*)&aInst);

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
