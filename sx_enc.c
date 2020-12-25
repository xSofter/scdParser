/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*     		1999 - 2008, All Rights Reserved		        */
/*									*/
/* MODULE NAME : sx_enc.c   						*/
/* PRODUCT(S)  : 							*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 01/30/08  EJV     57    Corr slogs (fun names).			*/
/************************************************************************/

#if defined (_WIN32)
#pragma warning(disable : 4996)
#endif

#include "glbtypes.h"
#include "sysincs.h"
#include "mem_chk.h"
#include "time_str.h"
#include "str_util.h"
#include "sx_defs.h"
#include "slog.h"

/************************************************************************/
/* For debug version, use a static pointer to avoid duplication of 	*/
/* __FILE__ strings.							*/

#ifdef DEBUG_SISCO
static ST_CHAR *SD_CONST thisFileName = __FILE__;
#endif


/************************************************************************/
ST_BOOLEAN sxUseFormatting;
ST_BOOLEAN sxLogOverrunAsFlow;
/************************************************************************/

#if 0
/* to increase speed we prefer not to call functions */
#define _SX_ADD_BUF(_src, _srcLen) sx_add_buf (sxEncCtrl, _src, _srcLen, &_writePos)
#define _SX_ADD_STRING(_src)       sx_add_string (sxEncCtrl, _src, &_writePos)

#else

#define _SX_ADD_BUF(_src, _srcLen)\
{\
	if (sxEncCtrl->useFp)\
{\
	if (fwrite( _src, sizeof( ST_CHAR ), _srcLen, sxEncCtrl->fp) != (ST_UINT) _srcLen)\
{\
	sxEncCtrl->errCode = SD_FAILURE;\
	SLOG_ERROR ("XML encode file write error");\
	return;\
}\
}\
  else\
{\
	if (_writePos + _srcLen >= sxEncCtrl->xmlBufEnd)\
{\
	sxEncCtrl->errCode = SX_XML_BUFFER_OVER_MAX;\
	if (sxLogOverrunAsFlow)\
{SLOG_WARN ("XML encode buffer overrun");}\
	  else\
{SLOG_ERROR ("XML encode buffer overrun");}\
	  return;\
}\
	else\
{\
	memcpy (_writePos, _src, _srcLen);\
	_writePos += _srcLen;\
	*_writePos = 0;\
}\
}\
}

#define _SX_ADD_STRING(_src)  _SX_ADD_BUF(_src, strlen(_src))
#endif


/************************************************************************/
/*			sx_add_string					*/
/************************************************************************/

ST_VOID sx_add_string (SX_ENC_CTRL *sxEncCtrl, SD_CONST ST_CHAR *_src,
					   ST_CHAR **writePosIo)
{
	sx_add_buf (sxEncCtrl, _src,  strlen(_src), writePosIo);
}

/************************************************************************/
/*			sx_add_buf					*/
/************************************************************************/

ST_RET sx_add_buf (SX_ENC_CTRL *sxEncCtrl, SD_CONST ST_CHAR *_src,  ST_INT _srcLen,
				   ST_CHAR **writePosIo)
{
	ST_CHAR *_writePos;

	if (sxEncCtrl->useFp)
	{
		if (fwrite( _src, sizeof( ST_CHAR ), _srcLen, sxEncCtrl->fp) != (ST_UINT) _srcLen)
		{
			sxEncCtrl->errCode = SD_FAILURE;
			SLOG_ERROR ("XML encode file write error");
			return (SD_FAILURE);
		}
	}
	else
	{
		_writePos = *writePosIo;
		if (_writePos + _srcLen >= sxEncCtrl->xmlBufEnd)
		{
			sxEncCtrl->errCode = SX_XML_BUFFER_OVER_MAX;
			if (sxLogOverrunAsFlow)
			{SLOG_WARN ("XML encode buffer overrun");}
			else
			{SLOG_ERROR ("XML encode buffer overrun");}
			return (SD_FAILURE);
		}
		else
		{
			memcpy (_writePos, _src, _srcLen);
			_writePos += _srcLen;
			*_writePos = 0;		/* need string termination for logging */
		}
		*writePosIo = _writePos; 
	}
	return (SD_SUCCESS);
}

