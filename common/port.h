/*
  The oRTP library is an RTP (Realtime Transport Protocol - rfc3550) stack.
  Copyright (C) 2001  Simon MORLAT simon.morlat@linphone.org

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/* this file is responsible of the portability of the stack */

#ifndef ORTP_PORT_H
#define ORTP_PORT_H

/* define the debug mode */
#define RTP_DEBUG 1

/* enables SO_REUSEADDR socket option in the rtp_session_set_local_addr() function */
#define SO_REUSE_ADDR 1

#define _CRT_SECURE_NO_WARNINGS


#if !defined(WIN32) && !defined(_WIN32_WCE)
/********************************/
/* definitions for UNIX flavour */
/********************************/

#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <dlfcn.h>
#include <time.h>
#ifdef __linux
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#endif

#if defined(__APPLE__) && defined(__GNUC__)
#include <objc/objc.h>
extern void mac_logv(const char *format, va_list args);
#else
#include <sys/prctl.h>
#endif

#define FLLD	"%lld"
#define FLLU	"%llu"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if defined(_XOPEN_SOURCE_EXTENDED) || !defined(__hpux)
#include <arpa/inet.h>
#endif



#include <sys/time.h>
#ifdef ORTP_INET6
#include <netdb.h>
#endif
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE==1
typedef unsigned long long  ortp_thread_id;
#else
typedef pthread_t  ortp_thread_id;
#endif
typedef int ortp_socket_t;
typedef pthread_t ortp_thread_t;
typedef pthread_mutex_t ortp_mutex_t;
typedef pthread_cond_t ortp_cond_t;
typedef pthread_key_t	ortp_key_t;
typedef void*			ortp_lib_t;

#ifdef __INTEL_COMPILER
#pragma warning(disable : 111)		// statement is unreachable
#pragma warning(disable : 181)		// argument is incompatible with corresponding format string conversion
#pragma warning(disable : 188)		// enumerated type mixed with another type
#pragma warning(disable : 593)		// variable "xxx" was set but never used
#pragma warning(disable : 810)		// conversion from "int" to "unsigned short" may lose significant bits
#pragma warning(disable : 869)		// parameter "xxx" was never referenced
#pragma warning(disable : 981)		// operands are evaluated in unspecified order
#pragma warning(disable : 1418)		// external function definition with no prior declaration
#pragma warning(disable : 1419)		// external declaration in primary source file
#pragma warning(disable : 1469)		// "cc" clobber ignored
#endif

#define ORTP_PUBLIC

#ifdef __cplusplus
extern "C"
{
#endif

int __ortp_thread_join(ortp_thread_t thread, void **ptr);
int __ortp_thread_create(pthread_t *thread, pthread_attr_t *attr, void * (*routine)(void*), void *arg);
int pthread_cond_timewait_(pthread_cond_t *cond, pthread_mutex_t *mutex, int msseconds);
ORTP_PUBLIC ortp_thread_id IOS_thread_gid();
#ifdef __cplusplus
}
#endif

#define ortp_thread_create	__ortp_thread_create
#define ortp_thread_join	__ortp_thread_join
#define ortp_thread_exit	pthread_exit
#if defined(TARGET_OS_IPHONE) || defined(TARGET_OS_OSX)
#define ortp_thread_self    IOS_thread_gid  // pthread_mach_thread_np
#define ortp_thread_sname(tname)    pthread_setname_np(tname)
#define ortp_thread_gname(tname)    pthread_getname_np(pthread_self(), tname, sizeof(tname))
#else
#define ortp_thread_self	gettid//pthread_self
#define ortp_thread_sname(tname)	prctl(PR_SET_NAME, tname)
#define ortp_thread_gname(tname)	prctl(PR_GET_NAME, tname)
#endif
#define ortp_process_self	getpid
#define ortp_mutex_init		pthread_mutex_init
#define ortp_mutex_lock		pthread_mutex_lock
#define ortp_mutex_trylock  pthread_mutex_trylock
#define ortp_mutex_unlock	pthread_mutex_unlock
#define ortp_mutex_destroy	pthread_mutex_destroy
#define ortp_cond_init		pthread_cond_init
#define ortp_cond_signal	pthread_cond_signal
#define ortp_cond_broadcast	pthread_cond_broadcast
#define ortp_cond_wait		pthread_cond_wait
#define ortp_cond_timewait	pthread_cond_timewait_
#define ortp_cond_destroy	pthread_cond_destroy
#define ortp_key_create		pthread_key_create
#define ortp_key_set		pthread_setspecific
#define ortp_key_get		pthread_getspecific
#define ortp_key_destroy	pthread_key_delete

#define ortp_lib_open		dlopen
#define ortp_lib_find		dlsym
#define ortp_lib_close		dlclose
#define ortp_lib_error		dlerror

#define SOCKET_OPTION_VALUE	void *
#define SOCKET_BUFFER		void *

#define getSocketError() strerror(errno)
#define getSocketErrorCode() (errno)

#define ortp_log10f(x)	log10f(x)


