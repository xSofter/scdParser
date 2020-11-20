/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*     		1999 - 2008, All Rights Reserved		        */
/*									*/
/* MODULE NAME : sx_dec.c						*/
/* PRODUCT(S)  : 							*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 02/06/08  EJV     88    Added SX_PARSING_OK, sx_err_str* (for slogs).*/
/* For early MODLOGS see previous revision of this file on PVCS.	*/
/************************************************************************/

#if defined (_WIN32)
#pragma warning(disable : 4996)
#endif

#include "glbtypes.h"
#include "sysincs.h"
#include "mem_chk.h"
#include "sx_defs.h"
//#include "time_str.h"
#include "str_util.h"
#include "slog.h"



ST_ULONG g_rowIndex;
/************************************************************************/
/* For debug version, use a static pointer to avoid duplication of 	*/
/* __FILE__ strings.							*/

#ifdef DEBUG_SISCO
SD_CONST static ST_CHAR *SD_CONST thisFileName = __FILE__;
#endif

#if defined(USE_EXPAT)
/* Assume linking to Expat "static" library. Must define XML_STATIC	*/
/* before including "expat.h".						*/
#define XML_STATIC
#include "expat.h"
#define EXPAT_BUF_SIZE	8192	/* parse buffer size if reading from file*/
#define ENTITY_BUF_SIZE	2000	/* initial size to alloc for entity buffer*/
/* reallocate larger if needed		*/

static void XMLCALL expatHandlerStartSkip(void *userData, const char *el, const char **attr);
static void XMLCALL expatHandlerEndSkip(void *userData, const char *el);
#endif
/************************************************************************/




/************************************************************************/

ST_BOOLEAN sxUseSax;
ST_BOOLEAN sxIgnoreNS;

ST_RET sx_rip_xml (SX_DEC_CTRL *sxDecCtrl);
ST_RET sx_rip_xml_file (SX_DEC_CTRL *sxDecCtrl);
ST_RET sx_rip_xml_mem (SX_DEC_CTRL *sxDecCtrl);

SX_ELEMENT *_uibed_find_element (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *tag, ST_INT **numOccOut);
static ST_VOID _sx_pop (SX_DEC_CTRL *sxDecCtrl, ST_BOOLEAN auto_pop);

/************************************************************************/
/*				sx_parseExx_mt 				*/
/* Note: when making changes/correction to this function revise also	*/
/*       other sx_parse functions.					*/
/* Decode the XML located in file by loading the whole thing into	*/
/* memory first ...							*/
/************************************************************************/

ST_RET sx_parseExx_mt (ST_CHAR *fileName, ST_INT numItems, 
					   SX_ELEMENT *itemTbl, ST_VOID *usr,
					   ST_RET (*u_sx_el_start_fun) (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *tag),
					   ST_RET (*u_sx_el_end_fun) (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *tag))
{
	ST_RET rc;
	ST_LONG fileSize;
	ST_LONG bytesRead;
	ST_CHAR *cfgData;
	struct stat buf;
	int result;
	FILE *fp;

	/* Get the size of the file */
	fp = fopen (fileName, "rb");
	if (fp == NULL)
	{
		SLOG_WARN ("XML File (%s) Open Error (errno=%d)", fileName, errno);
		return (SX_FILE_NOT_FOUND);
	}

	//fstat()用来将参数fp所指的文件状态, 复制到参数buf 所指的结构中(struct stat). 
	result = fstat (fileno (fp), &buf);
	if (result != 0)
		return (SD_FAILURE);

	/* Allocate a buffer and read all into memory */
	fileSize = buf.st_size;
	cfgData = (ST_CHAR *) chk_malloc (fileSize);
	if (cfgData == NULL) {
		return (SD_FAILURE);
	}
	memset(cfgData, 0, fileSize);

	//return 返回实际读取的单元个数,length
	//cfgData 接收数据的地址,1为单元的大小,每个单元fileSize个字节,fp为文件流
	bytesRead = fread (cfgData, 1, fileSize, fp);
	fclose (fp);

	if (bytesRead < fileSize)
	{
		chk_free (cfgData);
		SLOG_WARN ("Error: Could not read from '%s'", fileName);
		return (SX_FILE_NOT_FOUND);
	}

	SLOG_DEBUG("numItems: %d", numItems);
	rc = sx_parse_mt (bytesRead, cfgData, numItems, itemTbl, usr, 
		u_sx_el_start_fun, u_sx_el_end_fun); 
	if (rc != SD_SUCCESS)
	{
		SLOG_WARN ("ERROR: parsing failed, return code: '%d'", rc);
	}

	chk_free (cfgData);
	return (rc);
}

/************************************************************************/
/*				sx_parse_mt				*/
/* Note: when making changes/correction to this function revise also	*/
/*       other sx_parse functions.					*/
/* Decode the XML located in memory (thread safe version).		*/
/************************************************************************/
ST_RET sx_parse_mt (ST_LONG lMsgLen, ST_CHAR *xml, ST_INT numItems, 
					SX_ELEMENT *itemTbl, ST_VOID *usr,
					ST_RET (*u_sx_el_start_fun) (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *tag),
					ST_RET (*u_sx_el_end_fun) (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *tag))
{
	SX_DEC_CTRL *sxDecCtrl;
	ST_RET rc;

	SLOG_DEBUG ("Start Decoding XML");

	sxDecCtrl = (SX_DEC_CTRL *) calloc (1, sizeof (SX_DEC_CTRL));
	/* !we need to use here the system calloc */

	//这个函数,每次进来,栈深度加一
	sx_push (sxDecCtrl, numItems, itemTbl, SD_FALSE);
	SLOG_DEBUG("sxDecCtrl->itemStackLevel: %d", sxDecCtrl->itemStackLevel);
	sxDecCtrl->xmlStart = xml;
	sxDecCtrl->xmlLen = lMsgLen;
	sxDecCtrl->ignoreNS = sxIgnoreNS;
	sxDecCtrl->useFp = SD_FALSE;
	sxDecCtrl->usr = usr;
	sxDecCtrl->u_sx_el_start = u_sx_el_start_fun;
	sxDecCtrl->u_sx_el_end   = u_sx_el_end_fun;

#if defined(USE_EXPAT)
	sx_rip_xml_mem (sxDecCtrl);
#else
	sx_rip_xml (sxDecCtrl);
#endif

	if (sxDecCtrl->xmlNestLevel != 0 && sxDecCtrl->errCode == SD_SUCCESS)
	{
		sxDecCtrl->errCode = SX_XML_MALFORMED;
		SLOG_ERROR ("Invalid XML nesting");
	}

	rc = sxDecCtrl->errCode;

	free (sxDecCtrl);

	return (rc);
}

/************************************************************************/
/*				sx_parseEx_mt				*/
/* Note: when making changes/correction to this function revise also	*/
/*       other sx_parse functions.					*/
/* Decode the XML located in file (thread safe version).		*/
/************************************************************************/
ST_RET sx_parseEx_mt (ST_CHAR *fileName, ST_INT numItems, 
					  SX_ELEMENT *itemTbl, ST_VOID *usr,
					  ST_RET (*u_sx_el_start_fun) (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *tag),
					  ST_RET (*u_sx_el_end_fun) (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *tag))
{
	SX_DEC_CTRL *sxDecCtrl;
	ST_RET rc;
	FILE *fp;

	SLOG_DEBUG ("Start Decoding XML file %s", fileName);
	if ((fp = fopen (fileName,"r"))==NULL)
	{
		SLOG_WARN ("XML File (%s) Open Error",fileName);
		return (SX_FILE_NOT_FOUND);
	}

	sxDecCtrl = (SX_DEC_CTRL *) calloc (1, sizeof (SX_DEC_CTRL));
	/* !we need to use here the system calloc */

	sx_push (sxDecCtrl, numItems, itemTbl, SD_FALSE);
	sxDecCtrl->fp = fp;
	sxDecCtrl->ignoreNS = sxIgnoreNS;
	sxDecCtrl->useFp = SD_TRUE;
	sxDecCtrl->usr = usr;
	sxDecCtrl->u_sx_el_start = u_sx_el_start_fun;
	sxDecCtrl->u_sx_el_end   = u_sx_el_end_fun;

#if defined(USE_EXPAT)
	sx_rip_xml_file (sxDecCtrl);
#else
	sx_rip_xml (sxDecCtrl);
#endif

	if (sxDecCtrl->errCode == 0)
	{
		if (sxDecCtrl->xmlNestLevel != 0)
		{
			sxDecCtrl->errCode = SX_XML_MALFORMED;
			SLOG_ERROR ("Invalid XML nesting");
		}
	}

	rc = sxDecCtrl->errCode;

	free (sxDecCtrl);

	fclose (fp);
	return (rc);
}


/************************************************************************/
/*			sxStartElement 					*/
/************************************************************************/

