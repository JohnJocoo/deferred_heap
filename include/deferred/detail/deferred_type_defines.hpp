#pragma once

#include <type_traits>

#include "identity.hpp"
#include "class_member_info.hpp"

#define DEF_ENABLE_DEFERRED_REFLECTION(__class) \
public: \
using __deferred_detail_owner_class = __class

#define DEF_REGISTER_DEFERRED_MEMBER(__name, __I_num) \
static void \
__deferred_detail_declare_have_tag(\
        std::integral_constant<unsigned int, \
                __I_num>) \
{} \
static \
::def::detail::class_member_info<\
        decltype(::def::detail::get_class_type(\
                &__deferred_detail_owner_class::__name))::type, \
        decltype(::def::detail::get_member_type(\
                &__deferred_detail_owner_class::__name))::type, \
        &__deferred_detail_owner_class::__name> \
        __deferred_detail_get_member_info(\
        std::integral_constant<unsigned int, \
                __I_num>) \
{return {};}
