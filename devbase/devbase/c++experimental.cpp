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

//#define ELTASK_ASYNC_CONTINUOUS

template<class Rp, class... Args>
class elfunction;

template<class Rp>
class elresult{
    typedef Rp value_type;
    typedef value_type const& value_reference_type;
    value_type   __value;
    bool         __ready = false;
    
    std::function<void(Rp)>   __complete_ev = nullptr;
public:
    
    void set_value(value_reference_type vrt){
        __value = vrt;
        __ready = true;
        if( __complete_ev ){
#ifdef ELTASK_ASYNC_CONTINUOUS
            MThreadPool::instance()->async([&]{
#endif
                __complete_ev(__value);
#ifdef ELTASK_ASYNC_CONTINUOUS
            });
#endif
        }
    }
    
    bool is_ready(){
        return __ready;
    }
    
    value_reference_type get_value(){
        return __value;
    }
    
//    elfunction<int, Rp>& then( std::function<int(Rp)> __cf ){
//        __complete_ev = new elfunction<int, Rp>(__cf);
//        return *__complete_ev;
//    }
    
    template<class _Rp>
    elfunction<_Rp, Rp>& then( std::function<_Rp(Rp)> __cf ){
        auto __icomplete_ev = std::make_shared<elfunction<_Rp, Rp> >(__cf);
        __complete_ev = [__icomplete_ev](Rp rtv){
            (*__icomplete_ev)(rtv);
        };
        return *__icomplete_ev;
    }
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
    void set_func( std::function<Rp(Args...)> __f){
        __func = __f;
    }
    
    void operator()(Args... args);
    
    elresult<Rp>& get_result(){
        return __result;
    }
    
};

template<class Rp, class... Args>
void elfunction<Rp, Args...>::operator()(Args... args) {
    if( __func ){
        Rp _ret = __func(std::move(args)...);
        __result.set_value(_ret);
    }
}

//template<class Rp>
//class elfunction<Rp, void>  {
//    elresult<Rp>        __result;
//    std::function<Rp()>  __func = nullptr;
//public:
//    elfunction(){}
//    elfunction(std::function<Rp()> __f){
//        set_func(__f);
//    }
//    void set_func( std::function<Rp()> __f){
//        __func = __f;
//    }
//
//    void operator()();
//
//    elresult<Rp>& get_result(){
//        return __result;
//    }
//
//};
//
//template<class Rp>
//void elfunction<Rp, void>::operator()() {
//    if( __func ){
//        Rp _ret = __func();
//        __result.set_value(_ret);
//    }
//}

template<>
class elresult<void>{
    typedef void value_type;
    bool         __ready = false;
    
    std::function<void()>   __complete_ev = nullptr;
public:
    
    void set_value(){
        __ready = true;
        if( __complete_ev ){
#ifdef ELTASK_ASYNC_CONTINUOUS
            MThreadPool::instance()->async([&]{
#endif
                __complete_ev();
#ifdef ELTASK_ASYNC_CONTINUOUS
            });
#endif
        }
    }
    
    bool is_ready(){return __ready;}
    
    void get_value(){
        return;
    }
    
    template<class _Rp>
    elfunction<_Rp>& then( std::function<_Rp()> __cf ){
        auto __icomplete_ev = std::make_shared<elfunction<_Rp> >(__cf);
        __complete_ev = [__icomplete_ev](){
            (*__icomplete_ev)();
        };
        return *__icomplete_ev;
    }
};

template<class... Args>
class elfunction<void, Args...>  {
    elresult<void>        __result;
    std::function<void(Args...)>  __func = nullptr;
public:
    elfunction(){}
    elfunction(std::function<void(Args...)> __f){
        set_func(__f);
    }
    void set_func( std::function<void(Args...)> __f){
        __func = __f;
    }
    
    void operator()(Args... args){
        if( __func ){
            __func(std::move(args)...);
            __result.set_value();
        }
    }
    
    elresult<void>& get_result(){
        return __result;
    }
    
};

// ... Args 参数列表可以支持  有参数和无参数，无需实现void特化版本

int main(int argc, const char * argv[]) {
    
    MThreadPool::Create(1);
    
    int a=5, b=9;
    elfunction<void> ttask([&](){
        std::cout<< "exec task 1" << " thread:" << std::this_thread::get_id()<< std::endl;
        return a + b;
    });

    ttask.get_result().then<int>([]()->int{
        std::cout<< "exec post task rs:" << " thread:" << std::this_thread::get_id()<< std::endl;
        return 111;
    }).get_result().then<int>([](int rs){
        std::cout<< "exec post task rs:" << rs << " thread:" << std::this_thread::get_id()<< std::endl;
        return 0;
    });

    MThreadPool::instance()->sync([&]{
        ttask();
    });
    
    std::cout<< "exec task done-" << " thread:" << std::this_thread::get_id()<< std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    
    MThreadPool::Destroy();
}

