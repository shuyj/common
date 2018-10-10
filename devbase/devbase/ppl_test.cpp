//
//  ppl_test.cpp
//  devbase
//
//  Created by shuyj on 2018/9/29.
//  Copyright © 2018年 shuyj. All rights reserved.
//
#include <stdio.h>
#define BOOST_THREAD_VERSION 5
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
    using RetType = Rp;
    std::function<Rp(Args...)>  __func;
//    std::function<void(Rp)>    __complete;
    std::shared_ptr<ettask_bask>  __next_task;
public:
//    template<class Fp>
    ettask_bask( std::function<Rp(Args...)> __f){
//        __func = std::bind(__f, args...);
        __func = __f;
//        __complete = nullptr;
    }
    
//    调用then后，可以继续添加任务
//    template<class _Fp>
    std::shared_ptr<ettask_bask> then( std::function<void(Rp)> __cf ){
//        __complete = __cf;
        __next_task = std::make_shared<ettask_bask>(__cf);
        return __next_task;
    }
    
    // 由线程池调用
    void __run(){
//        __complete(__func());
        
    }
    
};

BOOST_AUTO_TEST_CASE( PPLTestMain ){
    ettask_bask<int, int, int> ttask([](int a, int b)->int{
        return a + b;
    });
    
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
    
