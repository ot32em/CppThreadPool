#include "catch.hpp"
#include "threadpool.h"
#include <utility>
#include <iostream>

struct Observer
{
    static const bool verbose_ = false;
    Observer(): moved_times_(0), copied_times_(0), default_created_times_(0){ }
    void moved() {
        if (verbose_) { std::cout << "moved\n"; }
        moved_times_++;
    }
    void copied() {
        if (verbose_) { std::cout << "copied\n"; }
        copied_times_++;
    }
    void created() {
        if (verbose_) { std::cout << "created\n"; }
        default_created_times_++;
    }
private:
    size_t moved_times_;
    size_t copied_times_;
    size_t default_created_times_;
public:
    size_t moved_times() const { return moved_times_; }
    size_t copied_times() const { return copied_times_; }
    size_t created_times() const { return default_created_times_; }
};

struct ObservedObject
{
    ObservedObject(Observer& obs) : obs_(obs)
    {
        obs_.created();
    }
    ObservedObject(const ObservedObject& rhs) : obs_(rhs.obs_)
    {
        obs_.copied();
    }
    ObservedObject(ObservedObject&& rhs) : obs_(rhs.obs_)
    {
        obs_.moved();
    }
    Observer& obs_;
};

TEST_CASE("invoker create, arguments are copied by value", "[invoker]") 
{
    auto funcByValue = [](ObservedObject) {};
    Observer obs;
    ObservedObject obj(obs);
    auto invoker = make_invoker(funcByValue, obj);
    REQUIRE(obs.copied_times() == 1);
    REQUIRE(obs.moved_times() == 0);
}

TEST_CASE("invoker create, arguments are moved to create if std::move", "[invoker]")
{
    auto funcByValue = [](ObservedObject) {};
    Observer obs;
    ObservedObject obj(obs);
    auto invoker = make_invoker(funcByValue, std::move(obj));
    REQUIRE(obs.copied_times() == 0);
    REQUIRE(obs.moved_times() == 1);
}

TEST_CASE("invoker call, arguments are moved to call", "[invoker]") 
{
    auto funcByValue = [](ObservedObject) {};
    Observer obs;
    ObservedObject obj(obs);
    auto invoker = make_invoker(funcByValue, obj);
    auto m = obs.moved_times();
    auto c = obs.copied_times();
    invoker();
    auto m_times = obs.moved_times() - m;
    auto c_times = obs.copied_times() - c;
    REQUIRE(m_times == 1);
    REQUIRE(c_times == 0);
}

TEST_CASE("invoker call, return the right value", "[invoker]")
{
    {
        auto add2 = [](int a, int b) { return a + b; };
        auto invoker = make_invoker(add2, 1, 2);
        REQUIRE(invoker() == add2(1, 2));
    }
    {
        auto concat = [](std::string a, std::string b) { return a + b; };
        auto invoker = make_invoker(concat, "Hello ", "World");
        REQUIRE(invoker() == concat("Hello ", "World"));
    }
}

TEST_CASE("invoker call, actually affected by reference", "[invoker]")
{
    {
        auto inc = [](int& a) { a++; };
        int a = 100;
        auto invoker = make_invoker(inc, std::ref(a));
        invoker();
        REQUIRE(a == 101);
    }
    {
        auto sayto = [](std::string& a) { a += " World"; };
        std::string a = "Hello";
        auto invoker = make_invoker(sayto, std::ref(a));
        invoker();
        REQUIRE(a == "Hello World");
    }
}