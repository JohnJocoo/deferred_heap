#pragma once

#include <cstddef>
#include <cassert>
#include <functional>


namespace def
{

class simple_allocator;

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
    using pointer	   = T*;
    using element_type = T;

public:
    // Constructors.

    /// Default constructor, creates an empty deferred_ptr.
    constexpr deferred_ptr() noexcept
    : m_ptr{nullptr}
    { }

    /// Creates an empty deferred_ptr.
    constexpr deferred_ptr(nullptr_t) noexcept
    : m_ptr{nullptr}
    { }

    /// Copy constructor. Do simple copy as deferred_ptr doesn't own an object.
    deferred_ptr(const deferred_ptr<T>&) = default;

    /// Converting constructor from another type.
    template<typename Up>
    deferred_ptr(const deferred_ptr<Up>& other) noexcept
    : m_ptr{other.m_ptr}
    { }

    /// Destructor, do nothing as deferred_ptr doesn't own an object.
    ~deferred_ptr() noexcept
    {
        m_ptr = nullptr;
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
        return *this;
    }

    /// Reset the deferred_ptr to empty.
    deferred_ptr& operator=(nullptr_t) noexcept
    {
        m_ptr = nullptr;
        return *this;
    }

    // Observers.

    /// Dereference the stored pointer.
    typename std::add_lvalue_reference<element_type>::type
    operator*() const noexcept
    {
        assert(m_ptr);
        return *m_ptr;
    }

    /// Return the stored pointer.
    pointer operator->() const noexcept
    {
        assert(m_ptr);
        return m_ptr;
    }

    /// Return the stored pointer.
    pointer get() const noexcept
    {
        return m_ptr;
    }

    /// Return true if the stored pointer is not null.
    explicit operator bool() const noexcept
    {
        return m_ptr != nullptr;
    }

private:
    /// Constructor to be used only by deferred allocator.
    explicit deferred_ptr(pointer ptr) noexcept
    : m_ptr{ptr}
    { }

private:
    friend class simple_allocator;

    pointer m_ptr;

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
