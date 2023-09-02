//
// Created by csq on 2/2/23.
//

#ifndef CPP_CONCURRENCY_THREADSAFE_LIST_H
#define CPP_CONCURRENCY_THREADSAFE_LIST_H

#include <memory>
#include <mutex>

template<typename T>
class threadsafe_list {
    struct node {
        std::mutex m;
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;

        node() : next() {}

        node(T const &value) : data(std::make_shared<T>(value)) {}
    };

    node head;
public:
    threadsafe_list() {}

    ~threadsafe_list() {
        remove_if();
    }

    threadsafe_list(threadsafe_list const &other) = delete;

    threadsafe_list &operator=(threadsafe_list const &other) = delete;

    void push_front(T const& value) {
        std::unique_ptr<node> new_node = std::make_unique<node>(value);
        std::lock_guard lk(head.m);
        new_node->next = std::move(head.next);
        head.next = std::move(new_node);
    }

    template<typename Function>
    void for_each(Function f) {
        node* current = &head;

    }
};

#endif //CPP_CONCURRENCY_THREADSAFE_LIST_H
