//------------------------------------------------
// Smart pointer mechanism for DirectFB instances
//------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


/**

``<utils/ext/directfb.hpp>`` --- DirectFB RAII and Exception Handling Support
=============================================================================

This module overrides the default deleter for DirectFB interfaces, to provide
:term:`RAII` via the ``utils::directfb::unique_ptr<T>`` and
``utils::directfb::shared_ptr<T>`` :term:`smart pointer`\ s, without providing
an explicit deleter.

It also overrides the bitwise operators for DirectFB flags, so that they can be
combined safely without using ``static_cast``.

Finally, it provides convenient functions to check and convert error codes into
exceptions.

Synopsis
--------

Usage::

    #include <memory>
    #include <directfb.h>
    #include <utils/ext/directfb.hpp>

    using utils::directfb::checked;

    utils::directfb::unique_ptr<IDirectFBSurface> create_PNG_surface(IDirectFB* context,
                                                                     const std::string& filename)
    {
        IDirectFBImageProvider* raw_provider;
        // 'checked' will throw if the return value is not DFB_OK
        checked(context->CreateImageProvider(context, filename.c_str(), &raw_provider));
        // this allows the 'raw_provider' to be Release'd when the scope exits
        utils::directfb::unique_ptr<IDirectFBImageProvider> provider (raw_provider);
        ...
    }

    void render_PNG_image(IDirectFB* context, const std::string& filename)
    {
        utils::directfb::unique_ptr<IDirectFBSurface> png_surface;
        try
        {
            png_surface = std::move(create_PNG_surface(context, filename));
        }
        catch(const utils::directfb::exception& exc)
        {
            // we can get back the error code from the public member.
            if (exc.error_code != DFB_FILENOTFOUND)
            {
                throw;
            }
            else
            {
                printf("File not found: %s\n", filename.c_str());
                return;
            }
        }
        ...
    }

*/

#ifndef EXT_DIRECTFB_HPP_R59TAE5TTV
#define EXT_DIRECTFB_HPP_R59TAE5TTV

#include <directfb.h>
#if D_DECLARE_INTERFACE
#define HAS_WATER_3VQ5DJV0B
#include <directfb_water.h>
#endif
#include "../memory.hpp"
#include "../boilerplates.hpp"

IMPLEMENT_ENUM_BITWISE_OPERATORS(
    (DFBDisplayLayerCapabilities)
    (DFBScreenCapabilities)
    (DFBDisplayLayerOptions)
    (DFBDisplayLayerBufferMode)
    (DFBSurfaceDescriptionFlags)
    (DFBPaletteDescriptionFlags)
    (DFBSurfaceCapabilities)
    (DFBPaletteCapabilities)
    (DFBSurfaceDrawingFlags)
    (DFBSurfaceBlittingFlags)
    (DFBSurfaceRenderOptions)
    (DFBAccelerationMask)
    (DFBDisplayLayerTypeFlags)
    (DFBInputDeviceTypeFlags)
    (DFBInputDeviceCapabilities)
    (DFBWindowDescriptionFlags)
    (DFBDataBufferDescriptionFlags)
    (DFBWindowCapabilities)
    (DFBWindowOptions)
    (DFBFontAttributes)
    (DFBFontDescriptionFlags)
//  (DFBSurfaceHintFlags)
    (DFBDisplayLayerSourceCaps)
    (DFBInputDeviceAxisInfoFlags)
    (DFBVideoProviderCapabilities)
    (DFBVideoProviderPlaybackFlags)
    (DFBVideoProviderAudioUnits)
    (DFBColorAdjustmentFlags)
    (DFBDisplayLayerConfigFlags)
    (DFBScreenMixerCapabilities)
    (DFBScreenMixerConfigFlags)
    (DFBScreenOutputCapabilities)
    (DFBScreenOutputConnectors)
    (DFBScreenOutputSignals)
    (DFBScreenOutputSlowBlankingSignals)
    (DFBScreenOutputResolution)
    (DFBScreenOutputConfigFlags)
    (DFBScreenEncoderCapabilities)
    (DFBScreenEncoderType)
    (DFBScreenEncoderTVStandards)
    (DFBScreenEncoderFrequency)
    (DFBScreenEncoderConfigFlags)
//  (DFBScreenEncoderPictureFraming)
    (DFBSurfaceFlipFlags)
    (DFBSurfaceTextFlags)
    (DFBSurfaceLockFlags)
    (DFBSurfaceMaskFlags)
    (DFBInputDeviceButtonMask)
    (DFBInputEventFlags)
    (DFBWindowEventType)
    (DFBWindowEventFlags)
    (DFBVideoProviderEventType)
    (DFBVideoProviderEventDataSubType)
//  (DFBWindowCursorFlags)
    (DFBImageCapabilities)
    (DFBStreamCapabilities)
    (DFBInputDeviceLockState)
/*  (DFBGL2ContextDescriptionFlags)
    (DFBGL2ContextCapabilities) */
/*  (DFBWindowConfigFlags)
    (DFBWindowStateFlags) */
)

