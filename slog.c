/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*		1993-2008, All Rights Reserved 				*/
/*									*/
/* MODULE NAME : slog.c							*/
/* PRODUCT(S)  : SLOG							*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 03/27/08  EJV     61    Use S_MAX_PATH instead of MAX_PATH.		*/
/*			   history.					*/
/************************************************************************/

#if defined(__OS2__)
#pragma data_seg(alldata)
#define INCL_BASE
#define INCL_DOS
#define INCL_DOSMISC
#define INCL_ERRORS
#define INCL_DOSPROCESS
#define INCL_DOSQUEUES
#define INCL_DOSSEMAPHORES
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSDATETIME
#define INCL_DOSDEVICES
#include <os2.h>
#include <stddef.h>
#endif

#include "glbtypes.h"
#include "sysincs.h"
#include "slog.h"
#include "str_util.h"
#include <time.h>

#if defined (_WIN32)
#include <windows.h>
#endif
#include <sys/time.h>
static LOG_CTRL g_lc;
static LOG_CTRL g_usrLog;
ST_UINT32 g_logType;
ST_CHAR* g_sourceFile;
ST_INT g_lineNum;
ST_CHAR* g_funName;

#ifdef DEBUG_SISCO
static SD_CONST ST_CHAR * thisFileName = __FILE__;
#endif

/************************************************************************/
/* Other prototypes.							*/
/************************************************************************/

static ST_VOID doSlog (LOG_CTRL *lc, 
					   SD_CONST ST_INT logType, 
					   SD_CONST ST_CHAR *SD_CONST logTypeStr, 
					   SD_CONST ST_CHAR *SD_CONST sourceFile,
					   SD_CONST ST_INT lineNum, 
					   SD_CONST ST_CHAR *SD_CONST format, va_list ap);


/************************************************************************/
/*                               slog                                   */
/************************************************************************/

ST_VOID slogHead (SD_CONST ST_INT logType,
				SD_CONST ST_CHAR *SD_CONST sourceFile, 
				SD_CONST ST_CHAR *SD_CONST functionName,
			  	SD_CONST ST_INT lineNum)
{
	g_logType=logType;
	g_sourceFile=sourceFile;
	g_lineNum=lineNum;
	g_funName = functionName;
}

/************************************************************************/
/*                               slog                                   */
/************************************************************************/

ST_VOID slog (SD_CONST ST_CHAR *SD_CONST format, ...)
{
	va_list	ap;
	// SD_CONST ST_UINT logType=g_logType;
	// SD_CONST ST_CHAR *SD_CONST sourceFile=g_sourceFile;
	// SD_CONST ST_INT lineNum=g_lineNum;
	// SD_CONST ST_CHAR *SD_CONST functionName=g_funName;
	
	//小于本级别的日志,不记录
	if(g_logType < g_lc.logCtrl )
	{
		// printf("logType %d not equals %d.\n", g_logType, g_lc.logType);
		return;
	}

	va_start (ap, format);
	doSlog (&g_lc, g_logType, g_funName, g_sourceFile, g_lineNum, format, ap);
	va_end(ap);
}

ST_VOID userLog (SD_CONST ST_CHAR *SD_CONST format, ...)
{
	
	ST_INT 	count;
	va_list	ap;
	ST_CHAR msg_buf[1024] = {0};
	ST_CHAR tmpBuf[128];
	va_start (ap, format);		
	g_usrLog.max_msg_size = 1024;
	if (format == NULL)
	{
		/* make buf a zero length string just in case			*/
		count = 0;
		msg_buf[0] = 0;
	} else {
#if defined(_WIN32)
		count = _vsnprintf(msg_buf,1024,format,ap);
#elif  defined(__QNX__) && defined(__WATCOMC__)
		count = _vbprintf(msg_buf,1024,format,ap);
#elif defined(_AIX) || defined(sun) || defined(_hpux) || defined(__alpha) || defined(linux)
		count = vsnprintf(msg_buf,1024,format,ap);
#else  /* other systems: VXWORKS, ... 					*/
		count = vsprintf (msg_buf, format, ap);
#endif
		msg_buf[g_usrLog.max_msg_size-1] = 0; 	/* terminate the buffer, Win and UNIX 	*/
#if defined(_WIN32)
		if (count < 0)
		{
			sprintf (tmpBuf,"*** LOG ERROR: LOG BUFFER OVERRUN (g_usrLog.max_msg_size=%d bytes) or _vsnprintf function error",
				g_usrLog.max_msg_size);
			strncpy_safe (msg_buf, tmpBuf, g_usrLog.max_msg_size-1);
			count = strlen (msg_buf);		/* count = len of this log message	*/
		}
#else  /* !defined(_WIN32) */
		if (count < 0)
		{
			sprintf (tmpBuf,"*** LOG ERROR: _vbprintf(QNX), vsnprintf(UNIX,LINUX), or vsprintf(other sys) function failed");
			strncpy_safe (msg_buf, tmpBuf, g_usrLog.max_msg_size-1);
			count = strlen (msg_buf);		/* count = len of this log message	*/
		}
		else if (count >= g_usrLog.max_msg_size)
		{
			sprintf (tmpBuf,"*** LOG ERROR: LOG BUFFER OVERRUN: message len=%d bytes (g_usrLog.max_msg_size=%d bytes)",
				count, g_usrLog.max_msg_size);
			strncpy_safe (msg_buf, tmpBuf, g_usrLog.max_msg_size-1);
			count = strlen (msg_buf);		/* set count to len of this log message */
		}
#endif  /* !defined(_WIN32) */
		count++;
	}
	
	fprintf (g_usrLog.fp,"%s\n", msg_buf);

	va_end(ap);	
}

