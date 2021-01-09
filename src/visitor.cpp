#include "deferred/detail/visitor.hpp"

#include <cassert>

#include "deferred/detail/memory_chunk_header.hpp"

namespace def
{

bool visitor::is_visited(detail::memory_chunk_header* ptr)
{
    assert(ptr != nullptr);
    return ptr->flags.is_visited();
}

} // namespace def
