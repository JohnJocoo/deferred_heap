#pragma once

#include <memory>
#include <cassert>

#include "deferred_ptr.hpp"
#include "root_ptr.hpp"
#include "memory_chunk_header.hpp"
#include "deferred_type_helper_impl.hpp"

namespace def::detail
{

class deferred_heap_impl;

void deferred_heap_impl_move_memory_to_deferred_heap(
        deferred_heap_impl&, memory_chunk_header*) noexcept(false);

template <typename T>
struct construct_helper
{
    template<typename Allocator, typename... Args>
    static
    std::pair<memory_chunk_header*, T*>
    construct(unsigned char* ptr, const std::size_t n_objects,
              const bool is_array, Allocator allocator, Args&&... args)
    {
        assert(ptr != nullptr);
        auto* offset_ptr = reinterpret_cast<T*>(
                ptr + sizeof(detail::memory_chunk_header) + sizeof(Allocator));
        auto* current_ptr = offset_ptr;
        std::size_t current_obj = 0;
        try
        {
            for (; current_obj != n_objects; ++current_obj, ++current_ptr)
            {
                std::allocator_traits<Allocator>::
                        template construct<T, Args...>(
                                allocator, current_ptr,
                                std::forward<Args>(args)...);
            }
            auto* control_ptr = construct_control(ptr, is_array, allocator);
            return {control_ptr, offset_ptr};
        }
        catch(...)
        {
            --current_ptr;
            for (std::size_t i = 0; i != current_obj; ++i, --current_ptr)
            {
                std::allocator_traits<Allocator>::
                        template destroy<T>(
                                allocator, current_ptr);
            }
            throw;
        }
    }

private:
    template<typename Allocator, typename... Args>
    static
    memory_chunk_header*
    construct_control(unsigned char* ptr, const bool is_array,
                      Allocator& allocator)
    {
        assert(ptr != nullptr);
        using control_allocator = typename std::allocator_traits<Allocator>::
                template rebind_alloc<memory_chunk_header>;
        using original_alloc_allocator =
                typename std::allocator_traits<Allocator>::
                    template rebind_alloc<Allocator>;

        auto allocator_control = control_allocator{allocator};
        auto* control_ptr = reinterpret_cast<memory_chunk_header*>(
                ptr + sizeof(Allocator));
        auto& helper = type_helper_impl<T, Allocator>::instance();
        std::allocator_traits<control_allocator>::
                template construct<memory_chunk_header>(
                        allocator_control, control_ptr,
                        helper, is_array);
        try
        {
            auto allocator_alloc = original_alloc_allocator{allocator};
            auto* alloc_ptr = reinterpret_cast<Allocator*>(ptr);
            std::allocator_traits<original_alloc_allocator>::
                    template construct<Allocator>(allocator_alloc, alloc_ptr,
                            allocator);
            return control_ptr;
        }
        catch (...)
        {
            std::allocator_traits<control_allocator>::
                    destroy(allocator_control, control_ptr);
        }
    }

}; // construct_helper<T>

template <typename T>
struct simple_allocator_helper
{
    template <typename Allocator, typename... Args>
    static T*
    allocate_deferred(deferred_heap_impl& heap, const Allocator& allocator,
                      Args&&... args)
    {
        using bytes_allocator = typename std::allocator_traits<Allocator>::
                template rebind_alloc<unsigned char>;
        constexpr std::size_t allocation_size =
                sizeof(T) + sizeof(memory_chunk_header) + sizeof(Allocator);

        auto allocator_raw = bytes_allocator{allocator};
        auto* raw_pointer = std::allocator_traits<bytes_allocator>::allocate(
                allocator_raw, allocation_size);
        assert(raw_pointer != nullptr);
        try
        {
            auto [control_ptr, offset_ptr] = construct_helper<T>::construct(
                    &(*raw_pointer), 1, false,
                    allocator, std::forward<Args>(args)...);
            assert(control_ptr != nullptr);
            assert(offset_ptr != nullptr);
            raw_pointer = nullptr;
            deferred_heap_impl_move_memory_to_deferred_heap(heap, control_ptr);
            return offset_ptr;
        }
        catch(...)
        {
            if (raw_pointer != nullptr)
            {
                std::allocator_traits<bytes_allocator>::deallocate(
                        allocator_raw, raw_pointer, allocation_size);
            }
            throw;
        }
    }

}; // simple_allocator_helper<T>

template <typename T>
struct simple_allocator_helper<T[]>
{
    template <typename Allocator>
    static T*
    allocate_deferred(deferred_heap_impl& heap, const Allocator& allocator,
                      std::size_t n_objects, const std::remove_extent_t<T>& u)
    {
        return allocate_deferred_impl(heap, allocator, n_objects, u);
    }

