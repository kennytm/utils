//----------------------------------------------------------------------
// utils/memory: Smart pointers and objects based on external allocation/
//               ref-counting mechanism
//----------------------------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**

``<utils/memory.hpp>`` --- Smart custom pointer/handles
=======================================================

This package provides a C++11-style interface to pointers and handles managed
by a custom deallocation or reference-counting mechanism.

*/

#ifndef MEMORY_HPP_E43O7TFU5KG
#define MEMORY_HPP_E43O7TFU5KG

#include <cstdio>
#include <utility>
#include <functional>
#include <type_traits>
#include <algorithm>
#include <boost/config.hpp>
#include <utils/traits.hpp>

namespace utils {

/**
Members
-------
*/

//{{{ unique_handle

/**
.. type:: class utils::unique_handle<HandleDeleter> final

    ::

        struct POSIXFileDeleter
        {
            static const int null = -1;
            static void release(int handle) noexcept
            {
                close(handle);
            }
        };

        ...

        {
            utils::unique_handle<int, POSIXFileDeleter> fd (open(...));
            read(fd.get(), ...);
            ...
        }

    A unique handle is a generalization of ``std::unique_ptr`` which can manage
    resources associated with arbitrary scalar types.

    In the :type:`~utils::unique_handle` class template, the *HandleDeleter*
    is a structure that describes how to deallocate a resource associated with
    the handle. It should provide two static members::

        static const T null;
        // Set the invalid state of the handle. (T must be a scalar type.)

        static void release(T handle) noexcept;
        // Release a resource associated with the handle.

    The unique_handle class itself contains almost members of unique_ptr,
    except that the functions that takes and returns a pointer now uses a handle
    (``T``) instead. The missing members are:

    * ``operator*``
    * ``operator->``
    * ``operator bool``
    * ``get_deleter``
*/
template <typename HandleDeleter>
class unique_handle
{
public:
    typedef typename std::remove_cv<decltype(HandleDeleter::null)>::type type;
    typedef HandleDeleter deleter_type;

    static_assert(std::is_scalar<type>::value, "Only scalar types are supported.");
    // ^ an arbitrary restriction, may lift in the future.

    type get() const noexcept { return _obj; }

    type release() noexcept
    {
        type retval = _obj;
        _obj = HandleDeleter::null;
        return retval;
    }

    void reset(type new_obj = HandleDeleter::null) noexcept
    {
        type old_obj = _obj;
        _obj = new_obj;
        if (old_obj != HandleDeleter::null)
            HandleDeleter::release(old_obj);
    }

    ~unique_handle()
    {
        if (_obj != HandleDeleter::null)
            HandleDeleter::release(_obj);
    }

    constexpr unique_handle() noexcept : _obj(HandleDeleter::null) {}
    explicit unique_handle(type obj) noexcept : _obj(obj) {}

    unique_handle(unique_handle&& other) noexcept : _obj(other._obj)
    { other._obj = HandleDeleter::null; }

    unique_handle(const unique_handle&) = delete;
    unique_handle& operator=(const unique_handle&) = delete;

    unique_handle& operator=(unique_handle&& other) noexcept
    {
        this->reset(other.release());
        return *this;
    }

    unique_handle& operator=(std::nullptr_t) noexcept
    {
        this->reset();
        return *this;
    }

    void swap(unique_handle& other) noexcept
    {
        std::swap(_obj, other._obj);
    }

