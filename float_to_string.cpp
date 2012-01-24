//--------------------------
// Floating point to string
//--------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <utils/float_to_string.hpp>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>

namespace utils {

static std::string to_string_impl(long double value, int sigfigs)
{
    if (isnanl(value))
        return "NaN";
    else if (std::signbit(value))
        return "-" + to_string_impl(-value, sigfigs);
    else if (value == std::numeric_limits<long double>::infinity())
        return "Infinity";
    else if (value == 0)
        return "0.0";

    // we should probably use dtoa...

    char result[64];
    snprintf(result, sizeof(result), "%#.*Lg", sigfigs, value);

    // find consecutive trailing zeros.
    bool should_check_consecutive_zero = false;
    size_t consecutive_zero_begin = 0, consecutive_zero_end = 0;
    size_t i = 0;
    while (true)
    {
        switch (result[i++])
        {
            case '\0':
                goto outside;

            case 'e':
                should_check_consecutive_zero = false;
                break;

            case '0':
                if (should_check_consecutive_zero)
                {
                    ++ consecutive_zero_end;
                }
                break;

            case '.':
                should_check_consecutive_zero = true;
                // fall-through

            default:
                if (should_check_consecutive_zero)
                {
                    consecutive_zero_begin = i;
                    consecutive_zero_end = i;
                }
                break;
        }
    }
outside:

    // remove those zeros.
    if (result[consecutive_zero_begin-1] == '.')
        ++ consecutive_zero_begin;
    memmove(result + consecutive_zero_begin, result + consecutive_zero_end, i - consecutive_zero_end);
    return result;
}

std::string to_string(long double value)
{
    return to_string_impl(value, std::numeric_limits<long double>::digits10);
}

std::string to_string(double value)
{
    return to_string_impl(value, std::numeric_limits<double>::digits10);
}

std::string to_string(float value)
{
    return to_string_impl(value, std::numeric_limits<float>::digits10);
}

}

