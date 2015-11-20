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
    pretend_busy(wait_in_ms * 1, job_id);
    return v1 + v2;
}

int main()
{
    int primes[] = { 2,3,5,7,11,13,17,19,23,29,31,37 };
    ThreadPool pool(5);
    for (int i = 0; i < 1; i++)
    {
        /* TODO: auto deduce types of passing function */
        pool.put(std::make_unique<Task<int, int, int, int, int>>(add2, 1, 2, primes[i], i));
    }
    std::cin.ignore();
    pool.abort();
    std::cin.ignore();
    return EXIT_SUCCESS;
}