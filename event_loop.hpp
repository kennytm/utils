//-----------------------------------------
// utils::event_loop: Event loop for C++11
//-----------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EVENT_LOOP_HPP_SOO2D3Q1T98
#define EVENT_LOOP_HPP_SOO2D3Q1T98 1

/**

``<utils/event_loop.hpp>`` --- Event loop
=========================================

The module is mainly used as a backend-independent interop layer for event loops
in C++. Users may choose between different backends by defining a macro before
including the header:

* ``#define UTILS_EVENT_LOOP_BACKEND_LIBEV``

    * Use `libev <http://software.schmorp.de/pkg/libev.html>` as backend

Synopsis
--------

Schedule a repeating event::

    #include <utils/event_loop.hpp>

    int main()
    {
        using utils::main_loop;

        main_loop.repeat(std::chrono::seconds(5), [](utils::event_loop& loop, utils::event_handle handle)
        {
            thread_local int i = 0;
            printf("%d\n", i);
            ++ i;
            if (i >= 10)
            {
                main_loop.stop();
            }
        });

        main_loop.run();
        return 0;
    }

*/

#include <chrono>

/**
Members
-------
*/

namespace utils
{
    class event_loop;

    enum class event_action
    {
        keep,
        cancel
    };

    event_loop& get_main_loop();
}


#if defined(UTILS_EVENT_LOOP_BACKEND_LIBEV)
#include "event_loop-libev.hpp"
#else
#error Please define one of the following: UTILS_EVENT_LOOP_BACKEND_LIBEV

/**
.. type:: type utils::event_handle
    :pod:

    An opaque POD type which can identify an event. It is guaranteed to be
    cheaply copyable.

    The handle is not usable across threads.

.. type:: enum class event_action

    What to do when the event is handled.

    .. member:: keep

        Keep the event running. This is the default for file read events and
        repeated events.

    .. member:: cancel

        Cancel the event. This is the default for delayed events.

.. type:: class utils::event_loop final
    :noncopyable:
    :nonmovable:
    :default_constructible:

    Represents an event loop.

    Every thread which can run user code has an associated worker object known
    as event loop. This could be a libev ev_loop, a Windows message loop, a
    Darwin (Mac OS X) NSRunLoop, or just another worker thread.

    An event loop must support 3 operations:

    1. Listening on a UNIX file descriptor, and notify the coding thread when it
       is available.
    2. Host timers, and notify the coding thread on timeout.
    3. Host immediate timers, and notify the coding thread immediately after the
       event loop ticks.

    An event loop is **not** thread-safe (even if it may use threads in the
    implementation). An instance of event_loop should stay in a single thread.

    .. function:: event_loop()

        Construct an event loop.

    .. function:: utils::event_handle listen(int fd, const std::function<void(int fd, event_action&)>& functor)

        Listen to the UNIX file descriptor *fd*. Calls the functor when *fd* has
        data to read.

        .. note:: You can only stop a listener via event_action, but not revive it.

    .. function:: utils::event_handle delay(std::chrono::duration<...> after, const std::function<void(event_action&)>& functor)
                  utils::event_handle delay(const std::function<void(event_action&)>& functor)

        Schedule a timer in the event loop. After a certain time interval, the
        *function* will be called and the event will be removed. If *after* is
        not supplied, the functor will be called as soon as possible (but after
        this function has completed).

    .. function:: utils::event_handle repeat<F>(std::chrono::duration<...> interval, const std::function<void(event_action&)>& functor)
                  utils::event_handle repeat<F>(const std::function<void(event_action&)>& functor)

        Schedule a repeated timer in the event loop. The function will be called
        repeatedly with the given interval.

    .. function:: void cancel(utils::event_handle handle)

        Cancel a previously scheduled event. The functor that is associated with
        that event will not be called.

    .. function:: void run()

        Start running the event loop. The function returns only after
        :func:`~utils::event_loop::stop` is called.

    .. function:: void stop()

        Stop the event loop. All scheduled events will be cancelled.

.. function:: utils::event_loop& utils::get_main_loop()

    Get the main loop for this thread.

*/

#endif

#endif


