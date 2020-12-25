/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*	(c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*	2000 - 2007 All Rights Reserved					*/
/*									*/
/* MODULE NAME : str_util.c						*/
/* PRODUCT(S)  : General Use						*/
/*									*/
/* MODULE DESCRIPTION :							*/
/*	General purpose string manipulation functions.			*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*			ascii_to_hex_str				*/
/*			hex_to_ascii_str				*/
/*			strnicmp (UNIX)					*/
/*			strcmpi	 (UNIX)					*/
/*			stricmp	 (UNIX)					*/
/*			strncat_maxstrlen				*/
/*			strncpy_safe					*/
/*			asciiToSlong 					*/
/*			asciiToUlong 					*/
/*			asciiToSint  					*/
/*			asciiToUint  					*/
/*			asciiToSint32					*/
/*			asciiToUint32					*/
/*			asciiToSint16					*/
/*			asciiToUint16					*/
/*			asciiToSint8 					*/
/*			asciiToUint8 					*/
/*			asciiToUchar 					*/
/*			asciiToFloat 					*/
/*			asciiToDouble					*/
/*			sInt8ToAscii  					*/
/*			uInt8ToAscii  					*/
/*			sInt16ToAscii 					*/
/*			uInt16ToAscii 					*/
/*			sInt32ToAscii 					*/
/*			uInt32ToAscii 					*/
/*			sIntToAscii   					*/
/*			uIntToAscii   					*/
/*			sLongToAscii  					*/
/*			uLongToAscii  					*/
/*			getKeywordFromFile				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev     Comments					*/
/* --------  ---  ------   -------------------------------------------  */
/* 11/07/07  EJV    23     Check before defining INT32_MAX, UINT32_MAX.	*/
/************************************************************************/

#include "glbtypes.h"
#include "sysincs.h"
#include "str_util.h"
#include "slog.h"

#include <float.h>
#include <math.h>
#ifdef DEBUG_SISCO
SD_CONST static ST_CHAR *SD_CONST thisFileName = __FILE__;
#endif

/* NOTE: limits.h (included in sysincs.h) should have these defined.	*/
/*       Used to compare to LONG_MAX, ULONG_MAX to detect the long size.*/
#if !defined(INT32_MAX)
#define INT32_MAX	0x7fffffff
#endif
#if !defined(UINT32_MAX)
#define UINT32_MAX	0xffffffff
#endif

ST_CHAR *nd_chk_strdup (ST_CHAR *str)
{
    if (!str || strlen(str) == 0) return NULL;
    ST_CHAR *new_str;
    new_str = (ST_CHAR *) malloc (strlen (str) + 1);
    if (!new_str) return NULL;
    strcpy (new_str, str);
    return (new_str);
}

/************************************************************************/
/*			ascii_to_hex_str				*/
/************************************************************************/

ST_RET ascii_to_hex_str (
						 ST_UCHAR *hstr,		/* hex string			*/
						 ST_UINT *hlen_out,	/* ptr to hex len to be set	*/
						 ST_UINT hlen_max,	/* maximum hex len to allow.	*/
						 ST_CHAR *astr)		/* ascii string			*/
{
	ST_INT digit;
	ST_BOOLEAN nibble;	/* SD_TRUE if nibble read, SD_FALSE if whole byte read*/

	*hlen_out = 0;
	nibble = SD_FALSE;
	for ( ;  *astr;  astr++)	/* read until NULL terminator	*/
	{
		if (isxdigit (*astr))	/* get hex number, check whether is 0~9,a~f */
		{
			if (*hlen_out >= hlen_max)
			{			/* hstr already full. Can't add digit.	*/
				return (SD_FAILURE);
			}

			digit = *astr;
			digit = isdigit (digit) ? digit - '0' : 10 + (toupper (digit) - 'A'); 
			if (nibble)
			{					/* set low nibble	*/
				nibble = SD_FALSE;
				hstr[*hlen_out] |= (ST_UCHAR) digit;
				(*hlen_out)++;
			}
			else
			{					/* set high nibble	*/
				nibble = SD_TRUE;
				hstr[*hlen_out] = (ST_UCHAR) digit << 4;
			}
		}
		else if (isspace (*astr))
			continue;
		else
		{		/* not hex digit, not white space, NOT GOOD.		*/
			return (SD_FAILURE);
		}
	}	/* end "while"	*/

	if (nibble)
	{					/* Only got half of byte.	*/
		return (SD_FAILURE);
	}

	return (SD_SUCCESS);
}

/************************************************************************/
/*			hex_to_ascii_str				*/
/* To make it more readable, a 'blank' is inserted after every 2 digits	*/
/* in the ascii string. The ascii string is NULL terminated.		*/
/* The ascii buffer must be at least 3 times the length of the hex	*/
/* string (hlen) to allow for the blanks and the NULL terminator.	*/
/************************************************************************/
#define cvt_nibble(a)	((ST_UCHAR) ((a) > 9 ? (a) + 'A' - 10 : (a) + '0'))

ST_VOID hex_to_ascii_str (
						  ST_CHAR *astr,		/* ascii string			*/
						  ST_UCHAR *hstr,		/* hex string			*/
						  ST_UINT hlen,		/* len of hex string		*/
						  ST_BOOLEAN hex_no_spaces)

{
	ST_UINT j;
	ST_INT hex_index = 0;
	ST_UCHAR ch;
	ST_UCHAR hex;

	for (j = 0;  j < hlen;  j++)
	{
		if (j && hex_no_spaces != SD_TRUE)
			astr[hex_index++] = ' ';	/* space between bytes, but not before first*/

		ch = *hstr++;  			/* get next character		*/
		hex = (ch >> 4) & (ST_UCHAR) 0x0f;/* write HEX chars for the byte	*/
		hex = cvt_nibble(hex);
		astr[hex_index++] = hex;
		hex = ch & (ST_UCHAR) 0x0f;
		hex = cvt_nibble(hex);
		astr[hex_index++] = hex;
	}
	astr[hex_index] = '\0';	/* NULL terminate it.		*/
}


/************************************************************************/
/*			    strnstr					*/
/************************************************************************/

char * strnstr (char *str1, char *str2, int len)
{
	char *cp = (char *) str1;
	char *s1, *s2;
	int cl;

	if ( !*str2 )
		return((char *)str1);

	while (*cp && len)
	{
		cl = len;
		s1 = cp;
		s2 = (char *) str2;

		while (*s1 && *s2 && cl && !(*s1-*s2) )
			s1++, s2++, cl--;

		if (!*s2)
			return(cp);

		cp++;
		len--;
	}

	return(NULL);
}

/* These functions used when system libraries don't provide them.
* Most UNIX-like systems provide similar functions strcasecmp &
* strncasecmp. On those systems, strnicmp etc. are remapped to strcasecmp &
* strncasecmp (see str_util.h), so these functions NOT needed.
*/
#if defined(VXWORKS) || defined(__LYNX)

/************************************************************************/
/*			    strnicmp					*/
/************************************************************************/

ST_INT strnicmp (ST_CHAR *str1, ST_CHAR *str2, ST_INT n )
{
	ST_INT i = 0;
	ST_INT test = 1;

	for (i = 0; i < n; i++)
	{
		if ( tolower ( *(str1+i) ) != tolower ( *(str2+i) ) )
		{
			test = 0;
			break;
		}
	}
	if (test == 1)
		return (0);
	else 
		if ( tolower ( *(str1+i) ) > tolower ( *(str2+i) ) )
			return (1);
		else
			return (-1);
}

/************************************************************************/
/*			strcmpi						*/
/************************************************************************/
ST_INT strcmpi (ST_CHAR *str1, ST_CHAR *str2)
{
	ST_INT iLen1, iLen2;

	iLen1 = strlen(str1);
	iLen2 = strlen(str2);
	return(strnicmp(str1, str2, ((iLen1 > iLen2) ? iLen1 : iLen2)));
}

/************************************************************************/
/*			stricmp						*/
/************************************************************************/
ST_INT stricmp (ST_CHAR *str1, ST_CHAR *str2)
{
	return(strcmpi(str1, str2));
}

#endif	/* system libs don't provide these functs & don't support strcasecmp, strncasecmp.*/

/************************************************************************/
/*			strncat_maxstrlen				*/
/* Similar to strncat but third arg is the maximum string length.	*/
/* Destination string "dest" is always NULL terminated.			*/
/* NOTE: Destination buffer size should always be at least		*/
/*	 (maxstrlen+1) so there is room for NULL.			*/
/* RETURNS: SD_SUCCESS or SD_FAILURE if maxstrlen already exceeded.	*/
/************************************************************************/
ST_RET strncat_maxstrlen (char *dest, char *src, size_t maxstrlen)
{
	size_t dest_len;
	ST_RET retcode;

	dest_len = strlen (dest);
	if (dest_len < maxstrlen)
	{
		strncat (dest, src, maxstrlen-dest_len);	/* always NULL-terminates dest*/
		retcode = SD_SUCCESS;
	}
	else
		retcode = SD_FAILURE;	/* "dest" is already too long. Don't do anything.	*/
	return (retcode);
}

/************************************************************************/
/*			strncpy_safe					*/
/* Same as strncpy but always NULL terminates "dest" string.		*/
/* strncpy return is useless so this function doesn't bother with return.*/
/************************************************************************/
ST_VOID strncpy_safe (char *dest, const char *src, int max_len)
{
	if (!src || !strlen (src)){
		dest[0] = '\0';
		return;
	}
	strncpy (dest, src, max_len);
	dest[max_len] = '\0';
}

/************************************************************************/
/*			asciiToSlong                                    */
/************************************************************************/

ST_RET asciiToSlong (const ST_CHAR *astr,	 /* ascii string			                            */
					 ST_LONG *sLong)	 /* ptr to long value obtained 
										 from converted ascii string */
{
	ST_CHAR *retPtr;

	*sLong = 0;
	errno = 0; 

	/* convert ascii string to signed long */
	*sLong = strtol (astr, &retPtr, 10);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

	return SD_SUCCESS;
}

/************************************************************************/
/*			asciiToUlong                                    */
/************************************************************************/

ST_RET asciiToUlong (const ST_CHAR *astr,	 /* ascii string			                            */
					 ST_ULONG *uLong)	 /* ptr to long value obtained 
										 from converted ascii string */
{
	ST_CHAR *retPtr;

	*uLong = 0; 
	errno = 0; 

	/* convert ascii string to unsigned long */
	*uLong = strtoul (astr, &retPtr, 10);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

	return SD_SUCCESS;
}

/************************************************************************/
/*			asciiToSint                                     */
/************************************************************************/

ST_RET asciiToSint (const ST_CHAR *astr, /* ascii string		*/
					ST_INT  *sInt)	 /* ptr to value obtained 
									 from converted ascii string */
{
	ST_CHAR *retPtr;
	ST_LONG sLong = 0;

	*sInt = 0;
	errno = 0; 

	/* convert ascii string to signed long */
	sLong = strtol (astr, &retPtr, 10);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

#if (INT_MAX != LONG_MAX)	/* if they are same, don't need to check */
	if (sLong < INT_MIN  || sLong > INT_MAX)
		/* value does not fit in the range of ST_INT */
		return SD_FAILURE;
#endif

	*sInt = (ST_INT) sLong;
	return SD_SUCCESS;
}

/************************************************************************/
/*			asciiToUint                                     */
/************************************************************************/

ST_RET asciiToUint  (const ST_CHAR *astr, /* ascii string               */
					 ST_UINT *uInt)	 /* ptr to value obtained 
									 from converted ascii string */
{
	ST_CHAR  *retPtr;
	ST_ULONG uLong = 0;

	*uInt = 0;
	errno = 0; 

	/* convert ascii string to unsigned long */
	uLong = strtoul (astr, &retPtr, 10);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

#if (UINT_MAX != ULONG_MAX)	/* if they are same, don't need to check */
	if (uLong > UINT_MAX)
		/* value does not fit in the range of ST_UINT */
		return SD_FAILURE;
#endif

	*uInt = (ST_UINT) uLong;
	return SD_SUCCESS;
}

/************************************************************************/
/*			asciiToSint32					*/
/************************************************************************/
ST_RET asciiToSint32 (const ST_CHAR *astr,	/* ascii string		*/
					  ST_INT32 *value)			/* ptr to converted value*/
{
	ST_CHAR  *retPtr;
	ST_LONG sLong = 0;

	*value = 0;
	errno = 0;

	/* convert ascii string to signed long */
	sLong = strtol (astr, &retPtr, 10);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

#if (LONG_MAX < INT32_MAX)	/* LONG smaller than 32 bits	*/
#error This code will not work on this platform.
#endif
#if (LONG_MAX > INT32_MAX)	/* LONG bigger than 32 bits	*/
	/* DEBUG: test this on 64-bit system.	*/
	if (sLong > INT32_MAX || sLong < (-0x80000000))
		/* value does not fit in the range of ST_INT32 */
		return (SD_FAILURE);
#endif
	/* if (LONG_MAX == INT32_MAX), long is same as ST_INT32,	*/
	/* so no need to chk range.					*/

	*value = (ST_INT32) sLong;
	return (SD_SUCCESS);
}

/************************************************************************/
/*			asciiToUint32					*/
/************************************************************************/
ST_RET asciiToUint32 (const ST_CHAR *astr,	/* ascii string		*/
					  ST_UINT32 *value)			/* ptr to converted value*/
{
	ST_CHAR  *retPtr;
	ST_ULONG uLong = 0;

	*value = 0;
	errno = 0; 

	/* convert ascii string to unsigned long */
	uLong = strtoul (astr, &retPtr, 10);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

#if (ULONG_MAX < UINT32_MAX)	/* ULONG smaller than 32 bits	*/
#error This code will not work on this platform.
#endif
#if (ULONG_MAX > UINT32_MAX)	/* ULONG bigger than 32 bits	*/
	if (uLong > UINT32_MAX)
		/* value does not fit in the range of ST_UINT32 */
		return (SD_FAILURE);
#endif
	/* if (ULONG_MAX == UINT32_MAX), unsigned long is same as ST_UINT32,*/
	/* so no need to chk range.					*/

	*value = (ST_UINT32) uLong;
	return (SD_SUCCESS);
}

/************************************************************************/
/*			asciiToSint16                                   */
/************************************************************************/

ST_RET asciiToSint16 (const ST_CHAR *astr,/* ascii string               */
					  ST_INT16 *sInt16)	 /* ptr to value obtained 
										 from converted ascii string */
{
	ST_CHAR *retPtr;
	ST_LONG sLong = 0;

	*sInt16 = 0;
	errno = 0; 

	/* convert ascii string to signed long */
	sLong = strtol (astr, &retPtr, 10);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

	if (sLong < SHRT_MIN  || sLong > SHRT_MAX)
		/* value does not fit in the range of ST_INT16 */
		return SD_FAILURE;

	*sInt16 = (ST_INT16) sLong;
	return SD_SUCCESS;
}

/************************************************************************/
/*			asciiToUint16                                   */
/************************************************************************/

ST_RET asciiToUint16 (const ST_CHAR *astr, /* ascii string		*/
					  ST_UINT16 *uInt16)	 /* ptr to value obtained 
											 from converted ascii string */
{
	ST_CHAR  *retPtr;
	ST_ULONG uLong = 0;

	*uInt16 = 0;
	errno = 0; 

	/* convert ascii string to unsigned long */
	uLong = strtoul (astr, &retPtr, 10);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

	if (uLong > USHRT_MAX)
		/* value does not fit in the range of ST_UINT16 */
		return SD_FAILURE;

	*uInt16 = (ST_UINT16) uLong;
	return SD_SUCCESS;
}

/************************************************************************/
/*			asciiToUchar                                    */
/************************************************************************/

ST_RET asciiToUchar (const ST_CHAR  *astr, /* ascii string              */
					 ST_UCHAR *uChar)	 /* ptr to value obtained 
										 from converted ascii string */
{
	ST_CHAR *retPtr;
	ST_ULONG uLong = 0;

	*uChar = 0;
	errno = 0; 

	/* convert ascii string to unsigned long */
	uLong = strtoul (astr, &retPtr, 10);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

	if (uLong > UCHAR_MAX)
		/* value does not fit in range of ST_UCHAR */
		return SD_FAILURE;

	*uChar = (ST_UCHAR) uLong;
	return SD_SUCCESS;
}

/************************************************************************/
/*			asciiToSint8                                    */
/************************************************************************/

ST_RET asciiToSint8 (const ST_CHAR *astr, /* ascii string               */
					 ST_INT8 *sInt8)	 /* ptr to value obtained 
										 from converted ascii string */
{
	ST_CHAR *retPtr;
	ST_LONG sLong = 0;

	*sInt8 = 0;
	errno = 0; 

	/* convert ascii string to signed long */
	sLong = strtol (astr, &retPtr, 10);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

	if (sLong < CHAR_MIN  || sLong > CHAR_MAX)
		/* value does not fit in range of ST_INT8 */
		return SD_FAILURE;

	*sInt8 = (ST_INT8) sLong;
	return SD_SUCCESS;
}

/************************************************************************/
/*			asciiToUint8                                    */
/************************************************************************/

ST_RET asciiToUint8 (const ST_CHAR *astr, /* ascii string               */
					 ST_UINT8 *uInt8)	 /* ptr to value obtained 
										 from converted ascii string */
{
	ST_CHAR  *retPtr;
	ST_ULONG uLong = 0;

	*uInt8 = 0;
	errno = 0; 

	/* convert ascii string to unsigned long */
	uLong = strtoul (astr, &retPtr, 10);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

	if (uLong > UCHAR_MAX)
		/* value does not fit in range of ST_UINT8 */
		return SD_FAILURE;

	*uInt8 = (ST_UINT8) uLong;
	return SD_SUCCESS;
}

/************************************************************************/
/*			asciiToFloat                                    */
/************************************************************************/

ST_RET asciiToFloat   (const ST_CHAR  *astr,	 			              
					   ST_FLOAT *floatNum)
{
	ST_CHAR  *retPtr;
	ST_DOUBLE doubleNum = 0.0;

	*floatNum = 0.0;
	errno = 0; 

	/* convert ascii string to double */
	doubleNum = strtod (astr, &retPtr);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

	if (fabs(doubleNum) > FLT_MAX)
		/* value does not fit in range of ST_FLOAT */
		return SD_FAILURE;

	*floatNum = (ST_FLOAT) doubleNum;
	return SD_SUCCESS;
}

/************************************************************************/
/*			asciiToDouble                                   */
/************************************************************************/

ST_RET asciiToDouble   (const ST_CHAR *astr,
						ST_DOUBLE *doubleNum)
{
	ST_CHAR  *retPtr;

	*doubleNum = 0.0;
	errno = 0; 

	/* convert ascii string to double */
	*doubleNum = strtod (astr, &retPtr);
	if (retPtr == astr || errno != 0)
		return (SD_FAILURE);		/* conversion error	*/

	return SD_SUCCESS;
}


/************************************************************************/
/************************************************************************/
/* 		Base 10 Integer to ASCII Functions			*/
/************************************************************************/

static char *lToStr(long int val,char *dst, int radix);

ST_CHAR *sInt8ToAscii  (ST_INT8   v, ST_CHAR *p) {lToStr ((long) v, p, -10); return p;}
ST_CHAR *uInt8ToAscii  (ST_UINT8  v, ST_CHAR *p) {lToStr ((long) v, p,  10); return p;}
ST_CHAR *sInt16ToAscii (ST_INT16  v, ST_CHAR *p) {lToStr ((long) v, p, -10); return p;}
ST_CHAR *uInt16ToAscii (ST_UINT16 v, ST_CHAR *p) {lToStr ((long) v, p,  10); return p;}
ST_CHAR *sInt32ToAscii (ST_INT32  v, ST_CHAR *p) {lToStr ((long) v, p, -10); return p;}
ST_CHAR *uInt32ToAscii (ST_UINT32 v, ST_CHAR *p) {lToStr ((long) v, p,  10); return p;}
ST_CHAR *sIntToAscii   (ST_INT    v, ST_CHAR *p) {lToStr ((long) v, p, -10); return p;}
ST_CHAR *uIntToAscii   (ST_UINT   v, ST_CHAR *p) {lToStr ((long) v, p,  10); return p;}
ST_CHAR *sLongToAscii  (ST_LONG   v, ST_CHAR *p) {lToStr ((long) v, p, -10); return p;}
ST_CHAR *uLongToAscii  (ST_ULONG  v, ST_CHAR *p) {lToStr ((long) v, p,  10); return p;}

static char *lToStr(long int val,char *dst, int radix)
{
	char buffer[65];
	char *p;
	long int new_val;

	if (radix < 0)                                /* -10 */
	{
		if (val < 0)
		{
			*dst++ = '-';
			val = -val;
		}
	}

	p = &buffer[sizeof(buffer)-1];
	*p = '\0';
	new_val= (long) ((unsigned long int) val / 10);
	*--p = '0'+ (char) ((unsigned long int) val - (unsigned long) new_val * 10);
	val = new_val;

	while (val != 0)
	{
		new_val=val/10;
		*--p = '0' + (char) (val-new_val*10);
		val= new_val;
	}
	while ((*dst++ = *p++) != 0) ;
	return dst-1;
}

/************************************************************************/
/************************************************************************/

#if 0
#include "stime.h"

int testItoA ()
{
	char dest[100];
	long numMs;
	double endTime;
	double startTime;
	int i;

	startTime = sGetMsTime ();
	for (i = 0; i < 100000; ++i)
		sprintf (dest, "%ld", 698);
	endTime = sGetMsTime ();
	numMs = (long) (endTime - startTime);
	printf ("\n sprintf took %ld ms", numMs); 

	startTime = sGetMsTime ();
	for (i = 0; i < 100000; ++i)
		ltoa (698, dest, 10);
	endTime = sGetMsTime ();
	numMs = (long) (endTime - startTime);
	printf ("\n ltoa took %ld ms", numMs); 

	startTime = sGetMsTime ();
	for (i = 0; i < 100000; ++i)
		lToStr (698, dest);
	endTime = sGetMsTime ();
	numMs = (long) (endTime - startTime);
	printf ("\n lToStr took %ld ms", numMs); 

	lToStr (0, dest);
	if (strcmp (dest, "0"))
		printf ("\nError in lToStr (0, dest);");

	lToStr (-1, dest);
	if (strcmp (dest, "-1"))
		printf ("\nError in lToStr (-1, dest);");

	lToStr (1, dest);
	if (strcmp (dest, "1"))
		printf ("\nError in lToStr (1, dest);");

	sInt16ToAscii (SHRT_MIN, dest);
	if (strcmp (dest, "-32768"))
		printf ("\nError in sInt16ToAscii (SHRT_MIN, dest);");

	sInt16ToAscii (SHRT_MAX, dest);
	if (strcmp (dest, "32767"))
		printf ("\nError in sInt16ToAscii (SHRT_MAX, dest);");

	uInt16ToAscii (USHRT_MAX, dest);
	if (strcmp (dest, "65535"))
		printf ("\nError in sUint16ToAscii (USHRT_MAX, dest);");

	sIntToAscii (INT_MIN, dest);
	if (strcmp (dest, "-2147483648"))
		printf ("\nError in sIntToAscii (INT_MIN, dest);");

	sIntToAscii (INT_MAX, dest);
	if (strcmp (dest, "2147483647"))
		printf ("\nError in sIntToAscii (INT_MAX, dest);");

	uIntToAscii (UINT_MAX, dest);
	if (strcmp (dest, "0xffffffff"))
		printf ("\nError in sUintToAscii (UINT_MAX, dest);");

	sLongToAscii (LONG_MIN, dest);
	if (strcmp (dest, "-2147483648"))
		printf ("\nError in sIntToAscii (INT_MIN, dest);");

	sLongToAscii (LONG_MAX, dest);
	if (strcmp (dest, "2147483647"))
		printf ("\nError in sIntToAscii (INT_MAX, dest);");

	uLongToAscii (ULONG_MAX, dest);
	if (strcmp (dest, "4294967295"))
		printf ("\nError in sUintToAscii (UINT_MAX, dest);");
}
#endif


/************************************************************************/
/*			getKeywordFromFile				*/
/*----------------------------------------------------------------------*/
/* Function to check if given keyword is found in a specified text file.*/
/* When found, keyword value (string) is copied to user buffer valBuf.	*/
/* This function assumes that there is one keyword per line in the file */
/* in the form:								*/
/*   keyword=val							*/
/* Where val is an int, long, string, etc.				*/
/* keywordStr should include the '=' char.				*/
/* Line length is limited to 255 bytes. 				*/
/* Any NL or CR characters are eliminated from the end of line.		*/
/* Parameters:								*/
/*  ST_CHAR *fileStr		file to open				*/
/*  ST_CHAR *keywordStr		keyword to look for in file		*/
/*  ST_CHAR *valBuf		ptr where to put keyword value		*/
/*  ST_UINT  valBufLen		len of buffer for the value.		*/
/* Return:								*/
/*	SD_TRUE		if keyword found.				*/
/*	SD_FALSE	otherwise					*/
/************************************************************************/
ST_BOOLEAN getKeywordFromFile (ST_CHAR *fileStr, ST_CHAR *keywordStr,
							   ST_CHAR *valBuf, ST_UINT valBufLen)
{
	ST_BOOLEAN ret = SD_FALSE;
	ST_CHAR    lineBuf[256];
	ST_CHAR   *keywordPtr = NULL;		/* set if keywordStr found	*/
	FILE      *fd = NULL;
	ST_INT     i;
	ST_INT     copyLen;

	/* make sure we got some valid parameters */
	if (!fileStr || strlen(fileStr) == 0)
		return (ret);
	if (!keywordStr || strlen(keywordStr) == 0)
		return (ret);
	if (!valBuf || valBufLen == 0 || valBufLen >= sizeof(lineBuf))
		return (ret);

	/* open and read the file line by line */
	if ((fd = fopen (fileStr, "r")) != NULL)
	{
		memset (lineBuf, 0, sizeof(lineBuf));
		while (fgets(lineBuf, sizeof(lineBuf)-1, fd) != NULL)
		{
			/* eliminate any NL or CR characters from the end of line */
			for (i=0; i<sizeof(lineBuf); i++)
			{
				if (lineBuf[i] == '\n' || lineBuf[i] == '\r')
					lineBuf[i] = 0;
			}
			/* find the keywordStr in lineBuf */
			keywordPtr = strstr(lineBuf, keywordStr);
			if (keywordPtr)
			{
				copyLen = min (valBufLen-1, strlen(lineBuf)-strlen(keywordStr));
				memcpy (valBuf, &keywordPtr[strlen(keywordStr)], copyLen);
				valBuf[copyLen] = 0;		/* NULL terminate the value	*/
				ret = SD_TRUE;
			}
			/* reset for next line */
			memset (lineBuf, 0, sizeof(lineBuf));
		}
		fclose (fd);
	}

	return (ret);
}

/************************************************************************/
/*			get_next_string					*/
/* Return pointer to next string in input buffer, separated by		*/
/* delimiters (string may be surrounded by "double quotes").		*/
/* ARGUMENTS:								*/
/*   ptrptr - address of current ptr. Current ptr changed by this funct.*/
/*   delimiters - set of delimiter chars (like strtok or strpbrk).	*/
/* RETURNS:	ptr to string found, NULL on error or end of input string.*/
/************************************************************************/
ST_CHAR *get_next_string (ST_CHAR **ptrptr, ST_CHAR *delimiters)
{
	ST_CHAR *curptr;
	ST_CHAR *string;
	ST_CHAR *next_delimiter;

	/* If (*ptrptr==NULL), can't do anything. Just return NULL.		*/
	/* May often happen if last call failed or hit end of input buffer.	*/
	if (*ptrptr == NULL)
		string = NULL;
	else
	{
		curptr = *ptrptr;	/* copy to local var for readability	*/

		/* if next char is double quote, get up to next double quote	*/
		if (*curptr == '\"')
		{
			curptr++;	/* point after starting quote	*/
			string = curptr;
			curptr = strpbrk (curptr, "\"");	/* find ending quote	*/
			/* NOTE: curptr should now point to ending quote, but	*/
			/*       it may be NULL, if ending quote not found.	*/
			if (curptr)
			{
				*curptr = '\0';	/* replace ending quote with NULL	*/
				curptr++;		/* point after ending quote	*/
			}
			if (curptr)
			{	/* make sure next character is delimiter, then skip it.	*/
				next_delimiter = strpbrk (curptr, delimiters);	/* find next delimiter	*/
				if (next_delimiter != curptr)
				{
					SLOG_INFO ("get_next_string ERROR: '%c' after ending quote. Expecting delimiter.", *curptr);
					string = NULL;	/* error	*/
					curptr = NULL;	/* error	*/
				}
				else
					curptr++;	/* skip this delimiter	*/
			}
		}
		else if (*curptr == '\0')
		{		/* no more strings	*/
			string = NULL;
			curptr = NULL;
		}
		else
		{
			string = curptr;
			curptr = strpbrk (curptr, delimiters);	/* find next delimiter	*/
			/* NOTE: curptr should now point to ending delimiter, but	*/
			/*       it may be NULL, if expected delimiter not found.	*/
			if (curptr)
			{
				*curptr = '\0';		/* replace delimiter with NULL	*/
				curptr++;		/* point after delimiter	*/
			}
		}

		*ptrptr = curptr;	/* NULL on any error OR end of input string	*/
	}
	return (string);	/* return ptr to string (NULL on any error)	*/ 
}      

ST_VOID replacechar(ST_CHAR *str,ST_CHAR des,ST_CHAR src)
{
	if (str==0)
		return;
	while ( *str !=0 )
	{
		if(*str==des)
			*str=src;
		str++;
	}
}