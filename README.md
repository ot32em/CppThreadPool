# CppThreadPool

## Features
- Task accepts any function signatures with **Typesafe**.
- Result is **promised** as `std::future<R>` while added Task into pool.
- Header-only library. Just include "threadpool.h".

### Task adding function signature

```cpp
template<typename F, typename ... Args>
std::future< typename std::result_of<F(Args...)>::type > 
ThreadPool::put(F&& func, Args&& ... args);
```

## Usage

Your functions
```cpp
int add2(int, int);
Report make_report(std::string);
```

Dispatch it
```cpp
size_t thread_count = 5;
ThreadPool pool(thread_count);
std::future<int> answer = pool.put(add2, 1, 2);
std::future<Report> report = pool.put(make_report, "Author Name");

answer.get();
report.get();
```
