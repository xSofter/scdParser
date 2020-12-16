/*
 * @Date: 2020-12-07 09:25
 * @LastEditTime: 2020-12-15 19:09
 * @LastEditors: tangkai3
 * @Description: 模块对外接口函数
 */

#include "sclPub.h"
#include "str_util.h"

#define LOG_FILE_NAME "scdpase.log"

#ifdef DEBUG_SISCO
static const ST_CHAR *thisFileName = __FILE__;
#endif
/**
 * @Description: 加载scl文件信息,加载完成后,释放log
 */
ST_RET load_scd_file(const char* fileName, SCL_INFO* sclInfo)
{
	// ST_CHAR *xmlFileName = "TXJD01.cid";//"jilindong.scd"; //"TXJD01.cid";
	ST_CHAR *iedName = "C5011";
	ST_CHAR *accessPointName = "S1";
	ST_RET rc = 0;

	slog_start(SX_LOG_ALWAY, LOG_FILE_EN, LOG_FILE_NAME); //SX_LOG_ERROR  SX_LOG_ALWAY
	rc = scl_parse(fileName, iedName, accessPointName, sclInfo);
	slog_end();

	return rc;
}

/**
 * @Description: 释放scl文件解析的内容
 */
void release_scd_file(SCL_INFO* sclInfo)
{
	if (!sclInfo) return;
	scl_info_destroy(sclInfo);
}


/**
 * @Description: 释放用户信息的内容
 */
void release_scd_userInfo(SCL_USER* userInfo) 
{
	if (!userInfo) return;
	SCL_COMM* comm;
	SCL_GSE*  gseHead;      /* head of list of GSE defs	*/
	SCL_SMV*  smvHead;      /* head of list of SMV defs	*/
	SCL_PORT* portHead;		/* head of list of */	
	SCL_DATASET *dataSet;
	SCL_LNINFO* lnInfo;
	while ((comm = (SCL_COMM *) list_get_first (&userInfo->pCommHead)) != NULL)
	{	
		while ((gseHead = (SCL_GSE *) list_get_first(&comm->gseHead)) != NULL)
		{
			chk_free(gseHead);
		}
		while ((smvHead = (SCL_SMV *) list_get_first(&comm->smvHead)) != NULL)
		{
			chk_free(smvHead);
		}
		while ((portHead = (SCL_PORT *) list_get_first(&comm->portHead)) != NULL)
		{
			chk_free(portHead);
		}	
		chk_free (comm);
	}	

	while ((dataSet = (SCL_DATASET *) list_get_first (&userInfo->pDataSet)) != NULL)
	{	
		SCL_FCDA* fcda;
		while ((fcda = (SCL_FCDA *) list_get_first (&dataSet->fcdaHead)) != NULL)
		{	
			chk_free (fcda);
		}
		if (dataSet->desc)
			chk_free (dataSet->desc);
		chk_free (dataSet);
	}		

	while ((lnInfo = (SCL_LNINFO *) list_get_first (&userInfo->pLnInfo)) != NULL)
	{	
		if (lnInfo->lnDesc)
			chk_free (lnInfo->lnDesc);
		if (lnInfo->doiDesc)
			chk_free (lnInfo->doiDesc);
		if (lnInfo->daiDesc)
			chk_free (lnInfo->daiDesc);
		if (lnInfo->daiVal)
			chk_free (lnInfo->daiVal);			
		if (lnInfo->ref)
			chk_free (lnInfo->ref);									
		chk_free (lnInfo);
	}	
}

/**
 * @Description: create comm_list
 */
SCL_COMM *user_comm_create (SCL_USER* usr)	
{
	SCL_COMM *scl_user_comm = NULL;	/* assume failure	*/
	if (!usr) {
		SLOG_ERROR ("NULL usr pointer");
		return NULL;
	}
	scl_user_comm = (SCL_COMM *) chk_calloc (1, sizeof (SCL_COMM));
	/* Add LD to front of LD List.	*/
	list_add_first (&usr->pCommHead, scl_user_comm);

	return (scl_user_comm);
}

/**
 * @Description: 添加GSE头
 */
SCL_GSE* user_comm_gse_add(SCL_USER* usr)
{
	SCL_GSE *user_gse = NULL;	/* assume failure	*/
	if (!usr->pCommHead) {
		SLOG_ERROR ("NULL user commhead");
		return NULL;
	}
	user_gse = (SCL_GSE *) chk_calloc (1, sizeof (SCL_GSE));
	list_add_last(&usr->pCommHead->gseHead, user_gse);

	return (user_gse);
}

