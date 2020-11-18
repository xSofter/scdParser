#ifndef STR_UTIL_INCLUDED
#define STR_UTIL_INCLUDED
/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*	(c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*	2000 - 2004 All Rights Reserved					*/
/*									*/
/* MODULE NAME : str_util.h						*/
/* PRODUCT(S)  : General Use						*/
/*									*/
/* MODULE DESCRIPTION :							*/
/*	General purpose string manipulation functions.			*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev     Comments					*/
/* --------  ---  ------   -------------------------------------------  */
/* 10/29/07  JRB    23     Add strncat_maxstrlen. Del strncat_safe (len	*/
/*			   was wrong), use strncat or strncat_maxstrlen.*/
/* 10/11/07  JRB    22     Add asciiToSint32, asciiToUint32		*/
/* 07/28/05  DWL    21     Use const ST_CHAR* in asciiTo* functions.	*/
/* 03/14/05  JRB    20     Add get_next_string.				*/
/* 12/03/04  JRB    19     Use strcasecmp, strncasecmp if supported.	*/
/* 07/09/04  JRB    18     Add strncat_safe, strncpy_safe.		*/
/* 03/08/04  EJV    17     Added getKeywordFromFile function.		*/
/* 12/12/03  JRB    16     Add LYNX support.				*/
/* 12/10/03  MDE    15     Added itoa/ltoa/utoa/ultoa replacements	*/
/* 04/10/03  DSF    14     Added strnstr				*/
/* 02/20/03  JRB    13     Del PSOS code.				*/
/* 12/20/02  CRM    12     Added "defined(linux)" for strcmpi, etc.     */
/* 02/28/02  EJV    11     Added asciiToFloat, asciiToDouble.		*/
/* 02/25/02  EJV    10     Replaced ascii_to_ macros with asciiToxxx fun*/
/*			   Added asciiToUlong, asciiToUint16,		*/
/*			    asciiToUint8, asciiToSint, asciiToUint	*/
/*			   hex_to_ascii_str: added param hex_no_spaces.	*/
/* 12/09/01  GLB    05     Added asciiToSlong, asciiToSint16,           */
/*                          asciiToSint8, asciiToUchar                  */
/* 09/10/01  GLB    08     Added ascii_to_uint & ascii_to_slongint      */
/* 08/06/01  GLB    07     Added ascii_to_sint                          */
/* 06/06/01  GLB    06     Added ascii_to_double & ascii_to_float       */
/* 05/21/01  MDE    05     Added bitstring_to_ascii_str			*/
/* 01/19/01  EJV    04     UNIX: added strcmpi, stricmp, strnicmp protos*/
/* 01/05/01  EJV    03     Corrected ascii_to_ushortint, AIX returned 0.*/
/*			   ascii_to_ulongint: changed "%ul" to "%lu".	*/
/* 11/02/00  MDE    02     Added _hex_no_spaces				*/
/* 02/03/00  JRB    01     Created					*/
/************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

ST_VOID hex_to_ascii_str (
		ST_CHAR *astr,		/* ascii string			*/
		ST_UCHAR *hstr,		/* hex string			*/
                ST_UINT hlen,		/* len of hex string		*/
		ST_BOOLEAN hex_no_spaces);

ST_RET ascii_to_hex_str (
		ST_UCHAR *hstr,		/* hex string			*/
                ST_UINT *hlen_out,	/* ptr to hex len to be set	*/
                ST_UINT hlen_max,	/* maximum hex len to allow.	*/
		ST_CHAR *astr);		/* ascii string			*/

ST_VOID bitstring_to_ascii_str (ST_CHAR *astr, ST_UCHAR *bstr,
                		ST_UINT numBits);


ST_RET asciiToSlong   (const ST_CHAR  *astr,	 			              
                       ST_LONG  *sLong);   
ST_RET asciiToUlong   (const ST_CHAR *astr,
                       ST_ULONG *uLong);

ST_RET asciiToSint    (const ST_CHAR *astr,
                       ST_INT *sInt);
ST_RET asciiToUint    (const ST_CHAR *astr,
                       ST_UINT *uInt);

