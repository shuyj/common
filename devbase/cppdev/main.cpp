//
//  main.cpp
//  cppdev
//
//  Created by shuyj on 2018/10/8.
//  Copyright © 2018年 shuyj. All rights reserved.
//

#include <iostream>
#include "MThreadPool.hpp"
#include "MThread.hpp"

class A : public std::enable_shared_from_this<A> {
    std::string  mcontent = "A content";
    char   mc2[20] = "";
public:
    ~A(){
        std::cout<< "~A destroy" << std::endl;
    }
    void printtest(){
        mcontent = "0";
        mc2[0] = '1';
        std::cout<< mcontent << mc2 << std::endl;
    }
    
    void println(){
        std::cout<< "A print" << std::endl;
        std::weak_ptr<A> weak_this = shared_from_this();
        MThreadPool::instance()->async([&, weak_this](){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto that = weak_this.lock();
            if (!that) {
                std::cout << "The object is already dead" << std::endl;
                return;
            }
            for (int i=0; i<5; i++) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                printtest();
            }
        });
    }
};

int testM(int a, int b){
    std::cout<<"a=" << a << " b=" << b<< std::endl;
    return a+b;
}

int testA(int){
    std::cout<<"testA" << std::endl;
    return 1;
}

typedef int(&TESTM)();

int main(int argc, const char * argv[]) {
    
    MThreadPool::Create(1);
    
    std::shared_ptr<A> a = std::make_shared<A>();
    std::cout<< "A print111" << std::endl;
    a->println();
    std::cout<< "A print222" << std::endl;
    a.reset();
    std::cout<< "A print333" << std::endl;
    MThreadPool::instance()->sync([](){
        std::cout<< "MThreadPool sync" << std::endl;
    });
    MThreadPool::Destroy();
    
    MThread<int>* mt = new MThread<int>();
    mt->start([](int){});
    int ret = mt->postSyncEvent(1, testM, 2, 5);
    
    std::cout<< "postSSyncEvent=" << ret << std::endl;
    
    delete mt;
    
    MTaskQueue<int> mtq;
    mtq.start(1);
    
    auto fu = mtq.async(std::bind(testM, 3, 5));
    
    std::cout<< "asyc=" << fu.get() << std::endl;
    
    mtq.shutdown();
    
    using rrrr = std::result_of<TESTM()>::type;
    
    rrrr ss = 1;
    
    MTaskQueueT1<int()> mtq1;
    mtq1.start(1);
    
//    auto fu1 = mtq1.async(std::bind(testA));
    auto fu1 = mtq1.async(std::bind(testA, 6));
    std::cout<< "asyc=" << fu1.get() << std::endl;
    
    mtq1.shutdown();
}
