#include <boost/test/unit_test.hpp>
#include <utils/typename.hpp>

namespace Geometry
{
    struct Rect {};
}

BOOST_AUTO_TEST_SUITE(typename_of)

BOOST_AUTO_TEST_CASE(basic_test)
{
    #define CHECK_TYPENAME_DMIPYEG5PMH(...) \
        BOOST_CHECK_EQUAL(utils::typename_of(utils::rt_val<__VA_ARGS__>()), #__VA_ARGS__);

    CHECK_TYPENAME_DMIPYEG5PMH(unsigned char)
    CHECK_TYPENAME_DMIPYEG5PMH(signed char)
    CHECK_TYPENAME_DMIPYEG5PMH(unsigned short)
    CHECK_TYPENAME_DMIPYEG5PMH(short)
    CHECK_TYPENAME_DMIPYEG5PMH(unsigned int)
    CHECK_TYPENAME_DMIPYEG5PMH(int)
    CHECK_TYPENAME_DMIPYEG5PMH(unsigned long)
    CHECK_TYPENAME_DMIPYEG5PMH(long)
    CHECK_TYPENAME_DMIPYEG5PMH(unsigned long long)
    CHECK_TYPENAME_DMIPYEG5PMH(long long)
    CHECK_TYPENAME_DMIPYEG5PMH(char)
    CHECK_TYPENAME_DMIPYEG5PMH(wchar_t)
    CHECK_TYPENAME_DMIPYEG5PMH(char16_t)
    CHECK_TYPENAME_DMIPYEG5PMH(char32_t)
    CHECK_TYPENAME_DMIPYEG5PMH(float)
    CHECK_TYPENAME_DMIPYEG5PMH(double)
    CHECK_TYPENAME_DMIPYEG5PMH(long double)
    CHECK_TYPENAME_DMIPYEG5PMH(std::string)
    CHECK_TYPENAME_DMIPYEG5PMH(std::nullptr_t)
    CHECK_TYPENAME_DMIPYEG5PMH(std::vector<int>)
    CHECK_TYPENAME_DMIPYEG5PMH(std::tuple<int,float,double>)
    CHECK_TYPENAME_DMIPYEG5PMH(utils::variant<int,std::vector<std::string>,double>)
    CHECK_TYPENAME_DMIPYEG5PMH(std::array<Geometry::Rect,16>)
    CHECK_TYPENAME_DMIPYEG5PMH(Geometry::Rect)
    CHECK_TYPENAME_DMIPYEG5PMH(Geometry::Rect*)
    CHECK_TYPENAME_DMIPYEG5PMH(Geometry::Rect const*)
    CHECK_TYPENAME_DMIPYEG5PMH(int(*)[15])
    CHECK_TYPENAME_DMIPYEG5PMH(int*[15])


    #undef CHECK_TYPENAME_DMIPYEG5PMH
}

BOOST_AUTO_TEST_SUITE_END()