#else
/*********************************/
/* definitions for WIN32 flavour */
/*********************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <winsock2.h>
#include <ws2tcpip.h>


#ifdef _MSC_VER
#define ORTP_PUBLIC	__declspec(dllexport)
#pragma push_macro("_WINSOCKAPI_")
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

typedef  unsigned __int64 uint64_t;
typedef  __int64 int64_t;
typedef  unsigned short uint16_t;
typedef  unsigned int uint32_t;
typedef  int int32_t;
typedef  unsigned char uint8_t;
typedef __int16 int16_t;
#else
#include <stdint.h> /*provided by mingw32*/
#include <io.h>
#define ORTP_PUBLIC
#endif

#define vsnprintf	_vsnprintf
#define srandom		srand
#define random		rand

#define FLLD	"%I64d"
#define FLLU	"%I64u"

typedef SOCKET ortp_socket_t;
typedef HANDLE ortp_cond_t;
typedef HANDLE ortp_mutex_t;
typedef HANDLE ortp_thread_t;
typedef DWORD  ortp_thread_id;
typedef DWORD  ortp_key_t;
typedef HMODULE ortp_lib_t;

#define ortp_thread_create	WIN_thread_create
#define ortp_thread_join	WIN_thread_join
#define ortp_thread_exit(arg)
#define ortp_thread_self	GetCurrentThreadId
#define ortp_thread_sname(tname) WIN_thread_sname(tname)
#define ortp_thread_gname()	WIN_thread_gname()

#define ortp_process_self	GetCurrentProcessId

#define ortp_mutex_init		WIN_mutex_init
#define ortp_mutex_lock		WIN_mutex_lock
#define ortp_mutex_unlock	WIN_mutex_unlock
#define ortp_mutex_destroy	WIN_mutex_destroy
#define ortp_cond_init		WIN_cond_init
#define ortp_cond_signal	WIN_cond_signal
#define ortp_cond_broadcast	WIN_cond_broadcast
#define ortp_cond_wait		WIN_cond_wait
#define ortp_cond_timewait	WIN_cond_timewait
#define ortp_cond_destroy	WIN_cond_destroy
#define ortp_key_create		WIN_key_create
#define ortp_key_set		TlsSetValue
#define ortp_key_get		TlsGetValue
#define ortp_key_destroy	TlsFree
#define RTLD_NOW	0
#define ortp_lib_open		WIN_lib_open
#define ortp_lib_find		GetProcAddress
#define ortp_lib_close		FreeLibrary
#define ortp_lib_error		GetLastError

#ifdef __cplusplus
extern "C"
{
#endif

ORTP_PUBLIC int WIN_mutex_init(ortp_mutex_t *m, void *attr_unused);
ORTP_PUBLIC int WIN_mutex_lock(ortp_mutex_t *mutex);
ORTP_PUBLIC int WIN_mutex_unlock(ortp_mutex_t *mutex);
ORTP_PUBLIC int WIN_mutex_destroy(ortp_mutex_t *mutex);
ORTP_PUBLIC int WIN_thread_create(ortp_thread_t *t, void *attr_unused, void *(*func)(void*), void *arg);
ORTP_PUBLIC int WIN_thread_join(ortp_thread_t thread, void **unused);
ORTP_PUBLIC int WIN_cond_init(ortp_cond_t *cond, void *attr_unused);
ORTP_PUBLIC int WIN_cond_wait(ortp_cond_t * cond, ortp_mutex_t * mutex);
ORTP_PUBLIC int WIN_cond_timewait(ortp_cond_t * cond, ortp_mutex_t * mutex, int msseconds);
ORTP_PUBLIC int WIN_cond_signal(ortp_cond_t * cond);
ORTP_PUBLIC int WIN_cond_broadcast(ortp_cond_t * cond);
ORTP_PUBLIC int WIN_cond_destroy(ortp_cond_t * cond);
ORTP_PUBLIC int WIN_key_create(ortp_key_t *m, void (*cleanup)(void*));
ORTP_PUBLIC ortp_lib_t WIN_lib_open(char* lpLibFileName, int unuse );
ORTP_PUBLIC int WIN_thread_sname(const char* tname);
ORTP_PUBLIC const char* WIN_thread_gname();
#ifdef __cplusplus
}
#endif

#define SOCKET_OPTION_VALUE	char *
#ifndef __cplusplus
#define inline			__inline
#endif
#if defined(_WIN32_WCE)

#define ortp_log10f(x)		(float)log10 ((double)x)

#ifdef assert
	#undef assert
#endif /*assert*/
#define assert(exp)	((void)0)

#ifdef errno
	#undef errno
#endif /*errno*/
#define  errno GetLastError()
#ifdef strerror
		#undef strerror
#endif /*strerror*/
const char * ortp_strerror(DWORD value);
#define strerror ortp_strerror


#else /*_WIN32_WCE*/

#define ortp_log10f(x)	log10f(x)

#endif

