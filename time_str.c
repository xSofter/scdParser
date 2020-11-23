/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*		1994 - 2008, All Rights Reserved			*/
/*									*/
/* MODULE NAME : time_str.c						*/
/* PRODUCT(S)  : 							*/
/*									*/
/* MODULE DESCRIPTION : 				  		*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 06/24/08  EJV     41    Btime6StringToVals CORR: *1000 was missing.	*/
/************************************************************************/

#if defined (_WIN32)
#pragma warning(disable : 4996)
#endif

#include "glbtypes.h"
#include "sysincs.h"
#include "slog.h"
#include "time_str.h"
#include "str_util.h"

#ifdef DEBUG_SISCO
static SD_CONST ST_CHAR *SD_CONST thisFileName = __FILE__;
#endif

/************************************************************************/

ST_CHAR *tstrTimeFormat  = TSTR_DEF_TIME_FORMAT;
ST_CHAR *BtimeTimeFormat = BTIME_DEF_TIME_FORMAT;

/************************************************************************/
/************************************************************************/
/*                       tstrTimeToString  				*/
/************************************************************************/

ST_RET tstrTimeToString (time_t t, ST_CHAR *dest)
{
	strftime (dest, MAX_TIME_STRING_LEN, tstrTimeFormat, localtime (&t));
	return (SD_SUCCESS);
}


/************************************************************************/
/*                       tstrTimeToStringGmt				*/
/*  this code was merged from time_str2.c                               */
/************************************************************************/

ST_RET tstrTimeToStringGmt (time_t t, ST_CHAR *dest)
{
	strftime (dest, MAX_TIME_STRING_LEN, tstrTimeFormat, gmtime (&t));
	return (SD_SUCCESS);
}


/************************************************************************/
/************************************************************************/

/* structure to hold results from parsing date and time strings  */

typedef struct
{
	int                       month;
	int                       day;
	int                       year;
	int                       hour;
	int                       min;
	int                       sec;
	int                       mSec;
	int                       uSec;
	int                       nSec;
	int                       zoneHour; 
	int                       zoneMin;
	char                      zoneCode;     /* 'Z', '+', '-' or 0 */
	int                       dateFound;    /* not needed by new parsers */
	int                       timeFound;    /* not needed by new parsers */
	int                       order;        /*date-field ordering; 0 =f ANY */
	char                      *pflags;      /* ptr to (qual= ... ) or NULL */

} _TS_DATETIME;

#if 0
/* obsoleted code */
static int parseDateString (char *s, _TS_DATETIME *dt);
static int parseTimeString (char *s, _TS_DATETIME *dt);
static int parseBtimeString (char *s, _TS_DATETIME *dt);

static int strToMonth (char *s, int *monthOut);
#endif

/************************************************************************/


typedef struct
{
	ST_INT32                  /*O*/ len;        /* length of field */
	ST_INT32                  /*O*/ value;      /* extracted num value */ 
	ST_INT32                  /*O*/ scale;      /* for scaling fractions */
	ST_CHAR                   /*O*/ delim;      /* field delimiter */
	ST_CHAR                   /*O*/ text[16];   /* extracted text value */
} DATETIME_FIELD;


/************************************************************************/
/*                       getDateTimeField				*/
/*  extract a (possible) date or time field, and report its value       */
/*  return pointer to next position to parse, or NULL if no field found */
/*  a _DATETIME_FIELD len of 0 also means the field is not valid        */
/************************************************************************/

static ST_CHAR *            getDateTimeField (
	ST_CHAR *                 /*I*/ buf,        /* field being parsed */
	ST_CHAR                   /*I*/ prevDelim,  /* '.' allows longer num */
	DATETIME_FIELD *          /*O*/ dtf)
{
	ST_INT32                  i;
	ST_CHAR                   work[4];

	static ST_CHAR *          monthTab[13] =
	{
		"",
		"JAN", "FEB", "MAR",
		"APR", "MAY", "JUN",
		"JUL", "AUG", "SEP",
		"OCT", "NOV", "DEC"
	};


	if (dtf == NULL)
	{
		return NULL;    /* bad parameter */
	}

	dtf->len   = 0;
	dtf->value = 0;
	dtf->scale = 1;
	dtf->delim = ' ';
	dtf->text[0] = 0;

	if (buf == NULL)
	{
		return NULL;    /* bad buf pointer */
	}

	/* skip leading whitespace */

	while (isspace (*buf))
	{
		buf++;
	}

	if (*buf == 0)
	{
		return NULL;    /* buf has no data */
	}

	/* accumulate value */

	while (isdigit (*buf))
	{
		dtf->text[dtf->len] = *buf;
		dtf->text[dtf->len+1] = 0;

		dtf->value = (dtf->value * 10) + (*buf - '0');
		dtf->scale *= 10;
		dtf->len++;
		buf++;

		if (dtf->len > 9)
		{
			return NULL;    /* numeric field too long */
		}
	} /* while */

	if (dtf->len > 0)    /* field is numeric */
	{

		/* only lengths 1, 2 and 4 are allowed */
		/* this allows for 1 or 2 digit months, days and years */
		/* and 4 digit years */

		/* when prevDelim is '.', the current field is a fraction */
		/* so, we allow longer field values */

		if (prevDelim == '.')
		{
			if (dtf->len == 9)
			{
				/* a (fake) length of 9 is a 'marker' for an alpha month */
				/* we change numbers of length 9 to 8 to get around this */

				/* we are lying about the length; don't worry, it's OK */

				/* the precise length of fractional seconds isn't critical anyway. */
				/* we need lengths mainly to analyze month/day/year ordering. */

				dtf->len = 8;
			}

			else if (dtf->len > 9)
			{
				/* too many digits for a fraction */
				return NULL;
			}
		}

		else /* prevDelim != '.' */
		{
			/* numbers for dates and times only have valid lengths of 1, 2 or 4 */
			/* except for fractional seconds, which can vary from 0 to 9 digits */

			if ( (dtf->len > 4)
				||   (dtf->len == 3) )
			{
				/* not a valid date or time numeric field size */
				return NULL;
			}

			/* if a number is followed by . then a blank, nul, +/-, T or Z */
			/* the . is just noise, so ignore it */

			/* this test is not made on the "if prevDelim == '.'" branch above */
			/* we don't want to permit values like .123. which is illegal */

			if (*buf == '.')
			{
				if ( (buf[1] ==  0 )
					||   (buf[1] == ' ')
					||   (buf[1] == '+')
					||   (buf[1] == '-')
					||   (buf[1] == 'T')
					||   (buf[1] == 'Z') )
				{
					buf++;    /* a pointless dot */
				}
			} /* *buf == '.' */

		} /* when prevDelim != '.' */

		/* next field is after curr delim at 'buf'; assume delim len is 1 */
		/* assume curr pos is the delimiter */
		/* example: buf == "-12", so delim == '-' and next field "12" at buf+1 */
		/* this holds in most cases, except AM/PM, when next field at buf+2 */

		dtf->delim = (ST_CHAR) toupper (*buf);

		if (*buf == 0)    /* end of buffer */
		{
			/* delim is blank when there is no non-blank delimiter */
			/* to determine if end of buffer, look at dtf->buf */
			/* we don't return buf+1 because end of buffer was reached */
			/* be careful not to run off end of buffer */
			dtf->delim = ' ';
			return buf;
		}

		if (isspace (*buf))
		{
			/* delim is blank when there is no non-blank delimiter */
			/* to determine if end of buffer, look at dtf->buf */
			dtf->delim = ' ';     /* just in case of tabs, etc. */
			return buf + 1;
		}

		if ( (*buf == ':')      /* part of a time */
			||   (*buf == '.')      /* part of a time */
			||   (*buf == '-')      /* part of a date */
			||   (*buf == '+') )    /* possibly part of a timezone offset */
		{
			return buf + 1;
		}

		if (*buf == '/')
		{
			dtf->delim = '-';     /* treat '/' same as '-' for dates */
			return buf + 1;
		}

		if ( (dtf->delim == 'T')
			&&   (isdigit (buf[1])) )    /* value is part of ISO 8601 time */
		{
			return buf + 1;
		}

		if (dtf->delim == 'Z')
		{
			if ( (buf[1] <= ' ')       /* Zulu/GMT timezone suffix */
				||   (buf[1] == '(') )     /* Z followed by (qual= ... ) */
			{
				return buf + 1;
			}
		} /* Z */

		/* look for A, P, AM or PM suffix */

		if ( (dtf->delim == 'A')
			||   (dtf->delim == 'P') )
		{
			if (!isalnum (buf[1]))          /* short AM/PM suffix */
			{
				return buf + 1;
			}

			if ( (toupper (buf[1]) == 'M')  /* long AM/PM suffix */
				&&   (!isalnum (buf[2])) )      /* good delim after M */
			{
				return buf + 2;
			}

			return NULL;                    /* malformed AM/PM */

		} /* A or P delim */

	} /* field is numeric */

	/* look for alphabetic month */
	/* we are supporting the following date formats: */

	/* 2006-Dec-17 */
	/*      Dec-17-06 */
	/*      Dec-17-2006 */
	/*      17-Dec-2006 */

	/* in each case, the alpha month is followed by '-' or '/' */

	/* the format 06-Dec-17 is not supported, because it is not */
	/* clear if it is day-month-year or year-month-day order. */

	/* if found, return equivalent value (1 to 12) and a (fake) length of 9 */
	/* check for length of 9 is used to see if an alpha month is present */

	if ( (buf[3] != '-')
		&&   (buf[3] != '/') )
	{
		return NULL;
	}

	work[0] = (ST_CHAR) toupper (buf[0]);
	work[1] = (ST_CHAR) toupper (buf[1]);
	work[2] = (ST_CHAR) toupper (buf[2]);
	work[3] = 0;

	strcpy (dtf->text, work);

	for (i=1; i <= 12; i++)
	{
		if (strcmp (work, monthTab[i]) == 0)
		{
			dtf->delim = '-';     /* treat '/' same as '-' for dates */
			dtf->value = i;
			dtf->len = 9;         /* fake length of 9 = alpha month */
			return buf + 4;
		}
	}

	return NULL;    /* no valid date/time field found */

} /* getDateTimeField */


