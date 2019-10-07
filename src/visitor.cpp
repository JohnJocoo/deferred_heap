#include "deferred/detail/visitor.hpp"

#include <cassert>

#include "deferred/detail/memory_chunk_header.hpp"

namespace def
{

detail::memory_chunk_header* visitor::to_header_impl(void* ptr)
{
    assert(ptr != nullptr);
    return detail::memory_chunk_header::from_object_start(ptr);
}

bool visitor::is_visited(detail::memory_chunk_header* ptr)
{
    assert(ptr != nullptr);
    ptr->flags.is_visited();
}

} // namespace def
