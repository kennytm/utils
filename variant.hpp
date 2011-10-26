#define DEBUG_GET_INDEX

#ifndef VARIANT_HPP_XMPYUK9CBNN
#define VARIANT_HPP_XMPYUK9CBNN

#include <utility>
#include <type_traits>
#include <iosfwd>
#if !defined(BOOST_NO_TYPEID)
#include <typeinfo>
#endif

namespace utils {

    template <typename RT = void>
    class static_visitor
    {
    public:
        typedef RT result_type;
    };

    template <typename... T>
    class variant;

    template <typename T>
    struct is_variant { enum { value = false }; };
    template <typename... T>
    struct is_variant<variant<T...>> { enum { value = true }; };
}

#include "variant-impl.hpp"

namespace utils {

    template <typename... T>
    class variant
    {
        typedef xx_impl::union_impl<T...> union_type;

        size_t _index;
        union_type _storage;

        class init_to_visitor : public static_visitor<void>
        {
        public:
            variant<T...>* this_;

            template <typename U>
            void operator()(U&& other)
            {
                this_->init(std::forward<U>(other));
            }
        };

        template <typename U>
        void init(U&& other)
        {
            typedef xx_impl::get_index<U, xx_impl::is_constructible, T...> index_tmpl;
            static_assert(index_tmpl::found, "Constructing variant from unexpected type.");
            static_assert(!index_tmpl::ambiguous, "Constructing variant with ambiguous conversion.");
            static const size_t index_of_U = index_tmpl::index;
            _index = index_of_U;

            xx_impl::init_visitor_1<U> ctor (std::forward<U>(other));
            xx_impl::static_applier<index_of_U>()(_storage, ctor);
        }

    public:
        variant() : _index(0)
        {
            new(&_storage.head) decltype(_storage.head);
        }

        ~variant()
        {
            xx_impl::destroy_visitor dtor;
            apply(_storage, _index, dtor);
        }

        template <typename U>
        variant(U&& other)
        {
            this->init(std::forward<U>(other));
        }

        template <typename... TOther>
        variant(variant<TOther...>& other)
        {
            init_to_visitor ctor;
            ctor.this_ = this;
            apply(other._storage, other._index, ctor);
        }

        template <typename... TOther>
        variant(variant<TOther...>&& other)
        {
            init_to_visitor ctor;
            ctor.this_ = this;
            apply(std::move(other._storage), other._index, ctor);
        }

        variant(variant<T...>& other) : _index(other._index)
        {
            xx_impl::init_visitor_2 ctor;
            apply(_storage, other._storage, _index, ctor);
        }

        variant(variant<T...>&& other) : _index(other._index)
        {
            xx_impl::init_visitor_2 ctor;
            apply(_storage, std::move(other._storage), _index, ctor);
        }

    private:
        template <typename CA>
        void perform_safe_copy(bool nothrow_movable, CA copy_action)
        {
            xx_impl::destroy_visitor dtor;

            if (nothrow_movable)
            {
                apply(_storage, _index, dtor);
                copy_action();
            }
            else
            {
                struct backup_owner
                {
                    size_t index;
                    union_type* storage;

                    constexpr bool can_restore() const noexcept { return index < sizeof...(T); }

                    backup_owner() : index(sizeof...(T)), storage(new union_type) {}
                    ~backup_owner()
                    {
                        if (storage != nullptr)
                        {
                            if (this->can_restore())
                            {
                                xx_impl::destroy_visitor dtor;
                                apply(*storage, index, dtor);
                            }
                            delete storage;
                        }
                    }
                } backup;

                xx_impl::init_visitor_2 ctor2;
                xx_impl::is_nothrow_movable_checker nothrow_movable_checker;

                try
                {
                    if (xx_impl::apply(_storage, _index, nothrow_movable_checker))
                        xx_impl::apply(*backup.storage, std::move(_storage), _index, ctor2);
                    else
                        xx_impl::apply(*backup.storage, _storage, _index, ctor2);
                    backup.index = _index;

                    xx_impl::apply(_storage, _index, dtor);
                    copy_action();
                }
                catch (...)
                {
                    if (backup.can_restore())
                    {
                        if (xx_impl::apply(_storage, backup.index, nothrow_movable_checker))
                            xx_impl::apply(_storage, std::move(*backup.storage), backup.index, ctor2);
                        else
                            xx_impl::apply(_storage, *backup.storage, backup.index, ctor2);
                    }

                    throw;
                }
            }
        }

