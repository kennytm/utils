#if __GNUC__
#include <cxxabi.h>
#endif
#include <typeinfo>
#include <string>
#include <type_traits>
#include <functional>
#include <boost/test/unit_test.hpp>
#include <utils/traits.hpp>
#include <map>

using utils::function_traits;

BOOST_AUTO_TEST_SUITE(traits)

// Using BOOST_AUTO_TEST_CASE_TEMPLATE to test the argument types is going to be
// a mess :)

template <typename T, typename U>
static std::string print_two_types()
{
    const char* t_name = typeid(T).name();
    const char* u_name = typeid(U).name();
    #if __GNUC__
        int status;
        t_name = abi::__cxa_demangle(t_name, NULL, NULL, &status);
        u_name = abi::__cxa_demangle(u_name, NULL, NULL, &status);
    #endif
    return t_name + std::string(" != ") + u_name;
}

#define BOOST_CHECK_TYPE_EQUAL(...) \
    BOOST_CHECK_MESSAGE((std::is_same<__VA_ARGS__>::value), (print_two_types<__VA_ARGS__>()))

BOOST_AUTO_TEST_CASE(normal_function_test_case)
{
    typedef function_traits<int(long, double*, const char&&, std::nullptr_t)> FT;

    BOOST_CHECK_TYPE_EQUAL(FT::result_type, int);
    BOOST_CHECK_EQUAL(FT::arity, 4);
    BOOST_CHECK_TYPE_EQUAL(FT::arg<0>::type, long);
    BOOST_CHECK_TYPE_EQUAL(FT::arg<1>::type, double*);
    BOOST_CHECK_TYPE_EQUAL(FT::arg<2>::type, const char&&);
    BOOST_CHECK_TYPE_EQUAL(FT::arg<3>::type, std::nullptr_t);
}

BOOST_AUTO_TEST_CASE(function_ptr_test_case)
{
    int f(std::pair<int, void*>, volatile double*, long(*)(size_t));

    typedef function_traits<decltype(&f)> FT;

    BOOST_CHECK_TYPE_EQUAL(FT::result_type, int);
    BOOST_CHECK_EQUAL(FT::arity, 3);
    BOOST_CHECK_TYPE_EQUAL(FT::arg<0>::type, std::pair<int, void*>);
    BOOST_CHECK_TYPE_EQUAL(FT::arg<1>::type, volatile double*);
    BOOST_CHECK_TYPE_EQUAL(FT::arg<2>::type, long(*)(size_t));

}

BOOST_AUTO_TEST_CASE(function_reference_test_case)
{
    typedef function_traits<void(&)()> FT;

    BOOST_CHECK_TYPE_EQUAL(FT::result_type, void);
    BOOST_CHECK_EQUAL(FT::arity, 0);
}

BOOST_AUTO_TEST_CASE(function_rvalue_reference_test_case)
{
    typedef function_traits<int(*(&&)(const long&&))[]> FT;

    BOOST_CHECK_TYPE_EQUAL(FT::result_type, int(*)[]);
    BOOST_CHECK_EQUAL(FT::arity, 1);
    BOOST_CHECK_TYPE_EQUAL(FT::arg<0>::type, const long&&);
}

BOOST_AUTO_TEST_CASE(member_pointer_test_case)
{
    struct S
    {
        short p(const char*) noexcept;
        unsigned int q(std::pair<int, int>, int) const volatile throw();
        bool operator==(const S&) const;
        virtual void r() volatile {}
        virtual ~S() {}
    };

    typedef function_traits<decltype(&S::p)> FTp;
    BOOST_CHECK_TYPE_EQUAL(FTp::owner_type, S&);
    BOOST_CHECK_TYPE_EQUAL(FTp::result_type, short);
    BOOST_CHECK_EQUAL(FTp::arity, 1);
    BOOST_CHECK_TYPE_EQUAL(FTp::arg<0>::type, const char*);

    typedef function_traits<decltype(&S::q)> FTq;
    BOOST_CHECK_TYPE_EQUAL(FTq::owner_type, const volatile S&);
    BOOST_CHECK_TYPE_EQUAL(FTq::result_type, unsigned int);
    BOOST_CHECK_EQUAL(FTq::arity, 2);
    BOOST_CHECK_TYPE_EQUAL(FTq::arg<0>::type, std::pair<int, int>);
    BOOST_CHECK_TYPE_EQUAL(FTq::arg<1>::type, int);

    typedef function_traits<decltype(&S::operator==)> FTe;
    BOOST_CHECK_TYPE_EQUAL(FTe::owner_type, const S&);
    BOOST_CHECK_TYPE_EQUAL(FTe::result_type, bool);
    BOOST_CHECK_EQUAL(FTe::arity, 1);
    BOOST_CHECK_TYPE_EQUAL(FTe::arg<0>::type, const S&);

    typedef function_traits<decltype(&S::r)> FTr;
    BOOST_CHECK_TYPE_EQUAL(FTr::owner_type, volatile S&);
    BOOST_CHECK_TYPE_EQUAL(FTr::result_type, void);
    BOOST_CHECK_EQUAL(FTr::arity, 0);

    const auto& sr = &S::r;
    typedef function_traits<decltype(sr)> FTr2;
    BOOST_CHECK_TYPE_EQUAL(FTr2::owner_type, volatile S&);
    BOOST_CHECK_TYPE_EQUAL(FTr2::result_type, void);
    BOOST_CHECK_EQUAL(FTr2::arity, 0);
}

