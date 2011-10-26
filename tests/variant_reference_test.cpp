//-----------------------------------------------------------------------------
// boost-libs variant/test/variant_reference_test.cpp source file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2003
// Eric Friedman, Itay Maman
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>
#include <utils/variant.hpp>
#include <type_traits>

/////
// support types and functions

struct base_t { };
struct derived_t : base_t { };

template <typename Base, typename Derived>
bool check_base_derived(Base* b, Derived* d, long)
{
    return b == d;
}

template <typename Base, typename Derived>
bool check_base_derived(Base& b, Derived& d, int)
{
    return &b == &d;
}

template <typename T>
    typename std::add_lvalue_reference<T>::type
wknd_get(utils::variant<T&>& var, long)
{
    return utils::get<T>(var);
}

template <typename T>
    typename std::add_lvalue_reference<T>::type
wknd_get(utils::variant<T>& var, int)
{
    return utils::get<T>(var);
}

/////
// test functions

/*
template <typename T>
void test_reference_content(T& t, const T& value1, const T& value2)
{
    BOOST_CHECK( !(value1 == value2) );

    /////

    utils::variant< T& > var(t);
    BOOST_CHECK(( utils::get<T>(&var) == &t ));

    t = value1;
    BOOST_CHECK(( utils::get<T>(var) == value1 ));

    /////

    utils::variant< T > var2(var);
    BOOST_CHECK(( utils::get<T>(var2) == value1 ));

    t = value2;
    BOOST_CHECK(( utils::get<T>(var2) == value1 ));
}
*/

template <typename Base, typename Derived>
void base_derived_test(Derived d)
{
    typedef typename std::is_pointer<Base>::type is_ptr;

    Base b(d);
    BOOST_CHECK((check_base_derived(
          b
        , d
        , 1L
        )));

    utils::variant<Base> base_var(d);
    BOOST_CHECK((check_base_derived(
          wknd_get(base_var, 1L)
        , d
        , 1L
        )));

    utils::variant<Derived> derived_var(d);
    utils::variant<Base> base_from_derived_var(derived_var);
    BOOST_CHECK((check_base_derived(
          wknd_get(base_from_derived_var, 1L)
        , wknd_get(derived_var, 1L)
        , 1L
        )));
}

BOOST_AUTO_TEST_SUITE(reference_test)

BOOST_AUTO_TEST_CASE(reference_test)
{
    //int i = 0;
    //test_reference_content(i, 1, 2);

    /////

    derived_t d;
    //base_derived_test< int&,int >(i);
    base_derived_test< base_t*,derived_t* >(&d);
    //base_derived_test< base_t&,derived_t& >(d);
}

BOOST_AUTO_TEST_SUITE_END()

