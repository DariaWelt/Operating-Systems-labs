#pragma once

#include <functional>
#include <vector>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

#include "../set/set.h"
#include "utils.h"

struct writeArgs_t {
    Set<int>* set;
    const std::vector<int>* items;
    int begin, end;

    writeArgs_t(Set<int>* set, const std::vector<int>* items, int begin, int end) :
        set(set), items(items), begin(begin), end(end) {}
};

struct readArgs_t {
    Set<int>* set;
    const std::vector<int>* items;
    std::vector<int>* removeCheckArr;
    int begin, end;

    readArgs_t(Set<int>* set, 
        const std::vector<int>* items, 
        std::vector<int>* removeCheckArr, 
        int begin, 
        int end) :
        set(set), items(items), removeCheckArr(removeCheckArr), begin(begin), end(end) {}
};


template<class SomeSet>
class Tester {
private:
    // write set variables
    inline static SomeSet set;
    inline static std::vector<int> items, removeCheckArr;
    inline static std::function<int (const int&, const int&)> comparator;

    // sync tools
    inline static pthread_cond_t cv;
    inline static bool start;

    static void startTesting() {
        pthread_cond_broadcast(&cv);
        start = true;
    }

    static void synchronize() {
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_lock(&mutex);
        if (!start)
            pthread_cond_wait(&cv, &mutex);
        else
            pthread_cond_broadcast(&cv);
        pthread_mutex_unlock(&mutex);
    }

    static void* threadWrite(void* inputArgs) {
        writeArgs_t* args = reinterpret_cast<writeArgs_t*>(inputArgs);
        
        synchronize();
        for (int i = args->begin; i < args->end; ++i)
            args->set->add((*(args->items))[i]);

        return (void*)nullptr;
    }

    static void* threadRead(void* inputArgs) {
        readArgs_t* args = reinterpret_cast<readArgs_t*>(inputArgs);
        
        synchronize();
        for (int i = args->begin; i < args->end; ++i) {
            if (args->set->remove((*(args->items))[i]))
                    (*(args->removeCheckArr))[i] = 1;
        }

        return (void*)nullptr;
    }

    Tester() = default;
public:

    static Tester& getInstance() {
      static Tester tester;
      return tester;
    }

    static void init(int &res,
        const std::function<int (const int&, const int&)> &cmp) {
        start = false;
        res = 0;
        comparator = cmp;
        set = SomeSet(res, cmp);
        cv = PTHREAD_COND_INITIALIZER;
    }

    static void prepareWriteTreads (
        std::vector<pthread_t>& threads,
        std::vector<writeArgs_t>& args) {
        start = false;
            
        int writers_cnt = (int)threads.size();
        args.reserve(writers_cnt);

        int size = items.size();
        int batch_size = size / writers_cnt;

        int begin = 0;
        int end = batch_size;
        for (int i = 0; i < writers_cnt; ++i) {
            if (i == writers_cnt - 1)
                end = size;

            args.emplace_back(&set, &items, begin, end);
            pthread_create(&threads[i], nullptr, threadWrite, (void*)&args.back());
            
            begin += batch_size;
            end += batch_size;
        }
    }

    static void prepareReadTreads (
        std::vector<pthread_t>& threads,
        std::vector<readArgs_t>& args) {
        start = false;
            
        int readers_cnt = (int)threads.size();
        args.reserve(readers_cnt);

        int size = items.size();
        int batch_size = size / readers_cnt;

        int begin = 0;
        int end = batch_size;
        for (int i = 0; i < readers_cnt; ++i) {
            if (i == readers_cnt - 1)
                end = size;

            args.emplace_back(&set, &items, &removeCheckArr, begin, end);
            pthread_create(&threads[i], nullptr, threadRead, (void*)&args.back());
            
            begin += batch_size;
            end += batch_size;
        }
    }

    static long long writersTesting(int writers_cnt) {
        std::vector<pthread_t> threads(writers_cnt);
        std::vector<writeArgs_t> args;
        prepareWriteTreads(threads, args);

        auto t1 = std::chrono::high_resolution_clock::now();
        
        startTesting();
        for (int i = 0; i < writers_cnt; ++i) 
            pthread_join(threads[i], nullptr);
        
        auto t2 = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    }

    static bool checkWriteCorrectWork(int n_writers, int size, int seed = 42) {
        n_writers = n_writers > sysconf(_SC_NPROCESSORS_ONLN) ? sysconf(_SC_NPROCESSORS_ONLN) : n_writers;
        items = generateItems(size, seed);
        writersTesting(n_writers);

        std::string result = "success";
        bool res = true;
        for (int item : items) {
            if (!set.contains(item)) {
                result = "fail: missed value";
                res = false;
                break;
            }
        }

        std::cout << "Write tesing result '" << set.getType() << 
            ": " << result << std::endl;
        return res;
    }
    
    static void checkWritePerformance(int n_writers, int size, int numOfTests, const std::vector<int> &stat_items) {
        n_writers = n_writers > sysconf(_SC_NPROCESSORS_ONLN) ? sysconf(_SC_NPROCESSORS_ONLN) : n_writers;
        items = stat_items;
        long long dt = 0;
        for (int i = 0; i < numOfTests; ++i) {
            int res = 0;
            set = SomeSet(res, comparator);
            dt += writersTesting(n_writers);
        }

        std::cout << "Write performance'" << set.getType() << 
            "': writers_cnt: " << n_writers << 
            ", size: " << size <<
            ", average time: " << dt / numOfTests << " ms" <<
            ", total experiments: " << numOfTests << std::endl;
    }