/************************************************************************/
/*                       getDateOrder                                   */
/*                                                                      */
/* on Windows systems, use GetLocaleInfo to determine the ordering of   */
/* month, day and year fields in a date, according to the current       */
/* locale.  on non-Windows systems, assume m/d/y.                       */
/*                                                                      */
/************************************************************************/

#ifdef _WIN32
/* on Windows, GetLocaleInfo() depends on windows.h and kernel32.lib */
#pragma comment(lib, "kernel32.lib")
#include <windows.h>
#endif

ST_INT32                         getDateOrder ()
{
#ifdef _WIN32
	ST_CHAR                      work[2];
	ST_INT32                  rc;

	work[0] = work[1] = 0;
	rc = GetLocaleInfoA (LOCALE_USER_DEFAULT, LOCALE_ILDATE, work, 2); 

	if (rc != 2)
	{
		return S_DATE_ORDER_MDY;  
	}

	if (work[0] == '1') 
	{
		return S_DATE_ORDER_DMY;  
	}

	if (work[0] == '2') 
	{
		return S_DATE_ORDER_YMD;  
	}

	/* in case of error, or default '0' returned */
	return S_DATE_ORDER_MDY;  

#else
	/* unable to determine ordering, so assume a default order */
	return S_DATE_ORDER_MDY;  
#endif
}


/************************************************************************/
/*                       storeDateValue                                 */
/*                                                                      */
/* take a DATETIME_FIELD array, determine field order of a date, and    */
/* store into a _TS_DATETIME.  if DATETIME_FIELD array is valid, and    */
/* the fields appear to be in the correct order, return SD_SUCCESS,     */
/* else return SD_FAILURE.                                              */
/*                                                                      */
/************************************************************************/

ST_RET                      storeDateValue (
	DATETIME_FIELD *          /*I*/  dtf, 
	_TS_DATETIME *            /*IO*/ dt)
{
	ST_INT32                  len;
	ST_INT32                  mm;
	ST_INT32                  dd;
	ST_INT32                  yy;
	ST_INT32                  order;

	if (dt == NULL)
	{
		return SD_FAILURE;
	}

	dt->month = 0;
	dt->day   = 0;
	dt->year  = 0;
	dt->dateFound = SD_FALSE;

	if (dtf == NULL)
	{
		return SD_FAILURE;
	}

	if ( (dtf[0].delim != '-')
		||   (dtf[1].delim != '-') )
	{
		return SD_FAILURE;
	}

	if ( (dtf[2].delim != 'T')
		&&   (dtf[2].delim != ' ') )
	{
		return SD_FAILURE;
	}

	/* determine field order by the lengths of 3 fields */
	/* form a single int value for easier testing */

	/* for example, 412 means a 4-digit field, then a 1-digit field */
	/* and finally a 2-digit field, in that order. */

	len = (100 * dtf[0].len) + (10 * dtf[1].len) + (dtf[2].len);

	/* if a specific date order is requested by caller, use it. */
	/* otherwise, ask system for the default order */

	if (dt->order == S_DATE_ORDER_ANY)
	{
		/* caller accepts any currently active order, based on locale */
		order = getDateOrder ();
	}

	else
	{
		/* caller insisted on date fields being in a particular order */
		order = dt->order;
	}

	switch (len)
	{
	case 411:   /* yyyy-m-d    : ordering is confident */
	case 412:   /* yyyy-m-dd   : ordering is confident */
	case 421:   /* yyyy-mm-d   : ordering is confident */
	case 422:   /* yyyy-mm-dd  : ordering is confident */
	case 491:   /* yyyy-Mon-d  : ordering is confident */
	case 492:   /* yyyy-Mon-dd : ordering is confident */

		/* when date starts with a 4-digit number, the first field is a year. */
		/* no (known) locales use year-day-month, so we can assume the order */
		/* is year-month-day here, regardless of locale, with confidence. */

		yy = dtf[0].value;
		mm = dtf[1].value;
		dd = dtf[2].value;
		break;

	case 914:   /* Mon-d-yyyy  : ordering is certain */
	case 924:   /* Mon-dd-yyyy : ordering is certain */

		mm = dtf[0].value;
		dd = dtf[1].value;
		yy = dtf[2].value;
		break;

	case 114:   /*   m-d-yyyy or   d-m-yyyy */
	case 124:   /*  m-dd-yyyy or  d-mm-yyyy */
	case 214:   /*  mm-d-yyyy or  dd-m-yyyy */
	case 224:   /* mm-dd-yyyy or dd-mm-yyyy */

		if (order == S_DATE_ORDER_DMY)
		{
			dd = dtf[0].value;
			mm = dtf[1].value;
		}

		else
		{
			/* for S_DATE_ORDER_MDY, the field ordering is certain */
			/* for S_DATE_ORDER_YMD, the field ordering is not certain */
			/* but since YMD implies month before year, we assume this is right */

			mm = dtf[0].value;
			dd = dtf[1].value;
		}

		yy = dtf[2].value;
		break;

	case 194:   /*  d-Mon-yyyy : for alpha month, ordering is certain */
	case 294:   /* dd-Mon-yyyy : for alpha month, ordering is certain */

		dd = dtf[0].value;
		mm = dtf[1].value;
		yy = dtf[2].value;
		break;

	case 911:   /*  Mon-d-y  : ordering is confident */
	case 921:   /* Mon-dd-y  : ordering is confident */
	case 912:   /*  Mon-d-yy : ordering is confident */
	case 922:   /* Mon-dd-yy : ordering is confident */

		/* for alpha month, there is no such format as Mon-yy-dd */
		/* so, the mm-dd-yy ordering is reasonably certain */

		mm = dtf[0].value;
		dd = dtf[1].value;
		yy = dtf[2].value;

		/* year is 2-digit, so form the default century */
		/* we would normally have a cutoff of 1970, but in some cases */
		/* a conversion error will leave a time_t with Dec 31 1969 */
		/* so, values < 69 are assumed to be in the 21 century */

		if (yy < 69)
		{
			yy += 2000;
		}

		else
		{
			yy += 1900;
		}

		break;

	case 191:   /*  d-Mon-y  */
	case 291:   /* dd-Mon-y  */
	case 192:   /*  d-Mon-yy */
	case 292:   /* dd-Mon-yy */

		if (order == S_DATE_ORDER_YMD)
		{
			yy = dtf[0].value;
			mm = dtf[1].value;
			dd = dtf[2].value;
		}

		else
		{
			/* for S_DATE_ORDER_DMY, the field ordering is certain */
			/* for S_DATE_ORDER_MDY, the field ordering is not certain */
			/* but since MDY implies day before year, we assume this is right */

			dd = dtf[0].value;
			mm = dtf[1].value;
			yy = dtf[2].value;
		}

		/* year is 2-digit, so form the default century */
		/* we would normally have a cutoff of 1970, but in some cases */
		/* a conversion error will leave a time_t with Dec 31 1969 */
		/* so, values < 69 are assumed to be in the 21 century */

		if (yy < 69)
		{
			yy += 2000;
		}

		else
		{
			yy += 1900;
		}

		break;

	case 111:   /*   d-m-y      y-m-d       m-d-y   */
	case 112:   /*   d-m-yy     y-m-dd      m-d-yy  */

	case 121:   /*   d-mm-y     y-mm-d      m-dd-y  */
	case 122:   /*   d-mm-yy    y-mm-dd     m-dd-yy */

	case 211:   /*  dd-m-y     yy-m-d      mm-d-y   */
	case 212:   /*  dd-m-yy    yy-m-dd     mm-d-yy  */

	case 221:   /*  dd-mm-y    yy-mm-d     mm-dd-y  */
	case 222:   /*  dd-mm-yy   yy-mm-dd    mm-dd-yy */

		if (order == S_DATE_ORDER_DMY)
		{
			dd = dtf[0].value;
			mm = dtf[1].value;
			yy = dtf[2].value;
		}

		else if (order == S_DATE_ORDER_YMD)
		{
			yy = dtf[0].value;
			mm = dtf[1].value;
			dd = dtf[2].value;
		}

		else /* assume mm-dd-yy */
		{
			mm = dtf[0].value;
			dd = dtf[1].value;
			yy = dtf[2].value;
		}

		/* year is 2-digit, so form the default century */
		/* we would normally have a cutoff of 1970, but in some cases */
		/* a conversion error will leave a time_t with Dec 31 1969 */
		/* so, values < 69 are assumed to be in the 21 century */

		if (yy < 69)
		{
			yy += 2000;
		}

		else
		{
			yy += 1900;
		}

		break;

	default:
		return SD_FAILURE;

	} /* switch */

	if ( (mm < 1)
		||   (mm > 12)
		||   (dd < 1)
		||   (dd > 31)
		||   (yy < 1969) )
	{
		return SD_FAILURE;
	}

	dt->month = mm;
	dt->day   = dd;
	dt->year  = yy;
	dt->dateFound = SD_TRUE;

	return SD_SUCCESS;

} /* storeDateValue */


/************************************************************************/
/*                          storeTimeValue                              */
/*                                                                      */
/* take a DATETIME_FIELD array, determine field order of a time, and    */
/* store into a _TS_DATETIME.  if DATETIME_FIELD array is valid, and    */
/* the fields appear to be in the correct order, return SD_SUCCESS,     */
/* else return SD_FAILURE.                                              */
/*                                                                      */
/************************************************************************/

