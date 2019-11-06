#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <array>
#include <memory>
#include <type_traits>

#include "deferred/simple_allocator"
#include "deferred/deferred_heap"

namespace
{

class simple_struct;

class simple_observer
{
public:
    using array_t = std::array<long, 4>;

    simple_observer()
    {}

    ~simple_observer() = default;

    MOCK_METHOD1(allocated, void(std::size_t));
    MOCK_METHOD1(deallocated, void(std::size_t));

    MOCK_METHOD0(allocator_constructed_allocator, void());
    MOCK_METHOD0(allocator_constructed_header, void());
    MOCK_METHOD0(allocator_constructed_object, void());
    MOCK_METHOD0(allocator_constructed_other, void());
    MOCK_METHOD0(allocator_destroyed_allocator, void());
    MOCK_METHOD0(allocator_destroyed_header, void());
    MOCK_METHOD0(allocator_destroyed_object, void());
    MOCK_METHOD0(allocator_destroyed_other, void());

    MOCK_METHOD0(created0, void());
    MOCK_METHOD1(created1, void(long));
    MOCK_METHOD1(created2, void(array_t));
    MOCK_METHOD1(created_copy, void(array_t));
    MOCK_METHOD1(destroyed, void(array_t));
};

template <typename T>
struct is_simple_allocator
{
    static constexpr bool value = false;
};

template <typename T>
class simple_std_allocator
{
public:
    using value_type = T;

    simple_std_allocator() noexcept
    : m_observer{nullptr}
    {}

    simple_std_allocator(simple_observer* o) noexcept
    : m_observer{o}
    {}

    template<typename U>
    simple_std_allocator(const simple_std_allocator<U>& other) noexcept
    : m_observer{other.m_observer}
    {}

    value_type*
    allocate(std::size_t n)
    {
        if (m_observer)
            m_observer->allocated(n);
        return static_cast<value_type*>(::operator new(n * sizeof(value_type)));
    }

    void
    deallocate(value_type* p, std::size_t n) noexcept
    {
        ::operator delete(p);
        if (m_observer)
            m_observer->deallocated(n);
    }

    template <typename U, typename... Args>
    void
    construct(U* p, Args&&... args)
    {
        if (m_observer)
        {
            using type = std::decay_t<U>;
            if constexpr (std::is_same_v<type,
                    def::detail::memory_chunk_header>)
                m_observer->allocator_constructed_header();
            else if constexpr (std::is_same_v<type, simple_struct>)
                m_observer->allocator_constructed_object();
            else if constexpr (is_simple_allocator<type>::value)
                m_observer->allocator_constructed_allocator();
            else
                m_observer->allocator_constructed_other();
        }
        ::new(p) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void
    destroy(U* p) noexcept
    {
        p->~U();
        if (m_observer)
        {
            using type = std::decay_t<U>;
            if constexpr (std::is_same_v<type,
                    def::detail::memory_chunk_header>)
                m_observer->allocator_destroyed_header();
            else if constexpr (std::is_same_v<type, simple_struct>)
                m_observer->allocator_destroyed_object();
            else if constexpr (is_simple_allocator<type>::value)
                m_observer->allocator_destroyed_allocator();
            else
                m_observer->allocator_destroyed_other();
        }
    }

private:
    template <typename U>
    friend class simple_std_allocator;

    simple_observer* m_observer;
};

template<typename T, typename U>
bool
operator==(const simple_std_allocator<T>&,
           const simple_std_allocator<U>&) noexcept
{
    return true;
}

template<typename T, typename U>
bool
operator!=(const simple_std_allocator<T>& x,
           const simple_std_allocator<U>& y) noexcept
{
    return !(x == y);
}

template <typename T>
struct is_simple_allocator<simple_std_allocator<T>>
{
    static constexpr bool value = true;
};

class simple_struct
{
public:
    explicit simple_struct()
    : m_observer{nullptr}
    , m_i{0, 0, 0, 0}
    {}

    explicit simple_struct(simple_observer* o)
    : m_observer{o}
    , m_i{0, 0, 0, 0}
    {
        o->created0();
    }

    explicit simple_struct(simple_observer* o, long value)
    : m_observer{o}
    , m_i{value, value, value, value}
    {
        o->created1(value);
    }

