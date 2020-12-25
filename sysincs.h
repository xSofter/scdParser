/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*      	  1986 - 2008, All Rights Reserved.		        */
/*									*/
/*		    PROPRIETARY AND CONFIDENTIAL			*/
/*									*/
/* MODULE NAME : sysincs.h						*/
/* PRODUCT(S)  : MMSEASE						*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*	The purpose of this include file is to bring in include files	*/
/*	that come with one of the various C compilers.			*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 03/27/08  EJV     48    Defined S_MAX_PATH for all systems.		*/
/*			   history.					*/
/************************************************************************/

#ifndef SYSINCS_INCLUDED
#define SYSINCS_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__OS2__)
#define INCL_BASE	/* include all OS2 definitions.			*/
#include <os2.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#ifdef kbhit		/* The OS/2 2.0 version is not what we want	*/
#undef kbhit
#endif
#include <process.h>		/* for _beginthread, _endthread	*/
#include <signal.h>		/* for "kill", etc.		*/
/* Sockets related includes	*/
#include <sys/socket.h>
#include <netinet/in.h>		/* IPPROTO_*, etc.			*/
#include <netdb.h>		/* gethostbyname, etc.			*/
#include <sys/ioctl.h>		/* defines FIONBIO			*/
#endif	/* OS2	*/

#if defined (_WIN32)
#if !defined (_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE
#endif
#pragma warning(disable : 4996)
#pragma warning(disable : 4786 4800)

#if defined (_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES)
#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#endif

/* winsock2.h MUST ALWAYS BE BEFORE windows.h to override defs in	*/
/* winsock.h (included by windows.h). Any module that includes windows.h*/
/* before sysincs.h, must also include winsock2.h before windows.h.	*/
#include <winsock2.h>		/* must be before windows.h	*/
#include <windows.h>
#include <process.h>		/* for _beginthread, _endthread	*/
#include <sys/timeb.h>		/* for ftime, timeb		*/
#endif  /* defined(_WIN32) */

#if defined(_WIN32) || defined(MSDOS) || defined(__MSDOS__)
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <conio.h>
#endif

#if defined(VXWORKS)
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <selectLib.h>
#include <limits.h>
#include <signal.h>		/* for "kill", etc.		*/
/* Sockets related includes	*/
#include <sys/socket.h>
#include <ioLib.h>
#include <sockLib.h>
#include <pipeDrv.h>
#include <sysLib.h>
#include <usrLib.h>
#include <netinet/in.h>		/* IPPROTO_*, etc.			*/
#include <arpa/inet.h>		/* inet_addr, etc.			*/
#include <netinet/tcp.h>	/* TCP_NODELAY, etc.			*/
#include <dirent.h>
#endif

#if defined(__QNX__)
/* FD_SETSIZE default is only 32 on QNX. Must define larger value	*/
/* here (before any system includes) to allow more TCP connections.	*/
#define FD_SETSIZE	600
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <conio.h>
#include <sys/types.h>
#include <sys/stat.h>		/* S_IFIFO, S_IRUSR, S_IWUSR, etc.	*/
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>		/* mknod, etc.				*/
#include <unix.h>	        /* added for "ftruncate" */
#include <fcntl.h>		/* open, O_RDONLY, O_WRONLY, etc.	*/
#include <process.h>		/* execlp, etc.				*/
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <termio.h>
#include <signal.h>		/* for "kill", etc.		*/
#include <sys/timeb.h>		/* for ftime, timeb		*/
/* Sockets related includes	*/
#include <sys/socket.h>
#include <netdb.h>		/* gethostbyname, etc.			*/
#include <netinet/in.h>		/* IPPROTO_*, etc.			*/
#include <arpa/inet.h>		/* inet_addr, etc.			*/
#include <netinet/tcp.h>	/* TCP_NODELAY, etc.			*/
#include <dirent.h>
#endif

/* UNIX or "UNIX-like" systems	*/
#if defined(_AIX) || defined(sun) || defined(__hpux) || defined(linux) \
    || (defined(__alpha) && !defined(__VMS)) || defined(__LYNX)
#include <unistd.h>	/* SHOULD be before any other include files 	*/
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#if (!defined(__LYNX))
#include <sys/time.h>
#include <sys/resource.h>
#endif
#define	max(a,b)	(((a) > (b)) ? (a) : (b))
#define	min(a,b)	(((a) < (b)) ? (a) : (b))
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <fcntl.h>		/* open, O_RDONLY, O_WRONLY, etc.	*/
#include <ctype.h>
#include <limits.h>

#if defined(linux)
#include <sys/timeb.h>		/* for ftime, timeb		*/
#endif

#if defined(__hpux)
#include <sys/termios.h>
#else
#include <termio.h>
#endif

#if (!defined (__hpux)) && (!defined(__LYNX))
#include <sys/select.h>
#endif
#include <signal.h>		/* for "kill", etc.		*/
#include <sys/ioctl.h>
#if defined(sun)
#include <sys/filio.h>
#endif
/* Sockets related includes	*/
#if defined(__LYNX)
#include <socket.h>
#else
#include <sys/socket.h>
#endif
#include <netdb.h>		/* gethostbyname, etc.			*/
#include <netinet/in.h>		/* IPPROTO_*, etc.			*/
#include <sys/un.h>		/* for sockaddr_un			*/
/* Forward references are supplied to eliminate xlC_r compiler warnings	*/
struct ether_addr;		/* forward reference			*/
struct sockaddr_dl;		/* forward reference			*/
#include <arpa/inet.h>		/* inet_addr, etc.			*/
#include <netinet/tcp.h>	/* TCP_NODELAY, etc.			*/
#ifndef INADDR_NONE
#define INADDR_NONE   ((in_addr_t) 0xffffffff)
#endif
#endif /* defined(_AIX) || defined(sun) || defined(__hpux) || defined(linux) ... */



