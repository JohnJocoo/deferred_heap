#pragma once

#include <memory>
#include <tuple>
#include <vector>

#include "memory_chunk_header.hpp"

namespace def::detail
{

struct deferred_memory_deleter
{
    void operator()(memory_chunk_header*) const;
};

class deferred_heap_impl
{
public:
    using chunks_number = std::size_t;
    using bytes_number = std::size_t;
    using objects_number = std::size_t;

    using chunk_unique_ptr = std::unique_ptr<detail::memory_chunk_header,
                                             detail::deferred_memory_deleter>;
    using chunk_ptr = detail::memory_chunk_header*;

public:
    deferred_heap_impl();

    ~deferred_heap_impl();

    chunks_number get_chunks_number() const;
    chunks_number get_root_chunks_number() const;
    objects_number get_objects_number() const;
    objects_number get_root_objects_number() const;
    bytes_number get_total_bytes() const;

    std::tuple<chunks_number, objects_number, bytes_number>
    mark_and_swipe();

    void receive_chunk(chunk_unique_ptr&&);

private:
    void clear_all_visited();
    void visit_mark_all();
    std::tuple<chunks_number, objects_number, bytes_number>
    swipe_all_non_marked();

private:
    std::vector<chunk_unique_ptr> m_all_chunks;

}; // class deferred_heap::impl

} // namespace def::detail
