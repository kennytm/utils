#include <boost/test/unit_test.hpp>
#include <utils/memory.hpp>

struct MemChecker
{
    static void release(int* obj) noexcept
    {
        -- *obj;
        if (obj == 0)
            delete obj;
    }
    static void add_ref(int* obj) noexcept
    {
        ++ *obj;
    }
};

BOOST_AUTO_TEST_SUITE(memory)

BOOST_AUTO_TEST_CASE(shared_ptr_nullptr_compare)
{
    auto f = new int(1);

    utils::generic_shared_ptr<int, MemChecker> foo (f);

    BOOST_CHECK(!!foo);
    BOOST_CHECK(foo != nullptr);
    BOOST_CHECK(nullptr != foo);
    BOOST_CHECK_EQUAL(*f, 1);

    {
        auto bar = foo;
        BOOST_CHECK(!!bar);
        BOOST_CHECK(bar != nullptr);
        BOOST_CHECK(nullptr != bar);
        BOOST_CHECK_EQUAL(*f, 2);
    }

    BOOST_CHECK(!!foo);
    BOOST_CHECK(foo != nullptr);
    BOOST_CHECK(nullptr != foo);
    BOOST_CHECK_EQUAL(*f, 1);

    foo.reset();
    BOOST_CHECK(!foo);
    BOOST_CHECK(foo == nullptr);
    BOOST_CHECK(nullptr == foo);

    foo.reset(new int(1));
    BOOST_CHECK(!!foo);
    BOOST_CHECK(foo != nullptr);
    BOOST_CHECK(nullptr != foo);
}

BOOST_AUTO_TEST_SUITE_END()