    public:
        template <typename U>
        variant<T...>& operator=(U&& other)
        {
            typedef xx_impl::get_index<U, xx_impl::is_assignable, T...> index_tmpl;
            static_assert(index_tmpl::found, "Assigning to variant from unexpected type.");
            static_assert(!index_tmpl::ambiguous, "Assigning to variant with ambiguous conversion.");
            static const size_t index_of_U = index_tmpl::index;

            if (_index == index_of_U)
            {
                xx_impl::assign_visitor_1<U> copier (std::forward<U>(other));
                xx_impl::static_applier<index_of_U>()(_storage, copier);
            }
            else
            {
                static const bool is_nothrow_copyable = std::is_lvalue_reference<U>::value
                                                     && xx_impl::is_nothrow_copy_constructible<U>();
                static const bool is_nothrow_movable = std::is_rvalue_reference<U>::value
                                                    && xx_impl::is_nothrow_move_constructible<U>();

                xx_impl::init_visitor_1<U> ctor (std::forward<U>(other));
                xx_impl::static_applier<index_of_U> static_applier;
                this->perform_safe_copy(is_nothrow_copyable || is_nothrow_movable,
                    [&, this] { static_applier(_storage, ctor); }
                );
                _index = index_of_U;
            }
            return *this;
        }

        template <typename... TOther>
        typename std::enable_if<!xx_impl::get_index<variant<TOther...>,
                                                    xx_impl::is_same,
                                                    T...>::is_exact,
                                 variant<T...>>::type&
            operator=(variant<TOther...>& other)
        {
            xx_impl::is_same_visitor is_same;
            xx_impl::is_one_of_visitor<T...> has_same;

            auto same_type_pair = xx_impl::apply2(_storage, _index, other._storage, other._index, is_same);
            bool has_same_type = xx_impl::apply(other._storage, other._index, has_same);

            if (same_type_pair.first || (!has_same_type && same_type_pair.second))
            {
                xx_impl::assign_visitor_2 assigner;
                xx_impl::apply2(_storage, _index, other._storage, other._index, assigner);
            }
            else
            {
                xx_impl::assign_to_visitor<variant<T...>> assigner (*this);
                xx_impl::apply(other._storage, other._index, assigner);
            }
            return *this;
        }

        template <typename... TOther>
        variant<T...>& operator=(variant<TOther...>&& other)
        {
            xx_impl::is_same_visitor is_same;
            xx_impl::is_one_of_visitor<T...> has_same;

            auto same_type_pair = xx_impl::apply2(_storage, _index, other._storage, other._storage, is_same);
            bool has_same_type = xx_impl::apply(other._storage, other._index, has_same);

            if (same_type_pair.first || (!has_same_type && same_type_pair.second))
            {
                xx_impl::assign_visitor_2 assigner;
                xx_impl::apply2(_storage, _index, std::move(other._storage), other._index, assigner);
            }
            else
            {
                xx_impl::assign_to_visitor<variant<T...>> assigner (*this);
                xx_impl::apply(std::move(other._storage), other._index, assigner);
            }
            return *this;
        }

