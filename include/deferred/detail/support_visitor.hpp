#pragma once

#include <type_traits>

#include "identity.hpp"
#include "visitor.hpp"

namespace def::detail
{

template <typename T>
struct has_visit_method
{
    static constexpr bool value = visitor::has_visit_method<T>();

}; // struct has_visit_method<T>

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
            decltype(visitor::check_visit_method_only_argument_visitor(
                        (t_type*)nullptr))::value;

public:
    static constexpr bool value = has_visit && is_argument_visitor;

}; // struct support_visitor<T>

} // namespace def