/************************************************************************/
/*			sx_start_encode 				*/
/************************************************************************/

SX_ENC_CTRL *sx_start_encode (ST_CHAR *xmlBuf, ST_INT xmlBufLen)
{
	SX_ENC_CTRL *sxEncCtrl;

	sxEncCtrl = (SX_ENC_CTRL *) M_CALLOC (NULL, 1, sizeof (SX_ENC_CTRL));
	sx_init_encode (sxEncCtrl, xmlBuf, xmlBufLen);
	return (sxEncCtrl);
}


/************************************************************************/
/*			sx_start_encodeEx 				*/
/************************************************************************/

SX_ENC_CTRL *sx_start_encodeEx (ST_CHAR *fileName)
{
	SX_ENC_CTRL *sxEncCtrl;

	sxEncCtrl = (SX_ENC_CTRL *) M_CALLOC (NULL, 1, sizeof (SX_ENC_CTRL));
	if (sx_init_encodeEx (sxEncCtrl, fileName) != SD_SUCCESS)
	{
		M_FREE (NULL, sxEncCtrl);
		sxEncCtrl = NULL;
	}
	return (sxEncCtrl);
}

/************************************************************************/
/*			sx_init_encode 					*/
/************************************************************************/

ST_VOID sx_init_encode (SX_ENC_CTRL *sxEncCtrl, 
						ST_CHAR *xmlBuf, ST_INT xmlBufLen)
{
	/* sxEncCtrl must be cleared before usage */
	memset (sxEncCtrl, 0, sizeof(SX_ENC_CTRL));
	sxEncCtrl->xmlBufLen = xmlBufLen;
	sxEncCtrl->xmlBuf = xmlBuf;
	sxEncCtrl->xmlBufEnd = xmlBuf + xmlBufLen;
	sxEncCtrl->useFp = SD_FALSE;
	sxEncCtrl->nextWritePos = xmlBuf;
	sxEncCtrl->currNestLevel = 0;

	sxEncCtrl->errCode = SD_SUCCESS;
	sxEncCtrl->bUseFormatting = sxUseFormatting;
	SLOG_DEBUG ("Initialized XML Encode");
}

/************************************************************************/
/*			sx_init_encodeEx 					*/
/************************************************************************/

ST_RET sx_init_encodeEx (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *fileName)
{
	FILE *fp;

	if ((fp = fopen (fileName, "w")) == NULL)
	{
		SLOG_ERROR ("XML File (%s) Open Error (errno=%d)", fileName, errno);
		return (SD_FAILURE);
	}

	/* sxEncCtrl must be cleared before usage */
	memset (sxEncCtrl, 0, sizeof(SX_ENC_CTRL));
	sxEncCtrl->xmlBufLen = 0;
	sxEncCtrl->xmlBuf = NULL;
	sxEncCtrl->xmlBufEnd = NULL;
	sxEncCtrl->useFp = SD_TRUE;
	sxEncCtrl->fp = fp;
	sxEncCtrl->nextWritePos = NULL;
	sxEncCtrl->currNestLevel = 0;

	sxEncCtrl->errCode = SD_SUCCESS;
	sxEncCtrl->bUseFormatting = sxUseFormatting;
	SLOG_DEBUG ("Initialized XML Encode to file '%s'", fileName);

	return (SD_SUCCESS);
}

/************************************************************************/
/*			sx_end_encode 					*/
/************************************************************************/