ST_VOID sxStartElement (SX_DEC_CTRL *sxDecCtrl) 
{
	SX_ELEMENT *item;
	SX_DEC_ELEMENT_INFO *sxDecElInfo;
	ST_CHAR *tag;
	ST_RET rc = SD_FAILURE;
	ST_INT stackLevelSave;
	ST_INT *numOccPtr;

	sxDecElInfo = &sxDecCtrl->sxDecElInfo;
	tag = sxDecElInfo->tag;
	SLOG_DEBUG ("Start element '%s'", tag);

	if (sxDecCtrl->errCode != SD_SUCCESS && sxDecCtrl->errCode != SX_ERR_CONVERT)
	{
		return;
	}

	item = _uibed_find_element (sxDecCtrl, tag, &numOccPtr);

	stackLevelSave = sxDecCtrl->itemStackLevel;

	SLOG_DEBUG ("start itemStackLevel %d", stackLevelSave);

	while (item == NULL && sxDecCtrl->itemStackLevel > 0)
	{
		// SLOG_DEBUG("sxDecCtrl->auto_pop[%d] = %d", sxDecCtrl->itemStackLevel-1,  sxDecCtrl->auto_pop[sxDecCtrl->itemStackLevel-1]);
		if (sxDecCtrl->auto_pop[sxDecCtrl->itemStackLevel-1] == SD_TRUE)
		{
			_sx_pop (sxDecCtrl, SD_TRUE);
			item = _uibed_find_element (sxDecCtrl, tag, &numOccPtr);
		}
		else
			break;
	}

	if (item != NULL)
	{
		if (*numOccPtr != 0 && ((item->elementFlags & SX_ELF_RPT) == 0))
		{
			sxDecCtrl->errCode = SX_DUPLICATE_NOT_ALLOWED;
			SLOG_ERROR("Row = %lu,Duplicate of element '%s' not allowed", g_rowIndex,tag);
			return;       
		}
		++(*numOccPtr);

		if (*numOccPtr > 1)
		{
			SLOG_WARN ("Number occurences: %d", *numOccPtr);
		}

		/* Save the item for later */
		++sxDecCtrl->xmlNestLevel;
		// SLOG_DEBUG ("xmlNestLevel : %d", sxDecCtrl->xmlNestLevel);
		sxDecCtrl->elTbl[sxDecCtrl->xmlNestLevel] = item;

		/* Call the user function, if there is one ... */
		sxDecCtrl->item = item;
		sxDecCtrl->reason = SX_ELEMENT_START;
		if ((item->elementFlags & SX_ELF_CSTART) != 0)
		{
			if (item->funcPtr != NULL)
			{
				//not use by default
				sxDecCtrl->elUser = item->user;
				//run sclStartElements _SCL_SEFun
				SLOG_DEBUG ("Run item tbl %s's func", item->tag);
				(item->funcPtr)(sxDecCtrl);
			}
			else
			{
				SLOG_WARN ("No state function for this element");
			}
		}
	}
	else if (sxDecCtrl->u_sx_el_start != NULL)
	{
		sxDecCtrl->itemStackLevel = stackLevelSave;

		if (sxDecCtrl->u_sx_el_start != NULL)
		{
			rc = (*(sxDecCtrl->u_sx_el_start)) (sxDecCtrl, tag);
		}
		if (rc == SD_SUCCESS)
		{
			++sxDecCtrl->xmlNestLevel;
			sxDecCtrl->elTbl[sxDecCtrl->xmlNestLevel] = NULL;
		}
		else
		{
			sxDecCtrl->errCode = SX_STRUCT_NOT_FOUND;
			SLOG_WARN ("u_sx_el_start failed for element '%s'", tag);
		}
	}
	else
	{
		sxDecCtrl->errCode = SX_STRUCT_NOT_FOUND;
		SLOG_ERROR ("Row = %lu, Could not find element '%s' in element table",g_rowIndex,tag);
	}
}

/************************************************************************/
/*			sxEndElement 					*/
/************************************************************************/

ST_VOID sxEndElement (SX_DEC_CTRL *sxDecCtrl)
{
	SX_DEC_ELEMENT_INFO *sxDecElInfo;
	SX_ELEMENT *item;
	ST_CHAR *tag;
	ST_RET rc = SD_FAILURE;

	sxDecElInfo = &sxDecCtrl->sxDecElInfo;
	tag = sxDecElInfo->tag;

	if (sxDecCtrl->errCode != SD_SUCCESS && sxDecCtrl->errCode != SX_ERR_CONVERT)
	{
		return;
	}

	item = sxDecCtrl->elTbl[sxDecCtrl->xmlNestLevel];
	--sxDecCtrl->xmlNestLevel;

	if (item != NULL)
	{
		if (strcmp (tag, item->tag) != 0) /* verify end tag */
		{
			sxDecCtrl->errCode = SX_XML_MALFORMED;
			SLOG_WARN("Row = %lu, XML malformed: found </%s>, expected </%s>",g_rowIndex, tag, item->tag);
		}
		else
		{
			//调用element end 函数,这个函数由bit mask 控制
			if ((item->elementFlags & SX_ELF_CEND) != 0)
			{
				sxDecCtrl->item = item;
				sxDecCtrl->reason = SX_ELEMENT_END;
				if (item->funcPtr != NULL)
				{
					sxDecCtrl->elUser = item->user;
					(item->funcPtr)(sxDecCtrl);
				}
				else
				{
					SLOG_WARN ("No state function for this element");
				}
			}
		}
	}
	else
	{
		if (sxDecCtrl->u_sx_el_end != NULL)
		{
			rc = (*(sxDecCtrl->u_sx_el_end)) (sxDecCtrl, sxDecCtrl->sxDecElInfo.tag);
			if (rc != SD_SUCCESS)
			{
				sxDecCtrl->errCode = SX_STRUCT_NOT_FOUND;
				SLOG_ERROR ("Row = %lu, u_sx_el_end failed for element '%s'",g_rowIndex, tag);
			}
		}
	}
}

/************************************************************************/
/************************************************************************/
/*				sx_push 				*/
/************************************************************************/

ST_VOID sx_push (SX_DEC_CTRL *sxDecCtrl, ST_INT numItems, SX_ELEMENT *itemTbl,
				 ST_BOOLEAN auto_pop)
{
	ST_INT i;
	SX_ELEMENT_TBL_CTRL *itemTblCtrl;
	ST_INT *numOccTbl;

	/* Do some sanity checks first */
	if (sxDecCtrl->itemStackLevel >= SX_MAX_STACK_LEVEL)
	{
		sxDecCtrl->errCode = SX_XML_NEST_TOO_DEEP;
		return;
	}
	if (numItems > SX_MAX_ITEMS_PER_TABLE)
	{
		sxDecCtrl->errCode = SX_ELEMENT_TBL_TOO_BIG;
		return;
	}

	itemTblCtrl = &sxDecCtrl->items[sxDecCtrl->itemStackLevel];
	numOccTbl = itemTblCtrl->numOccTbl;

	itemTblCtrl->itemTbl= itemTbl;
	itemTblCtrl->numItems= numItems;
	sxDecCtrl->auto_pop[sxDecCtrl->itemStackLevel] = auto_pop;
	++sxDecCtrl->itemStackLevel;
	SLOG_DEBUG ("Sx_push itemTblCtrl %s", itemTbl->tag);
	/* reset the numOCc elements */
	for (i = 0; i < numItems; ++i)
		numOccTbl[i] = 0;
}

/************************************************************************/
/*				sx_pop 					*/
/************************************************************************/

ST_VOID sx_pop (SX_DEC_CTRL *sxDecCtrl)
{
	_sx_pop (sxDecCtrl, SD_FALSE);
}


/************************************************************************/
/*			_sx_pop						*/
/************************************************************************/

static ST_VOID _sx_pop (SX_DEC_CTRL *sxDecCtrl, ST_BOOLEAN auto_pop)
{
	SX_ELEMENT_TBL_CTRL *itemTblCtrl;
	SX_ELEMENT *item;
	ST_INT i;
	ST_INT *numOccTbl;

	/* Check for mandatory elements */
	if (sxDecCtrl->itemStackLevel > 0)
	{
		--sxDecCtrl->itemStackLevel;
		/* If auto-popping AND have a unknown element handler, don't check mandatory */
		if (auto_pop && sxDecCtrl->u_sx_el_start != NULL)
			return;

		itemTblCtrl = &sxDecCtrl->items[sxDecCtrl->itemStackLevel];
		numOccTbl = itemTblCtrl->numOccTbl;
		for (i = 0; i < itemTblCtrl->numItems; ++i)
		{
			item = &itemTblCtrl->itemTbl[i];
			if (numOccTbl[i] == 0 && ((item->elementFlags & SX_ELF_OPT) == 0))
			{
				sxDecCtrl->errCode = SX_REQUIRED_TAG_NOT_FOUND;
				SLOG_ERROR ("Row = %lu, Mandatory element '%s' not found", g_rowIndex, item->tag);
				break;       
			}
		}
	}
}


/************************************************************************/
/************************************************************************/

SX_ELEMENT *_uibed_find_element (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *tag, ST_INT **numOccPtrOut)
{
	SX_ELEMENT_TBL_CTRL *itemTblCtrl;
	ST_INT numItems;
	SX_ELEMENT *item;
	ST_INT i;

	itemTblCtrl = &sxDecCtrl->items[sxDecCtrl->itemStackLevel-1];
	item = itemTblCtrl->itemTbl;
	numItems = itemTblCtrl->numItems;
	SLOG_DEBUG("item numItems %d", numItems);
	/* See if this element is in our table */
	for (i = 0; i < numItems; ++i, ++item)
	{
		if (strcmp (tag, item->tag) == 0)
		{
			*numOccPtrOut = &itemTblCtrl->numOccTbl[i];
			return (item);
		}
	}
	return (NULL);
}



/************************************************************************/
/************************************************************************/
/************************************************************************/

