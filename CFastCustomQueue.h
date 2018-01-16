#ifndef __CFASTCUSTOMQUEUE_H__
#define __CFASTCUSTOMQUEUE_H__
#include "IQueue.h"
#include <vector>

class BTRefCounterBase
{
public:
	BTRefCounterBase() :m_nCount(0){}
	virtual ~BTRefCounterBase(){}
	inline void add_ref()
	{
#ifdef _WIN32
		InterlockedIncrement((long *)&m_nCount);
#else
		__sync_fetch_and_add((&m_nCount), 1);
#endif
	}
	inline void release()
	{
#ifdef _WIN32
		if (!(InterlockedDecrement((long *)&m_nCount))){
#else
		if (__sync_fetch_and_sub((&m_nCount), 1) == 1 ){	// e api no general
#endif
			delete this;
		}
	}
	inline long curref()
	{
		return m_nCount;
	}
private:
	long m_nCount;
};

template<typename _Entity>
class CustomAllocator
{
	typedef _Entity*	_EntityPtr;
	typedef std::vector<_EntityPtr>	_VecArray;
	typedef typename _VecArray::iterator	    _MLIt;
	typedef typename _VecArray::const_iterator _MLCIt;
	_VecArray		m_container;
public:
	CustomAllocator(){}
	~CustomAllocator(){}
	_EntityPtr allocate(){
		_MLCIt lit = m_container.begin();
		_MLCIt leit = m_container.end();
		while(lit != leit)
		{
			_EntityPtr pmb = *lit;
			if( pmb->curref() == 1 )
			{
				pmb->add_ref();
				return *lit;
			}
			lit++;
		}
		_EntityPtr pmb = new _Entity();
		pmb->add_ref();
		pmb->add_ref();
		m_container.push_back(pmb);
		return pmb;
	}
	void recycle( _EntityPtr memfree ){
		memfree->release();
		
	}
	void clear( ){
		_MLIt lit = m_container.begin();
//		_MLIt leit = m_container.end();
		while(lit != m_container.end())
		{
			_EntityPtr pmb = *lit;
			_MLIt ldit = lit;
			if( pmb->curref() == 1 )
			{
				lit = m_container.erase(ldit);
				delete pmb;
			}
			else
			{
				ortp_error("%s clear error curref:%ld", __FUNCTION__, pmb->curref() );
				lit++;
			}
		}
	}
	uint64_t get_total_size( ){
		return m_container.size();
	}
	void print_mempool_info( ){

	}
};

template<typename T>
class FastSafeQueue : public IBlockQueue<T>
{
	class node_t : public BTRefCounterBase{
	public:
		T value; 
		node_t *next;
		void reset(){
			value = T();
			next = 0;
		}
	};
/*	typedef struct _node_t{
		T value; 
		_node_t *next;
	}node_t;*/
public:
	virtual void wait(){
		ortp_mutex_lock(&b_lock);
		ortp_cond_wait(&b_cond, &b_lock);
		ortp_mutex_unlock(&b_lock);
	}
	virtual void notify(){
		ortp_cond_signal(&b_cond);
	}
	FastSafeQueue(void){
		node_t* node = m_allocate.allocate();//(node_t*)malloc(sizeof(node_t));
		node->next = NULL;
		head = tail = node;
		incnt = 0;
		outcnt = 0;
		ortp_mutex_init(&h_lock, 0);
		ortp_mutex_init(&t_lock, 0);
		ortp_mutex_init(&b_lock, 0);
		ortp_cond_init(&b_cond, 0);
	}

	virtual ~FastSafeQueue(void){
		clear();
		m_allocate.recycle(head);
		m_allocate.clear();
		ortp_mutex_destroy(&h_lock);
		ortp_mutex_destroy(&t_lock);
		ortp_mutex_destroy(&b_lock);
		ortp_cond_destroy(&b_cond);
	}
	virtual void push(T value){
		node_t* node = m_allocate.allocate();//(node_t*)malloc(sizeof(node_t));
		node->value = value;
		node->next = NULL;
//		ortp_mutex_lock(&t_lock);
		tail->next = node;
		tail = node;
		incnt++;
//		ortp_mutex_unlock(&t_lock);
	}
	virtual T	 peek(){
		return head->next->value;
	}
	virtual T    pop(){
		T result;
		dequeue(&result);
		return result;
	}
	virtual bool empty(){
		return head->next == NULL;
	}
	virtual int size(){
		int ns = incnt-outcnt;
		if( ns < 0 )
			ortp_warning("%s in:%d out:%d", __FUNCTION__, incnt, outcnt);
		return ns;
	}
	virtual void clear(){
		T remove;
		while(dequeue(&remove));
	}

	bool dequeue(T *pvalue) {
//		ortp_mutex_lock(&h_lock);
		node_t* node = head;
		node_t* new_head = node->next;
		if (new_head == NULL)
		{	
//			ortp_mutex_unlock(&h_lock);
			return FALSE;
		}
		*pvalue = new_head->value;
		head = new_head;
		outcnt++;
//		ortp_mutex_unlock(&h_lock);
		node->reset();
		m_allocate.recycle(node);
//		free(node);
		return TRUE;
	}
private:
	node_t *head; 
	node_t *tail;
	ortp_mutex_t h_lock;
	ortp_mutex_t t_lock;
	ortp_mutex_t b_lock;
	ortp_cond_t  b_cond;
	unsigned int incnt;
	unsigned int outcnt;
	CustomAllocator<node_t>  m_allocate;
};
template<typename T>
IQueue<T>* IQueue<T>::createqueue_fast_implement()
{
	return NULL;
}

template<typename T>
IBlockQueue<T>* IBlockQueue<T>::createqueue_block_implement()
{
	return new FastSafeQueue<T>();
}

#endif