ST_RET                      storeTimeValue (
	DATETIME_FIELD *          /*I*/ dtf, 
	_TS_DATETIME *            /*O*/ dt)
{
	ST_INT32                  n = 0;
	ST_INT32                  len;
	ST_INT32                  hour = 0;
	ST_INT32                  min  = 0;
	ST_INT32                  zoneHour = 0;
	ST_INT32                  zoneMin  = 0;
	ST_INT32                  sec  = 0;
	ST_INT32                  mSec = 0;
	ST_INT32                  uSec = 0;
	ST_INT32                  nSec = 0;
	ST_CHAR                   work[32];


	if (dt == NULL)
	{
		return SD_FAILURE;
	}

	dt->hour = 0;
	dt->min  = 0;
	dt->sec  = 0;
	dt->mSec = 0;
	dt->uSec = 0;
	dt->nSec = 0;
	dt->timeFound = SD_FALSE;

	dt->zoneHour = 0;
	dt->zoneMin  = 0;
	dt->zoneCode = 0;

	if (dtf == NULL)
	{
		return SD_FAILURE;
	}

	/* hh:mm:ss[A] [.frac] [+hh:mm] yyyy-mm-dd */
	/* 0  1  2       3       4  5   6    7  8  */


	if ( (dtf[n].delim != ':')
		&&   (dtf[n].len   != 1)
		&&   (dtf[n].len   != 2) )
	{
		return SD_FAILURE;
	}

	hour = dtf[n++].value;

	/* store minutes, a required field */

	if ( (dtf[n].len != 1)
		&&   (dtf[n].len != 2) )
	{
		return SD_FAILURE;
	}

	min = dtf[n].value;

	/* see if seconds field is present */

	if (dtf[n].delim == ':')
	{
		n++;  /* look at seconds field */

		if ( (dtf[n].len != 1)
			&&   (dtf[n].len != 2) )
		{
			return SD_FAILURE;
		}

		sec = dtf[n].value;
	}

	/* see if AM/PM field is present, and validate hour */
	/* in AM/PM mode, hour cannot be 00 or > 12 */ 
	/* so 00:00 AM and 00:00 PM are illegal */
	/* as are 14:00 PM etc. */

	if (dtf[n].delim == 'A')
	{
		if ((hour < 1) || (hour > 12))
		{
			return SD_FAILURE;
		}

		if (hour == 12)
		{
			hour = 0;    /* 12:05 AM is really 00:00 */
		}
	}

	else if (dtf[n].delim == 'P')
	{
		if ((hour < 1) || (hour > 12))
		{
			return SD_FAILURE;
		}

		/* 12:05 PM is 12:05 (so hour == 12 is OK), but 1:05 PM is 13:05 */

		if (hour < 12)
		{
			hour += 12;
		}
	}

	/* if decimal point is followed by nothing, it is a null fraction */
	/* if so, treat this as the end of the string */
	/* for example, "12:34." is the same as "12:34" */

	if ( (dtf[n].delim == '.')
		&&   (dtf[n+1].len == 0) )
	{
		dtf[n].delim = ' ';   /* end of string */
	}

	if (dtf[n].delim == '.')
	{
		/* extract mSec */
		/* this requires normalizing the value to 3 digits */
		/* since the current field ends with a dot, there must be */
		/* a following field that is numeric */

		n++;    /* look at fraction field */

		len  = dtf[n].len;

		if (len > 9)
		{
			return SD_FAILURE;    /* malformed/missing mSec field */
		}

		/* express fraction as a count of milliseconds */
		/* method: pad or truncate ms field to 3 digits */

		strncpy (work, dtf[n].text, 3);
		work[3] = 0;
		strcat (work, "000");
		work[3] = 0;
		mSec = atoi (work);

		/* express fraction as a count of microseconds */
		/* method: pad or truncate ms field to 6 digits */

		strncpy (work, dtf[n].text, 6);
		work[6] = 0;
		strcat (work, "000000");
		work[6] = 0;
		uSec = atoi (work);

		/* express fraction as a count of nanoseconds */
		/* method: pad or truncate ms field to 9 digits */

		strncpy (work, dtf[n].text, 9);
		work[9] = 0;
		strcat (work, "000000000");
		work[9] = 0;
		nSec = atoi (work);
	}

	if (dtf[n].delim == 'Z')
	{
		dt->zoneCode = 'Z';
	}

	else if ( (dtf[n].delim == '+')      /* +hh:mm or +h:mm */
		||        (dtf[n].delim == '-') )    /* -hh:mm or -h:mm */
	{
		dt->zoneCode = dtf[n].delim;
		/* look at zone hour, it may be 1 or 2 digits */
		n++;

		/* allow for zone of +hhmm or -hhmm if it's the last field */

		if ( (dtf[n].delim == ' ')
			||   (dtf[n].delim == 'T') )
		{
			if (dtf[n].len != 4)
			{
				return SD_FAILURE;    /* in this format, exactly 4 digits needed */
			}

			zoneHour = (dtf[n].value) / 100;
			zoneMin  = (dtf[n].value) % 100;
		}

		else
		{
			if ( (dtf[n].delim != ':')    /* zone hour must have minutes */
				&&   (dtf[n].len   != 1)
				&&   (dtf[n].len   != 2) )
			{
				return SD_FAILURE;
			}

			zoneHour = dtf[n++].value;    /* grab hours, index to minutes */

			if (dtf[n].len != 2)
			{
				return SD_FAILURE;
			}

			zoneMin = dtf[n].value;

			if ( (zoneHour <  0)
				||   (zoneHour > 23)
				||   (zoneMin  <  0)
				||   (zoneMin  > 59) )
			{
				return SD_FAILURE;
			}
		}
	}

	if ( (hour <  0)
		||   (hour > 23)
		||   (min  <  0)
		||   (min  > 59)
		||   (sec  <  0) 
		||   (sec  > 59) )
	{
		return SD_FAILURE;
	}

	dt->hour = hour;
	dt->min  = min;
	dt->sec  = sec;
	dt->mSec = mSec;
	dt->uSec = uSec;
	dt->nSec = nSec;
	dt->timeFound = SD_TRUE;

	dt->zoneHour = zoneHour;
	dt->zoneMin  = zoneMin;

	return SD_SUCCESS;

} /* storeTimeValue */


/************************************************************************/
/*                       getTsDateTime					*/
/* parse a string containing a time and date value, and produce a       */
/* struct tm value.  accepted formats are:                              */
/*                                                                      */
/*     time [fraction] date                                             */
/*     date time [fraction]                                             */
/*                                                                      */
/* date may be in the following formats:                                */
/*                                                                      */
/*     yyyy-mm-dd                                                       */
/*     yy-mm-dd     (may be used in some locales)                       */
/*     mm-dd-yy     (US format)                                         */
/*     mm-dd-yyyy   (US format)                                         */
/*     dd-mm-yy     (European format)                                   */
/*     dd-mm-yyyy   (European format)                                   */
/*                                                                      */
/*     yyyy-Mon-dd                                                      */
/*     Mon-dd-yy                                                        */
/*     Mon-dd-yyyy                                                      */
/*     dd-Mon-yyyy  (European format)                                   */
/*                                                                      */
/* when date format may be ambiguous, the system is queried as to the   */
/* default field ordering, based on the current locale.  for example,   */
/* 01-02-03 could be Jan 01 2003, 01 Feb 2003 or 2001 Feb 03.           */
/*                                                                      */
/* when date contains one 4-digit value, it is assumed to be a year.    */
/* this helps to unambiguate some values.                               */
/*                                                                      */
/* date delimiter may be '-' or '/'                                     */
/*                                                                      */
/* time may or may not have a fraction and/or a timezone                */
/*                                                                      */
/* max number of fields possible: 9, consisting of:                     */
/*   date:     3                                                        */
/*   time:     3                                                        */
/*   fraction: 1                                                        */
/*   timezone: 2                                                        */
/*                                                                      */
/************************************************************************/

static ST_RET               getTsDateTime (
	ST_CHAR *                 /*I*/  in_buf, 
	_TS_DATETIME *            /*IO*/ out_dt)
{
	ST_INT32                  maxfield = 10;
	ST_INT32                  numfields = 0;
	ST_INT32                  i;
	ST_CHAR *                 buf;
	ST_CHAR                   prevDelim = ' ';
	ST_INT32                  datefield;
	ST_INT32                  timefield;
	DATETIME_FIELD            dtf[10] = {{0}};
	_TS_DATETIME              dt = {0};


	if ((in_buf == NULL) || (out_dt == NULL))
	{
		return SD_FAILURE;      /* bad parameters */
	}

	dt.order = out_dt->order;    /* copy date-ordering option */

	/* extract fields */
	/* prevDelim is set so that a '.' will allow the next field */
	/* to have 1 to 7 digits */

	buf = in_buf;

	for (i=0; i < maxfield; i++)
	{
		buf = getDateTimeField (buf, prevDelim, &dtf[i]);

		if (buf != NULL)
		{
			prevDelim = dtf[i].delim;
			numfields++;

			if (*buf == '(')
			{
				dt.pflags = buf;    /* start of quality-flag field */
				break;
			}
		}
	} /* for */

	if (numfields < 5)
	{
		return SD_FAILURE;    /* we were expecting year,mon,day,hour,min fields */
	}

	/* determine field order */

	if ( (dtf[0].delim == '-')
		&&   (dtf[1].delim == '-') )
	{
		/* first field is date */
		/* example field layout: */

		/* yyyy-mm-dd [T] hh:mm:ss[A] [.frac] [+hh:mm] */
		/* 0    1  2      3  4  5       6       7  8   */

		if ( (dtf[2].delim != ' ')
			&&   (dtf[2].delim != 'T') )
		{
			return SD_FAILURE;    /* bad delimiter */
		}

		datefield = 0;
		timefield = 3;
	} /* if - - found */

	else if (dtf[0].delim == ':')
	{
		/* first field is time (T code would not be present) */
		/* example field layout: */

		/* hh:mm:ss[A] [.frac] [Z|+hh:mm|-hh:mm] yyyy-mm-dd */
		/* 0  1  2       3         4  5          6    7  8  */

		/* ensure we have a valid time format */
		/* mm must be followed by AM/PM code, :ss or space, not a '-' */

		if ( (dtf[1].delim != ' ')
			&&   (dtf[1].delim != ':')
			&&   (dtf[1].delim != 'Z')
			&&   (dtf[1].delim != '+')
			&&   (dtf[1].delim != '-')
			&&   (dtf[1].delim != 'A')
			&&   (dtf[1].delim != 'P') )
		{
			return SD_FAILURE;    /* bad delimiter */
		}

		timefield = 0;
		datefield = 0;

		/* find date field by looking for two '-' delimiters */
		/* if numfields == 9, then last field is at [8] */
		/* so we look for pairs up to [numfields-1] */

		for (i = 2; i < numfields-1; i++)
		{
			if ( (dtf[i].delim   == '-')
				&&   (dtf[i+1].delim == '-') )
			{
				datefield = i;
				break;
			}
		} /* for */

		if (datefield == 0)
		{
			return SD_FAILURE;    /* cannot find date field */
		}

	} /* if : found */

	else
	{
		return SD_FAILURE;    /* cannot determine date-time vs. time-date */
	}

	/* populate dt structure with date and time values */

	if (storeDateValue (&dtf[datefield], &dt) != SD_SUCCESS)
	{
		return SD_FAILURE;    /* malformed date */
	}

	if (storeTimeValue (&dtf[timefield], &dt) != SD_SUCCESS)
	{
		return SD_FAILURE;    /* malformed time */
	}

	*out_dt = dt;

	return SD_SUCCESS;

} /* getTsDateTime */


