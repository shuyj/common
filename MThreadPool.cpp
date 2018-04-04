//
//  MThreadPool.cpp
//  PreloadManager2Test
//
//  Created by shuyj on 2018/2/23.
//  Copyright © 2018年 MOMO. All rights reserved.
//

#include "MThreadPool.hpp"
#include "XlogAdpater.h"

static MThreadPool* gThreadPool = nullptr;
static std::mutex gMutex;

void MThreadPool::Create(int threadNumber)
{
    std::unique_lock<std::mutex> lock(gMutex);
    if (!gThreadPool) {
        gThreadPool = new MThreadPool();
        gThreadPool->start(threadNumber);
        __MDLOGD_TAG("MThreadPool", "Create ThreadPool done");
    }
}
MThreadPool* MThreadPool::instance()
{
    std::unique_lock<std::mutex> lock(gMutex);
    if (gThreadPool) {
        return gThreadPool;
    }else{
        __MDLOGD_TAG("MThreadPool", "GlobalThreadPool not created !!! ");
    }
    return nullptr;
}
void MThreadPool::Destroy()
{
    if (gThreadPool) {
        gThreadPool->shutdown();
        gThreadPool = nullptr;
        __MDLOGD_TAG("MThreadPool", "Destroy ThreadPool done");
    }
}

