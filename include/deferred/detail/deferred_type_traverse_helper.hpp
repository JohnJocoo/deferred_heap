#pragma once

#include <type_traits>

#include "visitor.hpp"
#include "is_container.hpp"
#include "is_deferred_ptr.hpp"
#include "support_visitor.hpp"
#include "scan_for_parents.hpp"

namespace def::detail
{

template <typename T>
struct is_container_of_deferred_ptr
{
private:
    struct dummy { typedef void value_type; };
    using t_type = std::conditional_t<
            is_container<T>::value, T, dummy>;
    using container_type = typename t_type::value_type;

public:
    static constexpr bool value = is_deferred_ptr<container_type>::value;

}; // is_container_of_deferred_ptr<T>

template <typename T, typename Tag>
struct object_has_tag
{
private:
    using type = T;
    using tag = Tag;

    typedef char                      one;
    typedef struct { char array[2]; } two;

    template <typename C, C>
    struct helper;
    template <typename C>
    static one check(helper<void (*)(tag),
                            &C::__deferred_detail_declare_have_tag>*);
    template<typename C>
    static two check(...);

public:
    static constexpr bool value = sizeof(check<type>(0)) == sizeof(one);

}; // object_has_tag<T, Tag>

template <typename T, typename Tag>
struct member_traverse_helper
{
    using object_type = T;
    using tag = Tag;
    using member_info = decltype(T::__deferred_detail_get_member_info(tag{}));
    using member_type = std::decay_t<typename member_info::member_type>;

    static void apply_visitor_member(visitor& v, object_type& object)
    {
        constexpr auto member_pointer = member_info::member_pointer;
        apply_to_member<member_type>(v, object.*member_pointer);
    }

private:
    template <typename Ptr,
              std::enable_if_t<is_deferred_ptr<Ptr>::value, int> = 0>
    static void apply_to_member(visitor& v, Ptr& ptr)
    {
        v.visit(ptr);
    }

    template <typename C,
              std::enable_if_t<is_container_of_deferred_ptr<C>::value, int> = 0>
    static void apply_to_member(visitor& v, C& container)
    {
        for (auto& ptr: container)
        {
            v.visit(ptr);
        }
    }

}; // member_traverse_helper<T, Tag>

template <bool>
struct apply_visitor_object_impl
{
    template <typename T>
    static void call_visit(visitor& v, T& object)
    {
        object.visit(v);
    }

}; // apply_visitor_object_impl<true>

template <>
struct apply_visitor_object_impl<false>
{
    template <typename T>
    static void call_visit(visitor&, T&)
    { }

}; // apply_visitor_object_impl<false>

template <typename T>
struct visitable_object_helper
{
    using object_type = T;

    static void apply_visitor_object(visitor& v, object_type& object)
    {
        constexpr bool can_visit = support_visitor<object_type>::value;
        apply_visitor_object_impl<can_visit>::call_visit(v, object);
    }

}; // visitable_object_helper<T>

template <typename... Ts>
struct tags_list
{ }; // tags_list<Ts...>

template <bool has_tag, typename T, unsigned int I, typename... Ts>
struct object_tags_impl_conditional
{
    using type = tags_list<Ts...>;

}; // object_tags_impl_conditional<false, T, I, Ts...>

template <typename T, unsigned int I, typename... Ts>
struct object_tags_impl
{
    using tag_type = std::integral_constant<unsigned int, I>;
    static constexpr bool has_tag = object_has_tag<T, tag_type>::value;

    using type = typename object_tags_impl_conditional<
                              has_tag, T, I, Ts...>::type;

}; // object_tags_impl<T, I, Ts...>

template <typename T, unsigned int I, typename... Ts>
struct object_tags_impl_conditional<true, T, I, Ts...>
{
    using tag_type = std::integral_constant<unsigned int, I>;
    using type = typename object_tags_impl<T, I + 1, Ts..., tag_type >::type;

}; // object_tags_impl_conditional<true, T, I, Ts...>

template <typename T>
struct object_tags
{
    using type = typename object_tags_impl<T, 0u>::type;

}; // object_tags<T>

template <typename T>
struct object_traverse_helper
{
    using object_type = T;

    static void apply_visitor_to_all(visitor& v, object_type& object)
    {
        apply_visitor_parents(v, object);
        apply_visitor_members(v, object);
        call_visit(v, object);
    }

private:
    template <typename P>
    static void apply_visitor_parent(visitor& v, object_type& object)
    {
        static_assert(!std::is_same_v<T, P>, "logic error - parent class P can "
                                             "not be same type as "
                                             "child class T");
        static_assert(std::is_base_of_v<P, T>, "logic error - class P is not"
                                               " a base of T class");
        object_traverse_helper<P>::apply_visitor_to_all(v, object);
    }

    static void apply_visitor_parents(visitor& v, object_type& object)
    {
        using parents = typename parent_classes<object_type>::type;
        apply_visitor_parents_impl(v, object, parents{});
    }

    template <typename... Ts>
    static void apply_visitor_parents_impl(visitor& v, object_type& object,
                                           types_list<Ts...>)
    {
        (... , apply_visitor_parent<Ts>(v, object));
    }

    template <typename Tag>
    static void apply_visitor_member(visitor& v, object_type& object)
    {
        member_traverse_helper<object_type, Tag>
                ::apply_visitor_member(v, object);
    }

    static void apply_visitor_members(visitor& v, object_type& object)
    {
        using tags = typename object_tags<object_type>::type;
        apply_visitor_members_impl(v, object, tags{});
    }

    template <typename... Ts>
    static void apply_visitor_members_impl(visitor& v, object_type& object,
                                           tags_list<Ts...>)
    {
        (... , apply_visitor_member<Ts>(v, object));
    }

    static void call_visit(visitor& v, object_type& object)
    {
        visitable_object_helper<object_type>::apply_visitor_object(v, object);
    }

}; // object_traverse_helper<T>

} // namespace def::detail
