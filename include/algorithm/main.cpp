//
// Created by csq on 3/12/23.
//
#include <iostream>

#include "parallel_accumulate.h"

int main() {
    std::vector<int> nums(10000);
    std::iota(nums.begin(), nums.end(), 0);
    for (int i = 0; i < 100; ++i) {
        std::cout << i;
    }std::cout << std::endl;
//    int res1 = std::accumulate(nums.begin(), nums.end(), 0);
//    EXPECT_EQ(res1, 49995000);
    std::cout << "before test " << std::endl;
    int res2 = parallel_accumulate(nums.begin(), nums.end(), 0);
    std::cout << res2 << std::endl;
}