//
//  ppl_test.cpp
//  devbase
//
//  Created by shuyj on 2018/9/29.
//  Copyright © 2018年 shuyj. All rights reserved.
//
#include <stdio.h>
#define BOOST_THREAD_VERSION 5
#define BOOST_TEST_MODULE ThreadPPLTest
#include <boost/test/unit_test.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
//#include <future>


#include <boost/thread/future.hpp>
#include <functional>

template<class Rp, class... Args>
class ettask_bask{
    std::function<Rp(Args...)>  __func;
    std::function<void(Rp)>   __complete_ev = nullptr;
public:
    ettask_bask( std::function<Rp(Args...)> __f){
        __func = __f;
    }

    template<class __Rp>
    ettask_bask<__Rp, Rp>&  then( std::function<__Rp(Rp)> __cf ){
        auto __next_task = std::make_shared<ettask_bask<__Rp, Rp>>(__cf);
        __complete_ev = [__next_task](Rp rtv){
            (*__next_task)(rtv);
        };
        return *__next_task;
    }
    
    void operator()(Args... args){
        if( __func ){
            Rp _ret = __func(std::move(args)...);
            if( __complete_ev ) __complete_ev(_ret);
        }
    }
};

template<class... Args>
class ettask_bask<void, Args...>{
    std::function<void(Args...)>  __func;
    std::function<void(void)>   __complete_ev = nullptr;
public:
    ettask_bask( std::function<void(Args...)> __f){
        __func = __f;
    }
    
    template<class __Rp>
    ettask_bask<__Rp>&  then( std::function<__Rp(void)> __cf ){
        auto __next_task = std::make_shared<ettask_bask<__Rp>>(__cf);
        __complete_ev = [__next_task](){
            (*__next_task)();
        };
        return *__next_task;
    }
    
    void operator()(Args... args){
        if( __func ){
            __func(std::move(args)...);
            if( __complete_ev ) __complete_ev();
        }
    }
};

BOOST_AUTO_TEST_CASE( PPLTestMain ){
    ettask_bask<int, int, int> ttask([](int a, int b)->int{
        return a + b;
    });
    
    ttask.then<int>([](int rs)->int{
        std::cout << "then1:" << rs << " thread:" << std::this_thread::get_id() << std::endl;
        return 111;
    }).then<int>([](int rs){
        std::cout << "then2:" << rs << " thread:" << std::this_thread::get_id() << std::endl;
        return 0;
    });
    
    ttask(9, 8);
    
    
    ettask_bask<void> vtask([](){
        std::cout << "vtask:" << " thread:" << std::this_thread::get_id() << std::endl;
    });
    
    vtask.then<void>([](){
        std::cout << "vtask-then1:" << " thread:" << std::this_thread::get_id() << std::endl;
    }).then<void>([](){
        std::cout << "vtask-then2:" << " thread:" << std::this_thread::get_id() << std::endl;
    });
    
    vtask();
    
}
BOOST_AUTO_TEST_CASE( PPLBoostTestMain ){
    
    boost::packaged_task<int()> ptt([](){
        std::cout << "task:" << std::this_thread::get_id() << std::endl;
        return 555;
    });
//    boost::future<int> ff = boost::async(boost::launch::async, [](){
//        return 111;
//    });
    
    boost::future<int> ff = ptt.get_future();
    ff.set_deferred();

    ff.then( [](boost::future<int> rs){
        std::cout << "result:" << rs.get() << std::this_thread::get_id() << std::endl;
    }).then( [](boost::future<void> rs){
        std::cout << "all done" << std::this_thread::get_id() <<  std::endl;
    });
    
//    boost::async(boost::launch::async, [&]{
        ptt();
//    });
    
    try{

    ff.get();
    }catch( boost::exception &ex ){
        
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
}
    
