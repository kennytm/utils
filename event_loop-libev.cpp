#include <cstdio>
#include <algorithm>
#include <pthread.h>
#include "event_loop.hpp"

#ifdef UTILS_EVENT_LOOP_BACKEND_LIBEV

namespace utils {

//{{{ Structors of xx_impl::entry

namespace xx_impl
{
    event_entry::event_entry() {}

#define PERFORM_ON_FUNCS_I9X9CQ21RH(MACRO) \
    switch (type) \
    { \
        case event_type::io: \
        case event_type::signal: \
            MACRO(io_); \
            break; \
        \
        case event_type::io_simple: \
        case event_type::signal_simple: \
            MACRO(io_simple_); \
            break; \
        \
        case event_type::delay: \
        case event_type::delay_imm: \
            MACRO(delay_); \
            break; \
        \
        case event_type::repeat: \
        case event_type::repeat_imm: \
            MACRO(repeat_); \
            break; \
        \
        case event_type::delay_simple: \
        case event_type::delay_imm_simple: \
        case event_type::repeat_simple: \
        case event_type::repeat_imm_simple: \
            MACRO(timer_simple_); \
            break; \
        \
        default: \
            break; \
    }

#define DESTROY_FUNC_OREMUDH6SC(Prefix) \
    Prefix##callback.~Prefix##func()

#define MOVE_FUNC_LTPS9ZCP03S(Prefix) \
    new(&Prefix##callback) Prefix##func(std::move(other.Prefix##callback))


    event_entry::~event_entry()
    {
        PERFORM_ON_FUNCS_I9X9CQ21RH(DESTROY_FUNC_OREMUDH6SC)
    }

    event_entry::event_entry(event_entry&& other)
        : type(other.type)
    {
        PERFORM_ON_FUNCS_I9X9CQ21RH(MOVE_FUNC_LTPS9ZCP03S)

        switch (type)
        {
            case event_type::io:
            case event_type::io_simple:
                io_watcher = std::move(other.io_watcher);
                break;

            case event_type::signal:
            case event_type::signal_simple:
                signal_watcher = std::move(other.signal_watcher);
                break;

            case event_type::delay:
            case event_type::delay_simple:
            case event_type::repeat:
            case event_type::repeat_simple:
                timer_watcher = std::move(other.timer_watcher);
                break;

            default:
                break;
        }
    }
}

//}}}

//{{{ Event registration.

namespace xx_impl
{
    static constexpr int watcher_type_io = 1;
    static constexpr int watcher_type_signal = 2;
    static constexpr int watcher_type_timer = 3;
}

#define STORE_EVENT_EEBHGKGNTWN(EntryType, CallbackType) \
    auto handle = _event_counter ++; \
    \
    xx_impl::event_entry entry; \
    new(&entry.CallbackType##_callback) xx_impl::CallbackType##_func(std::move(callback)); \
    entry.type = xx_impl::event_type::EntryType; \
    \
    _events.push_front(std::move(entry)); \
    _map.emplace(handle, _events.begin())

#define START_LIBEV_EVENT_4LEMEBLRCSS(LibEVType, ...) \
    auto watcher = &(_events.front().LibEVType##_watcher); \
    ev_##LibEVType##_init(watcher, [](struct ev_loop* loop, ev_##LibEVType* watcher, int) \
    { \
        auto this_ = static_cast<event_loop*>(ev_userdata(loop)); \
        auto handle = reinterpret_cast<event_handle>(watcher->data); \
        this_->call(handle, watcher, xx_impl::watcher_type_##LibEVType); \
    }, __VA_ARGS__); \
    watcher->data = reinterpret_cast<void*>(handle); \
    ev_##LibEVType##_start(_loop, watcher)

event_handle event_loop::listen_impl(int fd, xx_impl::io_func&& callback)
{
    STORE_EVENT_EEBHGKGNTWN(io, io);
    START_LIBEV_EVENT_4LEMEBLRCSS(io, fd, EV_READ);
    return handle;
}

event_handle event_loop::listen_impl(int fd, xx_impl::io_simple_func&& callback)
{
    STORE_EVENT_EEBHGKGNTWN(io_simple, io_simple);
    START_LIBEV_EVENT_4LEMEBLRCSS(io, fd, EV_READ);
    return handle;
}

event_handle event_loop::signal_impl(int signum, xx_impl::io_func&& callback)
{
    STORE_EVENT_EEBHGKGNTWN(signal, io);
    START_LIBEV_EVENT_4LEMEBLRCSS(signal, signum);
    return handle;
}

event_handle event_loop::signal_impl(int signum, xx_impl::io_simple_func&& callback)
{
    STORE_EVENT_EEBHGKGNTWN(signal_simple, io_simple);
    START_LIBEV_EVENT_4LEMEBLRCSS(signal, signum);
    return handle;
}

event_handle event_loop::delay_impl(ev_tstamp after, xx_impl::delay_func&& callback)
{
    STORE_EVENT_EEBHGKGNTWN(delay, delay);
    START_LIBEV_EVENT_4LEMEBLRCSS(timer, after, after);
    return handle;
}

event_handle event_loop::delay_impl(ev_tstamp after, xx_impl::timer_simple_func&& callback)
{
    STORE_EVENT_EEBHGKGNTWN(delay_simple, timer_simple);
    START_LIBEV_EVENT_4LEMEBLRCSS(timer, after, after);
    return handle;
}

event_handle event_loop::repeat_impl(ev_tstamp rep, xx_impl::repeat_func&& callback)
{
    STORE_EVENT_EEBHGKGNTWN(repeat, repeat);
    START_LIBEV_EVENT_4LEMEBLRCSS(timer, rep, rep);
    return handle;
}

event_handle event_loop::repeat_impl(ev_tstamp rep, xx_impl::timer_simple_func&& callback)
{
    STORE_EVENT_EEBHGKGNTWN(repeat_simple, timer_simple);
    START_LIBEV_EVENT_4LEMEBLRCSS(timer, rep, rep);
    return handle;
}

#undef START_LIBEV_EVENT_4LEMEBLRCSS

event_handle event_loop::delay_imm_impl(xx_impl::delay_func&& callback)
{
    STORE_EVENT_EEBHGKGNTWN(delay_imm, delay);
    start_imm_watcher();
    return handle;
}

event_handle event_loop::delay_imm_impl(xx_impl::timer_simple_func&& callback)
{
    STORE_EVENT_EEBHGKGNTWN(delay_imm_simple, timer_simple);
    start_imm_watcher();
    return handle;
}

event_handle event_loop::repeat_imm_impl(xx_impl::repeat_func&& callback)
{
    STORE_EVENT_EEBHGKGNTWN(repeat_imm, repeat);
    start_imm_watcher();
    return handle;
}

event_handle event_loop::repeat_imm_impl(xx_impl::timer_simple_func&& callback)
{
    STORE_EVENT_EEBHGKGNTWN(repeat_imm_simple, timer_simple);
    start_imm_watcher();
    return handle;
}

#undef STORE_EVENT_EEBHGKGNTWN

//}}}

//{{{ Invocation

void event_loop::call(event_handle handle, void* watcher, int watcher_type)
{
    auto map_it = _map.find(handle);
    if (map_it != _map.end())
    {
        call_entry(handle, map_it);
        return;
    }

    switch (watcher_type)
    {
        case xx_impl::watcher_type_io:
            ev_io_stop(_loop, static_cast<ev_io*>(watcher));
            break;

        case xx_impl::watcher_type_signal:
            ev_signal_stop(_loop, static_cast<ev_signal*>(watcher));
            break;

        case xx_impl::watcher_type_timer:
            ev_timer_stop(_loop, static_cast<ev_timer*>(watcher));
            break;

        default:
            break;
    }
}

void event_loop::call_entry(event_handle handle, decltype(_map.begin()) map_it)
{
    auto& entry = *(map_it->second);
    switch (entry.type)
    {
        case xx_impl::event_type::io:
        case xx_impl::event_type::signal:
        {
            int param;
            if (entry.type == xx_impl::event_type::io)
                param = entry.io_watcher.fd;
            else
                param = entry.signal_watcher.signum;

            entry.io_callback(param, *this, handle);
            break;
        }

        case xx_impl::event_type::io_simple:
        case xx_impl::event_type::signal_simple:
        {
            int param;
            if (entry.type == xx_impl::event_type::io_simple)
                param = entry.io_watcher.fd;
            else
                param = entry.signal_watcher.signum;

            entry.io_simple_callback(param);
            break;
        }

        case xx_impl::event_type::delay:
        case xx_impl::event_type::delay_imm:
        {
            bool keep = false;
            entry.delay_callback(keep, *this, handle);
            if (keep)
                break;
            else if (entry.type == xx_impl::event_type::delay)
                goto erase_delay_handle;
            else
                goto erase_delay_imm_handle;
        }

        case xx_impl::event_type::delay_simple:
            entry.timer_simple_callback();
    erase_delay_handle:
            if (_map.find(handle) == map_it)
            {
                ev_timer_stop(_loop, &(entry.timer_watcher));
                _events.erase(map_it->second);
                _map.erase(map_it);
            }
            break;

        case xx_impl::event_type::delay_imm_simple:
            entry.timer_simple_callback();
    erase_delay_imm_handle:
            if (_map.find(handle) == map_it)
            {
                _events.erase(map_it->second);
                _map.erase(map_it);
            }
            break;

        case xx_impl::event_type::repeat:
        case xx_impl::event_type::repeat_imm:
            entry.repeat_callback(*this, handle);
            break;

        case xx_impl::event_type::repeat_simple:
        case xx_impl::event_type::repeat_imm_simple:
            entry.timer_simple_callback();
            break;

        default:
            break;
    }
}

//}}}

//{{{ Immediate callbacks:

void event_loop::init_imm_watcher()
{
    auto& watcher = _imm_watcher;
    ev_idle_init(&watcher, [](struct ev_loop* loop, ev_idle* w, int)
    {
        auto this_ = static_cast<event_loop*>(ev_userdata(loop));
        this_->call_imms();
    });
}

void event_loop::start_imm_watcher()
{
    auto& watcher = _imm_watcher;
    if (ev_is_active(&watcher))
        return;
    ev_idle_start(_loop, &watcher);
}

static inline bool is_imm(const xx_impl::event_entry& entry) noexcept
{
    auto type = entry.type;
    return type == xx_impl::event_type::delay_imm
        || type == xx_impl::event_type::repeat_imm
        || type == xx_impl::event_type::delay_imm_simple
        || type == xx_impl::event_type::repeat_imm_simple;
}

void event_loop::call_imms()
{
    auto cur = _map.begin();
    auto end = _map.end();
    while (cur != end)
    {
        auto prev = cur;
        ++ cur;

        if (is_imm(*prev->second))
            call_entry(prev->first, prev);
    }

    try_stop_imm_watcher();
}

void event_loop::try_stop_imm_watcher()
{
    if (std::none_of(_events.begin(), _events.end(), is_imm))
    {
        auto& watcher = _imm_watcher;
        ev_idle_stop(_loop, &watcher);
    }
}

//}}}

void event_loop::cancel(event_handle handle)
{
    bool do_try_stop_imm = false;

    auto map_it = _map.find(handle);
    if (map_it != _map.end())
    {
        auto& entry = *(map_it->second);
        switch (entry.type)
        {
            case xx_impl::event_type::io:
            case xx_impl::event_type::io_simple:
                ev_io_stop(_loop, &(entry.io_watcher));
                break;

            case xx_impl::event_type::signal:
            case xx_impl::event_type::signal_simple:
                ev_signal_stop(_loop, &(entry.signal_watcher));
                break;

            case xx_impl::event_type::delay:
            case xx_impl::event_type::delay_simple:
            case xx_impl::event_type::repeat:
            case xx_impl::event_type::repeat_simple:
                ev_timer_stop(_loop, &(entry.timer_watcher));
                break;

            case xx_impl::event_type::delay_imm:
            case xx_impl::event_type::delay_imm_simple:
            case xx_impl::event_type::repeat_imm:
            case xx_impl::event_type::repeat_imm_simple:
                do_try_stop_imm = true;
                break;

            default:
                break;
        }

        _events.erase(map_it->second);
        _map.erase(map_it);
        if (do_try_stop_imm)
            try_stop_imm_watcher();
    }
}

event_loop& get_main_loop()
{
#if defined(__GNUC__) && __GNUC__ <= 4
    static __thread event_loop* loop;
    static __thread pthread_once_t once;
    pthread_once(&once, []{ loop = new event_loop(); });
    return *loop;
#else
    static thread_local event_loop loop;
    return loop;
#endif
}

}

#endif

