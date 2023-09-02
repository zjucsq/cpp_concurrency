//
// Created by csq on 1/31/23.
//
#include <list>
#include <algorithm>
#include <iostream>
#include <future>
#include <chrono>
#include <random>

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input) {
    if (input.empty()) {
        return input;
    }
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const& pivot = *result.begin();

    auto divide_point = std::partition(input.begin(), input.end(), [&](T const& t){return t < pivot;});
    std::list<T> lower_part;
    lower_part.splice(lower_part.begin(), input, input.begin(), divide_point);
    std::future<std::list<T>> new_lower{std::async(&parallel_quick_sort<T>, std::move(lower_part))};
    auto new_higher{parallel_quick_sort(std::move(input))};
    result.splice(result.begin(), new_lower.get());
    result.splice(result.end(), new_higher);
    return result;
}

int main()
{
    std::default_random_engine e;
    std::list<int> input;
    for (int i = 0; i < 10000000; ++i) {
        input.push_back(e());
    }
    auto start = std::chrono::system_clock::now();
    auto output = parallel_quick_sort(input);
    auto end = std::chrono::system_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "μs" << std::endl;
//    for (auto v : output) {
//        std::cout << v << ' ';
//    }
    return 0;
}