    explicit simple_struct(simple_observer* o,
            long val1, long val2, long val3, long val4)
    : m_observer{o}
    , m_i{val1, val2, val3, val4}
    {
        o->created2(m_i);
    }

    simple_struct(const simple_struct& other)
    : m_observer{other.m_observer}
    , m_i{other.m_i}
    {
        if (m_observer)
            m_observer->created_copy(m_i);
    }

    ~simple_struct()
    {
        if (m_observer)
            m_observer->destroyed(m_i);
    }

private:
    simple_observer* m_observer;
    std::array<long, 4> m_i;
};

class simple_allocator : public ::testing::Test
{
public:
    simple_allocator()
    : m_heap{}
    , m_deferred_allocator{m_heap.get_simple_allocator()}
    {}

protected:
    void SetUp() override
    {
        EXPECT_EQ(0, m_heap.get_memory_chunks_number());
        EXPECT_EQ(0, m_heap.get_root_memory_chunks_number());
        EXPECT_EQ(0, m_heap.get_objects_number());
        EXPECT_EQ(0, m_heap.get_root_objects_number());
        EXPECT_EQ(0, m_heap.get_total_bytes());
    }

    void TearDown() override
    {
        EXPECT_EQ(0, m_heap.get_root_memory_chunks_number());
        EXPECT_EQ(0, m_heap.get_root_objects_number());

        const auto chunks_number = m_heap.get_memory_chunks_number();
        const auto objects_number = m_heap.get_objects_number();
        const auto bytes_number = m_heap.get_total_bytes();
        const auto release_stats = m_heap.release_unreachable();

        EXPECT_EQ(chunks_number, release_stats.chunks);
        EXPECT_EQ(objects_number, release_stats.objects);
        EXPECT_EQ(bytes_number, release_stats.bytes);

        EXPECT_EQ(0, m_heap.get_memory_chunks_number());
        EXPECT_EQ(0, m_heap.get_root_memory_chunks_number());
        EXPECT_EQ(0, m_heap.get_objects_number());
        EXPECT_EQ(0, m_heap.get_root_objects_number());
        EXPECT_EQ(0, m_heap.get_total_bytes());
    }

public:
    simple_observer m_observer;
    def::deferred_heap m_heap;
    def::simple_allocator m_deferred_allocator;
};

using testing::_;

}


TEST_F(simple_allocator, smoke)
{
    EXPECT_TRUE(true);
}

TEST_F(simple_allocator, make_std_one)
{
    const auto int_one_obj = m_deferred_allocator.make_deferred<int>(2354);
    ASSERT_TRUE(int_one_obj);
    const auto is_int_type = std::is_same_v<int&, decltype(*int_one_obj)>;
    EXPECT_TRUE(is_int_type);
    EXPECT_EQ(2354, *int_one_obj);

    const auto string_one_obj = m_deferred_allocator
            .make_deferred<std::string>();
    ASSERT_TRUE(string_one_obj);
    const auto is_string_type =
            std::is_same_v<std::string&, decltype(*string_one_obj)>;
    EXPECT_TRUE(is_string_type);
    EXPECT_EQ("", *string_one_obj);

    const auto string_one_obj2 = m_deferred_allocator
            .make_deferred<std::string>("rsrs4");
    ASSERT_TRUE(string_one_obj2);
    const auto is_string_type2 =
            std::is_same_v<std::string&, decltype(*string_one_obj2)>;
    EXPECT_TRUE(is_string_type2);
    EXPECT_EQ("rsrs4", *string_one_obj2);

    EXPECT_CALL(m_observer, created0()).Times(1);
    const auto one_obj_def = m_deferred_allocator
            .make_deferred<simple_struct>(&m_observer);
    ASSERT_TRUE(one_obj_def);
    const auto is_obj_type =
            std::is_same_v<simple_struct&, decltype(*one_obj_def)>;
    EXPECT_TRUE(is_obj_type);

    EXPECT_CALL(m_observer, created1(567)).Times(1);
    const auto one_obj_def2 = m_deferred_allocator
            .make_deferred<simple_struct>(&m_observer, 567);
    ASSERT_TRUE(one_obj_def2);
    const auto is_obj_type2 =
            std::is_same_v<simple_struct&, decltype(*one_obj_def2)>;
    EXPECT_TRUE(is_obj_type2);

    const auto arr_val3 = std::array<long, 4>{0, 2, 4, 8};
    EXPECT_CALL(m_observer, created2(arr_val3)).Times(1);
    const auto one_obj_def3 = m_deferred_allocator
            .make_deferred<simple_struct>(&m_observer, 0, 2, 4, 8);
    ASSERT_TRUE(one_obj_def3);
    const auto is_obj_type3 =
            std::is_same_v<simple_struct&, decltype(*one_obj_def3)>;
    EXPECT_TRUE(is_obj_type3);

    EXPECT_CALL(m_observer, destroyed(_)).Times(3);
}

TEST_F(simple_allocator, make_std_arr_one)
{
    const auto int_one_obj = m_deferred_allocator.make_deferred<int[]>(1, 34);
    ASSERT_TRUE(int_one_obj);
    const auto is_ints_type = std::is_same_v<
            std::add_lvalue_reference_t<int[]>, decltype(*int_one_obj)>;
    const auto is_int_arr_type = std::is_same_v<
            int&, decltype(int_one_obj[0])>;
    EXPECT_TRUE(is_ints_type);
    EXPECT_TRUE(is_int_arr_type);
    EXPECT_EQ(34, int_one_obj[0]);

    const auto string_one_obj = m_deferred_allocator
            .make_deferred<std::string[]>(1);
    ASSERT_TRUE(string_one_obj);
    const auto is_string_type = std::is_same_v<
            std::add_lvalue_reference_t<std::string[]>,
            decltype(*string_one_obj)>;
    const auto is_string_arr_type = std::is_same_v<
            std::string&, decltype(string_one_obj[0])>;
    EXPECT_TRUE(is_string_type);
    EXPECT_TRUE(is_string_arr_type);
    EXPECT_EQ(std::string{""}, string_one_obj[0]);

    const auto string_one_obj2 = m_deferred_allocator
            .make_deferred<std::string[]>(1, "rsrs5");
    ASSERT_TRUE(string_one_obj2);
    const auto is_string_type2 = std::is_same_v<
            std::add_lvalue_reference_t<std::string[]>,
            decltype(*string_one_obj2)>;
    const auto is_string_arr_type2 = std::is_same_v<
            std::string&, decltype(string_one_obj2[0])>;
    EXPECT_TRUE(is_string_type2);
    EXPECT_TRUE(is_string_arr_type2);
    EXPECT_EQ(std::string{"rsrs5"}, string_one_obj2[0]);

    EXPECT_CALL(m_observer, created1(258)).Times(1);
    const auto copy_object = simple_struct{&m_observer, 258};
    const auto array_val = std::array<long, 4>{258, 258, 258, 258};
    EXPECT_CALL(m_observer, created_copy(array_val)).Times(1);
    const auto one_obj_def = m_deferred_allocator
            .make_deferred<simple_struct[]>(1, copy_object);
    ASSERT_TRUE(one_obj_def);
    const auto is_obj_type = std::is_same_v<
            std::add_lvalue_reference_t<simple_struct[]>,
            decltype(*one_obj_def)>;
    const auto is_obj_arr_type = std::is_same_v<
            simple_struct&,
            decltype(one_obj_def[0])>;
    EXPECT_TRUE(is_obj_type);
    EXPECT_TRUE(is_obj_arr_type);

    EXPECT_CALL(m_observer, destroyed(array_val)).Times(2);
}

TEST_F(simple_allocator, make_std_arr_two)
{
    const auto int_two_obj = m_deferred_allocator.make_deferred<int[]>(2, 34);
    ASSERT_TRUE(int_two_obj);
    const auto is_ints_type = std::is_same_v<
            std::add_lvalue_reference_t<int[]>, decltype(*int_two_obj)>;
    const auto is_int_arr_type = std::is_same_v<
            int&, decltype(int_two_obj[0])>;
    EXPECT_TRUE(is_ints_type);
    EXPECT_TRUE(is_int_arr_type);
    EXPECT_EQ(34, int_two_obj[0]);
    EXPECT_EQ(34, int_two_obj[1]);

    const auto string_two_obj = m_deferred_allocator
            .make_deferred<std::string[]>(2);
    ASSERT_TRUE(string_two_obj);
    const auto is_string_type = std::is_same_v<
            std::add_lvalue_reference_t<std::string[]>,
            decltype(*string_two_obj)>;
    const auto is_string_arr_type = std::is_same_v<
            std::string&, decltype(string_two_obj[0])>;
    EXPECT_TRUE(is_string_type);
    EXPECT_TRUE(is_string_arr_type);
    EXPECT_EQ(std::string{""}, string_two_obj[0]);
    EXPECT_EQ(std::string{""}, string_two_obj[1]);

    const auto string_two_obj2 = m_deferred_allocator
            .make_deferred<std::string[]>(2, "rsrs5");
    ASSERT_TRUE(string_two_obj2);
    const auto is_string_type2 = std::is_same_v<
            std::add_lvalue_reference_t<std::string[]>,
            decltype(*string_two_obj2)>;
    const auto is_string_arr_type2 = std::is_same_v<
            std::string&, decltype(string_two_obj2[0])>;
    EXPECT_TRUE(is_string_type2);
    EXPECT_TRUE(is_string_arr_type2);
    EXPECT_EQ(std::string{"rsrs5"}, string_two_obj2[0]);
    EXPECT_EQ(std::string{"rsrs5"}, string_two_obj2[1]);

    EXPECT_CALL(m_observer, created1(258)).Times(1);
    const auto copy_object = simple_struct{&m_observer, 258};
    const auto array_val = std::array<long, 4>{258, 258, 258, 258};
    EXPECT_CALL(m_observer, created_copy(array_val)).Times(2);
    const auto two_obj_def = m_deferred_allocator
            .make_deferred<simple_struct[]>(2, copy_object);
    ASSERT_TRUE(two_obj_def);
    const auto is_obj_type = std::is_same_v<
            std::add_lvalue_reference_t<simple_struct[]>,
            decltype(*two_obj_def)>;
    const auto is_obj_arr_type = std::is_same_v<
            simple_struct&,
            decltype(two_obj_def[0])>;
    EXPECT_TRUE(is_obj_type);
    EXPECT_TRUE(is_obj_arr_type);

    EXPECT_CALL(m_observer, destroyed(array_val)).Times(3);
}

TEST_F(simple_allocator, allocate_custom_one)
{
    const auto int_allocator = simple_std_allocator<int>{&m_observer};

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_other()).Times(1);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto int_one_obj = m_deferred_allocator
            .allocate_deferred<int>(int_allocator, 2394);
    ASSERT_TRUE(int_one_obj);
    const auto is_int_type = std::is_same_v<int&, decltype(*int_one_obj)>;
    EXPECT_TRUE(is_int_type);
    EXPECT_EQ(2394, *int_one_obj);

