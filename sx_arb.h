/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*      		1999 - 2004, All Rights Reserved		*/
/*									*/
/* MODULE NAME : sx_arb.h						*/
/* PRODUCT(S)  : 							*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 09/19/07  JRB      04    Add TEXT_FORMAT_COMMA, sxaTextToLocalEx.	*/
/* 10/24/06  JRB      03    Add sxaLocalToText2 proto.			*/
/* 12/06/04  JRB      02    Add "sx_defs.h".				*/
/* 08/30/04  DSF      01    Module created				*/
/************************************************************************/
/************************************************************************/

#ifndef SX_ARB_INCLUDED
#define SX_ARB_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


/************************************************************************/

#include "mms_def2.h"
#include "mms_mp.h"
#include "mms_pvar.h"
#include "mms_vvar.h"
#include "sx_defs.h"	/* need SX_ENC_CTRL	*/

ST_RET sxd_decode_rtdata_el (SD_CONST ST_CHAR *elName, 
			   ST_CHAR *xml, ST_INT xmlLen, ST_INT *xmlUsed, 
			   ST_VOID *vdp, SD_CONST RUNTIME_TYPE *rt_head, 
			   ST_INT rt_num, ST_BOOLEAN *elPres, 
			   ST_INT sxdXmlStyle, ST_BOOLEAN checkDt);
ST_RET sxd_wr_rtdata (SX_ENC_CTRL *sxEncCtrl, RUNTIME_TYPE *rt, ST_INT numRt, 
		   ST_CHAR *elName, ST_VOID *data, ST_INT sxdXmlStyle,
		   ST_BOOLEAN encodeDt, ST_BOOLEAN *elPres);

/************************************************************************/
/************************************************************************/
/* Arbitrary data processing, for use in traversing the RUNTIME_TYPE	*/

typedef struct 
  {
  ST_RET (*arrStart) (ST_VOID *usr, RUNTIME_TYPE *rt);
  ST_RET (*arrEnd)   (ST_VOID *usr, RUNTIME_TYPE *rt);
  ST_RET (*strStart) (ST_VOID *usr, RUNTIME_TYPE *rt, SD_CONST RUNTIME_TYPE *rt_head);
  ST_RET (*strEnd)   (ST_VOID *usr, RUNTIME_TYPE *rt);
  ST_RET (*int8)     (ST_VOID *usr, ST_INT8    *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*int16)    (ST_VOID *usr, ST_INT16   *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*int32)    (ST_VOID *usr, ST_INT32   *data_dest, RUNTIME_TYPE *rt);
#ifdef INT64_SUPPORT
  ST_RET (*int64)    (ST_VOID *usr, ST_INT64   *data_dest, RUNTIME_TYPE *rt);
#endif
  ST_RET (*uint8)    (ST_VOID *usr, ST_UINT8   *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*uint16)   (ST_VOID *usr, ST_UINT16  *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*uint32)   (ST_VOID *usr, ST_UINT32  *data_dest, RUNTIME_TYPE *rt);
#ifdef INT64_SUPPORT
  ST_RET (*uint64)   (ST_VOID *usr, ST_UINT64  *data_dest, RUNTIME_TYPE *rt);
#endif
  ST_RET (*flt)      (ST_VOID *usr, ST_FLOAT   *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*dbl)      (ST_VOID *usr, ST_DOUBLE  *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*oct)      (ST_VOID *usr, ST_UCHAR   *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*booln)    (ST_VOID *usr, ST_BOOLEAN *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*bcd1)     (ST_VOID *usr, ST_INT8    *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*bcd2)     (ST_VOID *usr, ST_INT16   *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*bcd4)     (ST_VOID *usr, ST_INT32   *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*bs)       (ST_VOID *usr, ST_UCHAR   *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*vis)      (ST_VOID *usr, ST_CHAR    *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*bt4)      (ST_VOID *usr, ST_INT32   *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*bt6)      (ST_VOID *usr, ST_INT32   *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*gt)       (ST_VOID *usr, time_t     *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*utc)      (ST_VOID *usr, MMS_UTC_TIME *data_dest, RUNTIME_TYPE *rt);
  ST_RET (*utf8)     (ST_VOID *usr, ST_UCHAR   *data_dest, RUNTIME_TYPE *rt);
  } SXD_ARB_DATA_CTRL;

ST_RET sxd_process_arb_data (ST_CHAR *datptr, SD_CONST RUNTIME_TYPE *rt_head, 
			     ST_INT rt_num, ST_VOID *usr, 
			     SXD_ARB_DATA_CTRL *ac, ST_BOOLEAN *elPres);

ST_RET sxaTextToLocal (ST_CHAR *pSource, ST_VOID *pDest, ST_INT numRt, SD_CONST RUNTIME_TYPE *rtHead);
ST_CHAR *sxaLocalToText (ST_VOID *dataPtr, SD_CONST RUNTIME_TYPE *rtHead, ST_INT numRt);
/* sxaLocalToText2 is thread-safe (does not use global buffer).	*/
ST_CHAR *sxaLocalToText2 (ST_VOID *dataPtr, SD_CONST RUNTIME_TYPE *rtHead, ST_INT numRt,
	ST_CHAR *textBuf,	/* User buffer in which to write text	*/
	ST_UINT textBufSize);	/* size of user buffer			*/

/* Defines to use for textFormat argument of sxaTextToLocalEx.		*/
/* DEBUG: currently only ONE format supported (TEXT_FORMAT_COMMA), but 	*/
/*        this should make it easier to add other formats later.	*/
#define TEXT_FORMAT_COMMA	1	/* Comma-separated text format	*/
					/* Each value in the text may	*/
					/* also be surrounded by "".	*/
					/* Examples of text:		*/
					/*	"0","1.1","2","3.3","4"	*/
					/*	0,1.1,2,3.3,4		*/
ST_RET sxaTextToLocalEx (
	ST_CHAR *pSource,	/* source text	*/
	ST_VOID *pDest,		/* destination data	*/
	ST_INT numRt,
	SD_CONST RUNTIME_TYPE *rtHead,
	ST_INT textFormat);	/* text format from which to convert	*/

/************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* SX_ARB_INCLUDED */
