//------------------------------------------
// Smart pointer mechanism for GLib objects
//------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**

``<utils/ext/glib.hpp>`` --- GLib RAII Support
================================================

This module overrides the default deleter for GLib types, to provide
:term:`RAII` via the ``utils::glib::unique_ptr<T>`` and
``utils::glib::shared_ptr<T>`` :term:`smart pointer`\ s, without providing an
explicit deleter.

*/

#ifndef EXT_GLIB_HPP_GMXTHOT1A0G
#define EXT_GLIB_HPP_GMXTHOT1A0G

#include <glib-object.h>
#include "../memory.hpp"

namespace utils { namespace glib {

/**
Members
-------
*/

namespace xx_impl
{
    struct GLibDeallocator
    {
        static void add_ref(gpointer object) noexcept { g_object_ref(object); }
        static void release(gpointer object) noexcept { g_object_unref(object); }
    };
}

/**
.. type:: type utils::glib::unique_ptr<T> = utils::generic_unique_ptr<T, (unspecified)>

    Smart pointer type that asserts unique ownership to a GLib GObject, assuming
    it is not floating.
*/
UTILS_DEF_SMART_PTR_ALIAS(unique, xx_impl::GLibDeallocator)

/**
.. type:: type utils::glib::shared_ptr<T> = utils::generic_shared_ptr<T, (unspecified)>

    Smart pointer type that asserts shared ownership to a GLib GObject, assuming
    it is not floating.
*/
UTILS_DEF_SMART_PTR_ALIAS(shared, xx_impl::GLibDeallocator)

}}

UTILS_DEF_SMART_PTR_STD_FUNCS(utils::glib, unique)
UTILS_DEF_SMART_PTR_STD_FUNCS(utils::glib, shared)

#endif

