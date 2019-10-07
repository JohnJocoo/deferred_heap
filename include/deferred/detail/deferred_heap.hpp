#pragma once

#include <memory>

namespace def::detail
{

class deferred_heap_impl;

} // namespace def::detail

namespace def
{

class deferred_heap
{
public:
    using chunks_number = std::size_t;
    using objects_number = std::size_t;
    using bytes_number = std::size_t;

    struct stats
    {
        chunks_number chunks;
        objects_number objects;
        bytes_number bytes;

    }; // struct stats

public:
    deferred_heap();

    deferred_heap(const deferred_heap&) = delete;
    deferred_heap(deferred_heap&&) = delete;

    ~deferred_heap();


    deferred_heap& operator=(const deferred_heap&) = delete;
    deferred_heap& operator=(deferred_heap&&) = delete;

    chunks_number get_memory_chunks_number() const;
    chunks_number get_root_memory_chunks_number() const;
    objects_number get_objects_number() const;
    objects_number get_root_objects_number() const;
    bytes_number get_total_bytes() const;

    stats release_unreachable();

private:
    const std::unique_ptr<detail::deferred_heap_impl> m_pimpl;

}; // class deferred_heap

} // namespace def
