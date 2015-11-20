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
        LK_PRINTF("[queue] thread (%zu) check condition: non empty(): %d OR aborted: %d\n", get_id(), !q_.empty(), aborted_);
        if (aborted_) { return false; }
        entry = std::move(q_.front());
        q_.pop();
        return true;
    }
    void abort()
    {
        LK_PRINTF("[queue] thread (%zu) aborting...\n", get_id());
        std::lock_guard<std::mutex> lk(m_);
        aborted_ = true;
        LK_PRINTF("[queue] thread (%zu) aborted, notify all...\n", get_id());
        signal_.notify_all();
    }
    std::condition_variable signal_;
    std::mutex m_;
    std::queue<T> q_;
    bool aborted_;
};

struct Runable
{
    using PtrT = std::unique_ptr<Runable>;
    virtual void operator()() = 0; /* TODO */
};

template <typename R, typename ... Args>
struct Task : Runable
{
    Task(std::function<R(Args...)> f, Args... args)
        : lambda_([f, args...](std::promise<R>& p) { p.set_value(f(args...)); })
    { }

    virtual void operator()()
    {
        lambda_(p_);
    }

private:
    std::function<void(std::promise<R>&)> lambda_;
    std::promise<R> p_;
};

using std::this_thread::get_id;
struct WorkerThread
{
    WorkerThread(ThreadedQueue<Runable::PtrT>& task_queue, bool& aborted)
        :  task_queue_(task_queue), aborted_(aborted)  { }

    void operator()()
    {
        while(!aborted_)
        {
            LK_PRINTF("[Worker] thread (%zu) enter queue::get block\n", get_id());
            Runable::PtrT pTask;
            if (task_queue_.get(pTask))
            {
                (*pTask)();
            }
            LK_PRINTF("[Worker] thread (%zu) out of queue::get block\n", get_id());
        }
        LK_PRINTF("[Worker] thread (%zu) down\n", get_id());
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

    void put(Runable::PtrT pTask)
    {
        task_queue_.put(std::move(pTask));
    }

    void abort()
    {
        LK_PRINTF("[Pool] aborting  %zu threads\n", thread_pool_.size());
        aborted_ = true;
        task_queue_.abort();
        for (auto& t : thread_pool_)
        {
            if (t.joinable())
            {
                LK_PRINTF("[Pool] join thread %zu\n", t.get_id());
                t.join();
            }
            else
            {
                LK_PRINTF("[Pool] thread %zu is not joinable\n", t.get_id());
            }
        }
        LK_PRINTF("[Pool] aborted...\n");
    }
private:
    std::vector<std::thread> thread_pool_;
    ThreadedQueue<Runable::PtrT> task_queue_;
    bool aborted_;
};