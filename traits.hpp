//--------------------------------------
// utils/traits: Additional type traits
//--------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TRAITS_HPP_9ALQFEFX7TO
#define TRAITS_HPP_9ALQFEFX7TO 1

#include <cstdlib>
#include <tuple>
#include <functional>

namespace utils {

#define DECLARE_HAS_TYPE_MEMBER(member_name) \
    template <typename, typename = void> \
    struct has_##member_name \
    { enum { value = false }; }; \
    template <typename T> \
    struct has_##member_name<T, typename std::enable_if<sizeof(typename T::member_name)||true>::type> \
    { enum { value = true }; };

template <typename T>
struct function_traits
    : public function_traits<decltype(&T::operator())>
{};

template <typename ReturnType, typename... Args>
struct function_traits<ReturnType(Args...)>
{
    enum { arity = sizeof...(Args) };
    typedef ReturnType result_type;

    template <size_t i>
    struct arg
    {
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
    };
};

template <typename ReturnType, typename... Args>
struct function_traits<ReturnType(*)(Args...)>
    : public function_traits<ReturnType(Args...)>
{};

template <typename ReturnType, typename... Args>
struct function_traits<ReturnType(&)(Args...)>
    : public function_traits<ReturnType(Args...)>
{};

template <typename ReturnType, typename... Args>
struct function_traits<ReturnType(&&)(Args...)>
    : public function_traits<ReturnType(Args...)>
{};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...)>
    : public function_traits<ReturnType(Args...)>
{
    typedef ClassType owner_type;
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const>
    : public function_traits<ReturnType(Args...)>
{
    typedef /*const*/ ClassType owner_type;
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) volatile>
    : public function_traits<ReturnType(Args...)>
{
    typedef /*volatile*/ ClassType owner_type;
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const volatile>
    : public function_traits<ReturnType(Args...)>
{
    typedef /*const volatile*/ ClassType owner_type;
};

template <typename FunctionType>
struct function_traits<std::function<FunctionType>>
    : public function_traits<FunctionType>
{};

#ifdef _GLIBCXX_FUNCTIONAL

template <typename R, typename C>
struct function_traits<std::_Mem_fn<R C::*>>
    : public function_traits<R(C*)>
{};
template <typename R, typename C, typename... A>
struct function_traits<std::_Mem_fn<R(C::*)(A...)>>
    : public function_traits<R(C*, A...)>
{};
template <typename R, typename C, typename... A>
struct function_traits<std::_Mem_fn<R(C::*)(A...) const>>
    : public function_traits<R(const C*, A...)>
{};
template <typename R, typename C, typename... A>
struct function_traits<std::_Mem_fn<R(C::*)(A...) volatile>>
    : public function_traits<R(volatile C*, A...)>
{};
template <typename R, typename C, typename... A>
struct function_traits<std::_Mem_fn<R(C::*)(A...) const volatile>>
    : public function_traits<R(const volatile C*, A...)>
{};

#endif


}

#endif

