//
// Created by csq on 3/12/23.
//
#include <iostream>
#include <atomic>
#include <list>

#include "utils/thread_pool.h"
#include "algorithm/parallel_accumulate.h"
#include "algorithm/quicksort.h"
#include "gtest/gtest.h"

// helper function to launch multiple threads
template <typename... Args>
void LaunchParallelTest(uint64_t num_threads, Args &&...args) {
    std::vector<std::thread> thread_group;

    // Launch a group of threads
    for (uint64_t thread_itr = 0; thread_itr < num_threads; ++thread_itr) {
        thread_group.push_back(std::thread(args..., thread_itr));
    }

    // Join the threads with the main thread
    for (uint64_t thread_itr = 0; thread_itr < num_threads; ++thread_itr) {
        thread_group[thread_itr].join();
    }
}

std::atomic<int> n = 0;

void increase(int m) {
    for (int i = 0; i < m; ++i) {
        n.fetch_add(1, std::memory_order_relaxed);
    }
}

TEST(ThreadPoolTest, IncreaseTest) {
    thread_pool pool;
    
    std::vector<std::future<void>> futures(10);
    for (long i = 0; i < 10; ++i) {
        futures[i] = pool.submit(increase, 1000);
    }
    for (long i = 0; i < 10; ++i) {
        futures[i].get();
    }
    EXPECT_EQ(n, 10000);
}


TEST(ThreadPoolTest, AccumulateTest) {
    std::vector<int> nums(10000);
    std::iota(nums.begin(), nums.end(), 0);

    int res1 = std::accumulate(nums.begin(), nums.end(), 0);
    EXPECT_EQ(res1, 49995000);
    int res2 = parallel_accumulate(nums.begin(), nums.end(), 0);
    EXPECT_EQ(res2, 49995000);
}

TEST(ThreadPoolTest, QuicksortTest) {
    std::list<int> nums(10000);
    std::iota(nums.begin(), nums.end(), 0);
    std::list<int> nums_org(nums.begin(), nums.end());
    std::reverse(nums.begin(), nums.end());

    auto ret = parallel_quick_sort<int>(nums);
    EXPECT_EQ(ret, nums);
}