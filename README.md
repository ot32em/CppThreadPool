# CppThreadPool
Using c++11 thread library to implement thread pool

## Features
- Typesafe adding new Task for any function signature.
- std::future<R> is returned after added new Task.

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
size_t thread_count = 5;
```

Dispatch it
```cpp
ThreadPool pool(thread_count);
std::future<int> answer = pool.put(add2, 1, 2);
std::future<Report> report = pool.put(make_report, "Author Name");

answer.get();
report.get();
```
