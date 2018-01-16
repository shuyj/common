#ifndef __IBTQUEUE_H__
#define __IBTQUEUE_H__
template<typename T>
class IQueue
{
public:
	static IQueue<T>* createqueue_fast_implement();
	virtual ~IQueue(){}
	virtual void push(T t) = 0;
	virtual T	 peek() = 0;
	virtual T    pop() = 0;
	virtual bool empty() = 0;
	virtual int  size() = 0;
	virtual void clear() = 0;
};

template<typename T>
class IBlockQueue
{
public:
	static IBlockQueue<T>* createqueue_block_implement();
	virtual ~IBlockQueue(){}
	virtual void push(T t) = 0;
	virtual T	 peek() = 0;
	virtual T    pop() = 0;
	virtual bool empty() = 0;
	virtual int  size() = 0;
	virtual void clear() = 0;
	virtual void wait() = 0;
	virtual void notify() = 0;
};

#endif