/**
 * @Description: 添加PORT头
 */
SCL_PORT* user_comm_port_add(SCL_USER* usr)
{
	SCL_PORT *user_ports = NULL;	/* assume failure	*/
	if (!usr->pCommHead) {
		SLOG_ERROR ("NULL user commhead");
		return NULL;
	}
	user_ports = (SCL_PORT *) chk_calloc (1, sizeof (SCL_PORT));
	list_add_last(&usr->pCommHead->portHead, user_ports);

	return (user_ports);
}

/**
 * @Description: 根据传入的LN类型,抓取doi的配置
 * 入参 sclInfo 解析的scl文件信息 lntype 想要寻找的lntype
 */
ST_RET sclGetDOInfoByLnType(SCL_INFO* sclInfo, const char* lnType, SCL_DO** lnInfo) {
	if (!sclInfo || *lnType == '\0' ) {
		SLOG_ERROR("null pointer sclInfo or lnType");
		return -1;
	}

	if (!sclInfo->lnTypeHead) {
		SLOG_ERROR("null pointer lnTypeHead");
		return -2;
	}
	
	SCL_LNTYPE* lnodeType;
	ST_BOOLEAN found = SD_FALSE;
	//添加LNtype
	for (lnodeType = sclInfo->lnTypeHead; lnodeType != NULL; lnodeType = (SCL_LNTYPE*)list_get_next(sclInfo->lnTypeHead, lnodeType)) {
		if (strcmpi(lnodeType->id, lnType) != 0) continue;
		found = SD_TRUE;
		break;
	}

	if (lnodeType->doHead && found) {
		*lnInfo = lnodeType->doHead;
		return 0;
	}
	//没找到返回
	return -3;
}

/**
 * @Description: 根据Dotype字符串信息,获取DA链表,
 * pDaList返回找到的Dotype节点地址
 */
ST_RET sclGetDAListByDoType(SCL_INFO* sclInfo, const char* DoType, SCL_DA** pDaList) {
	if (!sclInfo || *DoType == '\0' ) {
		SLOG_ERROR("null pointer sclInfo or DoType");
		return -1;
	}

	if (!sclInfo->doTypeHead) {
		SLOG_ERROR("null pointer lnTypeHead");
		return -2;
	}	
	ST_BOOLEAN found = SD_FALSE;
	SCL_DOTYPE* ldoType;
	for (ldoType = sclInfo->doTypeHead; ldoType != NULL; ldoType = (SCL_DOTYPE *)list_get_next(sclInfo->doTypeHead, ldoType) ) {
		if (strcmpi(ldoType->id, DoType) != 0) continue;
		found |= SD_TRUE;
		break;		
	}

	//返回DA头部指针
	if (ldoType->daHead && found) {
		*pDaList = ldoType->daHead;
		return 0;
	}
	
	return -3;
}


/**
 * @Description: 根据DA info取出的type信息,获取Datype信息首地址
 */
ST_RET sclGetBdaByDaType(SCL_INFO* sclInfo, const char* DaType, SCL_BDA** pBdaHead) {
	if (!sclInfo || *DaType == '\0' ) {
		SLOG_ERROR("null pointer sclInfo or DaType");
		return -1;
	}

	if (!sclInfo->lnTypeHead) {
		SLOG_ERROR("null pointer lnTypeHead");
		return -2;
	}	

	SCL_DATYPE *daTypeHead;
	ST_BOOLEAN found = SD_FALSE;
	for (daTypeHead = sclInfo->daTypeHead; daTypeHead != NULL; daTypeHead = (SCL_DATYPE *)list_get_next(sclInfo->daTypeHead, daTypeHead) ) {
		if (strcmpi(daTypeHead->id, DaType)) continue;
		found |= SD_TRUE;
		break;		
	}

	if (daTypeHead->bdaHead && found) {
		*pBdaHead = daTypeHead->bdaHead;
		return 0;
	}
	
	return -3;
}

/**
 * @Description: 根据DA info取出的type信息,获取Datype信息首地址
 */
