#include "threadpool.h"
#include <memory>
#include <iostream>
#include <array>
#include "utility.h"

void pretend_busy(int wait_in_ms, int job_id)
{
    LK_PRINTF("  Thread(%zu) Job %zu starts\n", std::this_thread::get_id(), job_id);
    std::this_thread::sleep_for(u_ms(wait_in_ms));
    LK_PRINTF("  Thread(%zu) Job %zu ends\n", std::this_thread::get_id(), job_id);
}

int add2(int v1, int v2, int wait_in_ms, int job_id)
{
    pretend_busy(wait_in_ms * 100, job_id);
    return v1 + v2;
}

int main()
{
    int primes[] = { 2,3,5,7,11,13,17,19,23,29,31,37 };
    std::vector<std::future<int>> fs;
    ThreadPool pool(5);
    for (int i = 0; i < sizeof primes / sizeof primes[0]; i++)
    {
        fs.push_back(pool.put(add2, i, 100 * primes[i], primes[i], i));
    }
    for (auto& f : fs)
    {
        LK_PRINTF("future get: %d\n", f.get());
    }
    std::cin.ignore();
    pool.abort();
    return EXIT_SUCCESS;
}