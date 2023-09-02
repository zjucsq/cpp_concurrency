//
// Created by csq on 2/2/23.
//

#ifndef CPP_CONCURRENCY_THREADSAFE_QUEUE_H
#define CPP_CONCURRENCY_THREADSAFE_QUEUE_H

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>
class threadsafe_queue_base {
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue_base() {}

    threadsafe_queue_base(threadsafe_queue_base const &other) {
        std::lock_guard<std::mutex> lock(other.mut);
        data_queue = other.data_queue;
    }

    void push(T new_value) {
        std::lock_guard<std::mutex> lock(mut);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }

    void wait_and_pop(T &value) {
        std::lock_guard<std::mutex> lock(mut);
        data_cond.wait(lock, [this] { return !data_queue.empty(); });
        value = std::move(data_queue.front());
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop() {
        std::lock_guard<std::mutex> lock(mut);
        data_cond.wait(lock, [this] { return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool try_pop(T &value) {
        std::lock_guard<std::mutex> lock(mut);
        if (data_queue.empty())
            return false;
        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lock(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>{};
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mut);
        return data_queue.empty();
    }
};

#endif //CPP_CONCURRENCY_THREADSAFE_QUEUE_H
