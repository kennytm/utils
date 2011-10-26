utils/memory: Smart custom pointer/handles
==========================================

The package `utils/memory` provides a C++11-style interface to pointers and
handles managed by a custom deallocation or reference-counting mechanism.

unique_handle
-------------

```c++
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
    utils::unique_handle<int, POSIXFileDeallocator> fd (open(...));
    read(fd.get(), ...);
    ...
}
```

A unique handle is a generalization of `std::unique_ptr` which can manage
resources associated with arbitrary scalar types.

In the `utils::unique_handle<HandleDeleter>` class template, the `HandleDeleter`
is a structure that describes how to deallocate a resource associated with the
handle. It should provide two static members:

```c++
static const T null;
// Set the invalid state of the handle. (T must be a scalar type.)

static void release(T handle) noexcept;
// Release a resource associated with the handle.
```

The `unique_handle` class itself contains almost members of `unique_ptr`,
except that the functions that takes and returns a pointer now uses a handle
(`T`) instead. The missing members are:

* `operator*`
* `operator->`
* `operator bool`
* `get_deleter`

generic_unique_ptr and generic_shared_ptr
-----------------------------------------

```c++
struct GLibDeleter
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
```

These are generic version of the `std::unique_ptr` and `std::shared_ptr`
which uses an external reference-counting mechanism (like GLib, COM, Core
Foundation, etc.).

In the `utils::generic_unique_ptr<T, GenericDeleter>` and
`utils::generic_shared_ptr<T, GenericDeleter>` class templates, the
`SimpleAllocator` is a structure that describes how to do reference counting
on a resource held by the pointer `T*`. It should provide these static members:

```c++
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
```

Most members of in the standard classes should also be available in these
generic versions, except:

* The array variant of `unique_ptr`
* Anything that related to `weak_ptr` and `auto_ptr`
* Atomic access (libstdc++ doesn't have these either)
* `unique_ptr::get_deleter`
* `make_shared`, `allocate_shared`
* `dynamic_pointer_cast`

The `generic_shared_ptr` is similar to `boost::intrusive_ptr`, except the latter
does not follow the usual `shared_ptr` rules --- constructing from a raw pointer
will unnecessarily add a reference count by default.

These generic pointers are not meant to be used directly, but as a template
alias:

```c++
namespace glib
{
    template <typename T>
    using unique_ptr = utils::generic_unique_ptr<T, GLibAllocator>;

    template <typename T>
    using shared_ptr = utils::generic_unique_ptr<T, GLibAllocator>;
}

...

glib::unique_ptr<GObject> my_object (...);
```

Boilerplates
------------
Not all compilers support template aliases yet. Without template aliases, the
above alias have to be rewritten using subclassing:

```c++
namespace glib
{
    template <typename T>
    class unique_ptr : public utils::generic_unique_ptr<T, GLibAllocator>
    {
        unique_ptr(unique_ptr&& other)
            : utils::generic_unique_ptr<T, GLibAllocator>(std::move(other))
        {}
        // and all other constructors
    };

    template <typename T>
    class shared_ptr : public utils::generic_shared_ptr<T, GLibAllocator>
    {
        // and so on
    };
}
```

(one could also use the traditional ``struct unique_ptr { typedef ... type; };``
method, but then you lose the nice ``glib::unique_ptr<GObject>`` syntax).

Doing so for every type is error prone. Also, since now the subclass is a
different type, the ``std::swap`` and ``std::hash`` have to be defined again.

Such boilerplate codes are collected to the ``UTILS_DEF_SMART_PTR_ALIAS`` and
``UTILS_DEF_SMART_PTR_STD_FUNCS`` macros, such that a correct subclass will be
generated and have the corresponding standard functions overloaded on use. Of
course, if the compiler supports template alias (as recognized by Boost), these
will simply expand to the ``using`` statement.

```c++
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

```


utils::unique_file_ptr
----------------------

```c++
{
    utils::unique_file_ptr f (fopen("1.txt", "r"));
    fscanf(f.get(), "%d", &res);
}
```

This class is a specialization of ``generic_unique_ptr`` to the ``FILE*`` type.
When releasing, the file will be closed by ``fclose()``.

(Note: Do not use this with ``popen()``.)



