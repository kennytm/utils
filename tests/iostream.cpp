#include <sstream>
#include <string>
#include <boost/test/unit_test.hpp>
#include <utils/boilerplates.hpp>

enum class Foo
{
    a, b, c
};

struct Bar
{
    int x, y, z;
};

IMPLEMENT_ENUM_OSTREAM(Foo, (a)(b)(c))
IMPLEMENT_STRUCT_OSTREAM(Bar, (x)(y)(z))

BOOST_AUTO_TEST_SUITE(test_iostream)

BOOST_AUTO_TEST_CASE(enum_iostream)
{
    std::ostringstream ss;
    ss << Foo::a << Foo::b << Foo::c;

    BOOST_CHECK_EQUAL(ss.str(), "Foo::aFoo::bFoo::c");
}

BOOST_AUTO_TEST_CASE(enum_struct)
{
    std::ostringstream ss;
    ss << Bar{1, 4, -6};
    BOOST_CHECK_EQUAL(ss.str(), "{1, 4, -6}");
}

BOOST_AUTO_TEST_SUITE_END()

