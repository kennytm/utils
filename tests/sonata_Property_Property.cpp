#include <string>
#include <type_traits>
#include <boost/test/unit_test.hpp>
#include <utils/property.hpp>

class Req35a1
{
    int _integer;
    double _floating;
    std::string _string;

public:
    int get_integer() const noexcept { return _integer; }
    double get_floating() const noexcept { return _floating; }
    std::string get_string() const { return _string; }

    void set_integer(int i) noexcept { _integer = i; }
    void set_floating(double d) noexcept { _floating = d; }
    void set_string(const std::string& s) noexcept { _string = s; }
    void set_string(std::string&& s) noexcept { _string = std::move(s); }

    UTILS_PROPERTIES(Req35a1, _string)
    {
        declprop::read_write_byval<int, &Req35a1::get_integer,
                                        &Req35a1::set_integer> integer;
        declprop::read_write_byval<double, &Req35a1::get_floating,
                                           &Req35a1::set_floating> floating;
        declprop::read_write_movable<std::string, &Req35a1::get_string,
                                                  &Req35a1::set_string,
                                                  &Req35a1::set_string> string;
    };
};

struct NonComparableNonOStreamable
{
    int z;

    bool operator==(const NonComparableNonOStreamable&) = delete;
};

struct Test
{
    NonComparableNonOStreamable get_test() const noexcept
    {
        return {3};
    }

    UTILS_PROPERTIES_FOR_EMPTY_CLASS(Test)
    {
        declprop::read_only<NonComparableNonOStreamable, &Test::get_test> test;
    };
};

BOOST_AUTO_TEST_SUITE(property)

BOOST_AUTO_TEST_CASE(assignment_and_compare)
{
    Req35a1 inst;
    inst.integer = 678;
    inst.floating = 34.0;
    inst.string = "world";

    BOOST_CHECK_EQUAL(inst.integer, 678);
    BOOST_CHECK_EQUAL(inst.floating, 34.0);
    BOOST_CHECK_EQUAL(inst.string, "world");
}

BOOST_AUTO_TEST_CASE(operators)
{
    Req35a1 inst;
    inst.integer = 54;
    BOOST_CHECK_EQUAL(inst.integer, 54);

    inst.integer += 44;
    BOOST_CHECK_EQUAL(inst.integer, 98);

    inst.integer = inst.integer + 12;
    BOOST_CHECK_EQUAL(inst.integer, 110);

    inst.integer -= 'a';
    BOOST_CHECK_EQUAL(inst.integer, 13);

    inst.integer *= 10;
    BOOST_CHECK_EQUAL(inst.integer, 130);

    inst.integer /= 2;
    BOOST_CHECK_EQUAL(inst.integer, 65);

    inst.integer ^= 3;
    BOOST_CHECK_EQUAL(inst.integer, 66);

    inst.integer <<= 2;
    BOOST_CHECK_EQUAL(inst.integer, 264);

    inst.integer >>= 6;
    BOOST_CHECK_EQUAL(inst.integer, 4);

    inst.integer |= 14;
    BOOST_CHECK_EQUAL(inst.integer, 14);

    inst.integer &= 7;
    BOOST_CHECK_EQUAL(inst.integer, 6);

    BOOST_CHECK_EQUAL(inst.integer, 6.0);
    BOOST_CHECK_NE(inst.integer, 9.2);
    BOOST_CHECK_LT(inst.integer, 9.2);
    BOOST_CHECK_LE(inst.integer, 9.2);
    BOOST_CHECK_GT(inst.integer, 1.7);
    BOOST_CHECK_GE(inst.integer, 1.7);

    BOOST_CHECK_EQUAL(inst.integer, 6);
    BOOST_CHECK_NE(inst.integer, 9);
    BOOST_CHECK_LT(inst.integer, 9);
    BOOST_CHECK_LE(inst.integer, 9);
    BOOST_CHECK_GT(inst.integer, 1);
    BOOST_CHECK_GE(inst.integer, 1);

    BOOST_CHECK(inst.integer == 6);
    BOOST_CHECK(inst.integer != 9);
    BOOST_CHECK(inst.integer < 9);
    BOOST_CHECK(inst.integer <= 9);
    BOOST_CHECK(inst.integer > 1);
    BOOST_CHECK(inst.integer >= 1);
}

BOOST_AUTO_TEST_CASE(undef_opers_check)
{
    Test t;
    NonComparableNonOStreamable m = t.test;
    BOOST_CHECK_EQUAL(m.z, 3);
}

BOOST_AUTO_TEST_CASE(movable)
{
    struct Checker
    {
        bool moved;
        bool copied;

        Checker& operator=(Checker&&) { moved = true; return *this; }
        Checker& operator=(const Checker&) { copied = true; return *this; }

        Checker() : moved(false), copied(false) {}
        Checker(const Checker&) : moved(false), copied(true) {}
        Checker(Checker&&) : moved(true), copied(false) {}
    };

    struct Container
    {
        Checker _c;

        Checker get_c() const { return _c; }
        void set_c(const Checker& other) { _c = other; }
        void set_c(Checker&& other) { _c = std::move(other); }

        UTILS_PROPERTIES(Container, _c)
        {
            declprop::read_write_movable<Checker, &Container::get_c,
                                                  &Container::set_c,
                                                  &Container::set_c> c;
        };
    };

    Container k;
    BOOST_REQUIRE(!k._c.moved);
    BOOST_REQUIRE(!k._c.copied);

    Checker c;
    k.c = c;
    BOOST_CHECK(!k._c.moved);
    BOOST_CHECK(k._c.copied);
    k._c.copied = false;

    k.c = std::move(c);
    BOOST_CHECK(k._c.moved);
    BOOST_CHECK(!k._c.copied);
    k._c.moved = false;

    Container k2;
    k2 = k;
    BOOST_CHECK(!k2._c.moved);
    BOOST_CHECK(k2._c.copied);
    k2._c.copied = false;

    k2 = std::move(k);
    BOOST_CHECK(k2._c.moved);
    BOOST_CHECK(!k2._c.copied);
}

BOOST_AUTO_TEST_SUITE_END()

