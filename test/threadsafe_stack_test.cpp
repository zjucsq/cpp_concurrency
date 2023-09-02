//
// Created by csq on 2/2/23.
//
#include <thread>
#include <algorithm>
#include <numeric>
#include <future>

#include "data_structure/threadsafe_stack.h"
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

void PopHelper(threadsafe_stack<int> *s, int cnt, __attribute__((unused)) uint64_t thread_itr = 0) {
    for (int i = 0; i < cnt; ++i) {
        s->pop();
    }
}

int PopHelper2(threadsafe_stack<int> *s, int cnt) {
    int res = 0;
    for (int i = 0; i < cnt; ++i) {
        int a;
        s->pop(a);
        res += a;
    }
    return res;
}

TEST(StackTests, InsertTest) {
    threadsafe_stack<int> s;

    int size = 100000;
    std::vector<int> keys(size);
    std::iota(keys.begin(), keys.end(), 1);

    for (auto key : keys) {
        s.push(key);
    }

    int i = 100000;
    while (!s.empty()) {
        int a;
        s.pop(a);
        EXPECT_EQ(a, i);
        --i;
    }
}

TEST(StackTests, ConcurrentTest1) {
    threadsafe_stack<int> s;

    int size = 10000;
    std::vector<int> keys(size);
    std::iota(keys.begin(), keys.end(), 1);

    for (auto key : keys) {
        s.push(key);
    }

    LaunchParallelTest(2, PopHelper, &s, size / 2);

    EXPECT_EQ(true, s.empty());
}

TEST(StackTests, ConcurrentTest2) {
    threadsafe_stack<int> s;

    int size = 10000;
    int sum = size * (size + 1) / 2;
    std::vector<int> keys(size);
    std::iota(keys.begin(), keys.end(), 1);

    for (auto key : keys) {
        s.push(key);
    }

    std::future<int> res1 = std::async(PopHelper2, &s, size / 2);
    std::future<int> res2 = std::async(PopHelper2, &s, size / 2);

    EXPECT_EQ(res1.get() + res2.get(), sum);
    EXPECT_EQ(true, s.empty());
}