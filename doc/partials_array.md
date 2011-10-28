utils::partials_array: Data structure to cache the partial sums
===============================================================

The class ``partials_array`` is used for storing partial sums. For example, in a
sequence of animations, if we keep the a partial sum of durations of each
animation, then we can efficiently (O(log N)) find out which is the active
animation at a randomly given time.

Synopsis
--------

```c++
#include <chrono>
#include <functional>
#include <utils/partials_array.hpp>
using std::chrono::steady_clock;

int main(int argc, char* argv[])
{
    auto durations = utils::make_partials_array(std::mem_fn(&Animation::duration));
    // create a new partials array

    std::vector<Animation> animations = {anim0, anim1, anim2, anim3, anim4};
    durations.update(animations.cbegin(), animations.cend());
    // update the cacher with some content.

    animations.push_back(anim5);
    animations.push_back(anim6);
    durations.update(animations.cbegin(), animations.cend());
    // refresh the cache.

    animations.push_back(anim7);
    durations.push_back(anim7);
    // you can also push_back it directly.

    steady_clock::duration total_duration = durations.total();
    // the back gives the total sum in the cache.

    steady_clock::duration anim4_duration = durations[4];
    // this gives the value (*not* partial sum) at index 4.

    auto result = durations.find(animations.cbegin(), animations.cend(), std::chrono::seconds(10));
    auto which_anim_it = result.first;
    steady_clock::duration corresponding_duration = result.second;
    // find the animation which is active at 10 second.

    ...
}
```

Members
-------

```c++
template <typename ComputerType>
class partials_array
```

This is the partials array type, using the function object type *ComputerType*
to obtain the partial sums.

```c++
{
public:
    typedef ComputerType computer_type;
    typedef typename utils::function_traits<ComputerType>::result_type value_type;

    partials_array();
    explicit partials_array(const ComputerType& computer);
```

Creates a partials array using the computer instance (or the default one).

```c++
    template <typename RandomAccessIterator>
    void update(RandomAccessIterator begin, RandomAccessIterator end);
```

Recompute the partial sums using the given range. The *RandomAccessIterator*
must be an iterator that yields types acceptable by *ComputerType*, and if it
has been called before, the front part of the range must not change.

This will take time proportional to the difference between the input range's
size, and the cacher size.

If the iterator range has changed you have to first call ``invalidate()`` or
``invalidate_all()`` to invalidate the cache.

```c++
    template <typename... Args>
    void push_back(Args&&... args);
```

Add the value of the *args* to this array.

```c++
    value_type back() const;
```

Return the last value in this array (*not* the partial sum). If the array is
empty, 0 will be returned.

```c++
    value_type operator[](size_t index) const;
```

Return a value in this array (*not* the partial sum) given the index.

```c++
    size_t size() const;
```

Return the number of items in the array.

```c++
    void invalidate(size_t count);
```

Remove the last *count* values from the array.

```c++
    void invalidate_all();
```

Clear the array. It will become empty afterward.

```c++
    value_type total() const
```

Returns the sum of values in the array. It is an O(1) operation. If the array is
empty, 0 is returned.

```c++
    template <typename RandomAccessIterator>
    std::pair<RandomAccessIterator, value_type>
        find(RandomAccessIterator begin, RandomAccessIterator end, value_type partial);
```

Find an object such that the partial sum up to that point is strictly less than
*partial*. The result is a pair, the first value being the iterator of the
object found, and the second being the value (*not* the partial sum) associated
with that object.

As an illustration, if the array contains the values {1, 4, 2, 9}, i.e. the
partial sums are {1, 5, 7, 16}, then finding for "6" will yield the second
object (i.e. 4).

This operation takes logarithmic time (O(log(end - begin))).

```c++
};

template <typename ComputerType>
partials_array<ComputerType> make_partials_array(const ComputerType& computer_type);
```

This is a convenient function to construct a partials array give its computer.


