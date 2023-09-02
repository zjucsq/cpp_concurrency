//
// Created by csq on 3/11/23.
//

#ifndef CPP_CONCURRENCY_THREAD_POOL_H
#define CPP_CONCURRENCY_THREAD_POOL_H

#include <functional>
#include <thread>
#include <future>

#include "jthread.h"
#include "data_structure/threadsafe_queue.h"
#include "data_structure/threadsafe_queue_linkedlist.h"

class function_wapper {
private:
    struct impl_base {
        virtual void call() = 0;

        virtual ~impl_base() = default;;
    };

    std::unique_ptr<impl_base> impl;

    template<typename F>
    struct impl_type : impl_base {
        F f;

        explicit impl_type(F &&f_) : f(std::move(f_)) {}

        void call() override { f(); }
    };

public:
    template<class F>
    explicit function_wapper(F &&f) : impl(new impl_type<F>(std::move(f))) {}

    void operator()() { impl->call(); }

    function_wapper() = default;

    function_wapper(function_wapper &&other) noexcept : impl(std::move(other.impl)) {}

    function_wapper &operator=(function_wapper &&other) noexcept {
        impl = std::move(other.impl);
        return *this;
    }

    function_wapper(const function_wapper &other) = delete;

    function_wapper(function_wapper &other) = delete;

    function_wapper &operator=(const function_wapper &other) = delete;
};


class thread_pool {
private:
    std::atomic<bool> done;
    threadsafe_queue<function_wapper> work_queue;
    // threadsafe_queue<std::function<void()>> work_queue;
    std::vector<std::thread> threads;
    jthreads joiner;

    void work_thread() {
        while (!done) {
            function_wapper task;
            // std::function<void()> task;
            if (work_queue.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }

public:
    thread_pool() : done(false), joiner(threads) {
        unsigned const thread_count = std::thread::hardware_concurrency();
        try {
            for (unsigned i = 0; i < thread_count; ++i) {
                threads.emplace_back(&thread_pool::work_thread, this);
            }
        } catch (...) {
            done = true;
            throw;
        }
    }

    ~thread_pool() {
        done = true;
    }

    template<typename F, typename ...Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
        using result_type = decltype(f(args...));
        std::function<result_type()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        std::packaged_task<result_type()> task(std::move(func));
        std::future<result_type> res(task.get_future());
        work_queue.push(function_wapper(std::move(task)));
        // work_queue.push(std::move(task));
        return res;
    }

    void run_pending_task() {
        function_wapper task;
        // std::function<void()> task;
        if (work_queue.try_pop(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }
};


#endif //CPP_CONCURRENCY_THREAD_POOL_H