    explicit operator bool() const noexcept { return _obj != HandleDeleter::null; }

private:
    type _obj;
};

template <typename HandleDeleter>
bool operator==(const unique_handle<HandleDeleter>& a,
                const unique_handle<HandleDeleter>& b) noexcept
{
    return a.get() == b.get();
}
template <typename HandleDeleter>
bool operator!=(const unique_handle<HandleDeleter>& a,
                const unique_handle<HandleDeleter>& b) noexcept
{
    return a.get() != b.get();
}
template <typename HandleDeleter>
bool operator>=(const unique_handle<HandleDeleter>& a,
                const unique_handle<HandleDeleter>& b) noexcept
{
    return a.get() >= b.get();
}
template <typename HandleDeleter>
bool operator<=(const unique_handle<HandleDeleter>& a,
                const unique_handle<HandleDeleter>& b) noexcept
{
    return a.get() <= b.get();
}
template <typename HandleDeleter>
bool operator>(const unique_handle<HandleDeleter>& a,
                const unique_handle<HandleDeleter>& b) noexcept
{
    return a.get() > b.get();
}
template <typename HandleDeleter>
bool operator<(const unique_handle<HandleDeleter>& a,
               const unique_handle<HandleDeleter>& b) noexcept
{
    return a.get() < b.get();
}

//}}}

//{{{ unique invalidator

/**
.. type:: class unique_invalidator<InvalidatorType, InvalidatorType invalidator> final
    :default_constructible:
    :movable:
    :noncopyable:

    An smart object which owns a handle spit off by some resource pool, which
    will be erased when this invalidator is destroyed.

    The *InvalidatorType* should be a type of member function pointer
    *invalidator* of the resource pool. It should refer to a method which will
    be used to cancel the resource, and the first argument must be the handle,
    passed by value, const reference or rvalue reference.

    The invalidator must not outlive the pool it is associated with. If the
    owned handle is invalidated by external means,
    :func:`~utils::unique_invalidator::release_if` can be used to safely reset
    the handle without double invalidation.
*/
template <typename InvalidatorType, InvalidatorType invalidator>
class unique_invalidator final
{
    typedef function_traits<InvalidatorType> func_traits;

public:
    /**
    .. type:: type pool_type

        The type of the resource pool is invalidator has.
    */
    typedef typename func_traits::owner_type pool_type;

    /**
    .. type:: type handle_type

        The type of the handle to a resource in the resource pool.
    */
    typedef typename std::decay<typename func_traits::template arg<0>::type>::type handle_type;

    /**
    .. function:: inline const handle_type& get() const noexcept

        Get the handle this invalidator is owning. If the handle has already
        been invalidated via :func:`~utils::unique_invalidator::reset` or
        equivalent methods, the return value may not be valid.
    */
    const handle_type& get() const noexcept { return _handle; }

    /**
    .. function:: inline std::add_pointer<pool_type>::type get_pool() noexcept

        Get the pool this invalidator is using. If the handle has already been
        invalidator via :func:`~utils::unique_invalidator::reset` or equivalent
        methods, the return value is nullptr.
    */
    typename std::add_pointer<pool_type>::type get_pool() noexcept { return _pool; }

    /**
    .. function:: handle_type release()

        Release ownership of the handle, and return it.
    */
    handle_type release()
        noexcept(std::is_nothrow_move_constructible<handle_type>::value)
    {
        _pool = nullptr;
        return std::move(_handle);
    }

    /**
    .. function:: void reset()

        Invalidate the handle,
    */
    void reset()
        noexcept(noexcept((std::declval<pool_type>().*invalidator)(std::declval<handle_type&&>())))
    {
        if (!_pool)
            return;
        (_pool->*invalidator)(std::move(_handle));
        _pool = nullptr;
    }

    /**
    .. function:: void reset(pool_type& pool, handle_type&& handle)

        Invalidate the currently owned handle, and then adapt the provided one.
    */
    void reset(pool_type& pool, handle_type&& handle)
        noexcept(noexcept(std::declval<unique_invalidator>().reset())
              && std::is_nothrow_move_assignable<handle_type>::value)
    {
        reset();
        _handle = std::move(handle);
        _pool = &pool;
    }

    ~unique_invalidator() noexcept(noexcept(std::declval<unique_invalidator>().reset()))
    {
        reset();
    }

