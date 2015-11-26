#include <thread>
#include <memory>
#include <vector>
#include <queue>
#include <condition_variable>
#include <future>
#include <functional>
#include "utility.h"

template<typename T>
struct ThreadedQueue
{
    ThreadedQueue() : aborted_(false) {};
    bool put(T&& entry)
    {
        std::lock_guard<std::mutex> lk(m_);
        if (aborted_) { return false; }
        q_.push(std::move(entry));
        signal_.notify_one();
        return true;
    }
    bool get(T& entry)
    {
        std::unique_lock<std::mutex> lk(m_);
        signal_.wait(lk, [this]() { return !q_.empty() || aborted_; });
        //LK_PRINTF("[queue] thread (%zu) check condition: non empty(): %d OR aborted: %d\n", get_id(), !q_.empty(), aborted_);
        if (aborted_) { return false; }
        entry = std::move(q_.front());
        q_.pop();
        return true;
    }
    void abort()
    {
        //LK_PRINTF("[queue] thread (%zu) aborting...\n", get_id());
        std::lock_guard<std::mutex> lk(m_);
        aborted_ = true;
        //LK_PRINTF("[queue] thread (%zu) aborted, notify all...\n", get_id());
        signal_.notify_all();
    }
    std::condition_variable signal_;
    std::mutex m_;
    std::queue<T> q_;
    bool aborted_;
};

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

struct Runable
{
    using PtrT = std::unique_ptr<Runable>;
    virtual void operator()() = 0; /* TODO */
    virtual ~Runable() {}
};

template<typename R>
struct PromiseInvoker : Runable
{
    template<typename F, typename ... Args>
    PromiseInvoker(F&& f, Args&& ... args)
        : f_(Invoker<F, Args...>(std::forward<F>(f), std::forward<Args>(args)...))
    { }

    std::future<R> get_future() { return p_.get_future(); }

    virtual void operator()()
    {
        try {
            p_.set_value(f_());
        }
        catch (...)
        {
            p_.set_exception(std::current_exception());
        }
    }

private:
    std::promise<R> p_;
    std::function<R()> f_;
};

struct WorkerThread
{
    WorkerThread(ThreadedQueue<Runable::PtrT>& task_queue, bool& aborted)
        :  task_queue_(task_queue), aborted_(aborted)  { }

    void operator()()
    {
        while(!aborted_)
        {
            //LK_PRINTF("[Worker] thread (%zu) enter queue::get block\n", get_id());
            Runable::PtrT pTask;
            if (task_queue_.get(pTask))
            {
                (*pTask)();
            }
            //LK_PRINTF("[Worker] thread (%zu) out of queue::get block\n", get_id());
        }
        //LK_PRINTF("[Worker] thread (%zu) down\n", get_id());
    }

private:
    ThreadedQueue<Runable::PtrT>& task_queue_;
    bool& aborted_;
};
struct ThreadPool
{
    ThreadPool(size_t thread_count)
        : aborted_(false)
    {
        thread_pool_.reserve(thread_count);
        for(size_t i = 0 ; i != thread_count; i++)
        {
            thread_pool_.push_back(std::thread(WorkerThread(task_queue_, aborted_)));
        }
    }

    ~ThreadPool()
    {
        abort();
    }

    template<typename F, typename ... Args>
    std::future< typename std::result_of<F(Args...)>::type > put(F&& func, Args&& ... args)
    {
        using R = typename std::result_of<F(Args...)>::type;

        std::unique_ptr<PromiseInvoker<R>> p(new PromiseInvoker<R>(std::forward<F>(func), std::forward<Args>(args)...));
        std::future<R> fu = p->get_future();
        task_queue_.put(std::move(p));
        return std::move(fu);
    }

    void abort()
    {
        //LK_PRINTF("[Pool] aborting  %zu threads\n", thread_pool_.size());
        aborted_ = true;
        task_queue_.abort();
        for (auto& t : thread_pool_)
        {
            if (t.joinable())
            {
                //LK_PRINTF("[Pool] join thread %zu\n", t.get_id());
                t.join();
            }
            else
            {
                //LK_PRINTF("[Pool] thread %zu is not joinable\n", t.get_id());
            }
        }
        //LK_PRINTF("[Pool] aborted...\n");
    }
private:
    std::vector<std::thread> thread_pool_;
    ThreadedQueue<Runable::PtrT> task_queue_;
    bool aborted_;
};