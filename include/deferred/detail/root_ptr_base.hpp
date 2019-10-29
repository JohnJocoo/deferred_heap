#pragma once

namespace def::detail
{

struct memory_chunk_header;

class root_ptr_base
{
protected:
    static void increment_root_references(memory_chunk_header*);
    static void decrement_root_references(memory_chunk_header*);

}; // class root_ptr_base

} // namespace def::detail
