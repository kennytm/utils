//------------------------------------------
// Smart handle mechanism for POSIX objects
//------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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

typedef utils::unique_handle<xx_impl::POSIXFildesDeallocator> unique_fd;
typedef utils::generic_unique_ptr<DIR, xx_impl::DirDeallocator> unique_dir_ptr;
typedef utils::generic_unique_ptr<void, xx_impl::DLDeallocator> unique_dl_handle;

class exception : public std::exception
{
public:
    int error_number;
    exception(int errno_ = errno) : error_number(errno_) {}

    virtual const char* what() const noexcept
    {
        return strerror(this->error_number);
    }
};

static inline int checked(int retcode)
{
    if (retcode < 0)
        throw utils::posix::exception();
    return retcode;
}

}}

#endif

