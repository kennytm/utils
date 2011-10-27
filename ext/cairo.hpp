//----------------------------------------------------
// Smart pointer mechanism for cairo graphics objects
//----------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


#ifndef EXT_CAIRO_HPP_4MII6FAREFC
#define EXT_CAIRO_HPP_4MII6FAREFC

#include <cairo.h>
#include "../memory.hpp"

namespace utils { namespace cairo {

namespace xx_impl
{
    struct CairoDellocator
    {
        #define DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(middle) \
            static void add_ref(cairo##middle##_t* x) noexcept \
            { cairo##middle##_reference(x); } \
            static void release(cairo##middle##_t* x) noexcept \
            { cairo##middle##_destroy(x); } \
            static long use_count(cairo##middle##_t* x) noexcept \
            { cairo##middle##_get_reference_count(x); }

        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD()
        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(_pattern)
        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(_region)
        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(_font_face)
        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(_scaled_font)
        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(_device)
        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(_surface)

        #undef DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD
    };
}

UTILS_DEF_SMART_PTR_ALIAS(unique, CairoDellocator)
UTILS_DEF_SMART_PTR_ALIAS(shared, CairoDellocator)

}}

UTILS_DEF_SMART_PTR_STD_FUNCS(utils::cairo, unique)
UTILS_DEF_SMART_PTR_STD_FUNCS(utils::cairo, shared)

#endif

