//
// Created by Briac on 18/06/2025.
//

#ifndef SPARSEVOXRECON_ASYNCWORKERS_H
#define SPARSEVOXRECON_ASYNCWORKERS_H

#include "ThreadSafeQueue.h"
#include <thread>
#include <functional>

class AsyncWorkers {
public:
    AsyncWorkers(int threads=std::thread::hardware_concurrency());
    virtual ~AsyncWorkers();

    AsyncWorkers(const AsyncWorkers&) = delete;
    AsyncWorkers(AsyncWorkers&&) = delete;
    AsyncWorkers& operator=(const AsyncWorkers&) = delete;
    AsyncWorkers& operator=(AsyncWorkers&&) = delete;

    inline void checkErrors();

    void exec(std::function<void()>&& f);
    std::chrono::milliseconds execAll(std::vector<std::function<void()>>& tasks);

private:
    size_t scheduleTask(std::function<void()>&& task, ThreadSafeQueue<int>* results);
    void thread_loop(int ID);

    std::atomic_bool should_exit = false;
    std::atomic_bool error_occurred = false;
    std::vector<std::thread> threads;
    ThreadSafeQueue<std::function<void()>> tasks;
};


#endif //SPARSEVOXRECON_ASYNCWORKERS_H
