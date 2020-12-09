/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*              1986-2004 All Rights Reserved       			*/
/*									*/
/* MODULE NAME : mem_chk.h						*/
/* PRODUCT(S)  : General Use						*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*	This module contains the declarations of the dynamic memory 	*/
/*	handling functions.						*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 02/06/04  EJV     16    Added dyn_mem_ptr_status2.			*/
/* 04/14/03  DSF     15    Cleanup					*/
/* 04/08/03  DSF     14    added m_mem_crt_debug			*/
/* 04/04/03  DSF     13    new/delete checks for MFC			*/
/* 08/20/01  JRB     12    chk_* functions chged to nd_chk_*.		*/
/*			   chk_* names are now ALWAYS macros.		*/
/*			   Make 4 sets of macros depending on whether	*/
/*			   DEBUG_SISCO and/or SMEM_ENABLE defined.	*/
/*			   Don't define SMEM_ENABLE, do from makefiles.	*/
/*			   Remove logging backward compatibility stuff.	*/
/* 08/06/01  RKR     11    S_THISFILE was removed, need a thisFileName	*/
/* 03/19/01  JRB     10    Move SMEM context defs to "smem.h".		*/
/* 01/25/01  DSF     09    new/delete checks				*/
/* 11/01/00  MDE     08    Additional SMEM work				*/
/* 01/21/00  MDE     07    Added SMEM support				*/
/* 09/13/99  MDE     06    Added SD_CONST modifiers			*/
/* 01/26/98  MDE     05    Added 'chk_strdup'				*/
/* 12/08/98  MDE     04    Added 'ST_BOOLEAN m_auto_hw_log'		*/
/* 10/08/98  MDE     03    Migrated to updated SLOG interface		*/
/* 10/14/97  DSF     02    m_bad_ptr_val is now a pointer		*/
/* 09/16/97  DSF     01    chk_debug_en is UINT				*/
/* 04/02/97  DTL   7.00    MMSEASE 7.0 release. See MODL70.DOC for	*/
/*			   history.					*/
/************************************************************************/

#ifndef MEM_CHK_INCLUDED
#define MEM_CHK_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "glbtypes.h"
#include "sysincs.h"

#ifdef SMEM_ENABLE
#include "smem.h"
#endif

/************************************************************************/
/* MEM_CHK MACROS and FUNCTION PROTOTYPES				*/
/* Memory allocation macros. There are 4 sets of macros depending on	*/
/* DEBUG_SISCO and SMEM_ENABLE. Each set of macros calls a unique set	*/
/* of functions.							*/
/*   The DEBUG macros use thisFileName (instead of __FILE__) to reduce	*/
/* memory usage. Any file using these macros MUST contain the following	*/
/* statement:								*/
/*   static char *thisFileName = __FILE__;				*/
/************************************************************************/

#if defined(SMEM_ENABLE)
 #if defined(DEBUG_SISCO)
  #define M_MALLOC(ctx,x)	x_m_malloc  (ctx,x,  thisFileName,__LINE__)
  #define M_CALLOC(ctx,x,y)	x_m_calloc  (ctx,x,y,thisFileName,__LINE__)
  #define M_REALLOC(ctx,x,y)	x_m_realloc (ctx,x,y,thisFileName,__LINE__)
  #define M_STRDUP(ctx,x)	x_m_strdup  (ctx,x,  thisFileName,__LINE__)
  #define M_FREE(ctx,x)		x_m_free    (ctx,x,  thisFileName,__LINE__)

  #define chk_malloc(x)		x_m_malloc  (MSMEM_GEN,x,  thisFileName,__LINE__)
  #define chk_calloc(x,y)	x_m_calloc  (MSMEM_GEN,x,y,thisFileName,__LINE__)
  #define chk_realloc(x,y)	x_m_realloc (MSMEM_GEN,x,y,thisFileName,__LINE__)
  #define chk_strdup(x)		x_m_strdup  (MSMEM_GEN,x,  thisFileName,__LINE__)
  #define chk_free(x)		x_m_free    (MSMEM_GEN,x,  thisFileName,__LINE__)

  ST_VOID *x_m_malloc  (SMEM_CONTEXT *smem_ctx, ST_UINT size,  
			SD_CONST ST_CHAR *SD_CONST file, 
			SD_CONST ST_INT line);

  ST_VOID *x_m_calloc  (SMEM_CONTEXT *smem_ctx, ST_UINT num, 
			ST_UINT size,  
			SD_CONST ST_CHAR *SD_CONST file, 
			SD_CONST ST_INT line);

  ST_VOID *x_m_realloc (SMEM_CONTEXT *smem_ctx, ST_VOID *old, 
			ST_UINT size, 
			SD_CONST ST_CHAR *SD_CONST file, 
			SD_CONST ST_INT line);
  ST_VOID  x_m_free    (SMEM_CONTEXT *smem_ctx, ST_VOID *ptr, 
			SD_CONST ST_CHAR *SD_CONST file, 
			SD_CONST ST_INT line);
  ST_CHAR *x_m_strdup  (SMEM_CONTEXT *smem_ctx, ST_CHAR *str, 
			SD_CONST ST_CHAR *SD_CONST file, 
			SD_CONST ST_INT line);

 #else	/* !DEBUG_SISCO	*/

  #define M_MALLOC(ctx,x)	nd_m_malloc  (ctx,x)
  #define M_CALLOC(ctx,x,y)	nd_m_calloc  (ctx,x,y)
  #define M_REALLOC(ctx,x,y)	nd_m_realloc (ctx,x,y)
  #define M_STRDUP(ctx,x)	nd_m_strdup  (ctx,x)
  #define M_FREE(ctx,x)		nd_m_free    (ctx,x)
  
  #define chk_malloc(x)		nd_m_malloc  (MSMEM_GEN,x)
  #define chk_calloc(x,y)	nd_m_calloc  (MSMEM_GEN,x,y)
  #define chk_realloc(x,y)	nd_m_realloc (MSMEM_GEN,x,y)
  #define chk_strdup(x)		nd_m_strdup  (MSMEM_GEN,x)
  #define chk_free(x)		nd_m_free    (MSMEM_GEN,x)

  ST_VOID *nd_m_malloc  (SMEM_CONTEXT *smem_ctx, ST_UINT size);
  ST_VOID *nd_m_calloc  (SMEM_CONTEXT *smem_ctx, ST_UINT num, ST_UINT size);
  ST_VOID *nd_m_realloc (SMEM_CONTEXT *smem_ctx, ST_VOID *old, ST_UINT size);
  ST_VOID  nd_m_free    (SMEM_CONTEXT *smem_ctx, ST_VOID *ptr);
  ST_CHAR *nd_m_strdup  (SMEM_CONTEXT *smem_ctx, ST_CHAR *str);

 #endif	/* !DEBUG_SISCO	*/
