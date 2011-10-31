utils/property: Lightweight properties
======================================

This module provides a light-weight [property](http://en.wikipedia.org/wiki/Property_%28programming%29)
objects to C++. Adding a property only adds 1 extra byte to the class's size at
the end (not including the alignment), and the property access should be all
inlined down to the original getter and setter.

Synopsis
--------

```c++
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

...

my_complex z (3, 4);
std::cout << "Polar: " << z.abs << ", " << z.arg << std::endl;
// ^ prints: "Polar: 5, 0.927295"
z.abs = 20;
std::cout << "New z: " << z.re << ", " << z.im << std::endl;
// ^ prints: "New z: 12, 16"
```

UTILS_PROPERTIES(ClassName, last_member)
----------------------------------------
This macro sets up all necessary environment to allow properties to be declared.
Inside the scope it created, you could create the properties using the
declaration ``declprop::xxxx<T, functions...> property_name;``:

### declprop::read_only\<T, getter>

Declares a read-only property of type *T* by invoking *getter* of type
``T (ClassName::*)()``.

### declprop::write_only\<T, setter>

Declares a write-only property of type *T* by invoking *setter* of type
``void (ClassName::*)(T new_val)``.

### declprop::read_write_byval\<T, getter, setter>

Declares a read-write property of type *T*, where the setter will accept *T* by
value (``T``).

### declprop::read_write_byref\<T, getter, setter>

Declares a read-write property of type *T*, where the setter will accept *T* by
const-reference (``const T&``).

### declprop::read_write_movable\<T, getter, copy_setter, move_setter>

Declares a read-write property of type *T*, where there are two setters, one for
copying (accepting ``const T&``) and one for moving (accepting ``T&&``).

UTILS_PROPERTIES_FOR_EMPTY_CLASS(ClassName)
-------------------------------------------
Same as above, but is for empty **non-virtual** classes.

Caveats
=======
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

