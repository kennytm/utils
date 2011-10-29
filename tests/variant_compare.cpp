#include <vector>
#include <boost/test/unit_test.hpp>
#include <utils/variant.hpp>

typedef utils::variant<int, double> SomeVariant;

BOOST_AUTO_TEST_SUITE(variant_compare)

BOOST_AUTO_TEST_CASE(compare_test)
{
    SomeVariant x = 5;
    SomeVariant y = 4.5;

    BOOST_CHECK_EQUAL(x, x);
    BOOST_CHECK_EQUAL(y, y);
    BOOST_CHECK_NE(x, y);

    BOOST_CHECK_EQUAL(x, 5);
    BOOST_CHECK_EQUAL(5, x);
    BOOST_CHECK_NE(x, 4.5);
    BOOST_CHECK_NE(4.5, x);
    BOOST_CHECK_EQUAL(y, 4.5);
    BOOST_CHECK_EQUAL(4.5, y);
    BOOST_CHECK_NE(y, 5);
    BOOST_CHECK_NE(5, y);
}

BOOST_AUTO_TEST_SUITE_END()