BOOST_AUTO_TEST_CASE(function_object_test_case)
{
    typedef function_traits<std::function<long(int*, char** const*)>> FT;

    BOOST_CHECK_TYPE_EQUAL(FT::result_type, long);
    BOOST_CHECK_EQUAL(FT::arity, 2);
    BOOST_CHECK_TYPE_EQUAL(FT::arg<0>::type, int*);
    BOOST_CHECK_TYPE_EQUAL(FT::arg<1>::type, char** const*);
}

BOOST_AUTO_TEST_CASE(other_function_object_test_case)
{
    typedef function_traits<std::plus<int>> FTplus;
    BOOST_CHECK_TYPE_EQUAL(FTplus::result_type, int);
    BOOST_CHECK_EQUAL(FTplus::arity, 2);
    BOOST_CHECK_TYPE_EQUAL(FTplus::arg<0>::type, const int&);
    BOOST_CHECK_TYPE_EQUAL(FTplus::arg<1>::type, const int&);

    typedef function_traits<std::equal_to<std::string>> FTequal;
    BOOST_CHECK_TYPE_EQUAL(FTequal::result_type, bool);
    BOOST_CHECK_EQUAL(FTequal::arity, 2);
    BOOST_CHECK_TYPE_EQUAL(FTequal::arg<0>::type, const std::string&);
    BOOST_CHECK_TYPE_EQUAL(FTequal::arg<1>::type, const std::string&);

    typedef function_traits<std::logical_not<void*>> FTnot;
    BOOST_CHECK_TYPE_EQUAL(FTnot::result_type, bool);
    BOOST_CHECK_EQUAL(FTnot::arity, 1);
    BOOST_CHECK_TYPE_EQUAL(FTnot::arg<0>::type, void* const&);

    typedef function_traits<decltype(std::not2(std::plus<double>()))> FTnotplus;
    BOOST_CHECK_TYPE_EQUAL(FTnotplus::result_type, bool);
    BOOST_CHECK_EQUAL(FTnotplus::arity, 2);
    BOOST_CHECK_TYPE_EQUAL(FTnotplus::arg<0>::type, const double&);
    BOOST_CHECK_TYPE_EQUAL(FTnotplus::arg<1>::type, const double&);
}

/*
int bind_test_case_f(double, char, unsigned char&, long&&) { return 0; }

BOOST_AUTO_TEST_CASE(bind_test_case)
{
    using namespace std::placeholders;

    auto bx = std::bind(bind_test_case_f, _2, 'a', _1, _3);
    typedef function_traits<decltype(bx)> FTbind;
    BOOST_CHECK_TYPE_EQUAL(FTbind::result_type, int);
    BOOST_CHECK_EQUAL(FTbind::arity, 3);
    BOOST_CHECK_TYPE_EQUAL(FTbind::arg<0>::type, unsigned char&);
    BOOST_CHECK_TYPE_EQUAL(FTbind::arg<1>::type, double);
    BOOST_CHECK_TYPE_EQUAL(FTbind::arg<2>::type, long&&);
}
*/

