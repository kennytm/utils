//------------------------------------------------
// utils/property: Light-weight properties in C++
//------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**

``<utils/property.hpp>`` --- Lightweight properties
===================================================

This module provides a light-weight `property
<http://en.wikipedia.org/wiki/Property_%28programming%29>`_ objects to C++.
Adding a property only adds 1 extra byte to the class's size at the end (not
including the alignment), and the property access should be all inlined down to
the original getter and setter.

Synopsis
--------

Declaring::

    #include <cmath>
    #include <utils/property.hpp>

    struct my_complex
    {
        double re;
        double im;

        my_complex(double re_, double im_) noexcept : re(re_), im(im_) {}

        // First, define the conventional getters and setters.
        double get_abs() const noexcept
        {
            return hypot(re, im);
        }
        void set_abs(double new_abs) noexcept
        {
            double ratio = new_abs / get_abs();
            re *= ratio;
            im *= ratio;
        }

        double get_arg() const noexcept
        {
            return atan2(im, re);
        }
        void set_arg(double new_arg) noexcept
        {
            double cur_abs = get_abs();
            re = cur_abs * cos(new_arg);
            im = cur_abs * sin(new_arg);
        }

        // Then, declare the properties
        UTILS_PROPERTIES(my_complex, im)
        {
            declprop::read_write_byval<double, &my_complex::get_abs, &my_complex::set_abs> abs;
            declprop::read_write_byval<double, &my_complex::get_arg, &my_complex::set_arg> arg;
        };
    };

Using (similar to normal variables)::

    my_complex z (3, 4);
    std::cout << "Polar: " << z.abs << ", " << z.arg << std::endl;
    // ^ prints: "Polar: 5, 0.927295"
    z.abs = 20;
    std::cout << "New z: " << z.re << ", " << z.im << std::endl;
    // ^ prints: "New z: 12, 16"

*/

#ifndef PROPERTY_HPP_K8TCXM1VK5M
#define PROPERTY_HPP_K8TCXM1VK5M 1

#include <cstddef>
#include <iosfwd>
#include "traits.hpp"

namespace utils {

/**
Members
-------
*/

namespace xx_impl
{
    #define DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, op) \
        template <typename T_IRAPKRK260L> \
        ThisType& operator op##=(T_IRAPKRK260L&& t) \
        { \
            this->set(this->get() op std::forward<T_IRAPKRK260L>(t)); \
            return *this; \
        }

