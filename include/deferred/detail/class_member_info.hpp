#pragma once

#include "identity.hpp"

namespace def::detail
{

template <typename C, typename T, T C::*P>
struct class_member_info
{
    using owner_type            = C;
    using member_type           = T;
    using member_pointer_type   = member_type owner_type::*;

    static constexpr member_pointer_type member_pointer = P;

}; // struct class_member_info<C, T, T C::*P>

template <typename C, typename T>
identity<C> get_class_type(T C::*)
{
    return {};
}

template <typename C, typename T>
identity<T> get_member_type(T C::*)
{
    return {};
}

} // namespace def::detail
