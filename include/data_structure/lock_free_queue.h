//
// Created by csq on 2/2/23.
//

#ifndef CPP_CONCURRENCY_LOCK_FREE_QUEUE_H
#define CPP_CONCURRENCY_LOCK_FREE_QUEUE_H

#include <atomic>
#include <memory>

template<typename T>
class lock_free_queue_array_spsc {
public:
    lock_free_queue_array_spsc(size_t capacity) : m_array(new T[capacity + 1]), m_capacity(capacity + 1), m_head(0), m_tail(0),
                                             m_read_tail(0) {}

    ~lock_free_queue_array_spsc() {
        delete[]m_array;
    }

    inline size_t getRealPos(size_t num) {
        return num % m_capacity;
    }

    bool empty() {
        return m_head.load() == m_read_tail.load();
    }

    bool full() {
        return m_head.load() == getRealPos(m_tail.load() + 1);
    }

    bool pop(T &value) {
        size_t current_head = m_head.load();
        size_t current_read_tail = m_read_tail.load();
        if (current_head == current_read_tail) {
            return false;
        }
        m_head.store(getRealPos(current_head + 1));
        value = m_array[current_head];
        return true;
    }

    bool push(T new_value) {
        size_t current_head = m_head.load();
        size_t current_tail = m_tail.load();
        if (current_head == getRealPos(current_tail + 1)) {
            return false;
        }
        m_tail.store(getRealPos(current_tail + 1));
        m_array[current_tail] = new_value;
        m_read_tail.store(getRealPos(current_tail + 1));
        return true;
    }

private:
    T *m_array;
    size_t m_capacity;
    std::atomic<size_t> m_head;
    std::atomic<size_t> m_tail;
    std::atomic<size_t> m_read_tail;
};

template<typename T>
class lock_free_queue_array {
public:
    lock_free_queue_array(size_t capacity) : m_array(new T[capacity + 1]), m_capacity(capacity + 1), m_head(0), m_tail(0),
                                             m_read_tail(0) {}

    ~lock_free_queue_array() {
        delete[]m_array;
    }

    inline size_t getRealPos(size_t num) {
        return num % m_capacity;
    }

    bool empty() {
        return m_head.load() == m_read_tail.load();
    }

    bool full() {
        return m_head.load() == getRealPos(m_tail.load() + 1);
    }

    bool pop(T &value) {
        size_t current_head = m_head.load();
        size_t current_read_tail;
        do {
            current_read_tail = m_read_tail.load();
            if (current_head == current_read_tail) {
                return false;
            }
        } while (!m_head.compare_exchange_strong(current_head, getRealPos(current_head + 1)));
        value = m_array[current_head];
        return true;
    }

    bool push(T new_value) {
        size_t current_head;
        size_t current_tail = m_tail.load();
        do {
            current_head = m_head.load();
            if (current_head == getRealPos(current_tail + 1)) {
                return false;
            }
        } while (!m_tail.compare_exchange_strong(current_tail, getRealPos(current_tail + 1)));
        m_array[current_tail] = new_value;
        size_t push_pos;
        do {
            push_pos = current_tail;
        } while (!m_read_tail.compare_exchange_strong(push_pos, getRealPos(push_pos + 1)));
        return true;
    }

private:
    T *m_array;
    size_t m_capacity;
    std::atomic<size_t> m_head;
    std::atomic<size_t> m_tail;
    std::atomic<size_t> m_read_tail;
};

template<typename T>
class lock_free_queue_spsc {
private:
    struct node {
        std::shared_ptr<T> data;
        node *next;

        node() : next(nullptr) {}
    };

    std::atomic<node *> head;
    std::atomic<node *> tail;

    node *pop_head() {
        node *const old_head = head.load();
        if (old_head == tail.load()) {
            return nullptr;
        }
        head.store(old_head->next);
        return old_head;
    }

public:
    lock_free_queue_spsc() : head(new node), tail(head.load()) {}

    lock_free_queue_spsc(const lock_free_queue_spsc &other) = delete;

    lock_free_queue_spsc &operator=(const lock_free_queue_spsc &other) = delete;

    ~lock_free_queue_spsc() {
        while (node *const old_head = head.load()) {
            head.store(old_head->next);
            delete old_head;
        }
    }

    std::shared_ptr<T> pop() {
        node *const old_head = pop_head();
        if (!old_head) {
            return std::make_shared<T>();
        }
        std::shared_ptr<T> const res(old_head->data);
        delete old_head;
        return res;
    }

    void push(T new_value) {
        node *const new_tail = new node;
        std::shared_ptr<T> new_data = std::make_shared<T>(new_value);
        node *const old_tail = tail.load();
        old_tail->data.swap(new_data);
        old_tail->next = new_tail;
        tail.store(new_tail);
    }
};

#endif //CPP_CONCURRENCY_LOCK_FREE_QUEUE_H