    /**
    .. function:: unique_invalidator()

        Construct a unique invalidator with no owned handles.
    */
    unique_invalidator()
        noexcept(std::is_nothrow_default_constructible<handle_type>::value)
        : _pool(nullptr)
    {}

    /**
    .. function:: unique_invalidator(pool_type& pool, handle_type&& handle)

        Construct a unique invalidator owning the handle.
    */
    unique_invalidator(pool_type& pool, handle_type&& handle)
        noexcept(std::is_nothrow_move_constructible<handle_type>::value)
        : _pool(&pool),
          _handle(std::move(handle))
    {}

    unique_invalidator(unique_invalidator&& other)
        noexcept(std::is_nothrow_move_constructible<handle_type>::value)
        : _pool(other._pool),
          _handle(std::move(other._handle))
    { other._pool = nullptr; }

    unique_invalidator(const unique_invalidator&) = delete;
    unique_invalidator& operator=(const unique_invalidator&) = delete;

    unique_invalidator& operator=(unique_invalidator&& other)
        noexcept(noexcept(std::declval<unique_invalidator>().reset(std::declval<pool_type>(),
                                                                   std::declval<handle_type&&>()))
              && noexcept(std::declval<unique_invalidator>().release()))
    {
        if (this != &other)
        {
            auto pool = other._pool;
            auto handle = other.release();
            reset(*pool, std::move(handle));
        }
        return *this;
    }

    unique_invalidator& operator=(std::nullptr_t)
        noexcept(noexcept(std::declval<unique_invalidator>().reset()))
    {
        reset();
        return *this;
    }

    void swap(unique_invalidator& other)
        noexcept(noexcept(std::swap(std::declval<handle_type&>(), std::declval<handle_type&>())))
    {
        using std::swap;
        swap(_handle, other._handle);
        swap(_pool, other._pool);
    }

    explicit operator bool() const noexcept { return _pool != nullptr; }

    bool operator==(const unique_invalidator& other) const noexcept
    {
        return _pool == other._pool && _handle == other._handle;
    }

    bool operator!=(const unique_invalidator& other) const noexcept
    {
        return _pool != other._pool || _handle != other._handle;
    }

    /**
    .. function:: void release_if(const pool_type& pool, const handle_type& handle) noexcept

        Release the handle this instance is owning (without invalidation) if it
        has the same pool and handle as the input.
    */
    void release_if(const pool_type& pool, const handle_type& handle) noexcept
    {
        if (_pool == &pool && _handle == handle)
            _pool = nullptr;
    }

private:
    typename std::add_pointer<pool_type>::type _pool;
    handle_type _handle;
};

/**
.. function:: ForwardIterator utils::release_if<ForwardIterator>(ForwardIterator begin, ForwardIterator end, const utils::pointee<ForwardIterator>::pool_type& pool, const utils::pointee<ForwardIterator>::handle_type& handle)

    Perform :func:`utils::unique_invalidator<...>::release_if` on all the
    iterators, and remove them if they are really removed.

    Returns the new iterator to the end after the removal.
*/
template <typename ForwardIterator>
ForwardIterator release_if(ForwardIterator begin, ForwardIterator end,
                           const typename pointee<ForwardIterator>::pool_type& pool,
                           const typename pointee<ForwardIterator>::value_type& handle)
    noexcept(noexcept(handle == handle))
{
    return std::remove_if(begin, end, [&](utils::pointee<ForwardIterator>& inv) -> bool
    {
        inv.release_if(pool, handle);
        return !inv;
    });
}

//}}}

//{{{ common base of all smart pointers.

namespace xx_impl
{
    template <typename T, typename GenericDeleter, template <typename, typename> class Template>
    class generic_smart_ptr
    {
        typedef Template<T, GenericDeleter> Self;

    public:
        typedef T* pointer;
        typedef T element_type;

        typedef GenericDeleter deleter_type;

        T* operator->() const noexcept { return _p; }
        typename std::add_lvalue_reference<T>::type operator*() const { return *_p; }

