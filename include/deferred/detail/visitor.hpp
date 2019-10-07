#pragma once

#include <vector>
#include <cassert>

#include "deferred_ptr.hpp"

namespace def
{

namespace detail
{

struct memory_chunk_header;

} // namespace detail

class visitor
{
public:
    explicit visitor(std::vector<detail::memory_chunk_header*>& not_visited)
            : m_not_visited{not_visited}
    {}

    template <typename T>
    void visit(deferred_ptr<T>& ptr)
    {
        if (ptr == nullptr)
            return;
        auto* header = to_header(ptr.get());
        assert(header != nullptr);
        if (is_visited(header))
            return;
        m_not_visited.push_back(header);
    }

private:
    template <typename T>
    detail::memory_chunk_header* to_header(T* ptr)
    {
        return to_header_impl(reinterpret_cast<void*>(ptr));
    }

    static detail::memory_chunk_header* to_header_impl(void*);
    static bool is_visited(detail::memory_chunk_header*);

private:
    std::vector<detail::memory_chunk_header*>& m_not_visited;

}; // class visitor

} // namespace def