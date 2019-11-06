#pragma once

#include <cstddef>
#include <cassert>
#include <functional>
#include <type_traits>


namespace def
{

namespace detail
{

struct memory_chunk_header;

} // namespace detail

class simple_allocator;
class visitor;

/**
 * @brief deferred_ptr is a smart pointer used with deferred heap.
 * It does not own object it refers to, as its main goal is to
 * be automatically used be deferred heap for tracing reachability of objects.
 * Memory in deferred heap is actually owned only by heap itself.
 * This class is to be used as ordinary pointer.
 * @tparam T type of referred object
 */
template <typename T>
class deferred_ptr
{
public:
    using element_type = std::remove_extent_t<T>;
    using pointer      = element_type*;

public:
    // Constructors.

    /// Default constructor, creates an empty deferred_ptr.
    constexpr deferred_ptr() noexcept
    : m_ptr{nullptr}
    , m_header{nullptr}
    { }

    /// Creates an empty deferred_ptr.
    constexpr deferred_ptr(nullptr_t) noexcept
    : m_ptr{nullptr}
    , m_header{nullptr}
    { }

    /// Copy constructor. Do simple copy as deferred_ptr doesn't own an object.
    deferred_ptr(const deferred_ptr<T>&) = default;

    /// Converting constructor from another type.
    template<typename Up>
    deferred_ptr(const deferred_ptr<Up>& other) noexcept
    : m_ptr{other.m_ptr}
    , m_header{other.m_header}
    { }

    /// Destructor, do nothing as deferred_ptr doesn't own an object.
    ~deferred_ptr() noexcept
    {
        m_ptr = nullptr;
        m_header = nullptr;
    }

    // Assignment.

    /// Copy assignment operator.
    /// Do simple copy as deferred_ptr doesn't own an object.
    deferred_ptr& operator=(const deferred_ptr<T>&) = default;

    /// Assignment from another type.
    template<typename Up>
    deferred_ptr& operator=(const deferred_ptr<Up>& other) noexcept
    {
        m_ptr = other.m_ptr;
        m_header = other.m_header;
        return *this;
    }

    /// Reset the deferred_ptr to empty.
    deferred_ptr& operator=(nullptr_t) noexcept
    {
        m_ptr = nullptr;
        m_header = nullptr;
        return *this;
    }

    // Observers.

    /// Dereference the stored pointer.
    typename std::add_lvalue_reference<T>::type
    operator*() const noexcept
    {
        assert(m_ptr);
        return *((T*)m_ptr);
    }

    /// Return the stored pointer.
    T* operator->() const noexcept
    {
        assert(m_ptr);
        return m_ptr;
    }

    template <typename C = T>
    std::enable_if_t<std::is_array_v<C>, element_type&>
    operator[](std::ptrdiff_t idx) const
    {
        return m_ptr[idx];
    }

    /// Return the stored pointer.
    pointer get() const noexcept
    {
        return m_ptr;
    }

    /// Return true if the stored pointer is not null.
    explicit operator bool() const noexcept
    {
        return m_ptr != nullptr && m_header != nullptr;
    }

protected:
    detail::memory_chunk_header* get_header() const noexcept
    {
        return m_header;
    }

private:
    /// Constructor to be used only by deferred allocator.
    explicit deferred_ptr(detail::memory_chunk_header* header,
                          pointer ptr) noexcept
    : m_ptr{ptr}
    , m_header{header}
    {
        // both nullptr, or both have value
        assert((m_ptr == nullptr) == (m_header == nullptr));
    }

private:
    template <typename U>
    friend class deferred_ptr;
    friend class simple_allocator;
    friend class visitor;

    pointer                         m_ptr;
    detail::memory_chunk_header*    m_header;

}; // class deferred_ptr<T>

template <typename T1, typename T2>
inline bool operator==(const deferred_ptr<T1>& ptr_left,
                       const deferred_ptr<T2>& ptr_right)
{
    return ptr_left.get() == ptr_right.get();
}

template <typename T>
inline bool operator==(nullptr_t,
                       const deferred_ptr<T>& ptr)
{
    return !ptr;
}

template <typename T>
inline bool operator==(const deferred_ptr<T>& ptr,
                       nullptr_t)
{
    return !ptr;
}

template <typename T1, typename T2>
inline bool operator!=(const deferred_ptr<T1>& ptr_left,
                       const deferred_ptr<T2>& ptr_right)
{
    return ptr_left.get() != ptr_right.get();
}

template <typename T>
inline bool operator!=(nullptr_t,
                       const deferred_ptr<T>& ptr)
{
    return static_cast<bool>(ptr);
}

template <typename T>
inline bool operator!=(const deferred_ptr<T>& ptr,
                       nullptr_t)
{
    return static_cast<bool>(ptr);
}

template <typename T1, typename T2>
inline bool operator<(const deferred_ptr<T1>& ptr_left,
                      const deferred_ptr<T2>& ptr_right)
{
    typedef typename
    std::common_type<typename deferred_ptr<T1>::pointer,
                     typename deferred_ptr<T2>::pointer>::type CT;
    return std::less<CT>()(ptr_left.get().get(), ptr_right.get());
}

template <typename T>
inline bool operator<(nullptr_t,
                      const deferred_ptr<T>& ptr)
{
    return std::less<typename deferred_ptr<T>::pointer>()(nullptr,
                                                          ptr.get());
}

template <typename T>
inline bool operator<(const deferred_ptr<T>& ptr,
                      nullptr_t)
{
    return std::less<typename deferred_ptr<T>::pointer>()(ptr.get(),
                                                          nullptr);
}

template <typename T1, typename T2>
inline bool operator<=(const deferred_ptr<T1>& ptr_left,
                       const deferred_ptr<T2>& ptr_right)
{
    return !(ptr_right < ptr_left);
}

template <typename T>
inline bool operator<=(nullptr_t,
                       const deferred_ptr<T>& ptr)
{
    return !(ptr < nullptr);
}

template <typename T>
inline bool operator<=(const deferred_ptr<T>& ptr,
                       nullptr_t)
{
    return !(nullptr < ptr);
}

template <typename T1, typename T2>
inline bool operator>(const deferred_ptr<T1>& ptr_left,
                      const deferred_ptr<T2>& ptr_right)
{
    return ptr_right < ptr_left;
}

template <typename T>
inline bool operator>(nullptr_t,
                      const deferred_ptr<T>& ptr)
{
    return ptr < nullptr;
}

template <typename T>
inline bool operator>(const deferred_ptr<T>& ptr,
                      nullptr_t)
{
    return nullptr < ptr;
}

template <typename T1, typename T2>
inline bool operator>=(const deferred_ptr<T1>& ptr_left,
                       const deferred_ptr<T2>& ptr_right)
{
    return !(ptr_left < ptr_right);
}

template <typename T>
inline bool operator>=(nullptr_t,
                       const deferred_ptr<T>& ptr)
{
    return !(nullptr < ptr);
}

template <typename T>
inline bool operator>=(const deferred_ptr<T>& ptr,
                       nullptr_t)
{
    return !(ptr < nullptr);
}

} // namespace def