        T* get() const noexcept { return _p; }
        explicit operator bool() const noexcept { return _p != nullptr; }

        T* release() noexcept
        {
            T* retval = _p;
            _p = nullptr;
            return retval;
        }

        void reset(T* ptr = nullptr) noexcept
        {
            T* old_ptr = _p;
            _p = ptr;
            if (old_ptr != nullptr)
                GenericDeleter::release(old_ptr);
        }

        ~generic_smart_ptr()
        {
            if (_p != nullptr)
                GenericDeleter::release(_p);
        }

        void swap(Self& other) noexcept
        {
            std::swap(_p, other._p);
        }

        template <typename U>
        Self& operator=(Template<U, GenericDeleter>&& other) noexcept
        {
            this->reset(other.release());
            return *static_cast<Self*>(this);
        }

        Self& operator=(std::nullptr_t) noexcept
        {
            this->reset();
            return *static_cast<Self*>(this);
        }

    protected:
        constexpr generic_smart_ptr() noexcept : _p(nullptr) {}

        template <typename U>
        explicit generic_smart_ptr(U* ptr) noexcept : _p(ptr) {}

        template <typename U>
        generic_smart_ptr(U&& other) noexcept : _p(other._p)
        { other._p = nullptr; }

    private:
        T* _p;

        template <typename, typename, template<typename, typename> class>
        friend class generic_smart_ptr;
    };

    template <typename T, typename SA, template<typename, typename> class P>
    bool operator==(const utils::xx_impl::generic_smart_ptr<T, SA, P>& a, std::nullptr_t) noexcept
    {
        return !a;
    }
    template <typename T, typename SA, template<typename, typename> class P>
    bool operator==(std::nullptr_t, const utils::xx_impl::generic_smart_ptr<T, SA, P>& a) noexcept
    {
        return !a;
    }
    template <typename T, typename SA, template<typename, typename> class P>
    bool operator!=(const utils::xx_impl::generic_smart_ptr<T, SA, P>& a, std::nullptr_t) noexcept
    {
        return !!a;
    }
    template <typename T, typename SA, template<typename, typename> class P>
    bool operator!=(std::nullptr_t, const utils::xx_impl::generic_smart_ptr<T, SA, P>& a) noexcept
    {
        return !!a;
    }
}

//}}}

//{{{ generic_unique_ptr

#define BASE_GTI9R9EEWSN(self) \
    xx_impl::generic_smart_ptr<T, GenericDeleter, utils::generic_##self##_ptr>

/**
.. type:: class utils::generic_unique_ptr<T, GenericDeleter>
          class utils::generic_shared_ptr<T, GenericDeleter>

    ::

        struct GLibDeallocator
        {
            static void release(gpointer obj) noexcept
            {
                g_object_unref(obj);
            }
            static void add_ref(gpointer obj) noexcept
            {
                g_object_ref(obj);
            }
        };

        ...

        {
            utils::unique_ptr<GObject, GLibDeallocator> my_gobject (g_object_new(...));
            utils::shared_ptr<GObject, GLibDeallocator> another_object (std::move(my_gobject));
            ...
        }

    These are generic version of the ``std::unique_ptr`` and ``std::shared_ptr``
    which uses an external reference-counting mechanism (like GLib, COM, Core
    Foundation, etc.).

    In these class templates, the *GenericDeleter* is a structure that describes
    how to do reference counting on a resource held by the pointer ``T*``. It
    should provide these static members::

        static void release(T* ptr) noexcept;
        // decrease the reference count an object held by 'ptr', possibly
        // deallocating it.
        //
        // required by generic_unique_ptr<T> and generic_shared_ptr<T>.

        static void add_ref(T* ptr);
        // increase the reference count of the object.
        //
        // required by generic_shared_ptr<T>

        static long use_count(T* ptr) noexcept;
        // returns the reference count of the object.
        //
        // optional -- and if it cannot be done with noexcept, please do not define it.

    Most members of in the standard classes should also be available in these
    generic versions, except:

    * The array variant of ``unique_ptr``
    * Anything that related to ``weak_ptr`` and ``auto_ptr``
    * Atomic access (libstdc++ doesn't have these either)
    * ``unique_ptr::get_deleter``
    * ``make_shared``, ``allocate_shared``
    * ``dynamic_pointer_cast``

    The :type:`~utils::generic_shared_ptr` is similar to `boost::intrusive_ptr
    <http://www.boost.org/doc/libs/1_47_0/libs/smart_ptr/intrusive_ptr.html>`_,
    except the latter does not follow the usual ``shared_ptr`` rules ---
    constructing from a raw pointer will unnecessarily add a reference count by
    default.
*/

template <typename T, typename GenericDeleter>
class generic_unique_ptr : public BASE_GTI9R9EEWSN(unique)
{
public:
    constexpr generic_unique_ptr() noexcept : BASE_GTI9R9EEWSN(unique)() {}
    constexpr generic_unique_ptr(std::nullptr_t) noexcept : BASE_GTI9R9EEWSN(unique)() {}
    inline ~generic_unique_ptr() {}

