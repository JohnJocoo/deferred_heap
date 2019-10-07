#pragma once

namespace def::detail
{

class root_ptr_base
{
protected:
    template <typename T>
    static void increment_root_references(T* obj)
    {
        increment_root_references_impl(reinterpret_cast<void*>(obj));
    }

    template <typename T>
    static void decrement_root_references(T* obj)
    {
        decrement_root_references_impl(reinterpret_cast<void*>(obj));
    }

    static void increment_root_references_impl(void*);
    static void decrement_root_references_impl(void*);

}; // class root_ptr_base

} // namespace def::detail
