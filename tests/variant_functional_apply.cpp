#include <boost/test/unit_test.hpp>
#include <utils/variant.hpp>

BOOST_AUTO_TEST_SUITE(variant_functional_apply)

BOOST_AUTO_TEST_CASE(matcher)
{
    utils::variant<int, float> v (4);

    bool is_int = false;
    bool is_float = false;
    bool is_double = false;

    utils::case_of(v,
        [&](int) { is_int = true; },
        [&](float) { is_float = true; }
    );
    BOOST_CHECK(is_int);
    BOOST_CHECK(!is_float);

    is_float = is_int = false;
    v = 7.0f;
    utils::case_of(v,
        [&](int) { is_int = true; },
        [&](float) { is_float = true; }
    );
    BOOST_CHECK(!is_int);
    BOOST_CHECK(is_float);

    utils::case_of(v,
        [&](double) { is_double = true; }
    );
    BOOST_CHECK(is_double);
}

BOOST_AUTO_TEST_SUITE_END()

