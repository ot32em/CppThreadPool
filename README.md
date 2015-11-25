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

 - 0) Your own functions
```cpp
int add2(int, int);
Report make_report(std::string);
```
 - 1) Assign as task to pool.
```cpp
size_t thread_count = 5;
ThreadPool pool(thread_count);
std::future<int> promised_answer = pool.put(add2, 1, 2);
std::future<Report> promised_report = pool.put(make_report, "Author Name");
```
 - 2) Sync the result from threads if needed
```cpp
int answer = promised_answer.get();
Report report = promised_report.get();
```
