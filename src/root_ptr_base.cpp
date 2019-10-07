#include "deferred/detail/root_ptr_base.hpp"

#include "deferred/detail/memory_chunk_header.hpp"

namespace def::detail
{

void root_ptr_base::increment_root_references_impl(void* ptr)
{
    auto chunk_ptr = memory_chunk_header::from_object_start(ptr);
    if (chunk_ptr)
        chunk_ptr->flags.increment_root_reference();
}

void root_ptr_base::decrement_root_references_impl(void* ptr)
{
    auto chunk_ptr = memory_chunk_header::from_object_start(ptr);
    if (chunk_ptr)
        chunk_ptr->flags.decrement_root_reference();
}

} // namespace def::detail