#if HAS_WATER_3VQ5DJV0B
IMPLEMENT_ENUM_BITWISE_OPERATORS(
    (WaterTransformFlags)
    (WaterTransformType)
    (WaterRenderMode)
    (WaterPaintOptions)
    (WaterElementFlags)
    (WaterShapeFlags)
)
#endif


namespace utils { namespace directfb {

/**
Members
-------
*/

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

/**
.. type:: type utils::directfb::unique_ptr<T> = utils::generic_unique_ptr<T, (unspecified)>

    Smart pointer type that asserts unique ownership to a DirectFB interface.
*/
UTILS_DEF_SMART_PTR_ALIAS(unique, xx_impl::DirectFBDeallocator)

/**
.. type:: type utils::directfb::shared_ptr<T> = utils::generic_shared_ptr<T, (unspecified)>

    Smart pointer type that asserts shared ownership to a DirectFB interface.
*/
UTILS_DEF_SMART_PTR_ALIAS(shared, xx_impl::DirectFBDeallocator)

/**
.. type:: class utils::directfb::exception : public std::exception

    The base exception raised by DirectFB functions.
*/
class exception : public std::exception
{
public:
    /**
    .. data:: DFBResult error_code

        The error code associated with this exception. A list of error codes can
        be found in `the DirectFB documentation
        <http://directfb.org/docs/DirectFB_Reference_1_5/types.html#DFBResult>`_.
    */
    DFBResult error_code;

    exception(DFBResult error_code, const char* filename = nullptr, int line = 0)
        : error_code(error_code),
          _filename(filename),
          _line(line)
    {}

    virtual const char* what() const noexcept
    {
        if (!_filename)
            return DirectFBErrorString(error_code);
        else
        {
            std::string error_message (_filename);
            error_message.push_back('(');
            error_message += std::to_string(_line);
            error_message += "): ";
            error_message += DirectFBErrorString(error_code);
            return error_message.c_str();
        }
    }

private:
    const char* _filename;
    int _line;
};

/**
.. function:: static inline void utils::directfb::checked(DFBResult error_code, const char* filename = nullptr, int line = 0)

    A convenient function wrapped around DirectFB functions that may return an
    error code. If the *error_code* is not DFB_OK, a
    :type:`utils::directfb::exception` will be thrown.
*/
static inline void checked(DFBResult error_code, const char* filename = nullptr, int line = 0)
{
    if (error_code != DFB_OK)
        throw exception(error_code, filename, line);
}

/**
.. macro:: UTILS_DIRECTFB_CHECKED(...) = utils::directfb::checked((__VA_ARGS__), __FILE__, __LINE__)

    A convenient macro wrapped around :func:`utils::directfb::checked` which
    includes the current line file name and line number.
*/
#define UTILS_DIRECTFB_CHECKED(...) utils::directfb::checked((__VA_ARGS__), __FILE__, __LINE__)



/**
.. type:: class utils::directfb::lock final

    This class is a RAII type to provide direct bytes access to a DirectFB
    surface. This mainly a wrapper around the ``Lock`` and ``Unlock`` methods.
    For example::

        utils::directfb::unique_ptr<IDirectFBSurface> surface (...);

        {
            utils::directfb::lock lck (surface.get());

            unsigned char* data = lck.data();
            // Get the raw bytes of the surface.

            // make the first column of the surface transparent (assuming ARGB).
            for (int i = 0; i < height_of_surface; ++ i)
            {
                int offset = i * lck.stride();
                memset(data + offset, 0, 4);
            }
        }
*/
class lock
{
public:
    /**
    .. function:: explicit lock(IDirectFBSurface* surface, DFBSurfaceLockFlags flags = DSLF_READ|DSLF_WRITE)

        Lock the surface for direct access.
    */
    explicit lock(IDirectFBSurface* surface,
                  DFBSurfaceLockFlags flags = DSLF_READ|DSLF_WRITE)
        : _surface(surface), _ptr(nullptr), _pitch(0)
    {
        checked(surface->Lock(surface, flags, &_ptr, &_pitch));
    }

    /**
    .. function:: ~lock()

        Unlock the surface.
    */
    ~lock()
    {
        _surface->Unlock(_surface);
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

    /**
    .. function:: unsigned char* data() const noexcept

        Get the raw bytes data associated with the surface.
    */
    unsigned char* data() const noexcept
    {
        return static_cast<unsigned char*>(_ptr);
    }

    /**
    .. function:: int stride() const noexcept

        Get the row stride (a.k.a. pitch) for the raw bytes data of the surface.
    */
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

