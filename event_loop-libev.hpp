#ifndef EVENT_LOOP_LIBEV_HPP_U7KG92FBR1M
#define EVENT_LOOP_LIBEV_HPP_U7KG92FBR1M

#include <list>
#include <functional>
#include <unordered_map>
#include <tuple>
#include <ev.h>
#include "traits.hpp"
#include "event_loop.hpp"

namespace utils {

typedef uintptr_t event_handle;

namespace xx_impl
{
    enum class event_type
    {
        io,
        io_simple,
        signal,
        signal_simple,
        delay,
        delay_simple,
        delay_imm,
        delay_imm_simple,
        repeat,
        repeat_simple,
        repeat_imm,
        repeat_imm_simple
    };

    typedef std::function<void(int, event_loop&, event_handle)> io_func;
    typedef std::function<void(bool& keep, event_loop&, event_handle)> delay_func;
    typedef std::function<void(event_loop&, event_handle)> repeat_func;

    typedef std::function<void(int)> io_simple_func;
    typedef std::function<void()> timer_simple_func;

    struct event_entry
    {
        union
        {
            ev_io io_watcher;
            ev_signal signal_watcher;
            ev_timer timer_watcher;
        };

        union
        {
            io_func io_callback;
            delay_func delay_callback;
            repeat_func repeat_callback;
            io_simple_func io_simple_callback;
            timer_simple_func timer_simple_callback;
        };

        event_type type;

        ~event_entry();
        event_entry();
        event_entry(event_entry&&);
    };

    template <typename F, typename A, typename B>
    struct pick_event_func
    {
        typedef typename function_traits<F>::function_type FFType;
        typedef typename function_traits<A>::function_type AFType;
        typedef typename function_traits<B>::function_type BFType;
        enum { isA = std::is_same<FFType, AFType>::value,
               isB = std::is_same<FFType, BFType>::value };
        static_assert(isA || isB, "Function types for event_loop callbacks mismatch");
        typedef typename std::conditional<isA, A, B>::type type;
    };
}


class event_loop final
{
public:
    event_loop()
        : _loop(ev_loop_new(0)),
          _event_counter(0)
    {
        ev_set_userdata(_loop, this);
        init_imm_watcher();
    }

    template <typename F>
    event_handle listen(int fd, F&& gen_callback)
    {
        typename xx_impl::pick_event_func<F, xx_impl::io_func, xx_impl::io_simple_func>::type
            callback (std::forward<F>(gen_callback));
        return listen_impl(fd, std::move(callback));
    }

    template <typename F>
    event_handle signal(int signum, F&& gen_callback)
    {
        typename xx_impl::pick_event_func<F, xx_impl::io_func, xx_impl::io_simple_func>::type
            callback (std::forward<F>(gen_callback));
        return signal_impl(signum, std::move(callback));
    }

    template <typename R, typename P, typename F>
    event_handle delay(std::chrono::duration<R, P> after, F&& gen_callback)
    {
        using namespace std::chrono;
        auto seconds = duration_cast<duration<ev_tstamp>>(after).count();
        typename xx_impl::pick_event_func<F, xx_impl::delay_func, xx_impl::timer_simple_func>::type
            callback (std::forward<F>(gen_callback));
        return delay_impl(seconds, std::move(callback));
    }

    template <typename R, typename P, typename F>
    event_handle repeat(std::chrono::duration<R, P> interval, F&& gen_callback)
    {
        using namespace std::chrono;
        auto seconds = duration_cast<duration<ev_tstamp>>(interval).count();
        typename xx_impl::pick_event_func<F, xx_impl::repeat_func, xx_impl::timer_simple_func>::type
            callback (std::forward<F>(gen_callback));
        return repeat_impl(seconds, std::move(callback));
    }

    template <typename F>
    event_handle delay(F&& gen_callback)
    {
        typename xx_impl::pick_event_func<F, xx_impl::delay_func, xx_impl::timer_simple_func>::type
            callback (std::forward<F>(gen_callback));
        return delay_imm_impl(std::move(callback));
    }

    template <typename F>
    event_handle repeat(F&& gen_callback)
    {
        typename xx_impl::pick_event_func<F, xx_impl::repeat_func, xx_impl::timer_simple_func>::type
            callback (std::forward<F>(gen_callback));
        return repeat_imm_impl(std::move(callback));
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
    struct ev_loop* _loop;
    ev_idle _imm_watcher;

    event_handle _event_counter;
    std::list<xx_impl::event_entry> _events;
    std::unordered_map<event_handle, decltype(_events.begin())> _map;

    event_handle listen_impl(int fd, xx_impl::io_func&& callback);
    event_handle listen_impl(int fd, xx_impl::io_simple_func&& callback);
    event_handle signal_impl(int signum, xx_impl::io_func&& callback);
    event_handle signal_impl(int signum, xx_impl::io_simple_func&& callback);
    event_handle delay_impl(ev_tstamp after, xx_impl::delay_func&& callback);
    event_handle delay_impl(ev_tstamp after, xx_impl::timer_simple_func&& callback);
    event_handle repeat_impl(ev_tstamp rep, xx_impl::repeat_func&& callback);
    event_handle repeat_impl(ev_tstamp rep, xx_impl::timer_simple_func&& callback);
    event_handle delay_imm_impl(xx_impl::delay_func&& callback);
    event_handle delay_imm_impl(xx_impl::timer_simple_func&& callback);
    event_handle repeat_imm_impl(xx_impl::repeat_func&& callback);
    event_handle repeat_imm_impl(xx_impl::timer_simple_func&& callback);

    void call(event_handle handle, void* watcher, int watcher_type);
    void call_entry(event_handle handle, decltype(_map.begin()) map_it);

    void call_imms();
    void start_imm_watcher();
    void try_stop_imm_watcher();
    void init_imm_watcher();
};

}

#endif

