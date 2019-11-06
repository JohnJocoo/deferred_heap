#include "gtest/gtest.h"

#include <string>
#include <vector>
#include <list>
#include <array>

#include "deferred/deferred_ptr"
#include "deferred/defines"
#include "deferred/is_deferred_base_of"
#include "deferred/visitor"

#include "deferred/detail/is_deferred_ptr.hpp"
#include "deferred/detail/deferred_type_traverse_helper.hpp"

namespace
{

// Classes using library macros

struct simple_struct
{};

struct child_struct_one : public simple_struct
{};

struct deferred_child_struct_simple_one : public simple_struct
{
    def::deferred_ptr<deferred_child_struct_simple_one> ptr;

    DEF_ENABLE_DEFERRED_REFLECTION(deferred_child_struct_simple_one);

    DEF_REGISTER_DEFERRED_MEMBER(ptr);
};

struct deferred_child_struct_simple_one_with_user : public simple_struct
{
    def::deferred_ptr<deferred_child_struct_simple_one> ptr;

    DEF_ENABLE_DEFERRED_REFLECTION(deferred_child_struct_simple_one_with_user);

    DEF_REGISTER_BASE_CLASS(simple_struct);

    DEF_REGISTER_DEFERRED_MEMBER(ptr);
};

struct deferred_struct
{
    def::deferred_ptr<deferred_child_struct_simple_one> ptr;

    DEF_ENABLE_DEFERRED_REFLECTION(deferred_struct);

    DEF_REGISTER_DEFERRED_MEMBER(ptr);
};

struct deferred_struct2
{
    def::deferred_ptr<deferred_child_struct_simple_one> ptr2;

    DEF_ENABLE_DEFERRED_REFLECTION(deferred_struct2);

    DEF_REGISTER_DEFERRED_MEMBER(ptr2);
};

struct deferred_child_struct_one : public deferred_struct
{
    DEF_ENABLE_DEFERRED_REFLECTION(deferred_child_struct_one);
};

struct deferred_child_struct_one_with_user : public deferred_struct
{
    DEF_ENABLE_DEFERRED_REFLECTION(deferred_child_struct_one_with_user);

    DEF_REGISTER_BASE_CLASS(deferred_struct);
};

struct deferred_child_struct_two_no_user :
        public deferred_struct, public deferred_struct2
{
    DEF_ENABLE_DEFERRED_REFLECTION(deferred_child_struct_two_no_user);
};

struct deferred_child_struct_two_with_user :
        public deferred_struct, public deferred_struct2
{
    DEF_ENABLE_DEFERRED_REFLECTION(deferred_child_struct_two_with_user);

    DEF_REGISTER_BASE_CLASS(deferred_struct, deferred_struct2);
};

struct deferred_child2_struct_one : public deferred_child_struct_one
{
    DEF_ENABLE_DEFERRED_REFLECTION(deferred_child2_struct_one);
};

struct deferred_child2_struct_two_with_user :
        public deferred_child_struct_two_with_user
{
    DEF_ENABLE_DEFERRED_REFLECTION(deferred_child2_struct_two_with_user);
};

// Visitor enabled classes

struct right_visitor
{
    void visit(def::visitor&)
    {}
};

struct const_member_visitor
{
    void visit(def::visitor&) const
    {}
};

struct const_visitor
{
    void visit(const def::visitor&)
    {}
};

struct right_visitor_private
{
private:
    void visit(def::visitor&)
    {}

    friend class def::visitor;
};

struct args_visitor1
{
    void visit(def::visitor&, int = 0)
    {}
};

struct args_visitor2
{
    void visit(def::visitor&, int)
    {}
};

struct multiple_member_visitor
{
    void visit()
    {}

    void visit(def::visitor&)
    {}

    void visit(int)
    {}
};

struct static_visitor
{
    static void visit(def::visitor&)
    {}
};

struct wrong_visitor_ptr
{
    void visit(def::visitor*)
    {}
};

struct empty_visitor
{ };

struct wrong_visitor_name
{
    void vizit(def::visitor&)
    {}
};

struct wrong_visitor_noarg
{
    void visit()
    {}
};

}


