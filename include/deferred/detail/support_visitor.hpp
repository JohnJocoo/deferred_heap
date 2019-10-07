#pragma once

#include <type_traits>

#include "identity.hpp"

namespace def::detail
{

template <typename T>
struct has_visit_method
{
private:
    using type = T;

    struct disabled_t {};
    struct fallback { disabled_t visit(visitor&); };
    struct derived : type, fallback { };

    template<typename C, C> struct is_member;

    typedef char                      one;
    typedef struct { char array[2]; } two;

    template<typename C>
    static one (&f(is_member<disabled_t (fallback::*)(visitor&), &C::visit>*));
    template<typename C>
    static two (&f(...));

public:
    static constexpr bool value = sizeof(f<derived>(0)) == sizeof(two);

}; // struct has_visit_method<T>

template <typename T,
          typename V = decltype(((T*)nullptr)->visit(
                                   *(::def::visitor*)nullptr))>
std::true_type check_visit_method_only_argument_visitor(T*)
{
    return {};
}

template <typename T>
std::false_type check_visit_method_only_argument_visitor(const T*)
{
    return {};
}

} // namespace def::detail

namespace def
{

template <typename T>
struct support_visitor
{
private:
    static constexpr bool has_visit = detail::has_visit_method<T>::value;

    struct dummy { void visit(int); };
    using t_type = std::conditional_t<has_visit, T, dummy>;


    static constexpr bool is_argument_visitor =
            decltype(detail::check_visit_method_only_argument_visitor(
                        (t_type*)nullptr))::value;

public:
    static constexpr bool value = has_visit && is_argument_visitor;

}; // struct support_visitor<T>

} // namespace def
