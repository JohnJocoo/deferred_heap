#include "deferred/detail/deferred_simple_allocator.hpp"

#include "deferred/detail/deferred_heap_impl.hpp"

namespace def::detail
{

void deferred_heap_impl_move_memory_to_deferred_heap(
        deferred_heap_impl& heap, memory_chunk_header* header) noexcept(false)
{
    heap.receive_chunk(deferred_heap_impl::chunk_unique_ptr{header});
}

} // namespace def::detail