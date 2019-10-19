#pragma once

#include <type_traits>

#include "identity.hpp"
#include "class_member_info.hpp"
#include "static_class_counter.hpp"
#include "scan_for_parents.hpp"

#define DEF_ENABLE_DEFERRED_REFLECTION(__class) \
public: \
using __deferred_detail_parent_classes = \
::def::detail::scan_for_parents<__class>::type; \
using __deferred_detail_owner_class = __class

#define DEF_REGISTER_DEFERRED_MEMBER(__name) \
static constexpr \
::def::detail::class_member_info<\
        decltype(::def::detail::get_class_type(\
                &__deferred_detail_owner_class::__name))::type, \
        decltype(::def::detail::get_member_type(\
                &__deferred_detail_owner_class::__name))::type, \
        &__deferred_detail_owner_class::__name> \
        __deferred_detail_get_member_info(\
        std::integral_constant<unsigned int, \
                __DEF_DETAIL_CURRENT_COUNTER_VALUE(\
                        __deferred_detail_owner_class)>) \
{return {};} \
__DEF_DETAIL_INCREMENT_COUNTER_VALUE(__deferred_detail_owner_class) \
using __deferred_detail_require_semicolon_##__name = decltype(0)
