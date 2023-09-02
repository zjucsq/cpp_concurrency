//
// Created by csq on 3/13/23.
//
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>

#include "utils/thread_pool.h"

std::atomic<int> cnt{0};

void f(int num) {
    for (int n = 0; n < num; ++n) {
        cnt.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
//    std::vector<std::thread> v;
//    for (int n = 0; n < 10; ++n) {
//        v.emplace_back(f);
//    }
//    for (auto &t: v) {
//        t.join();
//    }
    {
        std::vector<std::thread> v;
        jthreads jp(v);
        for (int n = 0; n < 100; ++n) {
            v.emplace_back(f, 100);
        }
    }
    // std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "Final counter value is " << cnt << '\n';
}