/************************************************************************/
/*                       tstrStringToTm					*/
/* parse a string containing a time and date value, and produce a       */
/* struct tm value.  method: convert to _TS_DATEITIME via getTsDateTime */
/* then convert result to struct tm.                                    */
/************************************************************************/

ST_RET tstrStringToTm (ST_CHAR *in_buf, struct tm *out_struct_tm)
{
	_TS_DATETIME              dt = {0};

	struct tm                 w_struct_tm = {0};

	if ((in_buf == NULL) || (out_struct_tm == NULL))
	{
		return SD_FAILURE;      /* bad parameters */
	}

	/* compatibility with old time_str.c requires month-day-year ordering */

	dt.order = S_DATE_ORDER_MDY;

	if (getTsDateTime (in_buf, &dt) != SD_SUCCESS)
	{
		return SD_FAILURE;      /* bad parameters */
	}

	/*  convert the components to struct tm */

	w_struct_tm.tm_year  = dt.year - 1900;
	w_struct_tm.tm_mon   = dt.month - 1;
	w_struct_tm.tm_mday  = dt.day;
	w_struct_tm.tm_hour  = dt.hour;
	w_struct_tm.tm_min   = dt.min;
	w_struct_tm.tm_sec   = dt.sec;
	w_struct_tm.tm_isdst = -1;  	        /* let api determine DST */

	usr_mkgmtime (&w_struct_tm);          /* generate tm_wday, tm_yday */

	/* we do not know if supplied date was GMT or not, so DST is uncertain */
	w_struct_tm.tm_isdst = -1;  	        /* a reasonable default value */

	*out_struct_tm = w_struct_tm;

	return SD_SUCCESS;

} /* tstrStringToTm */


/************************************************************************/
/*                       tstrStringToTime				*/
/* parse a string containing a time and date value, and produce a       */
/* time_t value.  method: convert to struct tm via tstrStringToTm,      */
/* then convert result to time_t.                                       */
/************************************************************************/

ST_RET tstrStringToTime (ST_CHAR *in_buf, time_t *out_time_t)
{
	time_t                    w_time_t;
	struct tm                 w_struct_tm;

	if ((in_buf == NULL) || (out_time_t == NULL))
	{
		return SD_FAILURE;      /* bad parameters */
	}

	if (tstrStringToTm (in_buf, &w_struct_tm) != SD_SUCCESS)
	{
		return SD_FAILURE;      /* parse failed */
	}

	w_time_t = mktime (&w_struct_tm);

	if (w_time_t == (time_t) -1)
	{
		return SD_FAILURE;
	}

	*out_time_t = w_time_t;
	return SD_SUCCESS;

} /* tstrStringToTime */


/************************************************************************/
/*                       tstrStringToTimeGmt                            */
/* time_t value.  method: convert to struct tm via tstrStringToTm,      */
/* then convert result to time_t using usr_mkgmtime().                  */
/************************************************************************/

ST_RET tstrStringToTimeGmt (ST_CHAR *in_buf, time_t *out_time_t)
{
	time_t                    w_time_t;
	struct tm                 w_struct_tm;

	if ((in_buf == NULL) || (out_time_t == NULL))
	{
		return SD_FAILURE;      /* bad parameters */
	}

	if (tstrStringToTm (in_buf, &w_struct_tm) != SD_SUCCESS)
	{
		return SD_FAILURE;      /* parse failed */
	}

	w_time_t = usr_mkgmtime (&w_struct_tm);

	if (w_time_t == (time_t) -1)
	{
		return SD_FAILURE;
	}

	*out_time_t = w_time_t;
	return SD_SUCCESS;

} /* tstrStringToTimeGmt */


/************************************************************************/
/*                       tstrTmToString					*/
/* Convert struct tm to string.						*/
/************************************************************************/

ST_RET tstrTmToString (struct tm *t, ST_CHAR *dest)
{
	ST_CHAR *timeFormatStr = "%m-%d-%Y %H:%M:%S";
	strftime (dest, 30, timeFormatStr, t);
	return (SD_SUCCESS);
}


#if 0
/* obsoleted code */

/************************************************************************/
/*			parseDateString					*/
/************************************************************************/

/* parseDateString IS NO LONGER REFERENCED IN THIS CODE */

static ST_RET parseDateString (ST_CHAR *s, _TS_DATETIME *dt)
{
	ST_CHAR *p;
	ST_CHAR *d1;
	ST_CHAR *d2;
	ST_CHAR *d3;
	int century;

	dt->dateFound = SD_TRUE;

	/* break into three substrings */
	d1 = s;
	p =  strpbrk (s,"-");
	*p = 0;
	d2 = ++p;
	p =  strpbrk (d2,"-");
	if (!p)
		return (SD_FAILURE);
	*p = 0;
	d3 = ++p;

	/* Now process each date sub-substring seperately  */

	/* Check for alpha month forms */
	if (isalpha (*d1))
	{
		if (strToMonth (d1, &dt->month))
			return (SD_FAILURE);
		if (!sscanf (d2, "%d", &dt->day))
			return (SD_FAILURE);
	}
	else if (isalpha (*d2))
	{
		if (!sscanf (d1, "%d", &dt->day))
			return (SD_FAILURE);
		if (strToMonth (d2, &dt->month))
			return (SD_FAILURE);
	}
	else 	/* Not an ALPHA month form */
	{
		/* Numeric month is d1 */
		if (!sscanf (d1, "%d", &dt->month))
			return (SD_FAILURE);
		--dt->month;			/* we use 0-11 for month */

		/* Numeric day is d2 */
		if (!sscanf (d2, "%d", &dt->day))
			return (SD_FAILURE);
	}

	/* Numeric year is always d3 */
	if (!sscanf (d3, "%d", &dt->year))
		return (SD_FAILURE);
	if (strlen (d3) == 2)
	{
		/* any year < 1984 is assumed to be in the 21st century	*/
		century = (dt->year < 84) ? 2000 : 1900;
		dt->year += century;
	}
	return (SD_SUCCESS);
}


/* end of obsoleted code */
#endif


#if 0
/* obsoleted code */


/************************************************************************/
/*			parseTimeString					*/
/************************************************************************/

/* parseTimeString IS NO LONGER REFERENCED IN THIS CODE */

static ST_RET parseTimeString (ST_CHAR *s, _TS_DATETIME *dt)
{
	int pm;
	ST_CHAR *p, *t1, *t2, *t3;

	dt->timeFound = SD_TRUE;

	/* break into two substrings */
	p =  strpbrk (s,":");
	t1 = s;  		/* hours are here */
	*p = 0;
	t2 = ++p;		/* minutes are here */
	if (!p)
		return (SD_FAILURE);

	p = strpbrk (t2, ":");
	if (!p)
		t3 = NULL;
	else
	{
		*p = 0;
		t3 = ++p;
	}

	pm = SD_FALSE;    
	if (t3)
	{
		if (t3[2] == 'A' || t3[2] == 'a')
		{
			t3[2] = 0;
		}
		else if (t3[2] == 'P' || t3[2] == 'p')
		{
			pm = SD_TRUE;    
			t3[2] = 0;
		}
	}

	if (!sscanf (t1, "%d", &dt->hour))
		return (SD_FAILURE);

	if (!sscanf (t2, "%d", &dt->min))
		return (SD_FAILURE);

	if (t3)
		sscanf (t3, "%d", &dt->sec);
	else
		dt->sec = 0;

	if (pm)
		dt->hour += 12;

	return (SD_SUCCESS);
}


/* end of obsoleted code */
#endif


/************************************************************************/

#if 0
/* obsoleted code */

static ST_CHAR *monthStrings[12] =
{
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};


static ST_RET strToMonth (ST_CHAR *s, int *monthOut)
{
	int i;

	for (i = 0; i < 12; ++i)
	{
		if (!stricmp (s, monthStrings[i]))
		{
			*monthOut = i;
			return (SD_SUCCESS);
		}
	}
	return (SD_FAILURE);
}

#endif


/************************************************************************/
/* Btime6 Conversion Routines:						*/
/************************************************************************/

#define SEC_PER_MIN	60
#define SEC_PER_HOUR	(60 * SEC_PER_MIN)
#define SEC_PER_DAY	(24 * SEC_PER_HOUR)
#define MSEC_PER_SEC	1000


/************************************************************************/
/* Btime6StringToVals:  Receive string in BTIME_DEF_TIME_FORMAT+mSec	*/
/*			and return the number of days and msecs 	*/
/*			since Jan 1, 1984 midnight			*/
/*                                                                      */
/* format BTIME_DEF_TIME_FORMAT.mSec is 'mm-dd-yyyy hh:mm:ss.mmm'       */
/* NOTE: output values are NOT adjusted for timezone or DST             */
/*                                                                      */
/* method:  parse string, capturing mSec.  create time_t, and adjust    */
/* for difference in seconds between 1970-01-01 and 1984-01-01.         */
/************************************************************************/

