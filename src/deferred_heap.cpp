#include "deferred/detail/deferred_heap.hpp"
#include "deferred/detail/deferred_heap_impl.hpp"

#include <algorithm>
#include <numeric>
#include <iterator>
#include <cassert>

#include "deferred/detail/deferred_type_helper.hpp"

namespace
{

template <typename I, typename F>
class filtering_iterator :
        public std::iterator<std::forward_iterator_tag,
                             typename I::value_type>
{
    using iterator = I;
    using filter = F;
    using parent = std::iterator<std::forward_iterator_tag,
                                 typename I::value_type>;

public:
    using typename parent::iterator_category;
    using typename parent::value_type;
    using typename parent::difference_type;
    using typename parent::pointer;
    using typename parent::reference;

public:
    explicit filtering_iterator(const iterator& i,
            const iterator& end, const filter& f)
    : m_i{i}
    , m_end{end}
    , m_filter{f}
    {}

    filtering_iterator& operator++()
    {
        ++m_i;
        while (m_i != m_end && !m_filter(*m_i))
            ++m_i;
        return *this;
    }

    filtering_iterator operator++(int)
    {
        filtering_iterator tmp(*this);
        operator++();
        return tmp;
    }

    bool operator==(const filtering_iterator& other) const
    {
        return m_i == other.m_i;
    }

    bool operator!=(const filtering_iterator& other) const
    {
        return m_i != other.m_i;
    }

    auto& operator*()
    {
        return *m_i;
    }

    auto operator->()
    {
        return m_i.operator->();
    }

private:
    iterator m_i;
    iterator m_end;
    filter m_filter;

}; // class filtering_iterator<I, F>

const auto root_filter = [](const auto& chunk_ptr) -> bool
{
    return chunk_ptr->flags.is_root();
};

template <typename It>
auto count_objects(const It& _begin, const It& _end)
{
    return std::accumulate(_begin, _end, std::size_t{0},
            [](const auto& acc, const auto& chunk_ptr)
            {
                return acc + chunk_ptr->get_objects_number();
            });
}

} // namespace

namespace def
{

namespace detail
{

void deferred_memory_deleter::operator()(memory_chunk_header* chunk_ptr) const
{
    if (chunk_ptr == nullptr)
        return;
    if (!chunk_ptr->flags.is_destroyed())
        chunk_ptr->helper.destroy(*chunk_ptr);
    assert(chunk_ptr->flags.is_destroyed());
    chunk_ptr->helper.deallocate(chunk_ptr);
}

deferred_heap_impl::deferred_heap_impl() = default;

deferred_heap_impl::~deferred_heap_impl() = default;

deferred_heap_impl::chunks_number
deferred_heap_impl::get_chunks_number() const
{
    return m_all_chunks.size();
}

deferred_heap_impl::chunks_number
deferred_heap_impl::get_root_chunks_number() const
{
    return std::count_if(begin(m_all_chunks), end(m_all_chunks), root_filter);
}

deferred_heap_impl::objects_number
deferred_heap_impl::get_objects_number() const
{
    return count_objects(begin(m_all_chunks), end(m_all_chunks));
}

deferred_heap_impl::objects_number
deferred_heap_impl::get_root_objects_number() const
{
    const auto begin_v = begin(m_all_chunks);
    const auto end_v = end(m_all_chunks);
    return count_objects(filtering_iterator{begin_v, end_v, root_filter},
                         filtering_iterator{end_v, end_v, root_filter});
}

deferred_heap_impl::bytes_number
deferred_heap_impl::get_total_bytes() const
{
    return std::accumulate(
            begin(m_all_chunks), end(m_all_chunks), bytes_number{0},
            [](const auto& acc, const auto& chunk_ptr)
            {
                return acc + chunk_ptr->get_bytes_allocated();
            });
}

std::tuple<deferred_heap_impl::chunks_number,
        deferred_heap_impl::objects_number,
        deferred_heap_impl::bytes_number>
deferred_heap_impl::mark_and_swipe()
{
    clear_all_visited();
    visit_mark_all();
    return swipe_all_non_marked();
}

void deferred_heap_impl::receive_chunk(chunk_unique_ptr&& ptr)
{
    m_all_chunks.push_back(std::move(ptr));
}

void deferred_heap_impl::clear_all_visited()
{
    for (auto& chunk_ptr: m_all_chunks)
    {
        chunk_ptr->flags.clear_visited();
    }
}

void deferred_heap_impl::visit_mark_all()
{
    std::vector<chunk_ptr> root_chunks;
    for (auto& chunk_ptr: m_all_chunks)
    {
        if (!chunk_ptr->flags.is_root())
            continue;
        chunk_ptr->flags.mark_visited();
        root_chunks.push_back(chunk_ptr.get());
    }
    for (auto& chunk_ptr: root_chunks)
    {
        chunk_ptr->helper.mark_recursive(*chunk_ptr);
    }
}

std::tuple<deferred_heap_impl::chunks_number,
        deferred_heap_impl::objects_number,
        deferred_heap_impl::bytes_number>
deferred_heap_impl::swipe_all_non_marked()
{
    const auto remove_it = std::partition(
            begin(m_all_chunks), end(m_all_chunks),
            [](const auto& chunk_ptr) -> bool
            {
                return chunk_ptr->flags.is_visited();
            });
    const auto end_it = end(m_all_chunks);
    const auto chunks_num = std::distance(remove_it, end_it);
    const auto obj_bytes_num = std::accumulate(remove_it, end_it,
               std::make_pair(objects_number{0}, bytes_number{0}),
               [](const auto& acc, const auto& chunk_ptr)
               {
                   using pair = decltype(acc);
                   return pair{acc.first + chunk_ptr->get_objects_number(),
                               acc.second + chunk_ptr->get_bytes_allocated()};
               });
    m_all_chunks.erase(remove_it, end_it);
    m_all_chunks.shrink_to_fit();
    return {chunks_num, obj_bytes_num.first, obj_bytes_num.second};
}

} // namespace detail

deferred_heap::deferred_heap()
: m_pimpl{std::make_unique<detail::deferred_heap_impl>()}
{ }

deferred_heap::~deferred_heap() = default;

deferred_heap::chunks_number
deferred_heap::get_memory_chunks_number() const
{
    return m_pimpl->get_chunks_number();
}

deferred_heap::chunks_number
deferred_heap::get_root_memory_chunks_number() const
{
    return m_pimpl->get_root_chunks_number();
}

deferred_heap::objects_number
deferred_heap::get_objects_number() const
{
    return m_pimpl->get_objects_number();
}

deferred_heap::objects_number
deferred_heap::get_root_objects_number() const
{
    return m_pimpl->get_root_objects_number();
}

deferred_heap::bytes_number
deferred_heap::get_total_bytes() const
{
    return m_pimpl->get_total_bytes();
}

simple_allocator
deferred_heap::get_simple_allocator()
{
    return simple_allocator{*m_pimpl};
}

deferred_heap::stats
deferred_heap::release_unreachable()
{
    const auto tuple_res = m_pimpl->mark_and_swipe();
    stats result;
    result.chunks = std::get<0>(tuple_res);
    result.objects = std::get<1>(tuple_res);
    result.bytes = std::get<2>(tuple_res);
    return result;
}

} // namespace def
