//
// Created by Briac on 18/06/2025.
//

#ifndef SPARSEVOXRECON_THREADSAFEQUEUE_H
#define SPARSEVOXRECON_THREADSAFEQUEUE_H

#include <list>
#include <mutex>
#include <chrono>

template<typename T>
class ThreadSafeQueue{
public:
    ThreadSafeQueue(){
    }
    virtual ~ThreadSafeQueue(){
    }

    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue(ThreadSafeQueue&&) = delete;

    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

    // Push the element at the end of the queue, then return the new size.
    size_t push(T&& element);
    // Pop the first element of the queue, then return the new size.
    size_t get(T& element, const std::chrono::milliseconds& timeout, bool& success);
    size_t size() const;

    std::list<T>& getElements();
private:
    mutable std::mutex mutex;
    std::condition_variable condition;
    std::list<T> queue;
};

template<typename T>
size_t ThreadSafeQueue<T>::push(T &&element) {
    size_t s = 0;
    {
        std::unique_lock < std::mutex > lock(mutex);
        queue.emplace_back(std::move(element));
        s = queue.size();
    }
    condition.notify_one();
    return s;
}

template<typename T>
size_t ThreadSafeQueue<T>::get(T &element,
                               const std::chrono::milliseconds &timeout, bool &success) {
    std::unique_lock < std::mutex > lock(mutex);

    success = condition.wait_for(lock, timeout, [this]() {
        return !queue.empty();
    });

    if (success) {
        element = std::move(queue.front());
        queue.erase(queue.begin());
    }

    return queue.size();
}

template<typename T>
size_t ThreadSafeQueue<T>::size() const {
    std::unique_lock < std::mutex > lock(mutex);
    return queue.size();
}

template<typename T>
std::list<T>& ThreadSafeQueue<T>::getElements() {
    return queue;
}

#endif //SPARSEVOXRECON_THREADSAFEQUEUE_H
