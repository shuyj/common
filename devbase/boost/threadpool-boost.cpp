//
//  threadpool-boost.cpp
//  devbase
//
//  Created by shuyj on 2018/8/17.
//  Copyright © 2018年 shuyj. All rights reserved.
//
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
//#define BOOST_TEST_MODULE ThreadPoolTest
#include <boost/test/unit_test.hpp>
using namespace std;
using namespace boost;
typedef std::shared_ptr<std::thread> thread_ptr;
typedef std::vector<thread_ptr> vecThread;

class ThreadPool {
public:
    ThreadPool(int num) : threadNum_(num), stopped_(false), work_(io_) {
        for(int i=0; i<threadNum_; ++i) {
            threads_.push_back(std::make_shared<std::thread>([&](){io_.run();}));
        }
    }
    ~ThreadPool() {
        stop();
    }
    template<typename F, typename...Args>
    void post(F &&f, Args&&...args) {
        io_.post(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    }
    void stop() {
        if(!stopped_) {
            io_.stop();
            for(auto t : threads_) t->join();
            stopped_ = true;
        }
    }
    
private:
    bool             stopped_;
    vecThread        threads_;
    int              threadNum_;
    asio::io_service io_;
    asio::io_service::work work_;
};


void test1(int x) {std::cout<<"test 1:"<<x<<std::endl;}
void test2(int y) {std::cout<<"test 2:"<<y<<std::endl;}
//BOOST_AUTO_TEST_SUITE(test_boost_threadpool)

BOOST_AUTO_TEST_CASE( boost_threadpool_test )
{
    ThreadPool threads(5);
    threads.post([](){std::cout<<"test 1"<<std::endl;});
    threads.post([](){std::cout<<"test 2"<<std::endl;});
    threads.post(test1, 3);
    threads.post(test2, 5);
    
    std::this_thread::sleep_for(2s);
    threads.stop();
}
//BOOST_AUTO_TEST_SUITE_END()


