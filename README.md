# CppThreadPool
Using c++11 thread library to implement thread pool

## features
- Typesafe adding new Task for any function signature.
- std::future<R> is returned after added new Task.

## Adding Task function signature

```
template<typename F, typename ... Args>
std::future< typename std::result_of<F(Args...)>::type > 
put(F&& func, Args&& ... args);
```