    template <typename Allocator>
    static T*
    allocate_deferred(deferred_heap_impl& heap, const Allocator& allocator,
                      std::size_t n_objects)
    {
        return allocate_deferred_impl(heap, allocator, n_objects);
    }

private:
    template <typename Allocator, typename... Args>
    static T*
    allocate_deferred_impl(deferred_heap_impl& heap, const Allocator& allocator,
                           std::size_t n_objects, Args&&... args)
    {
        using bytes_allocator = typename std::allocator_traits<Allocator>::
                template rebind_alloc<unsigned char>;
        constexpr std::size_t allocation_size =
                sizeof(T) * n_objects +
                sizeof(memory_chunk_header) +
                sizeof(memory_chunk_header::size_t) +
                sizeof(Allocator);

        const auto allocator_raw = bytes_allocator{allocator};
        auto* raw_pointer = std::allocator_traits<bytes_allocator>::allocate(
                allocator_raw, allocation_size);
        assert(raw_pointer != nullptr);
        (*reinterpret_cast<memory_chunk_header::size_t*>(raw_pointer)) =
                n_objects;
        try
        {
            auto [control_ptr, offset_ptr] = construct_helper<T>::construct(
                    &(*raw_pointer) + sizeof(memory_chunk_header::size_t),
                    n_objects, true,
                    allocator, std::forward<Args>(args)...);
            assert(control_ptr != nullptr);
            assert(offset_ptr != nullptr);
            raw_pointer = nullptr;
            deferred_heap_impl_move_memory_to_deferred_heap(heap, control_ptr);
            return offset_ptr;
        }
        catch(...)
        {
            if (raw_pointer != nullptr)
            {
                std::allocator_traits<bytes_allocator>::deallocate(
                        allocator_raw, raw_pointer, allocation_size);
            }
            throw;
        }
    }

}; // simple_allocator_helper<T[]>

template <typename T, size_t N>
struct simple_allocator_helper<T[N]>
{
    template <typename Allocator>
    static T*
    allocate_deferred(deferred_heap_impl& heap, const Allocator& allocator,
                      const std::remove_extent_t<T>& u)
    {
        return simple_allocator_helper<T[]>::allocate_deferred(
                heap, allocator, N, u);
    }

    template <typename Allocator>
    static T*
    allocate_deferred(deferred_heap_impl& heap, const Allocator& allocator)
    {
        return simple_allocator_helper<T[]>::allocate_deferred(
                heap, allocator, N);
    }

}; // simple_allocator_helper<T[N]>

} // namespace def::detail

namespace def
{

class deferred_heap;

class simple_allocator
{
public:
    template <typename T, typename... Args>
    deferred_ptr<T> make_deferred(Args&&... args)
    {
        return allocate_deferred<T, std::allocator<T>, Args...>(
                std::allocator<T>{}, std::forward<Args>(args)...);
    }

    template <typename T, typename Allocator, typename... Args>
    deferred_ptr<T>
    allocate_deferred(const Allocator& allocator, Args&&... args)
    {
        assert(m_heap);
        const auto ptr = detail::simple_allocator_helper<T>::
                template allocate_deferred<Allocator, Args...>(
                        *m_heap, allocator, std::forward<Args>(args)...);
        return deferred_ptr<T>{ptr};
    }

    template <typename T>
    void destroy_deferred(deferred_ptr<T>& ptr)
    {
        destroy_deferred_impl<deferred_ptr<T>>(ptr);
    }

    template <typename T>
    void destroy_deferred(root_ptr<T>& ptr)
    {
        destroy_deferred_impl<root_ptr<T>>(ptr);
    }

    template <typename T>
    void destroy_deferred(deferred_ptr<T> ptr)
    {
        destroy_deferred_impl<deferred_ptr<T>>(ptr);
    }

private:
    explicit simple_allocator(detail::deferred_heap_impl& heap)
    : m_heap{&heap}
    { }

    template <typename P>
    void destroy_deferred_impl(P& def_ptr)
    {
        using deferred_pointer = P;
        using base_pointer = typename deferred_pointer::pointer;

        base_pointer ptr = def_ptr.get();
        if (ptr == nullptr)
            return;

        detail::memory_chunk_header* header =
                detail::memory_chunk_header::from_object_start(ptr);
        if (header && !header->flags.is_destroyed())
        {
            header->helper.destroy(*header);
            assert(header->flags.is_destroyed());
        }
        def_ptr = nullptr;
    }

private:
    friend class deferred_heap;

    detail::deferred_heap_impl* m_heap;

}; // simple_allocator

} // namespace def