TEST(is_deferred_base_of, simple_struct)
{
    const auto child_struct_one_val =
            def::is_deferred_base_of<simple_struct, child_struct_one>::value;
    EXPECT_FALSE(child_struct_one_val);

    const auto deferred_child_struct_simple_one_val =
            def::is_deferred_base_of<simple_struct,
                    deferred_child_struct_simple_one>::value;
    EXPECT_FALSE(deferred_child_struct_simple_one_val);

    const auto deferred_child_struct_simple_one_with_user_val =
            def::is_deferred_base_of<simple_struct,
                    deferred_child_struct_simple_one_with_user>::value;
    EXPECT_TRUE(deferred_child_struct_simple_one_with_user_val);
}

TEST(is_deferred_base_of, self)
{
    const auto deferred_struct_val =
            def::is_deferred_base_of<deferred_struct, deferred_struct>::value;
    EXPECT_FALSE(deferred_struct_val);

    const auto deferred_child_struct_one_val =
            def::is_deferred_base_of<deferred_child_struct_one,
                    deferred_child_struct_one>::value;
    EXPECT_FALSE(deferred_child_struct_one_val);
}

TEST(is_deferred_base_of, one_base)
{
    const auto deferred_child_struct_one_val =
            def::is_deferred_base_of<deferred_struct,
                    deferred_child_struct_one>::value;
    EXPECT_TRUE(deferred_child_struct_one_val);

    const auto deferred_child_struct_one_wrong_base_val =
            def::is_deferred_base_of<deferred_struct2,
                    deferred_child_struct_one>::value;
    EXPECT_FALSE(deferred_child_struct_one_wrong_base_val);

    const auto deferred_child_struct_one_with_user_val =
            def::is_deferred_base_of<deferred_struct,
                    deferred_child_struct_one_with_user>::value;
    EXPECT_TRUE(deferred_child_struct_one_with_user_val);

    const auto deferred_child2_struct_one_val =
            def::is_deferred_base_of<deferred_child_struct_one,
                    deferred_child2_struct_one>::value;
    EXPECT_TRUE(deferred_child2_struct_one_val);

    const auto deferred_child2_struct_one_base_val =
            def::is_deferred_base_of<deferred_struct,
                    deferred_child2_struct_one>::value;
    EXPECT_TRUE(deferred_child2_struct_one_base_val);
}

TEST(is_deferred_base_of, two_bases)
{
    const auto deferred_child_struct_two_no_user_val1 =
            def::is_deferred_base_of<deferred_struct,
                    deferred_child_struct_two_no_user>::value;
    EXPECT_FALSE(deferred_child_struct_two_no_user_val1);

    const auto deferred_child_struct_two_no_user_val2 =
            def::is_deferred_base_of<deferred_struct2,
                    deferred_child_struct_two_no_user>::value;
    EXPECT_FALSE(deferred_child_struct_two_no_user_val2);

    const auto deferred_child_struct_two_with_user_val1 =
            def::is_deferred_base_of<deferred_struct,
                    deferred_child_struct_two_with_user>::value;
    EXPECT_TRUE(deferred_child_struct_two_with_user_val1);

    const auto deferred_child_struct_two_with_user_val2 =
            def::is_deferred_base_of<deferred_struct2,
                    deferred_child_struct_two_with_user>::value;
    EXPECT_TRUE(deferred_child_struct_two_with_user_val2);

    const auto deferred_child2_struct_two_with_user_val =
            def::is_deferred_base_of<deferred_child_struct_two_with_user,
                    deferred_child2_struct_two_with_user>::value;
    EXPECT_TRUE(deferred_child2_struct_two_with_user_val);

    const auto deferred_child2_struct_two_with_user_val1 =
            def::is_deferred_base_of<deferred_struct,
                    deferred_child2_struct_two_with_user>::value;
    EXPECT_TRUE(deferred_child2_struct_two_with_user_val1);

    const auto deferred_child2_struct_two_with_user_val2 =
            def::is_deferred_base_of<deferred_struct2,
                    deferred_child2_struct_two_with_user>::value;
    EXPECT_TRUE(deferred_child2_struct_two_with_user_val2);
}

TEST(support_visitor, general)
{
    EXPECT_TRUE(def::support_visitor<right_visitor>::value);
    EXPECT_TRUE(def::support_visitor<const_member_visitor>::value);
    EXPECT_TRUE(def::support_visitor<right_visitor_private>::value);
    EXPECT_FALSE(def::support_visitor<empty_visitor>::value);
}