        variant<T...>& operator=(variant<T...>& other)
        {
            if (_index == other._index)
            {
                xx_impl::assign_visitor_2 assigner;
                xx_impl::apply(_storage, other._storage, _index, assigner);
            }
            else
            {
                xx_impl::is_nothrow_copyable_checker cc;
                bool is_nothrow_copyable = xx_impl::apply(other._storage, other._index, cc);

                this->perform_safe_copy(is_nothrow_copyable,
                    [=, &other] {
                        xx_impl::init_visitor_2 ctor;
                        xx_impl::apply(_storage, other._storage, other._index, ctor);
                    }
                );
                _index = other._index;
            }
            return *this;
        }

        variant<T...>& operator=(variant<T...>&& other)
        {
            if (_index == other._index)
            {
                xx_impl::assign_visitor_2 assigner;
                apply(_storage, std::move(other._storage), _index, assigner);
            }
            else
            {
                xx_impl::is_nothrow_movable_checker mc;
                bool is_nothrow_movable = xx_impl::apply(other._storage, other._index, mc);

                this->perform_safe_copy(is_nothrow_movable,
                    [=, &other] {
                        xx_impl::init_visitor_2 ctor;
                        xx_impl::apply(_storage, std::move(other._storage), other._index, ctor);
                    }
                );
                _index = other._index;
            }
            return *this;
        }

        void swap(variant<T...>& other)
        {
            if (_index == other._index)
            {
                xx_impl::swap_visitor_2 swapper;
                xx_impl::apply(_storage, other._storage, _index, swapper);
            }
            else
            {
                variant<T...> tmp = std::move(other);
                other = std::move(*this);
                *this = std::move(tmp);
            }
        }

        template <typename U>
        bool operator==(const U& other)
        {
            typedef xx_impl::get_index<U, xx_impl::is_equatable, T...> index_tmpl;
            static_assert(index_tmpl::found, "Equating variant to unexpected type.");
            static_assert(!index_tmpl::ambiguous, "Equating variant with ambiguous conversion.");
            static const size_t index_of_U = index_tmpl::index;

            if (index_of_U != _index)
                return false;

            xx_impl::equals_visitor_1<U> eq (other);
            return xx_impl::static_applier<index_of_U>()(_storage, eq);
        }

        template <typename U>
        bool operator<(const U& other)
        {
            typedef xx_impl::get_index<U, xx_impl::is_less_than_comparable, T...> index_tmpl;
            static_assert(index_tmpl::found, "Comparing variant with unexpected type.");
            static_assert(!index_tmpl::ambiguous, "Comparing variant with ambiguous conversion.");
            static const size_t index_of_U = index_tmpl::index;

            if (index_of_U != _index)
                return _index < index_of_U;

            xx_impl::less_than_visitor_1<U> lt (other);
            return xx_impl::static_applier<index_of_U>()(_storage, lt);
        }

        template <typename U>
        bool operator>(const U& other)
        {
            typedef xx_impl::get_index<U, xx_impl::is_greater_than_comparable, T...> index_tmpl;
            static_assert(index_tmpl::found, "Comparing variant with unexpected type.");
            static_assert(!index_tmpl::ambiguous, "Comparing variant with ambiguous conversion.");
            static const size_t index_of_U = index_tmpl::index;

            if (index_of_U != _index)
                return _index > index_of_U;

            xx_impl::greater_than_visitor_1<U> gt (other);
            return xx_impl::static_applier<index_of_U>()(_storage, gt);
        }

        bool operator==(const variant<T...>& other)
        {
            if (_index != other._index)
                return false;

            xx_impl::equals_visitor_2 eq;
            return xx_impl::apply(_storage, other._storage, _index, eq);
        }

        bool operator<(const variant<T...>& other)
        {
            if (_index != other._index)
                return _index < other._index;

            xx_impl::less_than_visitor_2 lt;
            return xx_impl::apply(_storage, other._storage, _index, lt);
        }

#if !defined(BOOST_NO_TYPEID)
        const std::type_info& type() const noexcept
        {
            xx_impl::typeid_visitor tv;
            return xx_impl::apply(_storage, _index, tv);
        }
#endif

        template <typename SV, typename V>
        friend typename SV::result_type apply_visitor(SV& visitor, V&& variant);

