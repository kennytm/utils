//------------------------------------------------
// utils/property: Light-weight properties in C++
//------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef PROPERTY_HPP_K8TCXM1VK5M
#define PROPERTY_HPP_K8TCXM1VK5M 1

#include <cstddef>
#include <iosfwd>
#include "traits.hpp"

namespace utils {

namespace xx_impl
{
    #define DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, op) \
        template <typename T_IRAPKRK260L> \
        ThisType& operator op##=(T_IRAPKRK260L&& t) \
        { \
            *this = *this op std::forward<T_IRAPKRK260L>(t); \
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
        DECLARE_COMPOUND_OP_AP7B0QH2PC(ThisType, <<)

    template <typename Owner, typename OwnerPtrConvertor>
    struct generic_property_store
    {
        template <typename T, T (Owner::*getter)() const>
        struct read_only
        {
            operator T() const
            {
                return (OwnerPtrConvertor()(this)->*getter)();
            }

            template <typename V>
            bool operator==(V&& other) const
            {
                return static_cast<T>(*this) == std::forward<V>(other);
            }

            friend std::ostream& operator<<(std::ostream& os, const read_only& pr)
            {
                return os << static_cast<T>(pr);
            }

            T operator->() const
            {
                static_assert(std::is_pointer<T>::value, "Cannot call -> on non-pointers");
                return static_cast<T>(*this);
            }
        };

        template <typename T, void (Owner::*setter)(T)>
        struct write_only
        {
            write_only& operator=(T value)
            {
                (OwnerPtrConvertor()(this)->*setter)(std::forward<T>(value));
                return *this;
            }
        };

        template <typename T, T (Owner::*getter)() const, void (Owner::*setter)(T)>
        struct read_write_byval
            : public read_only<T, getter>,
              private write_only<T, setter>
        {
            using write_only<T, setter>::operator=;
            DECLARE_ALL_COMPOUND_OPS_P6V6HIRH89(read_write_byval)
        };

        template <typename T, T (Owner::*getter)() const, void (Owner::*setter)(const T&)>
        struct read_write_byref
            : public read_only<T, getter>,
              private write_only<const T&, setter>
        {
            using write_only<const T&, setter>::operator=;
            DECLARE_ALL_COMPOUND_OPS_P6V6HIRH89(read_write_byref)
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
        };

    };

    #undef DECLARE_OP_AP7B0QH2PC
    #undef DECLARE_ALL_COMPOUND_OPS_P6V6HIRH89

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

#define UTILS_PROPERTIES(ClassName, last_member) \
    private: \
        typedef ::utils::property_store<ClassName, \
                                        decltype(ClassName::last_member), \
                                        &ClassName::last_member> declprop; \
    public: \
        union

#define UTILS_PROPERTIES_FOR_EMPTY_CLASS(ClassName) \
    private: \
        typedef ::utils::property_store_empty<ClassName> declprop; \
    public: \
        union

#endif

