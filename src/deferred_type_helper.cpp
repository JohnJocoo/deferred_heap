#include "deferred/detail/deferred_type_helper.hpp"

#include "deferred/detail/memory_chunk_header.hpp"

namespace def::detail
{

void type_helper::destroy(memory_chunk_header& header) const
{
    if (header.flags.is_destroyed())
        return;
    destroy_impl(header);
    header.flags.mark_destroyed();
}

} // namespace def::detail