ST_RET sclGetEnumValByDaType(SCL_INFO* sclInfo, const char* DaType, SCL_ENUMVAL** enumvalHead) {
	if (!sclInfo || *DaType == '\0' ) {
		SLOG_ERROR("null pointer sclInfo or DaType");
		return -1;
	}

	if (!sclInfo->lnTypeHead) {
		SLOG_ERROR("null pointer lnTypeHead");
		return -2;
	}	

	SCL_ENUMTYPE *enumTypeHead;
	ST_BOOLEAN found = SD_FALSE;

	for (enumTypeHead = sclInfo->enumTypeHead; enumTypeHead != NULL; enumTypeHead = (SCL_ENUMTYPE *)list_get_next(sclInfo->enumTypeHead, enumTypeHead) ) {
		if (strcmpi(enumTypeHead->id, DaType) != 0) continue;
		found |= SD_TRUE;
		break;		
	}

	// SCL_ENUMVAL *enumvalHead;
	if (enumTypeHead->enumvalHead && found) {
		*enumvalHead = enumTypeHead->enumvalHead;
		return 0;
	}
	
	return -3;
}

/**
 * @Description: 简化接口处理,直接返回DO的struct类型
 */
ST_CHAR* sclGetDOTypeByDoName(SCL_INFO* sclInfo, const char* lnType, const char* doName) {
	SCL_DO* doType = NULL;
	SCL_DO* pDo;
	if (sclGetDOInfoByLnType(sclInfo, lnType, &doType)) {
		return NULL;
	}

	for (pDo = doType; pDo != NULL; pDo = (SCL_DO *)list_get_next(doType, pDo)) {
		//如果匹配到DO
		if (strcasecmp(pDo->name, doName)) continue;
		// printf ("Do name:%s type:%s\n", pDo->name, pDo->type);
		return pDo->type;
	}	
	return NULL;
}

/**
 * @Description: 根据SDI的name找到DA的配置
 */
SCL_DA* sclGetDaTypeBySdiName(SCL_INFO* sclInfo, const char* doType, const char* sdiName) {
	// ST_CHAR* ret = NULL;
	if (!sdiName || strlen(sdiName) == 0) return NULL;
	if (!doType || strlen(doType) == 0) return NULL;
	SCL_DA* daHead;
	// SCL_DO* sdoHead;
	if (sclGetDAListByDoType(sclInfo, doType, &daHead))
	{
		//没有找到,跳过
		SLOG_ERROR ("daHead not found doType = %s SDIName = %s",doType, sdiName);
		return (NULL);
	}
	
	SCL_DA* pDa;
	for (pDa = daHead; pDa != NULL; pDa = (SCL_DA *)list_get_next(daHead, pDa))
	{
		if (strcasecmp(pDa->name, sdiName) == 0) {
			// if (strlen(pDa->type) > 0) {
			// 	return pDa->type;
			// }
			// return pDa->bType;
			if (pDa != NULL) {
				return pDa;
			} 
			return (NULL);
		}
	} 

	return (NULL);
}

/**
 * @Description: 从<DAType id=""> 找到<BDA name="scaleFactor" bType="FLOAT32"/>
 */
ST_CHAR* sclGetBdaTypeByDaTypeId(SCL_INFO* sclInfo, const char* daTypeId, const char* bdaName) {
	if (!daTypeId || strlen(daTypeId) == 0) return NULL;
	if (!bdaName || strlen(bdaName) == 0) return NULL;	
	if (!sclInfo->daTypeHead) return NULL;

	SCL_BDA* bda;
	SCL_DATYPE* daTypeHead;
	// ST_CHAR* type;

	for (daTypeHead = sclInfo->daTypeHead; daTypeHead != NULL; daTypeHead = (SCL_DATYPE *)list_get_next(sclInfo->daTypeHead, daTypeHead)) {
		if (strcasecmp(daTypeHead->id, daTypeId) == 0) {
			for (bda = daTypeHead->bdaHead; bda != NULL; bda = (SCL_BDA *)list_get_next (daTypeHead->bdaHead, bda)) {
				if (strcasecmp (bda->name, bda->name) == 0) {
					//如果是struct类型
					if (strlen(bda->type) && strcasecmp(bda->bType, "struct") == 0) {
						return bda->type;
					}
					return bda->bType;
				}
			}
		}
	}
	SLOG_ERROR ("Bda Type not found %s %s", daTypeId, bdaName);
	return NULL;
}

/**
 * @Description: 获取BDA的da数据类型
 */
