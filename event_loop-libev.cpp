#include <pthread.h>
#include <cstdio>
#include "event_loop.hpp"

#ifdef UTILS_EVENT_LOOP_BACKEND_LIBEV

namespace utils {

template <typename List, typename Functor>
typename List::iterator add_entry(List& entry_list, const Functor& callback)
{
    typename List::value_type new_entry;
    new_entry.callback = callback;
    entry_list.push_front(std::move(new_entry));
    return entry_list.begin();
}

//{{{ libev callbacks registration:

#define DEFINE_LIBEV_WATCHER_VTMWGQFDIO9(FuncProto, Kind, WatcherType, ...) \
    event_handle event_loop::FuncProto \
    { \
        auto entry_it = add_entry(_##Kind##_entries, callback); \
        auto watcher = &(entry_it->watcher); \
        _##Kind##_map.emplace(watcher, std::move(entry_it)); \
        \
        WatcherType##_init(watcher, [](struct ev_loop* loop, WatcherType* watcher, int) \
        { \
            auto this_ = static_cast<event_loop*>(ev_userdata(loop)); \
            this_->call_##Kind(watcher); \
        }, __VA_ARGS__); \
        WatcherType##_start(_loop, watcher); \
        \
        return watcher; \
    }

DEFINE_LIBEV_WATCHER_VTMWGQFDIO9(
    listen(int fd, const xx_impl::io_func& callback),
    io, ev_io, fd, EV_READ
)
DEFINE_LIBEV_WATCHER_VTMWGQFDIO9(
    delay_impl(ev_tstamp after, const xx_impl::delay_func& callback),
    delay, ev_timer, after, after
)
DEFINE_LIBEV_WATCHER_VTMWGQFDIO9(
    repeat_impl(ev_tstamp rep, const xx_impl::repeat_func& callback),
    repeat, ev_timer, rep, rep
)
DEFINE_LIBEV_WATCHER_VTMWGQFDIO9(
    signal(int signum, const xx_impl::signal_func& callback),
    signal, ev_signal, signum
)

#undef DEFINE_LIBEV_WATCHER_VTMWGQFDIO9

//}}}

//{{{ libev callbacks invocation:

void event_loop::call_io(ev_io* watcher)
{
    auto map_it = _io_map.find(watcher);
    if (map_it != _io_map.end())
        map_it->second->callback(watcher->fd, *this, watcher);
    else
        ev_io_stop(_loop, watcher);
}

void event_loop::call_signal(ev_signal* watcher)
{
    auto map_it = _signal_map.find(watcher);
    if (map_it != _signal_map.end())
        map_it->second->callback(watcher->signum, *this, watcher);
    else
        ev_signal_stop(_loop, watcher);
}

void event_loop::call_delay(ev_timer* watcher)
{
    auto map_it = _delay_map.find(watcher);
    if (map_it != _delay_map.end())
    {
        bool keep = false;
        auto entry_it = map_it->second;
        entry_it->callback(keep, *this, watcher);

        if (keep)
            return;

        _delay_map.erase(map_it);
        _delay_entries.erase(entry_it);
    }
    ev_timer_stop(_loop, watcher);
}

void event_loop::call_repeat(ev_timer* watcher)
{
    auto map_it = _repeat_map.find(watcher);
    if (map_it != _repeat_map.end())
        map_it->second->callback(*this, watcher);
    else
        ev_timer_stop(_loop, watcher);
}

//}}}

//{{{ Immediate callbacks:

event_handle event_loop::delay_imm_impl(const xx_impl::delay_func& callback)
{
    start_imm_watcher();
    return add_entry(_delay_imm_entries, callback);
}

event_handle event_loop::repeat_imm_impl(const xx_impl::repeat_func& callback)
{
    start_imm_watcher();
    return add_entry(_repeat_imm_entries, callback);
}

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

void event_loop::call_imms()
{
    auto delay_cur = _delay_imm_entries.begin();
    auto delay_end = _delay_imm_entries.end();
    while (delay_cur != delay_end)
    {
        auto entry_it = delay_cur;
        ++ delay_cur;

        bool keep = false;
        entry_it->callback(keep, *this, entry_it);
        if (keep)
            continue;

        _delay_imm_entries.erase(entry_it);
    }

    auto repeat_cur = _repeat_imm_entries.begin();
    auto repeat_end = _repeat_imm_entries.end();
    while (repeat_cur != repeat_end)
    {
        auto entry_it = repeat_cur;
        ++ repeat_cur;

        entry_it->callback(*this, entry_it);
    }

    try_stop_imm_watcher();
}

void event_loop::try_stop_imm_watcher()
{
    if (_delay_imm_entries.empty() && _repeat_imm_entries.empty())
    {
        auto& watcher = _imm_watcher;
        ev_idle_stop(_loop, &watcher);
    }
}

void event_loop::cancel(event_handle handle)
{
    case_of(handle,
        [this](ev_io* watcher)
        {
            auto map_iter = _io_map.find(watcher);
            if (map_iter != _io_map.end())
            {
                ev_io_stop(_loop, watcher);
                _io_entries.erase(map_iter->second);
                _io_map.erase(map_iter);
            }
        },
        [this](ev_signal* watcher)
        {
            auto map_iter = _signal_map.find(watcher);
            if (map_iter != _signal_map.end())
            {
                ev_signal_stop(_loop, watcher);
                _signal_entries.erase(map_iter->second);
                _signal_map.erase(map_iter);
            }
        },
        [this](ev_timer* watcher)
        {
            ev_timer_stop(_loop, watcher);

            auto repeat_map_it = _repeat_map.find(watcher);
            if (repeat_map_it != _repeat_map.end())
            {
                _repeat_entries.erase(repeat_map_it->second);
                _repeat_map.erase(repeat_map_it);
                return;
            }

            auto delay_map_it = _delay_map.find(watcher);
            if (delay_map_it != _delay_map.end())
            {
                _delay_entries.erase(delay_map_it->second);
                _delay_map.erase(delay_map_it);
            }
        },
        [this](std::list<xx_impl::delay_imm_entry>::iterator entry_it)
        {
            _delay_imm_entries.erase(entry_it);
            try_stop_imm_watcher();
        },
        [this](std::list<xx_impl::repeat_imm_entry>::iterator entry_it)
        {
            _repeat_imm_entries.erase(entry_it);
            try_stop_imm_watcher();
        }
    );
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