ST_VOID sx_end_encode (SX_ENC_CTRL *sxEncCtrl)
{
	ST_LONG xmlLen;

	if (sxEncCtrl->useFp)
	{
		if (sxEncCtrl->fp)
			fclose (sxEncCtrl->fp);
		if (sxEncCtrl->errCode == SD_SUCCESS)
		{
			SLOG_DEBUG ("Encode XML to file complete.");
		}
	}
	else
	{
		/* encoding into buffer */
		if (sxEncCtrl->errCode == SD_SUCCESS)
		{
			xmlLen = sxEncCtrl->nextWritePos - sxEncCtrl->xmlBuf;
			SLOG_DEBUG ("Encode XML Complete: xmlLen %ld", xmlLen);
		}
	}
	M_FREE (NULL, sxEncCtrl);
}

/************************************************************************/
/*			sx_end_element 					*/
/************************************************************************/

ST_VOID sx_end_element (SX_ENC_CTRL *sxEncCtrl)
{
	ST_CHAR *tag;

	if (sxEncCtrl->errCode == 0)
	{
		--sxEncCtrl->currNestLevel;
		tag = sxEncCtrl->tags[sxEncCtrl->currNestLevel];
		sx_write_element (sxEncCtrl, tag, 0, NULL, SD_TRUE, SD_FALSE);
	}
}

/************************************************************************/
/*			sx_write_element 				*/
/************************************************************************/

ST_VOID sx_write_element (SX_ENC_CTRL *sxEncCtrl, SD_CONST ST_CHAR *tag, 
						  ST_INT numAttr, SXE_ATTR_PAIR *attr, 
						  ST_BOOLEAN end, ST_BOOLEAN empty)
{
	ST_INT i;
	ST_CHAR *_writePos;
	ST_CHAR tabBuf[SX_MAX_XML_NEST+1];
	ST_CHAR attrValFormatted [8192];

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;
		if (sxEncCtrl->bUseFormatting)
		{
			if (!sxEncCtrl->bOneLineEl || (sxEncCtrl->bOneLineEl && !end))
			{
				for (i = 0; i < sxEncCtrl->currNestLevel; i++)
					tabBuf[i] = '\t';		/* assemble all tabs in buffer	*/
				_SX_ADD_BUF (tabBuf, sxEncCtrl->currNestLevel);
			}
		}

		_SX_ADD_BUF ("<", 1);
		if (end == SD_TRUE)
			_SX_ADD_BUF ("/", 1);
		_SX_ADD_STRING (tag);

		for (i = 0; i < numAttr; ++i, ++attr)
		{
			_SX_ADD_BUF (" ", 1);
			_SX_ADD_STRING (attr->name);
			_SX_ADD_BUF ("=", 1);
			_SX_ADD_BUF ("\"", 1);
			sx_format_string_enc (attrValFormatted, attr->value);
			_SX_ADD_STRING (attrValFormatted);
			_SX_ADD_BUF ("\"", 1);
		}

		if (empty == SD_TRUE)
			_SX_ADD_BUF ("/", 1);

		_SX_ADD_BUF (">", 1);
		if (sxEncCtrl->bUseFormatting)
		{
			if (!sxEncCtrl->bOneLineEl || (sxEncCtrl->bOneLineEl && end))
				_SX_ADD_BUF ("\n", 1);
		}

		if (empty != SD_TRUE && end != SD_TRUE)
		{
			strcpy (sxEncCtrl->tags[sxEncCtrl->currNestLevel], tag);
			++sxEncCtrl->currNestLevel;
		}
		sxEncCtrl->nextWritePos = _writePos;
		sxEncCtrl->bOneLineEl = SD_FALSE;
	}
}

/************************************************************************/
/*			sx_wr_string 					*/
/************************************************************************/

