//
//  actor-boost.cpp
//  devbase
//
//  Created by shuyj on 2018/8/16.
//  Copyright © 2018年 shuyj. All rights reserved.
//

#include <iostream>
#include <deque>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread/thread.hpp>

#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>

template<typename T>
class Actor : public boost::enable_shared_from_this<Actor<T>>
{
    public:
    Actor(boost::asio::io_service& service)
    :service_(service)
    {}
    ~Actor()
    {
        std::cout << "destroy" << std::endl;
    }
    
    void push_message(const T& msg)
    {
        bool is_runing = !msg_queue_.empty();
        msg_queue_.push_back(msg);
        if (!is_runing)
        {
            get_message();
        }
    }
    
    protected:
    void get_message()
    {
        service_.post(boost::bind(&Actor<T>::do_get_message, this));
    }
    
    virtual void process_message(const T& msg) = 0;
    private:
    void do_get_message()
    {
        process_message(msg_queue_.front());
        msg_queue_.pop_front();
        if (!msg_queue_.empty())
        {
            get_message();
        }
    }
    boost::asio::io_service& service_;
    std::deque<T> msg_queue_;
};

#include <boost/format.hpp>

class Actor1 : public Actor<int>
{
    public:
    Actor1(boost::asio::io_service& service)
    :Actor<int>(service)
    {}
    
    void set_other(boost::shared_ptr<Actor<std::string>> other)
    {
        other_ = other;
    }
    
    protected:
    virtual void process_message(const int& msg)
    {
        auto ptr = other_.lock();
        if (ptr)
        {
            ptr->push_message(boost::str(boost::format("receive: %d") % msg));
        }
    }
    private:
    boost::weak_ptr<Actor<std::string>> other_;
};

class Actor2 : public Actor<std::string>
{
    public:
    Actor2(boost::asio::io_service& service)
    :Actor<std::string>(service), num_(0)
    {}
    
    void set_other(boost::shared_ptr<Actor<int>> other)
    {
        other_ = other;
    }
    protected:
    virtual void process_message(const std::string& msg)
    {
        std::cout << msg << std::endl;
        
        auto ptr = other_.lock();
        if (!ptr)
        {
            return;
        }
        
        if (++num_ == 100)
        {
            std::cout << "finished.." << std::endl;
            return;
        }
        ptr->push_message(num_);
    }
    
    private:
    int num_;
    
    boost::weak_ptr<Actor<int>> other_;
};

BOOST_AUTO_TEST_SUITE(test_boost_suite)

BOOST_AUTO_TEST_CASE( boost_actor_test ) // 注册测试用例
{
    boost::asio::io_service service;
    
    
    
    boost::shared_ptr<Actor1> a1(boost::make_shared<Actor1>(service));
    boost::shared_ptr<Actor2> a2(boost::make_shared<Actor2>(service));
    a1->set_other(a2);
    a2->set_other(a1);
    a1->push_message(-1);
    
    /*
     io_service::work: 用于控制运行于io_service之上任务的起始和结束，声明即开始，作用域结束后自动释放，告诉io_service任务都结束了. 不过对于线程池的实现来说，只用了构造函数，告诉io_service要持续运行，为了让client可以主动释放，这里没有使用work的析构函数，而是调用了io_service提供的stop()方法，让run()结束进而释放线程资源.
     */
//    boost::asio::io_service::work work(service);
    // work内会对service进行run，但在析构时，会将service停止掉
    
    service.run();
}

BOOST_AUTO_TEST_SUITE_END()

