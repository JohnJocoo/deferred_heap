#pragma once

#include "deferred_type_helper.hpp"

#include <vector>
#include <memory>

#include "memory_chunk_header.hpp"
#include "visitor.hpp"
#include "deferred_type_traverse_helper.hpp"

namespace def::detail
{

template <typename T, typename Allocator>
class type_helper_impl : private type_helper
{
public:
    using type      = T;
    using allocator = Allocator;

public:
    explicit type_helper_impl()
    : type_helper{typeid(type), sizeof(type), sizeof(allocator)}
    { }

private:
    void mark_recursive(memory_chunk_header& header) const override
    {
        auto ptr = reinterpret_cast<type*>(header.get_object_start());
        std::vector<memory_chunk_header*> non_visited_chunks;
        visitor v{non_visited_chunks};
        const auto num_objects = header.get_objects_number();
        for (memory_chunk_header::size_t i = 0; i != num_objects; ++i, ++ptr)
        {
            object_traverse_helper<T>::apply_visitor_to_all(v, *ptr);
        }
        for (auto* chunk: non_visited_chunks)
        {
            assert(chunk != nullptr);
            assert(!chunk->flags.is_visited());
            chunk->flags.mark_visited();
            assert(chunk->flags.is_visited());
        }
        for (auto* chunk: non_visited_chunks)
        {
            assert(chunk != nullptr);
            chunk->helper.mark_recursive(*chunk);
        }
    }

    void deallocate(memory_chunk_header* header) const override
    {
        if (header == nullptr)
            return;

        using bytes_allocator = typename std::allocator_traits<allocator>::
                template rebind_alloc<unsigned char>;
        using control_allocator = typename std::allocator_traits<allocator>::
                template rebind_alloc<memory_chunk_header>;
        using original_alloc_allocator =
                typename std::allocator_traits<allocator>::
                    template rebind_alloc<allocator>;
        const std::size_t allocation_size = header->get_bytes_allocated();
        auto* raw_ptr = reinterpret_cast<unsigned char*>(
                header->get_raw_memory_start());
        auto* alloc_ptr = reinterpret_cast<allocator*>(
                header->get_allocator_start());
        auto allocator_raw = bytes_allocator{*alloc_ptr};
        auto allocator_control = control_allocator{*alloc_ptr};
        auto allocator_original_alloc = original_alloc_allocator{*alloc_ptr};

        std::allocator_traits<original_alloc_allocator>::
                template destroy<allocator>(
                        allocator_original_alloc, alloc_ptr);
        std::allocator_traits<control_allocator>::
                template destroy<memory_chunk_header>(
                        allocator_control, header);
        std::allocator_traits<bytes_allocator>::deallocate(
                allocator_raw, raw_ptr, allocation_size);
    }

    void destroy_impl(memory_chunk_header& header) const override
    {
        const auto num_objects = header.get_objects_number();
        if (num_objects == 0)
            return;

        auto& original_alloc = (*reinterpret_cast<allocator*>(
                header.get_allocator_start()));

        auto* offset_ptr = reinterpret_cast<type*>(header.get_object_start());
        offset_ptr += (num_objects - 1);
        for (memory_chunk_header::size_t i = 0;
                i != num_objects; ++i, --offset_ptr)
        {
            std::allocator_traits<allocator>::
                    template destroy<type>(original_alloc, offset_ptr);
        }
    }

public:
    static const type_helper& instance()
    {
        static const type_helper_impl<T, Allocator> object;
        return object;
    }

}; // class type_helper

} // namespace def::detail
