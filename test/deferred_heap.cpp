#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>

#include "deferred/simple_allocator"
#include "deferred/deferred_heap"
#include "deferred/deferred_ptr"
#include "deferred/root_ptr"

namespace
{

struct simple_struct
{
    simple_struct()
    : val{0}
    {}

    simple_struct(int val, std::string str)
    : val{val}
    , str{str}
    {}

    int val;
    std::string str;
};

struct simple_link_struct
{
    simple_link_struct()
    {}

    simple_link_struct(def::deferred_ptr<simple_struct> leaf)
    : leaf{leaf}
    {}
    
    simple_link_struct(def::deferred_ptr<simple_link_struct> next)
    : next{next}
    {}
    
    simple_link_struct(def::deferred_ptr<simple_struct> leaf,
                       def::deferred_ptr<simple_link_struct> next)
    : leaf{leaf}
    , next{next}
    {}
    
    void visit(def::visitor& visitor)
    {
        visitor.visit(leaf);
        visitor.visit(next);
    }

    def::deferred_ptr<simple_struct> leaf;
    def::deferred_ptr<simple_link_struct> next;
};

}

TEST(deferred_heap, smoke)
{
    def::deferred_heap heap;
    EXPECT_EQ(0, heap.get_memory_chunks_number());
    EXPECT_EQ(0, heap.get_root_memory_chunks_number());
    EXPECT_EQ(0, heap.get_objects_number());
    EXPECT_EQ(0, heap.get_root_objects_number());
    EXPECT_EQ(0, heap.get_total_bytes());
    
    auto stats = heap.release_unreachable();
    EXPECT_EQ(0, stats.bytes);
    EXPECT_EQ(0, stats.chunks);
    EXPECT_EQ(0, stats.objects);
}
