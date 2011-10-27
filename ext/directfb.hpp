//------------------------------------------------
// Smart pointer mechanism for DirectFB instances
//------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


#ifndef MEMORY_DIRECTFB_HPP_R59TAE5TTV
#define MEMORY_DIRECTFB_HPP_R59TAE5TTV

#include <directfb.h>
#include "../ptr.hpp"

namespace utils { namespace directfb {

namespace xx_impl
{
    struct DirectFBDeallocator
    {
        template <typename T>
        static void add_ref(T* obj)
        {
            obj->AddRef(obj);
        }

        template <typename T>
        static void release(T* obj) noexcept
        {
            obj->Release(obj);
        }
    };
}

UTILS_DEF_SMART_PTR_ALIAS(unique, DirectFBDeallocator)
UTILS_DEF_SMART_PTR_ALIAS(shared, DirectFBDeallocator)

class exception : public std::exception
{
public:
    DFBResult error_code;

    explicit Exception(DFBResult error_code_) : error_code(error_code_) {}

    virtual const char* what() const noexcept
    {
        return DirectFBErrorString(error_code);
    }
};

static inline void checked(DFBResult error_code)
{
    if (error_code != DFB_OK)
        throw exception(error_code);
}

class lock
{
public:
    explicit lock(IDirectFBSurface* surface,
                  DFBSurfaceLockFlags flags = DSLF_READ|DSLF_WRITE)
        : _surface(surface), _ptr(nullptr), _pitch(0)
    {
        checked(surface->Lock(surface, flags, &_ptr, &_pitch));
    }

    ~lock()
    {
        surface->Unlock(surface);
    }

    lock(lock&&) = delete;
    lock(const lock&) = delete;
    lock& operator=(lock&&) = delete;
    lock& operator=(const lock&) = delete;
    void* operator new(size_t) = delete;
    void operator delete(void*) = delete;
    void* operator new[](size_t) = delete;
    void operator delete[](void*) = delete;
    // ^ allow only stack allocation from the constructor.
    //   (which allows us to store the raw pointer instead of a shared_ptr)

    unsigned char* data() const noexcept
    {
        return static_cast<unsigned char*>(_ptr);
    }

    int stride() const noexcept
    {
        return _pitch;
    }

private:
    IDirectFBSurface* _surface;
    void* _ptr;
    int _pitch;
};

}}

UTILS_DEF_SMART_PTR_STD_FUNCS(utils::directfb, unique)
UTILS_DEF_SMART_PTR_STD_FUNCS(utils::directfb, shared)

#endif

