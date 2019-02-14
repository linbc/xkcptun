/************************************************************************
 * file	:                                                                     
 * desc	:本文件包含了所有本工程所需要的所有内部头文件以及平台相关的函数包装
 *		外部使用请包含netstd.h
 *		注意:外部项目不要引用该头文件
 *
 * author:	linbc
 * date:	20110416
 * version:
 ************************************************************************/

#ifndef _NET_STD_tcp_connection_H
#define _NET_STD_tcp_connection_H

#include "debug.h"

#ifdef WIN32
#  define __INLINE__			__inline
#else
#  define __INLINE__			inline
#endif // WIN32


#ifdef WIN32
#  ifdef _MSC_VER
#    pragma comment(lib, "wsock32.lib")
#    pragma comment(lib, "ws2_32.lib")		//for getnameinfo
#    pragma warning(disable:4514)
#  endif

typedef unsigned long ipaddr_t;
typedef unsigned short port_t;
typedef int socklen_t;

#  ifndef FD_SETSIZE
#    define FD_SETSIZE 1024
#  endif

#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <ws2tcpip.h>

#  define MSG_NOSIGNAL 0
#  define SHUT_WR 1
#  define Errno WSAGetLastError()

static __INLINE__ int wsa_initializer(BOOL init)
{
#ifdef WIN32
	static WSADATA m_wsadata;
	if(init) {
		if (WSAStartup(0x101,&m_wsadata))
			exit(-1);
	}
	else WSACleanup();
#endif
	return 0;
}

static __INLINE__ const char *StrError(int x)
{
	static	char tmp[100];
	switch (x)
	{
	case 10004: return "Interrupted function call.";
	case 10013: return "Permission denied.";
	case 10014: return "Bad address.";
	case 10022: return "Invalid argument.";
	case 10024: return "Too many open files.";
	case 10035: return "Resource temporarily unavailable.";
	case 10036: return "Operation now in progress.";
	case 10037: return "Operation already in progress.";
	case 10038: return "Socket operation on nonsocket.";
	case 10039: return "Destination address required.";
	case 10040: return "Message too long.";
	case 10041: return "Protocol wrong type for socket.";
	case 10042: return "Bad protocol option.";
	case 10043: return "Protocol not supported.";
	case 10044: return "Socket type not supported.";
	case 10045: return "Operation not supported.";
	case 10046: return "Protocol family not supported.";
	case 10047: return "Address family not supported by protocol family.";
	case 10048: return "Address already in use.";
	case 10049: return "Cannot assign requested address.";
	case 10050: return "Network is down.";
	case 10051: return "Network is unreachable.";
	case 10052: return "Network dropped connection on reset.";
	case 10053: return "Software caused connection abort.";
	case 10054: return "Connection reset by peer.";
	case 10055: return "No buffer space available.";
	case 10056: return "Socket is already connected.";
	case 10057: return "Socket is not connected.";
	case 10058: return "Cannot send after socket shutdown.";
	case 10060: return "Connection timed out.";
	case 10061: return "Connection refused.";
	case 10064: return "Host is down.";
	case 10065: return "No route to host.";
	case 10067: return "Too many processes.";
	case 10091: return "Network subsystem is unavailable.";
	case 10092: return "Winsock.dll version out of range.";
	case 10093: return "Successful WSAStartup not yet performed.";
	case 10101: return "Graceful shutdown in progress.";
	case 10109: return "Class type not found.";
	case 11001: return "Host not found.";
	case 11002: return "Nonauthoritative host not found.";
	case 11003: return "This is a nonrecoverable error.";
	case 11004: return "Valid name, no data record of requested type.";

	default:
		break;
	}
	sprintf(tmp, "Winsock error code: %d", x);
	return tmp;
}
#else
/* ----------------------------------------*/
/* common unix includes / defines*/
#  include <unistd.h>
#  include <sys/time.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netinet/tcp.h>
/*#include <netdb.h>*/

/*for struct ifreq */
#include <net/if.h>
#include <net/if_arp.h>

/*for ioctl()*/
#include <sys/ioctl.h>

#include <fcntl.h>

/* all typedefs in this file will be declared outside the sockets namespace,*/
/* because some os's will already have one or more of the type defined.*/
typedef int SOCKET;
#  define Errno errno
#  define StrError strerror

/* WIN32 adapt*/
#  define closesocket close
#  define INVALID_SOCKET -1
#  define SOCKET_ERROR -1

#  ifndef INADDR_NONE
#    define INADDR_NONE ((unsigned long) -1)
#  endif /* INADDR_NONE*/

#  ifndef MSG_NOSIGNAL
#    define MSG_NOSIGNAL 0 // oops - thanks Derek
#  endif
#endif

static __INLINE__ int setnonblocking(int s,int bNb){

#ifdef _WIN32
	unsigned long l = bNb ? 1 : 0;	
	if (ioctlsocket(s, FIONBIO, &l))			
		return -1;		
#else
	fcntl (s, F_SETFD, FD_CLOEXEC);
	fcntl (s, F_SETFL, O_NONBLOCK);
#endif
	return 0;
}

static __INLINE__ int setnodelay(int fd, int enable)
{
#ifdef WIN32
#else
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
#endif
    return 0;
}


#ifdef WIN32
#	include <WinSock2.h>
#   include <mmsystem.h>
#   include <time.h>
static __INLINE__ int gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
	return (0);
}

#else
# if defined(__APPLE_CC__)
#   include <time.h>
# endif
#   include <sys/time.h>
#   include <sys/timeb.h>
#endif


#endif

