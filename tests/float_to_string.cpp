#include <boost/test/unit_test.hpp>
#include <utils/float_to_string.hpp>

BOOST_AUTO_TEST_SUITE(float_to_string)

BOOST_AUTO_TEST_CASE(float_to_string)
{
    BOOST_CHECK_EQUAL(utils::to_string(0.5), "0.5");
    BOOST_CHECK_EQUAL(utils::to_string(1.2), "1.2");
    BOOST_CHECK_EQUAL(utils::to_string(1.0/3), "0.333333333333333");
    BOOST_CHECK_EQUAL(utils::to_string(-600.0), "-600.0");
    BOOST_CHECK_EQUAL(utils::to_string(25.01), "25.01");
    BOOST_CHECK_EQUAL(utils::to_string(92e+61), "9.2e+62");
    BOOST_CHECK_EQUAL(utils::to_string(-9e-61), "-9.0e-61");
    BOOST_CHECK_EQUAL(utils::to_string(std::numeric_limits<double>::quiet_NaN()), "NaN");
    BOOST_CHECK_EQUAL(utils::to_string(std::numeric_limits<double>::infinity()), "Infinity");
    BOOST_CHECK_EQUAL(utils::to_string(-std::numeric_limits<double>::infinity()), "-Infinity");
    BOOST_CHECK_EQUAL(utils::to_string(0.0), "0.0");
    BOOST_CHECK_EQUAL(utils::to_string(-0.0), "-0.0");
}

BOOST_AUTO_TEST_SUITE_END()

