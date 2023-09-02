//
// Created by csq on 2/24/23.
//

#ifndef CPP_CONCURRENCY_THREADSAFE_QUEUE_LINKEDLIST_H
#define CPP_CONCURRENCY_THREADSAFE_QUEUE_LINKEDLIST_H

#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>
class queue {
private:
    struct node {
        T data;
        std::unique_ptr<node> next;

        node(T data_) : data(std::move(data_)) {}
    };

    std::unique_ptr<node> head;
    node *tail;
public:
    queue() : tail(nullptr) {}

    queue(const queue &other) = delete;

    queue &operator=(const queue &other) = delete;

    std::shared_ptr<T> try_pop() {
        if (!head) {
            return std::shared_ptr<T>{};
        }
        std::shared_ptr<T> res = std::make_shared<T>(std::move(head->data));
        head = std::move(head->next);
        if (!head) {
            tail = nullptr;
        }
        return res;
    }

    void push(T new_value) {
        std::unique_ptr<T> p = std::make_unique<node>(std::move(new_value));
        node *const new_tail = p.get();
        if (tail) {
            tail->next = std::move(p);
        } else {
            head = std::move(p);
        }
        tail = new_tail;
    }
};

template<typename T>
class queue_dummy {
private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::unique_ptr<node> head;
    node *tail;
public:
    queue_dummy() : head(std::make_unique<node>()), tail(head.get()) {}

    queue_dummy(const queue_dummy &other) = delete;

    queue_dummy &operator=(const queue_dummy &other) = delete;

    std::shared_ptr<T> try_pop() {
        if (head.get() == tail) {
            return std::shared_ptr<T>{};
        }
        std::shared_ptr<T> const res = head->data;
        head = std::move(head->next);
        return res;
    }

    void push(T new_value) {
        std::shared_ptr<T> new_data = std::make_shared<T>(std::move(new_value));
        tail->data = new_data;
        tail->next = std::move(std::make_unique<node>());
        tail = tail->next.get();
    }
};

template<typename T>
class threadsafe_queue_simple {
private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::mutex head_mutex;
    std::mutex tail_mutex;
    std::unique_ptr<node> head;
    node *tail;

    node *get_tail() {
        std::lock_guard lk(tail_mutex);
        return tail;
    }

    std::unique_ptr<node> pop_head() {
        std::lock_guard lk(head_mutex);
        if (head.get() == get_tail()) {
            return nullptr;
        }
        auto old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }

public:
    threadsafe_queue_simple() : head(std::make_unique<node>()), tail(head.get()) {}

    threadsafe_queue_simple(const threadsafe_queue_simple &other) = delete;

    threadsafe_queue_simple &operator=(const threadsafe_queue_simple &other) = delete;

    std::shared_ptr<T> try_pop() {
        auto old_head = pop_head();
        return old_head ? old_head->data : std::make_shared<T>();
    }

    void push(T new_value) {
        std::shared_ptr<T> new_data = std::make_shared<T>(std::move(new_value));
        std::unique_ptr<node> new_node = std::make_unique<node>();
        node *const new_tail = new_node.get();
        std::lock_guard lk(tail_mutex);
        tail->data = new_data;
        tail->next = std::move(new_node);
        tail = new_tail;
    }
};

template<typename T>
class threadsafe_queue {
private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::mutex head_mutex;
    std::mutex tail_mutex;
    std::condition_variable data_cond;
    std::unique_ptr<node> head;
    node *tail;

    node *get_tail() {
        std::lock_guard lk(tail_mutex);
        return tail;
    }

    std::unique_ptr<node> pop_head() {
        auto old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }

    std::unique_lock<std::mutex> wait_for_data() {
        std::unique_lock head_lock(head_mutex);
        data_cond.wait(head_lock, [&]() { return head.get() != get_tail(); });
        return head_lock;
    }

    std::unique_ptr<node> wait_pop_head() {
        std::unique_lock head_lock(wait_for_data());
        return pop_head();
    }

    std::unique_ptr<node> wait_pop_head(T& value) {
        std::unique_lock head_lock(wait_for_data());
        value = std::move(*head->data);
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head() {
        std::lock_guard head_lock(head_mutex);
        if (head.get() == get_tail()) {
            return std::unique_ptr<node>();
        }
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head(T& value) {
        std::lock_guard head_lock(head_mutex);
        if (head.get() == get_tail()) {
            return std::unique_ptr<node>();
        }
        value = std::move(*head->data);
        return pop_head();
    }

public:
    threadsafe_queue() : head(std::make_unique<node>()), tail(head.get()) {}

    threadsafe_queue(const threadsafe_queue &other) = delete;

    threadsafe_queue &operator=(const threadsafe_queue &other) = delete;

    std::shared_ptr<T> try_pop() {
        std::unique_ptr<node> old_head = try_pop_head();
        return old_head ? old_head->data : std::shared_ptr<T>();
    }

    bool try_pop(T &value) {
        std::unique_ptr<node> old_head = try_pop_head(value);
        return !!old_head;
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_ptr<node> const old_head = wait_pop_head();
        return old_head->data;
    }

    void wait_and_pop(T &value) {
        std::unique_ptr<node> const old_head = wait_pop_head(value);
    }

    void push(T new_value) {
        std::shared_ptr<T> new_data = std::make_shared<T>(std::move(new_value));
        std::unique_ptr<node> new_node = std::make_unique<node>();
        {
            std::lock_guard lk(tail_mutex);
            tail->data = new_data;
            tail->next = std::move(new_node);
            tail = tail->next.get();
        }
        data_cond.notify_one();
    }

    bool empty() {
        std::lock_guard head_lock(head_mutex);
        return head.get() == get_tail();
    }
};

#endif //CPP_CONCURRENCY_THREADSAFE_QUEUE_LINKEDLIST_H
