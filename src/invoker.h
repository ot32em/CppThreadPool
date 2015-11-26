#pragma once

struct invoker_only_call_once : std::exception {};

template<typename F, typename ... Args>
struct Invoker
{
    Invoker(F&& f, Args&& ... args)
        : f_(std::forward<F>(f))
        , t_(std::forward<Args>(args)...)
        , called_(false)
    {
    }

    auto operator()()
    {
        if (called_) {
            throw invoker_only_call_once();
        }
        called_ = true;
        return expand(std::make_index_sequence<sizeof...(Args)>());
    }

private:
    template<std::size_t ... Index>
    auto expand(std::index_sequence<Index...>)
    {
        return f_( std::move(std::get<Index>(t_))...);
    }

    F f_;
    std::tuple<
        typename std::decay<Args>::type ...
    > t_;
    bool called_ = false;
};

template<typename F, typename ... Args>
auto make_invoker(F&& f, Args&&... args)
{
    return Invoker<F, Args...>(std::forward<F>(f), std::forward<Args>(args)...);
}