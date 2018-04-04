//
//  MTimer.hpp
//  ProxyPlayer
//
//  Created by shuyj on 2017/12/8.
//  Copyright © 2017年 MOMO. All rights reserved.
//

#ifndef MTimer_hpp
#define MTimer_hpp
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "MThreadPool.hpp"
#include <queue>
#include <vector>
#include <algorithm>
#include "XlogAdpater.h"

struct SchedulerEvent{
    unsigned int id;                   //定时事件的唯一标示id
    uint64_t interval;                   //事件的触发间隔，在重复事件中会用到这个属性
    std::chrono::time_point<std::chrono::high_resolution_clock>     deadline;
    std::function<void()> action;      //触发的事件
    bool isRepeat;                     //是否是重复执行事件
    SchedulerEvent( unsigned int tid, uint64_t interval, std::chrono::time_point<std::chrono::high_resolution_clock> timeline,std::function<void()> action,bool isRepeat){
        id = tid;
        this->interval = interval;
        this->deadline = timeline + std::chrono::milliseconds(interval);
        this->action = action;
        this->isRepeat = isRepeat;
//        __MDLOGD_TAG("MTimer","id:%u interval:%llu deadline:%llu repeat:%d\n", id, interval, deadline, this->isRepeat);
    }
    // 重新计时
    void reStart(){
        this->deadline += std::chrono::milliseconds(this->interval);
    }
};
static bool eventCmp(const struct SchedulerEvent &a, const struct SchedulerEvent &b){
    return a.deadline > b.deadline;
}
class MTimer {
public:
    
    static std::shared_ptr<MTimer> getInstance(unsigned int tick = 50){
        static std::shared_ptr<MTimer> timer = std::shared_ptr<MTimer>(new MTimer(tick));
        return timer;
    }
    
    
    ~MTimer(){
        stop();
    }
    
    // interval 毫秒
    unsigned int addEvent(double interval,std::function<void()> action,bool isRepeat = false){
        std::lock_guard<std::mutex> lck(mutex_);
        eventQueue.emplace_back(++timerId, interval,this->timeline,action,isRepeat);
        std::sort(eventQueue.begin(), eventQueue.end(), eventCmp);
        return timerId;
    }
    
    void deleteEvent(unsigned int ntimerId){
        std::lock_guard<std::mutex> lck(mutex_);
        auto eit = std::remove_if(eventQueue.begin(), eventQueue.end(), [ntimerId](const struct SchedulerEvent ev){
            return ntimerId == ev.id;
        });
        if( eit != eventQueue.end() ) {
            eventQueue.erase(eit);
        }
    }
    
    void asyncStart(){
        if(!this->isStart){
            m_thread = std::unique_ptr<std::thread>(new std::thread(&MTimer::syncStart,this));
        }
    }
    
    void syncStart(){
        if(!isStart){
            isStart = true;
            while(isStart){
                std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
                loopForExecute();
                uint64_t usems = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
                std::unique_lock<std::mutex> locker(mutex_);
                expired_cond_.wait_for(locker, std::chrono::milliseconds(tick-usems));
                if( usems > 5 )
                    __MDLOGD_TAG("MTimer","use time:%llu\n", usems);
                timeline = std::chrono::high_resolution_clock::now();
            }
            __MDLOGD_TAG("MTimer","MTimer exec Done\n");
        }
    }
    void stop(){
        if( isStart ){
            isStart = false;
            expired_cond_.notify_one();
            if (m_thread->joinable())
            {
                m_thread->join();
            }
        }
        __MDLOGD_TAG("MTimer","MTimer exec stop Done\n");
    }
private:
    MTimer(unsigned int ttick){
        timeline = std::chrono::high_resolution_clock::now();
        tick = ttick;
        isStart = false;
        timerId = 0;
    }
    
    void loopForExecute(){
        if( !eventQueue.empty() ){
            SchedulerEvent& top = eventQueue.back();
            while( top.deadline <= timeline ){
                MThreadPool::instance()->post_task(top.action);
                
                mutex_.lock();
                if(top.isRepeat){
                    top.reStart();
                    std::sort(eventQueue.begin(), eventQueue.end(), eventCmp);
                }else{
                    eventQueue.pop_back();
                }
                mutex_.unlock();
                
                if( eventQueue.empty() ){
                    break;
                }
                top = eventQueue.back();
            }
        }
    }
    
    unsigned int                    tick;
    std::chrono::time_point<std::chrono::high_resolution_clock>     timeline;
    std::atomic<bool>               isStart;
    unsigned int                    timerId;
    std::vector<SchedulerEvent>     eventQueue;
    std::mutex                      mutex_;
    std::condition_variable         expired_cond_;
    std::unique_ptr<std::thread>    m_thread;
};

#endif /* MTimer_hpp */
