//------------------------------------------------------------------
// partials_array: Sequential container that caches the partial sum
//------------------------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/**

``<utils/partials_array.hpp>`` --- Data structure to cache the partial sums
===========================================================================

The class :type:`~utils::partials_array` is used for storing partial sums. For
example, in a sequence of animations, if we keep the a partial sum of durations
of each animation, then we can efficiently (O(log N)) find out which is the
active animation at a randomly given time.

Synopsis
--------

::

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

*/

#ifndef PARTIALS_ARRAY_HPP_0YQ35OAC41IZ4CXR
#define PARTIALS_ARRAY_HPP_0YQ35OAC41IZ4CXR 1

#include <vector>
#include <algorithm>
#include <type_traits>
#include "traits.hpp"

namespace utils {

/**
Members
-------
*/

/**
.. type:: class utils::partials_array<ComputerType> final
    :default_constructible:
    :noncopyable:
    :nonmovable:

    This is the partials array type, using the function object type
    *ComputerType* to obtain the partial sums. The *ComputerType* is supposed to
    yield only nonnegative values.

    The function evaluation result of *ComputerType* must support ``<`` (i.e.
    has a weak strict ordering), be closed in ``+``, ``-``, and be default
    constructable (i.e. forms a monoid with addition).
*/
template <typename ComputerType>
class partials_array
{
public:
    /**
    .. type:: type computer_type = ComputerType

        This is the type that converts an object into a value that is to be
        partial-summed (e.g. giving the duration of an animation).
    */
    typedef ComputerType computer_type;

    /*
    .. type:: type value_type = (result of ComputerType)

        The value type produced by *ComputerType*.
    */
    typedef typename function_traits<ComputerType>::result_type value_type;

    /**
    .. function:: explicit partials_array(const ComputerType& partials_computer = ComputerType())

        The constructor.
    */
    partials_array() = default;
    explicit partials_array(const ComputerType& computer) : _computer(computer) {}

    /**
    .. function:: void update<RandomAccessIterator>(RandomAccessIterator begin, RandomAccessIterator end)

        Recompute the partial sums using the given range. The
        *RandomAccessIterator* must be an iterator that yields types acceptable
        by *ComputerType*, and if it has been called before, the front part of
        the range must not change.

        This will take time proportional to the difference between the input
        range's size, and the cacher size.

        If the iterator range has changed you have to first call
        ``invalidate()`` or ``invalidate_all()`` to invalidate the cache.
    */
    template <typename RandomAccessIterator>
    void update(RandomAccessIterator begin, RandomAccessIterator end)
    {
        size_t partials_size = _partials.size();
        auto collection_size = static_cast<size_t>(end - begin);
        if (collection_size <= partials_size)
            return;

        _partials.reserve(collection_size);

        std::advance(begin, partials_size);
        if (partials_size == 0)
        {
            _partials.push_back(_computer(*begin));
            ++ begin;
        }

        for (; begin != end; ++ begin)
        {
            _partials.push_back(_partials.back() + _computer(*begin));
        }
    }

    /**
    .. function:: void push_back<Args...>(Args&&... args)

        Add the value of the *args* to this array.
    */
    template <typename... Args>
    void push_back(Args&&... reference)
    {
        auto value = _computer(std::forward<Args>(reference)...);
        if (_partials.empty())
            _partials.push_back(value);
        else
            _partials.push_back(_partials.back() + value);
    }

    /**
    .. function:: value_type back() const

        Return the last value in this array (*not* the partial sum). If the
        array is empty, 0 will be returned.
    */
    value_type back() const
    {
        size_t n = _partials.size();
        switch (n)
        {
            default: return _partials[n-1] - _partials[n-2];
            case 1: return _partials.front();
            case 0: return value_type();
        }
    }

    /**
    .. function:: value_type operator[](size_t index) const

        Return a value in this array (*not* the partial sum) given the index.
    */
    value_type operator[](size_t index) const
    {
        auto last = index == 0 ? value_type() : _partials[index-1];
        return _partials[index] - last;
    }

    /**
    .. function:: size_t size() const

        Return the number of items in the array.
    */
    size_t size() const { return _partials.size(); }

    /**
    .. function:: void invalidate(size_t count)

        Remove the last *count* values from the array.
    */
    void invalidate(size_t count) { _partials.resize(_partials.size() - count); }

    /**
    .. function:: void invalidate_all()

        Clear the array. It will become empty afterward.
    */
    void invalidate_all() { _partials.clear(); }

    /**
    .. function:: value_type total() const

        Returns the sum of values in the array. It is an O(1) operation. If the
        array is empty, 0 is returned.
    */
    value_type total() const { return _partials.empty() ? value_type() : _partials.back(); }

    /**
    .. function:: std::pair<RandomAccessIterator, value_type> find<RandomAccessIterator>(RandomAccessIterator begin, RandomAccessIterator end, value_type partial)

        Find an object such that the partial sum up to that point is strictly
        less than *partial*. The result is a pair, the first value being the
        iterator of the object found, and the second being the value (*not* the
        partial sum) associated with that object.

        As an illustration, if the array contains the values {1, 4, 2, 9}, i.e.
        the partial sums are {1, 5, 7, 16}, then finding for "6" will yield the
        second object (i.e. 4).

        This operation takes logarithmic time (O(log(end - begin))).
    */
    template <typename RandomAccessIterator>
    std::pair<RandomAccessIterator, value_type> find(RandomAccessIterator start,
                                                     RandomAccessIterator end,
                                                     value_type partial)
    {
        this->update(start, end);
        auto partials_begin = _partials.cbegin();
        auto partials_end = _partials.cend();
        auto partials_it = std::upper_bound(partials_begin, partials_end, partial);
        RandomAccessIterator it = start + (partials_it - partials_begin);
        if (partials_it == partials_begin)
            return std::make_pair(it, partial);
        else
            return std::make_pair(it, partial - *--partials_it);
    }

private:
    ComputerType _computer;
    std::vector<value_type> _partials;
};

/**
.. function:: utils::partials_array<ComputerType> utils::make_partials_array<ComputerType>(const ComputerType& computer_type)

    This is a convenient function to construct a partials array give its computer.
*/
template <typename ComputerType>
partials_array<ComputerType> make_partials_array(const ComputerType& computer_type)
{
    return partials_array<ComputerType>(computer_type);
}

}

#endif

