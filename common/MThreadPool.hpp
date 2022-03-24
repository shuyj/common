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
#include <type_traits>
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
    
    bool current() {
        auto tid = std::this_thread::get_id();
        for (std::thread& th : m_threads) {
            if (th.get_id() == tid) {
                return true;
            }
        }
        return false;
    }
    
    void sync(MTask func)
    {
        if (m_stop.load())
            throw std::runtime_error("commit on ThreadPool is stopped.");
            
        if (current()) {
            func();
            return;
        }
        std::promise<void> taskpm;
        //auto task = std::make_shared<std::packaged_task<void()> >(func);
        auto task = func;
        std::future<void> future = taskpm.get_future();//task->get_future();
        {
            std::lock_guard<std::mutex> lock{ m_lock };
            m_tasks.emplace( [&](){  (task)(); taskpm.set_value(); }  );
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

/*   MThreadPool 是最早的一版
 MTaskQueue 采用模板 <返回值，参数>  方式。 MTaskQueueT1采用Func(Callable)+参数方式。 c++新的标准中也使用了MTaskQueue的方式，不使用Callable方式，这种方式在多个地方引用时需要二次处理。Callable没有返回值这样更基础
 */

template<class RetType, class... Args>
class MTaskQueue {
//    using RetType = typename std::result_of<F(Args...)>::type;
private:
    using MTask = std::function<RetType(Args...)>;
    std::vector<std::thread>        m_threads;
    std::queue<std::shared_ptr<std::packaged_task<RetType(Args...)>>>               m_tasks;
    std::atomic_bool                m_stop;
    std::mutex                      m_lock;
    std::condition_variable         m_condition;

public:
    MTaskQueue() : m_stop{ false }
    {
    }

    ~MTaskQueue()
    {
        shutdown();
    }

    void start(size_t size)
    {
        size = size < 1 ? 1 : size;
        m_threads.reserve(size);
        for (size_t i = 0; i < size; ++i)
        {
            m_threads.emplace_back(&MTaskQueue::schedual, this);
        }
    }

    std::future<RetType> async(MTask func)
    {
        if (m_stop.load())
            throw std::runtime_error("commit on ThreadPool is stopped.");
        
        auto task = std::make_shared<std::packaged_task<RetType(Args...)> >(func);
        std::future<RetType> future = task->get_future();
        {
            std::lock_guard<std::mutex> lock{ m_lock };
            m_tasks.emplace(task);
        }
        m_condition.notify_one();
        return future;
    }

    RetType sync(MTask func)
    {
        std::future<RetType> future = async(func);
        return future.get();
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
        std::shared_ptr<std::packaged_task<RetType(Args...)>> task;
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
                (*task.get())();
            }
        }
    }
};

/**
 https://www.cplusplus.com/reference/type_traits/decay/
 typedef std::decay<int>::type A;           // int
 typedef std::decay<int&>::type B;          // int
 typedef std::decay<int&&>::type C;         // int
 typedef std::decay<const int&>::type D;    // int
 typedef std::decay<int[2]>::type E;        // int*
 typedef std::decay<int(int)>::type F;      // int(*)(int)
 
 https://www.cplusplus.com/reference/type_traits/result_of/
 int fn(int) {return int();}                            // function
 typedef int(&fn_ref)(int);                             // function reference
 typedef int(*fn_ptr)(int);                             // function pointer
 struct fn_class { int operator()(int i){return i;} };  // function-like class
 当传入Function 指针，或是引用时， 无法在result_of   和   packaged_task同时使用相同参数
 注意： 当函数需要参数时，无法在执行时简单的为其传递参数，如需要传递参数设计到参数的保存与传递，所以Func默认都为无参数，现在lamdba可支持闭包函数，所以参数可以不使用显示传递，可通过[&]进行捕获
 注意：result_of直接使用<int()> 就是function不可以， 需要转换为 fcuntion pointer或reference     使用 std::decay转换
 
 */
template<class Func, class... Args>
class MTaskQueueT1 {
    using RetType = typename std::result_of<typename std::decay<Func>::type(Args...)>::type;
private:
    using MTask = std::function<Func>;
    std::vector<std::thread>        m_threads;
    std::queue<std::shared_ptr<std::packaged_task<Func>>>               m_tasks;
    std::atomic_bool                m_stop;
    std::mutex                      m_lock;
    std::condition_variable         m_condition;

public:
    MTaskQueueT1() : m_stop{ false }
    {
    }

    ~MTaskQueueT1()
    {
        shutdown();
    }

    void start(size_t size)
    {
        size = size < 1 ? 1 : size;
        m_threads.reserve(size);
        for (size_t i = 0; i < size; ++i)
        {
            m_threads.emplace_back(&MTaskQueueT1::schedual, this);
        }
    }
    
    std::future<RetType> async(MTask func)
    {
        if (m_stop.load())
            throw std::runtime_error("commit on ThreadPool is stopped.");
        
        auto task = std::make_shared<std::packaged_task<Func> >(func);
        std::future<RetType> future = task->get_future();
        {
            std::lock_guard<std::mutex> lock{ m_lock };
            m_tasks.emplace(task);
        }
        m_condition.notify_one();
        return future;
    }

    RetType sync(MTask func)
    {
        std::future<RetType> future = async(func);
        return future.get();
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
        std::shared_ptr<std::packaged_task<Func>> task;
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
                (*task.get())();
            }
        }
    }
};

#endif /* MThreadPool_hpp */
