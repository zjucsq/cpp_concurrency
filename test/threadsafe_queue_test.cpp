//
// Created by csq on 2/2/23.
//
#include <vector>
#include <thread>
#include <algorithm>
#include <numeric>
#include <future>

#include "data_structure/threadsafe_queue.h"
#include "gtest/gtest.h"

class config {
public:
    size_t size;
    std::vector<int> keys;

    config(size_t size_): size(size_), keys(size_) {
        std::iota(keys.begin(), keys.end(), 1);
    }
};

config c{100000};

TEST(ThreadSafeQueueTest, SampleTest) {
    threadsafe_queue_base<int> queue;

    // int size = 10000;
    // std::vector<int> keys(size);
    // std::iota(keys.begin(), keys.end(), 1);

    std::vector<int> ans;

    std::thread t1{[&] {
        for (auto n: c.keys) {
            queue.push(n);
        }
    }};
    t1.join();

    std::thread t2{[&] {
        while (ans.size() != c.size) {
            auto ret = queue.try_pop();
            if (ret) {
                ans.push_back(*ret);
            }
        }
    }};
    t2.join();

    for (int i = 0; i < c.size; ++i) {
        EXPECT_EQ(ans[i], c.keys[i]);
    }
}

TEST(ThreadSafeQueueTest, SPSCTest) {
    threadsafe_queue_base<int> queue;

    // int size = 10000;
    // std::vector<int> keys(size);
    // std::iota(keys.begin(), keys.end(), 1);

    std::vector<int> ans(c.size);

    int nsthread = 1;
    int ncthread = 1;
    std::vector<std::thread> thread_group;
    for (int i = 0; i < nsthread; ++i) {
        thread_group.emplace_back([&](int stride) {
            for (int j = stride; j < c.size; j += nsthread) {
                queue.push(c.keys[j]);
            }
        }, i);
    }

    for (int i = 0; i < ncthread; ++i) {
        thread_group.emplace_back([&](int stride) {
            for (int j = stride; j < c.size; j += ncthread) {
                while (!queue.try_pop(ans[j]));
            }
        }, i);
    }

    for (int i = 0; i < nsthread + ncthread; ++i) {
        thread_group[i].join();
    }

    for (int i = 0; i < c.size; ++i) {
        EXPECT_EQ(ans[i], c.keys[i]);
    }
//    for (int i = 0; i < size; ++i) {
//        std::cout << i + 1 << ": " << ans[i] << std::endl;
//    }
}


TEST(ThreadSafeQueueTest, SPMCTest) {
    threadsafe_queue_base<int> queue;

    // int size = 10000;
    // std::vector<int> keys(size);
    // std::iota(keys.begin(), keys.end(), 1);

    std::vector<int> ans(c.size);

    int nsthread = 1;
    int ncthread = 3;
    std::vector<std::thread> thread_group;
    for (int i = 0; i < nsthread; ++i) {
        thread_group.emplace_back([&](int stride) {
            for (int j = stride; j < c.size; j += nsthread) {
                queue.push(c.keys[j]);
            }
        }, i);
    }

    for (int i = 0; i < ncthread; ++i) {
        thread_group.emplace_back([&](int stride) {
            for (int j = stride; j < c.size; j += ncthread) {
                while (!queue.try_pop(ans[j]));
            }
        }, i);
    }

    for (int i = 0; i < nsthread + ncthread; ++i) {
        thread_group[i].join();
    }

    std::sort(ans.begin(), ans.end());
    for (int i = 0; i < c.size; ++i) {
        EXPECT_EQ(ans[i], c.keys[i]);
    }
//    for (int i = 0; i < size; ++i) {
//        std::cout << i + 1 << ": " << ans[i] << std::endl;
//    }
}

TEST(ThreadSafeQueueTest, MPSCTest) {
    threadsafe_queue_base<int> queue;

    // int size = 10000;
    // std::vector<int> keys(size);
    // std::iota(keys.begin(), keys.end(), 1);

    std::vector<int> ans(c.size);

    int nsthread = 3;
    int ncthread = 1;
    std::vector<std::thread> thread_group;
    for (int i = 0; i < nsthread; ++i) {
        thread_group.emplace_back([&](int stride) {
            for (int j = stride; j < c.size; j += nsthread) {
                queue.push(c.keys[j]);
            }
        }, i);
    }

    for (int i = 0; i < ncthread; ++i) {
        thread_group.emplace_back([&](int stride) {
            for (int j = stride; j < c.size; j += ncthread) {
                while (!queue.try_pop(ans[j]));
            }
        }, i);
    }

    for (int i = 0; i < nsthread + ncthread; ++i) {
        thread_group[i].join();
    }

    std::sort(ans.begin(), ans.end());
    for (int i = 0; i < c.size; ++i) {
        EXPECT_EQ(ans[i], c.keys[i]);
    }
//    for (int i = 0; i < size; ++i) {
//        std::cout << i + 1 << ": " << ans[i] << std::endl;
//    }
}

TEST(ThreadSafeQueueTest, MPMCTest) {
    threadsafe_queue_base<int> queue;

    // int size = 10000;
    // std::vector<int> keys(size);
    // std::iota(keys.begin(), keys.end(), 1);

    std::vector<int> ans(c.size);

    int nsthread = 3;
    int ncthread = 3;
    std::vector<std::thread> thread_group;
    for (int i = 0; i < nsthread; ++i) {
        thread_group.emplace_back([&](int stride) {
            for (int j = stride; j < c.size; j += nsthread) {
                queue.push(c.keys[j]);
            }
        }, i);
    }

    for (int i = 0; i < ncthread; ++i) {
        thread_group.emplace_back([&](int stride) {
            for (int j = stride; j < c.size; j += ncthread) {
                while (!queue.try_pop(ans[j]));
            }
        }, i);
    }

    for (int i = 0; i < nsthread + ncthread; ++i) {
        thread_group[i].join();
    }

    std::sort(ans.begin(), ans.end());
    for (int i = 0; i < c.size; ++i) {
        EXPECT_EQ(ans[i], c.keys[i]);
    }
//    for (int i = 0; i < size; ++i) {
//        std::cout << i + 1 << ": " << ans[i] << std::endl;
//    }
}
