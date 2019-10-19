#pragma once

#include "identity.hpp"

namespace def::detail
{

template <typename... Ts>
struct types_list
{ }; // types_list<Ts...>

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
                            typename C::__deferred_detail_parent_classes>
    static one check(C*);

    static two check(...);

public:
    static constexpr bool value = sizeof(check((type*)nullptr)) == sizeof(one);

}; // has_parent_classes<T>

template <bool has_parent_classes, typename T>
struct parent_classes_impl
{
    using type = typename T::__deferred_detail_parent_classes;

}; // parent_classes_impl<true, T>

template <typename T>
struct parent_classes_impl<false, T>
{
    using type = types_list<>;

}; // parent_classes_impl<false, T>

template <typename T>
struct parent_classes
{
    using type = typename parent_classes_impl<has_parent_classes<T>::value,
                                              T>::type;

}; // parent_classes<T>

} // namespace def::detail
