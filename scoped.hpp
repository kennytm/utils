//----------------------------------------
// utils::scoped: C++11-based scope guard
//----------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**

``<utils/scoped.hpp>`` --- C++11-based scope guard
==================================================

This module implements the :macro:`utils::make_scope_guard` function for
generating a scope guard using a lambda function.

Synopsis
--------

::

    {
        FILE* f = fopen("file.txt", "rb");
        auto f_guard = utils::make_scope_guard([&]{ fclose(f); });
        ...
        // the file 'f' will be closed when exiting the scope.
    }

*/

#ifndef SCOPED_HPP_I40D5JT8OZ9
#define SCOPED_HPP_I40D5JT8OZ9 1

#include <type_traits>
#include <utility>

namespace utils {

namespace xx_impl
{
    template <typename F>
    class scope_guard final
    {
        typename std::remove_reference<F>::type destructor;
    public:
        scope_guard(F&& destructor) : destructor(std::move(destructor)) {}
        ~scope_guard() { destructor(); }
    };
}

template <typename F>
const xx_impl::scope_guard<F> make_scope_guard(F&& f)
    noexcept(noexcept(F(std::declval<F&&>())))
{
    return xx_impl::scope_guard<F>(std::forward<F>(f));
}

}

#endif