    #define DECLARE_ALL_COMPOUND_OPS_P6V6HIRH89(ThisType) \
        DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, +) \
        DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, -) \
        DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, *) \
        DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, /) \
        DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, %) \
        DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, &) \
        DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, |) \
        DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, ^) \
        DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, >>) \
        DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, <<) \

    // declare the protected structors to avoid the user accidentally using
    // 'auto xxxx = property' and receives garbage.
    #define DECLARE_PROTECTED_STRUCTORS_GG8O624RPLU(ThisType) \
        protected: \
            ThisType() = default; \
            ThisType(ThisType&&) = default; \
            ThisType(const ThisType&) = default; \
            ThisType& operator=(const ThisType&) = default; \
            ThisType& operator=(ThisType&&) = default; \
            friend Owner;

    template <typename Owner, typename OwnerPtrConvertor>
    struct generic_property_store
    {
        template <typename T, T (Owner::*getter)() const>
        struct read_only
        {
            T get() const { return (OwnerPtrConvertor()(this)->*getter)(); }

            operator T() const { return get(); }
            T operator->() const { return get(); }

            template <typename V>
            bool operator==(V&& other) const
            {
                return get() == std::forward<V>(other);
            }

            friend std::ostream& operator<<(std::ostream& os, const read_only& pr)
            {
                return os << pr.get();
            }

            /* Note to user: If you see an 'error: ... is protected' here, it
               means you have tried to use 'auto' with a property. It does not
               work in C++. Please declare with explicit type.
             */
            DECLARE_PROTECTED_STRUCTORS_GG8O624RPLU(read_only)
        };

        template <typename T, void (Owner::*setter)(T)>
        struct write_only
        {
            template <typename U>
            void set(U&& value)
            {
                (OwnerPtrConvertor()(this)->*setter)(std::forward<U>(value));
            }

            write_only& operator=(T value)
            {
                set(std::forward<T>(value));
                return *this;
            }

            DECLARE_PROTECTED_STRUCTORS_GG8O624RPLU(write_only)
        };

        template <typename T, T (Owner::*getter)() const, void (Owner::*setter)(T)>
        struct read_write_byval
            : public read_only<T, getter>,
              private write_only<T, setter>
        {
            using write_only<T, setter>::operator=;
            DECLARE_ALL_COMPOUND_OPS_P6V6HIRH89(read_write_byval)

            /* Note to user: If you see an 'error: ... is protected' here, it
               means you have tried to use 'auto' with a property. It does not
               work in C++. Please declare with explicit type.
             */
            DECLARE_PROTECTED_STRUCTORS_GG8O624RPLU(read_write_byval)
        };

        template <typename T, T (Owner::*getter)() const, void (Owner::*setter)(const T&)>
        struct read_write_byref
            : public read_only<T, getter>,
              private write_only<const T&, setter>
        {
            using write_only<const T&, setter>::operator=;
            DECLARE_ALL_COMPOUND_OPS_P6V6HIRH89(read_write_byref)

            /* Note to user: If you see an 'error: ... is protected' here, it
               means you have tried to use 'auto' with a property. It does not
               work in C++. Please declare with explicit type.
             */
            DECLARE_PROTECTED_STRUCTORS_GG8O624RPLU(read_write_byref)
        };

        template <typename T, T (Owner::*getter)() const,
                              void (Owner::*copy_setter)(const T&),
                              void (Owner::*move_setter)(T&&)>
        struct read_write_movable
            : public read_only<T, getter>,
              private write_only<const T&, copy_setter>,
              private write_only<T&&, move_setter>
        {
            using write_only<const T&, copy_setter>::operator=;
            using write_only<T&&, move_setter>::operator=;
            DECLARE_ALL_COMPOUND_OPS_P6V6HIRH89(read_write_movable)

            /* Note to user: If you see an 'error: ... is protected' here, it
               means you have tried to use 'auto' with a property. It does not
               work in C++. Please declare with explicit type.
             */
            DECLARE_PROTECTED_STRUCTORS_GG8O624RPLU(read_write_movable)
        };

    };

    #undef DECLARE_OP_AP7B0QH2PC
    #undef DECLARE_ALL_COMPOUND_OPS_P6V6HIRH89
    #undef DECLARE_PROTECTED_STRUCTORS_GG8O624RPLU

    template <typename Owner, typename U, U Owner::* last_member>
    class owner_ptr_convertor_with_offset
    {
    private:
        template <typename T>
        static inline ptrdiff_t unsafe_reinterpret_cast_to_ptrdiff_t(const T& input) noexcept
        {
            static_assert(sizeof(T) == sizeof(ptrdiff_t), "sizeof(T) != sizeof(ptrdiff_t)");
            return *reinterpret_cast<const ptrdiff_t*>(&input);
        }

    public:
        template <typename T>
        typename copy_cv<T, Owner>::type* operator()(T* this_) const noexcept
        {
            return reinterpret_cast<typename copy_cv<T, Owner>::type*>(
                reinterpret_cast<typename copy_cv<T, char>::type*>(this_)
                    - sizeof(U)
                    - unsafe_reinterpret_cast_to_ptrdiff_t(last_member)
            );
        }
    };

    template <typename Owner>
    class owner_ptr_convertor_without_offset
    {
    public:
        template <typename T>
        typename copy_cv<T, Owner>::type* operator()(T* this_) const noexcept
        {
            static_assert(sizeof(Owner) == sizeof(T), "The owner class is not empty");
            return reinterpret_cast<typename copy_cv<T, Owner>::type*>(this_);
        }
    };
}

