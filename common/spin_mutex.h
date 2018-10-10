//
//  spin_mutex.h
//  ProxyPlayer
//
//  Created by shuyj on 2017/12/1.
//  Copyright © 2017年 MOMO. All rights reserved.
//

#ifndef spin_mutex_h
#define spin_mutex_h

#include <atomic>
#include <mutex>

class spin_mutex {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    spin_mutex() = default;
    spin_mutex(const spin_mutex&) = delete;
    spin_mutex& operator= (const spin_mutex&) = delete;
    void lock() {
        while(flag.test_and_set(std::memory_order_acquire))
            ;
    }
    void unlock() {
        flag.clear(std::memory_order_release);
    }
};

typedef std::lock_guard<spin_mutex> AutoLock;

#endif /* spin_mutex_h */