ST_RET sx_get_element_contents (SX_DEC_CTRL *sxDecCtrl, 
								ST_CHAR *destBuf, ST_INT destLen,
								ST_INT *lenOut)
{
	ST_CHAR *src;
	ST_INT len;
	ST_INT rc;

	rc = sx_find_element_contents (sxDecCtrl, &src, &len);
	if (rc != SD_SUCCESS)
		return (rc);

	if (destLen < len)
	{
		SLOG_ERROR ("Error: sx_get_element_contents: dest too small for tag '%s'", sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (SD_FAILURE);
	}

	memcpy (destBuf, src, len);
	*lenOut = len;
	return (SD_SUCCESS);
}

/************************************************************************/

ST_RET sx_find_element_contents (SX_DEC_CTRL *sxDecCtrl, 
								 ST_CHAR **elStartOut, ST_INT *lenOut)
{
	SX_DEC_ELEMENT_INFO *sxDecElInfo;
	ST_CHAR *start;
	ST_CHAR *end;
	ST_CHAR endTagBuf[100];
	ST_INT len;
#if defined(USE_EXPAT)
	int offset;
	int size;
	const char *ptr;

	/* Doesn't work if reading one buffer at a time from a file.	*/
	if (sxDecCtrl->fp)
	{
		SLOG_ERROR ("sx_find_element_contents only works if entire XML is in memory. Use sx_parseExx_mt or sx_parse_mt.");
		return (SD_FAILURE);
	}

	/* Set "start" ptr to point after end of this start tag.	*/
	ptr = XML_GetInputContext(sxDecCtrl->parser, &offset, &size);
	ptr += offset;	/* point to current position in XML buffer	*/
	/* should be beginning of start tag		*/
	start = strchr (ptr, '>') + 1;	/* point after end of start tag	*/

	/* save nest level to help find corresponding end tag	*/
	sxDecCtrl->skipNestLevel = sxDecCtrl->xmlNestLevel;
	/* Change handlers to skip to end of this element (look for end tag).	*/
	XML_SetCharacterDataHandler(sxDecCtrl->parser, NULL);	/* ignore element data	*/
	XML_SetElementHandler(sxDecCtrl->parser, expatHandlerStartSkip, expatHandlerEndSkip);
	sxDecElInfo = &sxDecCtrl->sxDecElInfo;
#else
	sxDecElInfo = &sxDecCtrl->sxDecElInfo;
	start = sxDecCtrl->xmlPos;
#endif
	/* Check to see if this is an empty element */
	if (*(start - 2) == '/')
	{
		*lenOut = 0;
		return (SD_SUCCESS);
	}

	*elStartOut = start;
	len = sxDecCtrl->xmlLen - (start - sxDecCtrl->xmlStart);

	/* We need to find the closing element for this start element */
	endTagBuf[0] = '<';
	endTagBuf[1] = '/';
	strcpy (&endTagBuf[2], sxDecElInfo->tag);
	strcat (endTagBuf, ">");

	end = strnstr (start, endTagBuf, len);
	if (end == NULL)
	{
		SLOG_ERROR("Can't find end tag '%s'", sxDecElInfo->tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (SD_FAILURE);
	}
	*lenOut = end - start;

#if !defined(USE_EXPAT)		/* with Expat, can't set or use xmlPos	*/
	sxDecCtrl->xmlPos = end;
#endif

	SLOG_DEBUG("sx_find_element_contents got data:");
	return (SD_SUCCESS);
}

/************************************************************************/
/************************************************************************/
/*			sx_get_entity 					*/
/************************************************************************/

ST_RET sx_get_entity (SX_DEC_CTRL *sxDecCtrl, 
					  ST_CHAR *destBuf, ST_INT destLen,
					  ST_INT *lenOut)
{
#if defined(USE_EXPAT)
	if (destLen < sxDecCtrl->entityLen)
	{
		SLOG_ERROR ("Error: sx_get_entity: dest too small (max %d bytes expected) for tag '%s'", destLen, sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		*lenOut = 0;	/* return empty buf	*/
		return (SD_FAILURE);
	}
	else
	{
		memcpy (destBuf, sxDecCtrl->entityBuf, sxDecCtrl->entityLen);
		*lenOut = sxDecCtrl->entityLen;
	}
	if (*lenOut == 0)
		SLOG_WARN("sx_get_entity (no data)");
	else
	{
		SLOG_DEBUG("sx_get_entity got data:");
		SXLOG_DECH (*lenOut, destBuf);
	}
	return (SD_SUCCESS);
#else	/* !USE_EXPAT	*/
	SX_DEC_ELEMENT_INFO *sxDecElInfo;
	ST_INT i;
	ST_CHAR *dest;
	ST_CHAR *src;
	ST_CHAR *end;

	sxDecElInfo = &sxDecCtrl->sxDecElInfo;
	src = sxDecElInfo->entityStart;
	dest = destBuf;
	end = sxDecElInfo->entityEnd;
	for (i = 0; i < destLen && src < end; ++i, ++src, ++dest)
		*dest = *src;

	*lenOut = i;

	if (src < end)
	{
		SLOG_ERROR ("Error: sx_get_entity: dest too small (max %d bytes expected) for tag '%s'", destLen, sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (SD_FAILURE);
	}

	// SLOG_DEBUG("sx_get_entity got data:");
	return (SD_SUCCESS);
#endif	/* !USE_EXPAT	*/
}


/************************************************************************/
/*                       sx_get_bool					*/
/************************************************************************/

ST_RET sx_get_bool (SX_DEC_CTRL *sxDecCtrl, ST_BOOLEAN *out_ptr)
{
	ST_INT d;
	ST_RET rc;

	rc = sx_get_value (sxDecCtrl, "%d", &d);
	if (rc != SD_SUCCESS)
		return (rc);

	if (d == 0)
		*out_ptr = SD_FALSE;
	else
		*out_ptr = SD_TRUE;

	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_get_int					*/
/************************************************************************/

ST_RET sx_get_int (SX_DEC_CTRL *sxDecCtrl, ST_INT *out_ptr)
{
	return (sx_get_value (sxDecCtrl, "%d", out_ptr));
}

/************************************************************************/
/*                       sx_get_float					*/
/************************************************************************/

ST_RET sx_get_float (SX_DEC_CTRL *sxDecCtrl, ST_FLOAT *out_ptr)
{
	return (sx_get_value (sxDecCtrl, "%f", out_ptr));
}

/************************************************************************/
/*                       sx_get_double					*/
/************************************************************************/

ST_RET sx_get_double (SX_DEC_CTRL *sxDecCtrl, ST_DOUBLE *out_ptr)
{
	/* Note: must use the '%le' format specifier for double */
	return (sx_get_value (sxDecCtrl, "%le", out_ptr));
}

/************************************************************************/
/*                       sx_get_uchar					*/
/************************************************************************/

ST_RET sx_get_uchar (SX_DEC_CTRL *sxDecCtrl, ST_UCHAR *out_ptr)
{
	return (sx_get_value (sxDecCtrl, "%c", out_ptr));
}

/************************************************************************/
/*                       sx_get_int16					*/
/************************************************************************/

ST_RET sx_get_int16 (SX_DEC_CTRL *sxDecCtrl, ST_INT16 *out_ptr)
{
	ST_INT i;
	ST_RET rc;

	if ((rc = sx_get_value (sxDecCtrl, "%d", &i)) == SD_SUCCESS)
		*out_ptr = (ST_INT16) i;
	return (rc);
}

/************************************************************************/
/*                       sx_get_uint16					*/
/************************************************************************/

ST_RET sx_get_uint16 (SX_DEC_CTRL *sxDecCtrl, ST_UINT16 *out_ptr)
{
	return (sx_get_value (sxDecCtrl, "%hu", out_ptr));
}

/************************************************************************/
/*                       sx_get_uint32					*/
/************************************************************************/

ST_RET sx_get_uint32 (SX_DEC_CTRL *sxDecCtrl, ST_UINT32 *out_ptr)
{
	ST_ULONG ul;
	ST_RET   rc;

	if ((rc = sx_get_value (sxDecCtrl, "%lu", &ul)) == SD_SUCCESS)
		*out_ptr = (ST_UINT32) ul;
	return (rc);
}

/************************************************************************/
/*                       sx_get_uint32_hex				*/
/************************************************************************/

ST_RET sx_get_uint32_hex (SX_DEC_CTRL *sxDecCtrl, ST_UINT32 *out_ptr)
{
	ST_ULONG ul;
	ST_RET   rc;

	if ((rc = sx_get_value (sxDecCtrl, "0x%lx", &ul)) == SD_SUCCESS)
		*out_ptr = (ST_UINT32) ul;
	return (rc);
}
/************************************************************************/
/*                       sx_get_int32					*/
/************************************************************************/

ST_RET sx_get_int32 (SX_DEC_CTRL *sxDecCtrl, ST_INT32 *out_ptr)
{
	ST_LONG sl;
	ST_RET  rc;

	if ((rc = sx_get_value (sxDecCtrl, "%ld", &sl)) == SD_SUCCESS)
		*out_ptr = (ST_INT32) sl;
	return (rc);
}

/************************************************************************/
/*                       sx_get_long					*/
/************************************************************************/

ST_RET sx_get_long (SX_DEC_CTRL *sxDecCtrl, ST_LONG *out_ptr)
{
	return (sx_get_value (sxDecCtrl, "%ld", out_ptr));
}

/************************************************************************/
/*                       sx_get_ulong					*/
/************************************************************************/

ST_RET sx_get_ulong (SX_DEC_CTRL *sxDecCtrl, ST_ULONG *out_ptr)
{
	return (sx_get_value (sxDecCtrl, "%lu", out_ptr));
}

/************************************************************************/
/*                       sx_get_uint					*/
/************************************************************************/

ST_RET sx_get_uint (SX_DEC_CTRL *sxDecCtrl, ST_UINT *out_ptr)
{
	return (sx_get_value (sxDecCtrl, "%u", out_ptr));
}

/************************************************************************/
/*                       sx_get_value					*/
/************************************************************************/

ST_RET sx_get_value (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *format_string, ST_VOID *out_ptr)
{
	ST_CHAR *str;
	ST_INT strLen;
	ST_RET rc;

	strLen = 0;
	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc != SD_SUCCESS)
		return (rc);
	SLOG_DEBUG("sx_get_value: %s, len %d", str, strLen);
	/* Convert to desired data format*/
	/* Note: sscanf may return 'bad' value if the number in the str	*/
	/*       exceeds the max value in format_string.		*/
	if (sscanf (str, format_string, out_ptr) != 1)
		/* we just want to get one value out of the string, any other ret is an error */
	{
		SLOG_ERROR ("Data Conversion Error for tag '%s'", sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (SD_FAILURE);
	}
	return (SD_SUCCESS);
}

/************************************************************************/
/*                       *sx_get_alloc_string				*/
/************************************************************************/

ST_RET sx_get_alloc_string (SX_DEC_CTRL *sxDecCtrl, ST_CHAR **strOut)
{
	ST_CHAR *str;
	ST_INT strLen;
	ST_RET rc;

	strLen = 0;
	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc != SD_SUCCESS)
		return (rc);

	*strOut = M_STRDUP(NULL,str);
	return (SD_SUCCESS);
}

/************************************************************************/
/*                       *sx_get_string					*/
/************************************************************************/

ST_RET sx_get_string (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *dest, ST_INT *lenOut)
{
	ST_CHAR *str;
	ST_INT strLen;
	ST_RET rc;

	strLen = 0;
	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc != SD_SUCCESS)
		return (rc);

	if (*lenOut != 0)
	{
		if (strLen > *lenOut)
		{
			SLOG_ERROR ("Error: String too long for tag '%s'", sxDecCtrl->sxDecElInfo.tag);
			sxDecCtrl->errCode = SD_FAILURE;
			return (SD_FAILURE);
		}
	}

	sx_format_string_dec (dest, str);
	*lenOut = strLen;
	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_format_string_dec			        */
/************************************************************************/

ST_VOID sx_format_string_dec (ST_CHAR *dest, ST_CHAR *src)
{
	ST_CHAR *srcPtr;
	ST_CHAR *srcPtrLast;
	ST_CHAR *dstPtr;
	ST_INT  diff;

	dstPtr = dest;
	srcPtrLast = src;
	srcPtr = strchr (src, '&');

	if (srcPtr == NULL)
	{
		strcpy (dest, src);
		return;
	}

	diff = srcPtr - srcPtrLast;
	strncpy (dstPtr, srcPtrLast, diff);
	dstPtr += diff;

	/* parse the source string and generate the dest string */
	while (srcPtr != NULL)
	{
		/* srcPtr points at a & */
		/* lets find out if the following characters are what we are looking for */
		if (strncmp (srcPtr, CODE_APOS, CODE_APOS_LEN) == 0)
		{
			*dstPtr++ = CHAR_APOS;
			srcPtr += CODE_APOS_LEN;
		}
		else if (strncmp (srcPtr, CODE_QUOT, CODE_QUOT_LEN) == 0)
		{
			*dstPtr++ = CHAR_QUOT;
			srcPtr += CODE_QUOT_LEN;
		}
		else if (strncmp (srcPtr, CODE_AMP, CODE_AMP_LEN) == 0)
		{
			*dstPtr++ = CHAR_AMP;
			srcPtr += CODE_AMP_LEN;
		}
		else if (strncmp (srcPtr, CODE_LT, CODE_LT_LEN) == 0)
		{
			*dstPtr++ = CHAR_LT;
			srcPtr += CODE_LT_LEN;
		}
		else if (strncmp (srcPtr, CODE_GT, CODE_GT_LEN) == 0)
		{
			*dstPtr++ = CHAR_GT;
			srcPtr += CODE_GT_LEN;
		}
		else
		{
			*dstPtr++ = '&';
			srcPtr++;
		}

		srcPtrLast = srcPtr;
		srcPtr = strchr (srcPtr, '&');

		if (srcPtr == NULL)
		{
			/* copy the remaining section of the string */
			strcpy (dstPtr, srcPtrLast);
		}
		else
		{
			diff = srcPtr - srcPtrLast;
			strncpy (dstPtr, srcPtrLast, diff);
			dstPtr += diff;
		}
	}
}


/************************************************************************/
/************************************************************************/
/************************************************************************/
/*                       sx_get_bool_attr				*/
/************************************************************************/

ST_RET sx_get_bool_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, ST_BOOLEAN *out_ptr)
{
	ST_INT d;
	ST_RET rc;

	rc = sx_get_attr_value (sxDecCtrl, name, "%d", &d);
	if (rc != SD_SUCCESS)
		return (rc);

	if (d == 0)
		*out_ptr = SD_FALSE;
	else
		*out_ptr = SD_TRUE;

	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_get_int_attr 				*/
/************************************************************************/

ST_RET sx_get_int_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, ST_INT *out_ptr)
{
	return (sx_get_attr_value (sxDecCtrl, name, "%d", out_ptr));
}

/************************************************************************/
/*                       sx_get_float_attr 				*/
/************************************************************************/

ST_RET sx_get_float_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, ST_FLOAT *out_ptr)
{
	return (sx_get_attr_value (sxDecCtrl, name, "%f", out_ptr));
}

/************************************************************************/
/*                       sx_get_double_attr 				*/
/************************************************************************/

ST_RET sx_get_double_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, ST_DOUBLE *out_ptr)
{
	return (sx_get_attr_value (sxDecCtrl, name, "%le", out_ptr));
}

/************************************************************************/
/*                       sx_get_uchar_attr 				*/
/************************************************************************/

ST_RET sx_get_uchar_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, ST_UCHAR *out_ptr)
{
	return (sx_get_attr_value (sxDecCtrl, name, "%c", out_ptr));
}

