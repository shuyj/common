#include "CThread.h"


CThread::CThread(const char* tname):m_tid(0),m_ntid(0)
{
	strcpy(m_tname, tname);
	m_bactive = false;
}

CThread::~CThread(void)
{
	if ( m_tid )
		ortp_thread_join(m_tid, NULL);
}
bool CThread::thread_is_run()
{
	return m_bactive;
}
int CThread::thread_start(){
	m_bactive = true;
	ortp_thread_create(&m_tid, NULL, thread_grun, this);
	return 0;
}

int CThread::thread_stop(){
	m_bactive = false;
	return 0;
}

int CThread::thread_join(){
	ortp_thread_join(m_tid, NULL);
	m_tid = 0;
	return 0;
}

void*	CThread::thread_grun(void* params)
{
	CThread* pfilter = (CThread*)params;
	pfilter->m_ntid = ortp_thread_self();
	ortp_thread_sname(pfilter->m_tname);
	ortp_warning("thread start tid:%lld name:%s", pfilter->m_ntid, pfilter->m_tname);
	pfilter->thread_run();
	ortp_warning("thread end tid:%lld name:%s", pfilter->m_ntid, pfilter->m_tname);
	return NULL;
}
