#include "deferred/detail/memory_chunk_header.hpp"

#include <limits>
#include <stdexcept>
#include <cassert>

#include "deferred/detail/deferred_type_helper.hpp"

namespace
{

using chunk_flags_underlying_type = uint16_t;

template <chunk_flags_underlying_type F>
struct flag_base
{
    static constexpr chunk_flags_underlying_type value = F;
    static constexpr chunk_flags_underlying_type negated_value = ~F;
};

template <chunk_flags_underlying_type F>
void set_flag(chunk_flags_underlying_type& data, const flag_base<F>&)
{
    data |= flag_base<F>::value;
}

template <chunk_flags_underlying_type F>
void remove_flag(chunk_flags_underlying_type& data, const flag_base<F>&)
{
    data &= flag_base<F>::negated_value;
}

template <chunk_flags_underlying_type F>
bool test_flag(const chunk_flags_underlying_type& data, const flag_base<F>&)
{
    return (data & flag_base<F>::value) == flag_base<F>::value;
}

const flag_base<0x0001u> array_flag;
const flag_base<0x0002u> destroyed_flag;
const flag_base<0x0004u> visited_flag;

}

namespace def::detail
{

memory_chunk_header::chunk_flags::chunk_flags(bool is_array) noexcept
: m_data(is_array ? array_flag.value : 0u)
, m_root_references{0u}
{ }

bool memory_chunk_header::chunk_flags::is_visited() const noexcept
{
    return test_flag(m_data, visited_flag);
}

void memory_chunk_header::chunk_flags::mark_visited() noexcept
{
    set_flag(m_data, visited_flag);
}

void memory_chunk_header::chunk_flags::clear_visited() noexcept
{
    remove_flag(m_data, visited_flag);
}

bool memory_chunk_header::chunk_flags::is_array() const noexcept
{
    return test_flag(m_data, array_flag);
}

bool memory_chunk_header::chunk_flags::is_destroyed() const noexcept
{
    return test_flag(m_data, destroyed_flag);
}

void memory_chunk_header::chunk_flags::mark_destroyed() noexcept
{
    set_flag(m_data, destroyed_flag);
}

bool memory_chunk_header::chunk_flags::is_root() const noexcept
{
    return m_root_references != 0u;
}

void memory_chunk_header::chunk_flags::increment_root_reference()
{
    if (m_root_references ==
        std::numeric_limits<root_reference_counter_type>::max())
    {
        throw std::overflow_error{"max number of root references reached"};
    }
    ++m_root_references;
}

void memory_chunk_header::chunk_flags::decrement_root_reference()
{
    if (m_root_references == 0u)
        assert(false);
    --m_root_references;
}

memory_chunk_header::size_t
memory_chunk_header::get_objects_number() const noexcept
{
    if (!flags.is_array())
        return 1;
    auto size_ptr =
            reinterpret_cast<uint8_t*>(get_allocator_start()) - sizeof(size_t);
    return *(reinterpret_cast<size_t*>(size_ptr));
}

void* memory_chunk_header::get_object_start() const noexcept
{
    const auto obj_ptr = (reinterpret_cast<const uint8_t*>(this)
            + sizeof(memory_chunk_header));
    return const_cast<uint8_t*>(obj_ptr);
}

void* memory_chunk_header::get_allocator_start() const noexcept
{
    const auto allocator_ptr =
            (reinterpret_cast<const uint8_t*>(this) -
            helper.bytes_per_allocator);
    return const_cast<uint8_t*>(allocator_ptr);
}

void* memory_chunk_header::get_raw_memory_start() const noexcept
{
    if (!flags.is_array())
        return get_allocator_start();
    return reinterpret_cast<uint8_t*>(get_allocator_start())
            - sizeof(memory_chunk_header::size_t);
}

memory_chunk_header::size_t
memory_chunk_header::get_bytes_allocated() const noexcept
{
    size_t res = helper.bytes_per_object * get_objects_number();
    if (flags.is_array())
        res += sizeof(size_t);
    return res + sizeof(memory_chunk_header) + helper.bytes_per_allocator;
}

} // namespace def::detail