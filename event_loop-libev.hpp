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
    struct io_entry;
    struct signal_entry;
    struct delay_entry;
    struct delay_imm_entry;
    struct repeat_entry;
    struct repeat_imm_entry;
}

typedef utils::variant<ev_io*,
                       ev_timer*,
                       ev_signal*,
                       std::list<xx_impl::delay_imm_entry>::iterator,
                       std::list<xx_impl::repeat_imm_entry>::iterator> event_handle;

namespace xx_impl
{
    typedef std::function<void(int fd, event_loop&, event_handle)> io_func;
    typedef std::function<void(int signum, event_loop&, event_handle)> signal_func;
    typedef std::function<void(bool& keep, event_loop&, event_handle)> delay_func;
    typedef std::function<void(event_loop&, event_handle)> repeat_func;

    struct io_entry
    {
        ev_io watcher;
        io_func callback;
    };

    struct signal_entry
    {
        ev_signal watcher;
        signal_func callback;
    };

    struct delay_entry
    {
        ev_timer watcher;
        delay_func callback;
    };

    struct delay_imm_entry
    {
        delay_func callback;
    };

    struct repeat_entry
    {
        ev_timer watcher;
        repeat_func callback;
    };

    struct repeat_imm_entry
    {
        repeat_func callback;
    };
}


class event_loop final
{
public:
    event_loop()
        : _loop(ev_loop_new(0))
    {
        ev_set_userdata(_loop, this);
        init_imm_watcher();
    }

    event_handle listen(int fd, const xx_impl::io_func& callback);
    event_handle signal(int signum, const xx_impl::signal_func& callback);

    template <typename R, typename P>
    event_handle delay(std::chrono::duration<R, P> after, const xx_impl::delay_func& callback)
    {
        using namespace std::chrono;
        auto seconds = duration_cast<duration<ev_tstamp>>(after).count();
        return delay_impl(seconds, callback);
    }

    template <typename R, typename P>
    event_handle repeat(std::chrono::duration<R, P> interval, const xx_impl::repeat_func& callback)
    {
        using namespace std::chrono;
        auto seconds = duration_cast<duration<ev_tstamp>>(interval).count();
        return repeat_impl(seconds, callback);
    }

    event_handle delay(const xx_impl::delay_func& callback)
    {
        return delay_imm_impl(callback);
    }

    event_handle repeat(const xx_impl::repeat_func& callback)
    {
        return repeat_imm_impl(callback);
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
    event_handle delay_impl(ev_tstamp after, const xx_impl::delay_func& callback);
    event_handle repeat_impl(ev_tstamp rep, const xx_impl::repeat_func& callback);
    event_handle delay_imm_impl(const xx_impl::delay_func& callback);
    event_handle repeat_imm_impl(const xx_impl::repeat_func& callback);

    void call_io(ev_io* watcher);
    void call_signal(ev_signal* watcher);
    void call_delay(ev_timer* watcher);
    void call_repeat(ev_timer* watcher);
    void call_imms();

    void start_imm_watcher();
    void try_stop_imm_watcher();
    void init_imm_watcher();

    struct ev_loop* _loop;
    ev_idle _imm_watcher;

    std::list<xx_impl::io_entry> _io_entries;
    std::list<xx_impl::signal_entry> _signal_entries;
    std::list<xx_impl::delay_entry> _delay_entries;
    std::list<xx_impl::delay_imm_entry> _delay_imm_entries;
    std::list<xx_impl::repeat_entry> _repeat_entries;
    std::list<xx_impl::repeat_imm_entry> _repeat_imm_entries;
    std::unordered_map<ev_io*, std::list<xx_impl::io_entry>::iterator> _io_map;
    std::unordered_map<ev_signal*, std::list<xx_impl::signal_entry>::iterator> _signal_map;
    std::unordered_map<ev_timer*, std::list<xx_impl::delay_entry>::iterator> _delay_map;
    std::unordered_map<ev_timer*, std::list<xx_impl::repeat_entry>::iterator> _repeat_map;
};

}

#endif

