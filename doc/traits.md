utils/traits: Additional type traits
====================================
This module provides some additional type traits missing or not good enough in
the standard C++11 library and Boost.

DECLARE_HAS_TYPE_MEMBER(member_name)
------------------------------------

This macro declares a template *has_member_name* which will check whether a type
member *member_name* exists in a particular type.

```c++
DECLARE_HAS_TYPE_MEMBER(result_type)

...

printf("%d\n", has_result_type< std::plus<int> >::value);
// ^ prints '1' (true)
printf("%d\n", has_result_type< double(*)() >::value);
// ^ prints '0' (false)
```


utils::function_traits
----------------------
Obtain compile-time information about a function object.

| Member                                      | Description                                                       |
|---------------------------------------------|-------------------------------------------------------------------|
| ``utils::function_traits<F>::result_type``  | The type returned by the function object type *F*                 |
| ``utils::function_traits<F>::arity``        | Number of arguments the function will take (this is a ``size_t``) |
| ``utils::function_traits<F>::arg<n>::type`` | The type of the *n*-th argument                                   |

This template currently supports the following types:

* Normal function types (``R(T...)``), function pointers (``R(*)(T...)``) and
  function references (``R(&)(T...)`` and ``R(&&)(T...)``)
* Member functions (``R(C::*)(T...)``)
* ``std::function<F>``
* Type of lambda functions, and any other types that has a unique ``operator()``.
* Type of ``std::mem_fn`` (only for GCC's libstdc++ and LLVM's libc++).
  Following the C++ spec, the first argument will be a raw pointer.

utils::forward_like<U>
----------------------
Forward an object like the l/r-valuedness of *U*.

utils::copy_cv
--------------
Copy the CV-qualifier of one type to another, e.g.

```c++
utils::copy_cv<const int, double>::type f = 4.0;
// ^ 'f' is a 'const double'.
```

utils::pointee
--------------

Returns the type that pointed by the input template parameter. This is similar
to ``std::remove_pointer``, but works with iterators too.

utils::ct_integers_list\<n...>
------------------------------
This is a compile-time data structure to hold a list of integers (``size_t``).

* ``push_back<m>::type`` --- Push an integer *m* to the end of the list.

utils::ct_iota\<n>::type
------------------------
Generate ``utils::ct_integers_list<0, 1, ..., n-1>``.