#if defined(__VMS)
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <stat.h>

#include <descrip.h>        /* VMS descriptor stuff */
#include <iodef.h>          /* I/O FUNCTION CODE DEFS */
#include <lib$routines.h>   /* LIB$ RTL-routine signatures. */
#include <libdtdef.h>       /* LIB$ RTL-routine signatures. */
#include <libdef.h>         /* LIB$ RTL-routine signatures. */
#include <ssdef.h>          /* SS$_<xyz> sys ser return stati <8-) */
#include <starlet.h>        /* Sys ser calls */
#include <wchar.h>          /* ... */
#include <cvt$routines.h>   /* Convert floating-point data type */
#include <cvtdef.h>         /* ... */
#include <signal.h>         /* UNIX style Signal Value Definitions */
#include <errno.h>
#include <errnodef.h>
#include <unistd.h>

#include "stdarg.h"
#include "iostream.h"
#include <limits.h>
#include <ctype.h>
#include <timers.h>

#include <tcp.h>            /* TCP descriptions */
#include <in.h>             /* internet system Constants and structures. */
#include <inet.h>           /* Network address info. */
#include <netdb.h>          /* Network database library info. */
#include <socket.h>         /* TCP/IP socket definitions. */
#include <ucx$inetdef.h>
#include <ioctl.h>          /* Operations on socket (?) */

#define	max(a,b)	((a > b) ? a : b)
#define	min(a,b)	((a < b) ? a : b)
#endif

#if (!defined(INT_MAX)) || (!defined(LONG_MAX))
#error INT_MAX and LONG_MAX must be defined. Usually defined in limits.h
#endif

#if defined(MAX_PATH)
#define S_MAX_PATH   MAX_PATH
#elif defined(PATH_MAX)    /* POSIX should have it defined in limits.h    */
#define S_MAX_PATH    PATH_MAX
#else
#define S_MAX_PATH    1024   /* default   */
#endif

	/*----------------------------------------------*/
	/* 	printf, sprintf, sscanf helper macros	*/
	/*----------------------------------------------*/

/* helper macro for 32-bit and 64-bit pointers support			*/
/* If pointer "0x%p" format is not supported then something like "0x%x",*/
/* "0x%lx", or "0x%llx" may be used, depending on the pointer size.    */
#if defined(_WIN32)
  #define S_FMT_PTR             "0x%p"
#else  /* all other systems (e.g. UNIX)	*/
  #define S_FMT_PTR             "0x%p"
#endif /* all other systems (e.g. UNIX)	*/

/* helper macro for time_t	*/
#if defined(_WIN32)
  #if defined(_USE_32BIT_TIME_T)
    #define S_FMT_TIME_T        "%d"
  #else
    #define S_FMT_TIME_T        "%I64d"
  #endif
#else  /* all other systems (e.g. UNIX)	*/
  #define S_FMT_TIME_T          "%d"
#endif /* all other systems (e.g. UNIX)	*/

#ifdef INT64_SUPPORT
#ifdef _WIN32
  #define S_FMT_INT64           "%I64d"
  #define S_FMT_UINT64          "%I64u"
#elif defined(_AIX) || defined(__hpux) || defined(linux) || defined(sun) || defined(__LYNX)
  #define S_FMT_INT64           "%lld"
  #define S_FMT_UINT64          "%llu"
#elif (defined(__alpha) && !defined(__VMS)) 
  #define S_FMT_INT64           "%ld"
  #define S_FMT_UINT64          "%lu"
#else  /* all other systems */
  #error Missing S_FMT_INT64 and S_FMT_UINT64 defines for this platform.
#endif /* all other systems */
#endif /* INT64_SUPPORT */

/* helper macro for HANDLE	*/
#if defined(_WIN32)
  #if (_MSC_VER >= 1300)
  #define S_FMT_HANDLE          "0x%p"
  #define S_FMT_THREAD_HANDLE   "0x%p"
  #else
  #define S_FMT_HANDLE          "%d"
  #define S_FMT_THREAD_HANDLE   "%lu"
  #endif
#else  /* all other systems (e.g. UNIX)	*/
  #define S_FMT_THREAD_HANDLE   "0x%p"
#endif /* all other systems (e.g. UNIX)	*/


/************************************************************************/
/************************************************************************/
/*		Assert stuff						*/
/************************************************************************/
/************************************************************************/

#include <assert.h>

#if defined (_WIN32) && defined (_DEBUG)
#include <crtdbg.h>
#endif

#if !defined (C_ASSERT)
#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]
#endif

#if !defined (ASSERT)
#define ASSERT		_ASSERT
#endif

#if !defined (ASSERTE)
#define ASSERTE		_ASSERTE
#endif

#if !defined (_SASSERT)
#define _SASSERT(e) \
	{ \
	int assertResult = (e) ? 1 : 0; \
	if (!assertResult) {SLOG_INFO3("Assertion failed: '%s', file %s, line %d", #e, __FILE__, __LINE__);} \
	assert (assertResult); \
	}
#endif

#if !defined (_SASSERTE)
#define _SASSERTE(e) \
	{ \
	int assertResult = (e) ? 1 : 0; \
	if (!assertResult) {SLOG_INFO3("Assertion failed: '%s', file %s, line %d", #e, __FILE__, __LINE__);} \
	assert (assertResult); \
	}
#endif

#if !defined (SASSERT)
#define SASSERT		_SASSERT
#endif

#if !defined (SASSERTE)
#define SASSERTE	_SASSERTE
#endif

#if !defined (VERIFY)
#if !defined (NDEBUG)
#define VERIFY(e)	_SASSERTE(e)
#else
#define VERIFY(e)	((void) (e))
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif


