#pragma once

#include <cstddef>
#include <cstdint>

namespace def::detail
{

class type_helper;

/// Util memory header for deferred heap to control memory chunks.
/// Any memory chunk allocated with deferred allocator
/// is allocated with bigger size to incorporate this header,
/// copy of allocator and also number of objects
/// in case of array allocation.
struct memory_chunk_header
{
    using size_t = std::size_t;

    class chunk_flags
    {
        using underlying_type = uint16_t;
        using root_reference_counter_type = uint16_t;

    public:
        explicit chunk_flags(bool is_array) noexcept;

        bool is_visited() const noexcept;
        void mark_visited() noexcept;
        void clear_visited() noexcept;

        bool is_array() const noexcept;

        bool is_destroyed() const noexcept;
        void mark_destroyed() noexcept;

        bool is_root() const noexcept;
        void increment_root_reference();
        void decrement_root_reference();

    private:
        underlying_type m_data;
        root_reference_counter_type m_root_references;

    }; // class chunk_flags

    explicit memory_chunk_header(const type_helper& helper,
                                 bool is_array) noexcept
    : helper{helper}
    , flags{is_array}
    { }

    size_t get_objects_number() const noexcept;
    void* get_object_start() const noexcept;
    void* get_allocator_start() const noexcept;
    void* get_raw_memory_start() const noexcept;

    size_t get_bytes_allocated() const noexcept;

    const type_helper& helper;
    chunk_flags flags;

}; // struct memory_chunk_header

} // namespace def::detail