ST_RET asciiToSint32 (const ST_CHAR *astr,	/* ascii string		*/
	ST_INT32 *value);			/* ptr to converted value*/
ST_RET asciiToUint32 (const ST_CHAR *astr,	/* ascii string		*/
	ST_UINT32 *value);			/* ptr to converted value*/

ST_RET asciiToSint16  (const ST_CHAR  *astr,	 			              
                       ST_INT16 *sInt16);   
ST_RET asciiToUint16  (const ST_CHAR *astr,
                       ST_UINT16 *uInt16);

ST_RET asciiToSint8   (const ST_CHAR  *astr,	 			              
                       ST_INT8  *sInt8);   
ST_RET asciiToUint8   (const ST_CHAR *astr,
                       ST_UINT8 *uInt8);

ST_RET asciiToUchar   (const ST_CHAR  *astr,	 			              
                       ST_UCHAR *uChar);   

ST_RET asciiToFloat   (const ST_CHAR  *astr,	 			              
                       ST_FLOAT *floatNum);   
ST_RET asciiToDouble   (const ST_CHAR *astr,
                       ST_DOUBLE *doubleNum);


char *strnstr (char *str1, char *str2, int len);

/* SISCO code calls strcmpi, stricmp, strnicmp (Windows functions).	*/
/* On systems that support strcasecmp, strncasecmp, remap to use them.	*/
#if defined(_AIX) || defined(sun) || defined(__alpha) || defined(__hpux) || \
    defined(linux)
#ifndef strcmpi
#define strcmpi strcasecmp
#endif
#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif
#endif	/* UNIX-like systems that support strcasecmp, strncasecmp.	*/

/* On systems that DON'T support strcasecmp, strncasecmp, use SISCO functions.	*/
#if defined(VXWORKS) || defined(__LYNX)
ST_INT strcmpi (ST_CHAR *,ST_CHAR *);
ST_INT stricmp (ST_CHAR *,ST_CHAR *);
ST_INT strnicmp (ST_CHAR *,ST_CHAR *,ST_INT n);
#endif

/* "strncat_maxstrlen" is like "strncat" but the third arg is the	*/
/* maximum length of the destination string.				*/
ST_RET strncat_maxstrlen (char *dest, char *src, size_t maxstrlen);

/* "strncpy_safe" makes sure that the "dest" string is NULL-terminated.	*/
/* The standard "strncpy" does NOT always NULL-terminate "dest".	*/
ST_VOID strncpy_safe (char *dest, char *src, int max_len);

ST_CHAR *sInt8ToAscii  (ST_INT8   v, ST_CHAR *p);
ST_CHAR *uInt8ToAscii  (ST_UINT8  v, ST_CHAR *p);
ST_CHAR *sInt16ToAscii (ST_INT16  v, ST_CHAR *p);
ST_CHAR *uInt16ToAscii (ST_UINT16 v, ST_CHAR *p);
ST_CHAR *sInt32ToAscii (ST_INT32  v, ST_CHAR *p);
ST_CHAR *uInt32ToAscii (ST_UINT32 v, ST_CHAR *p);
ST_CHAR *sIntToAscii   (ST_INT    v, ST_CHAR *p);
ST_CHAR *uIntToAscii   (ST_UINT   v, ST_CHAR *p);
ST_CHAR *sLongToAscii  (ST_LONG   v, ST_CHAR *p);
ST_CHAR *uLongToAscii  (ST_ULONG  v, ST_CHAR *p);

/* misc functions */
ST_BOOLEAN getKeywordFromFile (ST_CHAR *fileStr, ST_CHAR *keywordStr,
                               ST_CHAR *valBuf, ST_UINT valBufLen);

ST_CHAR *get_next_string (ST_CHAR **ptrptr, ST_CHAR *delimiters);

ST_VOID replacechar(ST_CHAR *str,ST_CHAR des,ST_CHAR src);

ST_VOID getCurrentTime(SD_CONST ST_CHAR *SD_CONST currTime);

#ifdef __cplusplus
}
#endif

#endif	/* !STR_UTIL_INCLUDED	*/

