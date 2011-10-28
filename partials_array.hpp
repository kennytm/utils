//------------------------------------------------------------------
// partials_array: Sequential container that caches the partial sum
//------------------------------------------------------------------
//
//          Copyright kennytm (auraHT Ltd.) 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file doc/LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef PARTIALS_ARRAY_HPP_0YQ35OAC41IZ4CXR
#define PARTIALS_ARRAY_HPP_0YQ35OAC41IZ4CXR 1

#include <vector>
#include <algorithm>
#include <type_traits>
#include "traits.hpp"

namespace utils {

template <typename ComputerType>
class partials_array
{
public:
    typedef ComputerType computer_type;
    typedef typename function_traits<ComputerType>::result_type value_type;

    partials_array() = default;
    explicit partials_array(const ComputerType& computer) : _computer(computer) {};

    template <typename RandomAccessIterator>
    void update(RandomAccessIterator begin, RandomAccessIterator end)
    {
        size_t partials_size = _partials.size();
        size_t collection_size = end - begin;
        if (collection_size <= partials_size)
            return;

        _partials.reserve(collection_size);

        auto cur = begin + partials_size;
        if (partials_size == 0)
        {
            _partials.push_back(_computer(*cur));
            ++ cur;
        }

        for (; cur != end; ++ cur)
        {
            _partials.push_back(_partials.back() + _computer(*cur));
        }
    }

    template <typename... Args>
    void push_back(Args&&... reference)
    {
        auto value = _computer(std::forward<Args>(reference)...);
        if (_partials.empty())
            _partials.push_back(value);
        else
            _partials.push_back(_partials.back() + value);
    }

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

    value_type operator[](size_t index) const
    {
        auto last = index == 0 ? value_type() : _partials[index-1];
        return _partials[index] - last;
    }

    size_t size() const { return _partials.size(); }

    void invalidate(size_t count) { _partials.resize(_partials.size() - count); }
    void invalidate_all() { _partials.clear(); }

    value_type total() const { return _partials.empty() ? value_type() : _partials.back(); }

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

template <typename ComputerType>
partials_array<ComputerType> make_partials_array(const ComputerType& computer_type)
{
    return partials_array<ComputerType>(computer_type);
}

}

#endif