    const auto string_allocator =
            simple_std_allocator<std::string>{&m_observer};

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_other()).Times(1);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto string_one_obj = m_deferred_allocator
            .allocate_deferred<std::string>(string_allocator);
    ASSERT_TRUE(string_one_obj);
    const auto is_string_type =
            std::is_same_v<std::string&, decltype(*string_one_obj)>;
    EXPECT_TRUE(is_string_type);
    EXPECT_EQ("", *string_one_obj);

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_other()).Times(1);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto string_one_obj2 = m_deferred_allocator
            .allocate_deferred<std::string>(string_allocator, "rs469");
    ASSERT_TRUE(string_one_obj2);
    const auto is_string_type2 =
            std::is_same_v<std::string&, decltype(*string_one_obj2)>;
    EXPECT_TRUE(is_string_type2);
    EXPECT_EQ("rs469", *string_one_obj2);

    const auto custom_struct_allocator =
            simple_std_allocator<simple_struct>{&m_observer};

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, created0()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_object()).Times(1);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto one_obj_def = m_deferred_allocator
            .allocate_deferred<simple_struct>(custom_struct_allocator,
                                              &m_observer);
    ASSERT_TRUE(one_obj_def);
    const auto is_obj_type =
            std::is_same_v<simple_struct&, decltype(*one_obj_def)>;
    EXPECT_TRUE(is_obj_type);

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, created1(132)).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_object()).Times(1);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto one_obj_def2 = m_deferred_allocator
            .allocate_deferred<simple_struct>(custom_struct_allocator,
                                              &m_observer, 132);
    ASSERT_TRUE(one_obj_def2);
    const auto is_obj_type2 =
            std::is_same_v<simple_struct&, decltype(*one_obj_def2)>;
    EXPECT_TRUE(is_obj_type2);

    const auto arr_val3 = std::array<long, 4>{7, 2, 4, 8};
    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, created2(arr_val3)).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_object()).Times(1);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto one_obj_def3 = m_deferred_allocator
            .allocate_deferred<simple_struct>(custom_struct_allocator,
                                              &m_observer, 7, 2, 4, 8);
    ASSERT_TRUE(one_obj_def3);
    const auto is_obj_type3 =
            std::is_same_v<simple_struct&, decltype(*one_obj_def3)>;
    EXPECT_TRUE(is_obj_type3);

    EXPECT_CALL(m_observer, deallocated(_)).Times(6);
    EXPECT_CALL(m_observer, allocator_destroyed_header()).Times(6);
    EXPECT_CALL(m_observer, allocator_destroyed_allocator()).Times(6);
    EXPECT_CALL(m_observer, allocator_destroyed_other()).Times(3);
    EXPECT_CALL(m_observer, allocator_destroyed_object()).Times(3);
    EXPECT_CALL(m_observer, destroyed(_)).Times(3);
}

