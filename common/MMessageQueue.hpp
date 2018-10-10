//
//  MMessageQueue.hpp
//  HttpServer
//
//  Created by shuyj on 2018/1/19.
//  Copyright © 2018年 MOMO. All rights reserved.
//

#ifndef MMessageQueue_hpp
#define MMessageQueue_hpp

#include <list>
#include <mutex>
#include <condition_variable>
#include <chrono>

template<typename T>
class MMessageQueue
{
    using AutoLock = std::lock_guard<std::mutex>;
    using MLock = std::unique_lock<std::mutex>;
public:
    MMessageQueue(){}
    void push(T t){
        AutoLock l(mMutex);
        mList.push_back(t);
    }
    template< class... Args >
    void push_emplace(Args&&... args){
        AutoLock l(mMutex);
        mList.emplace_back(args...);
    }
    T peek(){
        return mList.front();
    }
    T pop(){
        AutoLock l(mMutex);
        T front = mList.front();
        mList.pop_front();
        return front;
    }
    bool empty(){
        return mList.empty();
    }
    int  size(){
        return mList.size();
    }
    void clear(){
        MLock m(mMutex);
        mList.clear();
    }
    void wait(){
        MLock m(mMutex);
        mCond.wait(m);
    }
    void wait(unsigned int timeMs){
        MLock m(mMutex);
        mCond.wait_for(m, std::chrono::milliseconds(timeMs));
    }
    void notify(){
        MLock m(mMutex);
        mCond.notify_one();
    }
    
private:
    std::list<T>                mList;
    std::mutex                  mMutex;
    std::condition_variable     mCond;
};
template<typename T, typename ComFunc>
class MMessageSortedQueue{
public:
    void push(T& t){}
//    void push_delay(T& t, int delayMs){}
    // 查看第一个，是否可以取出
    T& top(){return T();}
    T& pop(){return T();}
};


#endif /* MMessageQueue_hpp */
