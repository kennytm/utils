//-------------------------------------------
// utils/factory.cpp: Predicate-based factory
//-------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <utils/factory.hpp>

namespace utils {

factory_error::factory_error(std::vector<std::exception_ptr>&& exceptions)
    : exceptions(std::move(exceptions))
{
    std::string ret_str = "Factory creation failed, and the following exceptions were thrown in the process:\n";
    for (auto& exc : this->exceptions)
    {
        try
        {
            std::rethrow_exception(exc);
        }
        catch (const std::exception& e)
        {
            ret_str += e.what();
            ret_str.push_back('\n');
        }
    }
    _what = std::move(ret_str);
}

factory_error::~factory_error() noexcept {}

const char* factory_error::what() const noexcept
{
    return _what.c_str();
}

}