TEST_F(simple_allocator, allocate_custom_arr_one)
{
    const auto int_allocator = simple_std_allocator<int>{&m_observer};

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_other()).Times(1);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto int_one_obj = m_deferred_allocator
            .allocate_deferred<int[]>(int_allocator, 1, 452);
    ASSERT_TRUE(int_one_obj);
    const auto is_int_type = std::is_same_v<
            std::add_lvalue_reference_t<int[]>, decltype(*int_one_obj)>;
    const auto is_int_arr_type = std::is_same_v<
            int&, decltype(int_one_obj[0])>;
    EXPECT_TRUE(is_int_type);
    EXPECT_TRUE(is_int_arr_type);
    EXPECT_EQ(452, int_one_obj[0]);

    const auto string_allocator =
            simple_std_allocator<std::string>{&m_observer};

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_other()).Times(1);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto string_one_obj = m_deferred_allocator
            .allocate_deferred<std::string[]>(string_allocator, 1);
    ASSERT_TRUE(string_one_obj);
    const auto is_string_type = std::is_same_v<
            std::add_lvalue_reference_t<std::string[]>,
            decltype(*string_one_obj)>;
    const auto is_string_arr_type =
            std::is_same_v<std::string&, decltype(string_one_obj[0])>;
    EXPECT_TRUE(is_string_type);
    EXPECT_TRUE(is_string_arr_type);
    EXPECT_EQ("", string_one_obj[0]);

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_other()).Times(1);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto string_one_obj2 = m_deferred_allocator
            .allocate_deferred<std::string[]>(string_allocator, 1, "edui");
    ASSERT_TRUE(string_one_obj2);
    const auto is_string_type2 = std::is_same_v<
            std::add_lvalue_reference_t<std::string[]>,
            decltype(*string_one_obj2)>;
    const auto is_string_arr_type2 =
            std::is_same_v<std::string&, decltype(string_one_obj2[0])>;
    EXPECT_TRUE(is_string_type2);
    EXPECT_TRUE(is_string_arr_type2);
    EXPECT_EQ("edui", string_one_obj2[0]);

    const auto custom_struct_allocator =
            simple_std_allocator<simple_struct>{&m_observer};

    EXPECT_CALL(m_observer, created1(935)).Times(1);
    const auto copy_object = simple_struct{&m_observer, 935};
    const auto array_val = std::array<long, 4>{935, 935, 935, 935};

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, created_copy(array_val)).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_object()).Times(1);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto one_obj_def = m_deferred_allocator
            .allocate_deferred<simple_struct[]>(custom_struct_allocator, 1,
                                                copy_object);
    ASSERT_TRUE(one_obj_def);
    const auto is_obj_type = std::is_same_v<
            std::add_lvalue_reference_t<simple_struct[]>,
            decltype(*one_obj_def)>;
    const auto is_obj_arr_type =
            std::is_same_v<simple_struct&, decltype(one_obj_def[0])>;
    EXPECT_TRUE(is_obj_type);
    EXPECT_TRUE(is_obj_arr_type);

    EXPECT_CALL(m_observer, deallocated(_)).Times(4);
    EXPECT_CALL(m_observer, allocator_destroyed_header()).Times(4);
    EXPECT_CALL(m_observer, allocator_destroyed_allocator()).Times(4);
    EXPECT_CALL(m_observer, allocator_destroyed_other()).Times(3);
    EXPECT_CALL(m_observer, allocator_destroyed_object()).Times(1);
    EXPECT_CALL(m_observer, destroyed(array_val)).Times(2);
}