ST_RET sclGetDaiTypeByDaiflatName(SCL_INFO* sclInfo, const char* doType, const char* flattenedName, ST_CHAR** bdabType, ST_CHAR** fc) {
	if (!doType || strlen(doType) == 0) return -1;
	if (!flattenedName || strlen(flattenedName) == 0) return -2;	
	if (!sclInfo->daTypeHead) return -3;

	//max 5 loops, 解析flattenedName 每一个配对
	ST_CHAR daName[5][MAX_IDENT_LEN+1];

	ST_CHAR str[MAX_IDENT_LEN+1];
	//do not use original do str
	strncpy_safe(str, flattenedName, MAX_IDENT_LEN);

	//strtok 第二个参数必须是字符串
	ST_CHAR* next = strtok (str, "$");
	ST_UINT8 count = 0;
	while (next != NULL)
	{
		strncpy_safe(daName[count], next, MAX_IDENT_LEN);
		// printf("	daName[%d] = %s\n", count,daName[count]);
		count++;
		/* get next token */
		next = strtok (NULL, "$");
	}

	SCL_DOTYPE *pDoType;
	SCL_DO* pSdoList;
	SCL_DA* pDaList;
	
	for (pDoType  = sclInfo->doTypeHead; pDoType != NULL; pDoType = (SCL_DOTYPE *)list_get_next(sclInfo->doTypeHead, pDoType)){	
		if (strcasecmp(pDoType->id, doType)) continue;	//没找到继续
		//SDO name list
		ST_CHAR* pBdaType;
		for (pSdoList = pDoType->sdoHead; pSdoList != NULL; pSdoList = (SCL_DO *)list_get_next(pDoType->sdoHead, pSdoList)) {
			if (strcasecmp(pSdoList->name, daName[0]) == 0) {
				// printf ("	sDaiFlattend %s count %d pDaList->name %s\n", flattenedName, count, pSdoList->name);
				
				SCL_DA* da = sclGetDaTypeBySdiName(sclInfo, pSdoList->type, daName[1]);
				if (!da->type || !da) {
					SLOG_ERROR("sclGetDaTypeBySdiName not found doType %s dAname %s", pSdoList->type, daName[1]);
					continue;
				}
				//超过两层,递归遍历
				if (count > 2) {
					ST_UINT8 i;
					pBdaType = da->type;
					for (i = 2; i < count; i++){
						pBdaType = sclGetBdaTypeByDaTypeId(sclInfo, pBdaType, daName[i-1]);
						if (!pBdaType) {
							SLOG_ERROR("sclGetBdaTypeByDaTypeId not found i = %d pBdaType %s name %s", i, pBdaType, daName[i-1]);
							continue;
						}
					}
				}else {
					pBdaType = da->bType;
				}

				*bdabType = pBdaType;
				*fc = da->fc;

				// printf ("	SdoName %s SdoType %s fc %s\n", pSdoList->name, *bdabType, *fc);
				return 0;
			}
		}
		//DA name list
		for (pDaList= pDoType->daHead; pDaList != NULL; pDaList = (SCL_DA *)list_get_next(pDoType->daHead, pDaList)) {
			if (strcasecmp(pDaList->name, daName[0]) == 0) {
				pBdaType = sclGetBdaTypeByDaTypeId(sclInfo, pDaList->type, daName[1]);
				//递归寻找
				if (count > 2) {
					ST_UINT8 i;
					for (i = 2; i < count; i++){
						pBdaType = sclGetBdaTypeByDaTypeId(sclInfo, pBdaType, daName[i-1]);
						if (!pBdaType) {
							SLOG_ERROR("sclGetBdaTypeByDaTypeId not found %s name %s", pBdaType, daName[i-1]);
							continue;
						}						
					}
				}

				*bdabType = pBdaType;
				*fc = pDaList->fc;

				return 0;
			}
		}
	}

	SLOG_ERROR ("doType %s flattenedName %s not found", doType, flattenedName);
	return -4;
}


/**
 * @Description: 根据DOType id获取DA的数据类型
 */