    template <typename U>
    explicit generic_unique_ptr(U* ptr) noexcept : BASE_GTI9R9EEWSN(unique)(ptr) {}

    template <typename U>
    generic_unique_ptr(generic_unique_ptr<U, GenericDeleter>&& other) noexcept
        : BASE_GTI9R9EEWSN(unique)(std::move(other))
    {}

    generic_unique_ptr(generic_unique_ptr&& other) noexcept
        : BASE_GTI9R9EEWSN(unique)(std::move(other))
    {}

    generic_unique_ptr& operator=(generic_unique_ptr&& other)
    {
        return BASE_GTI9R9EEWSN(unique)::operator=(std::move(other));
    };

    generic_unique_ptr(const generic_unique_ptr&) = delete;
    generic_unique_ptr& operator=(const generic_unique_ptr&) = delete;
};

//}}}

//{{{ generic_unique_ptr comparisons

template <typename T1, typename T2, typename GenericDeleter>
bool operator==(const generic_unique_ptr<T1, GenericDeleter>& a,
                const generic_unique_ptr<T2, GenericDeleter>& b) noexcept
{
    return a.get() == b.get();
}
template <typename T1, typename T2, typename GenericDeleter>
bool operator!=(const generic_unique_ptr<T1, GenericDeleter>& a,
                const generic_unique_ptr<T2, GenericDeleter>& b) noexcept
{
    return a.get() != b.get();
}
template <typename T1, typename T2, typename GenericDeleter>
bool operator>=(const generic_unique_ptr<T1, GenericDeleter>& a,
                const generic_unique_ptr<T2, GenericDeleter>& b) noexcept
{
    return a.get() >= b.get();
}
template <typename T1, typename T2, typename GenericDeleter>
bool operator<=(const generic_unique_ptr<T1, GenericDeleter>& a,
                const generic_unique_ptr<T2, GenericDeleter>& b) noexcept
{
    return a.get() <= b.get();
}
template <typename T1, typename T2, typename GenericDeleter>
bool operator>(const generic_unique_ptr<T1, GenericDeleter>& a,
               const generic_unique_ptr<T2, GenericDeleter>& b) noexcept
{
    return a.get() > b.get();
}
template <typename T1, typename T2, typename GenericDeleter>
bool operator<(const generic_unique_ptr<T1, GenericDeleter>& a,
               const generic_unique_ptr<T2, GenericDeleter>& b) noexcept
{
    return a.get() < b.get();
}

//}}}

//{{{ generic_shared_ptr

template <typename T, typename GenericDeleter>
class generic_shared_ptr : public BASE_GTI9R9EEWSN(shared)
{
public:
    constexpr generic_shared_ptr() noexcept : BASE_GTI9R9EEWSN(shared)() {}
    constexpr generic_shared_ptr(std::nullptr_t) noexcept : BASE_GTI9R9EEWSN(shared)() {}

