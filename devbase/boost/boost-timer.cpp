//
//  MessageBus-Test.cpp
//  devbase
//
//  Created by shuyj on 2018/8/24.
//  Copyright © 2018年 shuyj. All rights reserved.
//

#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#define BOOST_TEST_MODULE ThreadPoolTest
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <future>

void print2(const boost::system::error_code& e)
{
    std::cout << "print 2" << std::endl;
    
}

BOOST_AUTO_TEST_CASE( TestBoostTimer )
{
//    boost::asio::io_service io;
//
//    boost::asio::deadline_timer t(io, boost::posix_time::seconds(2));
//
//    t.async_wait(print2);
//    io.run();

    
    std::future<void> tfuture = std::async(std::launch::async, [](){
        std::cout << "print 2" << std::endl;
    });
    
    tfuture.wait();
    
    tfuture = std::async(std::launch::async, [](){
        std::cout << "print 3" << std::endl;
    });
}

