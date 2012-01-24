//--------------------------
// Floating point to string
//--------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**
``<utils/float_to_string.hpp>`` --- Convert floating point number to string
===========================================================================
*/

#ifndef FLOAT_TO_STRING_HPP_VM6YL68OWW
#define FLOAT_TO_STRING_HPP_VM6YL68OWW 1

#include <string>

namespace utils {

/**
Members
-------

.. function:: std::string utils::to_string(long double value)
              std::string utils::to_string(double value)
              std::string utils::to_string(float value)

    Convert the value to string accurately.
*/
std::string to_string(long double value);
std::string to_string(double value);
std::string to_string(float value);

}

#endif

