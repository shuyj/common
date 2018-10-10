#ifndef __CBTTHREAD_H__
#define __CBTTHREAD_H__

#include "port.h"
#include "logging.h"

class CThread
{
public:
	CThread(const char* tname);
	virtual ~CThread(void);
	int thread_start();
	int thread_stop();
	int thread_join();
	bool thread_is_run();
	virtual int	thread_run()=0;
private:
	static  void*	thread_grun(void* params);
protected:
	ortp_thread_t	m_tid;
	ortp_thread_id	m_ntid;
	char			m_tname[64];
	bool			m_bactive;
};

#endif
