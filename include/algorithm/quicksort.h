//
// Created by csq on 3/13/23.
//

#ifndef CPP_CONCURRENCY_QUICKSORT_H
#define CPP_CONCURRENCY_QUICKSORT_H

#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <list>
#include <numeric>

#include "utils/thread_pool.h"

template <typename T>
struct sorter {
  thread_pool pool;

  std::list<T> do_sort(std::list<T> &chunk_data) {
    if (chunk_data.empty()) {
      return chunk_data;
    }
    std::list<T> result;
    result.splice(result.begin(), chunk_data, chunk_data.begin());
    T const &partition_val = *result.begin();
    auto divide_point =
        std::partition(chunk_data.begin(), chunk_data.end(), [&](T const &val) { return val < partition_val; });
    std::list<T> new_lower_chunk;
    new_lower_chunk.splice(new_lower_chunk.begin(), chunk_data, chunk_data.begin(), divide_point);
    std::future<std::list<T>> new_lower = pool.submit(&sorter::do_sort, this, std::move(new_lower_chunk));
    std::list<T> new_higher(do_sort(chunk_data));
    result.splice(result.end(), new_higher);
    while (new_lower.wait_for(std::chrono::seconds(0)) == std::future_status::timeout) {
      pool.run_pending_task();
    }
    result.splice(result.begin(), new_lower.get());
    return result;
  }
};

template <typename T>
std::list<T> parallel_quick_sort(std::list<T> input) {
  if (input.empty()) {
    return input;
  }
  sorter<T> s;
  return s.do_sort(input);
};

#endif  // CPP_CONCURRENCY_QUICKSORT_H
