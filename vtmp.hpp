//---------------------------------------------------
// utils/vtmp.hpp: Variadic template metaprogramming
//---------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**
``<utils/vtmp.hpp>`` --- Variadic template metaprogramming
==========================================================

vtmp is a library that applies template metaprogramming techniques using C++11
variadic templates.

*/

#ifndef VTMP_HPP_KFLYEISDKHB
#define VTMP_HPP_KFLYEISDKHB 1

#include <cstdlib>
#include <type_traits>
#include <utility>
#include <array>
#include <utils/traits.hpp>

namespace utils { namespace vtmp {

/**
Meta-types
----------
*/

/**
.. type:: struct utils::vtmp::integers<size_t... ns>

    A concrete type that holds a compile-time integer list *ns*.
*/
template <size_t... ns>
struct integers
{
    /**
    .. type:: type push_back<size_t n> = utils::vtmp::integers<ns..., n>

        Insert *n* to the end of the integer list.
    */
    template <size_t n>
    using push_back = integers<ns..., n>;

};

/**
Meta-functions
--------------
*/

/**
.. type:: type utils::vtmp::iota<size_t n> = utils::vtmp::integers<0, 1, ..., n-1>
*/
namespace xx_impl
{
    template <size_t n>
    struct iota_impl
    {
        typedef typename iota_impl<n-1>::type::template push_back<n-1> type;
    };

    template <>
    struct iota_impl<0>
    {
        typedef integers<> type;
    };
}

template <size_t n>
using iota = typename xx_impl::iota_impl<n>::type;

}

/**
Extra functions for ``std::tuple``
----------------------------------
*/

namespace xx_impl
{
    template <typename Source, typename Target>
    struct copy_cr { typedef Target type; };

    template <typename Source, typename Target>
    struct copy_cr<Source&, Target> { typedef Target& type; };
    template <typename Source, typename Target>
    struct copy_cr<Source&&, Target> { typedef Target&& type; };
    template <typename Source, typename Target>
    struct copy_cr<const Source&, Target> { typedef const Target& type; };
    template <typename Source, typename Target>
    struct copy_cr<const Source&&, Target> { typedef const Target&& type; };

    template <typename... T, typename F, typename G, size_t... ns, typename Tuple>
    auto tuple_funcs(F&& reduce, G&& map, const vtmp::integers<ns...>&, Tuple&& tup)
        -> decltype(reduce(map(std::declval<typename copy_cr<Tuple, T>::type>())...))
    {
        return reduce(map(std::get<ns>(tup))...);
    }

    struct no_op_func
    {
        template <typename T>
        T&& operator()(T&& value) const noexcept
        {
            return std::forward<T>(value);
        }
    };

    struct make_tuple_func
    {
        template <typename... T>
        auto operator()(T&&... values) const
            -> decltype(std::make_tuple(std::declval<T>()...))
        {
            return std::make_tuple(std::forward<T>(values)...);
        }
    };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
    template <typename C>
    struct constructor_func
    {
        template <typename... T>
        C operator()(T&&... values) const
        {
            return C{std::forward<T>(values)...};
        }
    };
#pragma GCC diagnostic pop
}

/**
.. function:: auto utils::tuple_apply<F>(std::tuple<T...>& tup, F&& func)
              auto utils::tuple_apply<F>(std::tuple<T...>&& tup, F&& func)
              auto utils::tuple_apply<F>(const std::tuple<T...>& tup, F&& func)

    Unpack *tup* and evaluate it using the a function *F*. Conceptually this
    function performs::

        return func(std::get<0>(tup), std::get<1>(tup), std::get<2>(tup), ...);

    *func* should be a function or function object.
*/
template <typename F, typename... T>
auto tuple_apply(std::tuple<T...>& tup, F&& func)
    -> decltype(func(std::declval<T&>()...))
{
    vtmp::template iota<sizeof...(T)> instance;
    return xx_impl::tuple_funcs<T...>(std::forward<F>(func), xx_impl::no_op_func(),
                                      instance, tup);
}
template <typename F, typename... T>
auto tuple_apply(std::tuple<T...>&& tup, F&& func)
    -> decltype(func(std::declval<T&&>()...))
{
    vtmp::template iota<sizeof...(T)> instance;
    return xx_impl::tuple_funcs<T...>(std::forward<F>(func), xx_impl::no_op_func(),
                                      instance, std::move(tup));
}
template <typename F, typename... T>
auto tuple_apply(const std::tuple<T...>& tup, F&& func)
    -> decltype(func(std::declval<const T&>()...))
{
    vtmp::template iota<sizeof...(T)> instance;
    return xx_impl::tuple_funcs<T...>(std::forward<F>(func), xx_impl::no_op_func(),
                                      instance, tup);
}

/**
.. function:: auto utils::tuple_map<F>(std::tuple<T...>&& tup, F&& func)
              auto utils::tuple_map<F>(const std::tuple<T...>& tup, F&& func)

    Perform the function on every element of the tuple, and return the tuple of
    those return values.

    Conceptually this function performs::

        return std::make_tuple(func(std::get<0>(tup)), func(std::get<1>(tup)), ...)

    The returned tuple will only contain value type elements.
*/
template <typename F, typename... T>
auto tuple_map(std::tuple<T...>&& tup, F&& func)
    -> std::tuple<decltype(func(std::declval<T>()))...>
{
    vtmp::template iota<sizeof...(T)> instance;
    return xx_impl::tuple_funcs<T...>(xx_impl::make_tuple_func(), std::forward<F>(func),
                                      instance, std::move(tup));
}

template <typename F, typename... T>
auto tuple_map(const std::tuple<T...>& tup, F&& func)
    -> std::tuple<decltype(func(std::declval<T>()))...>
{
    vtmp::template iota<sizeof...(T)> instance;
    return xx_impl::tuple_funcs<T...>(xx_impl::make_tuple_func(), std::forward<F>(func),
                                      instance, tup);
}

/**
.. function:: C&& utils::tuple_init<C>(std::tuple<T...>&& tup)
              C&& utils::tuple_init<C>(const std::tuple<T...>& tup)

    Initialize a class *C* using the elements of the tuple as an initializer
    list, and return that class.

    Conceptually this function performs::

        return C{std::get<0>(tup), std::get<1>(tup), ...};
*/
template <typename C, typename... T>
C tuple_construct(std::tuple<T...>&& tup)
{
    vtmp::template iota<sizeof...(T)> instance;
    return xx_impl::tuple_funcs<T...>(xx_impl::constructor_func<C>(), xx_impl::no_op_func(),
                                      instance, std::move(tup));
}

template <typename C, typename... T>
C tuple_construct(const std::tuple<T...>& tup)
{
    vtmp::template iota<sizeof...(T)> instance;
    return xx_impl::tuple_funcs<T...>(xx_impl::constructor_func<C>(), xx_impl::no_op_func(),
                                      instance, tup);
}



}

#endif

