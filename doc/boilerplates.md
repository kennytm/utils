utils/boilerplates
==================

When writing a library that depends on others, those will often expect some sort
of capabilities (e.g. outputing to ``std::ostream``) that has a fixed pattern of
implementation. This module provides macros for mixing in the implementation.

IMPLEMENT_ENUM_OSTREAM(ENUM_NAME, ENUM_MEMBERS_SEQ)
---------------------------------------------------

Implements the ``<<`` operator for ``std::ostream`` on an enum.

The parameter *ENUM_NAME* should be the type name of the enumeration, and
*ENUM_MEMBERS_SEQ* is a [Boost preprocessor sequence][1] of all members
in the enum.

It is assumed that the enum will be passed by value to the stream.

The printed out string will be of the form ``EnumName::enumMember``, or
``EnumName::<unknown>`` if not found.

Example:

```c++
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

int main(int argc, char** argv)
{
    StandardColor color = StandardColor::red;
    std::cout << color << std::endl;
    // prints "StandardColor::red"

    color = static_cast<StandardColor>(12);
    std::cout << color << std::endl;
    // prints "StandardColor::<unknown>"

    return 0;
}
```

IMPLEMENT_STRUCT_OSTREAM(STRUCT_NAME, STRUCT_MEMBERS_SEQ)
---------------------------------------------------------

Implements the ``<<`` operator for ``std::ostream`` on a structure.

The parameter *STRUCT_NAME* should be the type name of the structure, and
*STRUCT_MEMBERS_SEQ* is a [Boost preprocessor sequence][1] of all members in the
struct to be printed.

The printed out string will be of the form ``{member1, member2, member3}``.

Example:

```c++
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
```

It is assumed that the struct will be **passed by value** to the stream. If
you'd like to pass by reference, call the macro as::

```c++
IMPLEMENT_STRUCT_OSTREAM(const StructName&, (a)(b)(c)(d))
//                       ^^^^^^^^^^^^^^^^^
```

IMPLEMENT_ENUM_BITWISE_OPERATORS(ENUM_NAMES_SEQ)
------------------------------------------------

Implement the bitwise operators (``|``, ``&``, ``^``, ``~``) on a [Boost
preprocessor sequence][1] of enums, so that they can be treated as flags without
``static_cast``.

The sequence is only for convenience of defining the overloads over a large
number of enums. Every enum in the sequence has no relation with each other.

Example:

```c++
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
```

[1]: http://www.boost.org/doc/libs/1_47_0/libs/preprocessor/doc/data/sequences.html