    template <typename U>
    explicit generic_shared_ptr(U* ptr) noexcept : BASE_GTI9R9EEWSN(shared)(ptr) {}

    template <typename U>
    generic_shared_ptr(generic_shared_ptr<U, GenericDeleter>&& other) noexcept
        : BASE_GTI9R9EEWSN(shared)(std::move(other))
    {}

    template <typename U>
    generic_shared_ptr(generic_unique_ptr<U, GenericDeleter>&& other) noexcept
        : BASE_GTI9R9EEWSN(shared)(std::move(other))
    {}

    generic_shared_ptr(generic_shared_ptr&& other) noexcept
        : BASE_GTI9R9EEWSN(shared)(std::move(other))
    {}

    template <typename U>
    generic_shared_ptr(const generic_shared_ptr<U, GenericDeleter>& other)
        : BASE_GTI9R9EEWSN(shared)(other.get())
    {
        if (this->get() != nullptr)
            GenericDeleter::add_ref(this->get());
    }

    generic_shared_ptr(const generic_shared_ptr& other)
        : BASE_GTI9R9EEWSN(shared)(other.get())
    {
        if (this->get() != nullptr)
            GenericDeleter::add_ref(this->get());
    }

    template <typename U>
    generic_shared_ptr& operator=(generic_unique_ptr<U, GenericDeleter>&& other) noexcept
    {
        return BASE_GTI9R9EEWSN(shared)::operator=(std::move(other));
    }

    generic_shared_ptr& operator=(generic_shared_ptr&& other) noexcept
    {
        return BASE_GTI9R9EEWSN(shared)::operator=(std::move(other));
    }

    template <typename U>
    generic_shared_ptr& operator=(const generic_shared_ptr<U, GenericDeleter>& other)
    {
        generic_shared_ptr(other).swap(*this);
        return *this;
    }

    generic_shared_ptr& operator=(const generic_shared_ptr& other)
    {
        generic_shared_ptr(other).swap(*this);
        return *this;
    }

    long use_count() const noexcept
    {
        return GenericDeleter::use_count(this->_p);
    }

    bool unique() const noexcept
    {
        return use_count() == 1;
    }
};
#undef BASE_GTI9R9EEWSN

namespace xx_impl
{
    template <typename T>
    struct is_generic_shared_ptr
    {
        enum { value = false };
    };
    template <typename T, typename SA>
    struct is_generic_shared_ptr<generic_shared_ptr<T, SA>>
    {
        enum { value = true };
    };
}

//}}}

//{{{ generic_shared_ptr comparisons

template <typename T1, typename T2, typename GenericDeleter>
bool operator==(const generic_shared_ptr<T1, GenericDeleter>& a,
                const generic_shared_ptr<T2, GenericDeleter>& b) noexcept
{
    return a.get() == b.get();
}
template <typename T1, typename T2, typename GenericDeleter>
bool operator!=(const generic_shared_ptr<T1, GenericDeleter>& a,
                const generic_shared_ptr<T2, GenericDeleter>& b) noexcept
{
    return a.get() != b.get();
}
template <typename T1, typename T2, typename GenericDeleter>
bool operator>=(const generic_shared_ptr<T1, GenericDeleter>& a,
                const generic_shared_ptr<T2, GenericDeleter>& b) noexcept
{
    return a.get() >= b.get();
}
template <typename T1, typename T2, typename GenericDeleter>
bool operator<=(const generic_shared_ptr<T1, GenericDeleter>& a,
                const generic_shared_ptr<T2, GenericDeleter>& b) noexcept
{
    return a.get() <= b.get();
}
template <typename T1, typename T2, typename GenericDeleter>
bool operator>(const generic_shared_ptr<T1, GenericDeleter>& a,
               const generic_shared_ptr<T2, GenericDeleter>& b) noexcept
{
    return a.get() > b.get();
}
template <typename T1, typename T2, typename GenericDeleter>
bool operator<(const generic_shared_ptr<T1, GenericDeleter>& a,
               const generic_shared_ptr<T2, GenericDeleter>& b) noexcept
{
    return a.get() < b.get();
}

//}}}

//{{{ file_ptr

namespace xx_impl
{
    struct FileAllocator
    {
        static void release(FILE* f) noexcept { fclose(f); }
    };
}

/**
.. type:: type utils::unique_file_ptr = utils::generic_unique_ptr<FILE, (unspecified)>

    ::

        {
            utils::unique_file_ptr f (fopen("1.txt", "r"));
            fscanf(f.get(), "%d", &res);
        }

    This class is a specialization of :type:`~utils::generic_unique_ptr` to the
    ``FILE*`` type. When destructing, the file will be closed by ``fclose()``.

    .. warning:: Do not use this with ``popen()``.
*/
typedef generic_unique_ptr<FILE, xx_impl::FileAllocator> unique_file_ptr;

//}}}

}

