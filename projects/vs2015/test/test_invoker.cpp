#include "catch.hpp"
#include "threadpool.h"
#include <utility>

#define ASSERT_TUPLE_TYPE(f_, arg_, type_) std::is_same<std::tuple_element<0, decltype(make_invoker(f_, arg_))::TupleT>::type, type_>::value == true

TEST_CASE("invoker tuple type (builtin type)", "[invoker][type]") {
    auto func = [](int) {};
    REQUIRE(ASSERT_TUPLE_TYPE(func, int(1), int));                          // pvalue

    int var = 1;
    REQUIRE(ASSERT_TUPLE_TYPE(func, var, int));                             // lvalue
    REQUIRE(ASSERT_TUPLE_TYPE(func, std::move(var), int));                  // xvalue
}

TEST_CASE("invoker tuple type (dynamic type)", "[invoker][type]") {
    auto func = [](std::string) {};
    REQUIRE(ASSERT_TUPLE_TYPE(func, std::string("Hello"), std::string));    // pvalue

    std::string var = "Hello";  
    REQUIRE(ASSERT_TUPLE_TYPE(func, var, std::string));                     // lvalue
    REQUIRE(ASSERT_TUPLE_TYPE(func, std::move(var), std::string));          // xvalue
}

TEST_CASE("invoker tuple type (c string type)", "[invoker][type]") {
    auto func = [](std::string) {};
    REQUIRE(ASSERT_TUPLE_TYPE(func, "Hello", const char*));         // pvalue

    const char var[] = "Hello";
    REQUIRE(ASSERT_TUPLE_TYPE(func, var, const char*));             // lvalue
    REQUIRE(ASSERT_TUPLE_TYPE(func, std::move(var), const char*));  // xvalue
}

