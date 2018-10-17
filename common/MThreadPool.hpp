//
//  MThreadPool.hpp
//  HttpServer
//
//  Created by shuyj on 2018/1/31.
//  Copyright © 2018年 MOMO. All rights reserved.
//

#ifndef MThreadPool_hpp
#define MThreadPool_hpp
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>
#include <functional>
#include <stdexcept>
#include <memory>
//#include "XlogAdpater.h"

class MThreadPool {
    
private:
    using MTask = std::function<void()>;
    std::vector<std::thread>        m_threads;
    std::queue<MTask>               m_tasks;
    std::atomic_bool                m_stop;
    std::mutex                      m_lock;
    std::condition_variable         m_condition;
    
public:
    
    static void Create(int threadNumber = 2);
    static MThreadPool* instance();
    static void Destroy();
    
    MThreadPool() : m_stop{ false }
    {
    }

    ~MThreadPool()
    {
        shutdown();
    }

    void start(size_t size)
    {
        size = size < 1 ? 1 : size;
        m_threads.reserve(size);
        for (size_t i = 0; i < size; ++i)
        {
            m_threads.emplace_back(&MThreadPool::schedual, this);
        }
    }

    void async(MTask task)
    {
        if (m_stop.load())
        {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m_tasks.push(task);
        }
        m_condition.notify_one();
    }
    
    void sync(MTask func)
    {
        if (m_stop.load())
            throw std::runtime_error("commit on ThreadPool is stopped.");
            
            auto task = std::make_shared<std::packaged_task<void()> >(func);
            std::future<void> future = task->get_future();
        {
            std::lock_guard<std::mutex> lock{ m_lock };
            m_tasks.emplace( [task](){  (*task)();  }  );
        }
        m_condition.notify_one();
        
        return future.wait();
    }

    void shutdown()
    {
        if (m_stop.load())
        {
            return;
        }
        m_stop.store(true);
        m_condition.notify_all();
        for (std::thread& thread : m_threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
        m_threads.clear();
    }

    void schedual()
    {
        MTask task;
        while (true)
        {
            task = NULL;
            {
                std::unique_lock<std::mutex> lock{ m_lock };
                if (m_tasks.empty())
                {
                    m_condition.wait(lock, [this]() { return !m_tasks.empty() || m_stop.load(); });
                    if (m_tasks.empty())
                    {
    //                    std::cout << "thread " << std::this_thread::get_id() << " end" << std::endl;
                        break;
                    }
                }
                task = m_tasks.front();
                m_tasks.pop();
            }
            if (task)
            {
                task();
            }
        }
    }
};

#endif /* MThreadPool_hpp */
