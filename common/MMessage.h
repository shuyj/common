//
//  MMessage.h
//  ProxyPlayer
//
//  Created by shuyj on 2018/4/19.
//  Copyright © 2018年 MOMO. All rights reserved.
//

#ifndef MMessage_h
#define MMessage_h
#include <vector>

class AnyValue {
public:
    virtual ~AnyValue(){}
    
    template<typename ValueType>
    ValueType any_cast();
};


template <class T>
class MValue : public AnyValue
{
public:
    MValue(const T& t):data(t){ }
//    MValue(T&& t):data(t) { }
    operator T&() { return data; }
    operator const T& () const { return data; }
private:
    T data;
};

template<typename ValueType>
ValueType AnyValue::any_cast() {
    return (*(static_cast<MValue<ValueType>*>(this)));
}

class MAnyValue {
public:
    MAnyValue() : content_(NULL) {}
    template<typename ValueType>
    MAnyValue(const ValueType& value) : content_(new MValue<ValueType>(value)) {
    }
    ~MAnyValue() {
        delete content_;
    }
    template<typename ValueType>
    ValueType* any_cast(){
        if (content_ == NULL)
            return NULL;
        else
            return static_cast<MValue<ValueType> *>(content_);
    }
private:
    AnyValue* content_;
};

template< typename T >
class MMessage{
public:
    T                         msg;
    std::vector<AnyValue>      params;
    MMessage():msg(0){};
    MMessage(const T& t){ msg = t;}
    MMessage(T&& t){ msg = t;}
    MMessage(T&& t, AnyValue first){ msg = t; params.push_back(first); }
    MMessage(T&& t, AnyValue first, AnyValue second){ msg = t; params.push_back(first);params.push_back(second); }
};

#endif /* MMessage_h */
