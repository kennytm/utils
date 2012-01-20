//--------------------------------------------------------
// typename: Convert a type to a readable name in runtime
//--------------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**

``<utils/typename.hpp>`` --- Convert type to names
==================================================

*/

#ifndef TYPENAME_HPP_VFENXB3FC1G
#define TYPENAME_HPP_VFENXB3FC1G 1

#include <string>

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <queue>

#include <utils/variant.hpp>
#include <utils/traits.hpp>

namespace utils {

namespace xx_impl
{
    std::string remove_insignificant_spaces(std::string input);
}


/**
Members
-------
*/

/**
.. function:: std::string typeinfo_name(const std::type_info& type)

    Get the demangled type name with the given type info.

    The result of this function may be different from
    :func:`~utils::typename_of`.
*/
std::string typeinfo_name(const std::type_info& type);

/**
.. function:: std::string utils::typename_of<T...>(const T&...)

    A function to obtain the name of types *T*.

    If *T* is a list of more than one type, it will return a the string with
    all type names concatenated with commas.

    The *typename_of* function may sometimes call overloads of itself. Overloads
    may be provided via ADL to give a custom type name. By default, the function
    will use the type information with ``abi::__cxa_demangle`` to obtain the
    type name.
*/
template <typename T>
std::string typename_of(const T&)
{
    return typeinfo_name(typeid(T));
}


#define SPECIALIZE_TYPENAME_V5Q3A9QL4FE(Type) \
    static inline std::string typename_of(const Type&) { return #Type; }

SPECIALIZE_TYPENAME_V5Q3A9QL4FE(unsigned char)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(signed char)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(unsigned short)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(short)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(unsigned int)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(int)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(unsigned long)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(long)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(unsigned long long)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(long long)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(char)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(wchar_t)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(char16_t)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(char32_t)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(float)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(double)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(long double)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(std::string)
SPECIALIZE_TYPENAME_V5Q3A9QL4FE(std::nullptr_t)

#undef SPECIALIZE_TYPENAME_V5Q3A9QL4FE

#define SPECIALIZE_CONTAINERS_NCF313J0DX(Container) \
    template <typename T> \
    std::string typename_of(const std::Container<T>&) \
    { \
        return "std::" #Container "<" + typename_of(rt_val<T>()) + ">"; \
    }

SPECIALIZE_CONTAINERS_NCF313J0DX(deque)
SPECIALIZE_CONTAINERS_NCF313J0DX(forward_list)
SPECIALIZE_CONTAINERS_NCF313J0DX(list)
SPECIALIZE_CONTAINERS_NCF313J0DX(vector)
SPECIALIZE_CONTAINERS_NCF313J0DX(set)
SPECIALIZE_CONTAINERS_NCF313J0DX(multiset)
SPECIALIZE_CONTAINERS_NCF313J0DX(unordered_set)
SPECIALIZE_CONTAINERS_NCF313J0DX(unordered_multiset)
SPECIALIZE_CONTAINERS_NCF313J0DX(stack)
SPECIALIZE_CONTAINERS_NCF313J0DX(queue)
SPECIALIZE_CONTAINERS_NCF313J0DX(priority_queue)

#undef SPECIALIZE_CONTAINERS_NCF313J0DX

#define SPECIALIZE_CONTAINERS_2_0R7BYC4BG1FR(Container) \
    template <typename K, typename V> \
    std::string typename_of(const std::Container<K, V>&) \
    { \
        return "std::" #Container "<" + typename_of(rt_val<K>()) + "," \
                                      + typename_of(rt_val<V>()) + ">"; \
    }

SPECIALIZE_CONTAINERS_2_0R7BYC4BG1FR(map)
SPECIALIZE_CONTAINERS_2_0R7BYC4BG1FR(multimap)
SPECIALIZE_CONTAINERS_2_0R7BYC4BG1FR(unordered_map)
SPECIALIZE_CONTAINERS_2_0R7BYC4BG1FR(unordered_multimap)

#undef SPECIALIZE_CONTAINERS_2_0R7BYC4BG1FR

template <typename T, size_t n>
std::string typename_of(const std::array<T, n>&)
{
    return "std::array<" + typename_of(rt_val<T>()) + "," + std::to_string(n) + ">";
}

template <typename T, typename... Rest>
std::string typename_of(const T& a, const Rest&... rest)
{
    return typename_of(a) + "," + typename_of(rest...);
}

template <typename... T>
std::string typename_of(const utils::variant<T...>&)
{
    return "utils::variant<" + typename_of(rt_val<T>()...) + ">";
}

}

#endif

