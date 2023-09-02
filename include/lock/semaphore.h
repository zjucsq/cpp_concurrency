//
// Created by csq on 3/25/23.
//

#ifndef CPP_CONCURRENCY_SEMAPHORE_H
#define CPP_CONCURRENCY_SEMAPHORE_H

#include <semaphore.h>
#include <mutex>
#include <condition_variable>
#include <atomic>

class semaphore_c {
public:
    semaphore_c(int value) {
        sem_init(&m_sem, 0, value);
    }

    ~semaphore_c() {
        sem_destroy(&m_sem);
    }

    void wait() {
        sem_wait(&m_sem);
    }

    void post() {
        sem_post(&m_sem);
    }

private:
    sem_t m_sem;
};

class semaphore {
public:
    semaphore(int cnt_ = 0) : cnt(cnt_) {}

    void acquire() {
        std::unique_lock lk(m);
        cv.wait(lk, [&]() { return cnt > 0; });
        cnt--;
    }

    void release() {
        std::lock_guard lk(m);
        cnt++;
        cv.notify_one();
    }

private:
    int cnt;
    std::mutex m;
    std::condition_variable cv;
};


#endif //CPP_CONCURRENCY_SEMAPHORE_H
