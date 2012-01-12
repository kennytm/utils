#include <pthread.h>
#include <cstdio>
#include "event_loop.hpp"

#ifdef UTILS_EVENT_LOOP_BACKEND_LIBEV

namespace utils {

xx_impl::io_entry::io_entry(xx_impl::io_function&& functor)
    : functor(std::move(functor))
{}

xx_impl::imm_entry::imm_entry(xx_impl::timer_function&& functor, event_action default_action)
    : functor(std::move(functor)),
      default_action(default_action)
{}

event_handle event_loop::listen(int fd, xx_impl::io_function functor)
{
    _io_list.emplace_front(std::move(functor));

    auto watcher = &(_io_list.front().io);
    _io_map.emplace(watcher, _io_list.begin());

    ev_io_init(watcher, [](struct ev_loop* loop, ev_io* w, int)
    {
        auto this_ = static_cast<event_loop*>(ev_userdata(loop));
        auto map_iter = this_->_io_map.find(w);
        if (map_iter == this_->_io_map.end())
        {
            ev_io_stop(loop, w);
            return;
        }

        auto list_iter = map_iter->second;
        bool is_inactive = !ev_is_active(w);
        auto action = is_inactive ? event_action::cancel : event_action::keep;
        list_iter->functor(w->fd, action);
        if (action == event_action::cancel || is_inactive)
        {
            if (!is_inactive)
                ev_io_stop(loop, w);
            this_->_io_list.erase(list_iter);
            this_->_io_map.erase(map_iter);
        }

    }, fd, EV_READ);
    ev_io_start(_loop, watcher);

    return watcher;
}

ev_timer* event_loop::timer_impl(ev_tstamp repeat,
                                 event_action default_action,
                                 xx_impl::timer_function&& functor)
{
    _timer_list.emplace_front(std::move(functor), default_action);

    auto watcher = &(_timer_list.front().timer);
    _timer_map.emplace(watcher, _timer_list.begin());

    ev_timer_init(watcher, [](struct ev_loop* loop, ev_timer* w, int)
    {
        auto this_ = static_cast<event_loop*>(ev_userdata(loop));
        auto map_iter = this_->_timer_map.find(w);
        if (map_iter == this_->_timer_map.end())
        {
            ev_timer_stop(loop, w);
            return;
        }

        auto list_iter = map_iter->second;
        auto action = list_iter->default_action;
        list_iter->functor(action);
        if (action == event_action::cancel)
        {
            ev_timer_stop(loop, w);
            this_->_timer_list.erase(list_iter);
            this_->_timer_map.erase(map_iter);
        }

    }, repeat, repeat);
    ev_timer_start(_loop, watcher);

    return watcher;
}

std::list<xx_impl::imm_entry>::iterator event_loop::imm_impl(event_action default_action,
                                                             xx_impl::timer_function&& functor)
{
    add_check_watcher();

    _imm_list.emplace_front(std::move(functor), default_action);
    return _imm_list.begin();
}

void event_loop::init_imm_watcher()
{
    auto& watcher = _imm_watcher;
    ev_idle_init(&watcher, [](struct ev_loop* loop, ev_idle* w, int)
    {
        auto this_ = static_cast<event_loop*>(ev_userdata(loop));
        auto& imm_list = this_->_imm_list;

        auto cur = imm_list.begin();
        auto end = imm_list.end();
        while (cur != end)
        {
            auto entry = cur;
            ++ cur;

            auto action = entry->default_action;
            entry->functor(action);

            if (action == event_action::cancel)
                imm_list.erase(entry);
        }

        if (imm_list.empty())
        {
            ev_idle_stop(loop, w);
        }
    });
}

void event_loop::add_check_watcher()
{
    auto& w = _imm_watcher;

    if (ev_is_active(&w))
        return;

    ev_idle_start(_loop, &w);
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
                _io_list.erase(map_iter->second);
                _io_map.erase(map_iter);
            }
        },
        [this](ev_timer* watcher)
        {
            auto timer_map_iter = _timer_map.find(watcher);
            if (timer_map_iter != _timer_map.end())
            {
                ev_timer_stop(_loop, watcher);
                _timer_list.erase(timer_map_iter->second);
                _timer_map.erase(timer_map_iter);
            }
        },
        [this](std::list<xx_impl::imm_entry>::iterator imm_iter)
        {
            _imm_list.erase(imm_iter);
            if (_imm_list.empty())
            {
                auto& w = _imm_watcher;
                ev_idle_stop(_loop, &w);
            }
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