template <typename Owner, typename U, U Owner::* last_member>
class property_store
    : public xx_impl::generic_property_store<Owner,
          xx_impl::owner_ptr_convertor_with_offset<Owner, U, last_member>>
{};

template <typename Owner>
class property_store_empty
    : public xx_impl::generic_property_store<Owner,
          xx_impl::owner_ptr_convertor_without_offset<Owner>>
{};

}

/**
.. macro:: UTILS_PROPERTIES(ClassName, last_member)

    This macro sets up all necessary environment to allow properties to be
    declared. Inside the scope it created, you could create the properties using
    the declaration ``declprop::xxxx<T, functions...> property_name;``, where
    ``declprop`` is a typedef of an appropriate :type:`~utils::property_store`.

    .. warning::
        The *last_member* **must** be the data member just preceding the
        invocation of this macro. Otherwise, the properties will not work (and
        will likely crash the program).
*/
#define UTILS_PROPERTIES(ClassName, last_member) \
    private: \
        typedef ::utils::property_store<ClassName, \
                                        decltype(ClassName::last_member), \
                                        &ClassName::last_member> declprop; \
    public: \
        union

/**
.. macro:: UTILS_PROPERTIES_FOR_EMPTY_CLASS(ClassName)

    Similar as :macro:`UTILS_PROPERTIES`, but is for **non-virtual** empty
    classes which has no data members. The ``declprop`` will be aliased to
    :type:`~utils::property_store_empty`.
*/
#define UTILS_PROPERTIES_FOR_EMPTY_CLASS(ClassName) \
    private: \
        typedef ::utils::property_store_empty<ClassName> declprop; \
    public: \
        union

/**
.. type:: class utils::property_store<Owner, U, U Owner::* last_member> final
          class utils::property_store_empty<Owner> final
    :pod:

    These classes are type containers to various property types. These store
    classes provide a way for the property type to automatically determine the
    address of the owner (via pointer arithmetic involving the *last_member*).

    .. type:: struct read_only<T, T (Owner::* getter)() const>

        Creates a read-only property of type *T*. When the property is accessed
        as an *T*, the *getter* will be invoked.

        .. function:: T get() const

            Access the getter directly.

    .. type:: struct write_only<T, void (Owner::* setter)(T)>

        Creates a write-only property of type *T*. When the property is assigned,
        the *setter* will be invoked.

        .. function:: void set<U>(U&& value)

            Access the setter directly.

    .. type:: struct read_write_byval<T, T (Owner::* getter)() const, void (Owner::* setter)(T)>

        Creates a read-write property passed by value.

    .. type:: struct read_write_byref<T, T (Owner::* getter)() const, void (Owner::* setter)(const T&)>

        Creates a read-write property passed by const reference.

    .. type:: struct read_write_movable<T, T (Owner::* getter)() const, void (Owner::* copy_setter)(const T&), void (Owner::* move_setter)(T&&)>

        Creates a read-write property passed by reference, and with a
        move-assignment setter as well.
*/

/**

Caveats
-------

C++ does not have native support of properties. This module can provide a very
good simulation, but after all it is just a simulation. Using properties may
have some unexpected consequences that the user may need to be aware of.

* Since properties are implemented as data members in a union, the class's
  braced initializer list will not work properly (e.g. in above you cannot write
  ``my_complex z = {3, 4};``).
* Properties can be implicitly converted to its type, but that does not mean the
  property *has* that type. You may need to explicitly cast it to the desired
  type in some variadic or template functions (e.g.
  ``printf("%g", static_cast<double>(z.abs));``).
* Member access (``a.x``) will not work.
* ``++`` and ``--`` are not supported yet. Use ``+= 1`` and ``-= 1`` if needed.
* ``auto`` cannot be used to receive a property value.

*/

#endif