ST_RET Btime6StringToVals (ST_CHAR *src, ST_INT32 *numDays, ST_INT32 *numMSec)
{
	_TS_DATETIME              dt = {0};
	struct tm                 w_struct_tm = {0};

	time_t                    w_time_t;
	time_t                    w_days;
	time_t                    w_msec;

	if ((src == NULL) || (numDays == NULL) || (numMSec == NULL))
	{
		return SD_FAILURE;      /* bad parameters */
	}

	/* for Btime strings, month-day-year date ordering is required */

	dt.order = S_DATE_ORDER_MDY;

	if (getTsDateTime (src, &dt) != SD_SUCCESS)
	{
		SLOGALWAYS ("Btime6StringToVals: Unable to parse Btime6 '%s'", src);
		return SD_FAILURE;      /* bad parameters */
	}

	/*  convert the components to struct tm */

	w_struct_tm.tm_year  = dt.year - 1900;
	w_struct_tm.tm_mon   = dt.month - 1;
	w_struct_tm.tm_mday  = dt.day;
	w_struct_tm.tm_hour  = dt.hour;
	w_struct_tm.tm_min   = dt.min;
	w_struct_tm.tm_sec   = dt.sec;
	w_struct_tm.tm_isdst = 0;  	        /* BTime is GMT based */

	w_time_t = usr_mkgmtime (&w_struct_tm);    /* portable GMT-based mktime() */

	if (w_time_t == (time_t) -1) 
	{
		SLOGALWAYS ("Btime6StringToVals: Unable to convert '%s' to time_t", src);
		return SD_FAILURE;
	}

	/* adjust for diff between 1984 (Btime epoch) and 1970 (Unix time epoch) */
	/* this is a known constant amount, so we don't recalculate it each time */

	w_time_t -= (time_t) S_SECS_DIFF_1984_1970;

	w_days = w_time_t / SEC_PER_DAY;
	w_msec = (w_time_t % SEC_PER_DAY) * 1000; /* EJV CORR: the multiplication by 1000 was missing */

	/* save the calculated time value */
	*numDays = (ST_UINT32) w_days;
	*numMSec = (ST_UINT32) w_msec + (ST_UINT32) dt.mSec;

	return SD_SUCCESS;

} /* Btime6StringToVals */


/************************************************************************/
/* Btime6ValsToString:  Receive the number of days and msecs since	*/
/*			Jan 1, 1984 - midnight and return a string in	*/
/*			BTIME_DEF_TIME_FORMAT+mSec			*/
/************************************************************************/

ST_RET Btime6ValsToString (char *dest, ST_INT32 numDays, ST_INT32 numMSec)
{
	struct tm tmVal;
	time_t tmRslt;
	long numSeconds, balMSec;
	char stash[MAX_TIME_STRING_LEN+1];
	ldiv_t divResult;
	_TS_DATETIME dt;

	divResult = ldiv (numMSec, MSEC_PER_SEC);
	numSeconds = divResult.quot;
	balMSec = divResult.rem;

	divResult = ldiv (numSeconds, SEC_PER_DAY);
	dt.day = divResult.quot + numDays;
	numSeconds = divResult.rem;

	divResult = ldiv (numSeconds, SEC_PER_HOUR);
	dt.hour = divResult.quot;
	numSeconds = divResult.rem;

	divResult = ldiv (numSeconds, SEC_PER_MIN);
	dt.min = divResult.quot;
	numSeconds = divResult.rem;

	dt.sec = numSeconds;

	/* set up the struct tm	*/
	tmVal.tm_wday = 0;		/* this is an output parameter */
	tmVal.tm_yday = 0;		/* this is an output parameter */

	tmVal.tm_year = 84;
	tmVal.tm_mon = 0;
	tmVal.tm_mday = dt.day+1;
	tmVal.tm_hour = dt.hour;
	tmVal.tm_min = dt.min;
	tmVal.tm_sec = dt.sec;
	tmVal.tm_isdst = -1;  	/* let function guess */
	tmRslt = mktime(&tmVal);

	if (tmRslt == (time_t) -1) 
		return (SD_FAILURE);

	/* now turn it into a string	with out mSeconds 			*/
	strftime (stash, MAX_TIME_STRING_LEN, BtimeTimeFormat, localtime (&tmRslt));

	/* now append the mSeconds to the string				*/
	sprintf (dest, "%s.%03ld", stash, balMSec);

	return (SD_SUCCESS);

} /* Btime6ValsToString */


/************************************************************************/
/* Btime4StringToVals:  Receive string in BTIME_DEF_TIME_FORMAT.mSec	*/
/*			and return the number of msecs since midnight	*/
/* format BTIME_DEF_TIME_FORMAT.mSec is 'mm-dd-yyyy hh:mm:ss.mmm'       */
/************************************************************************/

ST_RET Btime4StringToVals (char *src, ST_INT32 *numMSec)
{
	_TS_DATETIME              dt = {0};
	struct tm  *              p_struct_tm = {0};

	time_t                    w_time_t;
	ST_INT32                  curDay;
	ST_INT32                  curMonth;
	ST_INT32                  curYear;
	ST_INT32                  totalMsec;


	if ((src == NULL) || (numMSec == NULL))
	{
		return SD_FAILURE;      /* bad parameters */
	}

	/* get current time for validation of date */

	w_time_t = time (NULL);

	if (w_time_t == (time_t) -1) 
	{
		SLOGALWAYS ("Btime4StringToVals: Unable to obtain current time");
		return SD_FAILURE;      /* normally should not occur */
	}

	/* current time is converted to a GMT-based struct tm */
	/* because the Btime6 version of this function calculates the offset */
	/* from 1984-01-01 GMT, so we use a GMT-based struct tm here to be */
	/* consistent. */

	p_struct_tm = gmtime (&w_time_t);

	if (p_struct_tm == NULL) 
	{
		SLOGALWAYS (
			"Btime4StringToVals: Unable to convert current time_t to struct tm");
		return SD_FAILURE;      /* normally should not occur */
	}

	curYear  = p_struct_tm->tm_year + 1900;
	curMonth = p_struct_tm->tm_mon  + 1;
	curDay   = p_struct_tm->tm_mday;

	/* for Btime strings, month-day-year date ordering is required */

	dt.order = S_DATE_ORDER_MDY;

	if (getTsDateTime (src, &dt) != SD_SUCCESS)
	{
		SLOGALWAYS ("Btime4StringToVals: Unable to parse Btime4 '%s'", src);
		return SD_FAILURE;      /* bad parameters */
	}

	if ( (dt.year  != curYear )
		||   (dt.month != curMonth)
		||   (dt.day   != curDay  ) )
	{
		SLOGALWAYS ("Btime4 Conversion Error: Input date must be today's date");
		return SD_FAILURE;
	}

	/* get mSecs from hours/min/secs */

	totalMsec = (dt.hour * SEC_PER_HOUR * MSEC_PER_SEC)
		+ (dt.min  * SEC_PER_MIN  * MSEC_PER_SEC)
		+ (dt.sec                 * MSEC_PER_SEC)
		+ (dt.mSec);

	/* ensure mSecs does not exceed num mSecs in a day.  the count must not */
	/* equal ms/day either, otherwise it would be an offset to the next day */

	if ( (totalMsec < 0)
		||   (totalMsec >= (SEC_PER_DAY * MSEC_PER_SEC)) )
	{
		SLOGALWAYS ("Btime4 Conversion Error: Invalid millisecond count: %d", totalMsec);
		return SD_FAILURE;
	}

	*numMSec = totalMsec;

	return SD_SUCCESS;

} /* Btime4StringToVals */


/************************************************************************/
/* Btime4ValsToString: Receive number of msecs since midnight and return*/
/*		       string in BTIM_DEF_TIME_FORMAT.mSec		*/
/************************************************************************/

ST_RET Btime4ValsToString (char *dest, ST_INT32 numMSec)
{
	int curDay, curMonth, curYear;
	struct tm tmVal, *curTime;
	time_t tmRslt;
	long numSeconds, balMSec;
	char stash[MAX_TIME_STRING_LEN+1];
	ldiv_t divResult;
	_TS_DATETIME dt;
	time_t theTime;

	/* figure out todays date */
	theTime = time (NULL);
	curTime = localtime (&theTime);
	curDay = curTime->tm_mday;
	curMonth = curTime->tm_mon;
	curYear = curTime->tm_year;

	divResult = ldiv (numMSec, MSEC_PER_SEC);
	numSeconds = divResult.quot;
	balMSec = divResult.rem;

	divResult = ldiv (numSeconds, SEC_PER_HOUR);
	dt.hour = divResult.quot;
	numSeconds = divResult.rem;

	divResult = ldiv (numSeconds, SEC_PER_MIN);
	dt.min = divResult.quot;
	numSeconds = divResult.rem;

	dt.sec = numSeconds;

	/* set up the struct tm	*/
	tmVal.tm_wday = 0;		/* this is an output parameter */
	tmVal.tm_yday = 0;		/* this is an output parameter */

	tmVal.tm_year = curYear;
	tmVal.tm_mon = curMonth;
	tmVal.tm_mday = curDay;
	tmVal.tm_hour = dt.hour;
	tmVal.tm_min = dt.min;
	tmVal.tm_sec = dt.sec;
	tmVal.tm_isdst = -1;  	/* let function guess */
	tmRslt = mktime(&tmVal);

	if (tmRslt == (time_t) -1) 
		return (SD_FAILURE);

	/* now turn it into a string	with out mSeconds 			*/
	strftime (stash, MAX_TIME_STRING_LEN, BtimeTimeFormat, localtime (&tmRslt));

	/* now append the mSeconds to the string				*/
	sprintf (dest, "%s.%03ld", stash, balMSec);

	return (SD_SUCCESS);
}


#if 0
/* obsoleted code */

/************************************************************************/
/* parseBtimeString:  parse HH:MM:SS:msec				*/
/************************************************************************/

/* parseBtimeString IS NO LONGER REFERENCED IN THIS CODE */

static ST_RET parseBtimeString (char *s, _TS_DATETIME *dt)
{
	char stash[MAX_TIME_STRING_LEN+1];
	char toFind[] = ":.";
	char *token;
	int count = 0;

	dt->timeFound = SD_TRUE;

	/* make a copy first */
	strcpy (stash, s);

	/* set default values */
	dt->hour = dt->min = dt->sec = dt->mSec = 0;

	/* do the token thing to separate the string */
	token = strtok (stash, toFind);
	while (token != NULL)
	{
		switch (count)
		{
		case 0:
			dt->hour = atoi (token);
			break;
		case 1:
			dt->min = atoi (token);
			break;
		case 2:
			dt->sec = atoi (token);
			break;
		case 3:
			dt->mSec = atoi (token);
			break;
		}
		count++;
		/* get next token */
		token = strtok (NULL, toFind);
	}

	return (SD_SUCCESS);
}


