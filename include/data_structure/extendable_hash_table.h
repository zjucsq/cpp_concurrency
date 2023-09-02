//
// Created by csq on 3/3/23.
//

#ifndef CPP_CONCURRENCY_EXTENDABLE_HASH_TABLE_H
#define CPP_CONCURRENCY_EXTENDABLE_HASH_TABLE_H

#include <list>
#include <memory>
#include <mutex>  // NOLINT
#include <utility>
#include <vector>
#include <algorithm>

template<typename K, typename V>
class ExtendibleHashTable {
public:
    explicit ExtendibleHashTable(size_t bucket_size) : global_depth_(0), bucket_size_(bucket_size), num_buckets_(1) {
        dir_.push_back(std::make_shared<Bucket>(bucket_size));
    }

    auto GetGlobalDepth() const -> int {
        return GetGlobalDepthInternal();
    }

    auto GetLocalDepth(int dir_index) const -> int {
        return dir_[dir_index]->GetDepth();
    }

    auto GetNumBuckets() const -> int {
        return GetNumBucketsInternal();
    }

    auto Find(const K &key, V &value) -> bool {
        std::scoped_lock<std::mutex> lock(latch_);
        int dir_index = IndexOf(key);
        return dir_[dir_index]->Find(key, value);
    }

    void Insert(const K &key, const V &value) {
        std::scoped_lock<std::mutex> lock(latch_);
        int dir_index = IndexOf(key);
        if (dir_[dir_index]->FindAndInsert(key, value)) {
            return;
        }
        while (dir_[dir_index]->IsFull()) {
            RedistributeBucket(dir_index);
            // dir_index may change
            dir_index = IndexOf(key);
        }
        dir_[dir_index]->Insert(key, value);
    }

    auto Remove(const K &key) -> bool {
        std::scoped_lock<std::mutex> lock(latch_);
        int dir_index = IndexOf(key);
        return dir_[dir_index]->Remove(key);
    }

    class Bucket {
    public:
        explicit Bucket(size_t size, int depth = 0): size_(size), depth_(depth) {}

        /** @brief Check if a bucket is full. */
        inline auto IsFull() const -> bool { return list_.size() == size_; }

        /** @brief Get the local depth of the bucket. */
        inline auto GetDepth() const -> int { return depth_; }

        /** @brief Increment the local depth of a bucket. */
        inline void IncrementDepth() { depth_++; }

        inline auto GetItems() -> std::list<std::pair<K, V>> & { return list_; }

        auto Find(const K &key, V &value) -> bool {
            for (const auto &[ki, vi] : list_) {
                if (ki == key) {
                    value = vi;
                    return true;
                }
            }
            return false;
        }

        auto Remove(const K &key) -> bool {
            for (auto it = list_.begin(); it != list_.end(); ++it) {
                if ((*it).first == key) {
                    list_.erase(it);
                    return true;
                }
            }
            return false;
        }

        auto Insert(const K &key, const V &value) -> bool {
            list_.emplace_back(key, value);
            // list_.push_back({key, value});
            return true;
        }

        // if K is already in the bucket
        auto FindAndInsert(const K &key, const V &value) -> bool {
            for (auto it = list_.begin(); it != list_.end(); ++it) {
                if (it->first == key) {
                    it->second = value;
                    return true;
                }
            }
            return false;
        }

        // Only call by RedistributeBucket
        auto MoveInsert(K &&key, V &&value) -> bool {
            list_.emplace_back(key, value);
            return true;
        }

    private:
        size_t size_;
        int depth_;
        std::list<std::pair<K, V>> list_;
    };

private:
    int global_depth_;    // The global depth of the directory
    size_t bucket_size_;  // The size of a bucket
    int num_buckets_;     // The number of buckets in the hash table
    mutable std::mutex latch_;
    std::vector<std::shared_ptr<Bucket>> dir_;  // The directory of the hash table

    auto RedistributeBucket(int dir_index) -> void {
        // std::cout << dir_index << std::endl;
        int local_depth = GetLocalDepth(dir_index);
        // for local_depth == global_depth_, double dir_
        if (local_depth == global_depth_) {
            size_t dir_size = dir_.size();
            dir_.resize(2 * dir_size);
            std::copy_n(dir_.begin(), dir_size, dir_.begin() + dir_size);
            ++global_depth_;
        }

        int mask = (1 << local_depth) - 1;
        int valid_index_0 = dir_index & mask;
        // add 1 to valid_index: 0101->1101
        int valid_index_1 = valid_index_0 | (mask + 1);
        ++local_depth;
        ++num_buckets_;
        auto bucket0 = std::make_shared<Bucket>(bucket_size_, local_depth);
        auto bucket1 = std::make_shared<Bucket>(bucket_size_, local_depth);

        auto &list_ref = dir_[dir_index]->GetItems();
        int new_mask = (1 << local_depth) - 1;
        for (auto it = list_ref.begin(); it != list_ref.end(); ++it) {
            size_t hash_value = IndexOf(it->first);
            if ((hash_value & new_mask) == valid_index_0) {
                bucket0->MoveInsert(std::move(it->first), std::move(it->second));
            } else {
                bucket1->MoveInsert(std::move(it->first), std::move(it->second));
            }
        }
        int gap = (valid_index_1 - valid_index_0) * 2;
        for (size_t i = valid_index_0; i < dir_.size(); i += gap) {
            if ((i & new_mask) == valid_index_0) {
                dir_[i] = bucket0;
            }
        }
        for (size_t i = valid_index_1; i < dir_.size(); i += gap) {
            if ((i & new_mask) == valid_index_1) {
                dir_[i] = bucket1;
            }
        }
    }

    auto IndexOf(const K &key) -> size_t {
        int mask = (1 << global_depth_) - 1;
        return std::hash<K>()(key) & mask;
    }

    auto GetGlobalDepthInternal() const -> int {
        return global_depth_;
    }

    auto GetLocalDepthInternal(int dir_index) const -> int {
        return dir_[dir_index]->GetDepth();
    }

    auto GetNumBucketsInternal() const -> int {
        return num_buckets_;
    }
};

#endif //CPP_CONCURRENCY_EXTENDABLE_HASH_TABLE_H
