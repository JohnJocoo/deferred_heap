#pragma once

#include <typeinfo>

namespace def::detail
{

struct memory_chunk_header;

/// Base for helper for deferred-enabled type.
class type_helper
{
public:
    explicit type_helper(const std::type_info& info,
                         std::size_t bytes_object,
                         std::size_t bytes_allocator)
    : type_info{info}
    , bytes_per_object{bytes_object}
    , bytes_per_allocator{bytes_allocator}
    { }

    /// Traverse through deferred pointers known to
    /// deferred-enabled type and recursively mark them as visited.
    virtual void mark_recursive(memory_chunk_header&) const = 0;

    /// Run destructor(s).
    void destroy(memory_chunk_header&) const;

    /// Free memory.
    virtual void deallocate(memory_chunk_header*) const = 0;

private:
    virtual void destroy_impl(memory_chunk_header&) const = 0;

public:
    const std::type_info& type_info;
    const std::size_t bytes_per_object;
    const std::size_t bytes_per_allocator;

}; // class type_helper

} // namespace def::detail
