//--------------------------
// Common boilerplate codes
//--------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**

``<utils/boilerplates.hpp>`` --- Implement some common overloads
================================================================

When writing a library that depends on others, those will often expect some sort
of capabilities (e.g. outputing to ``std::ostream``) that has a fixed pattern of
implementation. This module provides macros for mixing in the implementation.

*/

#ifndef BOILERPLATES_HPP_KJR7FBERPMTGWRK9
#define BOILERPLATES_HPP_KJR7FBERPMTGWRK9 1

#include <ostream>
#include <boost/preprocessor.hpp>

/**
Members
-------
*/

#define IMPLEMENT_ENUM_OSTREAM_PRE_NIVF9GBVKILA0PB9(ENUM_NAME)                  \
    std::ostream& operator<<(std::ostream& stream, ENUM_NAME e)                 \
    {                                                                           \
        const char* name;                                                       \
        switch (e)                                                              \
        {                                                                       \
            default:                                                            \
                name = #ENUM_NAME "::<unknown>";                                \
                break;                                                          \

#define IMPLEMENT_ENUM_OSTREAM_CASE_HZM4576EWJ49GGB9(r, ENUM_NAME, ENUM_MEMBER) \
            case ENUM_NAME::ENUM_MEMBER:                                        \
                name = #ENUM_NAME "::" BOOST_PP_STRINGIZE(ENUM_MEMBER);         \
                break;                                                          \

#define IMPLEMENT_ENUM_OSTREAM_POST_WT03SAMDJQAJ1YVI                            \
       }                                                                        \
       return stream << name;                                                   \
   }

/**
.. macro:: IMPLEMENT_ENUM_OSTREAM(ENUM_NAME, ENUM_MEMBERS_SEQ)

    Implements the ``<<`` operator for ``std::ostream`` on an enum.

    The parameter *ENUM_NAME* should be the type name of the enumeration, and
    *ENUM_MEMBERS_SEQ* is a :term:`Boost preprocessor sequence` of all members
    in the enum.

    It is assumed that the enum will be passed by value to the stream.

    The printed out string will be of the form ``EnumName::enumMember``, or
    ``EnumName::<unknown>`` if not found.

    Example::

        enum class StandardColor
        {
            white,
            red,
            green,
            blue
        };
        std::ostream& operator<<(std::ostream&, StandardColor);

        ...

        IMPLEMENT_ENUM_OSTREAM(StandardColor, (white)(red)(green)(blue))
        // Implements 'std::ostream& operator<<(std::ostream&, StandardColor)'

        ...

        #include <iostream>

        int main(int argc, char* argv[])
        {
            StandardColor color = StandardColor::red;
            std::cout << color << std::endl;
            // prints "StandardColor::red"

            color = static_cast<StandardColor>(12);
            std::cout << color << std::endl;
            // prints "StandardColor::<unknown>"

            return 0;
        }
*/
#define IMPLEMENT_ENUM_OSTREAM(ENUM_NAME, ENUM_MEMBERS_SEQ)                     \
        IMPLEMENT_ENUM_OSTREAM_PRE_NIVF9GBVKILA0PB9(ENUM_NAME)                  \
        BOOST_PP_SEQ_FOR_EACH(IMPLEMENT_ENUM_OSTREAM_CASE_HZM4576EWJ49GGB9,     \
                              ENUM_NAME,                                        \
                              ENUM_MEMBERS_SEQ)                                 \
        IMPLEMENT_ENUM_OSTREAM_POST_WT03SAMDJQAJ1YVI

#define IMPLEMENT_STRUCT_OSTREAM_PRE_NLS0KBFD97M(STRUCT_NAME)                   \
    std::ostream& operator<<(std::ostream& stream, STRUCT_NAME s)               \
    {                                                                           \
        return stream << '{'

#define IMPLEMENT_STRUCT_OSTREAM_MEMBER_0B23DUC83WP8(r, d, INDEX, STRUCT_MEMBER)\
        << BOOST_PP_EXPR_IF(INDEX, ", " <<) s.STRUCT_MEMBER

#define IMPLEMENT_STRUCT_OSTREAM_POST_L8YB90ELQ0M                               \
        << '}';                                                                 \
    }

