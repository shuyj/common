//
//  StringUtils.cpp
//  devbase
//
//  Created by yajun18 on 2022/3/22.
//  Copyright © 2022 shuyj. All rights reserved.
//

#include "StringUtils.hpp"

#include <stdarg.h>
#include <string>
#include <cstring>
#include <chrono>

std::string StringFormatV(const char* fmt, va_list args) {
    int size;
    va_list tmp_args;
    
    /* args is undefined after vsnprintf.
     * So we need a copy here to avoid the
     * second vsnprintf access undefined args.
     */
    va_copy(tmp_args, args);
    size = vsnprintf(NULL, 0, fmt, tmp_args);
    va_end(tmp_args);
    
    std::string str;
    
    if (0 < size) {
        str.resize(size+1); // add the terminating null byte
        vsnprintf((char*)str.data(), size+1, fmt, args);
        str.resize(size);
    }
    
    return str;
}

std::string StringFormat(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    std::string str = StringFormatV(fmt, args);
    
    va_end(args);
    return str;
}

std::string GetTimestamp() {
    std::time_t time;   // Current time (sec)
    int         msec;   // Current time (msec part)

    // Because C-style date & time utilities don't support millisecond precison,
    // we have to handle it on our own.
    auto time_now = std::chrono::system_clock::now();
    auto duration_in_ms = duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
    auto ms_part = duration_in_ms - duration_cast<std::chrono::seconds>(duration_in_ms);
    
    time = std::chrono::system_clock::to_time_t(time_now);
    msec = (int)ms_part.count();
    
    static const int BUFLEN = 60;
    char tmpBuf[BUFLEN];
    
    strftime(tmpBuf, BUFLEN, "%F %T", localtime(&time));
    
    return StringFormat("%s.%03d", tmpBuf, msec);
}

int64_t GetCurTimeUs() {
    auto time_now = std::chrono::system_clock::now();
    auto duration_in_us = duration_cast<std::chrono::microseconds>(time_now.time_since_epoch());
    return (int64_t)duration_in_us.count();
}
