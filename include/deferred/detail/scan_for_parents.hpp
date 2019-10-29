#pragma once

#include <type_traits>

#include "identity.hpp"

namespace def::detail
{

template <typename... Ts>
struct types_list
{ }; // types_list<Ts...>

template <typename T1, typename T2>
struct unite_type_lists
{
private:
    template <typename T, typename... Ts>
    static constexpr auto add_one(types_list<Ts...>)
    {
        constexpr bool has_t = (... || std::is_same_v<T, Ts>);
        if constexpr (has_t)
            return types_list<Ts...>{};
        else
            return types_list<Ts..., T>{};
    }

    template <typename T>
    static constexpr T unite(types_list<>)
    {
        return {};
    }

    template <typename T, typename T3, typename... Ts>
    static constexpr auto unite(types_list<T3, Ts...>)
    {
        using new_types = decltype(add_one<T3>(T{}));
        return unite<new_types>(types_list<Ts...>{});
    }

public:
    using type = decltype(unite<T1>(T2{}));

}; // unite_type_lists<types_list<L1...>, types_list<L2...>>

template <typename T>
struct scan_for_parents
{
private:
    using _type = T;

    template <typename C, typename P =
                            typename C::__deferred_detail_owner_class>
    static identity<types_list<P>>    f(C*);

    static identity<types_list<>> f(...);

public:
    using type = typename decltype(f((_type*)nullptr))::type;

}; // scan_for_parent<T>

template <typename T>
struct has_parent_classes
{
private:
    using type = T;

    typedef char                      one;
    typedef struct { char array[2]; } two;

    template <typename C, typename P =
                            typename C::__deferred_detail_auto_parent_classes>
    static one check(C*);

    static two check(...);

public:
    static constexpr bool value = sizeof(check((type*)nullptr)) == sizeof(one);

}; // has_parent_classes<T>

template <typename T>
struct has_user_parent_classes
{
private:
    using type = T;

    typedef char                      one;
    typedef struct { char array[2]; } two;

    template <typename C, typename P =
                            typename C::__deferred_detail_user_parent_classes>
    static one check(C*);

    static two check(...);

public:
    static constexpr bool value = sizeof(check((type*)nullptr)) == sizeof(one);

}; // has_user_parent_classes<T>

template <bool has_parent_classes, typename T>
struct parent_classes_impl
{
    using type = typename T::__deferred_detail_auto_parent_classes;

}; // parent_classes_impl<true, T>

template <typename T>
struct parent_classes_impl<false, T>
{
    using type = types_list<>;

}; // parent_classes_impl<false, T>

template <bool has_parent_classes, typename T>
struct parent_user_classes_impl
{
    using type = typename T::__deferred_detail_user_parent_classes;

}; // parent_user_classes_impl<true, T>

template <typename T>
struct parent_user_classes_impl<false, T>
{
    using type = types_list<>;

}; // parent_user_classes_impl<false, T>

template <typename T>
struct parent_classes
{
private:
    using auto_types = typename parent_classes_impl<
            has_parent_classes<T>::value, T>::type;
    using user_types = typename parent_user_classes_impl<
            has_user_parent_classes<T>::value, T>::type;

public:
    using type = typename unite_type_lists<auto_types, user_types>::type;

}; // parent_classes<T>

} // namespace def::detail
