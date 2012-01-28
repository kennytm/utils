#include <stdexcept>
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

        static std::shared_ptr<S> create_with_exception(int x)
        {
            if (x % 2 == 0 && x != 0)
                return std::make_shared<SEven>();
            else
                throw std::logic_error("x should be even and nonzero");
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

        static std::shared_ptr<S> create_with_exception(int x)
        {
            if (x % 2 == 1)
                return std::make_shared<SOdd>();
            else
                throw std::logic_error("x should be odd");
        }

        virtual bool is_even() const override { return false; }
    };
}

typedef utils::factory<std::unique_ptr<S>(int)> SFactoryRegistrar;
typedef utils::factory<std::shared_ptr<S>(int)> SWithExceptionFactoryRegistrar;

static SFactoryRegistrar s_even_registrar (&SEven::create);
static SFactoryRegistrar s_odd_registrar (&SOdd::create);
static SWithExceptionFactoryRegistrar swe_even_registrar (&SEven::create_with_exception);
static SWithExceptionFactoryRegistrar swe_odd_registrar (&SOdd::create_with_exception);

BOOST_AUTO_TEST_SUITE(test_factory)

BOOST_AUTO_TEST_CASE(test_factory)
{
    BOOST_CHECK(SFactoryRegistrar::create(4)->is_even());
    BOOST_CHECK(!SFactoryRegistrar::create(7)->is_even());
    BOOST_CHECK(!SFactoryRegistrar::create(0));
}

BOOST_AUTO_TEST_CASE(test_factory_with_exception)
{
    BOOST_CHECK(SWithExceptionFactoryRegistrar::create(4)->is_even());
    BOOST_CHECK(!SWithExceptionFactoryRegistrar::create(7)->is_even());
    BOOST_CHECK_THROW(SWithExceptionFactoryRegistrar::create(0), utils::factory_error);
}



BOOST_AUTO_TEST_SUITE_END()