TEST_F(simple_allocator, allocate_custom_arr_two)
{
    const auto int_allocator = simple_std_allocator<int>{&m_observer};

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_other()).Times(2);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto int_one_obj = m_deferred_allocator
            .allocate_deferred<int[]>(int_allocator, 2, 152);
    ASSERT_TRUE(int_one_obj);
    const auto is_int_type = std::is_same_v<
            std::add_lvalue_reference_t<int[]>, decltype(*int_one_obj)>;
    const auto is_int_arr_type = std::is_same_v<
            int&, decltype(int_one_obj[0])>;
    EXPECT_TRUE(is_int_type);
    EXPECT_TRUE(is_int_arr_type);
    EXPECT_EQ(152, int_one_obj[0]);
    EXPECT_EQ(152, int_one_obj[1]);

    const auto string_allocator =
            simple_std_allocator<std::string>{&m_observer};

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_other()).Times(2);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto string_one_obj = m_deferred_allocator
            .allocate_deferred<std::string[]>(string_allocator, 2);
    ASSERT_TRUE(string_one_obj);
    const auto is_string_type = std::is_same_v<
            std::add_lvalue_reference_t<std::string[]>,
            decltype(*string_one_obj)>;
    const auto is_string_arr_type =
            std::is_same_v<std::string&, decltype(string_one_obj[0])>;
    EXPECT_TRUE(is_string_type);
    EXPECT_TRUE(is_string_arr_type);
    EXPECT_EQ("", string_one_obj[0]);
    EXPECT_EQ("", string_one_obj[1]);

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_other()).Times(2);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto string_one_obj2 = m_deferred_allocator
            .allocate_deferred<std::string[]>(string_allocator, 2, "trui");
    ASSERT_TRUE(string_one_obj2);
    const auto is_string_type2 = std::is_same_v<
            std::add_lvalue_reference_t<std::string[]>,
            decltype(*string_one_obj2)>;
    const auto is_string_arr_type2 =
            std::is_same_v<std::string&, decltype(string_one_obj2[0])>;
    EXPECT_TRUE(is_string_type2);
    EXPECT_TRUE(is_string_arr_type2);
    EXPECT_EQ("trui", string_one_obj2[0]);
    EXPECT_EQ("trui", string_one_obj2[1]);

    const auto custom_struct_allocator =
            simple_std_allocator<simple_struct>{&m_observer};

    EXPECT_CALL(m_observer, created1(915)).Times(1);
    const auto copy_object = simple_struct{&m_observer, 915};
    const auto array_val = std::array<long, 4>{915, 915, 915, 915};

    EXPECT_CALL(m_observer, allocator_constructed_header()).Times(1);
    EXPECT_CALL(m_observer, allocator_constructed_allocator()).Times(1);
    EXPECT_CALL(m_observer, created_copy(array_val)).Times(2);
    EXPECT_CALL(m_observer, allocator_constructed_object()).Times(2);
    EXPECT_CALL(m_observer, allocated(_)).Times(1);
    const auto one_obj_def = m_deferred_allocator
            .allocate_deferred<simple_struct[]>(custom_struct_allocator, 2,
                                                copy_object);
    ASSERT_TRUE(one_obj_def);
    const auto is_obj_type = std::is_same_v<
            std::add_lvalue_reference_t<simple_struct[]>,
            decltype(*one_obj_def)>;
    const auto is_obj_arr_type =
            std::is_same_v<simple_struct&, decltype(one_obj_def[0])>;
    EXPECT_TRUE(is_obj_type);
    EXPECT_TRUE(is_obj_arr_type);

    EXPECT_CALL(m_observer, deallocated(_)).Times(4);
    EXPECT_CALL(m_observer, allocator_destroyed_header()).Times(4);
    EXPECT_CALL(m_observer, allocator_destroyed_allocator()).Times(4);
    EXPECT_CALL(m_observer, allocator_destroyed_other()).Times(6);
    EXPECT_CALL(m_observer, allocator_destroyed_object()).Times(2);
    EXPECT_CALL(m_observer, destroyed(array_val)).Times(3);
}
