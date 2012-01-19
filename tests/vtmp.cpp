#include <boost/test/unit_test.hpp>
#include <utils/vtmp.hpp>

struct UnpackChecker
{
    template <typename T, typename U, typename V>
    void operator()(T x, U y, V z)
    {
        static_assert(std::is_same<T, int>::value, "not int");
        static_assert(std::is_same<U, const char*>::value, "not string");
        static_assert(std::is_same<V, double>::value, "not double");
        BOOST_CHECK_EQUAL(x, 4);
        BOOST_CHECK(strcmp(y, "z") == 0);
        BOOST_CHECK_EQUAL(z, 8.0);
    }
};

struct Plus1Func
{
    template <typename T>
    auto operator()(T value) const -> decltype(value + 1)
    {
        return value + 1;
    }
};

struct SizeofFunc
{
    template <typename T>
    size_t operator()(T) const noexcept
    {
        return sizeof(T);
    }
};

BOOST_AUTO_TEST_SUITE(variadic_templates)

BOOST_AUTO_TEST_CASE(iota)
{
    using v1 = utils::vtmp::iota<7>;
    using v2 = utils::vtmp::integers<0, 1, 2, 3, 4, 5, 6>;

    static_assert(std::is_same<v1, v2>::value, "unexpected.");

    using v3 = utils::vtmp::iota<0>;
    using v4 = utils::vtmp::integers<>;

    static_assert(std::is_same<v3, v4>::value, "unexpected.");

    using v5 = utils::vtmp::iota<1>;
    using v6 = utils::vtmp::integers<0>;

    static_assert(std::is_same<v5, v6>::value, "unexpected.");
}

BOOST_AUTO_TEST_CASE(unpack)
{
    auto tup = std::make_tuple(4, "z", 8.0);
    utils::tuple_apply(tup, UnpackChecker());
    utils::tuple_apply(std::move(tup), UnpackChecker());
}

BOOST_AUTO_TEST_CASE(map_and_construct)
{
    int x[2];
    auto tup = std::make_tuple(40, -12.5, "hello", 'j', x);
    auto mapped = utils::tuple_map(tup, Plus1Func());
    BOOST_CHECK_EQUAL(std::get<0>(mapped), 41);
    BOOST_CHECK_EQUAL(std::get<1>(mapped), -11.5);
    BOOST_CHECK(strcmp(std::get<2>(mapped), "ello") == 0);
    BOOST_CHECK_EQUAL(std::get<3>(mapped), 'k');
    BOOST_CHECK(std::get<4>(mapped) == x+1);

    auto sizes_tuple = utils::tuple_map(tup, SizeofFunc());
    auto sizes = utils::tuple_construct<std::array<size_t, 5>>(std::move(sizes_tuple));
    size_t expected_sizes[] = {sizeof(int), sizeof(double), sizeof(const char*), 1, sizeof(int*)};
    BOOST_CHECK_EQUAL_COLLECTIONS(sizes.begin(), sizes.end(),
                                  std::begin(expected_sizes), std::end(expected_sizes));
}

BOOST_AUTO_TEST_CASE(integers_push_back_test)
{
    typedef utils::vtmp::integers<1, 4, 9, 16> ints;
    typedef ints::push_back<5> ints_with_5;

    BOOST_CHECK((std::is_same<ints_with_5, utils::vtmp::integers<1, 4, 9, 16, 5>>::value));
}

BOOST_AUTO_TEST_SUITE_END()

