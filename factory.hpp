//-------------------------------------------
// utils/factory.hpp: Predicate-based factory
//-------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**

``<utils/factory.hpp>`` --- Predicate-based factory
===================================================

This module provides a generic way to create a unique object of some subclass in
a class hierarchy. The subclass is chosen in runtime using pre-registered
functions (the factory methods).

Example::

    // Image.hpp:

    #include <utils/factory.hpp>

    // Construct the class hierarchy.
    class Image { ... };
    class PNGImage : public Image { ... };
    class JPEGImage : public Image { ... };
    ...

    // We will use this method to read an image file. If it is a .png file, it
    // will return a PNGImage instance; if it is a .jpeg file, it will return a
    // JPEGImage instance, etc.
    std::unique_ptr<Image> read_image(const std::string& filename);

    // Alias the factory registrar, but don't instantiate it.
    extern template class utils::factory<decltype(read_image)>;
    typedef utils::factory<decltype(&read_image)> ImageFactoryRegistrar;

::

    // Image.cpp

    #include "Image.hpp"

    // Instantiate the template in the .cpp file.
    template class utils::factory<decltype(&read_image)>;

    // Implement the read_image method using the factory.
    std::unique_ptr<Image> read_image(const std::string& filename)
    {
        return ImageFactoryRegistrar::create(filename);
    }

::

    // .cpp of the subclasses

    #include "Image.hpp"

    // Create the factory methods.
    std::unique_ptr<Image> create_png_image(const std::string& filename)
    {
        if (!ends_with(filename, ".png"))
            return nullptr;
        return std::unique_ptr<Image>(new PNGImage(filename));
    }

    // Register the factory.
    ImageFactoryRegistrar png_registrar (&create_png_image);

*/

#ifndef FACTORY_HPP_X2ZEGUKD8T
#define FACTORY_HPP_X2ZEGUKD8T 1

#include <vector>
#include <algorithm>
#include <exception>
#include <utils/traits.hpp>

namespace utils {

/**
.. type:: class utils::factory_error : public std::exception

    An error thrown when all factory methods failed to create a valid instance,
    and some registered factory methods throw exceptions.
*/
class factory_error : public std::exception
{
public:
    factory_error(std::vector<std::exception_ptr>&& exceptions);
    virtual ~factory_error() noexcept;
    virtual const char* what() const noexcept override;

    /**
    .. data:: const std::vector<std::exception_ptr> exceptions

        A vector of exceptions thrown inside the factory.
    */
    const std::vector<std::exception_ptr> exceptions;

private:
    std::string _what;
};


/**
.. type:: class utils::factory<FactoryMethodType, size_t tag = 0> final

    The factory registrar type for registering factory methods of type
    "*FactoryMethodType*". The *FactoryMethodType* should be a function type
    which returns a unique or shared pointer of the root class to be built.

    Do not instantiate this template class in a .hpp. This could be done by
    declaring an extern template.

    Like normal C++, multiple instantiation of the same "factory" type will
    share the same registrar. If there are distinct registrar having the same
    *FactoryMethodType*, the *tag* parameter could be used for disambiguation.
*/
template <typename FactoryMethodType, size_t tag = 0>
class factory final
{
public:
    /**
    .. function:: factory(FactoryMethodType factory_method)

        Register a factory method.

        The factory method should return a pointer to the desired subclass if
        the arguments can be accepted, and return nullptr otherwise.
    */
    factory(FactoryMethodType* factory_method)
        : _factory_method(factory_method)
    {
        // TODO: Thread safety?
        get_factories().push_back(factory_method);
    }

    /**
    .. function:: ~factory()

        Unregister a factory method.
    */
    ~factory()
    {
        // TODO: Thread safety?
        auto& factories = get_factories();
        auto end = std::remove(factories.begin(), factories.end(), _factory_method);
        factories.erase(end, factories.end());
    }

    /**
    .. function:: static std::result_of<FactoryMethodType>::type create(T... args)

        Create an instance from the factory methods. The type *R* is the return
        type of *FactoryMethodType* and *A...* are the arguments of
        *FactoryMethodType*.

        If any factory method created an instance, it will be returned. If all
        methods created nothing (returned nullptr), nullptr will be returned.
        If multiple factory methods will create valid instances, the one
        corresponding to the last registered method will be returned.

        If a factory method throws an exception, it will be treated as if it
        cannot create a valid instance (returned nullptr) and skip to the next
        method. However, if all factories failed to create instances, all these
        caught exceptions will be rethrown collectively as a
        :type:`~utils::factory_error`.
    */
    template <typename... T>
    static typename function_traits<FactoryMethodType>::result_type create(T... args)
    {
        std::vector<std::exception_ptr> exceptions;

        // TODO: Thread safety?
        for (auto method : get_factories())
        {
            try
            {
                auto retval = method(args...);
                if (retval)
                    return std::move(retval);
            }
            catch (...)
            {
                exceptions.push_back(std::current_exception());
            }
        }

        if (exceptions.empty())
            return nullptr;
        else
            throw factory_error(std::move(exceptions));
    }

private:
    typedef std::vector<FactoryMethodType*> factory_list;

    __attribute__((noinline))
    static factory_list& get_factories()
    {
        static factory_list factories;
        return factories;
    }

    FactoryMethodType* _factory_method;
};

}

#endif