    static long long readersTesting(int readers_cnt) {
        std::vector<pthread_t> threads(readers_cnt);
        std::vector<readArgs_t> args;
        prepareReadTreads(threads, args);

        auto t1 = std::chrono::high_resolution_clock::now();
        
        startTesting();
        for (int i = 0; i < readers_cnt; ++i) 
            pthread_join(threads[i], nullptr);
        
        auto t2 = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    }

    static bool checkReadCorrectWork(int n_readers, int size, int seed = 42) {
        n_readers = n_readers > sysconf(_SC_NPROCESSORS_ONLN) ? sysconf(_SC_NPROCESSORS_ONLN) : n_readers;
        items = generateUniqueItems(size, seed);
        removeCheckArr = std::vector(size, 0);
        for (int item : items)
            set.add(item);

        readersTesting(n_readers);

        std::string result = "success";
        bool res = true;
        if (!set.empty()) {
            result = "fail: not empty";
            res = false;
        }
        else {
            for (int check : removeCheckArr) {
               if (check == 0) {
                    result = "fail: not removed value";
                    res = false;
                    break;
                }
            }
        }

        std::cout << "Read tesing result '" << set.getType() << ": " << result << std::endl;
        return res;
    }
    
    static void checkReadPerformance(int n_readers, int size, int numOfTests, const std::vector<int> &stat_items) {
        n_readers = n_readers > sysconf(_SC_NPROCESSORS_ONLN) ? sysconf(_SC_NPROCESSORS_ONLN) : n_readers;
        items = stat_items;

        long long dt = 0;
        for (int i = 0; i < numOfTests; ++i) {
            int res = 0;
            set = SomeSet(res, comparator);
            for (int item : items)
                set.add(item);

            dt += readersTesting(n_readers);
        }

        std::cout << "Read performance'" << set.getType() << 
            "': readers_cnt: " << n_readers << 
            ", size: " << size <<
            ", average time: " << dt / numOfTests << " ms" <<
            ", total experiments: " << numOfTests << std::endl;
    }

    static long long generalTesting(int writers_cnt, int readers_cnt) {
        std::vector<pthread_t> write_threads(writers_cnt);
        std::vector<writeArgs_t> write_args;
        std::vector<pthread_t> read_threads(readers_cnt);
        std::vector<readArgs_t> read_args;

        prepareReadTreads(read_threads, read_args);
        prepareWriteTreads(write_threads, write_args);
        
        auto t1 = std::chrono::high_resolution_clock::now();
        
        startTesting();
        for (int i = 0; i < writers_cnt; ++i) 
            pthread_join(read_threads[i], nullptr);
        for (int i = 0; i < readers_cnt; ++i) 
            pthread_join(write_threads[i], nullptr);
        
        auto t2 = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    }

    static void checkGeneralCorrectWork(int max_threads, int size, int seed = 42) {
        max_threads = max_threads > sysconf(_SC_NPROCESSORS_ONLN) ? sysconf(_SC_NPROCESSORS_ONLN) : max_threads;
        items = generateItems(size, seed);

        std::cout << "General testing " << set.getType()<< std::endl;

        for (int readers_cnt = 1; readers_cnt < max_threads; readers_cnt++) {
            int writers_cnt = max_threads - readers_cnt;
            int set_res = 0;

            set = SomeSet(set_res, comparator);
            items = generateUniqueItems(size, seed);
            removeCheckArr = std::vector(size, 0);
            
            generalTesting(writers_cnt, readers_cnt);

            std::string result = "success";
            
            for (int i = 0; i < size; i++) {
                if (removeCheckArr[i] == 0 && set.contains(items[i]))
                    continue;
                else if (removeCheckArr[i] == 1 && !set.contains(items[i]))
                    continue;
                else {
                    result = "fail: there is not removed ot not added value";
                    break;
                }
            }
            
            std::cout << " - readers_cnt " << readers_cnt << 
                ", writers_cnt: " << writers_cnt <<
                ", result: " << result << std::endl;
        }
    }
    
    static void checkGeneralPerformance(int max_threads, int size, int numOfTests, const std::vector<int> &stat_items) {
        max_threads = max_threads > sysconf(_SC_NPROCESSORS_ONLN) ? sysconf(_SC_NPROCESSORS_ONLN) : max_threads;
        items = stat_items;

        std::cout << "General performance" << set.getType()<< std::endl;

        for (int readers_cnt = 1; readers_cnt < max_threads; readers_cnt++) {
            long long dt = 0;
            int writers_cnt = max_threads - readers_cnt;

            for (int i = 0; i < numOfTests; i++) {
                int set_res = 0;
                set = SomeSet(set_res, comparator);
                removeCheckArr = std::vector(size, 0);
            
                dt += generalTesting(writers_cnt, readers_cnt);
            }

            std::cout << " - readers_cnt " << readers_cnt << 
                ", writers_cnt: " << writers_cnt <<
                ", size: " << size <<
                ", size: " << size <<
                ", time: " << dt / numOfTests << " ms" << std::endl;
        }
    }
};