/* end of obsoleted code */
#endif


/************************************************************************/
/*                          validQualField                              */
/* verify that a string contains a valid Quality field (qual=b,b,b,n)   */
/* there must be at least 3 bit flags present, and proper comma delims. */
/* if valid, return number characterizing the field, else return 0.     */
/* then, create a buffer with the extracted flags and 'n' value,        */
/* with default values if trailing flags or ''n' value is omitted.      */
/************************************************************************/

static ST_INT32             validQualField (
	ST_CHAR *                 qual,
	ST_CHAR *                 flagBuf)
{
	ST_CHAR                   work[32];
	ST_INT32                  i;
	ST_INT32                  digit = 0;

	if ((qual == NULL)  | (flagBuf == NULL))
	{
		return 0;
	}

	/* (qual=1,2,3,n) */
	/* 0123456789012345 */
	/*           111111 */

	strcpy (flagBuf, "00000");    /* create defaults */

	/* make copy of string, modifying it to create a pattern to verify */

	for (i=0; i < 24; i++)
	{
		if (qual[i] <= ' ')
		{
			break;
		}

		else if (qual[i] == ')')
		{
			work[i++] = ')';
			break;
		}

		else if (isdigit (qual[i]))
		{
			/* first 3 digits must be 0 or 1, rest is value 0 to 31 */
			digit++;

			if (digit <= 3)
			{
				if (qual[i] > '1')
				{
					return 0;    /* char is digit but > '1', not a valid bit value */
				}
				work[i] = '1';    /* '1' stands for '0' or '1' */
			}

			else    /* digits after 3rd */
			{
				work[i] = '9';    /* to check pattern */
			}

		} /* digit */

		else
		{
			work[i] = (ST_CHAR) toupper (qual[i]);
		}

	} /* for */

	work[i] = 0;


	if      (strcmp (work, "(QUAL=1,1,1)") == 0)
	{
		flagBuf[0] = qual[6];
		flagBuf[1] = qual[8];
		flagBuf[2] = qual[10];
		return 3;
	}                     
	else if (strcmp (work, "(QUAL=1,1,1,9)") == 0)
	{
		flagBuf[0] = qual[6];
		flagBuf[1] = qual[8];
		flagBuf[2] = qual[10];
		flagBuf[3] = qual[12];    /* 1-digit 'n' value */
		flagBuf[4] = 0;           /* shorten return value */

		/* when 'n' value is 1-digit, any digit value is OK */
		return 4;
	}
	else if (strcmp (work, "(QUAL=1,1,1,99)") == 0)
	{
		ST_INT32                num;

		flagBuf[0] = qual[6];
		flagBuf[1] = qual[8];
		flagBuf[2] = qual[10];
		flagBuf[3] = qual[12];    /* 2-digit 'n' value */
		flagBuf[4] = qual[13];

		/* when 'n' value is 1-digit, check for range 0 to 31 */
		/* the 'n' value is supposed to be a 5-bit value */

		num = atoi (flagBuf + 3);

		if ((num < 0) || (num > 31))
		{
			return 0;    /* 'n' value is out of range */
		}

		return 6;
	}


	/* quality field was none of the above formats */

	return 0;

} /* validQualField */


/************************************************************************/
/* UTC Time Conversion Functions:                                       */
/*        Format = YYYY-MM-DDThh:mm:ss.000000000Z(qual=b,b,b,b,n)       */
/************************************************************************/
/* UtcStringToVals:                                                     */
/************************************************************************/

ST_RET UtcStringToVals (
						ST_CHAR *                 src, 
						ST_UINT32 *               pSecs, 
						ST_UINT32 *               pFraction, 
						ST_UINT32 *               pQflags)
{
	_TS_DATETIME              dt = {0};

	struct tm                 w_struct_tm = {0};
	time_t                    w_time_t;
	ST_CHAR *                 pflags;
	ST_CHAR                   flagBuf[8];

	ST_INT                    b0;
	ST_INT                    b1;
	ST_INT                    b2;
	ST_INT                    bx;

	ST_DOUBLE                 decSecs;
	ST_DOUBLE                 binSecs;


	if ( (src       == NULL) 
		||   (pSecs     == NULL)
		||   (pFraction == NULL)
		||   (pQflags   == NULL) )
	{
		SLOGALWAYS ("UtcStringToVals: NULL parameters");
		return SD_FAILURE;      /* bad parameters */
	}

	/* for UTC time strings, date ordering is yyyy-mm-dd*/
	dt.order = S_DATE_ORDER_YMD;
	dt.pflags = NULL;

	if (getTsDateTime (src, &dt) != SD_SUCCESS)
	{
		SLOGALWAYS ("UtcStringToVals: Unable to parse time '%s'", src);
		return SD_FAILURE;      /* bad parameters */
	}

	if (dt.zoneCode != 'Z')
	{
		SLOGALWAYS ("UtcStringToVals: GMT timezone code Z missing in '%s'", src);
		return SD_FAILURE;
	}

	pflags = dt.pflags;    /* pointer to (qual string */

	if (pflags == NULL)
	{
		/* quality fields were not present, assume they are all 0 */
		/* SLOGALWAYS ("UtcStringToVals: Missing quality in '%s'", src); */

		*pQflags = 0;
	}
	else if (validQualField (pflags, flagBuf) < 3)    /* not enough flags */
	{
		SLOGALWAYS ("UtcStringToVals: Invalid quality in '%s'", src);
		return SD_FAILURE;
	}
	else
	{
		/* flags are already validated as '0' or '1' char values, so AND to get */
		/* bit values, and shift into correct positions */

		b0 = (flagBuf[0] & 1) << 7;
		b1 = (flagBuf[1] & 1) << 6;
		b2 = (flagBuf[2] & 1) << 5;
		bx = atoi (flagBuf + 3);

		*pQflags = (b0 | b1 | b2 | bx);
	}

	/*  convert the components to struct tm */
	w_struct_tm.tm_year  = dt.year - 1900;
	w_struct_tm.tm_mon   = dt.month - 1;
	w_struct_tm.tm_mday  = dt.day;
	w_struct_tm.tm_hour  = dt.hour;
	w_struct_tm.tm_min   = dt.min;
	w_struct_tm.tm_sec   = dt.sec;
	w_struct_tm.tm_isdst = 0;

	w_time_t = usr_mkgmtime (&w_struct_tm);    /* portable GMT-based mktime() */

	if (w_time_t == (time_t) -1) 
	{
		SLOGALWAYS ("UtcStringToVals: Unable to convert '%s' to time_t", src);
		return SD_FAILURE;
	}

	/* save the calculated time value */
	*pSecs = (ST_UINT32) w_time_t;

	/* set the decimal fraction of seconds */
	/* dt.nSec contains a count of nanoseconds */
	/* this must be converted to a 24-bit binary fraction of fractional time */

	/* note: 0x1000000 == 16777216 */

	/* form decimal fraction of 1 second from 0.0 to 0.999,999,999 */

	/*                                 123456789                   */
	decSecs = ((ST_DOUBLE) dt.nSec) / 1000000000.0;

	/* convert to binary fraction of 1 second from 0x0.0 to 0x0.FFFFFF */
	binSecs = (decSecs * 16777216.0) + 0.5;

	/* return int equivalent count to caller */
	*pFraction = (ST_UINT32) binSecs;

	return SD_SUCCESS;

} /* UtcStringToVals */


/************************************************************************/
/* UtcValsToString:                                                     */
/************************************************************************/

ST_RET UtcValsToString (char *dest, ST_UINT32 secs, ST_UINT32 fraction, 
						ST_UINT32 qflags)
{
	ST_CHAR theDate[MAX_TIME_STRING_LEN];
	ST_CHAR theFraction[25];
	ST_CHAR theQual[25];
	ST_DOUBLE dFraction;
	ST_CHAR *pFract;

	ST_CHAR b0, b1, b2;
	ST_INT rest;

	time_t t = secs;
	struct tm *pTm;

	/* get the date portion */

	pTm = gmtime (&t);

	if (!pTm)
	{
		SLOGALWAYS ("UtcValsToString:  conversion failure - invalid seconds.");
		return SD_FAILURE;
	}

	strftime (theDate, MAX_TIME_STRING_LEN, UTC_DEF_TIME_FORMAT, pTm);

	/* get the fraction portion */

	dFraction = ((ST_DOUBLE) fraction / (ST_DOUBLE) 0x01000000);
	sprintf (theFraction, " %#0.09f", dFraction);
	pFract = strchr (theFraction, '.');

	if (!pFract)
	{
		SLOGALWAYS ("UtcToString - unable to convert fraction %d", fraction);
		return SD_FAILURE;
	}

	/* get the qflags */

	b0 = b1 = b2 = rest ='0';

	if (qflags & 0x80) b0 = '1';
	if (qflags & 0x40) b1 = '1';
	if (qflags & 0x20) b2 = '1';
	rest = (qflags & 0x1F);
	sprintf (theQual, "Z(qual=%c,%c,%c,%d)", b0, b1, b2, rest);

	/* put them together */
	sprintf (dest, "%s%s%s", theDate, pFract, theQual);

	return SD_SUCCESS;
}


