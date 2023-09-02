//
// Created by csq on 3/11/23.
//

#ifndef CPP_CONCURRENCY_JTHREAD_H
#define CPP_CONCURRENCY_JTHREAD_H

#include <vector>
#include <thread>

class jthread {
private:
    std::thread &t;
public:
    jthread(std::thread &t_) : t(t_) {}

    jthread(const jthread &) = delete;

    jthread &operator=(const jthread &) = delete;

    ~jthread() {
        if (t.joinable()) {
            t.join();
        }
    }
};

class jthreads {
private:
    std::vector<std::thread> &threads;
public:
    explicit jthreads(std::vector<std::thread> &threads_) : threads(threads_) {}

    jthreads(const jthreads &) = delete;

    jthreads &operator=(const jthreads &) = delete;

    ~jthreads() {
        for (auto &t: threads) {
            if (t.joinable()) {
                t.join();
            }
        }
//        for (int i = 0; i < threads.size(); ++i) {
//            if (threads[i].joinable()) {
//                threads[i].join();
//            }
//        }
    }
};

#endif //CPP_CONCURRENCY_JTHREAD_H