/************************************************************************/
/*                               doSlog                                 */
/* Main logging function, called from slog or slogx.			*/
/* Just print the message to be logged, pass to memory & file logging	*/
/* functions if enabled							*/
/************************************************************************/

#define SLOG_MISSED_LOG_MSG  "Warning: SLOG Log messages missed"

static ST_VOID doSlog (LOG_CTRL *lc, 
					   SD_CONST ST_INT logType, 
					   SD_CONST ST_CHAR *SD_CONST functionName, 
					   SD_CONST ST_CHAR *SD_CONST sourceFile,
					   SD_CONST ST_INT lineNum, 
					   SD_CONST ST_CHAR *SD_CONST format, va_list ap)
{
	ST_INT 	count;
	ST_CHAR msg_buf[1024];
	ST_CHAR tmpBuf[128];
	ST_CHAR currTime[128];
	lc->max_msg_size=1024;
	
	/* It is OK to pass in a NULL format string when using the dynamic 	*/
	/* logging functions - no vsprintf if so				*/
	ST_CHAR logLevel[16] = {0};
	switch (logType) {
		case SX_LOG_DEBUG:
			strncpy_safe(logLevel, "DEBUG", strlen("DEBUG"));
			break;
		case SX_LOG_ALWAY:
			strncpy_safe(logLevel, "INFO", strlen("INFO"));
			break;
		case SX_LOG_WARN:
			strncpy_safe(logLevel, "WARNING", strlen("WARNING"));
			break;						
		case SX_LOG_ERROR:
			strncpy_safe(logLevel, "ERROR", strlen("ERROR"));
			break;		
		default:
			strncpy_safe(logLevel, "Unknow Level", strlen("Unknow Level"));	
			break;	
	}
	if (format == NULL)
	{
		/* make buf a zero length string just in case			*/
		count = 0;
		msg_buf[0] = 0;
	}
	else
	{
#if defined(_WIN32)
		count = _vsnprintf(msg_buf,lc->max_msg_size,format,ap);
#elif  defined(__QNX__) && defined(__WATCOMC__)
		count = _vbprintf(msg_buf,lc->max_msg_size,format,ap);
#elif defined(_AIX) || defined(sun) || defined(_hpux) || defined(__alpha) || defined(linux)
		count = vsnprintf(msg_buf,lc->max_msg_size,format,ap);
#else  /* other systems: VXWORKS, ... 					*/
		count = vsprintf (msg_buf, format, ap);
#endif
		msg_buf[lc->max_msg_size-1] = 0; 	/* terminate the buffer, Win and UNIX 	*/
		/* functions don't behave the same 	*/
		/* NOTE: On _WIN32 count could be negative because of error or too small buffer. 	*/
		/*       On other systems count is negative because of error, too small buffer is	*/
		/*       indicated by return of count larger than buf_size supplied to the function.*/
#if defined(_WIN32)
		if (count < 0)
		{
			sprintf (tmpBuf,"*** LOG ERROR: LOG BUFFER OVERRUN (lc->max_msg_size=%d bytes) or _vsnprintf function error",
				lc->max_msg_size);
			strncpy_safe (msg_buf, tmpBuf, lc->max_msg_size-1);
			count = strlen (msg_buf);		/* count = len of this log message	*/
		}
#else  /* !defined(_WIN32) */
		if (count < 0)
		{
			sprintf (tmpBuf,"*** LOG ERROR: _vbprintf(QNX), vsnprintf(UNIX,LINUX), or vsprintf(other sys) function failed");
			strncpy_safe (msg_buf, tmpBuf, lc->max_msg_size-1);
			count = strlen (msg_buf);		/* count = len of this log message	*/
		}
		else if (count >= lc->max_msg_size)
		{
			sprintf (tmpBuf,"*** LOG ERROR: LOG BUFFER OVERRUN: message len=%d bytes (lc->max_msg_size=%d bytes)",
				count, lc->max_msg_size);
			strncpy_safe (msg_buf, tmpBuf, lc->max_msg_size-1);
			count = strlen (msg_buf);		/* set count to len of this log message */
		}
#endif  /* !defined(_WIN32) */

		count++;					/* allow for null terminator		*/
	}

	getCurrentTime(currTime);
	//修改了源代码logctrl无法输出到日志文件中的问题
	if (lc->logCtrl && (g_lc.logType == LOG_FILE_EN) )	/* File Logging enabled		*/
	{
		/* Now print the message buffer						*/
		fprintf (lc->fp,"[%s %s %s %d]%s: %s\n", currTime, sourceFile, functionName, lineNum, logLevel, msg_buf);

	}

	if (lc->logCtrl && (g_lc.logType == LOG_MEM_EN) )
	{
		printf ("[%s %s %s %d]%s: %s\n", currTime, sourceFile, functionName, lineNum, logLevel, msg_buf);
	}

}

