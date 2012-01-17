#ifndef SIGNALS_SIGSLOT_INC_HPP_U4VA0FLBH2G
#define SIGNALS_SIGSLOT_INC_HPP_U4VA0FLBH2G 1

#include <memory>
#include <list>
#include <functional>
#include <stdexcept>

namespace utils {

namespace xx_impl
{
    template <typename T>
    struct slot
    {
        std::function<T> function;
        bool is_suspended;
        mutable bool is_using;

        template <typename F>
        slot(F&& f)
            : function(std::forward<F>(f)),
              is_suspended(false),
              is_using(false)
        {}

        ~slot() {}
    };

    template <typename T>
    using slot_container = std::list<xx_impl::slot<T>>;

    class slot_connection_impl_base
    {
    public:
        virtual void disconnect() = 0;
        virtual void set_is_suspended(bool) = 0;
        virtual bool get_is_suspended() const = 0;
        virtual ~slot_connection_impl_base() {}
    };

    template <typename Subclass, typename F>
    class signal_base;
}

class slot_connection
{
public:
    void disconnect()
    {
        if (_impl)
        {
            _impl->disconnect();
            _impl = nullptr;
        }
    }

    void suspend()
    {
        if (_impl)
            _impl->set_is_suspended(true);
    }

    void resume()
    {
        if (_impl)
            _impl->set_is_suspended(false);
    }

    bool is_connected() const
    {
        return !!_impl;
    }

    bool is_suspended() const
    {
        return !_impl || _impl->get_is_suspended();
    }

    slot_connection() = default;
    slot_connection(slot_connection&&) = default;
    slot_connection& operator=(slot_connection&&) = default;

private:
    std::unique_ptr<xx_impl::slot_connection_impl_base> _impl;
    slot_connection(xx_impl::slot_connection_impl_base* impl) : _impl(impl) {}

    template <typename T>
    friend class xx_impl::signal_base;
};

namespace xx_impl
{
    template <typename SignalSubclass, typename T>
    class slot_connection_impl final : public slot_connection_impl_base
    {
    public:
        slot_connection_impl(signal_base<SignalSubclass, T>* sig)
            : _signal(sig),
              _it(sig->_slots.begin())
        {}

        virtual void disconnect() override
        {
            if (_it->is_using)
                throw std::logic_error("Do not disconnect while emitting that signal!");
            _signal->_slots.erase(_it);
        }

        virtual void set_is_suspended(bool is_suspended) override
        {
            _it->is_suspended = is_suspended;
        }

        virtual bool get_is_suspended() const override
        {
            return _it->is_suspended;
        }

    private:
        signal_base<SignalSubclass, T>* _signal;
        typename slot_container<T>::iterator _it;
    };

    template <typename Subclass, typename R, typename... A>
    class signal_base<Subclass, R(A...)>
    {
    public:
        template <typename F>
        slot_connection connect(F&& func)
        {
            _slots.emplace_front(std::forward<F>(func));
            return make_connection();
        }

        slot_connection connect(Subclass& sig)
        {
            _slots.emplace_front([&](A... args){ return sig.emit(args...); });
            return make_connection();
        }

        template <typename T>
        slot_connection operator+=(T&& slot)
        { return connect(std::forward<T>(slot)); }

        bool empty() const noexcept
        { return _slots.empty(); }

        ~signal_base() {}

    protected:
        xx_impl::slot_container<R(A...)> _slots;

    private:
        slot_connection make_connection()
        {
            return slot_connection(new slot_connection_impl<Subclass, R(A...)>(this));
        }

        template <typename>
        friend class xx_impl::slot_connection_impl;
    };
}

template <typename F, typename Accumulator=void>
class signal;

template <typename R, typename... A, typename Accumulator>
class signal<R(A...), Accumulator> : public xx_impl::signal_base<signal<R(A...)>, R(A...)>
{
public:
    R emit(A... args) const
    {
        R retval;
        auto it = this->_slots.begin();
        while (it != this->_slots.end())
        {
            const auto& slot = *it;
            slot.is_using = false;
            ++ it;
            if (it != this->_slots.end())
                it->is_using = true;
            if (!slot.is_suspended)
                retval = slot.function(args...);
        }

        return retval;
    }
};

template <typename... A>
class signal<void(A...)> : public xx_impl::signal_base<signal<void(A...)>, void(A...)>
{
public:
    void emit(A... args) const
    {
        // Note: DO NOT change this loop into a range-based for loop!
        //       Disconnection may happen when the function is called, which
        //       will change the _slot's begin() and end(). Since a range-based
        //       for loop will cache these iterators, the loop result may be
        //       wrong.
        auto it = this->_slots.begin();
        while (it != this->_slots.end())
        {
            const auto& slot = *it;
            slot.is_using = false;
            ++ it;
            if (it != this->_slots.end())
                it->is_using = true;
            if (!slot.is_suspended)
                slot.function(args...);
        }
    }

    ~signal() {}
};

}

#endif

