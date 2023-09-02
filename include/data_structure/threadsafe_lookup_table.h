//
// Created by csq on 2/2/23.
//

#ifndef CPP_CONCURRENCY_THREADSAFE_LOOKUP_TABLE_H
#define CPP_CONCURRENCY_THREADSAFE_LOOKUP_TABLE_H

#include <list>
#include <shared_mutex>
#include <functional>
#include <algorithm>

template<typename Key, typename Value, typename Hash=std::hash<Key>>
class threadsafe_lookup_table {
private:
    class bucket_type {
    private:
        using bucket_value = std::pair<Key, Value>;
        using bucket_data = std::list<bucket_value>;
        using bucket_iterator = bucket_data::iterator;
        bucket_data data;
        mutable std::shared_mutex mutex;

        bucket_iterator find_entry_for(Key const &key) const {
            return std::find_if(data.begin(), data.end(), [&](bucket_value const &item) { return item.first == key; });
        }

    public:
        Value value_for(Key const &key, Value const &default_value) const {
            std::shared_lock<std::shared_mutex> lock(mutex);
            auto it = find_entry_for(key);
            if (it != data.end()) {
                return default_value;
            } else {
                return it->second;
            }
        }

        void add_or_update_mapping(Key const &key, Value const &value) {
            std::lock_guard<std::shared_mutex> lock(mutex);
            auto it = find_entry_for(key);
            if (it != data.end()) {
                it->second = value;
            } else {
                data.emplace_back(key, value);
            }
        }

        void remove_mapping(Key const &key) {
            std::lock_guard<std::shared_mutex> lock(mutex);
            auto it = find_entry_for(key);
            if (it != data.end()) {
                data.erase(it);
            }
        }
    };

    std::vector<std::unique_ptr<bucket_type>> buckets;
    Hash hasher;

    bucket_type &get_bucket(Key const &key) const {
        return *buckets[hasher(key) % buckets.size()];
    }

public:
    using key_type = Key;
    using mapped_type = Value;
    using hash_type = Hash;

    threadsafe_lookup_table(unsigned num_buckets = 19, Hash const &hasher_ = Hash()) : buckets(num_buckets),
                                                                                       hasher(hasher_) {
        for (auto i = 0; i < num_buckets; ++i) {
            buckets[i].reset(new bucket_type);
        }
    }

    threadsafe_lookup_table(threadsafe_lookup_table const &other) = delete;

    threadsafe_lookup_table &operator=(threadsafe_lookup_table const &other) = delete;

    Value value_for(Key const &key, Value const &default_value = Value()) const {
        return get_bucket(key).value_for(key, default_value);
    }

    void add_or_update_mapping(Key const &key, Value const &value) {
        get_bucket(key).add_or_update_mapping(key, value);
    }

    void remove_mapping(Key const &key) {
        get_bucket(key).remove_mapping(key);
    }
};

#endif //CPP_CONCURRENCY_THREADSAFE_LOOKUP_TABLE_H