ST_RET sclGetDaListByDAName(SCL_INFO* sclInfo, const char* doTypeId, const char* daName, SCL_DA** pDaList) {
	if (!daName || strlen(daName) == 0) return -1;
	if (!doTypeId || strlen(doTypeId) == 0) return -2;	
	if (!sclInfo->doTypeHead || !sclInfo) return -3;	

	SCL_DOTYPE* pDoType;
	for (pDoType = sclInfo->doTypeHead; pDoType != NULL; pDoType = (SCL_DOTYPE *)list_get_next(sclInfo->doTypeHead, pDoType)) {
		if (strcasecmp(pDoType->id , doTypeId) == 0)  {
			SCL_DA* pDa;
			for (pDa = pDoType->daHead; pDa != NULL; pDa = (SCL_DA *)list_get_next(pDoType->daHead, pDa)) {
				if (strcasecmp(pDa->name , daName) == 0) {
					*pDaList = pDa;
					return 0;
				}
			}
		}
	}
	
	SLOG_ERROR("doTypeId %s daName %s not found.", doTypeId, daName);
	return -4;
}

/**
 * @Description: 根据枚举的string获取枚举的数值
 * <EnumType id="Mod">
 * 先默认枚举序号不为-1
 */
ST_INT sclGetEnumValByString(SCL_INFO* sclInfo, const char* enumTypeId, const char* enumString) {
	if (!enumString || strlen(enumString) == 0) return -1;
	if (!enumTypeId || strlen(enumTypeId) == 0) return -2;
	if (!sclInfo->enumTypeHead) return -3;	

	SCL_ENUMVAL* pEnum;
	SCL_ENUMTYPE* pEnumType;
	for (pEnumType = sclInfo->enumTypeHead; pEnumType != NULL; pEnumType = (SCL_ENUMTYPE *)list_get_next(sclInfo->enumTypeHead, pEnumType)) {
		if (strcasecmp(pEnumType->id, enumTypeId)) continue; 	//if not found 
		for (pEnum = pEnumType->enumvalHead; pEnum != NULL; pEnum = (SCL_ENUMVAL *)list_get_next(pEnumType->enumvalHead, pEnum)) {
			if (strcasecmp(pEnum->EnumVal , enumString) == 0)  {
				return pEnum->ord;
			}
		}
	}

	return -4;
}
/**
 * @Description: 获取LN内的DOI详细信息
 * 返回结构链表
 */
