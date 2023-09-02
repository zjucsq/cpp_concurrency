//
// Created by csq on 3/25/23.
//

#ifndef CPP_CONCURRENCY_RWLOCK_H
#define CPP_CONCURRENCY_RWLOCK_H

#include <mutex>
#include <condition_variable>
#include <atomic>

class rwlock {
private:
    int readers;
    bool has_writer;
    std::mutex m;
    std::condition_variable reader_cv;
    std::condition_variable writer_cv;

public:
    void RLock() {
        std::unique_lock lk(m);
        reader_cv.wait(lk, [&]() { return !has_writer; });
        readers++;
    }

    void WLock() {
        std::unique_lock lk(m);
        writer_cv.wait(lk, [&]() { return !has_writer && readers == 0; });
        has_writer = true;
    }

    void RUnLock() {
        std::lock_guard lk(m);
        readers--;
        if (readers == 0) {
            writer_cv.notify_one();
        }
    }

    void WUnLock() {
        std::lock_guard lk(m);
        has_writer = false;
        // 需要在notify之前解锁吗
        // 公平性问题
        writer_cv.notify_one();
        reader_cv.notify_one();
    }
};


#endif //CPP_CONCURRENCY_RWLOCK_H