/************************************************************************/
/*                       sx_get_int16_attr 				*/
/************************************************************************/

ST_RET sx_get_int16_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, ST_INT16 *out_ptr)
{
	ST_INT i;
	ST_RET rc;

	if ((rc = sx_get_attr_value (sxDecCtrl, name, "%d", &i)) == SD_SUCCESS)
		*out_ptr = (ST_INT16) i;
	return (rc);
}

/************************************************************************/
/*                       sx_get_uint16_attr 				*/
/************************************************************************/

ST_RET sx_get_uint16_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, ST_UINT16 *out_ptr)
{
	ST_UINT i;
	ST_RET rc;

	if ((rc = sx_get_attr_value (sxDecCtrl, name, "%u", &i)) == SD_SUCCESS)
		*out_ptr = (ST_UINT16) i;
	return (rc);
}

/************************************************************************/
/*                       sx_get_uint32_attr				*/
/************************************************************************/

ST_RET sx_get_uint32_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, ST_UINT32 *out_ptr)
{
	ST_ULONG ul;
	ST_RET   rc;

	if ((rc = sx_get_attr_value (sxDecCtrl, name, "%lu", &ul)) == SD_SUCCESS)
		*out_ptr = (ST_UINT32) ul;
	return (rc);
}

/************************************************************************/
/*                       sx_get_int32_attr				*/
/************************************************************************/

ST_RET sx_get_int32_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, ST_INT32 *out_ptr)
{
	ST_LONG sl;
	ST_RET  rc;

	if ((rc = sx_get_attr_value (sxDecCtrl, name, "%ld", &sl)) == SD_SUCCESS)
		*out_ptr = (ST_INT32) sl;
	return (rc);
}

/************************************************************************/
/*                       sx_get_long_attr					*/
/************************************************************************/

ST_RET sx_get_long_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, ST_LONG *out_ptr)
{
	return (sx_get_attr_value (sxDecCtrl, name, "%ld", out_ptr));
}

/************************************************************************/
/*                       sx_get_ulong_attr				*/
/************************************************************************/

ST_RET sx_get_ulong_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, ST_ULONG *out_ptr)
{
	return (sx_get_attr_value (sxDecCtrl, name, "%lu", out_ptr));
}

/************************************************************************/
/*                       sx_get_uint_attr 				*/
/************************************************************************/

ST_RET sx_get_uint_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, ST_UINT *out_ptr)
{
	return (sx_get_attr_value (sxDecCtrl, name, "%u", out_ptr));
}

/************************************************************************/
/*                       XmlStringToDuration 				*/
/************************************************************************/

ST_RET XmlStringToDuration (ST_CHAR *src, SX_DURATION *out_ptr)
{
	ST_CHAR *p;
	ST_CHAR temp[50]; /* arbitrary length */
	ST_CHAR temp2[50]; /* arbitrary length */
	ST_BOOLEAN foundP = SD_FALSE;
	ST_BOOLEAN foundT = SD_FALSE;
	ST_BOOLEAN foundDot = SD_FALSE;
	ST_UINT i;
	ST_BOOLEAN something = SD_FALSE;  /* Make sure we got SOMETHING out of */
	/* this, any of the fields can be missing */

	out_ptr->inUse = SD_TRUE;
	p = src;
	memset (temp, 0, sizeof(temp));
	memset (temp2, 0, sizeof(temp2));
	for (i = 0; i < strlen(src); i++, p++)
	{
		/* check all error conditions I can think of */
		if ((*p == '-' && foundP) || 
			(isdigit(*p) && !foundP) ||
			(*p == 'Y' && !foundP) ||
			(*p == 'M' && !foundP) ||
			(*p == 'D' && !foundP) ||
			(!foundP && foundT) ||
			(*p == 'H' && !foundT) ||
			(*p == 'S' && !foundT) ||
			(*p != 'P' && *p != 'T' && isalpha(*p) && strlen(temp) == 0))
			return (SD_FAILURE);

		/* check for negative */
		else if (*p == '-')
			out_ptr->negative = SD_TRUE;
		else if (*p == '.')
			foundDot = SD_TRUE;
		/* We found a number */
		else if (isdigit(*p))
		{
			if (foundDot)
				strncat (temp2, p, 1);
			else
				strncat (temp, p, 1);
		}
		/* We found a letter */
		else if (isalpha(*p))
		{
			switch (*p)
			{
			case 'P':
				foundP = SD_TRUE;
				break;
			case 'T':
				foundT = SD_TRUE;
				break;
			case 'Y':
				sscanf(temp, "%d", &out_ptr->years);
				temp[0] = 0;
				something = SD_TRUE;
				break;
			case 'M':
				if (foundT)
					sscanf(temp, "%d", &out_ptr->minutes);
				else
					sscanf(temp, "%d", &out_ptr->months);
				temp[0] = 0;
				something = SD_TRUE;
				break;
			case 'D':
				sscanf(temp, "%d", &out_ptr->days);
				temp[0] = 0;
				something = SD_TRUE;
				break;
			case 'H':
				sscanf(temp, "%d", &out_ptr->hours);
				temp[0] = 0;
				something = SD_TRUE;
				break;
			case 'S':
				sscanf(temp, "%d", &out_ptr->seconds);
				temp[0] = 0;
				sscanf(temp2, "%ld", &out_ptr->microseconds);
				temp2[0] = 0;
				something = SD_TRUE;
				break;
			default:
				return (SD_FAILURE);
			}
		}
	}
	if (something == SD_FALSE)
		return (SD_FAILURE);

	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_get_duration_attr 				*/
/************************************************************************/

ST_RET sx_get_duration_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, SX_DURATION *out_ptr)
{
	ST_RET rc;
	ST_CHAR *sxDuration;

	memset (out_ptr, 0, sizeof (*out_ptr));

	rc = sx_get_attr_ptr (sxDecCtrl, &sxDuration, name);
	if (rc)
		return (rc);

	rc = XmlStringToDuration (sxDuration, out_ptr);
	if (rc != SD_SUCCESS)
	{
		SLOG_ERROR ("Error: String to Duration conversion for tag '%s'", sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (SD_FAILURE);
	}

	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_get_xtime_attr 				*/
/************************************************************************/

ST_RET sx_get_xtime_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, SX_DATE_TIME *out_ptr)
{
	ST_RET rc;
	ST_CHAR *sxDateTime;

	memset (out_ptr, 0, sizeof (*out_ptr));

	rc = sx_get_attr_ptr (sxDecCtrl, &sxDateTime, name);
	if (rc)
		return (rc);

	rc = XmlStringToUtcValue (sxDateTime, out_ptr);
	if (rc != SD_SUCCESS)
	{
		SLOG_ERROR ("Error: String to Value date/time conversion for tag '%s'", sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (rc);
	}

	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_get_tm_attr 				*/
/************************************************************************/

ST_RET sx_get_tm_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, struct tm *out_ptr)
{
	ST_RET rc;
	ST_CHAR *theTime;

	memset (out_ptr, 0, sizeof (*out_ptr));

	rc = sx_get_attr_ptr (sxDecCtrl, &theTime, name);
	if (rc)
		return (rc);

	rc = tstrStringToTm (theTime, out_ptr);
	if (rc != SD_SUCCESS)
	{
		SLOG_ERROR ("Error: String to Value (struct tm) date/time conversion for tag '%s'", sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (rc);
	}

	return (SD_SUCCESS);
}

/************************************************************************/
/*			sx_get_attr_value 				*/
/************************************************************************/

ST_RET sx_get_attr_value (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name, 
						  ST_CHAR *format_string, ST_VOID *out_ptr)
{
	ST_UINT i;

	for (i = 0; i < sxDecCtrl->sxDecElInfo.attrCount; ++i)
	{
		if (strcmp (sxDecCtrl->sxDecElInfo.attr[i].name, name) == 0)
		{
			/* Convert to desired data format*/
			if (!sscanf (sxDecCtrl->sxDecElInfo.attr[i].value, 
				format_string, out_ptr))
			{
				SLOG_ERROR ("Error: Attribute Data Conversion Error for tag '%s'", sxDecCtrl->sxDecElInfo.tag);
				sxDecCtrl->errCode = SD_FAILURE;
				return (SD_FAILURE);
			}
			return (SD_SUCCESS);
		}
	}
	return (SD_FAILURE);
}

/************************************************************************/
/*                       sx_get_attr					*/
/************************************************************************/

ST_RET sx_get_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *value, ST_CHAR *name)
{
	ST_UINT i;

	for (i = 0; i < sxDecCtrl->sxDecElInfo.attrCount; ++i)
	{
		if (strcmp (sxDecCtrl->sxDecElInfo.attr[i].name, name) == 0)
		{
			strcpy (value, sxDecCtrl->sxDecElInfo.attr[i].value);
			return (SD_SUCCESS);
		}
	}
	return (SD_FAILURE);
}

/************************************************************************/
/*                       sx_get_attr_ptr				*/
/************************************************************************/

ST_RET sx_get_attr_ptr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR **ptrOut, ST_CHAR *name)
{
	ST_UINT i;

	for (i = 0; i < sxDecCtrl->sxDecElInfo.attrCount; ++i)
	{
		if (strcmp (sxDecCtrl->sxDecElInfo.attr[i].name, name) == 0)
		{
			*ptrOut = sxDecCtrl->sxDecElInfo.attr[i].value;
			return (SD_SUCCESS);
		}
	}
	return (SD_FAILURE);
}

/************************************************************************/
/*                       *sx_get_bitstring				*/
/************************************************************************/

ST_RET sx_get_bitstring (SX_DEC_CTRL *sxDecCtrl, ST_INT *dest, ST_INT *lenOut)
{
	ST_CHAR *str;
	ST_INT strLen;
	ST_INT *bits;
	ST_CHAR *pStr;
	ST_RET rc;
	ST_INT i;

	strLen = 0;
	bits = dest; 
	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc != SD_SUCCESS)
		return (rc);

	if (*lenOut != 0)
	{
		if (strLen > *lenOut)
		{
			SLOG_ERROR ("Error: String too long for tag '%s'", sxDecCtrl->sxDecElInfo.tag);
			sxDecCtrl->errCode = SD_FAILURE;
			return (SD_FAILURE);
		}
	}

	pStr = str;
	pStr += strlen(str) - 1;
	for (i=0;i<(ST_INT)strlen(str);++i,--pStr)
	{
		if (strncmp(pStr, "0", 1) != 0 &&
			strncmp(pStr, "1", 1) != 0)
		{
			SLOG_ERROR ("Error: Not a bitstring for tag '%s'", sxDecCtrl->sxDecElInfo.tag);
			sxDecCtrl->errCode = SD_FAILURE;
			return (SD_FAILURE);
		}

		if (strncmp(pStr, "1", 1) == 0)
		{
			*bits |= (1 << i);
		}
	}

	*lenOut = strLen;
	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_get_string_ptr   				*/
/************************************************************************/

ST_RET sx_get_string_ptr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR **strOut, ST_INT *lenOut)
{
	ST_INT vLen;
	ST_RET rc;

	rc = sx_get_entity (sxDecCtrl, sxDecCtrl->elemBuf, sizeof(sxDecCtrl->elemBuf), &vLen);
	if (rc != SD_SUCCESS)
	{
		SLOG_ERROR ("Sx_get_entity failed");
		return (SD_FAILURE);
	}

	/* Convert to desired data format*/
	sxDecCtrl->elemBuf[vLen] = 0;
	*strOut = sxDecCtrl->elemBuf;
	*lenOut = vLen;
	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_get_string_YesNo   				*/
/*----------------------------------------------------------------------*/
/* This user helper function parses element value for the string	*/
/* "Yes"/"No" and sets the dest to SD_TRUE/SD_FALSE respectively.	*/
/* The string comparison is not case sensitive.				*/
/* Parameters:								*/
/*   SX_DEC_CTRL *sxDecCtrl	pointer to SX decoding control		*/
/*   ST_BOOLEAN  *dest		pointer to dest to SD_TRUE/SD_FALSE	*/
/*   ST_RET       errCode       SX err code to set in sxDecCtrl->errCode*/
/*			          in case of invalid element value,	*/
/*			        SX_ERR_CONVERT allows continue parsing.	*/
/* Return:								*/
/*	SD_SUCCESS		if function successful			*/
/*	SD_FAILURE		otherwise,and the sxDecCtrl->errCode is	*/
/*                              set to errCode.				*/
/************************************************************************/
ST_RET sx_get_string_YesNo (SX_DEC_CTRL *sxDecCtrl, ST_BOOLEAN *dest,
							ST_RET errCode)
{
	ST_RET   rc;
	ST_CHAR *str;
	ST_INT   strLen;

	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc == SD_SUCCESS)
	{
		if (stricmp (str, "Yes") == 0)
			*dest = SD_TRUE;
		else if (stricmp (str, "No") == 0)
			*dest = SD_FALSE;
		else
		{
			SLOG_ERROR ("SX DEC ERROR: invalid %s value '%s' (Yes/No expected)",
				sxDecCtrl->sxDecElInfo.tag, str);
			sxDecCtrl->errCode = errCode;
			rc = SD_FAILURE;
		}
	}

	return (rc);
}


