//-----------------------------------------------------------------------------
// boost-libs variant/test/variant_visit_test.cpp source file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2003
// Eric Friedman
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>
#include <utils/variant.hpp>
#include <type_traits>

struct udt1
{
};

struct udt2
{
};

template <typename T>
class unary_check_content_type
    : public utils::static_visitor<bool>
{
public:

    // not recommended design, but simplifies workarounds:

    template <typename U>
    bool operator()(U&) const
    {
        return ::std::is_same<T,U>::value;
    }

};

template <typename T1, typename T2>
class binary_check_content_type
    : public utils::static_visitor<bool>
{
public:

    // not recommended design, but simplifies workarounds:

    template <typename U1, typename U2>
    bool operator()(U1&, U2&) const
    {
        return ::std::is_same<T1,U1>::value
            && ::std::is_same<T2,U2>::value;
    }

};

template <typename Checker, typename Variant>
static void unary_test(Variant& var, Checker* = 0)
{
    Checker checker;
    const Checker& const_checker = checker;

    // standard tests

    BOOST_CHECK( utils::apply_visitor(checker, var) );
    BOOST_CHECK( utils::apply_visitor(const_checker, var) );
    BOOST_CHECK( utils::apply_visitor(Checker(), var) );

    // delayed tests

    BOOST_CHECK( utils::apply_visitor(checker)(var) );
    BOOST_CHECK( utils::apply_visitor(const_checker)(var) );
}

template <typename Checker, typename Variant1, typename Variant2>
static void binary_test(Variant1& var1, Variant2& var2, Checker* = 0)
{
    Checker checker;
    const Checker& const_checker = checker;

    // standard tests

    BOOST_CHECK( utils::apply_visitor(checker, var1, var2) );
    BOOST_CHECK( utils::apply_visitor(const_checker, var1, var2) );
    BOOST_CHECK( utils::apply_visitor(Checker(), var1, var2) );

    // delayed tests

    BOOST_CHECK( utils::apply_visitor(checker)(var1, var2) );
    BOOST_CHECK( utils::apply_visitor(const_checker)(var1, var2) );
}

BOOST_AUTO_TEST_SUITE(visit_test)

BOOST_AUTO_TEST_CASE(visit_test)
{
    typedef utils::variant<udt1,udt2> var_t;
    udt1 u1;
    var_t var1(u1);
    udt2 u2;
    var_t var2(u2);

    const var_t& cvar1 = var1;
    const var_t& cvar2 = var2;

    //
    // unary tests
    //

    typedef unary_check_content_type<udt1> check1_t;
    typedef unary_check_content_type<const udt1> check1_const_t;
    typedef unary_check_content_type<udt2> check2_t;
    typedef unary_check_content_type<const udt2> check2_const_t;

    unary_test< check1_t       >(var1);
    unary_test< check1_const_t >(cvar1);

    unary_test< check2_t       >(var2);
    unary_test< check2_const_t >(cvar2);

    //
    // binary tests
    //

    typedef binary_check_content_type<udt1,udt2> check12_t;
    typedef binary_check_content_type<const udt1, const udt2> check12_const_t;
    typedef binary_check_content_type<udt2,udt1> check21_t;
    typedef binary_check_content_type<const udt2, const udt1> check21_const_t;

    binary_test< check12_t       >(var1,var2);
    binary_test< check12_const_t >(cvar1,cvar2);

    binary_test< check21_t       >(var2,var1);
    binary_test< check21_const_t >(cvar2,cvar1);
}

BOOST_AUTO_TEST_SUITE_END()

