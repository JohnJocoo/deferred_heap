#pragma once

#include <utility>

#include "deferred_ptr.hpp"
#include "root_ptr_base.hpp"

namespace def
{

/**
 * @brief root_ptr is a smart pointer used with deferred heap.
 * It behaves exactly as deferred_ptr, but it will also mark objects
 * as root ones until there is at least one root_ptr referencing it.
 * Root objects are in the base of deferred_heap graph and
 * are never deleted. Other objects are not deleted until
 * there is a way to get to them from root object through
 * other deferred_ptrs.
 * @tparam T type of referred object
 */
template <typename T>
class root_ptr : public detail::root_ptr_base, public deferred_ptr<T>
{
public:
    using pointer	   = typename deferred_ptr<T>::pointer;
    using element_type = typename deferred_ptr<T>::element_type;

    using deferred_ptr<T>::get;

public:
    // Constructors.

    /// Default constructor, creates an empty root_ptr.
    constexpr root_ptr() = default;

    /// Creates an empty root_ptr.
    constexpr root_ptr(nullptr_t) noexcept
    : deferred_ptr<T>{nullptr}
    { }

    /// Copy constructor. Marks object referred by other as root.
    root_ptr(const deferred_ptr<T>& other)
    : deferred_ptr<T>{other}
    {
        if (has_ptr())
            increment_root_references(get());
    }

    /// Converting constructor from another type.
    template<typename Up>
    root_ptr(const deferred_ptr<Up>& other)
    : deferred_ptr<T>{other}
    {
        if (has_ptr())
            increment_root_references(get());
    }

    /// Move constructor. Move pointer from other,
    /// avoiding redundant calls to mark object as root.
    root_ptr(root_ptr<T>&& other) noexcept
    : deferred_ptr<T>{std::move(other)}
    {
        static_cast<deferred_ptr<T>&>(other).operator=(nullptr);
    }

    /// Converting move constructor from another type.
    template<typename Up>
    root_ptr(root_ptr<Up>&& other) noexcept
    : deferred_ptr<T>{std::move(other)}
    {
        static_cast<deferred_ptr<Up>&>(other).operator=(nullptr);
    }

    /// Destructor, try to release root mark .
    ~root_ptr() noexcept
    {
        release();
    }

    // Assignment.

    /// Copy assignment operator. Mark object as root.
    root_ptr& operator=(const deferred_ptr<T>& other)
    {
        release();
        deferred_ptr<T>::operator=(other);
        if (has_ptr())
            increment_root_references(get());
        return *this;
    }

    /// Assignment from another type.
    template<typename Up>
    root_ptr& operator=(const deferred_ptr<Up>& other)
    {
        release();
        deferred_ptr<T>::operator=(other);
        if (has_ptr())
            increment_root_references(get());
        return *this;
    }

    /// Move operator. Avoid redundant call to mark object as root.
    root_ptr& operator=(root_ptr<T>&& other) noexcept
    {
        release();
        deferred_ptr<T>::operator=(other);
        static_cast<deferred_ptr<T>&>(other).operator=(nullptr);
        return *this;
    }

    /// Move from another type.
    template<typename Up>
    root_ptr& operator=(root_ptr<Up>&& other) noexcept
    {
        release();
        deferred_ptr<T>::operator=(other);
        static_cast<deferred_ptr<Up>&>(other).operator=(nullptr);
        return *this;
    }

    /// Reset the root_ptr to empty. Try to release root mark.
    root_ptr& operator=(nullptr_t) noexcept
    {
        release();
        return *this;
    }

private:
    bool has_ptr() const noexcept
    {
        return static_cast<bool>(*this);
    }

    void release() noexcept
    {
        if (has_ptr())
            decrement_root_references(get());
        deferred_ptr<T>::operator=(nullptr);
    }

}; // class root_ptr<T>

} // namespace def
