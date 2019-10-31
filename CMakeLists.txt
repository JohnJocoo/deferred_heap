cmake_minimum_required(VERSION 3.7)

project(DeferredHeap LANGUAGES CXX VERSION 0.1.0)

option(DEFERRED_HEAP_BUILD_TEST "Build tests for DeferredHeap" OFF)

set(CMAKE_CXX_STANDARD 17)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/deferred")
set(LIB_HEADERS
        "${INCLUDE_DIR}/deferred_heap"
        "${INCLUDE_DIR}/deferred_ptr"
        "${INCLUDE_DIR}/visitor"
        "${INCLUDE_DIR}/root_ptr"
        "${INCLUDE_DIR}/defines"
        "${INCLUDE_DIR}/simple_allocator"
        "${INCLUDE_DIR}/is_deferred_base_of"
        "${INCLUDE_DIR}/detail/deferred_heap.hpp"
        "${INCLUDE_DIR}/detail/deferred_heap_impl.hpp"
        "${INCLUDE_DIR}/detail/deferred_ptr.hpp"
        "${INCLUDE_DIR}/detail/deferred_simple_allocator.hpp"
        "${INCLUDE_DIR}/detail/deferred_type_defines.hpp"
        "${INCLUDE_DIR}/detail/deferred_type_helper.hpp"
        "${INCLUDE_DIR}/detail/deferred_type_helper_impl.hpp"
        "${INCLUDE_DIR}/detail/deferred_type_traverse_helper.hpp"
        "${INCLUDE_DIR}/detail/visitor.hpp"
        "${INCLUDE_DIR}/detail/memory_chunk_header.hpp"
        "${INCLUDE_DIR}/detail/root_ptr.hpp"
        "${INCLUDE_DIR}/detail/root_ptr_base.hpp"
        "${INCLUDE_DIR}/detail/class_member_info.hpp"
        "${INCLUDE_DIR}/detail/identity.hpp"
        "${INCLUDE_DIR}/detail/is_container.hpp"
        "${INCLUDE_DIR}/detail/is_deferred_ptr.hpp"
        "${INCLUDE_DIR}/detail/support_visitor.hpp"
        "${INCLUDE_DIR}/detail/static_class_counter.hpp"
        "${INCLUDE_DIR}/detail/scan_for_parents.hpp"
        "${INCLUDE_DIR}/detail/is_deferred_base_of.hpp")

set(IMPL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LIB_SOURCES
        "${IMPL_DIR}/deferred_heap.cpp"
        "${IMPL_DIR}/memory_chunk_header.cpp"
        "${IMPL_DIR}/root_ptr_base.cpp"
        "${IMPL_DIR}/deferred_simple_allocator.cpp"
        "${IMPL_DIR}/deferred_type_helper.cpp"
        "${IMPL_DIR}/visitor.cpp")

add_library(DeferredHeap 
            ${LIB_HEADERS} ${LIB_SOURCES})
target_include_directories(DeferredHeap PUBLIC
                           "${CMAKE_CURRENT_SOURCE_DIR}/include")

if (DEFERRED_HEAP_BUILD_TEST)
    add_subdirectory(test)
endif(DEFERRED_HEAP_BUILD_TEST)
unset(DEFERRED_HEAP_BUILD_TEST CACHE)