ST_RET sclGetDoiNameValue(SCL_INFO* sclInfo, SCL_USER* usr) {
	if (!sclInfo || !usr){
		SLOG_ERROR("sclInfo or userinfo");
		return -1;
	} 

	SCL_ACCESSPOINT *scl_acpoint;
	SCL_LD *scl_ld = NULL;
	SCL_LN *scl_ln;

	if (list_get_sizeof(sclInfo->accessPointHead) == 0){
		SLOG_ERROR("Empty accessPoint");
		return -2;
	}
	
	for (scl_acpoint = sclInfo->accessPointHead; scl_acpoint != NULL; scl_acpoint = (SCL_ACCESSPOINT *)list_get_next(sclInfo->accessPointHead, scl_acpoint))
	{
		for (scl_ld = scl_acpoint->ldHead; scl_ld != NULL; scl_ld = (SCL_LD *)list_get_next(scl_acpoint->ldHead, scl_ld)){
			//for test
			// if (strcasecmp(scl_ld->inst, "prot")) continue;
			for (scl_ln = scl_ld->lnHead; scl_ln != NULL; scl_ln = (SCL_LN *)list_get_next(scl_ld->lnHead, scl_ln)) {
				// printf("<LN VarName=\"%s\" desc=\"%s\" lnType=\"%s\" lnClass=\"%s\" prefix=\"%s\" inst=\"%s\">\n",
				// scl_ln->varName, scl_ln->desc, scl_ln->lnType, scl_ln->lnClass, scl_ln->prefix, scl_ln->inst);	
				
				SCL_DOI* doiList;
				//for test
				// if (strcasecmp(scl_ln->varName, "ITCTR2")) continue;
				
				for (doiList = scl_ln->doiHead; doiList != NULL; doiList = (SCL_DOI *)list_get_next(scl_ln->doiHead, doiList)) {
					ST_CHAR* doiType = sclGetDOTypeByDoName(sclInfo, scl_ln->lnType, doiList->name);
					// printf("  <DOI name=\"%s\" desc=\"%s\" type=\"%s\">\n", doiList->name, doiList->desc, doiType);
					SCL_SDI* pSdi;
					SCL_DAI* pDai;
					ST_INT rec = 0;
					// printf("doiList->sdiHead = %d\n", list_get_sizeof(doiList->sdiHead));
					for (pSdi = doiList->sdiHead; pSdi != NULL; pSdi = (SCL_SDI *)list_get_next(doiList->sdiHead, pSdi)) {
						// ST_CHAR* daTypeName = sclGetDaBySdiName(sclInfo, doType, pSdi->name);
	
						if (list_get_sizeof(pSdi->sdaiHead) > 0) {
							SCL_DAI* pSDAi;
							for (pSDAi = pSdi->sdaiHead; pSDAi != NULL; pSDAi = (SCL_DAI *)list_get_next(pSdi->sdaiHead, pSDAi)) {
								// ST_CHAR* daType;
								SCL_LNINFO* lnInfo = (SCL_LNINFO *)chk_calloc (1, sizeof (SCL_LNINFO));
								if (list_add_last(&usr->pLnInfo, lnInfo)) {	//TODO 注意释放内存
									SLOG_ERROR("Add lnInfo list error");
									return -3;
								}
								
								ST_CHAR* dabType;
								ST_CHAR* fc;
								if ((rec =  sclGetDaiTypeByDaiflatName(sclInfo, doiType, pSDAi->flattened, &dabType, &fc)) !=0){
									SLOG_ERROR ("sclGetDaiTypeByDaiflatName err %d", rec);
									continue;
								}
								
								strncpy_safe(lnInfo->ldinst, scl_ld->inst, MAX_IDENT_LEN);
								strncpy_safe(lnInfo->lnVarName, scl_ln->varName, MAX_IDENT_LEN);
								lnInfo->lnDesc = nd_chk_strdup(scl_ln->desc); //TODO 注意释放内存
								strncpy_safe(lnInfo->doiName, doiList->name, MAX_IDENT_LEN);
								lnInfo->doiDesc = nd_chk_strdup(doiList->desc); //TODO 注意释放内存
								strncpy_safe(lnInfo->daiName, pSDAi->name, MAX_IDENT_LEN);
								lnInfo->daiDesc = nd_chk_strdup(pSDAi->desc); //TODO 注意释放内存
								strncpy_safe(lnInfo->daisAddr, pSDAi->sAddr, MAX_IDENT_LEN);
								lnInfo->daiVal = nd_chk_strdup(pSDAi->Val);//TODO 注意释放内存
								if (dabType != NULL && strlen(dabType) > 0) {
									strncpy_safe(lnInfo->daiType, dabType, strlen(dabType));
								}else
								{
									strncpy_safe(lnInfo->daiType, "NULL", strlen("NULL"));
								}
	
								strncpy_safe(lnInfo->fc, fc, MAX_FC_LEN);
								ST_CHAR refStr[100 + 1] = {0};
								snprintf (refStr, 100, "%s%s/%s$%s$%s$%s", sclInfo->lIEDHead->iedName, scl_ld->inst, scl_ln->varName, lnInfo->fc, doiList->name, pSDAi->flattened);
								lnInfo->ref = nd_chk_strdup(refStr);//TODO 注意释放内存
							}
						}
				
					}	

					for (pDai = doiList->daiHead; pDai != NULL; pDai = (SCL_DAI *)list_get_next(doiList->daiHead, pDai)) {
						SCL_LNINFO* lnInfo = (SCL_LNINFO *)chk_calloc (1, sizeof (SCL_LNINFO));
						if (list_add_last(&usr->pLnInfo, lnInfo)) {	//TODO 注意释放内存
							SLOG_ERROR("Add lnInfo list error");
							return -3;
						}			

						SCL_DA* daList;
						if ((rec = sclGetDaListByDAName (sclInfo, doiType, pDai->name, &daList)) !=0){
							SLOG_ERROR ("sclGetDaListByDAName err %d", rec);
							continue;
						}

						strncpy_safe(lnInfo->ldinst, scl_ld->inst, MAX_IDENT_LEN);
						strncpy_safe(lnInfo->lnVarName, scl_ln->varName, MAX_IDENT_LEN);
						lnInfo->lnDesc = nd_chk_strdup(scl_ln->desc); //TODO 注意释放内存
						strncpy_safe(lnInfo->doiName, doiList->name, MAX_IDENT_LEN);
						lnInfo->doiDesc = nd_chk_strdup(doiList->desc); //TODO 注意释放内存
						strncpy_safe(lnInfo->daiName, pDai->name, MAX_IDENT_LEN);
						lnInfo->daiDesc = nd_chk_strdup(pDai->desc); //TODO 注意释放内存
						strncpy_safe(lnInfo->daisAddr, pDai->sAddr, MAX_IDENT_LEN);
						lnInfo->daiVal = nd_chk_strdup(pDai->Val);//TODO 注意释放内存

						if (daList->bType != NULL && strlen(daList->bType) > 0) {
							strncpy_safe(lnInfo->daiType, daList->bType, strlen(daList->bType));
						} else
						{
							strncpy_safe(lnInfo->daiType, "NULL", strlen("NULL"));
						}
						
						strncpy_safe(lnInfo->fc, daList->fc, MAX_FC_LEN);				

						ST_CHAR refStr[100 + 1] = {0};
						snprintf (refStr, 100, "%s%s/%s$%s$%s$%s", sclInfo->lIEDHead->iedName, scl_ld->inst, scl_ln->varName, lnInfo->fc, doiList->name, pDai->flattened);
						lnInfo->ref = nd_chk_strdup(refStr);//TODO 注意释放内存		

					}	

				} 				
			}
			
		}
	}
	
	return 0;	
}