/**
 * @description: getCurrentTime 获取当前YYYY-mm-dd HH:MM:SS:MS
 * @param {*} 当前时间指针
 * @return {*} null
 */
ST_VOID getCurrentTime(SD_CONST ST_CHAR *SD_CONST currTime) {
	struct timespec
	{
		time_t tv_sec; /* 秒*/
		long tv_nsec; /* 纳秒*/
	};
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time); //获取相对于1970到现在的秒数
	struct tm nowTime;
	localtime_r(&time.tv_sec, &nowTime);
	
	sprintf(currTime, "%4d-%02d-%02d %02d:%02d:%02d", nowTime.tm_year + 1900, nowTime.tm_mon + 1, nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
}

ST_VOID slogCallStack (LOG_CTRL *lc, SD_CONST ST_CHAR *txt)
{
	lc = lc;
#if !defined(CODAN)
#if defined(MSDOS) && !defined(TC)
	static ST_UINT32 ptr_to_abs (ST_VOID *ptr);
	ST_UINT16 os,sg;
	ST_UINT16 os2,sg2;
	ST_UINT16 os3,sg3;
	ST_UINT16 os4,sg4;
	ST_UINT16 os5,sg5;
#endif

	/* For DOS we can save the caller's return address		 	*/
#if defined(MSDOS) && !defined(TC)
	_asm {
		push si                     ; save SI

			mov si, bp                  ; first BP frame
			mov ax, ss:[si+2]		; get return address "offset" from stack
			mov os, ax			; store in "offset" variable
			mov ax, ss:[si+4]		; get return address "segment" from stack
			mov sg, ax			; store in "segment" variable

			mov si, ss:[si]             ; second BP frame
			mov ax, ss:[si+2]		; get return address "offset" from stack
			mov os2, ax			; store in "offset" variable
			mov ax, ss:[si+4]		; get return address "segment" from stack
			mov sg2, ax			; store in "segment" variable

			mov si, ss:[si]             ; third BP frame
			mov ax, ss:[si+2]		; get return address "offset" from stack
			mov os3, ax			; store in "offset" variable
			mov ax, ss:[si+4]		; get return address "segment" from stack
			mov sg3, ax			; store in "segment" variable

			mov si, ss:[si]             ; fourth BP frame
			mov ax, ss:[si+2]		; get return address "offset" from stack
			mov os4, ax			; store in "offset" variable
			mov ax, ss:[si+4]		; get return address "segment" from stack
			mov sg4, ax			; store in "segment" variable

			mov si, ss:[si]             ; fifth BP frame
			mov ax, ss:[si+2]		; get return address "offset" from stack
			mov os5, ax			; store in "offset" variable
			mov ax, ss:[si+4]		; get return address "segment" from stack
			mov sg5, ax			; store in "segment" variable

			pop si                      ; restore SI
	}
#endif

#endif

	if (txt != NULL)
		SLOG_WARN( "%s", txt);
}
//////////////////////////////////////////////////////////////////////////
//启动日志
//
//
//////////////////////////////////////////////////////////////////////////
ST_RET slog_start (SD_CONST ST_UINT32 logLevel, SD_CONST ST_UINT32 logType, SD_CONST ST_CHAR *sFile, SD_CONST ST_CHAR *sUsrLog)
{
	g_lc.logCtrl=logLevel;
	g_lc.logType=logType;
	
	if (sFile)
	{
		if ( (g_lc.fp=fopen(sFile,"w")) == NULL)
		{
			printf("Failed to open %s\n", sFile);
			exit(-1);
		}
	}

	if (sUsrLog)
	{
		if ( (g_usrLog.fp=fopen(sUsrLog,"w")) == NULL)
		{
			printf("Failed to open %s\n", sUsrLog);
			exit(-1);
		}
	}

	return SD_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
//停止日志
//
//
//////////////////////////////////////////////////////////////////////////
ST_RET slog_end ()
{
	if (g_lc.fp)
	{
		fclose(g_lc.fp);
		g_lc.fp=0;
	}

	//保存用户日志
	if (g_usrLog.fp){
		fclose(g_usrLog.fp);
		g_usrLog.fp=0;
	}
	return SD_SUCCESS;
}