TEST(support_visitor, detail)
{
    EXPECT_TRUE(def::support_visitor<const_visitor>::value);
    EXPECT_TRUE(def::support_visitor<args_visitor1>::value);
    EXPECT_FALSE(def::support_visitor<args_visitor2>::value);
    EXPECT_TRUE(def::support_visitor<multiple_member_visitor>::value);
    EXPECT_TRUE(def::support_visitor<static_visitor>::value);
    EXPECT_FALSE(def::support_visitor<wrong_visitor_ptr>::value);
    EXPECT_FALSE(def::support_visitor<wrong_visitor_name>::value);
    EXPECT_FALSE(def::support_visitor<wrong_visitor_noarg>::value);
}

TEST(inner_traits, is_deferred_ptr)
{
    struct dummy {};

    EXPECT_FALSE(def::detail::is_deferred_ptr<void>::value);
    EXPECT_FALSE(def::detail::is_deferred_ptr<int>::value);
    EXPECT_FALSE(def::detail::is_deferred_ptr<std::string>::value);
    EXPECT_FALSE(def::detail::is_deferred_ptr<dummy>::value);

    EXPECT_TRUE(def::detail::is_deferred_ptr<
            def::deferred_ptr<void>>::value);
    EXPECT_TRUE(def::detail::is_deferred_ptr<
            def::deferred_ptr<int>>::value);
    EXPECT_TRUE(def::detail::is_deferred_ptr<
            def::deferred_ptr<std::string>>::value);
    EXPECT_TRUE(def::detail::is_deferred_ptr<
            def::deferred_ptr<dummy>>::value);

    EXPECT_TRUE(def::detail::is_deferred_ptr<
            def::root_ptr<void>>::value);
    EXPECT_TRUE(def::detail::is_deferred_ptr<
            def::root_ptr<int>>::value);
    EXPECT_TRUE(def::detail::is_deferred_ptr<
            def::root_ptr<std::string>>::value);
    EXPECT_TRUE(def::detail::is_deferred_ptr<
            def::root_ptr<dummy>>::value);
}

TEST(inner_traits, is_container_of_deferred_ptr)
{
    struct dummy {};

    struct my_container
    {
        using const_iterator = int*;

        const_iterator begin() const
        {
            return nullptr;
        }

        const_iterator end() const
        {
            return nullptr;
        }
    };

    struct my_deferred_container
    {
        using const_iterator = def::deferred_ptr<int>*;

        const_iterator begin() const
        {
            return nullptr;
        }

        const_iterator end() const
        {
            return nullptr;
        }
    };

    using array_dummy = std::array<dummy, 8>;
    using array_deferred = std::array<def::deferred_ptr<dummy>, 8>;

    EXPECT_FALSE(def::detail::is_container_of_deferred_ptr<
            void>::value);
    EXPECT_FALSE(def::detail::is_container_of_deferred_ptr<
            int>::value);
    EXPECT_FALSE(def::detail::is_container_of_deferred_ptr<
            std::string>::value);
    EXPECT_FALSE(def::detail::is_container_of_deferred_ptr<
            dummy>::value);
    EXPECT_FALSE(def::detail::is_container_of_deferred_ptr<
            std::vector<dummy>>::value);
    EXPECT_FALSE(def::detail::is_container_of_deferred_ptr<
            std::list<dummy>>::value);
    EXPECT_FALSE(def::detail::is_container_of_deferred_ptr<
            array_dummy>::value);
    EXPECT_FALSE(def::detail::is_container_of_deferred_ptr<
            dummy[]>::value);
    EXPECT_FALSE(def::detail::is_container_of_deferred_ptr<
            dummy[8]>::value);
    EXPECT_FALSE(def::detail::is_container_of_deferred_ptr<
            my_container>::value);

    EXPECT_TRUE(def::detail::is_container_of_deferred_ptr<
            std::vector<def::deferred_ptr<dummy>>>::value);
    EXPECT_TRUE(def::detail::is_container_of_deferred_ptr<
            std::list<def::deferred_ptr<dummy>>>::value);
    EXPECT_TRUE(def::detail::is_container_of_deferred_ptr<
            array_deferred>::value);
    // no size information to get std::end()
    EXPECT_FALSE(def::detail::is_container_of_deferred_ptr<
            def::deferred_ptr<dummy>[]>::value);
    EXPECT_TRUE(def::detail::is_container_of_deferred_ptr<
            def::deferred_ptr<dummy>[8]>::value);
    EXPECT_TRUE(def::detail::is_container_of_deferred_ptr<
            my_deferred_container>::value);

    EXPECT_TRUE(def::detail::is_container_of_deferred_ptr<
            std::vector<def::root_ptr<dummy>>>::value);
}
