//
//  main.cpp
//  devbase
//
//  Created by shuyj on 2018/8/16.
//  Copyright © 2018年 shuyj. All rights reserved.
//

#include <iostream>
#include <future>
#include <system_error>
#include <boost/test/unit_test.hpp>

void testFunc()
{
//    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    printf("testFunc exec...\n");
}

void testFunc2()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    printf("testFunc2 exec...\n");
}

void testFunc3(std::thread* t1)
{
    try {
        t1->join();
    } catch (std::system_error& err) {
        printf("3err=%s\n", err.what());
    }
    
    printf("testFunc3 exec...\n");
}

//int main(int argc, const char * argv[]) {
BOOST_AUTO_TEST_CASE( TestMain ){
    // insert code here...
    
//    std::future<void> testFu = std::async(std::launch::async, &testFunc);
//
//    testFu = std::async(std::launch::async, &testFunc2);
//
//    testFu.wait();
    
    
    std::thread*  t1 = new std::thread(&testFunc);
    std::thread*  t2 = (std::thread*)malloc(sizeof(std::thread));
    memcpy(t2, t1, sizeof (std::thread));
    t1->join();
    try {
        t2->join();
    } catch (std::system_error& err) {
        printf("err=%s\n", err.what());
    }
    
//    t1 = std::thread(&testFunc2);
    
//    std::thread  t2;
//    printf("t2.get_id()=%d\n", t2.native_handle());
////    if( t2.joinable() )
//    t2.join();
    
//    std::thread t1 = std::thread(&testFunc);
//    std::thread  t2;
//    memcpy(&t2, &t1, sizeof t1);
    
//    std::thread t3 = std::thread(&testFunc3, &t1);
//
//    std::thread t4 = std::thread(&testFunc3, &t1);
//
//    t3.join();
//    t4.join();
    
//    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
//    return 0;
}
