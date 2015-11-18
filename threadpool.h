#include <thread>
#include <memory>
#include <vector>
#include <queue>

template<typename T>
struct ThreadedQueue
{
    void put(T){ /* TODO */ }
    T get(){ return T(); /* TODO */  }
    std::condition_variable hasEntry;
    std::queue<T> q_;
};

struct Task
{
    using PtrT = std::unique_ptr<Task>;
    virtual void run() = 0; /* TODO */
};

struct WorkerThread
{
    WorkerThread( ThreadedQueue<Task::PtrT>& task_queue, bool& aborted)
        : task_queue_(task_queue), aborted_(aborted) {}
        
    void operator()()
    {
        while(true && !aborted_)
        {
            Task::PtrT pTask = std::move(task_queue_.get());
            pTask->run();
        }
    }
    
    ThreadedQueue<Task::PtrT>& task_queue_;
    bool& aborted_;
};

struct ThreadPool
{
    ThreadPool(size_t thread_count)
    {
        thread_pool_.reserve(thread_count);
        for(size_t i = 0 ; i != thread_count; i++)
        {
            thread_pool_.push_back(std::thread(WorkerThread(task_queue_, aborted_)));
        }
    }
    ~ThreadPool()
    {
        aborted_ = true;
        for(auto& t: thread_pool_)
        {
            t.join();
        }
    }
    
    void put(Task::PtrT pTask)
    {
        task_queue_.put(std::move(pTask));
    }
    
    void abort()
    {
        aborted_ = true;
    }
    
private:
    std::vector<std::thread> thread_pool_;
    ThreadedQueue<Task::PtrT> task_queue_;
    bool aborted_;
};