BOOST_AUTO_TEST_CASE(lambda_test_case)
{
    double x = 4;
    auto lambda = [=](int y) { return x + y; };

    typedef function_traits<decltype(lambda)> FT;
    BOOST_CHECK_TYPE_EQUAL(FT::result_type, double);
    BOOST_CHECK_EQUAL(FT::arity, 1);
    BOOST_CHECK_TYPE_EQUAL(FT::arg<0>::type, int);

    auto lambda2 = [&](float z) mutable -> void* { x = z; return nullptr; };
    typedef function_traits<decltype(lambda2)> FT2;
    BOOST_CHECK_TYPE_EQUAL(FT2::result_type, void*);
    BOOST_CHECK_EQUAL(FT2::arity, 1);
    BOOST_CHECK_TYPE_EQUAL(FT2::arg<0>::type, float);
}

BOOST_AUTO_TEST_CASE(mem_fn_test_case)
{
    struct K
    {
        int p;
        double q(float r) const { return 0.0; }
    };

    auto pp = std::mem_fn(&K::p);
    auto qq = std::mem_fn(&K::q);

    typedef function_traits<decltype(pp)> FTp;
    typedef function_traits<decltype(qq)> FTq;

    BOOST_CHECK_TYPE_EQUAL(FTp::result_type, int);
    BOOST_CHECK_EQUAL(FTp::arity, 1);
    BOOST_CHECK_TYPE_EQUAL(FTp::arg<0>::type, K*);
    BOOST_CHECK_TYPE_EQUAL(FTq::result_type, double);
    BOOST_CHECK_EQUAL(FTq::arity, 2);
    BOOST_CHECK_TYPE_EQUAL(FTq::arg<0>::type, const K*);
    BOOST_CHECK_TYPE_EQUAL(FTq::arg<1>::type, float);
}

struct ForwardLike_S
{
    char f(int&) const noexcept { return 'L'; }
    char f(const int&) const noexcept { return 'C'; }
    char f(int&&) const noexcept { return 'R'; }

    template <typename U>
    char g(U&& u) const noexcept
    {
        return this->f(utils::forward_like<U>(u.z));
    }
};

BOOST_AUTO_TEST_CASE(forward_like_test_case)
{
    struct T
    {
        int z;
        static T get_t() noexcept { return T(); }
        static const T get_const_t() noexcept { return T(); }
    };

    ForwardLike_S s;

    T t;
    BOOST_CHECK_EQUAL(s.g(t), 'L');
    BOOST_CHECK_EQUAL(s.g(T::get_t()), 'R');
    BOOST_CHECK_EQUAL(s.g(T::get_const_t()), 'C');
}

BOOST_AUTO_TEST_CASE(copy_cv_test_case)
{
    BOOST_CHECK_TYPE_EQUAL(utils::copy_cv<const int, double>::type, const double);
    BOOST_CHECK_TYPE_EQUAL(utils::copy_cv<int, const double>::type, double);
    BOOST_CHECK_TYPE_EQUAL(utils::copy_cv<int* volatile, long long>::type, volatile long long);
    BOOST_CHECK_TYPE_EQUAL(utils::copy_cv<const char*, int>::type, int);
}

BOOST_AUTO_TEST_CASE(pointee_test_case)
{
    BOOST_CHECK_TYPE_EQUAL(utils::pointee<int*>::type, int);
    BOOST_CHECK_TYPE_EQUAL(utils::pointee<int**>::type, int*);
    BOOST_CHECK_TYPE_EQUAL(utils::pointee<const int*>::type, const int);
    BOOST_CHECK_TYPE_EQUAL(utils::pointee<std::string::const_iterator>::type, const char);
    typedef std::map<short, char> Map;
    BOOST_CHECK_TYPE_EQUAL(utils::pointee<Map::reverse_iterator>::type, Map::value_type);
}

BOOST_AUTO_TEST_CASE(member_function_type)
{
    struct S;

    typedef utils::function_traits<float(double)> FType;
    BOOST_CHECK_TYPE_EQUAL(FType::member_function_type<S>, float(S::*)(double));
    BOOST_CHECK_TYPE_EQUAL(FType::member_function_type<const S&>, float(S::*)(double) const);
    BOOST_CHECK_TYPE_EQUAL(FType::member_function_type<volatile S>, float(S::*)(double) volatile);
    BOOST_CHECK_TYPE_EQUAL(FType::member_function_type<const volatile S*>, float(S::*)(double) const volatile);
}

BOOST_AUTO_TEST_SUITE_END()