ORTP_PUBLIC const char *getWinSocketError(int error);
#define getSocketErrorCode() WSAGetLastError()
#define getSocketError() getWinSocketError(WSAGetLastError())

#define snprintf _snprintf
#define strcasecmp _stricmp


#ifdef __cplusplus
extern "C"{
#endif
int gettimeofday (struct timeval *tv, void* tz);
#ifdef _WORKAROUND_MINGW32_BUGS
char * WSAAPI gai_strerror(int errnum);
#endif
#ifdef __cplusplus
}
#endif

#endif

typedef unsigned char bool_t;
#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

typedef struct ortpTimeSpec{
	int64_t tv_sec;
	int64_t tv_nsec;
}ortpTimeSpec;

#ifdef __cplusplus
extern "C"{
#endif

ORTP_PUBLIC void* ortp_malloc(size_t sz);
ORTP_PUBLIC void ortp_free(void *ptr);
ORTP_PUBLIC void* ortp_realloc(void *ptr, size_t sz);
ORTP_PUBLIC void* ortp_malloc0(size_t sz);
ORTP_PUBLIC char * ortp_strdup(const char *tmp);

/*override the allocator with this method, to be called BEFORE ortp_init()*/
typedef struct _OrtpMemoryFunctions{
	void *(*malloc_fun)(size_t sz);
	void *(*realloc_fun)(void *ptr, size_t sz);
	void (*free_fun)(void *ptr);
}OrtpMemoryFunctions;

void ortp_set_memory_functions(OrtpMemoryFunctions *functions);

#define ortp_new(type,count)	(type*)ortp_malloc(sizeof(type)*(count))
#define ortp_new0(type,count)	(type*)ortp_malloc0(sizeof(type)*(count))

ORTP_PUBLIC int close_socket(ortp_socket_t sock);
ORTP_PUBLIC int set_non_blocking_socket(ortp_socket_t sock);

ORTP_PUBLIC char *ortp_strndup(const char *str,int n);
ORTP_PUBLIC char *ortp_strdup_printf(const char *fmt,...);
ORTP_PUBLIC char *ortp_strdup_vprintf(const char *fmt, va_list ap);

ORTP_PUBLIC int ortp_file_exist(const char *pathname);

ORTP_PUBLIC void ortp_get_cur_time(ortpTimeSpec *ret);

ORTP_PUBLIC uint64_t get_wallclock_ms(void);

/* portable named pipes  and shared memory*/
#if !defined(_WIN32_WCE)
#ifdef WIN32
typedef HANDLE ortp_pipe_t;
#define ORTP_PIPE_INVALID INVALID_HANDLE_VALUE
#else
typedef int ortp_pipe_t;
#define ORTP_PIPE_INVALID (-1)
#endif

ORTP_PUBLIC ortp_pipe_t ortp_server_pipe_create(const char *name);
/*
 * warning: on win32 ortp_server_pipe_accept_client() might return INVALID_HANDLE_VALUE without
 * any specific error, this happens when ortp_server_pipe_close() is called on another pipe.
 * This pipe api is not thread-safe.
*/
ORTP_PUBLIC ortp_pipe_t ortp_server_pipe_accept_client(ortp_pipe_t server);
ORTP_PUBLIC int ortp_server_pipe_close(ortp_pipe_t spipe);
ORTP_PUBLIC int ortp_server_pipe_close_client(ortp_pipe_t client);

ORTP_PUBLIC ortp_pipe_t ortp_client_pipe_connect(const char *name);
ORTP_PUBLIC int ortp_client_pipe_close(ortp_pipe_t sock);

ORTP_PUBLIC int ortp_pipe_read(ortp_pipe_t p, uint8_t *buf, int len);
ORTP_PUBLIC int ortp_pipe_write(ortp_pipe_t p, const uint8_t *buf, int len);

ORTP_PUBLIC void *ortp_shm_open(unsigned int keyid, int size, int create);
ORTP_PUBLIC void ortp_shm_close(void *memory);

#endif

#ifdef __cplusplus
}

#endif


#if (defined(WIN32) || defined(_WIN32_WCE)) && !defined(ORTP_STATIC)
#ifdef ORTP_EXPORTS
   #define VAR_DECLSPEC    __declspec(dllexport)
#else
   #define VAR_DECLSPEC    __declspec(dllimport)
#endif
#else
   #define VAR_DECLSPEC    extern
#endif

static void sleepMs(int ms){
#ifdef WIN32
#if 1//WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	Sleep(ms);
#else
	HANDLE sleepEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
	if (!sleepEvent) return;
	WaitForSingleObjectEx(sleepEvent, ms, FALSE);
#endif
#else
	struct timespec ts;
	ts.tv_sec = ms/1000;
	ts.tv_nsec = (ms%1000) * 1000000LL;
	nanosleep(&ts, NULL);
#endif
}

/*define __ios when we are compiling for ios.
 The TARGET_OS_IPHONE macro is stupid, it is defined to 0 when compiling on mac os x.
*/
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE==1
#define __ios 1
#endif

#endif


