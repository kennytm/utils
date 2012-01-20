//--------------------------------------------------------
// typename: Convert a type to a readable name in runtime
//--------------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cctype>
#include <utility>
#include <string>
#include <cxxabi.h>

namespace utils { namespace xx_impl {

static inline bool is_ident_char(char c) noexcept
{
    return isalnum(c) || c == '_';
}

std::string remove_insignificant_spaces(std::string input)
{
    /*
    A space is significant when removing it will form a new token. Therefore, a
    space is significant only when between:

        - Two alphanumerics
        - Around " and '

    Trigraphs and strings are disregarded (they won't appear in a type anyway).
    */

    size_t len = input.length();
    size_t j = 0;
    for (size_t i = 0; i < len; ++ i)
    {
        if (isblank(input[i]))
        {
            if (i == 0 || i == len-1)
                continue;

            char prev = input[i-1];
            char next = input[i+1];
            if (!is_ident_char(prev) || !is_ident_char(next))
                continue;
        }

        input[j++] = input[i];
    }

    input.erase(j, len - j);
    return input;
}

}

std::string typeinfo_name(const std::type_info& type)
{
    int status;
    char* real_name = abi::__cxa_demangle(type.name(), 0, 0, &status);
    std::string res (real_name);
    free(real_name);
    return xx_impl::remove_insignificant_spaces(res);
}

}


