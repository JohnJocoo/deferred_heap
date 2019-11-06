#pragma once

#include <vector>
#include <cassert>

#include "deferred_ptr.hpp"

namespace def
{

namespace detail
{

struct memory_chunk_header;

template <bool is_class, typename T>
struct has_visit_method_impl;

template <bool>
struct apply_visitor_object_impl;

template <typename T, typename Allocator>
class type_helper_impl;

} // namespace detail

template <typename T>
struct support_visitor;

class visitor
{
public:
    template <typename T>
    void visit(deferred_ptr<T>& ptr)
    {
        if (ptr == nullptr)
            return;
        auto* header = ptr.get_header();
        assert(header != nullptr);
        if (is_visited(header))
            return;
        m_not_visited.push_back(header);
    }

private:
    explicit visitor(std::vector<detail::memory_chunk_header*>& not_visited)
    : m_not_visited{not_visited}
    {}

    static bool is_visited(detail::memory_chunk_header*);

private:
    std::vector<detail::memory_chunk_header*>& m_not_visited;

private:
    // def::detail::has_visit_method functionality port
    // allow user class to use only 'friend class def::visitor;'

    struct disabled_t {};
    struct fallback { disabled_t visit(visitor&); };

    template<typename C, C> struct is_member;

    typedef char                      one;
    typedef struct { char array[2]; } two;

    template<typename C>
    static one (&f(is_member<disabled_t (fallback::*)(visitor&), &C::visit>*));
    template<typename C>
    static two (&f(...));

private:
    // def::detail::has_visit_method functionality port
    template <typename T>
    static constexpr bool has_visit_method()
    {
        struct derived : T, fallback { };
        return sizeof(f<derived>(0)) == sizeof(two);
    }

    // port of some utilities for def::support_visitor
    template <typename T,
            typename V = decltype(((T*)nullptr)->visit(
                    *(::def::visitor*)nullptr))>
    static constexpr std::true_type
    check_visit_method_only_argument_visitor(T*)
    {
        return {};
    }

    template <typename T>
    static constexpr std::false_type
    check_visit_method_only_argument_visitor(const T*)
    {
        return {};
    }

    // helper for calling T::visit.
    // T::visit might be private.
    // Then T declares def::visitor as friend.
    template <typename T>
    void call_visit(T& object)
    {
        object.visit(*this);
    }

    template <bool is_class, typename T>
    friend struct detail::has_visit_method_impl;

    template <typename T>
    friend struct support_visitor;

    friend struct detail::apply_visitor_object_impl<true>;

    template <typename T, typename Allocator>
    friend class detail::type_helper_impl;

}; // class visitor

} // namespace def
