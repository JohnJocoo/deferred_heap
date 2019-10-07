#pragma once

#include "deferred_ptr.hpp"
#include "root_ptr.hpp"

namespace def::detail
{

template <typename T>
struct is_deferred_ptr
{
    static constexpr bool value = false;

}; // struct is_deferred_ptr<T>

template <typename T>
struct is_deferred_ptr<::def::deferred_ptr<T>>
{
    static constexpr bool value = true;

}; // struct is_deferred_ptr<deferred_ptr>

template <typename T>
struct is_deferred_ptr<::def::root_ptr<T>>
{
    static constexpr bool value = true;

}; // struct is_deferred_ptr<root_ptr>

} // namespace def::detail

