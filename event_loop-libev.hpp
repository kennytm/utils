#ifndef EVENT_LOOP_LIBEV_HPP_U7KG92FBR1M
#define EVENT_LOOP_LIBEV_HPP_U7KG92FBR1M

#include <list>
#include <functional>
#include <unordered_map>
#include <tuple>
#include <ev.h>
#include "event_loop.hpp"
#include "variant.hpp"

namespace utils {

namespace xx_impl
{
    typedef std::function<void(event_action&)> timer_function;
    typedef std::function<void(int, event_action&)> io_function;

    struct imm_entry
    {
        timer_function functor;
        event_action default_action;

        imm_entry(timer_function&& functor, event_action default_action);
    };
    struct timer_entry : imm_entry
    {
        ev_timer timer;

        timer_entry(timer_function&& functor, event_action default_action)
            : imm_entry(std::move(functor), default_action)
        {}
    };
    struct io_entry
    {
        io_function functor;
        ev_io io;

        io_entry(io_function&& functor);
    };
}

typedef utils::variant<ev_io*, ev_timer*, std::list<xx_impl::imm_entry>::iterator> event_handle;

class event_loop final
{
public:
    event_loop()
        : _loop(ev_loop_new(0))
    {
        ev_set_userdata(_loop, this);
    }

    event_handle listen(int fd, xx_impl::io_function functor);

    template <typename R, typename P>
    event_handle delay(std::chrono::duration<R, P> after, xx_impl::timer_function functor)
    {
        using namespace std::chrono;
        auto seconds = duration_cast<duration<ev_tstamp>>(after).count();
        return timer_impl(seconds, event_action::cancel, std::move(functor));
    }

    template <typename R, typename P>
    event_handle repeat(std::chrono::duration<R, P> interval, xx_impl::timer_function functor)
    {
        using namespace std::chrono;
        auto seconds = duration_cast<duration<ev_tstamp>>(interval).count();
        return timer_impl(seconds, event_action::keep, std::move(functor));
    }

    event_handle delay(xx_impl::timer_function functor)
    {
        return imm_impl(event_action::cancel, std::move(functor));
    }

    event_handle repeat(xx_impl::timer_function functor)
    {
        return imm_impl(event_action::keep, std::move(functor));
    }

    void cancel(event_handle handle);

    void run()
    {
        ev_run(_loop, 0);
    }

    void stop()
    {
        ev_break(_loop, EVBREAK_ONE);
    }

    struct ev_loop* get_libev_loop() const noexcept { return _loop; }

    ~event_loop()
    {
        ev_loop_destroy(_loop);
    }

private:
    ev_timer* timer_impl(ev_tstamp repeat,
                         event_action default_action,
                         xx_impl::timer_function&& functor);
    std::list<xx_impl::imm_entry>::iterator imm_impl(event_action default_action,
                                                     xx_impl::timer_function&& functor);
    void add_check_watcher();

    struct ev_loop* _loop;
    ev_idle _imm_watcher;

    std::list<xx_impl::io_entry> _io_list;
    std::list<xx_impl::timer_entry> _timer_list;
    std::list<xx_impl::imm_entry> _imm_list;
    std::unordered_map<ev_io*, std::list<xx_impl::io_entry>::iterator> _io_map;
    std::unordered_map<ev_timer*, std::list<xx_impl::timer_entry>::iterator> _timer_map;
};

}

#endif

