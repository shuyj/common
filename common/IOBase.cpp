//
//  IOBase.cpp
//  devbase
//
//  Created by yajun18 on 2021/12/14.
//  Copyright © 2021 shuyj. All rights reserved.
//

#include "IOBase.hpp"
#include "common.h"

ACacheBuffer::ACacheBuffer(int capacity) {
    _buf = std::make_unique<char[]>(capacity);
    _bufPosr = 0; _bufPosw = 0;
    _capacity = capacity;
}

int ACacheBuffer::freeSpace() {
    return _bufPosr > _bufPosw ? _bufPosr - _bufPosw - 1 : _bufPosr + _capacity - _bufPosw - 1;   // notice: -1 avoid roration
}

int ACacheBuffer::len() {
    return _bufPosw >= _bufPosr ? _bufPosw - _bufPosr : _bufPosw + _capacity - _bufPosr;
}

bool ACacheBuffer::write(const char* pb, int len) {
    if (len > freeSpace()) {
        return false;
    }
    int wpos = 0;
    while (wpos < len) {
        int cl = MMIN(_capacity - _bufPosw, len - wpos);
        memcpy(&_buf[_bufPosw], pb + wpos, cl);
        _bufPosw = (_bufPosw + cl) % _capacity;
        wpos += cl;
    }
    return true;
}

bool ACacheBuffer::read(char* pb, int len) {
    if (len > this->len()) {
        return false;
    }
    int rpos = 0;
    while (rpos < len) {
        int cl = MMIN(_capacity - _bufPosr, len - rpos);
        memcpy(pb + rpos, &_buf[_bufPosr], cl);
        _bufPosr = (_bufPosr + cl) % _capacity;
        rpos += cl;
    }

    return true;
}
