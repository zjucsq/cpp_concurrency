//
// Created by csq on 3/12/23.
//

#ifndef CPP_CONCURRENCY_PARALLEL_ACCUMULATE_H
#define CPP_CONCURRENCY_PARALLEL_ACCUMULATE_H

#include <future>
#include <algorithm>
#include <numeric>

#include "utils/thread_pool.h"

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    const auto length = std::distance(first, last);
    if (!length) {
        return init;
    }
    const long block_size = 25;
    auto num_blocks = (length + block_size - 1) / block_size;
    std::vector<std::future<T>> futures(num_blocks - 1);
    thread_pool pool;
    Iterator block_start = first;
    for (long i = 0; i < (num_blocks - 1); ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        futures[i] = pool.submit([=]{return std::accumulate(block_start, block_end, 0);});
        block_start = block_end;
    }
    T result = std::accumulate(block_start, last, init);
    for (long i = 0; i < (num_blocks - 1); ++i) {
        result += futures[i].get();
    }
    return result;
}

#endif //CPP_CONCURRENCY_PARALLEL_ACCUMULATE_H
