#include <boost/test/unit_test.hpp>
#include <utils/factory.hpp>

namespace
{
    class S
    {
    public:
        virtual bool is_even() const = 0;
        virtual ~S() {}
    };

    class SEven final : public S
    {
    public:
        static std::unique_ptr<S> create(int x)
        {
            if (x % 2 == 0 && x != 0)
                return std::unique_ptr<S>(new SEven());
            else
                return nullptr;
        }

        virtual bool is_even() const override { return true; }
    };

    class SOdd final : public S
    {
    public:
        static std::unique_ptr<S> create(int x)
        {
            if (x % 2 == 1)
                return std::unique_ptr<S>(new SOdd());
            else
                return nullptr;
        }

        virtual bool is_even() const override { return false; }
    };
}

typedef utils::factory<std::unique_ptr<S>(int)> SFactoryRegistrar;

SFactoryRegistrar s_even_registrar (&SEven::create);
SFactoryRegistrar s_odd_registrar (&SOdd::create);

BOOST_AUTO_TEST_SUITE(test_factory)

BOOST_AUTO_TEST_CASE(test_factory)
{
    BOOST_CHECK(SFactoryRegistrar::create(4)->is_even());
    BOOST_CHECK(!SFactoryRegistrar::create(7)->is_even());
    BOOST_CHECK(!SFactoryRegistrar::create(0));
}

BOOST_AUTO_TEST_SUITE_END()