#else	/* !SMEM_ENABLE	*/
 #if 0

  #define M_MALLOC(ctx,x)	x_chk_malloc  (x,  thisFileName,__LINE__)
  #define M_CALLOC(ctx,x,y)	x_chk_calloc  (x,y,thisFileName,__LINE__)
  #define M_REALLOC(ctx,x,y)	x_chk_realloc (x,y,thisFileName,__LINE__)
  #define M_STRDUP(ctx,x)	x_chk_strdup  (x,  thisFileName,__LINE__)
  #define M_FREE(ctx,x)		x_chk_free    (x,  thisFileName,__LINE__)

  #define chk_malloc(x)		x_chk_malloc  (x,  thisFileName,__LINE__)
  #define chk_calloc(x,y)	x_chk_calloc  (x,y,thisFileName,__LINE__)
  #define chk_realloc(x,y)	x_chk_realloc (x,y,thisFileName,__LINE__)
  #define chk_strdup(x)		x_chk_strdup  (x,  thisFileName,__LINE__)
  #define chk_free(x)		x_chk_free    (x,  thisFileName,__LINE__)

  ST_VOID *x_chk_realloc (ST_VOID *old, 
			ST_UINT size, 
			SD_CONST ST_CHAR *SD_CONST file, 
			SD_CONST ST_INT line);
  ST_VOID *x_chk_malloc  (ST_UINT size,  
			SD_CONST ST_CHAR *SD_CONST file, 
			SD_CONST ST_INT line);
  ST_VOID *x_chk_calloc  (ST_UINT num, 
			ST_UINT size,  
			SD_CONST ST_CHAR *SD_CONST file, 
			SD_CONST ST_INT line);
  ST_CHAR *x_chk_strdup  (ST_CHAR *str, 
			SD_CONST ST_CHAR *SD_CONST file, 
			SD_CONST ST_INT line);
  ST_VOID  x_chk_free    (ST_VOID *old, 
			SD_CONST ST_CHAR *SD_CONST file, 
			SD_CONST ST_INT line);

 #else	/* !DEBUG_SISCO	*/

static ST_CHAR *nd_chk_strdup (ST_CHAR *str)
{
	ST_CHAR *new_str;
  if (*str == '\0') return NULL;
	new_str = (ST_CHAR *) malloc (strlen (str) + 1);
	strcpy (new_str, str);
	return (new_str);
}

  #define M_MALLOC(ctx,x)	malloc  (x)
  #define M_CALLOC(ctx,x,y)	calloc  (x,y)
  #define M_REALLOC(ctx,x,y)	realloc (x,y)
  #define M_STRDUP(ctx,x)	nd_chk_strdup  (x)
  #define M_FREE(ctx,x)		free    (x)

  #define chk_malloc(x)		malloc  (x)
  #define chk_calloc(x,y)	calloc  (x,y)
  #define chk_realloc(x,y)	realloc (x,y)
  #define chk_strdup(x)		nd_chk_strdup  (x)
  #define chk_free(x)		free    (x)



 

 #endif	/* !DEBUG_SISCO	*/
#endif	/* !SMEM_ENABLE	*/



/************************************************************************/
/************************************************************************/
#ifdef __cplusplus
}
#endif


#endif /* mem_chk.h already included */
