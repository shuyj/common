//
//  c++experimental.cpp
//  devbase
//
//  Created by shuyj on 2018/10/8.
//  Copyright © 2018年 shuyj. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <future>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <utility>
#include "MThreadPool.hpp"

#define ELTASK_ASYNC_CONTINUOUS

template<class Rp, class... Args>
class elfunction;

template<class Rp>
class elresult{
    typedef Rp value_type;
    typedef value_type const& value_reference_type;
    value_type   __value;
    bool         __ready = false;
    
    elfunction<int, Rp>*   __complete_ev = nullptr;
public:
    ~elresult();
    
    void set_value(value_reference_type vrt);
    
    bool is_ready();
    
    value_reference_type get_value();
    
    elfunction<int, Rp>& then( std::function<int(Rp)> __cf );
};

template<class Rp, class... Args>
class elfunction  {
    elresult<Rp>        __result;
    std::function<Rp(Args...)>  __func = nullptr;
public:
    elfunction(){}
    elfunction(std::function<Rp(Args...)> __f){
        set_func(__f);
    }
    void set_func( std::function<Rp(Args...)> __f);
    
    void operator()(Args... args);
    
    elresult<Rp>& get_result();
    
};

template<class Rp>
elresult<Rp>::~elresult(){
    if( __complete_ev )
        delete __complete_ev;
}
template<class Rp>
void elresult<Rp>::set_value(value_reference_type vrt){
    __value = vrt;
    __ready = true;
    if( __complete_ev ){
#ifdef ELTASK_ASYNC_CONTINUOUS
        MThreadPool::instance()->async([&]{
#endif
            (*__complete_ev)(__value);
#ifdef ELTASK_ASYNC_CONTINUOUS
        });
#endif
    }
}
template<class Rp>
bool elresult<Rp>::is_ready(){
    return __ready;
}
template<class Rp>
typename elresult<Rp>::value_reference_type elresult<Rp>::get_value(){
    return __value;
}
template<class Rp>
elfunction<int, Rp>& elresult<Rp>::then( std::function<int(Rp)> __cf )
{
    __complete_ev = new elfunction<int, Rp>(__cf);
    return *__complete_ev;
}

template<class Rp, class... Args>
void elfunction<Rp, Args...>::set_func( std::function<Rp(Args...)> __f){
    __func = __f;
}
template<class Rp, class... Args>
void elfunction<Rp, Args...>::operator()(Args... args) {
    if( __func )
        __result.set_value(__func(std::move(args)...));
}
template<class Rp, class... Args>
elresult<Rp>& elfunction<Rp, Args...>::get_result(){
    return __result;
}

int main(int argc, const char * argv[]) {
    
    MThreadPool::Create(1);
    
    int a=5, b=9;
    elfunction<int> ttask([&]()->int{
        std::cout<< "exec task 1" << " thread:" << std::this_thread::get_id()<< std::endl;
        return a + b;
    });

    ttask.get_result().then([](int rs)->int{
        std::cout<< "exec post task rs:" << rs << " thread:" << std::this_thread::get_id()<< std::endl;
        return 111;
    }).get_result().then([](int rs){
        std::cout<< "exec post task rs:" << rs << " thread:" << std::this_thread::get_id()<< std::endl;
        return 0;
    });

    MThreadPool::instance()->sync([&]{
        ttask();
    });
    
    std::cout<< "exec task done-" << " thread:" << std::this_thread::get_id()<< std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    MThreadPool::Destroy();
}

