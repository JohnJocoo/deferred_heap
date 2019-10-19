#pragma once

#include <type_traits>

#define __DEF_DETAIL_CURRENT_COUNTER_VALUE(__class) \
::def::detail::current_class_counter_unique<__class, __COUNTER__>::value

#define __DEF_DETAIL_INCREMENT_COUNTER_VALUE(__class) \
static constexpr void \
__deferred_detail_declare_have_tag(\
        std::integral_constant<unsigned int, \
                __DEF_DETAIL_CURRENT_COUNTER_VALUE(__class)>) \
{}

namespace def::detail
{

template <typename T, typename Tag, int UID>
struct object_has_tag_unique
{
private:
    using type = T;
    using tag = Tag;

    typedef char                      one;
    typedef struct { char array[2]; } two;

    template<typename C, C>
    struct helper;

    template<typename C>
    static one check(helper<void (*)(tag),
                            &C::__deferred_detail_declare_have_tag> *);

    template<typename C>
    static two check(...);

public:
    static constexpr bool value = sizeof(check<type>(0)) == sizeof(one);

}; // object_has_tag_unique<T, Tag, UID>

template <bool has_tag, typename T, unsigned int N, int UID>
struct current_class_counter_unique_impl_conditional
{
    static constexpr unsigned int value = N;

}; // current_class_counter_unique_impl_conditional<false, T, N, UID>

template <typename T, unsigned int N, int UID>
struct current_class_counter_unique_impl
{
    using tag_type = std::integral_constant<unsigned int, N>;
    static constexpr bool has_tag =
            object_has_tag_unique<T, tag_type, UID>::value;

    static constexpr unsigned int value =
            current_class_counter_unique_impl_conditional<
                has_tag, T, N, UID>::value;

}; // current_class_counter_unique_impl<T, N, UID>

template <typename T, unsigned int N, int UID>
struct current_class_counter_unique_impl_conditional<true, T, N, UID>
{
    static constexpr unsigned int value =
            current_class_counter_unique_impl<T, N + 1, UID>::value;

}; // current_class_counter_unique_impl_conditional<true, T, N, UID>

template <typename T, int UID>
struct current_class_counter_unique
{
    static constexpr unsigned int value =
            current_class_counter_unique_impl<T, 0u, UID>::value;

}; // current_class_counter_unique<T, UID>

} // namespace def::detail
