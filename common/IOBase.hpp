//
//  IOBase.hpp
//  devbase
//
//  Created by yajun18 on 2021/12/14.
//  Copyright © 2021 shuyj. All rights reserved.
//

#ifndef IOBase_hpp
#define IOBase_hpp

#include <memory>


// 输入输出接口
/*
                            组件(输入输出个数，输入输出类型，流动数据结构，推拉模式)
 线程驱动
 
 
 上下连接关系
 输入{ 类型，编号，推拉模式 } 输出{ 类型， 编号， 推拉模式}

    对象感知向下连接
    集合<目标>      只能推模式       如果数据流动让每个对象不感知，交由外部来控制会不会更好
 
 
    每个对象不感知上下连接
    源-读   处理-写读  目标-写    读写的参数都相同  可以有效解决 进1出非1问题
    线程处理逻辑为：  首先拿到一个链， 从头开始进行 读  然后写入下一个对象中，在读出来在写入下一个...    是一种推模式。 这个方案实现拉模式比较复杂， 需要找上一个对象读，读不到时在找上一个对象读，当读取到时在向下写入数据
    内部为异步时: 处理完回调时，存入队列。等到 调用 读时在取出返回
    一对多: 需要在非 源 的filter中多次读取直到eof
    《如何实现分叉和整合  每个对象都需要是一个数组。 整个链是一个二维数组。 执行完第一组，切换到下一组。 每组产生的数据存入一个结果数组，下次按组下标进行 》  建立连接图谱： 包含连接关系，产生的数据写入给谁。 产生的不同类型数据？是否区分类型
 
 
    对象感知上下连接      线程在一头驱动             每个对象都是一个Runnable，这样让线程与对象分离
    集合<源>          拉模式使用，调用源的读取
    集合<目标>        推模式使用，调用目标的写入
    源  处理1 处理2 处理3 处理4  目标
 
 */
// 无pin
// 参数可变，可尝试模板化
// 参数内部支持多类型，INFO, META, DATA
// 驱动单元与接口独立
// 解耦性大于简洁性

// avfilter  filter(filter定义) filter_context(filter实例)  filterInOut(处理上下连接接口)    filtergraph(filter容器)   可提取共用部分in,out与连接graph部分
/* 结论： 1.连接关系上，只保留单方向会导致无法支持，推，拉完整模式。 所以需要上下关系都保留
         2.为了让实现的处理逻辑与框架尽可能的隔离开，让处理逻辑部分在一个非常简洁接口上实现，可能的话框架部分可以有多个实现，处理逻辑部分可以无缝切换到其它框架上
         分层拆分：  1、线程驱动层
 Core{Runnable}
                   2、pipeline链路层
 List{ Unit{} }
                   3. 上下连接关系层
 Unit{ reader, readerID, writer, writerID } 一个输入输出关系
                   3、处理逻辑层
 Type STDData;
 Reader{ STDData read(编号); }
 Writer{ void write(编号，STDData); }
 //Transform{ int write(编号，StdData);  STDData read(编号); }
 
 simplest
 filter{ in_queue[], out_queue[],
 
 
 只实现逻辑层， 线程和上下关系极简化，可生成一些工具类来帮助构建关系。   如何简单方便的来实现多输入和多输出
 
 
 
 */

class ACacheBuffer {
public:
    ACacheBuffer(int capacity);
    int freeSpace();
    int len();
    bool write(const char* pb, int len);
    bool read(char* pb, int len);

private:
    std::unique_ptr<char[]>     _buf;
    int                         _capacity;
    int                         _bufPosw;
    int                         _bufPosr;
};

#endif /* IOBase_hpp */
