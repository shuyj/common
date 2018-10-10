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

template<class Rp, class... Args>
class ettask_task;

template<class Rp>
class et_result{
    typedef Rp value_type;
    typedef value_type const& value_reference_type;
    value_type   __value;
    std::atomic_bool       __ready = false;
    
    ettask_task<int, Rp>*   __complete_ev = nullptr;
public:
    ~et_result();
    
    void set_value(value_reference_type vrt);
    
    bool is_ready();
    
    value_reference_type get_value();
    
    ettask_task<int, Rp>& then( std::function<int(Rp)> __cf );
};

template<class Rp, class... Args>
class ettask_task  {
    et_result<Rp>        __result;
    std::function<Rp(Args...)>  __func = nullptr;
public:
    ettask_task(){}
    ettask_task(std::function<Rp(Args...)> __f){
        set_func(__f);
    }
    void set_func( std::function<Rp(Args...)> __f);
    
    void operator()(Args... args);
    
    et_result<Rp>& get_result();
    
};

template<class Rp>
et_result<Rp>::~et_result(){
    if( __complete_ev )
        delete __complete_ev;
}
template<class Rp>
void et_result<Rp>::set_value(value_reference_type vrt){
    __value = vrt;
    __ready = true;
    if( __complete_ev )
        (*__complete_ev)(__value);
}
template<class Rp>
bool et_result<Rp>::is_ready(){
    return __ready;
}
template<class Rp>
typename et_result<Rp>::value_reference_type et_result<Rp>::get_value(){
    return __value;
}
template<class Rp>
ettask_task<int, Rp>& et_result<Rp>::then( std::function<int(Rp)> __cf )
{
    __complete_ev = new ettask_task<int, Rp>(__cf);
    return *__complete_ev;
}


//template<>
//class et_result<void>{
//public:
//    typedef void value_type;
//    std::atomic_bool       __ready = false;
//
//    void set_value(void){
//        __ready = true;
//    }
//
//    bool is_ready(){
//        return __ready;
//    }
//
//    void get_value(){
//        return;
//    }
//};

//class ettask_base{
//public:
//    template<class... Args>
//    void operator()(Args... args){}
//};
//
//template<class Rp, class... Args>
//class ettask_bask : public ettask_base{
//    using RetType = Rp;
//    et_result<RetType>        __result;
//    std::function<Rp(Args...)>  __func;
//    std::shared_ptr<ettask_base>  __next_task;
//public:
//    ettask_bask( std::function<Rp(Args...)> __f){
//        __func = __f;
//    }
//
////    inline std::shared_ptr<ettask_bask<void, RetType>>
////    template<class RRp>
//    std::shared_ptr<ettask_base> then( std::function<int(int)> __cf ){
////        ettask_base* ttm = new ettask_bask<int, int>(__cf);
//        __next_task = std::make_shared<ettask_bask<int, int>>(__cf);
////        __next_task = std::make_shared<ettask_bask<void, RetType>>(__cf);
//        return __next_task;
//    }
////    std::shared_ptr<ettask_bask> then( std::function<void(Rp)> __cf ){
////        __next_task = std::make_shared<ettask_bask<void, Rp>>(__cf);
////        return __next_task;
////    }
//
//    // 由线程池调用
//    void operator()(Args... args) {
//        __result.set_value(__func(std::move(args)...));
//        if( __next_task ){
//            (*(ettask_bask<int,int>*)__next_task.get())(__result.get_value());
//        }
//    }
//
//};

template<class Rp, class... Args>
void ettask_task<Rp, Args...>::set_func( std::function<Rp(Args...)> __f){
    __func = __f;
}
template<class Rp, class... Args>
void ettask_task<Rp, Args...>::operator()(Args... args) {
    if( __func )
        __result.set_value(__func(std::move(args)...));
}
template<class Rp, class... Args>
et_result<Rp>& ettask_task<Rp, Args...>::get_result(){
    return __result;
}

int main(int argc, const char * argv[]) {

//    ettask_bask<int, int, int> ttask([](int a, int b)->int{
//        std::cout<< "exec task 1" << std::endl;
//        return a + b;
//    });
//
//    ttask.then([](int rs)->int{
//        std::cout<< "exec post task 1" << std::endl;
//        return 111;
//    });
//
//    ttask(2, 3);
    
//    et_task<int, int, int> ttask1([](int a, int b){
//        std::cout<< "exec task 1" << std::endl;
//        return a + b;
//    });
//
//    ttask1.then([](int rs){
//        std::cout<< "exec post task 1--:" << rs << std::endl;
//        return 1;
//    });
//
//    ttask1(22, 55);
    
    ettask_task<int, int, int> ttask([](int a, int b)->int{
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

    ttask(2, 3);
    std::cout<< "exec task done-" << " thread:" << std::this_thread::get_id()<< std::endl;
}