/**
 * @Description: 获取IED的通信配置头
 * return -1 failed 0 successful
 */
ST_RET scdGetCommunicationInfo(SCL_INFO* sclInfo, SCL_USER* usr)
{
    if (!sclInfo || !sclInfo->subnetHead || !usr)
    {
        return -1;
    }
	SCL_COMM* pCommHead;
	//添加LDevice 链表
	
    SCL_SUBNET *net;
    SCL_CAP *cap;
    SCL_GSE *gse;
	for(net = sclInfo->subnetHead; net!=NULL; net = (SCL_SUBNET*)list_get_next (sclInfo->subnetHead, net))
	{
		
		pCommHead = user_comm_create (usr);
		if (pCommHead == NULL)
		{
			SLOG_ERROR("Create SCL_COMM list failed"); 
			return -2;
		}		
		strncpy_safe(pCommHead->subNetWorkName, net->name, MAX_IDENT_LEN);
		strncpy_safe(pCommHead->subNetWorkType, net->type, MAX_IDENT_LEN);
		
		// printf ("<SubNetwork name=\"%s\" type=\"%s\">\n",pCommHead->subNetWorkName, pCommHead->subNetWorkType);
		for(cap = net->capHead; cap!=NULL; cap = (SCL_CAP *)list_get_next (net->capHead, cap))
		{
			strncpy_safe(pCommHead->apName, cap->apName, 2);
			if (cap->addr) 
			{
				strncpy_safe(pCommHead->address.IP, cap->addr->IP, 20);
				strncpy_safe(pCommHead->address.IPSUBNET, cap->addr->IPSUBNET, 20);
				// printf ("IP Address %s IP-Subnet %s\n", pCommHead->address.IP, pCommHead->address.IPSUBNET);
			}
			SCL_PORT *port;

			for (gse = cap->gseHead; gse != NULL; gse = (SCL_GSE *)list_get_next (cap->gseHead, gse)) {
				pCommHead->gseHead = user_comm_gse_add(usr);
				if (!pCommHead->gseHead){
					SLOG_ERROR("Create gseHead list failed"); 
					return -2;					
				}
				strncpy_safe(pCommHead->gseHead->ldInst, gse->ldInst, MAX_IDENT_LEN);
				strncpy_safe(pCommHead->gseHead->cbName, gse->cbName, MAX_IDENT_LEN);
				strncpy_safe(pCommHead->gseHead->MAC, gse->MAC, CLNP_MAX_LEN_MAC);
				pCommHead->gseHead->APPID = gse->APPID;
				pCommHead->gseHead->VLANID = gse->VLANID;
				pCommHead->gseHead->VLANPRI = gse->VLANPRI;
				pCommHead->gseHead->minTime = gse->minTime;
				pCommHead->gseHead->maxTime = gse->maxTime;
				// printf("GSE: ldInst: %s cbName %s\n  MAC %s APPID %d VLAN %d VLANPORI %d min %d max %d\n", 
				// 	pCommHead->gseHead->ldInst, pCommHead->gseHead->cbName, 
				// 	pCommHead->gseHead->MAC,  pCommHead->gseHead->APPID,  pCommHead->gseHead->VLANID,  pCommHead->gseHead->VLANPRI, pCommHead->gseHead->minTime, pCommHead->gseHead->maxTime);
			}
			
			for (port = cap->portHead; port != NULL; port = (SCL_PORT *)list_get_next (cap->portHead, port)) {
				pCommHead->portHead = user_comm_port_add(usr);
				if (!pCommHead->portHead){
					SLOG_ERROR("Create gseHead list failed"); 
					return -2;					
				}				
				strncpy_safe(pCommHead->portHead->portCfg, port->portCfg, MAX_IDENT_LEN);
				// printf("\tPort: %s\n", pCommHead->portHead->portCfg);
			}
		}
	}    

	return 0;
}

