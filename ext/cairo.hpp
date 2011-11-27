//----------------------------------------------------
// Smart pointer mechanism for cairo graphics objects
//----------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**

``<utils/ext/cairo.hpp>`` --- cairo RAII Support
================================================

This module overrides the default deleter for cairo types, to provide
:term:`RAII` via the ``utils::cairo::unique_ptr<T>`` and
``utils::cairo::shared_ptr<T>`` :term:`smart pointer`\ s, without providing an
explicit deleter.

Synopsis
--------

Usage::

    #include <cmath>
    #include <memory>
    #include <utils/ext/cairo.hpp>

    void draw_unit_circle(cairo_surface_t* surface)
    {
        utils::cairo::unique_ptr<cairo_t> context (cairo_create(surface));
        cairo_arc(context.get(),
                  /\*xc*\/0.0, /\*yc*\/0.0, /\*radius*\/1.0,
                  /\*angle1*\/0.0, /\*angle2*\/2*M_PI);
        // cairo_destroy(context.get()) is called implicitly here
    }

*/

#ifndef EXT_CAIRO_HPP_4MII6FAREFC
#define EXT_CAIRO_HPP_4MII6FAREFC

#include <cairo.h>
#include "../memory.hpp"

namespace utils { namespace cairo {

/**
Members
-------
*/

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
            { return cairo##middle##_get_reference_count(x); }

        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD()
        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(_pattern)
      //DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(_region)
        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(_font_face)
        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(_scaled_font)
        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(_device)
        DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD(_surface)

        static void add_ref(cairo_region_t* x) noexcept
        { cairo_region_reference(x); }
        static void release(cairo_region_t* x) noexcept
        { cairo_region_destroy(x); }

        #undef DEFINE_CAIRO_REFCOUNT_METHODS_0BSUGSC2MTPD
    };
}

/**
.. type:: type utils::cairo::unique_ptr<T> = utils::generic_unique_ptr<T, (unspecified)>

    Smart pointer type that asserts unique ownership to a cairo object.
*/
UTILS_DEF_SMART_PTR_ALIAS(unique, xx_impl::CairoDellocator)

/**
.. type:: type utils::cairo::shared_ptr<T> = utils::generic_shared_ptr<T, (unspecified)>

    Smart pointer type that asserts shared ownership to a cairo object.
*/
UTILS_DEF_SMART_PTR_ALIAS(shared, xx_impl::CairoDellocator)

}}

UTILS_DEF_SMART_PTR_STD_FUNCS(utils::cairo, unique)
UTILS_DEF_SMART_PTR_STD_FUNCS(utils::cairo, shared)

#endif

