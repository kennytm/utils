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

* ``#define UTILS_SIGNALS_BACKEND_LIBSIGC``

    * Use `libsigc++ <http://libsigc.sourceforge.net/>` as backend (not
      thread-safe)

* ``#define UTILS_SIGNALS_BACKEND_SINGLE_THREADED``

    * Use a backend defined in this library using standard C++11 for
      single-threaded environment (not thread-safe)

Synopsis
--------

Defining a signal is simply adding a data member::

    #include <utils/signals.hpp>

    class Controller
    {
    public:
        utils::signal<void(int x, int y)> on_mouse_move;
        utils::signal<void(int x, int y)> on_touch_move;

        void move_touch_to(int x, int y) const;
    };

Connecting slots to a signal::

    int main()
    {
        Controller controller;

        // Connecting a function to a slot.
        controller.on_touch_move += [](int x, int y)
        {
            printf("Finger moved to %d %d\n", x, y);
        };
        controller.on_mouse_move += [](int x, int y)
        {
            printf("Mouse moved to %d %d\n", x, y);
        };

        // Forward the signal to another signal.
        controller.on_touch_move += controller.on_mouse_move;

        controller.move_touch_to(1, 2);

        return 0;
    }

Emitting a singal::

    void Controller::move_touch_to(int x, int y) const
    {
        on_touch_move.emit(x, y);
    }

*/

#include <type_traits>

#if defined(UTILS_SIGNALS_BACKEND_LIBSIGC)
#include "signals-libsigc++.inc.hpp"
#elif defined(UTILS_SIGNALS_BACKEND_SINGLE_THREADED)
#include "signals-single_threaded.inc.hpp"
#else
#error Please define one of the following: UTILS_SIGNALS_BACKEND_LIBSIGC, UTILS_SIGNALS_BACKEND_SINGLE_THREADED

namespace utils {

/**
.. type:: struct utils::slot_connection<F>
    :default_constructible:
    :movable:
    :noncopyable:

    This structure represents a signal-slot connection.
*/
struct slot_connection
{
    /**
    .. function:: void disconnect()

        Disconnect the signal from the slot. After calling this method, the slot
        will become invalid, and all mutating methods in this instance will
        become no-op.
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
.. type:: struct utils::signal<R(A...)>
    :default_constructible:
    :movable:
    :copyable:

    Declare a signal which works like a function accepting arguments *A* and
    returning *R*. The return type *R* should be either ``void`` or a
    default-constructible type.

    Multiple function objects with signature compatible with *R(A...)* can be
    connected to a single signal. When the signal is emitted, all these
    connected function objects will be called with the same argument.

    If the return type *R* of this signal is not ``void``, the return value of
    the last connected and active function will be used.
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
    .. function:: bool empty() const noexcept

        Check if the signal has no connected slots.
    */
    bool empty() const noexcept;

    /**
    .. function:: R emit(A... args) const

        Emit the signal.
    */
    template <typename... Args>
    typename std::result_of<F>::type emit(Args&&... args) const;
};

}

#endif

#endif

