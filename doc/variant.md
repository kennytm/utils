utils::variant
==============

**utils::variant** is a rewrite of [boost::variant](http://www.boost.org/doc/libs/1_47_0/doc/html/variant.html) for C++11 support.

**utils::variant** support the following features over **boost::variant**:

* Proper move semantics
* Stricter, compile-time type checking

**boost::variant** has some features that will likely not ported:

* Recursive variant
* Reference members
* Boost.MPL
* C++03, and support for compilers other than gcc ≥4.6 (and clang when its C++11
  support becomes better).

**utils::variant** requires the following compiler and library features:

* Unrestricted union
* Variadic template
* Rvalue references
* ``constexpr`` functions (maybe lifted in the future)
* ``noexcept``
* ``decltype`` (maybe lifted in the future)
* Lambda functions (maybe lifted in the future)
* ``<type_traits>``
* ``static_assert``

Synopsis
--------

```c++
#include <cstdio>
#include <vector>
#include <utils/variant.hpp>

class print_content_visitor : public utils::static_visitor<void>
{
public:
    void operator()(const std::vector<int>& vec) const
    {
        for (int val : vec)
            printf("%d\n", val);
    }

    void operator()(const std::string& str) const
    {
        printf("[%s]\n", str.c_str());
    }
};

int main()
{
    std::vector<int> v {8, 9, 10, 12, 18, 24, 36, 48, 64, 72, 96};

    utils::variant<std::vector<int>, std::string> u = std::move(v);

    utils::apply_visitor(print_content_visitor(), u);

    return 0;
}
```

