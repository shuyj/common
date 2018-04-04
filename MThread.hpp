//
//  MThread.hpp
//  ProxyPlayer
//
//  Created by shuyj on 2018/3/26.
//  Copyright © 2018年 MOMO. All rights reserved.
//

#ifndef MThread_hpp
#define MThread_hpp

#include "MMessageQueue.hpp"

template< typename T >
class MThread {
    
private:
    using MTask = std::function<void(T)>;
    struct CommandEvent{
        std::shared_ptr<MTask>       process;
        T           msg;
        ~CommandEvent(){
//            __MDLOGD_TAG("MThread","CommandEvent destroy %d!!!\n", msg);
        }
        CommandEvent( const T& msg, std::shared_ptr<MTask> task){
            this->msg = msg;
            this->process = task;
        }
    };
    std::unique_ptr<std::thread>    m_thread;
    std::shared_ptr<MTask>                           m_task;
    MMessageQueue<std::shared_ptr<CommandEvent>>     m_msgqueue;
    std::atomic_bool                m_stop;
    std::mutex                      m_lock;
    std::condition_variable         m_condition;
    
public:
    MThread() : m_stop{ false }
    {
    }
    ~MThread()
    {
        stop();
    }
    // 启动消息处理线程，传入消息默认处理函数
    void start(const MTask task)
    {
        m_task = std::make_shared<MTask>(task);
        m_thread = std::unique_ptr<std::thread>(new std::thread(&MThread::dispose, this));
    }
    // 投递消息，指定task时(不执行默认处理函数)，执行指定task
    void postEvent(const T& msg, MTask task = nullptr)
    {
        if (m_stop.load())
        {
            return;
        }
        {
            std::shared_ptr<MTask> stask = nullptr;
            if( task == nullptr ){
                stask = m_task;
            }else{
                stask = std::make_shared<MTask>(task);
            }
            std::shared_ptr<CommandEvent> ev(new CommandEvent(msg, stask));
            std::lock_guard<std::mutex> lock(m_lock);
            m_msgqueue.push(ev);
        }
        m_condition.notify_one();
    }
    // 投递可变参数函数进行执行
    template<class F, class... Args>
    void postEvent(const T& msg, F&& f, Args&&... args)/*->std::future<decltype(f(args...))>*/
    {
        if (m_stop.load())
            return;//throw std::runtime_error("commit on MThread is stopped.");
        
        using RetType = decltype(f(args...));
        //        std::function<void()> task = std::function<void()>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        auto task = std::make_shared<std::function<void()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        //        CommandEvent evtest(msg+1, nullptr);
        //        std::future<RetType> future = task->get_future();
        {
            std::shared_ptr<MTask> stask = std::make_shared<MTask>([task](const T& msg){
                //                printf("task exec :%d\n", 1111 );
                (*task)();
            });
            std::shared_ptr<CommandEvent> ev(new CommandEvent(msg, stask));
            //            ev->realfunc = task;
            std::lock_guard<std::mutex> lock{ m_lock };
            m_msgqueue.push(ev);
        }
        m_condition.notify_one();
        //        return future;
    }
    // 停止线程，会等待所有消息处理完毕
    void stop()
    {
        if (m_stop.load())
        {
            return;
        }
        m_stop.store(true);
        m_condition.notify_all();
        if (m_thread->joinable())
        {
            m_thread->join();
        }
    }
private:
    void dispose()
    {
        std::shared_ptr<CommandEvent> ev;
        while (true)
        {
            ev.reset();
            {
                std::unique_lock<std::mutex> lock{ m_lock };
                if (m_msgqueue.empty())
                {
                    m_condition.wait(lock, [this]() { return !m_msgqueue.empty() || m_stop.load(); });
                    if (m_msgqueue.empty())
                    {
//                      std::cout << "thread " << std::this_thread::get_id() << " end" << std::endl;
                        break;
                    }
                }
                ev = m_msgqueue.pop();
            }
            if ( ev && ev->process )
            {
                (*ev->process)(ev->msg);
            }
        }
        __MDLOGD_TAG("MThread","mthread dispose done!!!\n");
    }
};

#endif /* MThread_hpp */
