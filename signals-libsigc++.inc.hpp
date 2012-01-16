#ifndef SINGALS_LIBSIGC_INC_HPP_MHA3KVRSUWG
#define SINGALS_LIBSIGC_INC_HPP_MHA3KVRSUWG 1

#include <utility>
#include <type_traits>
#include <sigc++/sigc++.h>
#include <utils/traits.hpp>

namespace utils {

struct slot_connection
{
public:
    slot_connection() = default;
    //slot_connection(slot_connection&&) = default;
    slot_connection(const slot_connection&) = default;
    //slot_connection& operator=(slot_connection&&) = default;
    slot_connection& operator=(const slot_connection&) = default;

    void disconnect() { _conn.disconnect(); }
    void suspend() { _conn.block(); }
    void resume() { _conn.unblock(); }
    bool is_connected() const { return _conn.connected(); }
    bool is_suspended() const { return _conn.blocked(); }

private:
    slot_connection(sigc::connection&& conn) : _conn(std::move(conn)) {}

    sigc::connection _conn;

    template <typename F>
    friend class signal;
};

template <typename F>
class signal;

template <typename R, typename... Args>
class signal<R(Args...)>
{
private:
    typedef R F(Args...);
    sigc::signal<R, Args...> _signal;

public:
    slot_connection connect(signal& another_signal)
    { return slot_connection(_signal.connect(another_signal._signal)); }

    template <typename T>
    slot_connection connect(T&& slot)
    { return slot_connection(_signal.connect(std::forward<T>(slot))); }

    template <typename T>
    slot_connection operator+=(T&& slot)
    { return connect(std::forward<T>(slot)); }

    R emit(Args... args) const
    { return _signal.emit(std::forward<Args>(args)...); }

    bool empty() const noexcept
    { return _signal.empty(); }
};

}

/*
template <typename R, typename... Args>
template <typename T>
utils::slot_connection<F> utils::signal<R(Args...)>::connect(T&& slot)
*/

#endif

