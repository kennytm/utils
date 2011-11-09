utils/ext/*
===========

This is a growing list of headers that helps to simplify using third-party C
library codes in C++11. They are not wrappers or ports. They usually provide
RAII constructs like ``shared_ptr`` to ensure a valid state for the C libraries
even in case of exceptions, or convert status code to exceptions for better
error management.

The libraries in ``<utils/ext/library.hpp>`` will have a namespace
``utils::library``.

utils::cairo
------------

```c++
template <typename T>
using unique_ptr = utils::generic_unique_ptr<T, (unspecified)>;

template <typename T>
using shared_ptr = utils::generic_shared_ptr<T, (unspecified)>;
```

Provides ``utils::cairo::unique_ptr<T>`` and ``utils::cairo::shared_ptr<T>`` for
RAII access of various reference-counting cairo types, e.g. cairo_t and
cairo_surface_t.

utils::directfb
---------------

```c++
template <typename T>
using unique_ptr = utils::generic_unique_ptr<T, (unspecified)>;

template <typename T>
using shared_ptr = utils::generic_shared_ptr<T, (unspecified)>;
```

Provides ``utils::directfb::unique_ptr<T>`` and
``utils::directfb::shared_ptr<T>`` for RAII access of various reference-counting
DirectFB interfaces, e.g. IDirectFB and IDirectFBSurface.

```c++
class exception : public std::exception
{
public:
    DFBResult error_code;
    exception(DFBResult error_code);
};

static inline void checked(DFBResult error_code);
```

Use ``utils::directfb::checked()`` to check if the result of a DirectFB function
has no error. This function will throw a ``utils::directfb::exception`` when the
error code is not DFB_OK.

```c++
class lock final
{
public:
    explicit lock(IDirectFBSurface* surface, DFBSurfaceLockFlags flags = DSLF_READ|DSLF_WRITE);
    ~lock();

    unsigned char* data() const noexcept;
    int stride() const noexcept;
};
```

An RAII structure to lock a surface for direct bytes access (using the Lock and
Unlock methods in IDirectFBSurface).

utils::posix
------------

```c++
typedef utils::unique_handle<(unspecified)> unique_fd;
```

A smart handle to store a POSIX file descriptor, which is an ``int``. The owned
fd will be ``close(2)``-ed on destruction.

```c++
typedef utils::generic_unique_ptr<DIR, (unspecified)> unique_dir_ptr;
```

A smart pointer to the DIR structure. It will be ``closedir(3)``-ed on
destruction.

```c++
class exception : public std::exception
{
public:
    int error_number;

    exception(int errno_ = errno);
};
```

An exception which wraps an ``errno(3)``.

```c++
int checked(int result);
```

Throws an ``utils::posix::exception`` when the result is negative.

