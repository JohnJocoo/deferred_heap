#pragma once

#include <type_traits>

#include "scan_for_parents.hpp"

namespace def
{

template <typename base, typename derived>
struct is_deferred_base_of
{
private:
    using parents = typename detail::parent_classes<derived>::type;

    template <typename T>
    static constexpr bool is_base_type()
    {
        static_assert(!std::is_same_v<derived, T>, "logic error - parent class "
                                                   "is the same as derived");
        return std::is_same_v<base, T> ||
                is_deferred_base_of<base, T>::value;
    }

    template <typename... Ts>
    static constexpr bool is_base_types(detail::types_list<Ts...>)
    {
        return (... || is_base_type<Ts>());
    }

public:
    static constexpr bool value = is_base_types(parents{});

}; // is_deferred_base_of<base, derived>

} // namespace def
