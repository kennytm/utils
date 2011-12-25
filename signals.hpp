//---------------------------------------------
// utils::signals: Signals-and-slots for C++11
//---------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef SIGNALS_HPP_FX8WXO7WATS
#define SIGNALS_HPP_FX8WXO7WATS 1

/**

``<utils/signals.hpp>`` --- Signals and slots
=============================================

The module is mainly used as a backend-independent interop layer for signals and
slots in C++. Users may choose between different backends by defining a macro
before including the header:

* ``#define UTILS_SIGNALS_BACKEND_LIBSIGC`` -- Use libsigc++ as backend

*/

#include <type_traits>

#if defined(UTILS_SIGNALS_BACKEND_LIBSIGC)
#include "signals-libsigc++.inc.hpp"
#else
#error Please define one of the following: UTILS_SIGNALS_BACKEND_LIBSIGC

namespace utils {

/**
.. type:: struct utils::slot_connection<F>

    This structure represents a signal-slot connection.
*/
struct slot_connection
{
    /**
    .. function:: void disconnect()

        Disconnect the signal from the slot.
    */
    void disconnect();

    /**
    .. function:: void suspend()

        Suspend the connection, so that emitted signals will not reach the slot.
    */
    void suspend();

    /**
    .. function:: void resume()

        Resume the connection, so that emitted signals can reach the slot.
    */
    void resume();

    /**
    .. function:: bool is_connected() const

        Check whether the connection is still active.
    */
    bool is_connected() const;

    /**
    .. function:: bool is_suspended() const

        Check whether the connection has been suspended.
    */
    bool is_suspended() const;
};

/**
.. type:: struct utils::signal<F>

    Declare a signal which works like a function of type *F*.
*/
template <typename F>
struct signal
{
    static_assert(std::is_function<F>::value, "Please supply a function type.");

    /**
    .. function:: utils::slot_connection connect<T>(T&& slot)
                  utils::slot_connection operator+=(T&& slot)

        Connect the slot to the signal.
    */
    template <typename T>
    slot_connection<F> connect(T&& slot);

    template <typename T>
    slot_connection<F> operator+=(T&& slot)
    { return connect(std::forward<T>(slot)); }

    /**
    .. function:: auto emit<A...>(A&&... args)

        Emit the signal.
    */
    template <typename... A>
    typename std::result_of<F>::type emit(A&&... args);
};

}

#endif

#endif

