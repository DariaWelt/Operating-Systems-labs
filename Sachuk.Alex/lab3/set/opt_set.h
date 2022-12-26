#pragma once

#include <string>
#include <stdexcept>
#include <pthread.h>
#include <functional>
#include <sys/syslog.h>

#include "set.h"

template <typename T>
class OptNode {
public:
    pthread_mutex_t m;

    T item;
    OptNode<T> *next, *next_rem;
    std::function<int (const T&, const T&)> comparator;

    OptNode() = delete;
public:
    OptNode(int &res,
        const std::function<int(const T&, const T&)> &cmp) : 
        item(), next(nullptr), next_rem(nullptr), comparator(cmp) {
        res = pthread_mutex_init(&m, 0);
        if (res != 0)
            syslog(LOG_ERR, "Failed to destroy node");
    };

    OptNode(int &res,
        const T& _item, 
        const std::function<int(const T&, const T&)> &cmp, 
        OptNode<T> *_next = nullptr) : 
        item(_item), next(_next), next_rem(nullptr), comparator(cmp) {
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

    ~OptNode() {
        int res = pthread_mutex_destroy(&m);
        if (res != 0)
            syslog(LOG_ERR, "Failed to destroy node");
    }
};


// TODO : MAYBE WE NEED TO MAKE LIKE IN FINE SET ADDING?
template <typename T>
class OptSet : public Set<T> {

friend class OptNode<T>;

private:
    OptNode<T> *head, *head_rem;
    std::function<int (const T&, const T&)> comparator;

    bool validate(const OptNode<T> *pred, const OptNode<T> *curr) {
        OptNode<T> *node = head;
        
        while (node != nullptr && node->compare(pred->item) <= 0) {
            if (node == pred) {
                return pred->next == curr;
            }
            node = node->next;
        }
        return false;
    }

    void clearRemoved() {
        OptNode<T> *tmp = nullptr;
        OptNode<T> *cur = head_rem->next_rem;
        
        head_rem->next_rem = nullptr;
        while (cur != nullptr) {
            tmp = cur->next_rem;
            delete cur;
            cur = tmp;
        }
    }

    void clearNotRemoved() {
        OptNode<T> *tmp = nullptr;
        OptNode<T> *cur = head->next;
        
        while (cur != nullptr) {
            tmp = cur->next;
            delete cur;
            cur = tmp;
        }
        delete head;
        delete head_rem;
    }

public:
    OptSet(int &res,
        const std::function<int (const T&, const T&)> &cmp) :
        comparator(cmp) {
        res = 0;

        head = new OptNode<T>(res, comparator);
        if (res != 0) {
            syslog(LOG_ERR, "ERROR [OptErr]: Bad initialization; code: %i.", res);
            delete head;
            return;
        }

        head_rem = new OptNode<T>(res, comparator);
        if (res != 0) {
            syslog(LOG_ERR, "ERROR [OptErr]: Bad initialization; code: %i.", res);
            delete head;
            delete head_rem;
            return;
        }
    }

    virtual bool add(const T& item) override {
        while (true) {
            OptNode<T> *pred = head;
            OptNode<T> *cur = pred->next;

            // Find place to put new element
            while (cur != nullptr && cur->compare(item) < 0) {
                pred = cur;
                cur = cur->next;
            }

            // Lock mutexes
            if (!pred->lock()) {
                syslog(LOG_ERR, "ERROR [OptErr]: can't lock mutex.");
                return false;
            }
            if (cur != nullptr && !cur->lock()) {
                syslog(LOG_ERR, "ERROR [OptErr]: can't lock mutex.");
                return false;
            }

            if (!validate(pred, cur)) {
                pred->unlock();
                if (cur != nullptr)
                    cur->unlock();
                continue;
            }

            if (cur != nullptr && cur->compare(item) == 0) {
                pred->unlock();
                if (cur != nullptr)
                    cur->unlock();
                return false;
            }

            int res = 0;
            OptNode<T> *new_node = new OptNode<T>(res, item, comparator, cur);
            if (res != 0) {
                syslog(LOG_ERR, "ERROR [OptErr]: can't create node; code: %i.", res);
                delete new_node;
                
                pred->unlock();
                if (cur != nullptr)
                    cur->unlock();

                throw std::runtime_error("ERROR [OptErr]: can't create node; code: " + std::to_string(res));
            }
            pred->next = new_node;

            pred->unlock();
            if (cur != nullptr)
                cur->unlock();
            return true;
        }
    }

    virtual bool remove(const T& item) override {
        while (true) {
            OptNode<T> *pred = head;
            OptNode<T> *cur = pred->next;

            // Find place to delete correct element
            while (cur != nullptr && cur->compare(item) < 0) {
                pred = cur;
                cur = cur->next;
            }

            if (cur == nullptr)
                return false;

            // Lock mutexes: pred
            {
                if (!pred->lock()) {
                    syslog(LOG_ERR, "ERROR [OptErr]: can't lock mutex.");
                    return false;
                }

                // Lock mutexes: cur
                {
                    if (!cur->lock()) {
                        pred->unlock();
                        syslog(LOG_ERR, "ERROR [OptErr]: can't lock mutex.");
                        return false;
                    }

                    if (!validate(pred, cur)) {
                        pred->unlock();
                        cur->unlock();
                        continue;
                    }

                    if (cur->compare(item) == 0) {
                        pred->next = cur->next;
                        pred->unlock();
                        cur->unlock();

                        // Set deleted nodes in rem_list
                        {
                            head_rem->lock();
                            cur->next_rem = head_rem->next_rem;
                            head_rem->next_rem = cur;
                            head_rem->unlock();
                        }
                        return true;
                    }

                    cur->unlock();
                }

                pred->unlock();
                return false;
            }
        }
    }

    virtual bool contains(const T& item) override {
        while (true) {
            OptNode<T> *pred = head;
            OptNode<T> *cur = pred->next;

            // Find place to put new element
            while (cur != nullptr && cur->compare(item) < 0) {
                pred = cur;
                cur = cur->next;
            }

            if (cur == nullptr)
                return false;
            
            // Lock mutexes: pred
            bool res = false;
            {
                if (!pred->lock()) {
                    syslog(LOG_ERR, "ERROR [OptErr]: can't lock mutex.");
                    return false;
                }

                // Lock mutexes: cur
                {
                    if (!cur->lock()) {
                        pred->unlock();
                        syslog(LOG_ERR, "ERROR [OptErr]: can't lock mutex.");
                        return false;
                    }

                    if (!validate(pred, cur)) {
                        pred->unlock();
                        cur->unlock();
                        continue;
                    }

                    res = cur->compare(item) == 0;
                    cur->unlock();
                }

                pred->unlock();
                return res;
            }
        }
    }

    ~OptSet() {
        clearRemoved();
        clearNotRemoved();
    }
};
