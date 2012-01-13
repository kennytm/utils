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
    event_loop& get_main_loop();

/**
.. type:: type utils::event_handle
    :pod:

    An opaque POD type which can identify an event. It is guaranteed to be
    cheaply copyable.

    The handle is not usable across threads.

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

    .. function:: utils::event_handle listen(int fd, const std::function<void(int fd, utils::event_loop&, utils::event_handle)>& callback)

        Listen to the UNIX file descriptor *fd*. Calls the functor when *fd* has
        data to read.

    .. function:: utils::event_handle delay(std::chrono::duration<...> after, const std::function<void(bool& keep, utils::event_loop&, utils::event_handle)>& callback)
                  utils::event_handle delay(const std::function<void(bool& keep, utils::event_loop&, utils::event_handle)>& callback)

        Schedule a timer in the event loop. After a certain time interval, the
        *callback* will be called and the event will be removed. If *after* is
        not supplied, the functor will be called as soon as possible (but after
        this function has completed).

        .. warning::

            Do not call :func:`~utils::event_loop::cancel` on the handle! It
            will cause undefined behavior because the handle by default will be
            cancelled automatically, and cancelling twice will cause
            double-deletion.

        In the callback, the *keep* parameter can be set to ``true`` to
        reschedule the time-out event one more time.

    .. function:: utils::event_handle repeat(std::chrono::duration<...> interval, const std::function<void(utils::event_loop&, utils::event_handle)>& callback)
                  utils::event_handle repeat(const std::function<void(utils::event_loop&, utils::event_handle)>& callback)

        Schedule a repeated timer in the event loop. The function will be called
        repeatedly with the given interval.

    .. function:: utils::event_handle signal(int signum, const std::function<void(int signum, utils::event_loop&, utils::event_handle)>& callback)

        Watch for the specified signal (if it can be trapped). Calls the
        callback functor when the signal is raised.

    .. function:: void cancel(utils::event_handle handle)

        Cancel a previously scheduled event. The functor that is associated with
        that event will not be called afterward.

    .. function:: void run()

        Start running the event loop. The function returns only after
        :func:`~utils::event_loop::stop` is called or when all event handlers
        have been cancelled.

    .. function:: void stop()

        Stop the event loop. All scheduled events will be cancelled.

.. function:: utils::event_loop& utils::get_main_loop()

    Get the main loop for this thread.

*/
}


#if defined(UTILS_EVENT_LOOP_BACKEND_LIBEV)
#include "event_loop-libev.hpp"
#else
#error Please define one of the following: UTILS_EVENT_LOOP_BACKEND_LIBEV
#endif

namespace utils {

/**
.. type:: class utils::unique_event final
    :movable:
    :noncopyable:
    :default_constructible:

    A unique owner of an event handle. Once destroyed, the associated handle
    will be cancelled.
*/
class unique_event final
{
public:
    /**
    .. function:: unique_event(utils::event_loop& loop, utils::event_handle&& handle)

        Construct the unique owner of the given *handle*, which will be
        cancelled from the *loop* when this owner is destroyed.

        The *loop* must be valid throughout the lifetime of this object,
        otherwise an undefined behavior will happen.
    */
    unique_event(event_loop& loop, event_handle&& handle)
        : _loop(&loop), _handle(std::move(handle))
    {}

    unique_event()
        : _loop(nullptr)
    {}

    unique_event(unique_event&& other)
        : _loop(other._loop), _handle(std::move(other._handle))
    { other._loop = nullptr; }

    unique_event& operator=(unique_event&& other)
    {
        if (this != &other)
        {
            if (_loop)
                _loop->cancel(_handle);
            _loop = other._loop;
            _handle = std::move(other._handle);
            other._loop = nullptr;
        }
        return *this;
    }

    /**
    .. function:: inline const utils::event_handle& get() const noexcept

        Return the handle of this event.
    */
    const event_handle& get() const noexcept { return _handle; }

    /**
    .. function:: void cancel()

        Cancel the event.
    */
    void cancel()
    {
        if (_loop)
        {
            _loop->cancel(_handle);
            _loop = nullptr;
        }
    }

    /**
    .. function:: inline explicit operator bool() const noexcept

        Check if the event has been cancelled via this object.
    */
    explicit operator bool() const noexcept { return _loop != nullptr; }

    /**
    .. function:: void swap(utils::unique_event& other)

        Swap the owner of the two events.
    */
    void swap(unique_event& other)
    {
        using std::swap;    // Allows ADL if possible.
        swap(_loop, other._loop);
        swap(_handle, other._handle);
    }

    ~unique_event() { cancel(); }
    unique_event(const unique_event&) = delete;
    unique_event& operator=(const unique_event&) = delete;

private:
    event_loop* _loop;
    event_handle _handle;
};

}

namespace std
{
    template <>
    inline void swap(utils::unique_event& a, utils::unique_event& b)
    {
        a.swap(b);
    }
}

#endif


