#include <boost/test/unit_test.hpp>
#include <utils/ext/xterm256.hpp>

using namespace utils::xterm256;


BOOST_AUTO_TEST_SUITE(xterm256)

BOOST_AUTO_TEST_CASE(to_color)
{
    BOOST_CHECK_EQUAL(get_color(0), (color{0x00, 0x00, 0x00}));
    BOOST_CHECK_EQUAL(get_color(1), (color{0xc0, 0x00, 0x00}));
    BOOST_CHECK_EQUAL(get_color(9), (color{0xff, 0x80, 0x80}));
    BOOST_CHECK_EQUAL(get_color(6), (color{0x00, 0xc0, 0xc0}));
    BOOST_CHECK_EQUAL(get_color(15), (color{0xff, 0xff, 0xff}));

    BOOST_CHECK_EQUAL(get_color(16), (color{0x00, 0x00, 0x00}));
    BOOST_CHECK_EQUAL(get_color(21), (color{0x00, 0x00, 0xff}));
    BOOST_CHECK_EQUAL(get_color(44), (color{0x00, 0xd7, 0xd7}));
    BOOST_CHECK_EQUAL(get_color(180), (color{0xd7, 0xaf, 0x87}));
    BOOST_CHECK_EQUAL(get_color(202), (color{0xff, 0x5f, 0x00}));
    BOOST_CHECK_EQUAL(get_color(231), (color{0xff, 0xff, 0xff}));

    BOOST_CHECK_EQUAL(get_color(232), (color{0x08, 0x08, 0x08}));
    BOOST_CHECK_EQUAL(get_color(242), (color{0x6c, 0x6c, 0x6c}));
    BOOST_CHECK_EQUAL(get_color(255), (color{0xee, 0xee, 0xee}));
}

BOOST_AUTO_TEST_CASE(almost_idempotent)
{
    for (auto i = 0; i <= 255; ++ i)
    {
        color c = get_color(static_cast<uint8_t>(i));
        auto j = get_index(c);
        auto target = i;
        switch (i)
        {
            default:
                break;
            case 16:    // pure black
                target = 0;
                break;
            case 231:   // pure white
                target = 15;
                break;
            case 244:   // 50% gray
                target = 8;
                break;
        }
        BOOST_CHECK_EQUAL(j, target);
    }
}

BOOST_AUTO_TEST_CASE(nearest_color)
{
    BOOST_CHECK_EQUAL(get_index({0xfe,0x01,0x01}), 196);
    BOOST_CHECK_EQUAL(get_index({0x40,0x30,0x20}), 236);
    BOOST_CHECK_EQUAL(get_index({0xcc,0x00,0x00}), 160);
    BOOST_CHECK_EQUAL(get_index({0xcb,0x00,0x00}), 1);
}

BOOST_AUTO_TEST_SUITE_END()

