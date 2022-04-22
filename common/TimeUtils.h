//
//  TimeUtils.h
//  devbase
//
//  Created by yajun18 on 2022/4/20.
//  Copyright © 2022 shuyj. All rights reserved.
//

#ifndef TimeUtils_h
#define TimeUtils_h
#include <chrono>

static int64_t GetCurrentTimeMs(){
    auto curTime = std::chrono::system_clock::now();
    int64_t startms = std::chrono::duration_cast<std::chrono::milliseconds>(curTime.time_since_epoch()).count();
    return startms;
}


#endif /* TimeUtils_h */
