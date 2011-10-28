#include <boost/test/unit_test.hpp>
#include <utils/partials_array.hpp>

struct Fn
{
    constexpr int operator()(int x) const noexcept { return x*x; }
};

struct FindTestData
{
    int input;
    int locate;
    int offset;
};

BOOST_AUTO_TEST_SUITE(PartialsCacher)

BOOST_AUTO_TEST_CASE(push_back)
{
    utils::partials_array<Fn> cacher;

    cacher.push_back(1);
    cacher.push_back(2);
    cacher.push_back(3);
    cacher.push_back(4);

    BOOST_CHECK_EQUAL(cacher.total(), 30);
}

BOOST_AUTO_TEST_CASE(update_find)
{
    auto cacher = utils::make_partials_array([](int x) { return x*x; });

    int vals[] = {2, 3, 4, 5};
    int partials[] = {0, 4, 13, 29, 54};
    int backs[] = {0, 4, 9, 16, 25};
    for (size_t i = 0; i <= 4; ++ i)
    {
        cacher.update(vals, vals + i);
        BOOST_CHECK_EQUAL(cacher.total(), partials[i]);
        BOOST_CHECK_EQUAL(cacher.back(), backs[i]);
    }

    BOOST_CHECK_EQUAL(cacher[0], 4);
    BOOST_CHECK_EQUAL(cacher[1], 9);
    BOOST_CHECK_EQUAL(cacher[2], 16);
    BOOST_CHECK_EQUAL(cacher[3], 25);

    FindTestData test_data[] =
    {
        {-5, 0, -5},
        {0, 0, 0},
        {1, 0, 1},
        {2, 0, 2},
        {4, 1, 0},
        {10, 1, 6},
        {29, 3, 0},
        {50, 3, 21},
        {70, 4, 16},
    };

    for (auto data : test_data)
    {
        auto res = cacher.find(vals, vals+4, data.input);
        BOOST_CHECK_EQUAL(res.first - vals, data.locate);
        BOOST_CHECK_EQUAL(res.second, data.offset);
    }
}

BOOST_AUTO_TEST_CASE(copy_move)
{
    utils::partials_array<Fn> cacher;
    int vals[] = {2, 3, 4, 5};
    cacher.update(vals, vals+4);

    auto copy = cacher;
    BOOST_CHECK_EQUAL(copy.total(), 54);
    BOOST_CHECK_EQUAL(copy.back(), 25);
    copy.push_back(6);
    BOOST_CHECK_EQUAL(copy.total(), 90);
    BOOST_CHECK_EQUAL(copy.back(), 36);
    BOOST_CHECK_EQUAL(cacher.total(), 54);
    BOOST_CHECK_EQUAL(cacher.back(), 25);

    auto move = std::move(copy);
    BOOST_CHECK_EQUAL(move.total(), 90);
    BOOST_CHECK_EQUAL(move.back(), 36);
    move.push_back(7);
    BOOST_CHECK_EQUAL(move.total(), 139);
    BOOST_CHECK_EQUAL(move.back(), 49);
    BOOST_CHECK_EQUAL(cacher.total(), 54);
    BOOST_CHECK_EQUAL(cacher.back(), 25);
}

BOOST_AUTO_TEST_SUITE_END()

