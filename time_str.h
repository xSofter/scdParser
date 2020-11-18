/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*		1994 - 2003, All Rights Reserved			*/
/*									*/
/* MODULE NAME : time_str.h						*/
/* PRODUCT(S)  : 							*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 02/28/08  JRB     16    Incl "timeb.h" only in "sysincs.h"		*/
/* 11/30/06  RLH     15    merge time_str and time_str2                 */
/* 07/13/06  RLH     14    tstrTimeToStringGmt and tstrStringToTimeGmt  */
/*                         prototypes added                             */
/* 03/06/06  RLH     13    add GetTimeAndUsec, tstrTimeToStringGmt,     */
/*                         tstrStringToTimeGmt                          */
/* 07/27/04  DWL     12    Added tstrTmToString (struct tm)		*/
/* 07/16/04  DWL     11    Added tstrStringToTm (struct tm)		*/
/* 10/30/03  EJV     10    Added dataLen param to UtcValueToXmlString	*/
/* 10/15/03  JRB     09    Del _WIN32 ifdef.				*/
/* 09/01/03  GLB     08    Added "XmlStringToUtcValue" & 		*/
/*                          "UtcValueToXmlString"                       */
/* 07/12/02  NAV     07    Add UtcValsToString and UtcStringToVals      */
/* 02/02/01  EJV     06    tstrStringToTime: chg (long *) to (time_t *) */ 
/* 07/13/98  NAV     05    Add _cplusplus support			*/
/* 10/15/97  NAV     04    Add Btime4 Support Functions			*/
/* 10/08/97  NAV     03    Add seconds to TSTR_DEF_TIME_FORMAT		*/
/* 09/04/97  NAV     02    Add Btime6 Conversion routines		*/
/* 11/08/94  MDE     01    New						*/
/************************************************************************/

/************************************************************************/
/*		TIME STRING FORM FOR READ DATA				*/
/*									*/
/*  Date{ws}Time							*/
/*  Time{ws}Date	(? should we allow this)			*/
/*  Time only	(uses current time)					*/
/*  Date only	(uses current date)					*/
/*									*/
/*	Date forms :							*/
/*	 12/01/56							*/
/*	 12/01/1956							*/
/*	 12-01-56 							*/
/*	 12-01-1956 							*/
/*	 12-1-56 							*/
/*	 12-1-56	 						*/
/* 	 26-Aug-1994							*/
/* 	 26-AUG-1994							*/
/* 	 AUG-26-1993							*/
/*									*/
/*	Time forms :							*/
/*	 7:21:43P							*/
/*	 7:21:43p							*/
/*	 7:21:43a							*/
/*	 7:21:43A							*/
/*	 07:21:43a							*/
/*	 7:21:43							*/
/*	 21:21:43							*/
/*									*/
/************************************************************************/
#include "sx_defs.h"

#ifdef __cplusplus
extern "C" {
#endif


/* parms that control how a date is parsed in terms of field ordering */

#define S_DATE_ORDER_ANY    0    /* check system locale for ordering */
#define S_DATE_ORDER_MDY    1    /* date ordering is month-day-year */
#define S_DATE_ORDER_DMY    2    /* date ordering is day-month-year */
#define S_DATE_ORDER_YMD    3    /* date ordering is year-month-day */

/* number of seconds difference between 1984-01-01 and 1970-01-01. */
/* this amounts to 14 years including 3 leap-year days (1972, 1976 */
/* and 1980) converted to a number of seconds. 1984 is a leap year */
/* but the span of dates does not include 1984-02-29, so 1984's    */
/* leap-year day is not counted.                                   */

/* S_SECS_DIFF_1984_1970 is used to calculate Btime6 values. */

#define S_SECS_DIFF_1984_1970 ((((1984-1970)*365)+3) * 86400)

#define MAX_TIME_STRING_LEN 256

#define TSTR_DEF_TIME_FORMAT  "%m-%d-%Y %H:%M:%S"

extern char *tstrTimeFormat;

ST_RET tstrTimeToStringGmt (time_t t, char *dest);
ST_RET tstrStringToTimeGmt (char *src, time_t *out);

ST_RET tstrStringToTime (char *src, time_t *out);
ST_RET tstrStringToTm (char *src, struct tm *out);

ST_RET tstrTimeToString    (time_t t, char *dest);
ST_RET tstrTimeToStringGmt (time_t t, char *dest);

ST_RET tstrTmToString    (struct tm *t, char *dest);
ST_RET tstrTmToStringGmt (struct tm *t, char *dest);

#define BTIME_DEF_TIME_FORMAT "%m-%d-%Y %H:%M:%S"

ST_RET Btime6StringToVals (char *src, ST_INT32 *numDays, ST_INT32 *numMSec);
ST_RET Btime6ValsToString (char *dest, ST_INT32 numDays, ST_INT32 numMSec);

ST_RET Btime4StringToVals (char *src, ST_INT32 *numMSec);
ST_RET Btime4ValsToString (char *dest, ST_INT32 numMSec);

ST_RET UtcStringToVals (char *src, ST_UINT32 *pSecs, ST_UINT32 *pFraction, 
                        ST_UINT32 *pQflags );
ST_RET UtcValsToString (char *dest, ST_UINT32 secs, ST_UINT32 fraction, 
                        ST_UINT32 qflags);
ST_RET UtcStringToValues (char *src, ST_UINT32 *pSecs, ST_UINT32 *pFraction);
ST_RET UtcValuesToString (char *dest, ST_UINT32 secs, ST_UINT32 fraction);
ST_RET XmlStringToUtcValue (ST_CHAR *src, SX_DATE_TIME *sxDateTime);
ST_RET UtcValueToXmlString (ST_CHAR *dest, ST_UINT destLen, SX_DATE_TIME *sxDateTime);
ST_DOUBLE CalculateTimeZoneOffset (ST_VOID);
#define UTC_DEF_TIME_FORMAT "%Y-%m-%dT%H:%M:%S"

#define GET_TIME_AND_USEC_DEFINED   1
time_t GetTimeAndUsec (long *usec);

time_t usr_mkgmtime (struct tm * t);

#ifdef __cplusplus
} /* End of 'C' functions	*/

#endif