/************************************************************************/
/* XmlStringToUtcValue                                                  */
/************************************************************************/
/*   An input time and date string is converted to the number of        */
/*   seconds since 1/1/1970.                                            */
/*                                                                      */ 
/*   Any of the following strings are valid input to this subroutine:	*/
/*									*/
/*     "yyyy-mm-ddThh:mm:ss.fffff+/-hh:mm"   				*/
/*     "yyyy-mm-ddThh:mm:ss+/-hh:mm"	      				*/
/*									*/
/*     "yyyy-mm-ddThh:mm:ss.fffffZ"	 				*/
/*     "yyyy-mm-ddThh:mm:ssZ"		 				*/
/*									*/
/*     "yyyy-mm-ddThh:mm:ss.fffff"	 				*/
/*     "yyyy-mm-ddThh:mm:ss"		  				*/
/*									*/
/*     Note:	                                                        */
/*       Decimal fraction for microseconds:                     .fffff  */
/*       East time zone offset from GMT (Greenwich Mean Time):  +hh:mm  */
/*       West time zone offset from GMT (Greenwich Mean Time):  -hh:mm  */
/*									*/
/*                                                                      */
/* Output is stored in SX_DATE_TIME structure as:                       */
/*                                                                      */
/*    dateTime	        stored as number of seconds elapsed since       */
/*    		        midnight (00:00:00) January 1, 1970,            */
/*		        UTC (Coordinated Universal Time), according     */
/*		        to the system clock                             */
/*    useMicroseconds   indicates decimal fraction for seconds was      */
/*                      specified                                       */
/*    microseconds      specified decimal fraction of seconds stored as */
/*                      microseconds                                    */
/*    useTZ             indicates a time zone offset is present         */
/*    tz                time zone offset specified as minutes           */
/*									*/
/* Time zone offset "tz" and time zone presence "useTZ" will be         */
/* specified in output as follows:                                      */
/*     									*/
/*  "yyyy-mm-ddThh:mm:ss.fffff+/-hh:mm" 'useTZ = SD_TRUE'               */
/*                                      'tz = +/-seconds'               */	        
/*  "yyyy-mm-ddThh:mm:ss+/-hh:mm"       'useTZ = SD_TRUE'               */ 
/*                                      'tz = +/-seconds'	        */
/*									*/
/*  "yyyy-mm-ddThh:mm:ss.fffffZ"	'useTZ = SD_TRUE'  'tz = 0' 	*/
/*  "yyyy-mm-ddThh:mm:ssZ"	        'useTZ = SD_TRUE'  'tz = 0' 	*/
/*								 	*/
/*  "yyyy-mm-ddThh:mm:ss.fffff"	        'useTZ = SD_FALSE' 'tz ignored' */
/*  "yyyy-mm-ddThh:mm:ss"	        'useTZ = SD_FALSE' 'tz ignored' */
/*                                  					*/
/************************************************************************/

ST_RET XmlStringToUtcValue (ST_CHAR *in_buf, SX_DATE_TIME *sxDateTime)
{
	_TS_DATETIME              dt = {0};

	struct tm                 w_struct_tm = {0};
	time_t                    w_time_t;    /* local time in seconds */


	if ((in_buf == NULL) || (sxDateTime == NULL))
	{
		SLOGALWAYS ("XmlStringToUtcValue: NULL parameters");
		return SD_FAILURE;      /* bad parameters */
	}

	/* split up the string input from XML file so we can store */
	/* number of seconds, number of microseconds (decimal      */
	/* fraction of seconds) and number of minutes in time zone */
	/* offset separately                                       */

	/* for XML time strings, date ordering is yyyy-mm-dd*/

	dt.order = S_DATE_ORDER_YMD;

	if (getTsDateTime (in_buf, &dt) != SD_SUCCESS)
	{
		return SD_FAILURE;      /* bad parameters */
	}

	/*  convert the components to struct tm */

	w_struct_tm.tm_year  = dt.year - 1900;
	w_struct_tm.tm_mon   = dt.month - 1;
	w_struct_tm.tm_mday  = dt.day;
	w_struct_tm.tm_hour  = dt.hour;
	w_struct_tm.tm_min   = dt.min;
	w_struct_tm.tm_sec   = dt.sec;

	/* set the decimal fraction of seconds if present */

	if (dt.uSec == 0)
	{
		sxDateTime->microseconds = 0;
		sxDateTime->useMicroseconds = SD_FALSE;
	}

	else
	{
		sxDateTime->microseconds = dt.uSec;
		sxDateTime->useMicroseconds = SD_TRUE;
	}

	/* get the time zone offset in minutes if present */

	if (dt.zoneCode)    /* '+', '-' or 'Z' present */
	{
		sxDateTime->useTZ = SD_TRUE;
		sxDateTime->tz = (dt.zoneHour * 60) + dt.zoneMin;

		if (dt.zoneCode == '-')
		{
			sxDateTime->tz = -sxDateTime->tz;
		}
	}

	else
	{
		sxDateTime->useTZ = SD_FALSE;
		sxDateTime->tz = 0;
	}

	/* calculate total seconds in UTC time so we can store it */
	/* as a number of seconds since 01/01/1970 */

	if (sxDateTime->useTZ && sxDateTime->tz == 0)
	{
		w_struct_tm.tm_isdst = 0;
		w_time_t = usr_mkgmtime (&w_struct_tm);    /* portable GMT-based mktime() */
	}

	else
	{
		w_struct_tm.tm_isdst = -1; 
		w_time_t = mktime (&w_struct_tm);
	}

	if (w_time_t == (time_t) -1) 
	{
		SLOGALWAYS ("XmlStringToUtcValue: Unable to convert time '%s'", in_buf);
		return SD_FAILURE;
	}

	/* save the calculated time value */
	sxDateTime->dateTime = w_time_t;

	return SD_SUCCESS;

} /* XmlStringToUtcValue */


/************************************************************************/
/* UtcValueToXmlString                                                  */
/************************************************************************/  
/*  The specified number of seconds since 1/1/1970 is converted to a    */
/*  time and date string.                                               */
/*                                                                      */
/*  Input is stored in "SX_DATE_TIME" as:                               */
/*                                                                      */
/*    dateTime	        stored as number of seconds elapsed since       */
/*    		        midnight (00:00:00) January 1, 1970,            */
/*		        UTC (Coordinated Universal Time), according     */
/*		        to the system clock                             */
/*    useMicroseconds   indicates decimal fraction of seconds was       */
/*                      specified                                       */
/*    microseconds      decimal fraction of seconds specified stored as */
/*                      microseconds                                    */
/*    useTZ             indicates a time zone offset is present         */
/*    tz                time zone offset from GMT (Greenwich Mean Time) */
/*                      specified as minutes                            */
/*									*/
/*  Output string format will depend upon values present in the         */
/*  "SX_DATE_TIME" structure as follows:                                */
/*                                                                      */
/*  'useTZ = SD_TRUE' 'tz = nn..n'  "yyyy-mm-ddThh:mm:ss.fffff+/-hh:mm" */
/*  'useTZ = SD_TRUE' 'tz = nn..n'  "yyyy-mm-ddThh:mm:ss+/-hh:mm"       */
/*									*/
/*  'useTZ = SD_TRUE'  'tz = 0'     "yyyy-mm-ddThh:mm:ss.fffffZ"	*/
/*  'useTZ = SD_TRUE'  'tz = 0'     "yyyy-mm-ddThh:mm:ssZ"		*/
/*									*/
/*  'useTZ = SD_FALSE' 'tz ignored' "yyyy-mm-ddThh:mm:ss.fffff"		*/
/*  'useTZ = SD_FALSE' 'tz ignored' "yyyy-mm-ddThh:mm:ss"		*/
/*									*/
/************************************************************************/

ST_RET UtcValueToXmlString (ST_CHAR *dest, ST_UINT destLen,
							SX_DATE_TIME *sxDateTime)
{
	ST_CHAR theFraction[64], theTZ[64];
	ST_CHAR tzSign = '+';
	ST_INT  tzValue;

#define MAX_DATE_TIME_STR_LEN	32	/* longest output format:		*/
	/*  "yyyy-mm-ddThh:mm:ss.fffff+/-hh:mm"	*/

	if (destLen < MAX_DATE_TIME_STR_LEN)
	{
		SLOGALWAYS ("ERROR: Buffer %d bytes may be too small for XML string (min=%d)",
			destLen, MAX_DATE_TIME_STR_LEN);
		return (SD_FAILURE);
	}

	/* get the date and time from the "dateTime" or 
	number of seconds from 1/1/1970 (UTC time) 
	stored in "sxDateTime" and place it in a
	structure of type "tm" returned from the call
	to "gmtime" as:
	sec
	min
	hour
	day of month
	month 
	year
	day of week
	day of year                                     */

	/* from the data stored in the structure utilize 
	"strftime" to format a date and time string using 
	the following output format:
	%Y is year with century as a decimal number
	%m is month as a decimal number (01-12)
	%d is day of month as a decimal number (01-31)
	%H is hour in 24 hour format (00-23)
	%M is minute as decimal number (00-59)
	%S is second as decimal number (00-59)	        */

	if (sxDateTime->useTZ && sxDateTime->tz == 0)
		strftime (dest, destLen, "%Y-%m-%dT%H:%M:%S", gmtime (&sxDateTime->dateTime));
	else
		strftime (dest, destLen, "%Y-%m-%dT%H:%M:%S", localtime (&sxDateTime->dateTime));

	/* then get number of microseconds stored as an       */
	/* "long" value in the "sxDateTime" structure and     */
	/* format it as a string with a leading decimal point */

	if (sxDateTime->useMicroseconds)
	{
		/* this code was merged from time_str2.c */
		/* number of microsecond digits changed from 5 to to 6 */

		sprintf (theFraction, ".%06ld", sxDateTime->microseconds);
		strcat (dest, theFraction);
	}

	/* now get the number of minutes stored as an integer */
	/* for time zone offset and place it in a string      */
	/* in the format of "+hh:mm" or "-hh:mm"              */

	if (sxDateTime->useTZ)
	{
		tzValue = sxDateTime->tz;
		/* there are 1440 minutes in a day */
		/* if tzValue is outsize this value +/- it is invalid */
		/* if so, we don't know the correct zone but will default to zero */

		/* we are allowing +/- 23:59 just to be tolerant, but usually only */
		/* only +/- 12 hours is actually used. */

		if ((tzValue >= 1440) || (tzValue <= -1440))
		{
			SLOGALWAYS ("ERROR: UtcValueToXmlString sxDateTime.tz value %d out of range",
				sxDateTime->tz);
			strcat (dest, "Z");    /* assume GMT in case our retcode is ignored */
			return SD_FAILURE;
		}

		if (tzValue == 0)
			/* output format:  "yyyy-mm-ddThh:mm:ss.fffffZ"           	*/
			strcat (dest, "Z");

		else
		{
			/* output format:  "yyyy-mm-ddThh:mm:ss.fffff+/-hh:mm"           */

			if (tzValue < 0)
			{
				tzSign = '-';
				tzValue = -tzValue;
			}

			sprintf (theTZ, "%c%02d:%02d", tzSign, tzValue / 60, tzValue % 60);
			strcat (dest, theTZ);
		}
	}

	return SD_SUCCESS;
}				