/**
.. macro:: IMPLEMENT_STRUCT_OSTREAM(STRUCT_NAME, STRUCT_MEMBERS_SEQ)

    Implements the ``<<`` operator for ``std::ostream`` on a structure.

    The parameter *STRUCT_NAME* should be the type name of the structure, and
    *STRUCT_MEMBERS_SEQ* is a :term:`Boost preprocessor sequence` of all members
    in the struct to be printed.

    The printed out string will be of the form ``{member1, member2, member3}``.

    Example::

        struct Vector3
        {
            double x;
            double y;
            double z;
        };
        std::ostream& operator<<(std::ostream&, Vector3);

        ...

        IMPLEMENT_STRUCT_OSTREAM(Vector3, (x)(y)(z))
        // Implements 'std::ostream& operator<<(std::ostream&, Vector3)'

        ...

        #include <iostream>

        int main(int argc, char* argv[])
        {
            Vector3 vec {1.0, 0.0, -2.5};

            std::cout << vec << std::endl;
            // prints "{1, 0, -2.5}"

            return 0;
        }

    It is assumed that the struct will be **passed by value** to the stream. If
    you'd like to pass by reference, call the macro as::

        IMPLEMENT_STRUCT_OSTREAM(const StructName&, (a)(b)(c)(d))
        //                       ^^^^^^^^^^^^^^^^^
*/
#define IMPLEMENT_STRUCT_OSTREAM(STRUCT_NAME, STRUCT_MEMBERS_SEQ)               \
        IMPLEMENT_STRUCT_OSTREAM_PRE_NLS0KBFD97M(STRUCT_NAME)                   \
        BOOST_PP_SEQ_FOR_EACH_I(IMPLEMENT_STRUCT_OSTREAM_MEMBER_0B23DUC83WP8, , \
                                STRUCT_MEMBERS_SEQ)                             \
        IMPLEMENT_STRUCT_OSTREAM_POST_L8YB90ELQ0M

#define OVERLOAD_ENUM_BINARY_BIT_OP_T3Y80DQY0Q(EnumName, op) \
    static inline EnumName operator op(EnumName a, EnumName b) noexcept \
    { \
        return static_cast<EnumName>( \
            static_cast<std::underlying_type<EnumName>::type>(a) op \
            static_cast<std::underlying_type<EnumName>::type>(b) \
        ); \
    } \
    static inline EnumName& operator op##=(EnumName& a, EnumName b) noexcept \
    { \
        return (a = (a op b)); \
    }

#define IMPLEMENT_ENUM_BIT_OPS_9MKW9IBN5Y(r, d, EnumName) \
    OVERLOAD_ENUM_BINARY_BIT_OP_T3Y80DQY0Q(EnumName, &) \
    OVERLOAD_ENUM_BINARY_BIT_OP_T3Y80DQY0Q(EnumName, |) \
    OVERLOAD_ENUM_BINARY_BIT_OP_T3Y80DQY0Q(EnumName, ^) \
    static inline EnumName operator~(EnumName a) noexcept \
    { \
        return static_cast<EnumName>( \
            ~static_cast<std::underlying_type<EnumName>::type>(a) \
        ); \
    }

/**
.. macro:: IMPLEMENT_ENUM_BITWISE_OPERATORS(ENUM_NAMES_SEQ)

    Implement the bitwise operators (``|``, ``&``, ``^``, ``~``) on a
    :term:`Boost preprocessor sequence` of enums, so that they can be treated as
    flags without ``static_cast``.

    The sequence is only for convenience of defining the overloads over a large
    number of enums. Every enum in the sequence has no relation with each other.

    Example::

        enum class ColorFlag : unsigned
        {
            red = 1,
            green = 2,
            blue = 4
        };

        IMPLEMENT_ENUM_BITWISE_OPERATORS((ColorFlag))

        ...

        ColorFlag yellow = ColorFlag::red | ColorFlag::green;
        // compiler will not complain.
*/
#define IMPLEMENT_ENUM_BITWISE_OPERATORS(ENUM_NAMES_SEQ)                        \
    BOOST_PP_SEQ_FOR_EACH(IMPLEMENT_ENUM_BIT_OPS_9MKW9IBN5Y, , ENUM_NAMES_SEQ)

#endif

