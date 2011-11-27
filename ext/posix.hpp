//------------------------------------------
// Smart handle mechanism for POSIX objects
//------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**

``<utils/ext/posix.hpp>`` --- POSIX RAII Support
================================================

This module provides some smart handle structures for file descriptors and other
resources.

*/

#ifndef POSIX_HPP_6UESMHVE4MT
#define POSIX_HPP_6UESMHVE4MT 1

#include <cerrno>
#include <cstring>
#include <exception>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include "../memory.hpp"

namespace utils { namespace posix {

/**
Members
-------
*/

namespace xx_impl
{
    struct POSIXFildesDeallocator
    {
        static const int null = -1;
        static void release(int fd) noexcept { close(fd); }
    };

    struct DirDeallocator
    {
        static void release(DIR* d) noexcept { closedir(d); }
    };

    struct DLDeallocator
    {
        static void release(void* handle) noexcept { dlclose(handle); }
    };
}

/**
.. type:: type utils::posix::unique_fd = utils::unique_handle<(unspecified)>

    Smart handler type that asserts unique ownership to a file descriptor (an
    ``int``).

    On destruction, the file descriptor will be closed by ``close(2)``.
*/
typedef utils::unique_handle<xx_impl::POSIXFildesDeallocator> unique_fd;

/**
.. type:: type utils::posix::unique_dir_ptr = utils::generic_unique_ptr<DIR, (unspecified)>

    Smart pointer type that asserts unique ownership to a DIR pointer.

    On destruction, the DIR pointer will be closed by ``closedir(3)``.
*/
typedef utils::generic_unique_ptr<DIR, xx_impl::DirDeallocator> unique_dir_ptr;


/**
.. type:: type utils::posix::unique_dl_handle = utils::generic_unique_ptr<void, (unspecified)>

    Smart pointer type that asserts unique ownership to a handle returned by
    ``dlopen(3)``.

    On destruction, the DIR pointer will be closed by ``dlclose(3)``.
*/
typedef utils::generic_unique_ptr<void, xx_impl::DLDeallocator> unique_dl_handle;

/**
.. type:: class utils::posix::exception : public std::exception

    An exception coming from an unsuccessful ``errno(3)``.
*/
class exception : public std::exception
{
public:
    /**
    .. data:: int error_number

        The corresponding errno.
    */
    int error_number;

    /**
    .. function:: exception(int error_number = errno)

        Initialize the constructor. If the argument is not given, the current
        errno will be substituted.
    */
    exception(int errno_ = errno) : error_number(errno_) {}

    virtual const char* what() const noexcept
    {
        return strerror(this->error_number);
    }
};

/**
.. function:: inline int utils::posix::checked(int result)

    Check if a POSIX function is completed successfully. If the *result* is
    negative (usually meaning an unsuccessful call), a
    :type:`utils::posix::exception` with the current errno will be thrown.
    Otherwise, the code will be returned unmodified.
*/
static inline int checked(int retcode)
{
    if (retcode < 0)
        throw utils::posix::exception();
    return retcode;
}

}}

#endif