        template <typename SV, typename V1, typename V2>
        friend typename SV::result_type apply_visitor(SV& visitor, V1&& variant1, V2&& variant2);

        template <typename SV, typename V>
        friend typename SV::result_type apply_visitor(SV&& visitor, V&& variant);

        template <typename SV, typename V1, typename V2>
        friend typename SV::result_type apply_visitor(SV&& visitor, V1&& variant1, V2&& variant2);

        template <typename U, typename... TX>
        friend U* get(variant<TX...>* v) noexcept;

        template <typename U, typename... TX>
        friend const U* get(const variant<TX...>* v) noexcept;

        template <typename...>
        friend class variant;
    };

    template <typename SV, typename V>
    typename SV::result_type apply_visitor(SV& visitor, V&& variant)
    {
        return xx_impl::apply(variant._storage, variant._index, visitor);
    }

    template <typename SV, typename V1, typename V2>
    typename SV::result_type apply_visitor(SV& visitor, V1&& variant1, V2&& variant2)
    {
        return xx_impl::apply2(variant1._storage, variant1._index,
                               variant2._storage, variant2._index, visitor);
    }

    template <typename SV, typename V>
    typename SV::result_type apply_visitor(SV&& visitor, V&& variant)
    {
        return xx_impl::apply(variant._storage, variant._index, visitor);
    }

    template <typename SV, typename V1, typename V2>
    typename SV::result_type apply_visitor(SV&& visitor, V1&& variant1, V2&& variant2)
    {
        return xx_impl::apply2(variant1._storage, variant1._index,
                               variant2._storage, variant2._index, visitor);
    }

    template <typename SV>
    xx_impl::delayed_visitor<SV> apply_visitor(SV& visitor)
    {
        return xx_impl::delayed_visitor<SV>(visitor);
    }

    template <typename U, typename... T>
    U* get(variant<T...>* v) noexcept
    {
        typedef xx_impl::get_index<U, xx_impl::is_same, T...> index_impl;
        static_assert(index_impl::is_exact, "Getting from unexpected type.");
        if (index_impl::index == v->_index)
        {
            xx_impl::getter_visitor<U> getter;
            return xx_impl::static_applier<index_impl::index>()(v->_storage, getter);
        }
        else
            return nullptr;
    }

    template <typename U, typename... T>
    const U* get(const variant<T...>* v) noexcept
    {
        typedef xx_impl::get_index<U, xx_impl::is_same, T...> index_impl;
        static_assert(index_impl::is_exact, "Getting from unexpected type.");
        if (index_impl::index == v->_index)
        {
            xx_impl::getter_visitor<const U> getter;
            return xx_impl::static_applier<index_impl::index>()(v->_storage, getter);
        }
        else
            return nullptr;
    }

    class bad_get : public std::exception {
    public:
        virtual const char* what() const noexcept { return "bad_get"; }
    };

    template <typename U, typename... T>
    U& get(variant<T...>& v)
    {
        auto res = get<U>(&v);
        if (res == nullptr)
            throw bad_get();
        return *res;
    }

    template <typename U, typename... T>
    const U& get(const variant<T...>& v)
    {
        auto res = get<U>(&v);
        if (res == nullptr)
            throw bad_get();
        return *res;
    }

    template <typename U, typename... T>
    bool operator==(const U& a, const variant<T...>& v) { return v == a; }
    template <typename U, typename... T>
    bool operator<(const U& a, const variant<T...>& v) { return v > a; }
    template <typename U, typename... T>
    bool operator>(const U& a, const variant<T...>& v) { return v < a; }

    template <typename... T>
    std::ostream& operator<<(std::ostream& stream, const variant<T...>& v)
    {
        xx_impl::ostream_visitor visitor (stream);
        return apply_visitor(visitor, v);
    }
}

namespace std
{
    template <typename... T>
    void swap(utils::variant<T...>& a, utils::variant<T...>& b)
    {
        a.swap(b);
    }
}

#endif

