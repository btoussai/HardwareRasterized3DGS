//
// Created by Briac on 18/06/2025.
//

#include "AsyncWorkers.h"

#include <iostream>

inline AsyncWorkers::AsyncWorkers(int threads) {

    for (int ID = 0; ID < threads; ID++) {
        std::function < void() > f = [this, ID]() {
            this->thread_loop(ID);
        };
        this->threads.emplace_back(f);
    }
}

inline AsyncWorkers::~AsyncWorkers() {
    if(!error_occurred){
        ThreadSafeQueue<int> l;
        exec([&](){
            l.push(-1); // A dummy task that pushes an element on the queue
        });

        // make sure all previous tasks have been started
        while(true){
            int res;
            bool success;
            l.get(res, std::chrono::milliseconds(100), success);
            if(success){
                break;
            }
        }
        // At this point, all tasks should have terminated unless some were added concurrently
    }

    should_exit = true;
    for (std::thread &thread : threads) {
        thread.join();
    }
    std::cout << "Async threads deleted." << std::endl;

    if(tasks.size() > 0){
        std::cout << tasks.size() <<" tasks remaining." << std::endl;
    }
}


inline void AsyncWorkers::exec(std::function<void()> &&f) {
    if(error_occurred){
        throw std::runtime_error("An error has occurred in the async threads");
    }
    tasks.push(std::move(f));
}

inline std::chrono::milliseconds AsyncWorkers::execAll(
        std::vector<std::function<void()>> &tasks) {
    if(error_occurred){
        throw std::runtime_error("An error has occurred in the async threads");
    }

    ThreadSafeQueue<int> l;
    for(auto& t : tasks){
        this->tasks.push([&](){
            t();
            l.push(0);
        });
    }

    int completed = 0;
    auto t0 = std::chrono::system_clock::now();
    while(completed < (int)tasks.size()){
        int res;
        bool success;
        l.get(res, std::chrono::milliseconds(1000), success);
        if(success){
            completed++;
        }
    }
    auto t1 = std::chrono::system_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
}

inline void AsyncWorkers::thread_loop(int ID) {

    std::chrono::milliseconds timeout(200);
    while (!should_exit) {
        std::function<void()> task;
        bool success = false;
        tasks.get(task, timeout, success);

        if (success) {
            try{
                task();
            } catch(std::string& s){
                std::stringstream ss;
                ss <<"An exception occurred in thread " <<ID <<": " <<std::endl;
                ss <<s <<std::endl;
                std::cout <<ss.str() <<std::endl;
                should_exit = true;
                error_occurred = true;
            }
        }
    }
}

void AsyncWorkers::checkErrors() {
    if(error_occurred){
        throw std::runtime_error("An error has occurred in the async threads.");
    }
}