/******************************************
 * @Description: 获取LN0里面的dataset信息
 ******************************************/
ST_RET scdGetDataSetInfo(SCL_INFO* sclInfo, SCL_USER* usr) {
	if (!sclInfo || !usr){
		SLOG_ERROR("Empty sclInfo or userinfo");
		return -1;
	} 

	SCL_ACCESSPOINT *scl_acpoint;
	SCL_LD *scl_ld;
	SCL_LN *scl_ln;

	if (list_get_sizeof(sclInfo->accessPointHead) == 0){
		SLOG_ERROR("Empty accessPoint");
		return -2;
	}
	
	for (scl_acpoint = sclInfo->accessPointHead; scl_acpoint != NULL; scl_acpoint = (SCL_ACCESSPOINT *)list_get_next(sclInfo->accessPointHead, scl_acpoint))
	{
		for (scl_ld = scl_acpoint->ldHead; scl_ld != NULL; scl_ld = (SCL_LD *)list_get_next(scl_acpoint->ldHead, scl_ld)){
			for (scl_ln = scl_ld->lnHead; scl_ln != NULL; scl_ln = (SCL_LN *)list_get_next(scl_ld->lnHead, scl_ln)) {
				if (strcmpi(scl_ln->varName, "LLN0")) continue; 	//非LLN0跳过
				//如果是dataset
				SCL_DATASET* ds;
				SCL_DATASET* pDataSet;
				pDataSet = (SCL_DATASET *) chk_calloc (1, sizeof (SCL_DATASET));
				if (list_add_last(&usr->pDataSet, pDataSet)) {	//TODO 注意释放内存
					SLOG_ERROR("Add usrDataSet list error");
					return -3;
				}
				
				for(ds = scl_ln->datasetHead; ds != NULL; ds = (SCL_DATASET *)list_get_next(scl_ln->datasetHead, ds))
				{
					SCL_FCDA* fcda;
					strncpy_safe(pDataSet->name, ds->name, MAX_IDENT_LEN);
					pDataSet->desc = nd_chk_strdup(ds->desc);		//TODO 注意释放内存
					// strncpy_safe(, ds->desc);
					// printf("    <DataSet: name %s Desc %s>\n", pDataSet->name, pDataSet->desc); //
					for(fcda = ds->fcdaHead; fcda != NULL; fcda = (SCL_FCDA*)list_get_next(ds->fcdaHead, fcda)) 
					{
						SCL_FCDA* pFcda;
						
						pFcda = (SCL_FCDA *) chk_calloc (1, sizeof (SCL_FCDA));
						if (list_add_last(&pDataSet->fcdaHead, pFcda)) {
							SLOG_ERROR("Add usrDataSet Fcda list error");
							continue;
						}
						
						strncpy_safe(pFcda->domName, fcda->domName, MAX_IDENT_LEN);
						strncpy_safe(pFcda->ldInst, fcda->ldInst, MAX_IDENT_LEN);
						strncpy_safe(pFcda->prefix, fcda->prefix, MAX_IDENT_LEN);
						strncpy_safe(pFcda->lnClass, fcda->lnClass, MAX_IDENT_LEN);
						strncpy_safe(pFcda->lnInst, fcda->lnInst, MAX_IDENT_LEN);
						strncpy_safe(pFcda->doName, fcda->doName, MAX_IDENT_LEN);
						strncpy_safe(pFcda->daName, fcda->daName, MAX_IDENT_LEN);
						strncpy_safe(pFcda->fc, fcda->fc, MAX_IDENT_LEN);
						strncpy_safe(pFcda->doRefDesc, fcda->doRefDesc, MAX_IDENT_LEN);
						strncpy_safe(pFcda->doRefsAddr, fcda->doRefsAddr, MAX_IDENT_LEN);
						strncpy_safe(pFcda->lnRefType, fcda->lnRefType, MAX_IDENT_LEN);										
					}
				}				
			}
		}
	
	}

	return 0;
}