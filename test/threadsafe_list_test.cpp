//
// Created by csq on 2/2/23.
//
#include <thread>
#include <algorithm>
#include <numeric>

#include "data_structure/threadsafe_list.h"
#include "gtest/gtest.h"

// helper function to launch multiple threads
template <typename... Args>
void LaunchParallelTest(uint64_t num_threads, Args &&...args) {
    std::vector<std::thread> thread_group;

    // Launch a group of threads
    for (uint64_t thread_itr = 0; thread_itr < num_threads; ++thread_itr) {
        thread_group.push_back(std::thread(args..., thread_itr));
    }

    // Join the threads with the main thread
    for (uint64_t thread_itr = 0; thread_itr < num_threads; ++thread_itr) {
        thread_group[thread_itr].join();
    }
}

void PopHelper(threadsafe_stack<int> *s, int &cnt, __attribute__((unused)) uint64_t thread_itr = 0) {
    for (int i = 0; i < cnt; ++i) {
        s->pop();
    }
}

TEST(StackTests, InsertTest) {
    threadsafe_stack<int> s;

    int size = 100000;
    std::vector<int> keys(size);
    std::iota(keys.begin(), keys.end(), 1);

    for (auto key : keys) {
        s.push(key);
    }

    int i = 100000;
    while (!s.empty()) {
        int a;
        s.pop(a);
        EXPECT_EQ(a, i);
        --i;
    }
}

TEST(StackTests, ConcurrentTest) {
    threadsafe_stack<int> s;

    int size = 10000;
    std::vector<int> keys(size);
    std::iota(keys.begin(), keys.end(), 1);

    for (auto key : keys) {
        s.push(key);
    }

    LaunchParallelTest(2, PopHelper, size / 2);

    EXPECT_EQ(true, s.empty());
}
//
//TEST(BPlusTreeTests, ScaleInsertTest3) {
//    // create KeyComparator and index schema
//    auto key_schema = ParseCreateStatement("a bigint");
//    GenericComparator<8> comparator(key_schema.get());
//
//    auto *disk_manager = new DiskManager("test.db");
//    BufferPoolManager *bpm = new BufferPoolManagerInstance(50, disk_manager);
//    // create b+ tree
//    BPlusTree<GenericKey<8>, RID, GenericComparator<8>> tree("foo_pk", bpm, comparator);
//    GenericKey<8> index_key;
//    RID rid;
//    // create transaction
//    auto *transaction = new Transaction(0);
//    // create and fetch header_page
//    page_id_t page_id;
//    auto header_page = bpm->NewPage(&page_id);
//    (void)header_page;
//
//    int size = 500000;
//
//    std::vector<int64_t> keys(size);
//
//    std::iota(keys.begin(), keys.end(), 1);
//
//    std::random_device rd;
//    std::mt19937 g(rd());
//    std::shuffle(keys.begin(), keys.end(), g);
//    std::cout << "---------" << std::endl;
//    int i = 0;
//    (void)i;
//    for (auto key : keys) {
//        i++;
//        int64_t value = key & 0xFFFFFFFF;
//        rid.Set(static_cast<int32_t>(key >> 32), value);
//        index_key.SetFromInteger(key);
//        tree.Insert(index_key, rid, transaction);
//    }
//    std::vector<RID> rids;
//
//    std::shuffle(keys.begin(), keys.end(), g);
//
//    for (auto key : keys) {
//        rids.clear();
//        index_key.SetFromInteger(key);
//        tree.GetValue(index_key, &rids);
//        EXPECT_EQ(rids.size(), 1);
//
//        int64_t value = key & 0xFFFFFFFF;
//        EXPECT_EQ(rids[0].GetSlotNum(), value);
//    }
//
//    int64_t start_key = 1;
//    int64_t current_key = start_key;
//    index_key.SetFromInteger(start_key);
//
//    for (auto iterator = tree.Begin(index_key); iterator != tree.End(); ++iterator) {
//        auto location = (*iterator).second;
//        EXPECT_EQ(location.GetPageId(), 0);
//        EXPECT_EQ(location.GetSlotNum(), current_key);
//        current_key = current_key + 1;
//    }
//
//    EXPECT_EQ(current_key, keys.size() + 1);
//
//    std::shuffle(keys.begin(), keys.end(), g);
//    for (auto key : keys) {
//        i++;
//        index_key.SetFromInteger(key);
//        tree.Remove(index_key, transaction);
//    }
//
//    EXPECT_EQ(true, tree.IsEmpty());
//
//    bpm->UnpinPage(HEADER_PAGE_ID, true);
//
//    delete transaction;
//    delete disk_manager;
//    delete bpm;
//    remove("test.db");
//    remove("test.log");
//}
//
