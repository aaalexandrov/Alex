#pragma once

// helper macros for variadic macro overloading
#define VA_HELPER_EXPAND(_X)                    _X  // workaround for Visual Studio
#define VA_COUNT_HELPER(_1, _2, _3, _4, _5, _6, _Count, ...) _Count
#define VA_COUNT(...)                           VA_HELPER_EXPAND(VA_COUNT_HELPER(__VA_ARGS__, 6, 5, 4, 3, 2, 1))
#define VA_SELECT_CAT(_Name, _Count, ...)       VA_HELPER_EXPAND(_Name##_Count(__VA_ARGS__))
#define VA_SELECT_HELPER(_Name, _Count, ...)    VA_SELECT_CAT(_Name, _Count, __VA_ARGS__)
#define VA_SELECT(_Name, ...)                   VA_SELECT_HELPER(_Name, VA_COUNT(__VA_ARGS__), __VA_ARGS__)

// overloads for NAMESPACE_BEGIN
#define NAMESPACE_BEGIN_HELPER1(_Ns1)             namespace _Ns1 {
#define NAMESPACE_BEGIN_HELPER2(_Ns1, _Ns2)       namespace _Ns1 { NAMESPACE_BEGIN_HELPER1(_Ns2)
#define NAMESPACE_BEGIN_HELPER3(_Ns1, _Ns2, _Ns3) namespace _Ns1 { NAMESPACE_BEGIN_HELPER2(_Ns2, _Ns3)

// overloads for NAMESPACE_END
#define NAMESPACE_END_HELPER1(_Ns1)               }
#define NAMESPACE_END_HELPER2(_Ns1, _Ns2)         } NAMESPACE_END_HELPER1(_Ns2)
#define NAMESPACE_END_HELPER3(_Ns1, _Ns2, _Ns3)   } NAMESPACE_END_HELPER2(_Ns2, _Ns3)

// final macros
#define NAMESPACE_BEGIN(_Namespace, ...)    VA_SELECT(NAMESPACE_BEGIN_HELPER, _Namespace, __VA_ARGS__)
#define NAMESPACE_END(_Namespace, ...)      VA_SELECT(NAMESPACE_END_HELPER,   _Namespace, __VA_ARGS__)