/************************************************************************/
/* CalculateTimeZoneOffset                                              */
/************************************************************************/
/*  Figures out the difference between UTC/GMT/Zulu time and            */
/*  local time.                                                         */
/*  This difference can be used after the "mktime"                      */
/*  function is called. The "mktime" function returns local time        */
/*  and to convert this time to a UTC time this calculated              */
/*  adjustment must be added to the local time value returned           */
/*  from "mktime".                                                      */
/************************************************************************/

/* NOTE: this function may no longer be necessary */

ST_DOUBLE CalculateTimeZoneOffset (ST_VOID)
{
	time_t     currTime, local_t, utc_t;
	struct tm *pJunkTm, localTm, utcTm;
	ST_DOUBLE  timeZoneAdjustment;

	currTime = time (NULL);		          /* get the current system time */
	pJunkTm = gmtime (&currTime);		          /* convert current time value in seconds to a structure */
	memcpy (&utcTm, pJunkTm, sizeof (utcTm));	  /* save current UTC time */
	pJunkTm = localtime (&currTime);		  /* convert current time value and correct for local time zone */
	memcpy (&localTm, pJunkTm, sizeof (localTm));	  /* save local UTC time */
	utc_t = mktime (&utcTm);                        /* convert UTC time to UTC seconds */
	local_t = mktime (&localTm);			  /* convert local time to UTC seconds */
	timeZoneAdjustment = difftime (local_t, utc_t); /* find the difference or time zone offset */

	return (timeZoneAdjustment);
}

/*   this code was merged from time_str2.c                              */

/************************************************************************/
/*   GetTimeAndUsec                                                     */
/************************************************************************/  
/* a function to supply current time as a time_t, and the number of     */
/* microseconds, in a single call.  this will simplify the setting of   */
/* data structures such as DateTime which have both of these values.    */
/************************************************************************/  

#if defined(_WIN32) || defined(linux) || defined(__QNX__)
time_t GetTimeAndUsec (long *usec)
{
#ifdef _WIN32
	struct _timeb tb;
	_ftime (&tb);  
#else
	struct timeb  tb;
	ftime (&tb);
#endif

	if (usec != NULL)
	{
		*usec = ((long)tb.millitm) * 1000;
	}
	return tb.time;
} 
#endif	/* some platforms	*/


/*****************************************************************************/
/*                          usr_mkgmtime_isleap                              */
/*  return 1 if 'year' is a leap year, else return 0                         */
/*****************************************************************************/

static time_t               usr_mkgmtime_isleap (time_t year)
{
	if ((year < 0 ) || (year > 32767))
	{
		return 0;               /* assume invalid years are not leap years */
	}

	if ((year % 4000) == 0)
	{
		return 0;               /* multiples of 4000 are not leap years */
	}

	if ((year % 400) == 0)
	{
		return 1;               /* multiples of 400 are leap years */
	}

	if ((year % 100) == 0)
	{
		return 0;               /* multiples of 100 are not leap years */
	}

	if ((year % 4) == 0)
	{
		return 1;               /* multiples of 4 are leap years */
	}

	return 0;                 /* all others are not leap years */

} /* usr_mkgmtime_isleap */


/*****************************************************************************/
/*                          usr_mkgmtime_leap_year_days                      */
/*  return number leap-year days based on 'year'                             */
/*****************************************************************************/

static time_t               usr_mkgmtime_leap_year_days (time_t year)
{
	/* number of 4000-year multiples */
	time_t n4000 = (year / (time_t) 4000);

	/* number of 400-year multiples in excess of 4000 */
	time_t n400  = (year % (time_t) 4000) / (time_t) 400;

	/* number of 100-year multiples in excess of 400 */
	time_t n100  = (year % (time_t) 400) / (time_t) 100;

	/* number of 4-year multiples in excess of 100 */
	time_t n4    = (year % (time_t) 100) / (time_t) 4;

	return
		( (time_t) 969 * n4000 )
		+ ( (time_t)  97 * n400  )
		+ ( (time_t)  24 * n100  )
		+ ( (time_t)   1 * n4    );

} /* usr_mkgmtime_leap_year_days */


/*****************************************************************************/
/*                          usr_mkgmtime                                     */
/*  a portable implementation of mkgmtime(), a GMT-based mktime() function.  */
/*                                                                           */
/*  note: unlike typical system implementations of mktime(), this function   */
/*  does NOT normalize the struct tm fields, which are required to be valid. */
/*                                                                           */
/*  for consistency, tm_isdst should be set to 0 upon entry (since GMT does  */
/*  not use daylight savings time), but the function ignores this field.     */
/*  (when tm_isdst is set to 0, the system mktime() api could be used to     */
/*  normalize the input fields prior to calling usr_mkgmtime, if necessary.) */
/*                                                                           */
/*  the output fields tm_wday and tm_yday are correctly set upon exit when   */
/*  the result time_t value is normal.                                       */
/*                                                                           */
/*  upon error, a value of (time_t) (-1) is returned, and the output fields  */ 
/*  tm_wday and tm_yday are not set in that case.  detected errors are: NULL */
/*  pointer to the struct tm, date/time values out of valid range, and any   */
/*  input that results in a negative time_t value.  note that the range of   */
/*  input values that could produce a negative time_t are dependent on the   */
/*  datatype of time_t, which could be 32 or 64-bit, and signed or unsigned  */
/*  on some platforms.                                                       */
/*****************************************************************************/


#define USR_MKGMTIME_BASE_DAY    719527


time_t                      usr_mkgmtime (
struct tm *               t)
{
	time_t                    w_time_t;
	time_t                    this_year, this_mon, this_day, leap, max_day;
	int                       wday;

	static int                usr_mkgmtime_days_per_mon [13] =
	{  0,
	31,   /* JAN */
	28,   /* FEB */
	31,   /* MAR */
	30,   /* APR */
	31,   /* MAY */
	30,   /* JUN */
	31,   /* JUL */
	31,   /* AUG */
	30,   /* SEP */
	31,   /* OCT */
	30,   /* NOV */
	31    /* DEC */
	};

	/* offset from start of year for a given month.          */
	/* to compensate for normal day numbers starting with 1, */
	/* we pre-subtract 1 to optimize the calculation.        */

	static int                usr_mkgmtime_mon_offset [13] =
	{  0,
	0-1, /* JAN */       /* so Jan 1 is day 0 */
	31-1, /* FEB */
	59-1, /* MAR */
	90-1, /* APR */
	120-1, /* MAY */
	151-1, /* JUN */
	181-1, /* JUL */
	212-1, /* AUG */
	243-1, /* SEP */
	273-1, /* OCT */
	304-1, /* NOV */
	334-1  /* DEC */
	};

	/* validate struct tm input values */

	if (t == NULL)
	{
		return (time_t) (-1);
	}

	this_year = t->tm_year + 1900;
	this_mon  = t->tm_mon  + 1;
	this_day  = t->tm_mday;

	if ( (this_year  < 1970)
		||   (this_mon   <  1)
		||   (this_mon   > 12)
		||   (this_day   <  1)
		||   (t->tm_hour <  0)
		||   (t->tm_hour > 23)
		||   (t->tm_min  <  0)
		||   (t->tm_min  > 59)
		||   (t->tm_sec  <  0)
		||   (t->tm_sec  > 59) )
	{
		return (time_t) (-1);
	}

	leap = usr_mkgmtime_isleap (this_year);
	max_day = usr_mkgmtime_days_per_mon[this_mon];

	if (this_mon == (time_t) 2)
	{
		max_day += leap;
	}

	if (this_day > max_day)
	{
		return (time_t) (-1);
	}

	/* form number of days for given year.   we start by determining number */
	/* of leap-year days in the years prior to current one */

	w_time_t = (time_t) (((time_t) 365 * this_year) + 
		usr_mkgmtime_leap_year_days (this_year-1));

	/* convert year/mon to days using month-offset table */

	/* we must account for the fact that Jan 1 is day 0 of a year. */
	/* so, "day number" is one less than the number of days */
	/* by subtracting 1 from the day-offset of a given month */

	/* however, since this value will be constant for a given month, */
	/* the offset table already applies the '-1' factor, to save time. */
	/* see the comments above for the table's definition. */

	this_day += (time_t) usr_mkgmtime_mon_offset [this_mon];

	if (this_mon > (time_t) 2)
	{
		this_day += leap;    /* leap-year day's effect doesn't occur until March */
	}

	/* calculate base day of Jan 1 1970.  1970 was not a leap year, */
	/* so the number of leap-year days of 1970 is the same as 1969, */
	/* but to be consistent, we call the function using (1970-1).   */
	/* since 1,970 years go from the (theoretical) year 0 to 1969,  */
	/* the two parts of this equation are consistent.               */

	/* the following would calculate the base day value */

	/* however, since the values are all constants, the same base day will be  */
	/* produced each time.  running the equation with these constants produces */
	/* the number 719527, which we use in order to save time.                  */

	/* USR_MKGMTIME_BASE_DAY = (time_t) (1970 * 365)                           */
	/*                       + usr_mkgmtime_leap_year_days ((time_t) (1970-1)) */

	w_time_t += (time_t) (this_day - ((time_t) USR_MKGMTIME_BASE_DAY));

	/* create output fields: tm_wday and tm_yday */
	/* the unix epoch of 1970-01-01 was a Thursday, or tm_wday == 4, */

	/* tm_wday: Day of week (0 ?6; Sunday = 0) */
	/* tm_yday: Day of year (0 ?365; January 1 = 0) */

	wday = ((int) w_time_t + 4) % 7;

	w_time_t = (w_time_t * (time_t) 24) + (time_t) t->tm_hour;
	w_time_t = (w_time_t * (time_t) 60) + (time_t) t->tm_min;
	w_time_t = (w_time_t * (time_t) 60) + (time_t) t->tm_sec;

	if (w_time_t < (time_t) 0)
	{
		return (time_t) (-1);
	}

	/* store output fields after time_t has been validated */

	t->tm_wday = wday;
	t->tm_yday = (int) this_day;;

	return w_time_t;

} /* usr_mkgmtime */