//{{{ swaps & hashes

namespace std
{
    template <typename HandleDeleter>
    void swap(utils::unique_handle<HandleDeleter>& a,
              utils::unique_handle<HandleDeleter>& b) noexcept
    {
        a.swap(b);
    }

    template <typename InvalidatorType, InvalidatorType invalidator>
    void swap(utils::unique_invalidator<InvalidatorType, invalidator>& a,
              utils::unique_invalidator<InvalidatorType, invalidator>& b) noexcept(noexcept(a.swap(b)))
    {
        a.swap(b);
    }

    template <typename T, typename SA>
    void swap(utils::generic_unique_ptr<T, SA>& a,
              utils::generic_unique_ptr<T, SA>& b) noexcept
    {
        a.swap(b);
    }

    template <typename T, typename SA>
    void swap(utils::generic_shared_ptr<T, SA>& a,
              utils::generic_shared_ptr<T, SA>& b) noexcept
    {
        a.swap(b);
    }

    template <typename HandleDeleter>
    struct hash<utils::unique_handle<HandleDeleter>>
        : std::unary_function<utils::unique_handle<HandleDeleter>, size_t>
    {
        size_t operator()(const utils::unique_handle<HandleDeleter>& x) const
        {
            return std::hash<typename utils::unique_handle<HandleDeleter>::type>()(x.get());
        }
    };

    template <typename T, typename SA>
    struct hash<utils::generic_unique_ptr<T, SA>>
        : std::unary_function<utils::generic_unique_ptr<T, SA>, size_t>::type
    {
        size_t operator()(const T& x) const
        {
            return std::hash<T*>()(x.get());
        }
    };

    template <typename T, typename SA>
    struct hash<utils::generic_shared_ptr<T, SA>>
        : std::unary_function<utils::generic_shared_ptr<T, SA>, size_t>::type
    {
        size_t operator()(const T& x) const
        {
            return std::hash<T*>()(x.get());
        }
    };

    template <typename T, typename U>
    typename std::enable_if<utils::xx_impl::is_generic_shared_ptr<U>::value,
                            utils::generic_shared_ptr<T, typename U::deleter_type>>::type
        static_pointer_cast(const U& r) noexcept
    {
        T* ret_ptr = static_cast<T*>(r.get());
        if (ret_ptr != nullptr)
            U::deleter_type::add_ref(ret_ptr);
        return utils::generic_shared_ptr<T, typename U::deleter_type>(ret_ptr);
    }

    template <typename T, typename U>
    typename std::enable_if<utils::xx_impl::is_generic_shared_ptr<U>::value,
                            utils::generic_shared_ptr<T, typename U::deleter_type>>::type
        const_pointer_cast(const U& r) noexcept
    {
        T* ret_ptr = const_cast<T*>(r.get());
        if (ret_ptr != nullptr)
            U::deleter_type::add_ref(ret_ptr);
        return utils::generic_shared_ptr<T, typename U::deleter_type>(ret_ptr);
    }
}