/************************************************************************/
/*			sx_get_string_OnOff_mask			*/
/*----------------------------------------------------------------------*/
/* This user helper function parses element value for the string	*/
/* "On"/"Off" and sets/resets a maskBit in the mask.			*/
/* The string comparison is not case sensitive.				*/
/* Parameters:								*/
/*   SX_DEC_CTRL *sxDecCtrl	pointer to SX decoding control		*/
/*   ST_UINT     *mask		pointer to mask to set/reset a bit	*/
/*   ST_UINT      maskBit       mask bit to set/reset if ON/OFF	found	*/
/*   ST_RET       errCode       SX err code to set in sxDecCtrl->errCode*/
/*			          in case of invalid element value,	*/
/*			        SX_ERR_CONVERT allows continue parsing.	*/
/* Return:								*/
/*	SD_SUCCESS		if function successful			*/
/*	SD_FAILURE		otherwise,and the sxDecCtrl->errCode is	*/
/*                              set to errCode				*/
/************************************************************************/

ST_RET sx_get_string_OnOff_mask (SX_DEC_CTRL *sxDecCtrl, ST_UINT *mask,
								 ST_UINT maskBit, ST_RET errCode)
{
	ST_RET   rc;
	ST_CHAR *str;
	ST_INT   strLen;

	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc == SD_SUCCESS)
	{
		if (stricmp (str, "On") == 0)
		{
			*mask |= maskBit;		/* turn on this bit in the mask		*/
		}						       
		else if (stricmp (str, "Off") == 0)
		{						       
			*mask &= ~maskBit;	/* turn off this bit in the mask	*/
		}
		else
		{
			SLOG_ERROR ("SX DEC ERROR: invalid %s value '%s' (On/Off expected)",
				sxDecCtrl->sxDecElInfo.tag, str);
			sxDecCtrl->errCode = errCode;
			rc = SD_FAILURE;
		}
	}
	return (rc);
}

/************************************************************************/
/*			sx_get_string_OnOff_bool			*/
/*----------------------------------------------------------------------*/
/* This user helper function parses element value for the string	*/
/* "On"/"Off" and sets the dest to SD_TRUE/SD_FALSE respectively.	*/
/* The string comparison is not case sensitive.				*/
/* Parameters:								*/
/*   SX_DEC_CTRL *sxDecCtrl	pointer to SX decoding control		*/
/*   ST_BOOLEAN  *dest		pointer to dest to SD_TRUE/SD_FALSE	*/
/*   ST_RET       errCode       SX err code to set in sxDecCtrl->errCode*/
/*			          in case of invalid element value,	*/
/*			        SX_ERR_CONVERT allows continue parsing.	*/
/* Return:								*/
/*	SD_SUCCESS		if function successful			*/
/*	SD_FAILURE		otherwise,and the sxDecCtrl->errCode is	*/
/*                              set to errCode				*/
/************************************************************************/

ST_RET sx_get_string_OnOff_bool (SX_DEC_CTRL *sxDecCtrl, ST_BOOLEAN *dest,
								 ST_RET errCode)
{
	ST_RET   rc;
	ST_CHAR *str;
	ST_INT   strLen;

	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc == SD_SUCCESS)
	{
		if (stricmp (str, "On") == 0)
			*dest = SD_TRUE;
		else if (stricmp (str, "Off") == 0)
			*dest = SD_FALSE;
		else
		{
			SLOG_ERROR ("SX DEC ERROR: invalid %s value '%s' (On/Off expected)",
				sxDecCtrl->sxDecElInfo.tag, str);
			sxDecCtrl->errCode = errCode;
			rc = SD_FAILURE;
		}
	}
	return (rc);
}

