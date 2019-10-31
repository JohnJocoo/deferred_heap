#pragma once

#include <string>
#include <type_traits>

namespace def::detail
{

template<typename T>
struct has_const_iterator
{
private:
    typedef char                      one;
    typedef struct { char array[2]; } two;

    template<typename C>
    static one test(typename C::const_iterator*);
    template<typename C>
    static two test(...);

public:
    static constexpr bool value = sizeof(test<T>(0)) == sizeof(one);

}; // struct has_const_iterator<T>

template <typename T>
struct has_begin_end
{
private:
    struct dummy { typedef void const_iterator; };
    typedef typename std::conditional<has_const_iterator<T>::value,
                                      T, dummy>::type t_type;
    typedef typename t_type::const_iterator iter;

    struct fallback { iter begin() const; iter end() const; };
    struct derived : t_type, fallback { };

    template<typename C, C> struct is_member;

    typedef char                      one;
    typedef struct { char array[2]; } two;

    template<typename C>
    static one (&f(is_member<iter (fallback::*)() const, &C::begin>*));
    template<typename C>
    static two (&f(...));
    template<typename C>
    static one (&g(is_member<iter (fallback::*)() const, &C::end>*));
    template<typename C>
    static two (&g(...));

public:
    static constexpr bool beg_value = sizeof(f<derived>(0)) == sizeof(two);
    static constexpr bool end_value = sizeof(g<derived>(0)) == sizeof(two);

}; // struct has_begin_end<T>

template <typename T>
struct is_container
{
    static constexpr bool value = has_const_iterator<T>::value &&
                                  has_begin_end<T>::beg_value &&
                                  has_begin_end<T>::end_value;

}; // struct is_container<T>

template<typename _CharT, typename _Traits, typename _Alloc>
struct is_container<std::basic_string<_CharT, _Traits, _Alloc>>
{
    static constexpr bool value = false;

}; // struct is_container<std::basic_string>

template<typename T, std::size_t N>
struct is_container<T[N]>
{
    static constexpr bool value = true;

}; // struct is_container<T[N]>

} // namespace def::detail
