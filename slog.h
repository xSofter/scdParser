/*
 * @Date: 2020-11-16 08:45
 * @LastEditTime: 2020-12-23 10:52
 * @LastEditors: tangkai3
 * @Description: 
 */
/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*      	  1993 - 2008, All Rights Reserved.		        */
/*									*/
/*		    PROPRIETARY AND CONFIDENTIAL			*/
/*									*/
/* MODULE NAME : slog.h							*/
/* PRODUCT(S)  : SLOG							*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 08/04/08  MDE     78    Added slogIpcEventEx				*/
/*			   history.					*/
/************************************************************************/

#ifndef SLOG_INCLUDED
#define SLOG_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "scdParse_export.h"
#include "gen_list.h"


#define  LOG_FILE_EN 0x02
#define  LOG_MEM_EN  0x01

typedef struct log_ctrl
{
  /* User sets these elements						*/
  ST_UINT32 logType;
  ST_UINT32 logCtrl; 	/* Logging Control flags - see LOG_xxx defines	*/
  /* Internal variables.							*/
  ST_INT max_msg_size;	/* max allowed log msg size.			*/
  FILE *fp;
} LOG_CTRL;


#define SX_LOG_NONE  	0x0000
#define SX_LOG_ALWAY  	0x0001   
#define SX_LOG_DEBUG  	0x0002   
#define SX_LOG_WARN 	0x0004   
#define SX_LOG_ERROR 	0x0008   

/************************************************************************/
/* Main entry points into the SLOG library				*/
/************************************************************************/

ST_VOID slog (SD_CONST ST_CHAR *format, ...);

ST_VOID slogHead (SD_CONST ST_INT logType,
				SD_CONST ST_CHAR *SD_CONST sourceFile, 
				SD_CONST ST_CHAR *SD_CONST functionName,
			  	SD_CONST ST_INT lineNum);

ST_RET SCDPAESE_API slog_start (SD_CONST ST_UINT32 logCtrl, SD_CONST ST_UINT32 logType, SD_CONST ST_CHAR *sFile, SD_CONST ST_CHAR *sUsrLog);
ST_RET SCDPAESE_API slog_end ();
ST_VOID userLog (SD_CONST ST_CHAR *SD_CONST format, ...);
#define DEBUG_SISCO
/************************************************************************/
#if defined(DEBUG_SISCO)
/************************************************************************/
#define SLOGALWAYS slogHead(SX_LOG_ALWAY,thisFileName,__FUNCTION__, __LINE__);slog
#define SLOG_DEBUG slogHead(SX_LOG_DEBUG,thisFileName,__FUNCTION__, __LINE__);slog
#define SLOG_WARN slogHead(SX_LOG_WARN,thisFileName,__FUNCTION__, __LINE__);slog
#define SLOG_ERROR  slogHead(SX_LOG_ERROR,thisFileName,__FUNCTION__, __LINE__);slog

/************************************************************************/
#else	/* #if defined(DEBUG_SISCO) */
/************************************************************************/
#define SLOGALWAYS 
#define SLOG_DEBUG 
#define SLOG_WARN 
#define SLOG_ERROR
#endif

#ifdef __cplusplus
}
#endif

#endif  /* end of 'already included' 	*/