/************************************************************************/
/*                       sx_get_time					*/
/************************************************************************/
ST_RET sx_get_time (SX_DEC_CTRL *sxDecCtrl, time_t *out_ptr)
{
	ST_CHAR *str;
	ST_RET rc;
	time_t t;
	ST_INT strLen;

	strLen = 0;
	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc != SD_SUCCESS)
		return (rc);


	rc = tstrStringToTime (str, &t);
	if (rc != SD_SUCCESS)
	{
		SLOG_ERROR ("Error: Time conversion (0x%04x) for tag '%s'", rc, sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (rc);
	}

	*out_ptr = t;
	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_get_tm					*/
/************************************************************************/
ST_RET sx_get_tm (SX_DEC_CTRL *sxDecCtrl, struct tm *out_ptr)
{
	ST_CHAR *str;
	ST_RET rc;
	ST_INT strLen;

	strLen = 0;
	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc != SD_SUCCESS)
		return (rc);

	rc = tstrStringToTm (str, out_ptr);
	if (rc != SD_SUCCESS)
	{
		SLOG_ERROR ("Error: Time conversion (0x%04x) for tag '%s'", rc, sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (rc);
	}

	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_get_tm_ex					*/
/************************************************************************/
ST_RET sx_get_tm_ex (SX_DEC_CTRL *sxDecCtrl, struct tm *out_ptr, ST_LONG *microseconds)
{
	ST_CHAR *str;
	ST_RET rc;
	ST_INT strLen;
	ST_CHAR *strMicro = NULL;	/* init to invalid value*/
	char *periodLoc;

	strLen = 0;
	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc != SD_SUCCESS)
		return (rc);

	periodLoc = strchr (str, '.');
	if (periodLoc != NULL)
	{
		if ((periodLoc + 1) == '\0')
		{
			SLOG_ERROR ("Error: Time conversion (0x%04x) for tag '%s'", rc, sxDecCtrl->sxDecElInfo.tag);
			sxDecCtrl->errCode = SD_FAILURE;
			return (rc);
		}
		*periodLoc = '\0';
		strMicro = periodLoc + 1;
	}

	rc = tstrStringToTm (str, out_ptr);
	if (rc != SD_SUCCESS)
	{
		SLOG_ERROR ("Error: Time conversion (0x%04x) for tag '%s'", rc, sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (rc);
	}

	if (strMicro != NULL)
	{
		while (strlen(strMicro) < 6)
			strcat (strMicro, "0");
		while (strlen(strMicro) > 6)
			strMicro[strlen(strMicro)-1] = '\0';
		*microseconds = atol(strMicro);
	}
	else
		*microseconds = 0;

	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_get_duration				*/
/************************************************************************/
/* Store duration string found in an xml file into the structure	*/
/* "SX_DURATION".                               			*/
/************************************************************************/

ST_RET sx_get_duration (SX_DEC_CTRL *sxDecCtrl, SX_DURATION *sxDuration)
{
	ST_CHAR *str;
	ST_RET rc;
	ST_INT strLen;

	memset (sxDuration, 0, sizeof (*sxDuration));

	strLen = 0;
	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc != SD_SUCCESS)
		return (rc);

	rc = XmlStringToDuration (str, sxDuration);
	if (rc != SD_SUCCESS)
	{
		SLOG_ERROR ("Error: String to Duration for tag '%s'", sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (rc);
	}

	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_get_xtime					*/
/************************************************************************/
/* Store date and time string found in an xml file into the structure   */
/* "SX_DATE_TIME" specifying the number of seconds from                 */
/* 1/1/1970 (UTC time), the number of microseconds in a decimal         */
/* fraction if it is specified and the number of minutes in the time    */
/* zone offset if it is specified.                                      */
/************************************************************************/

ST_RET sx_get_xtime (SX_DEC_CTRL *sxDecCtrl, SX_DATE_TIME *sxDateTime)
{
	ST_CHAR *str;
	ST_RET rc;
	ST_INT strLen;

	memset (sxDateTime, 0, sizeof (*sxDateTime));

	strLen = 0;
	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc != SD_SUCCESS)
		return (rc);

	/* convert input xml string to date and time */
	/* store data in "SX_DATE_TIME" structure    */
	rc = XmlStringToUtcValue (str, sxDateTime);
	if (rc != SD_SUCCESS)
	{
		SLOG_ERROR ("Error: String to Value date/time conversion for tag '%s'", sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (rc);
	}

	return (SD_SUCCESS);
}

/************************************************************************/
/*                       sx_get_time_ex					*/
/************************************************************************/
ST_RET sx_get_time_ex (SX_DEC_CTRL *sxDecCtrl, time_t *out_ptr, ST_LONG *microseconds)
{
	ST_CHAR *str;
	ST_CHAR strMicro[10];
	char *periodLoc;
	ST_RET rc;
	time_t t;
	ST_INT strLen;

	strLen = 0;
	rc = sx_get_string_ptr (sxDecCtrl, &str, &strLen);
	if (rc != SD_SUCCESS)
		return (rc);

	periodLoc = strchr (str, '.');
	if (periodLoc != NULL)
	{
		if ((periodLoc + 1) == '\0')
		{
			SLOG_ERROR ("Error: Time conversion (0x%04x) for tag '%s'", rc, sxDecCtrl->sxDecElInfo.tag);
			sxDecCtrl->errCode = SD_FAILURE;
			return (rc);
		}
		strcpy(strMicro, periodLoc + 1);
		*periodLoc = '\0';
	}

	rc = tstrStringToTime (str, &t);
	if (rc != SD_SUCCESS)
	{
		SLOG_ERROR ("Error: Time conversion (0x%04x) for tag '%s'", rc, sxDecCtrl->sxDecElInfo.tag);
		sxDecCtrl->errCode = SD_FAILURE;
		return (rc);
	}

	*out_ptr = t;
	if (periodLoc != NULL)
	{
		while (strlen(strMicro) < 6)
			strcat (strMicro, "0");
		while (strlen(strMicro) > 6)
			strMicro[strlen(strMicro)-1] = '\0';
		*microseconds = atol(strMicro);
	}
	else
		*microseconds = 0;

	return (SD_SUCCESS);
}

/************************************************************************/
/************************************************************************/
#define MAX_BYTES_IN_CHUNK  (2*SX_MAX_ELEM_LEN)	/* MUST BE EVEN */


#define SX_LOAD_CHAR(a)  do {\
	if (sxDecCtrl->useFp == SD_TRUE)\
	{\
		sx_load_characters (lineBuf, &eof, sxDecCtrl->fp, a);\
		sxDecElInfo->entityStart-= a;\
		sxDecElInfo->entityEnd-= a;\
	}\
	else\
	{\
		xml+= a;\
		if( ( *xml == '\n' ) && (a != 0))\
		{\
			g_rowIndex++;\
		}\
	}\
}while(0)

//termFlag 提前终止标识符
#define SX_RIP_NOT_DONE (!sxDecCtrl->termFlag && (((sxDecCtrl->useFp == SD_FALSE) && (xml < xmlEnd)) || ((sxDecCtrl->useFp == SD_TRUE) && (*xml != '\0'))))

#define SX_RIP_DONE (sxDecCtrl->termFlag || ((sxDecCtrl->useFp == SD_FALSE) && (xml >= xmlEnd)) || ((sxDecCtrl->useFp == SD_TRUE) && (*xml == '\0')))


#if !defined(USE_EXPAT)
/************************************************************************/
/*			sx_rip_xml 					*/
/************************************************************************/



ST_RET sx_rip_xml (SX_DEC_CTRL *sxDecCtrl)
{
	SX_DEC_ELEMENT_INFO *sxDecElInfo;
	ST_CHAR *xml;
	ST_INT  nestLevel;
	ST_CHAR *tagDest;
	ST_CHAR *attribNameDest;
	ST_CHAR *attribValDest;
	ST_CHAR attribValCopy[SX_MAX_ATTR_VALUE];
	ST_INT len;
	ST_INT midBuff;
	ST_INT numChRead;
	ST_LONG xmlLen;
	ST_CHAR *xmlEnd = NULL;		/* init to avoid compiler warning	*/
	ST_CHAR c;
	ST_BOOLEAN bEmptyTag;
	ST_BOOLEAN eof;
	ST_BOOLEAN sawStartTag = SD_FALSE;
	ST_CHAR lineBuf[MAX_BYTES_IN_CHUNK];

	g_rowIndex = 0;
	sxDecElInfo = &sxDecCtrl->sxDecElInfo;

	bEmptyTag = SD_FALSE;

	if (sxDecCtrl->useFp == SD_TRUE)
	{
		memset (lineBuf, 0, MAX_BYTES_IN_CHUNK);
		eof = SD_FALSE;
		midBuff = MAX_BYTES_IN_CHUNK / 2;

		/* get the first (MAX_BTYES_IN_CHUNK / 2) chunk from the file */
		numChRead = fread(lineBuf + midBuff, 1, MAX_BYTES_IN_CHUNK - midBuff, sxDecCtrl->fp);
		if (numChRead > 0)
			xml = lineBuf + midBuff;
		else
		{
			SLOG_ERROR ("SX decode error: could not read xml from file");
			sxDecCtrl->errCode = SD_FAILURE;
			return (SD_FAILURE);
		}
	}
	else
	{
		sxDecElInfo = &sxDecCtrl->sxDecElInfo;
		xml = sxDecCtrl->xmlStart;
		xmlLen = sxDecCtrl->xmlLen;
		xmlEnd = xml + xmlLen;
	}
	
	while (SX_RIP_NOT_DONE)
	{		
		/* Find a begin or end tag */
		while (*xml != '<' && SX_RIP_NOT_DONE)
			SX_LOAD_CHAR (1);

		if (SX_RIP_DONE)
			break;

		sawStartTag = SD_TRUE;
		sxDecElInfo->tagStart = xml;
		
		/* OK, this should be the start of a start tag, an end tag, or a comment */
		/* or block of binary CDATA						 */
		/* < next  */
		SX_LOAD_CHAR (1);

		if (*xml == '?') /* title */
		{
			while (strncmp (xml, "?>", 2)  != 0 &&
				SX_RIP_NOT_DONE)
			{
				SX_LOAD_CHAR (1);
			}
		}
		else if (strncmp (xml, "!--", 3) == 0)/* Comment */
		{
			while (strncmp (xml, "-->", 3) != 0 && 
				SX_RIP_NOT_DONE)
			{
				SX_LOAD_CHAR (1);
			}
		}
		else if (strncmp (xml, "![CDATA[", 8) == 0) /* CDATA */
		{
			while (strncmp (xml, "]]>", 3) != 0 &&
				SX_RIP_NOT_DONE)
			{
				SX_LOAD_CHAR (1);
			}
		}
		else if (strncmp (xml, "!DOCTYPE", 8) == 0) /* DocType */
		{
			nestLevel = 0;
			while (SD_TRUE)
			{
				if (*xml == '>')
				{
					if (nestLevel == 0)
						break;
					else
						--nestLevel;
				}
				if (*xml == '<')
					++nestLevel;

				SX_LOAD_CHAR (1);
				if (SX_RIP_DONE)
				{
					SLOG_ERROR ("Row = %lu, SX decode error: could not find DOCTYPE end",g_rowIndex);
					sxDecCtrl->errCode = SD_FAILURE;
					return (SD_FAILURE);
				}
			}
			SX_LOAD_CHAR (1);
		}
		else if (*xml != '/')	/* Begin tag */
		{
			/* We have a element tag start, get the tag  first  */
			memset(sxDecElInfo->tag, 0 , SX_MAX_TAG_LEN);
			tagDest = sxDecElInfo->tag;	//当前tag指针

			len = 0;
			/* 获取 每一个数据块的tag <SCL ****>*/
			while (SX_RIP_NOT_DONE)
			{
				c = *xml;
				if (c == '>' || c == ' ' || c == '/' || c == 10 || c == 9 || c == 13) 	/* Found the end of the tag */
					break;

				*(tagDest++) = c;	//获取tag 内容
				++len;
				if (len >= SX_MAX_TAG_LEN)
				{
					SLOG_ERROR("Row = %lu, SX decode error: tag too long",g_rowIndex);
					sxDecCtrl->errCode = SD_FAILURE;
					return (SD_FAILURE);
				}
				SX_LOAD_CHAR (1);	//ptr xml++
				if (sxDecCtrl->ignoreNS && c == ':')
				{
					tagDest = sxDecElInfo->tag;
					len = 0;
				}
				
			}
			SLOG_DEBUG ("=========================tag start: %s=========================", sxDecElInfo->tag);
			
			//if not found
			if (SX_RIP_DONE)
			{
				SLOG_ERROR ("Row = %lu, SX decode error: could not find tag end",g_rowIndex);
				sxDecCtrl->errCode = SD_FAILURE;
				return (SD_FAILURE);
			}
			*tagDest = 0;		/* terminate the tag */

			/* Now look for attributes */         
			sxDecElInfo->attrCount = 0;
			//遍历和统计Attributes
			while (*xml != '>' && *xml != '/') /* we could have attributes! */
			{
				/* skip any whitespace before the start of the attribute name */
				while (*xml == ' ' && SX_RIP_NOT_DONE)
					SX_LOAD_CHAR (1);

				if (SX_RIP_DONE)
				{
					SLOG_ERROR ("Row = %lu, SX decode error: could not find attribute name",g_rowIndex);
					sxDecCtrl->errCode = SD_FAILURE;
					return (SD_FAILURE);
				}

				
				//从行首遍历到行尾
				if (*xml != '>' && *xml != '/')
				{
					
					if (sxDecElInfo->attrCount >= SX_MAX_ATTRIB)
					{
						SLOG_ERROR ("Row = %lu, SX decode error: too many attributes. SX_MAX_ATTRIB define is %d",g_rowIndex, SX_MAX_ATTRIB);
						sxDecCtrl->errCode = SD_FAILURE;
						return (SD_FAILURE);
					}
					//bug fix,解决name和value有残留的问题
					memset(sxDecElInfo->attr[sxDecElInfo->attrCount].name, 0, SX_MAX_ATTR_NAME);
					memset(sxDecElInfo->attr[sxDecElInfo->attrCount].value, 0, SX_MAX_ATTR_VALUE);					
					/* This should be the start of an attribute name */ 
					attribNameDest = sxDecElInfo->attr[sxDecElInfo->attrCount].name;
					len = 0;

					//1 get attribute name
					while (SX_RIP_NOT_DONE)
					{
						c = *xml;
						if (c == '=' || c == ' ')	/* Found the end of the name */
							break;

						if (c == '>') /* this isn't really an attribute like we first thought */
							break;

						if (c != 9 && c != 10 && c != 13) /* dont include these characters */
						{
							*(attribNameDest++) = c;
							++len;
						}

						if (len >= SX_MAX_ATTR_NAME)
						{
							SLOG_ERROR ("Row = %lu, SX decode error: attribute name too long",g_rowIndex);
							sxDecCtrl->errCode = SD_FAILURE;
							return (SD_FAILURE);
						}
						SX_LOAD_CHAR (1);
						if (sxDecCtrl->ignoreNS && c == ':')
						{
							attribNameDest = sxDecElInfo->attr[sxDecElInfo->attrCount].name;
							len = 0;
						}
					}

					SLOG_DEBUG ("sxDecElInfo->attr[%d].name: %s", sxDecElInfo->attrCount, sxDecElInfo->attr[sxDecElInfo->attrCount].name);

					if (SX_RIP_DONE)
					{
						SLOG_ERROR ("Row = %lu, SX decode error: could not find attribute name end",g_rowIndex);
						sxDecCtrl->errCode = SD_FAILURE;
						return (SD_FAILURE);
					}

					if (*xml == '>') /* this isn't really an attribute like we first thought */
						break;

					*attribNameDest = 0;		/* terminate the attrib name */

					/* skip to the attribute '=' */
					while (*xml != '=' && SX_RIP_NOT_DONE)
						SX_LOAD_CHAR (1);

					if (SX_RIP_DONE)
					{
						SLOG_ERROR ("Row = %lu, SX decode error: could not find attribute '='",g_rowIndex);
						sxDecCtrl->errCode = SD_FAILURE;
						return (SD_FAILURE);
					}
					SX_LOAD_CHAR (1);

					/* skip white space */
					while ((*xml == ' ' || *xml == '\t') && SX_RIP_NOT_DONE)
						SX_LOAD_CHAR (1);

					/* OK, get the attrib value */
					if (*(xml) != '"' && *(xml) != '\'')		/* skip the opening " */
					{
						SLOG_ERROR ("Row = %lu, SX decode error: could not find leading attribute value '\"'",g_rowIndex);
						sxDecCtrl->errCode = SD_FAILURE;
						return (SD_FAILURE);
					}

					SX_LOAD_CHAR (1);

					/*2  At the start of the attribute value */ 
					attribValDest = sxDecElInfo->attr[sxDecElInfo->attrCount].value;
					len = 0;
					while (SX_RIP_NOT_DONE)
					{
						c = *xml;
						if (c == '"' || c == '\'')	/* Found the end of the attrib */
							break;

						if (c != 9 && c != 10 && c != 13) /* dont include these characters */
						{
							*(attribValDest++) = c;
							++len;
						}

						if (len >= SX_MAX_ATTR_VALUE)
						{
							SLOG_ERROR ("Row = %lu, SX decode error: attribute value too long",g_rowIndex);
							sxDecCtrl->errCode = SD_FAILURE;
							return (SD_FAILURE);
						}
						SX_LOAD_CHAR (1);
					}

					if (SX_RIP_DONE)
					{
						SLOG_ERROR ("Row = %lu, SX decode error: could not find closing attribute value '\"'",g_rowIndex);
						sxDecCtrl->errCode = SD_FAILURE;
						return (SD_FAILURE);
					}

					if (*(xml) != '"' && *(xml) != '\'')		/* skip the closing " */
					{
						SLOG_ERROR ("Row = %lu, SX decode error: could not find closing attribute value '\"'",g_rowIndex);
						sxDecCtrl->errCode = SD_FAILURE;
						return (SD_FAILURE);
					}
					SX_LOAD_CHAR (1);

					*attribValDest = 0;	/* terminate the attrib value */
					
					SLOG_DEBUG ("sxDecElInfo->attr[%d].value: %s", sxDecElInfo->attrCount, sxDecElInfo->attr[sxDecElInfo->attrCount].value);					

					strcpy (attribValCopy, sxDecElInfo->attr[sxDecElInfo->attrCount].value);
					//删除引用符号
					sx_format_string_dec (sxDecElInfo->attr[sxDecElInfo->attrCount].value, attribValCopy);
					
					++sxDecElInfo->attrCount;
				}

				// < > 行结束
				if (SX_RIP_DONE)
				{
					SLOG_ERROR ("Row = %lu, SX decode error: could not find tag end",g_rowIndex);
					sxDecCtrl->errCode = SD_FAILURE;
					return (SD_FAILURE);
				}
			} // end of while (*xml != '>' && *xml != '/') 一行结束

			/* Could be empty tag */
			if (*xml == '/')
			{
				bEmptyTag = SD_TRUE;
				while (*xml != '>')
					SX_LOAD_CHAR (1);
			}
			SX_LOAD_CHAR (1); /* skip the '>' */

			/*3  OK, now call the element start function  读取下一层次Element的内容*/
			sxDecElInfo->entityStart = xml;
			sxDecElInfo->entityEnd = xml;
			sxDecCtrl->xmlPos  = xml;	/* Save current dec position 	*/

			//读取下一层数据
			sxStartElement (sxDecCtrl); 
			/* Fail on any error except convert error */
			if (sxDecCtrl->errCode != SD_SUCCESS && sxDecCtrl->errCode != SX_ERR_CONVERT)
			{
				return (SD_FAILURE);
			}

			if (bEmptyTag)
			{
				/* OK, now call the element end function */
				sxEndElement (sxDecCtrl);
				bEmptyTag = SD_FALSE;
			}
			SX_LOAD_CHAR (sxDecCtrl->xmlPos - xml);	//计算这个entity经历了多少指针
			sxDecCtrl->xmlPos  = xml;	/* Save current dec position 	*/

		}
		else		/* End tag */
		{
			//这个函数在最后运行
			sxDecElInfo->entityEnd = xml - 1;
			SX_LOAD_CHAR (1);
			//clear tag array
			memset(sxDecElInfo->tag, 0 , SX_MAX_TAG_LEN);
			tagDest = sxDecElInfo->tag;
	
			len = 0;
			while (SX_RIP_NOT_DONE)
			{
				c = *xml;
				if (c == '>' || c == ' ')	/* Found the end of the tag */
					break;

				*(tagDest++) = c;
				++len;
				if (len >= SX_MAX_TAG_LEN)
				{
					SLOG_ERROR ("Row = %lu, SX decode error: tag too long",g_rowIndex);
					sxDecCtrl->errCode = SD_FAILURE;
					return (SD_FAILURE);
				}
				SX_LOAD_CHAR (1);
				if (sxDecCtrl->ignoreNS && c == ':')
				{
					tagDest = sxDecElInfo->tag;
					len = 0;
				}
			}
			
			//判断文件是否读完
			if (SX_RIP_DONE)
			{
				SLOG_ERROR ("Row = %lu, SX decode error: could not find tag end",g_rowIndex);
				sxDecCtrl->errCode = SD_FAILURE;
				return (SD_FAILURE);
			}
			*tagDest = 0;		/* terminate the tag */

			SLOG_DEBUG ("=========================tag end: %s=========================", sxDecElInfo->tag);

			/* any white space up to the end of the tag name */
			while (*xml != '>' && SX_RIP_NOT_DONE)
				SX_LOAD_CHAR (1);

			if (SX_RIP_DONE)
			{
				SLOG_ERROR ("Row = %lu, SX decode error: could not find tag end",g_rowIndex);
				sxDecCtrl->errCode = SD_FAILURE;
				return (SD_FAILURE);
			}
			SX_LOAD_CHAR (1);

			sxDecCtrl->xmlPos  = xml;	/* Save current dec position 	*/

			/* OK, now call the element end function */
			sxEndElement (sxDecCtrl);
			/* Fail on any error except convert error */
			if (sxDecCtrl->errCode != SD_SUCCESS && sxDecCtrl->errCode != SX_ERR_CONVERT)
			{
				return (SD_FAILURE);
			}
			
		}

	
		/* for debug*/
		if (sxDecCtrl->itemStackLevel > 5)  {
			pause();
		}	
	}

	if (!sawStartTag)
	{
		SLOG_ERROR ("SX decode error: could not find start tag");
		sxDecCtrl->errCode = SD_FAILURE;
		return (SD_FAILURE);
	}
	else
		return (SD_SUCCESS);
}

ST_VOID sx_load_characters (ST_CHAR *lineBuf, ST_BOOLEAN *eof, FILE *fp, ST_INT numToRead)
{
	ST_LONG numChRead;
	ST_LONG i;

	if (numToRead > 0)
	{
		/* first shift the characters by numToRead */
		memmove (lineBuf, &lineBuf[numToRead], MAX_BYTES_IN_CHUNK - numToRead);

		if (*eof != SD_TRUE)
		{
			numChRead = fread(lineBuf + (MAX_BYTES_IN_CHUNK - numToRead), 1, numToRead, fp);
			if (numChRead == 0)
			{
				*eof = SD_TRUE;
				for (i = 1; i < numToRead + 1; i++)
				{
					lineBuf[MAX_BYTES_IN_CHUNK - i] = '\0';
				}
			}
		}
	}
}
#else	/* USE_EXPAT	*/


/************************************************************************/
/*			expatHandlerStart				*/
/* Normal 'start tag' handler.						*/
/* Log and set "errCode" if any string is too long to be stored.	*/
/************************************************************************/
static void XMLCALL expatHandlerStart(void *userData, const char *el, const char **attr)
{
	SX_DEC_CTRL *sxDecCtrl = (SX_DEC_CTRL *) userData;
	int i;
	const ST_CHAR *ptr;

	if (!sxDecCtrl->termFlag)
	{
		/* Copy tag to sxDecCtrl	*/
		/* If ignoreNS flag set, ignore namespace prefix on tag.	*/
		if (sxDecCtrl->ignoreNS)
		{
			/* If ':' in the string, copy only text after ':'	*/
			if ((ptr = strrchr (el, ':')) != NULL)
				ptr += 1;	/* point after ':'	*/
			else
				ptr = el;
		}
		else
			ptr = el;

		if (strlen (ptr) < SX_MAX_TAG_LEN)
			strcpy (sxDecCtrl->sxDecElInfo.tag, ptr);
		else
		{
			SXLOG_ERR1 ("start tag '%s' too long. Can't be stored.", ptr);
			sxDecCtrl->errCode = SD_FAILURE;
		}

		/* Copy attributes to sxDecCtrl	*/
		for (i = 0; attr[i]; i += 2)
		{
			if (strlen (attr[i]) < SX_MAX_ATTR_NAME)
				strcpy (sxDecCtrl->sxDecElInfo.attr[i/2].name, attr[i]);
			else
			{
				SXLOG_ERR1 ("attr name '%s' too long. Can't be stored.", ptr);
				sxDecCtrl->errCode = SD_FAILURE;
			}

			if (strlen (attr[i+1]) < SX_MAX_ATTR_VALUE)
				strcpy (sxDecCtrl->sxDecElInfo.attr[i/2].value, attr[i+1]);
			else
			{
				SXLOG_ERR1 ("attr value '%s' too long. Can't be stored.", ptr);
				sxDecCtrl->errCode = SD_FAILURE;
			}
		}
		sxDecCtrl->sxDecElInfo.attrCount = i/2;
		sxDecCtrl->entityLen = 0;	/* reset entityLen	*/
		sxStartElement (sxDecCtrl);
	}	/* !termFlag	*/
}

/************************************************************************/
/*			expatHandlerEnd					*/
/* Normal 'end tag' handler.						*/
/************************************************************************/
static void XMLCALL expatHandlerEnd(void *userData, const char *el)
{
	SX_DEC_CTRL *sxDecCtrl = (SX_DEC_CTRL *) userData;

	if (!sxDecCtrl->termFlag)
	{
		strcpy (sxDecCtrl->sxDecElInfo.tag, el);
		sxEndElement (sxDecCtrl);
		sxDecCtrl->entityLen = 0;	/* reset entityLen	*/
	}	/* !termFlag	*/
}

/************************************************************************/
/*			expatHandlerData				*/
/* Normal data handler.							*/
/* Log and set "errCode" if data is too long to be stored.		*/
/************************************************************************/
static void XMLCALL expatHandlerData(void *userData,
									 const XML_Char *s,
									 int len)
{
	SX_DEC_CTRL *sxDecCtrl = (SX_DEC_CTRL *) userData;
	ST_INT sizeNeeded;	/* buffer size needed to store this data	*/

	if (!sxDecCtrl->termFlag)
	{
		sizeNeeded = len + sxDecCtrl->entityLen;

		/* Save data to access with "sx_get_entity" later.	*/
		if (sizeNeeded > sxDecCtrl->entityBufSize)
		{
			/* Reallocate buffer twice as big as currently needed.	*/
			sxDecCtrl->entityBufSize = sizeNeeded * 2;
			SLOG_WARN ("entity buffer too small. Reallocating entity buffer size = %d", sxDecCtrl->entityBufSize);
			sxDecCtrl->entityBuf = chk_realloc (sxDecCtrl->entityBuf, sxDecCtrl->entityBufSize);
		}

		memcpy (&(sxDecCtrl->entityBuf [sxDecCtrl->entityLen]), s, len);
		sxDecCtrl->entityLen += len;
	}	/* !termFlag	*/
}

/************************************************************************/
/*			expatHandlerStartSkip				*/
/* The 'start tag' handler when tags are being skipped.			*/
/************************************************************************/
static void XMLCALL expatHandlerStartSkip(void *userData, const char *el, const char **attr)
{
	SX_DEC_CTRL *sxDecCtrl = (SX_DEC_CTRL *) userData;

	if (!sxDecCtrl->termFlag)
	{
		/* Not calling normal start funct, so must increment nest level here*/
		sxDecCtrl->xmlNestLevel++;
	}	/* !termFlag	*/
}

/************************************************************************/
/*			expatHandlerEndSkip				*/
/* The 'end tag' handler when tags are being skipped.			*/
/************************************************************************/
static void XMLCALL expatHandlerEndSkip(void *userData, const char *el)
{
	SX_DEC_CTRL *sxDecCtrl = (SX_DEC_CTRL *) userData;

	if (!sxDecCtrl->termFlag)
	{
		if (sxDecCtrl->xmlNestLevel == sxDecCtrl->skipNestLevel)
		{
			SLOG_DEBUG ("End element   '%s' (contents skipped)", el);
			/* Go back to normal handlers.	*/
			XML_SetCharacterDataHandler(sxDecCtrl->parser, expatHandlerData);
			XML_SetElementHandler(sxDecCtrl->parser, expatHandlerStart, expatHandlerEnd);
		}

		/* Not calling normal end funct, so must decrement nest level here*/
		sxDecCtrl->xmlNestLevel--;
	}	/* !termFlag	*/
}
/************************************************************************/
/*			setup_expat					*/
/* NOTE: If "sxDecCtrl->termFlag" is set by user (i.e. error found),	*/
/*       let parse complete, but ignore data (see "expatHandler*").	*/
/************************************************************************/
static XML_Parser setup_expat(SX_DEC_CTRL *sxDecCtrl)
{
	XML_Parser parser = XML_ParserCreate(NULL);
	if (! parser)
	{
		SLOG_ERROR ("Couldn't allocate memory for XML parser");
		return (parser);
	}
	XML_SetUserData (parser, sxDecCtrl);	/* passes sxDecCtrl to handlers	*/
	XML_SetCharacterDataHandler(parser, expatHandlerData);
	XML_SetElementHandler(parser, expatHandlerStart, expatHandlerEnd);
	sxDecCtrl->parser = parser;	/* CRITICAL: save parser to use in callbacks*/

	return (parser);
}
/************************************************************************/
/*			sx_rip_xml_file					*/
/* Parse XML from a file. The caller must open a file			*/
/* and store the file ptr in "sxDecCtrl->fp". It reads one segment	*/
/* at a time from the file and passes it to XML_Parse.			*/
/************************************************************************/
ST_RET sx_rip_xml_file (SX_DEC_CTRL *sxDecCtrl)
{
	XML_Parser parser;		/* Expat parser control structure	*/
	ST_RET retcode = SD_SUCCESS;
	char *parseBuf;	/* temporary buffer to store text read from input file	*/

	if (!(parser = setup_expat (sxDecCtrl)))
		retcode = SD_FAILURE;	/* error already logged	in setup_expat	*/
	else
	{
		/* Allocate a reasonable size entity buffer.		*/
		/* This may be reallocated later if it is too small.	*/
		sxDecCtrl->entityBufSize = ENTITY_BUF_SIZE;
		sxDecCtrl->entityBuf = chk_malloc (sxDecCtrl->entityBufSize);

		parseBuf = chk_malloc (EXPAT_BUF_SIZE);

		/* In a loop, read the file one segment at a time & pass to parser.	*/
		for (;;)
		{
			int done;
			int len;

			len = fread(parseBuf, 1, EXPAT_BUF_SIZE, sxDecCtrl->fp);
			if (ferror(sxDecCtrl->fp))
			{
				fprintf(stderr, "Read error\n");
				retcode = SD_FAILURE;
				break;	/* stop now	*/
			}
			done = feof(sxDecCtrl->fp);

			if (XML_Parse(parser, parseBuf, len, done) == XML_STATUS_ERROR)
			{
				SLOG_ERROR ("XML parse error at line %d: %s\n",
					XML_GetCurrentLineNumber(parser),
					XML_ErrorString(XML_GetErrorCode(parser)));
				retcode = SD_FAILURE;
				break;	/* stop now	*/
			}

			if (done)
				break;
		}	/* end main loop	*/
		chk_free (parseBuf);
		chk_free (sxDecCtrl->entityBuf);
	}
	return (retcode);
}
/************************************************************************/
/*			sx_rip_xml_mem					*/
/* Parse XML from a memory buffer. The caller must set			*/
/* the following members of sxDecCtrl before calling this function:	*/
/*   sxDecCtrl->xmlStart points to the buffer.				*/
/*   sxDecCtrl->xmlLen contains the length of the buffer.		*/
/* It simply sets up the parse and passes the buffer to XML_Parse.	*/
/************************************************************************/
ST_RET sx_rip_xml_mem (SX_DEC_CTRL *sxDecCtrl)
{
	XML_Parser parser;		/* Expat parser control structure	*/
	int done = SD_TRUE;	/* indicates to parser that all data is being passed*/
	ST_RET retcode = SD_SUCCESS;

	if (!(parser = setup_expat (sxDecCtrl)))
		retcode = SD_FAILURE;	/* error already logged	in setup_expat	*/
	else
	{
		/* Allocate a reasonable size entity buffer.		*/
		/* This may be reallocated later if it is too small.	*/
		sxDecCtrl->entityBufSize = ENTITY_BUF_SIZE;
		sxDecCtrl->entityBuf = chk_malloc (sxDecCtrl->entityBufSize);

		/* Just pass data all at once to XML_Parse.	*/
		if (XML_Parse(parser, sxDecCtrl->xmlStart, sxDecCtrl->xmlLen, done) == XML_STATUS_ERROR)
		{
			SLOG_ERROR ("XML parse error at line %d: %s\n",
				XML_GetCurrentLineNumber(parser),
				XML_ErrorString(XML_GetErrorCode(parser)));
			retcode = SD_FAILURE;
		}
		chk_free (sxDecCtrl->entityBuf);
	}

	return (retcode);
}
#endif	/* USE_EXPAT	*/