//}}}

#ifndef BOOST_NO_TEMPLATE_ALIASES

/**
.. macro:: UTILS_DEF_SMART_PTR_ALIAS(ptr_type, GenericDeleter)
           UTILS_DEF_SMART_PTR_STD_FUNCS(Namespace, ptr_type)

    ::

        namespace glib
        {
            UTILS_DEF_SMART_PTR_ALIAS(unique, GLibAllocator)
            // ^ defines the 'unique_ptr<T>' type in this namespace as an alias to
            //   'utils::generic_unique_ptr<T, GLibAllocator>'

            UTILS_DEF_SMART_PTR_ALIAS(shared, GLibAllocator)
            // ^ this is for 'shared_ptr'
        }

        UTILS_DEF_SMART_PTR_STD_FUNCS(glib, unique)
        // ^ specializes 'std::swap' and 'std::hash' if required.
        //
        // (note: must be called in global namespace)

        UTILS_DEF_SMART_PTR_STD_FUNCS(glib, shared)
        // ^ this is for 'glib::shared_ptr'

    The :type:`~utils::generic_unique_ptr` and :type:`~utils::generic_shared_ptr`
    smart pointers are not meant to be used directly, but as a template alias.
    However, not all compilers support template aliases yet. Without template
    aliases, the aliases have to be rewritten using subclassing. Doing so for
    every type is error prone. Also, since now the subclass is a different type,
    the ``std::swap`` and ``std::hash`` have to be defined again.

    Such boilerplate codes are collected to the
    :macro:`UTILS_DEF_SMART_PTR_ALIAS` and :macro:`UTILS_DEF_SMART_PTR_STD_FUNCS`
    macros, such that a correct subclass will be generated and have the
    corresponding standard functions overloaded on use. Of course, if the
    compiler supports template alias (as recognized by Boost), these will simply
    expand to the ``using`` statement.
*/

#define UTILS_DEF_SMART_PTR_ALIAS(PtrType, GenericDeleter) \
    template <typename T> \
    using PtrType##_ptr = ::utils::generic_##PtrType##_ptr<T, GenericDeleter>;

#define UTILS_DEF_SMART_PTR_STD_FUNCS(PtrType, GenericDeleter)

#else

#define UTILS_DEF_SMART_PTR_ALIAS(PtrType, GenericDeleter) \
    template <typename T> \
    class PtrType##_ptr : public ::utils::generic_##PtrType##_ptr<T, GenericDeleter> \
    { \
    public: \
        template <typename Param> \
        PtrType##_ptr(Param&& param) \
            : ::utils::generic_##PtrType##_ptr<T, GenericDeleter>( \
                std::forward<Param>(param) \
            ) \
        { static_assert(!std::is_pointer<Param>::value, \
                        "Cannot implicitly convert a raw pointer to a smart pointer"); } \
        template <typename U> \
        explicit PtrType##_ptr(U* ptr) noexcept \
            : ::utils::generic_##PtrType##_ptr<T, GenericDeleter>(ptr) \
        {} \
        PtrType##_ptr() noexcept \
            : ::utils::generic_##PtrType##_ptr<T, GenericDeleter>() \
        {} \
    };

#define UTILS_DEF_SMART_PTR_STD_FUNCS(Namespace, PtrType) \
namespace std \
{ \
    template <typename T> \
    void swap(Namespace::PtrType##_ptr<T>& a, \
              Namespace::PtrType##_ptr<T>& b) noexcept { a.swap(b); } \
    template <typename T> \
    struct hash<Namespace::PtrType##_ptr<T>> \
        : public std::unary_function<Namespace::PtrType##_ptr<T>, size_t> \
    { \
        size_t operator()(const Namespace::PtrType##_ptr<T>& x) \
        { return hash<T*>()(x.get()); } \
    }; \
}

#endif

#endif

