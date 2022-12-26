#pragma once

#include <pthread.h>
#include <functional>
#include <sys/syslog.h>

#include "set.h"

template <typename T>
class FineNode {
public:
    pthread_mutex_t m;

    T item;
    FineNode<T> *next;
    std::function<int (const T&, const T&)> comparator;

    FineNode() = delete;
public:
    FineNode(int &res, 
        const std::function<int(const T&, const T&)> &cmp) : 
        item(), next(nullptr), comparator(cmp) {
        res = pthread_mutex_init(&m, 0);
        if (res != 0)
            syslog(LOG_ERR, "Failed to destroy node");
    };

    FineNode(int &res,
        const T& _item, 
        const std::function<int(const T&, const T&)> &cmp, 
        FineNode<T> *_next = nullptr) : 
        item(_item), next(_next), comparator(cmp) {
        res = pthread_mutex_init(&m, 0);
    };

    int compare(const T& val) {
        return this->comparator(this->item, val);
    }

    bool lock() {
        int res = pthread_mutex_lock(&m);
        return res == 0;
    }

    bool unlock() {
        int res = pthread_mutex_unlock(&m);
        return res == 0;
    }

    ~FineNode() {
        int res = pthread_mutex_destroy(&m);
        if (res != 0)
            syslog(LOG_ERR, "Failed to destroy node");
    }
};

template <typename T>
class FineSet : public Set<T> {

friend class FineNode<T>;

private:
    FineNode<T> *head;
    pthread_mutex_t head_m;
    std::function<int (const T&, const T&)> comparator;

    void clear() {
        FineNode<T> *tmp = nullptr;
        FineNode<T> *cur = head;
        
        while (cur != nullptr) {
            tmp = cur->next;
            delete cur;
            cur = tmp;
        }

        delete head;
    }

    bool lock() {
        int res = pthread_mutex_lock(&head_m);
        return res == 0;
    }

    bool unlock() {
        int res = pthread_mutex_unlock(&head_m);
        return res == 0;
    }

public:
    FineSet(int &res,
        const std::function<int (const T&, const T&)> &cmp) :
        comparator(cmp) {
        pthread_mutex_init(&head_m, 0);
    }

    virtual bool add(const T& item) override {
        // step 1: try modify head.
        // if empty set
        lock();
        if (head == nullptr) {
            int res;
            head = new FineNode<T>(res, item, comparator);
            unlock();
            return true;
        }
        // if add to begin
        else if (head->compare(item) > 0) {
            int res;
            head = new FineNode<T>(res, item, comparator, head);
            unlock();
            return true;
        }

        // step 2: try to put item in middle.
        // if need to add anywhere else (we know that we have at least one element in set at least)
        FineNode<T> *pred = head;
        FineNode<T> *cur = pred->next;

        pred->lock();
        while (cur != nullptr) {
            cur->lock();
            if (cur->compare(item) < 0) {
                if (pred == head)
                    unlock();

                pred->unlock();
                pred = cur;
                cur = cur->next;
            }
            else
                break;
        }

        if (cur != nullptr && cur->compare(item) == 0) {
            cur->unlock();
            pred->unlock();

            if (pred == head)
                unlock();
            return false;
        }
        else {
            int res;
            FineNode<T> *new_node = new FineNode<T>(res, item, comparator, cur);
            pred->next = new_node;

            if (cur != nullptr)
                cur->unlock();
            pred->unlock();

            if (pred == head)
                unlock();
            return true;
        }
    }

    virtual bool remove(const T& item) override {
        // step 0: leave if empty
        lock();
        if (head == nullptr) {
            unlock();
            return false;
        }
        
        // step 1: if want delete from begin
        head->lock();
        if (head->compare(item) == 0) {
            FineNode<T> *tmp = head;
            head = head->next;
            tmp->unlock();
            delete tmp;
            unlock();
            return true;
        }
        
        // step 2: if want delete from somewhere else
        FineNode<T> *pred = head;       // already locked
        FineNode<T> *cur = pred->next;
        
        while (cur != nullptr) {
            cur->lock();
            if (cur->compare(item) < 0) {
                if (pred == head)
                    unlock();

                pred->unlock();
                pred = cur;
                cur = cur->next;
            }
            else
                break;
        }

        // if founded -> delete
        if (cur != nullptr && cur->compare(item) == 0) {
            pred->next = cur->next;
            cur->unlock();
            delete cur;
            pred->unlock();

            if (pred == head)
                unlock();
            return true;
        }
        // if not -> just unlock all mutexes
        else {
            if (cur != nullptr)
                cur->unlock();
            pred->unlock();

            if (pred == head)
                unlock();
            return false;        
        }
    }

    virtual bool contains(const T& item) override {
        // step 0: leave if empty
        lock();
        if (head == nullptr) {
            unlock();
            return false;
        }

        // step 1: search in whole set
        FineNode<T> *pred = nullptr;
        FineNode<T> *cur = head;
        cur->lock();

        while (cur != nullptr && cur->compare(item) < 0) {
            if (cur->next != nullptr)
                cur->next->lock();

            if (pred != nullptr)
                pred->unlock();
            else
                unlock();

            pred = cur;
            cur = cur->next;
        }

        // step 2: save res and unlock mutexes
        bool res = cur != nullptr && cur->compare(item) == 0;
        if (cur != nullptr)
            cur->unlock();
        if (pred != nullptr)
            pred->unlock();
        else
            unlock();

        return res;
    }

    ~FineSet() {
        clear();
    }
};
