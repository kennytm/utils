//----------------------------------------------------------------------
// utils/ext/xterm256.hpp: Convert colors to and from xterm-256 indices
//----------------------------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**

``<utils/ext/xterm256.hpp>`` --- xterm 256 color support
========================================================

.. seealso:: http://www.mudpedia.org/wiki/Xterm_256_colors

*/

#ifndef XTERM256_HPP_CUF9RSPXIM
#define XTERM256_HPP_CUF9RSPXIM 1

#include <array>
#include <cstdint>
#include <iosfwd>

namespace utils { namespace xterm256 {

/**
.. type:: struct utils::xterm256::color
    :ostream:

    Represents a 24-bit color.
*/
struct color
{
    /**
    .. data:: uint8_t r

        The red component.
    */
    uint8_t r;

    /**
    .. data:: uint8_t g

        The green component.
    */
    uint8_t g;

    /**
    .. data:: uint8_t b

        The blue component.
    */
    uint8_t b;
};

/**
.. function:: utils::xterm256::color utils::xterm256::get_color(uint8_t index) noexcept

    Get the RGB color components of a given xterm-256 color index.
*/
color get_color(uint8_t index) noexcept;

/**
.. function:: uint8_t utils:xterm256::get_index(utils::xterm256::color color) noexcept

    Get the xterm-256 color index closest to the given color in terms of
    absolute difference in RGB components.
*/
uint8_t get_index(color color) noexcept;

constexpr bool operator==(color a, color b) noexcept
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}
constexpr bool operator!=(color a, color b) noexcept
{
    return a.r != b.r || a.g != b.g || a.b != b.b;
}
std::ostream& operator<<(std::ostream&, color);


}}

#endif

