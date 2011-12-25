//-------------------------------------------
// Smart pointer mechanism for Pango objects
//-------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**

``<utils/ext/pango.hpp>`` --- Pango RAII Support
================================================

This module overrides the default deleter for Pango types, to provide
:term:`RAII` via the ``utils::pango::unique_ptr<T>`` and
``utils::pango::shared_ptr<T>`` :term:`smart pointer`\ s, without providing an
explicit deleter.

*/

#ifndef EXT_PANGO_HPP_A9P2EVFZ8E6
#define EXT_PANGO_HPP_A9P2EVFZ8E6 1

#include <pango/pango.h>
#include "../memory.hpp"

namespace utils { namespace pango {

/**
Members
-------
*/

namespace xx_impl
{
    struct PangoDeallocator
    {
        #define DEFINE_RELEASE_METHOD_O6175SVG1MG(Type, method) \
            static void release(Pango##Type* x) noexcept { pango_##method##_free(x); }

        DEFINE_RELEASE_METHOD_O6175SVG1MG(Item, item)
        DEFINE_RELEASE_METHOD_O6175SVG1MG(Matrix, matrix)
        DEFINE_RELEASE_METHOD_O6175SVG1MG(GlyphString, glyph_string)
        DEFINE_RELEASE_METHOD_O6175SVG1MG(GlyphItem, glyph_item)
        DEFINE_RELEASE_METHOD_O6175SVG1MG(GlyphItemIter, glyph_item_iter)
        DEFINE_RELEASE_METHOD_O6175SVG1MG(FontDescription, font_description)
        DEFINE_RELEASE_METHOD_O6175SVG1MG(Color, color)
        DEFINE_RELEASE_METHOD_O6175SVG1MG(TabArray, tab_array)
        DEFINE_RELEASE_METHOD_O6175SVG1MG(LayoutIter, layout_iter)
        DEFINE_RELEASE_METHOD_O6175SVG1MG(ScriptIter, script_iter)

        #undef DEFINE_RELEASE_METHOD_O6175SVG1MG
    };
}

/**
.. type:: type utils::pango::unique_ptr<T> = utils::generic_unique_ptr<T, (unspecified)>

    Smart pointer type that asserts unique ownership to a Pango structure. Note
    that this will not handle Pango types that are GLib objects. Use
    :type:`utils::glib::unique_ptr\<T>` for those instead.
*/
UTILS_DEF_SMART_PTR_ALIAS(unique, xx_impl::PangoDeallocator)

}}

UTILS_DEF_SMART_PTR_STD_FUNCS(utils::pango, unique)

#endif