ST_VOID sx_wr_string (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *str)
{
	ST_CHAR *_writePos;
	ST_CHAR strFormatted [8192];

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;
		if (!sxEncCtrl->bUnformattedStrings)
		{
			sx_format_string_enc (strFormatted, str);
			_SX_ADD_STRING (strFormatted);
		}
		else
			_SX_ADD_STRING (str);


		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_wrx_string_el 					*/
/************************************************************************/

ST_VOID sx_wrx_string_el (SX_ENC_CTRL *sxEncCtrl, SD_CONST ST_CHAR *tag, ST_CHAR *str, 
						  ST_INT numAttr, SXE_ATTR_PAIR *attr)

{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);
	sx_wr_string (sxEncCtrl, str);
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/*			sx_wr_nstring 					*/
/************************************************************************/

ST_VOID sx_wr_nstring (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *str, ST_INT len)
{
	ST_CHAR *_writePos;
	ST_CHAR strFormatted [8192];
	ST_INT  formattedLen;

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;
		if (!sxEncCtrl->bUnformattedStrings)
		{
			sx_format_nstring_enc (strFormatted, str, len);
			formattedLen = strlen (strFormatted);
			_SX_ADD_BUF (strFormatted, formattedLen);
		}
		else
			_SX_ADD_BUF (str, len);

	
		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_wrx_string_el 				*/
/************************************************************************/

ST_VOID sx_wrx_nstring_el (SX_ENC_CTRL *sxEncCtrl, SD_CONST ST_CHAR *tag, ST_CHAR *str, ST_INT len,
						   ST_INT numAttr, SXE_ATTR_PAIR *attr)

{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);
	sx_wr_nstring (sxEncCtrl, str, len);
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/*			sx_wr_bitstring					*/
/************************************************************************/

ST_VOID sx_wr_bitstring (SX_ENC_CTRL *sxEncCtrl, ST_INT numBits, 
						 ST_UINT8 *bitStr)
{
	ST_CHAR *_writePos;
	ST_UINT8 bitMask;
	ST_UINT8 *bytePtr;
	ST_INT i;

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;

		bitMask = 0x80;
		bytePtr = bitStr;
		for (i = 0; i < numBits; ++i)
		{
			if (*bytePtr & bitMask)
			{
				_SX_ADD_BUF ("1", 1);
			}
			else
			{
				_SX_ADD_BUF ("0", 1);
			}

			if (bitMask == 0x01)
			{
				bitMask = 0x80;
				++bytePtr;
			}
			else
				bitMask = bitMask >> 1;
		}  

	
		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_wrx_bitstring_el 					*/
/************************************************************************/

ST_VOID sx_wrx_bitstring_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, 
							 ST_INT numBits, ST_UINT8 *bitStr,
							 ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_bitstring (sxEncCtrl, numBits, bitStr);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/*			sx_wr_int 					*/
/************************************************************************/

ST_VOID sx_wr_int (SX_ENC_CTRL *sxEncCtrl, ST_INT val)
{
	ST_CHAR *_writePos;
	ST_CHAR strVal[100];

	if (sxEncCtrl->errCode == 0)
	{
#if defined(_WIN32) || defined(__QNX__)
		itoa (val, strVal, 10);
#else
		sprintf (strVal, "%d", val);
#endif
		_writePos = sxEncCtrl->nextWritePos;
		_SX_ADD_STRING (strVal);

	
		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_wrx_int_el 					*/
/************************************************************************/

ST_VOID sx_wrx_int_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, ST_INT val,
					   ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_int (sxEncCtrl, val);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/*			sx_wr_uint 					*/
/************************************************************************/

ST_VOID sx_wr_uint (SX_ENC_CTRL *sxEncCtrl, ST_UINT val)
{
	ST_CHAR *_writePos;
	ST_CHAR strVal[100];

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;
		sprintf (strVal, "%u", val);
		_SX_ADD_STRING (strVal);


		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_wrx_uint_el 					*/
/************************************************************************/

ST_VOID sx_wrx_uint_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, ST_UINT val,
						ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_uint (sxEncCtrl, val);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/*			sx_wr_ulong 					*/
/************************************************************************/

ST_VOID sx_wr_ulong (SX_ENC_CTRL *sxEncCtrl, ST_ULONG val)
{
	ST_CHAR *_writePos;
	ST_CHAR strVal[100];

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;
		sprintf (strVal, "%lu", val);
		_SX_ADD_STRING (strVal);


		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_wrx_ulong_el					*/
/************************************************************************/

ST_VOID sx_wrx_ulong_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, ST_ULONG val,
						 ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_ulong (sxEncCtrl, val);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/*			sx_wr_uint32_hex					*/
/************************************************************************/

ST_VOID sx_wr_uint32_hex (SX_ENC_CTRL *sxEncCtrl, ST_UINT32 val)
{
	ST_CHAR *_writePos;
	ST_CHAR strVal[100];

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;
		sprintf (strVal, "0x%08lx", val);
		_SX_ADD_STRING (strVal);

		sxEncCtrl->nextWritePos = _writePos;
	}
}
/************************************************************************/
/*		     sx_wrx_uint32_hex_el  				*/
/************************************************************************/

ST_VOID sx_wrx_uint32_hex_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, ST_UINT32 val,
							  ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_uint32_hex (sxEncCtrl, val);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/************************************************************************/
/*			sx_wr_double 					*/
/************************************************************************/

ST_VOID sx_wr_double (SX_ENC_CTRL *sxEncCtrl, ST_DOUBLE val)
{
	ST_CHAR *_writePos;
	ST_CHAR strVal[100];

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;
		sprintf (strVal, "%e", val);
		_SX_ADD_STRING (strVal);

		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_wrx_double_el				*/
/************************************************************************/

ST_VOID sx_wrx_double_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, ST_DOUBLE val,
						  ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_double (sxEncCtrl, val);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/************************************************************************/
/*			sx_wr_float 					*/
/************************************************************************/

ST_VOID sx_wr_float (SX_ENC_CTRL *sxEncCtrl, ST_FLOAT val)
{
	ST_CHAR *_writePos;
	ST_CHAR strVal[100];

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;
		sprintf (strVal, "%f", val);
		_SX_ADD_STRING (strVal);

		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_wrx_float_el					*/
/************************************************************************/

ST_VOID sx_wrx_float_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, ST_FLOAT val,
						 ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_float (sxEncCtrl, val);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/************************************************************************/
/*			sx_wr_long 					*/
/************************************************************************/

ST_VOID sx_wr_long (SX_ENC_CTRL *sxEncCtrl, ST_LONG val)
{
	ST_CHAR *_writePos;
	ST_CHAR strVal[100];

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;
#if defined(_WIN32) || defined(__QNX__)
		ltoa (val, strVal, 10);
#else
		sprintf (strVal,"%ld", val);
#endif
		_SX_ADD_STRING (strVal);


		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_wrx_long_el 					*/
/************************************************************************/

ST_VOID sx_wrx_long_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, ST_LONG val,
						ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_long (sxEncCtrl, val);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/************************************************************************/
/*			sx_wr_bool 					*/
/************************************************************************/

ST_VOID sx_wr_bool (SX_ENC_CTRL *sxEncCtrl, ST_BOOLEAN val)
{
	ST_CHAR *_writePos;

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;
		if (val != SD_FALSE)
		{
			_SX_ADD_BUF ("1", 1);
		}
		else
		{
			_SX_ADD_BUF ("0", 1);
		}

		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_wrx_bool_el 					*/
/************************************************************************/
ST_VOID sx_wrx_bool_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, ST_BOOLEAN val,
						ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_bool (sxEncCtrl, val);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/************************************************************************/
/************************************************************************/
/*			sx_wr_time 					*/
/************************************************************************/
#define SX_MAX_TIME_STRING_LEN 30
ST_CHAR *sxTimeFormatStr = "%m-%d-%Y %H:%M:%S";

ST_VOID sx_wr_time (SX_ENC_CTRL *sxEncCtrl, time_t val)
{
	ST_CHAR *_writePos;
	ST_CHAR timeStr[50];

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;
		strftime (timeStr, SX_MAX_TIME_STRING_LEN, sxTimeFormatStr, localtime (&val));
		_SX_ADD_STRING (timeStr);


		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_wr_tm 					*/
/************************************************************************/

ST_VOID sx_wr_tm (SX_ENC_CTRL *sxEncCtrl, struct tm *val)
{
	ST_CHAR *_writePos;
	ST_CHAR timeStr[50];

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;
		strftime (timeStr, SX_MAX_TIME_STRING_LEN, sxTimeFormatStr, val);
		_SX_ADD_STRING (timeStr);

		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*                       XmlDurationToString 				*/
/************************************************************************/

ST_RET XmlDurationToString (ST_CHAR *buffer, ST_LONG size, SX_DURATION *sxDuration)
{
	ST_CHAR temp[256];
	ST_BOOLEAN dateMemberFound = SD_FALSE;
	ST_BOOLEAN timeMemberFound = SD_FALSE;
	/* convert duration information stored in           */
	/* "SX_DURATION" structure to an xml string         */
	/* Use strncat_maxstrlen to avoid buffer overrun    */
	if (sxDuration->negative == SD_TRUE)
		strncpy_safe( buffer, "-P", size - 1 );
	else
		strncpy_safe( buffer, "P", size - 1 );

	if (sxDuration->years > 0 || 
		sxDuration->months > 0 ||
		sxDuration->days > 0)
		dateMemberFound = SD_TRUE;

	if (sxDuration->years > 0)
	{
		sprintf( temp, "%dY", sxDuration->years );
		strncat_maxstrlen( buffer, temp, size - 1 );
	}
	if (sxDuration->months > 0)
	{
		sprintf( temp, "%dM", sxDuration->months );
		strncat_maxstrlen( buffer, temp, size - 1 );
	}    
	if (sxDuration->days > 0)
	{
		sprintf( temp, "%dD", sxDuration->days );
		strncat_maxstrlen( buffer, temp, size - 1 );
	}    

	if (sxDuration->hours > 0 ||
		sxDuration->minutes > 0 ||
		sxDuration->seconds > 0 ||
		sxDuration->microseconds > 0)
	{
		timeMemberFound = SD_TRUE;
		sprintf( temp, "T" );
		strncat_maxstrlen( buffer, temp, size - 1 );
	}

	if (sxDuration->hours > 0)
	{
		sprintf( temp, "%dH", sxDuration->hours );
		strncat_maxstrlen( buffer, temp, size - 1 );
	}    
	if (sxDuration->minutes > 0)
	{
		sprintf( temp, "%dM", sxDuration->minutes );
		strncat_maxstrlen( buffer, temp, size - 1 );
	}    
	if (sxDuration->seconds > 0)
	{
		sprintf( temp, "%d", sxDuration->seconds );
		strncat_maxstrlen( buffer, temp, size - 1 );
	}    
	if (sxDuration->microseconds > 0)
	{
		if (sxDuration->seconds > 0)
			sprintf( temp, ".%06ld", sxDuration->microseconds );
		else
			sprintf( temp, "0.%06ld", sxDuration->microseconds );
		strncat_maxstrlen( buffer, temp, size - 1 );
	}    
	if (sxDuration->seconds > 0 || sxDuration->microseconds > 0)
	{
		sprintf( temp, "S" );
		strncat_maxstrlen( buffer, temp, size - 1 );
	}    

	if (dateMemberFound == SD_FALSE && timeMemberFound == SD_FALSE)
	{
		sprintf( temp, "T0S" ); /* 0 seconds = duration of zero */
		strncat_maxstrlen( buffer, temp, size - 1 );
	}    

	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_wr_duration					*/
/************************************************************************/
/* Convert a SX_DURATION structure into a text string, and store it in	*/
/* control structure for output to an XML file.  The string		*/
/* looks like the following: PnYnMnDTnHnMnS where n are the various	*/
/* values, P = period, Y = years, M = months, D = days, T = date/time	*/
/* seperator, H = hours, M = minutes, S = seconds.  Seconds may be a	*/
/* decimal number of arbitrary precisions.				*/
/* Ex: P12Y10M2DT0H40M27.87S						*/
/* Ex: P12Y10M2DT40M27.87S						*/
/************************************************************************/

ST_VOID sx_wr_duration (SX_ENC_CTRL *sxEncCtrl, SX_DURATION *sxDuration)
{
	// ST_RET rc;
	ST_CHAR *_writePos;
	ST_CHAR buffer[100]; /* arbitrary size */

	if (sxEncCtrl->errCode != 0)
		return;

	XmlDurationToString (buffer, sizeof(buffer), sxDuration);

	/* place newly created date/time string in xml encode structure */
	_writePos = sxEncCtrl->nextWritePos;
	_SX_ADD_STRING (buffer);

	sxEncCtrl->nextWritePos = _writePos;
}

/************************************************************************/
/*                       sx_wr_xtime					*/
/************************************************************************/
/* Using the date and time data in the "SX_DATE_TIME" structure convert */
/* the number of seconds from 1/1/1970 (UTC time), the decimal fraction */
/* of microseconds if specified and the number of minutes in the        */
/* time zone offset if specified to a date and time string.             */
/* The string is stored in an encode structure for output to an xml     */
/* file.                                                                */
/************************************************************************/

ST_VOID sx_wr_xtime (SX_ENC_CTRL *sxEncCtrl, SX_DATE_TIME *sxDateTime)
{
	ST_RET rc;
	ST_CHAR *_writePos;
	ST_CHAR dateTimeStr[MAX_TIME_STRING_LEN];

	if (sxEncCtrl->errCode != 0)
		return;

	/* convert date and time information stored in     */
	/* "SX_DATE_TIME" structure to an xml string       */
	rc = UtcValueToXmlString (dateTimeStr, sizeof(dateTimeStr), sxDateTime);
	if (rc != SD_SUCCESS)
	{
		SLOG_ERROR ("ERROR: Value to String date/time conversion ");
		sxEncCtrl->errCode = SD_FAILURE;
		return;
	}

	/* place newly created date/time string in xml encode structure */
	_writePos = sxEncCtrl->nextWritePos;
	_SX_ADD_STRING (dateTimeStr);

	sxEncCtrl->nextWritePos = _writePos;
}

/************************************************************************/
/*			sx_wrx_duration_el 				*/
/************************************************************************/

ST_VOID sx_wrx_duration_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, SX_DURATION *val,
							ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_duration (sxEncCtrl, val);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/*			sx_wrx_xtime_el 				*/
/************************************************************************/

ST_VOID sx_wrx_xtime_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, SX_DATE_TIME *val,
						 ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_xtime (sxEncCtrl, val);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}


/************************************************************************/
/*			sx_wrx_time_el 					*/
/************************************************************************/

ST_VOID sx_wrx_time_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, time_t val,
						ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_time (sxEncCtrl, val);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/*			sx_wrx_tm_el 					*/
/************************************************************************/

ST_VOID sx_wrx_tm_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, struct tm *val,
					  ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_tm (sxEncCtrl, val);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}

/************************************************************************/
/*			sx_wr_cdata 					*/
/************************************************************************/

ST_VOID sx_wr_cdata (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *str, ST_INT len)
{
	ST_INT i;
	ST_CHAR *_writePos;
	ST_CHAR tabBuf[SX_MAX_XML_NEST+1];

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;

		if (sxEncCtrl->bUseFormatting)
		{
			for (i = 0; i < sxEncCtrl->currNestLevel; i++)
				tabBuf[i] = '\t';		/* assemble all tabs in buffer	*/
			_SX_ADD_BUF (tabBuf, sxEncCtrl->currNestLevel);
		}

		_SX_ADD_BUF ("<![CDATA[", 9);
		_SX_ADD_BUF (str, len);
		_SX_ADD_BUF ("]]>", 3);

		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_wrx_cdata_el 					*/
/************************************************************************/

ST_VOID sx_wrx_cdata_el (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *tag, 
						 ST_CHAR *str, ST_INT len,
						 ST_INT numAttr, SXE_ATTR_PAIR *attr)
{
	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_write_element (sxEncCtrl, tag, numAttr, attr, SD_FALSE, SD_FALSE);

	sx_wr_cdata (sxEncCtrl, str, len);

	sxEncCtrl->bOneLineEl = SD_TRUE;
	sx_end_element (sxEncCtrl);
}


/************************************************************************/
/*			sx_wr_comment 					*/
/************************************************************************/

ST_VOID sx_wr_comment (SX_ENC_CTRL *sxEncCtrl, ST_CHAR *str)
{
	ST_INT i;
	ST_CHAR *_writePos;
	ST_INT    writeLen;
	ST_CHAR tabBuf[SX_MAX_XML_NEST+1];

	if (sxEncCtrl->errCode == 0)
	{
		_writePos = sxEncCtrl->nextWritePos;

		if (sxEncCtrl->bUseFormatting)
		{
			for (i = 0; i < sxEncCtrl->currNestLevel; i++)
				tabBuf[i] = '\t';		/* assemble all tabs in buffer	*/
			_SX_ADD_BUF (tabBuf, sxEncCtrl->currNestLevel);
		}

		_SX_ADD_BUF ("<!--", 4);
		/* check if there is NL char at the end of string */
		writeLen = (ST_INT)strlen(str);
		if (str[writeLen-1] == '\n')
		{
			/* NL present, we will write it after the end of comment */
			--writeLen;
			_SX_ADD_BUF (str, writeLen);
			_SX_ADD_BUF ("-->\n", 4);
		}
		else
		{
			_SX_ADD_BUF (str, writeLen);
			_SX_ADD_BUF ("-->", 3);
		}

		sxEncCtrl->nextWritePos = _writePos;
	}
}

/************************************************************************/
/*			sx_format_string_enc 				*/
/************************************************************************/

ST_VOID sx_format_string_enc (ST_CHAR *dest, ST_CHAR *src)
{
	ST_CHAR *srcPtr = src;
	ST_CHAR *destPtr = dest;

	*destPtr = '\0';

	while (*srcPtr != '\0')
	{
		switch (*srcPtr)
		{
		case CHAR_APOS:
			strcat (destPtr, CODE_APOS);
			destPtr += CODE_APOS_LEN;
			break;

		case CHAR_QUOT:
			strcat (destPtr, CODE_QUOT);
			destPtr += CODE_QUOT_LEN;
			break;

		case CHAR_AMP:
			strcat (destPtr, CODE_AMP);
			destPtr += CODE_AMP_LEN;
			break;

		case CHAR_LT:
			strcat (destPtr, CODE_LT);
			destPtr += CODE_LT_LEN;
			break;

		case CHAR_GT:
			strcat (destPtr, CODE_GT);
			destPtr += CODE_GT_LEN;
			break;

		default:
			*destPtr = *srcPtr;
			*(destPtr + 1) = '\0';
			destPtr++;
			break;
		}

		srcPtr++;
	}
}

/************************************************************************/
/*			sx_format_nstring_enc 				*/
/************************************************************************/

ST_VOID sx_format_nstring_enc (ST_CHAR *dest, ST_CHAR *src, ST_INT len)
{
	ST_CHAR *srcPtr = src;
	ST_CHAR *destPtr = dest;
	ST_INT i = 0;

	*destPtr = '\0';

	while (i++ < len)
	{
		switch (*srcPtr)
		{
		case CHAR_APOS:
			strcat (destPtr, CODE_APOS);
			destPtr += CODE_APOS_LEN;
			break;

		case CHAR_QUOT:
			strcat (destPtr, CODE_QUOT);
			destPtr += CODE_QUOT_LEN;
			break;

		case CHAR_AMP:
			strcat (destPtr, CODE_AMP);
			destPtr += CODE_AMP_LEN;
			break;

		case CHAR_LT:
			strcat (destPtr, CODE_LT);
			destPtr += CODE_LT_LEN;
			break;

		case CHAR_GT:
			strcat (destPtr, CODE_GT);
			destPtr += CODE_GT_LEN;
			break;

		default:
			*destPtr = *srcPtr;
			*(destPtr + 1) = '\0';
			destPtr++;
			break;
		}

		srcPtr++;
	}
}

