#pragma once

#include <stdarg.h>
#include <mutex>

// time
using u_mu = std::chrono::microseconds;
using u_ms = std::chrono::milliseconds;
using u_s = std::chrono::seconds;

inline auto make_stamp() { return std::chrono::steady_clock::now(); }
template<typename TimeUnit = u_ms> inline auto make_dur(decltype(make_stamp()) ts) 
{ return std::chrono::duration_cast<TimeUnit>(std::chrono::steady_clock::now() - ts).count(); }

// debug
#ifdef NDEBUG
#define LK_PRINTF(...)
#else
#define LK_PRINTF(...) lk_printf(__VA_ARGS__)
#endif

inline void lk_printf(const char* fmt, ...)
{
    static std::mutex mx;
    std::lock_guard<std::mutex> lk(mx);
    va_list va;
    va_start(va, fmt);
    vprintf_s(fmt, va);
    va_end(va);
}