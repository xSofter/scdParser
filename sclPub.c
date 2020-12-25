/*
 * @Date: 2020-12-07 09:25
 * @LastEditTime: 2020-12-24 17:37
 * @LastEditors: tangkai3
 * @Description: 模块对外接口函数
 */

#include "sclPub.h"

#define DBG_LOG_FILE_NAME "scdParse.log"
#define USR_LOG_FILE_NAME "scdRes.txt"

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

	slog_start(SX_LOG_ALWAY, LOG_FILE_EN, DBG_LOG_FILE_NAME, USR_LOG_FILE_NAME); //SX_LOG_ERROR  SX_LOG_ALWAY
	rc = scl_parse(fileName, iedName, accessPointName, sclInfo);
	
	return rc;
}

/**
 * @Description: 释放scl文件解析的内容
 */
void release_scd_file(SCL_INFO* sclInfo)
{
	if (!sclInfo) return;
	slog_end();
	scl_info_destroy(sclInfo);
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
		// SLOG_DEBUG("lnodeType->doHead %s %s", lnodeType->doHead->name, lnodeType->doHead->desc);
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
	SCL_DO* pSdo;
	ST_CHAR sdoName[MAX_IDENT_LEN + 1] = {0};
	ST_CHAR doiName[MAX_IDENT_LEN + 1] = {0};
	
	if (sclGetDOInfoByLnType(sclInfo, lnType, &doType)) {
		SLOG_ERROR ("Can not find doType by lnType %s", lnType);
		return NULL;
	}

	ST_BOOLEAN found = SD_FALSE;
	if (strstr(doName, ".") != NULL) {
		sx_get_daName_combined(doName, ".", doiName, sdoName);
		// SLOG_DEBUG ("doName %s doiName %s sdoName %s",doName, doiName, sdoName);
		for (pDo = doType; pDo != NULL; pDo = (SCL_DO *)list_get_next(doType, pDo)) {
			//如果匹配到DO
			if (strcasecmp(pDo->name, doiName)) continue;
			found |= SD_TRUE;
			break;
		}		

		if (found) {
			SCL_DOTYPE* doTypeHead;
			for (doTypeHead = sclInfo->doTypeHead; doTypeHead != NULL; doTypeHead = (SCL_DOTYPE *)list_get_next(sclInfo->doTypeHead, doTypeHead)) {
				if (strcasecmp(doTypeHead->id, pDo->type)) continue;
				//找到dotype
				for (pSdo = doTypeHead->sdoHead; pSdo != NULL; pSdo = (SCL_DO* )list_get_next(doTypeHead->sdoHead, pSdo)) {
					if (strcasecmp(pSdo->name, sdoName)) continue;
					return pSdo->type;
				}
			}
		}
	} 
	else
	{
		strncpy_safe(doiName, doName, MAX_IDENT_LEN);
		for (pDo = doType; pDo != NULL; pDo = (SCL_DO *)list_get_next(doType, pDo)) {
			//如果匹配到DO
			if (strcasecmp(pDo->name, doiName)) continue;
			return pDo->type;
		}		
	}

	
	SLOG_ERROR ("Can not find doType by doName %s doType %s", doName, doType->name);
	return NULL;
}

/**
 * @Description: sclGetDaNameByFcName 通过DoTYpeid 和 fc类型,精准匹配daiName
 */
ST_CHAR* sclGetDaNameByFcName(SCL_INFO* sclInfo, const char* doType, const char* fcName) {
	SCL_DA* pDa = NULL;
	SCL_DOTYPE* pDoType;
	if (!doType || strlen(doType) == 0) return NULL;
	if (!fcName || strlen(fcName) == 0) return NULL;
	if (!sclInfo->doTypeHead) return NULL;

	for (pDoType = sclInfo->doTypeHead; pDoType != NULL; pDoType = (SCL_DOTYPE *)list_get_next(sclInfo->doTypeHead, pDoType)) {
		//如果匹配到DO
		if (strcasecmp(pDoType->id, doType)) continue;
		for (pDa = pDoType->daHead; pDa != NULL; pDa = (SCL_DA *)list_get_next(pDoType->daHead, pDa)) {
			if (strcasecmp(pDa->fc, fcName)) continue;
			return pDa->name;
		}
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
				// SLOG_DEBUG("bda->name %s", bda->name);
				if (strcasecmp (bda->name, bdaName) == 0) {
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
		count++;
		/* get next token */
		next = strtok (NULL, "$");
	}
	ST_UINT8 i;
	
	SCL_DOTYPE *pDoType;
	SCL_DO* pSdoList;
	SCL_DA* pDaList;
	
	for (pDoType  = sclInfo->doTypeHead; pDoType != NULL; pDoType = (SCL_DOTYPE *)list_get_next(sclInfo->doTypeHead, pDoType)){	
		if (strcasecmp(pDoType->id, doType)) continue;	//没找到继续
		//SDO name list
		ST_CHAR* pBdaType;
		for (pSdoList = pDoType->sdoHead; pSdoList != NULL; pSdoList = (SCL_DO *)list_get_next(pDoType->sdoHead, pSdoList)) {
			if (strcasecmp(pSdoList->name, daName[0]) == 0) {
				
				SCL_DA* da = sclGetDaTypeBySdiName(sclInfo, pSdoList->type, daName[1]);
				if (!da->type || !da) {
					SLOG_ERROR("sclGetDaTypeBySdiName not found doType %s dAname %s", pSdoList->type, daName[1]);
					return -3;
				}
				//超过两层,递归遍历
				if (count > 2) {
					pBdaType = da->type;
					for (i = 2; i < count; i++){
						pBdaType = sclGetBdaTypeByDaTypeId(sclInfo, pBdaType, daName[i]);
						if (!pBdaType) {
							SLOG_ERROR("sclGetBdaTypeByDaTypeId not found doType %s da->type %s pDaName %s daName[%d]=%s count %d", 
								doType, da->type, da->name, i, daName[i], count);
							return -3;
						}
					}
				}else {
					pBdaType = da->bType;
				}

				*bdabType = pBdaType;
				*fc = da->fc;

				return 0;
			}
		}
		//DA name list
		for (pDaList= pDoType->daHead; pDaList != NULL; pDaList = (SCL_DA *)list_get_next(pDoType->daHead, pDaList)) {
			if (strcasecmp(pDaList->name, daName[0]) == 0) {
				pBdaType = sclGetBdaTypeByDaTypeId(sclInfo, pDaList->type, daName[1]);
				if (!pBdaType) {
					SLOG_ERROR("sclGetBdaTypeByDaTypeId not found doType%s daName=%s pDaList->type=%s daName[1]=%s", 
							doType, pDaList->name, pBdaType, daName[1]);
					return -3;
				}
				//递归寻找
				if (count > 2) {
					for (i = 2; i < count; i++){
						pBdaType = sclGetBdaTypeByDaTypeId(sclInfo, pBdaType, daName[i]);
						if (!pBdaType) {
							SLOG_ERROR("sclGetBdaTypeByDaTypeId not found doType %s pDaName %s pBdaType %s daName[%d]=%s count=%d", 
								doType, pDaList->name, pBdaType, i, daName[i],count);
							return -3;
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
ST_RET sclGetDoiNameValue(SCL_INFO* sclInfo, void* database) {

	if (!sclInfo ){
		SLOG_ERROR("Empty SclInfo");
		return -1;
	} 

	SCL_ACCESSPOINT *scl_acpoint;
	SCL_LD *scl_ld = NULL;
	SCL_LN *scl_ln;
	if (!sclInfo->lIEDHead) {
		SLOG_ERROR("Empty ied");
		return -2;		
	}

	if (list_get_sizeof(sclInfo->lIEDHead->accessPointHead) == 0){
		SLOG_ERROR("Empty accessPoint");
		return -2;
	}
#ifdef DB_SQLITE3	
	struct timeval start, end;
	sqlite3* db = (sqlite3*)database;
	gettimeofday(&start, NULL);
	sqlite3_exec(db,"begin;",0,0,0);
	sqlite3_stmt *stmt; 
	char* zErrMsg = 0;
	char tabsql[1024 + 1] = {0};
	strcpy(tabsql, "insert into iec_ied_data(iedname,LD,LN,FC,DOI,DAI,ref,sAddr,val,val_type,val_size,ref_type,ref_size) values (?,?,?,?,?,?,?,?,?,?,'0',0,-255)");
	if (SQLITE_OK != sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt,0)) {
		SLOG_ERROR("Error: sqlite3_prepare_v2 %s", sqlite3_errmsg(db));
		if (stmt) sqlite3_finalize(stmt);
	}
#endif	
	SCL_IED* scl_ied;
	for (scl_ied = sclInfo->lIEDHead; scl_ied != NULL; scl_ied = (SCL_IED *)list_get_next(sclInfo->lIEDHead, scl_ied)) {
		userLog("==============================GetLNData IED NAME: %s==============================", scl_ied->iedName);
		for (scl_acpoint = scl_ied->accessPointHead; scl_acpoint != NULL; scl_acpoint = (SCL_ACCESSPOINT *)list_get_next(scl_ied->accessPointHead, scl_acpoint))
		{
			for (scl_ld = scl_acpoint->ldHead; scl_ld != NULL; scl_ld = (SCL_LD *)list_get_next(scl_acpoint->ldHead, scl_ld)){
				//for test
				// if (strcasecmp(scl_ld->inst, "prot")) continue;
				for (scl_ln = scl_ld->lnHead; scl_ln != NULL; scl_ln = (SCL_LN *)list_get_next(scl_ld->lnHead, scl_ln)) {
					userLog("<LN VarName=\"%s\" desc=\"%s\" lnType=\"%s\" lnClass=\"%s\" prefix=\"%s\" inst=\"%s\">",
					scl_ln->varName, scl_ln->desc, scl_ln->lnType, scl_ln->lnClass, scl_ln->prefix, scl_ln->inst);	
					
					SCL_DOI* doiList;
					//for test
					// if (strcasecmp(scl_ln->varName, "ITCTR2")) continue;
					
					for (doiList = scl_ln->doiHead; doiList != NULL; doiList = (SCL_DOI *)list_get_next(scl_ln->doiHead, doiList)) {
						ST_CHAR* doiType = sclGetDOTypeByDoName(sclInfo, scl_ln->lnType, doiList->name);

						userLog("  <DOI name=\"%s\" desc=\"%s\" type=\"%s\">", doiList->name, doiList->desc, doiType);
						SCL_SDI* pSdi;
						SCL_DAI* pDai;
						ST_INT rec = 0;
						for (pSdi = doiList->sdiHead; pSdi != NULL; pSdi = (SCL_SDI *)list_get_next(doiList->sdiHead, pSdi)) {
							// ST_CHAR* daTypeName = sclGetDaBySdiName(sclInfo, doType, pSdi->name);
							if (list_get_sizeof(pSdi->sdaiHead) > 0) {
								SCL_DAI* pSDAi;
								ST_CHAR* dabType;
								ST_CHAR* fc;								

								for (pSDAi = pSdi->sdaiHead; pSDAi != NULL; pSDAi = (SCL_DAI *)list_get_next(pSdi->sdaiHead, pSDAi)) {
									if ((rec =  sclGetDaiTypeByDaiflatName(sclInfo, doiType, pSDAi->flattened, &dabType, &fc)) !=0){
										SLOG_ERROR ("sclGetDaiTypeByDaiflatName err %d", rec);
										SLOG_DEBUG ("scl_ln->varName=%s doiName=%s doitype=%s sdiName=%s pSDAi->flattened=%s", scl_ln->varName, doiList->name, doiType, pSdi->name, pSDAi->flattened);
										continue;
									}
									ST_CHAR refStr[100 + 1] = {0};
									snprintf (refStr, 100, "%s%s/%s$%s$%s$%s", scl_ied->iedName, scl_ld->inst, scl_ln->varName, fc, doiList->name, pSDAi->flattened);
									userLog ("    <DAI ref=\"%s\" fc=\"%s\" sAddr=\"%s\" value=\"%s\" daType=\"%s\"/>", 
												refStr, fc, pSDAi->sAddr, pSDAi->Val, dabType);
#ifdef DB_SQLITE3									
									sqlite3_reset(stmt);
									sqlite3_bind_text(stmt, 1, scl_ied->iedName, strlen(scl_ied->iedName), NULL);
									sqlite3_bind_text(stmt, 2, scl_ld->inst, strlen(scl_ld->inst), NULL);
									sqlite3_bind_text(stmt, 3, scl_ln->varName, strlen(scl_ln->varName), NULL);
									sqlite3_bind_text(stmt, 4, fc, strlen(fc), NULL);
									sqlite3_bind_text(stmt, 5, doiList->name, strlen(doiList->name), NULL);
									sqlite3_bind_text(stmt, 6, pSDAi->name, strlen(pSDAi->name), NULL);
									
									sqlite3_bind_text(stmt, 7, refStr, strlen(refStr), NULL);
									sqlite3_bind_text(stmt, 8, pSDAi->sAddr , strlen(pSDAi->sAddr), NULL);
									if (pSDAi->Val != '\0'){
										sqlite3_bind_text(stmt, 9, pSDAi->Val, strlen(pSDAi->Val), NULL);
									}
									if (dabType != NULL && strlen(dabType) > 0) {
										sqlite3_bind_text(stmt, 10, dabType, strlen(dabType), NULL);
									}
									
									sqlite3_step(stmt);  
#endif									
								}
							}
					
						}	

						for (pDai = doiList->daiHead; pDai != NULL; pDai = (SCL_DAI *)list_get_next(doiList->daiHead, pDai)) {

							SCL_DA* daList;
							if ((rec = sclGetDaListByDAName (sclInfo, doiType, pDai->name, &daList)) !=0){
								SLOG_ERROR ("sclGetDaListByDAName err %d", rec);
								SLOG_DEBUG ("scl_ln->varName=%s doiName=%s doitype=%s daName=%s", scl_ln->varName, pDai->name, doiType, pDai->name);
								continue;
							}
							ST_CHAR refStr[100 + 1] = {0};
							snprintf (refStr, 100, "%s%s/%s$%s$%s$%s", scl_ied->iedName, scl_ld->inst, scl_ln->varName, daList->fc, doiList->name, pDai->flattened);							
							userLog ("    <DAI ref=\"%s\" fc=\"%s\" sAddr=\"%s\" value=\"%s\" daType=\"%s\"/>", 
										refStr, daList->fc, pDai->sAddr, pDai->Val, daList->bType);
#ifdef DB_SQLITE3			
							//"insert into iec_ied_data(iedname,LD,LN,FC,DOI,DAI,ref,sAddr,val,val_type,val_size,ref_type,ref_size) values (?,?,?,?,?,?,?,?,?,?,'0',0,-255)"						
							sqlite3_reset(stmt);
							sqlite3_bind_text(stmt, 1, scl_ied->iedName, strlen(scl_ied->iedName), NULL);
							sqlite3_bind_text(stmt, 2, scl_ld->inst, strlen(scl_ld->inst), NULL);
							sqlite3_bind_text(stmt, 3, scl_ln->varName, strlen(scl_ln->varName), NULL);
							sqlite3_bind_text(stmt, 4, daList->fc, strlen(daList->fc), NULL);
							sqlite3_bind_text(stmt, 5, doiList->name, strlen(doiList->name), NULL);
							sqlite3_bind_text(stmt, 6, pDai->name, strlen(pDai->name), NULL);
							sqlite3_bind_text(stmt, 7, refStr, strlen(refStr), NULL);

							sqlite3_bind_text(stmt, 8, pDai->sAddr, strlen(pDai->sAddr), NULL);
							if (pDai->Val != '\0'){
								sqlite3_bind_text(stmt, 9, pDai->Val, strlen(pDai->Val), NULL);
							}
							if (daList->bType != NULL && strlen(daList->bType) > 0) {
								sqlite3_bind_text(stmt, 10, daList->bType, strlen(daList->bType), NULL);
							}
						
							sqlite3_step(stmt);  
#endif
						}	

					} 				
				}
				
			} //end of LDevice inst
		}
	}
	
#ifdef DB_SQLITE3		
	sqlite3_finalize(stmt);  
	if (SQLITE_OK != sqlite3_exec(db,"commit;",0,0,&zErrMsg))
	{
		SLOG_ERROR("SQL error: %s", zErrMsg);	
		sqlite3_free(zErrMsg);		
		return -3;
	}
	//test proformence
	gettimeofday(&end, NULL);

	int cost = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	printf ("sclGetDoiNameValue succeess cost time: %d ms\n", cost/1000);
#endif

	
	return 0;	
}

/**
 * @Description: 获取IED的信息 名称和CRC等
 *  "iedname varchar(32) NOT NULL PRIMARY KEY,\
    type varchar(32),\
    desc varchar(64),\
    manufacturer varchar(32),\
    configVersion varchar(8),\
    iedCrc varchar(8)",
 */
void scdGetIedStructInfo(SCL_INFO* sclInfo, void* database) {
	if (!sclInfo) return;
#ifdef DB_SQLITE3
	char* zErrMsg = 0;
	char tabsql[1024 + 1] = {0};
	struct timeval start, end;

	sqlite3* db = (sqlite3*)database;
	gettimeofday(&start, NULL);

	sqlite3_exec(db,"begin;",0,0,0);
	sqlite3_stmt *stmt1; 
	strcpy(tabsql, "insert into iec_ied_struct(iedname,type,desc,manufacturer,configVersion,iedCrc, substationCRC) values (?,?,?,?,?,?,?)");
	if (SQLITE_OK != sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt1,0)) {
		SLOG_ERROR("sqlite3_prepare_v2 %s error", sqlite3_errmsg(db));
		if (stmt1) sqlite3_finalize(stmt1);
		return;
	}	
#endif	
	SCL_IED* ied;
	
	for(ied = sclInfo->lIEDHead; ied != NULL; ied = (SCL_IED*)list_get_next (sclInfo->lIEDHead, ied))
	{
		userLog("==============================GetDataset IED Struct: %s==============================", ied->iedName);
		userLog ("<IED name=\"%s\" type=\"%s\" desc=\"%s\" manufacturer=\"%s\" configVersion=\"%s\" iedCrc=\"%s\">", 
					ied->iedName, ied->iedType, ied->desc, ied->manufacturer, ied->configVersion, ied->iedDeviceCrc);
#ifdef DB_SQLITE3		
		sqlite3_reset(stmt1);
		sqlite3_bind_text(stmt1, 1, ied->iedName, strlen(ied->iedName), NULL);
		if (ied->iedType) sqlite3_bind_text(stmt1, 2, ied->iedType, strlen(ied->iedType), NULL);
		if (ied->desc)	sqlite3_bind_text(stmt1, 3, ied->desc, strlen(ied->desc), NULL);
		if (ied->manufacturer) sqlite3_bind_text(stmt1, 4, ied->manufacturer, strlen(ied->manufacturer), NULL);
		sqlite3_bind_text(stmt1, 5, ied->configVersion, strlen(ied->configVersion), NULL);
		sqlite3_bind_text(stmt1, 6, ied->iedDeviceCrc, strlen(ied->iedDeviceCrc), NULL);
		sqlite3_bind_text(stmt1, 7, sclInfo->Header.sclCrc, strlen(sclInfo->Header.sclCrc), NULL);
		sqlite3_step(stmt1);
#endif		
	}

#ifdef DB_SQLITE3	
	sqlite3_finalize(stmt1);  
	if (SQLITE_OK != sqlite3_exec(db,"commit;",0,0,&zErrMsg))
	{
		SLOG_ERROR("SQL error: %s", zErrMsg);	
		sqlite3_free(zErrMsg);
		return ;
	}
	//test proformence
	gettimeofday(&end, NULL);

	int cost = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	printf ("scdGetIedStructInfo succeess cost time: %d us\n", cost);			
#endif	
	return ;	
}
/**
 * @Description: 保存通信配置表
 */
void scdGetCommuncationInfo(SCL_INFO* sclInfo, void* database) {
	if (!sclInfo) return;
	
#ifdef DB_SQLITE3
	char* zErrMsg = 0;
	char tabsql[1024 + 1] = {0};
	struct timeval start, end;

	sqlite3* db = (sqlite3*)database;
	gettimeofday(&start, NULL);

	sqlite3_exec(db,"begin;",0,0,0);
	sqlite3_stmt *stmt1, *stmt2, *stmt3; 
	strcpy(tabsql, "insert into iec_ied_commu(iedname,substation,apname,ipAddress,ipSubNet,ipGateway) values (?,?,?,?,?,?)");
	if (SQLITE_OK != sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt1,0)) {
		SLOG_ERROR("sqlite3_prepare_v2 %s error", sqlite3_errmsg(db));
		if (stmt1) sqlite3_finalize(stmt1);
		
		return;
	}	

	strcpy(tabsql, "insert into iec_ied_commu_gse(subnetId,mac_address,vlan_id,vlan_priority,appid,mintime,maxtime) values (?,?,?,?,?,?,?)");
	if (SQLITE_OK != sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt2,0)) {
		SLOG_ERROR("sqlite3_prepare_v2 %s error", sqlite3_errmsg(db));
		if (stmt2) sqlite3_finalize(stmt2);
		
		return;
	}
	strcpy(tabsql, "insert into iec_ied_commu_phyConn(subnetId, apname, port) values (?,?,?)");
	if (SQLITE_OK != sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt3,0)) {
		SLOG_ERROR("sqlite3_prepare_v2 %s error", sqlite3_errmsg(db));
		if (stmt3) sqlite3_finalize(stmt3);
		return;
	}
#endif

	SCL_SUBNET* net;
	SCL_CAP* cap;
	SCL_GSE* gse;
	SCL_PORT* port;
	int subNetworkid = 1;
	userLog("==============================GetDataset IED Communcation==============================");
	for(net = sclInfo->subnetHead; net!=NULL; net = (SCL_SUBNET*)list_get_next (sclInfo->subnetHead, net))
	{
		userLog ("<SubNetwork name=\"%s\" type=\"%s\", desc=\"%s\">",net->name, net->type, net->desc == NULL ? "" : net->desc);
		// printf ("net->capHead size %d\n", list_get_sizeof(net->capHead));
		for ( cap= net->capHead; cap != NULL; cap = (SCL_CAP*)list_get_next(net->capHead, cap) )
		{
			userLog ("  <ConnectedAP iedName=\"%s\" apName=\"%s\">", cap->iedName, cap->apName);
			SCL_ADDRESS* addr;
			if( list_get_sizeof(cap->addrHead) ) {
				for (addr = cap->addrHead; addr != NULL; addr = (SCL_ADDRESS *)list_get_next(cap->addrHead, addr) ){		
					if (strlen(addr->IP)) {
						userLog("    IPAddress: %s IPSUBNET:%s IPGATEWAY:%s", addr->IP, addr->IPSUBNET, addr->IPGATEWAY);
#ifdef DB_SQLITE3						
						sqlite3_reset(stmt1);
						sqlite3_bind_text(stmt1, 1, cap->iedName, strlen(cap->iedName), NULL);
						sqlite3_bind_text(stmt1, 2, net->name, strlen(net->name), NULL);			
						sqlite3_bind_text(stmt1, 3, cap->apName, strlen(cap->apName), NULL);						
						sqlite3_bind_text(stmt1, 4, addr->IP, strlen(addr->IP), NULL);
						sqlite3_bind_text(stmt1, 5, addr->IPSUBNET, strlen(addr->IPSUBNET), NULL);
						sqlite3_bind_text(stmt1, 6, addr->IPGATEWAY, strlen(addr->IPGATEWAY), NULL);	
						sqlite3_step(stmt1);
#endif						
					}
				}
			}
#ifdef DB_SQLITE3			
			else{
				sqlite3_reset(stmt1);
				sqlite3_bind_text(stmt1, 1, cap->iedName, strlen(cap->iedName), NULL);
				sqlite3_bind_text(stmt1, 2, net->name, strlen(net->name), NULL);			
				sqlite3_bind_text(stmt1, 3, cap->apName, strlen(cap->apName), NULL);	
				// sqlite3_bind_text(stmt1, 4, "", 0, NULL);
				// sqlite3_bind_text(stmt1, 5, "", 0, NULL);							
				sqlite3_step(stmt1); 
			}
#endif			
			for (gse = cap->gseHead; gse != NULL; gse = (SCL_GSE *)list_get_next(cap->gseHead, gse)) {
				
				userLog("	GSE: ldInst: %s cbName %s MAC %s APPID %04x VLAN %d VLANPORI %d min %d max %d", 
							gse->ldInst, gse->cbName, 
							gse->MAC,  gse->APPID,  gse->VLANID,  gse->VLANPRI, gse->minTime, gse->maxTime);
#ifdef DB_SQLITE3							
				sqlite3_reset(stmt2);
				sqlite3_bind_int(stmt2, 1,subNetworkid);
				sqlite3_bind_text(stmt2, 2, gse->MAC, strlen(gse->MAC), NULL);
				sqlite3_bind_int(stmt2, 3, gse->VLANID);
				sqlite3_bind_int(stmt2, 4, gse->VLANPRI);
				sqlite3_bind_int(stmt2, 5, gse->APPID);
				sqlite3_bind_int(stmt2, 6, gse->minTime);
				sqlite3_bind_int(stmt2, 7, gse->maxTime);
				sqlite3_step(stmt2); 
#endif				
			}
			for (port = cap->portHead; port != NULL; port = (SCL_PORT *)list_get_next(cap->portHead, port)) {
				userLog("	Port: %s %s %s %s", port->portValue, port->portPlug, port->portType, port->portCable);
#ifdef DB_SQLITE3				
				sqlite3_reset(stmt3);
				sqlite3_bind_int(stmt3, 1,subNetworkid);
				sqlite3_bind_text(stmt3, 2, cap->apName, strlen(cap->apName), NULL);
				sqlite3_bind_text(stmt3, 3, port->portValue, strlen(port->portValue), NULL);
				sqlite3_step(stmt3); 
#endif								
			}			
		}
		subNetworkid++;
	}	
#ifdef DB_SQLITE3	
	sqlite3_finalize(stmt1);  
	sqlite3_finalize(stmt2);
	sqlite3_finalize(stmt3);
	if (SQLITE_OK != sqlite3_exec(db,"commit;",0,0,&zErrMsg))
	{
		SLOG_ERROR("SQL error: %s", zErrMsg);	
		sqlite3_free(zErrMsg);
		return ;
	}
	//test proformence
	gettimeofday(&end, NULL);

	int cost = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	printf ("scdGetCommuncationInfo succeess cost time: %d us\n", cost);			
#endif

	
	return;	
}

/******************************************
 * @Description: 获取LN0里面的dataset信息
 ******************************************/
ST_RET scdGetDataSetInfo(SCL_INFO* sclInfo, void* database) {
	if (!sclInfo){
		SLOG_ERROR("Empty sclInfo");
		return -1;
	} 
	
	SCL_ACCESSPOINT *scl_acpoint;
	SCL_LD *scl_ld;
	SCL_LN *scl_ln;

	if (list_get_sizeof(sclInfo->lIEDHead) == 0){
		SLOG_ERROR("Empty IED");
		return -2;
	}
	
#ifdef DB_SQLITE3

	sqlite3* db = (sqlite3*)database;
	char* zErrMsg = 0;
	int errCode;
	struct timeval start, end;
	char tabsql[1024 + 1] = {0};	
	sqlite3_exec(db,"begin;",0,0,0);
	sqlite3_stmt *stmt;
	gettimeofday(&start, NULL); 
	//total 11 elements
	strcpy(tabsql, "insert into iec_ied_dataset(iedname,ldInst,dsName,prefix,lnClass,lnInst,doName,daName,fc,fcda,desc,sAddr) values (?,?,?,?,?,?,?,?,?,?,?,?)");
	if ((errCode = sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt,0)) != SQLITE_OK) {
		SLOG_ERROR("error sqlite3_prepare_v2 %s", sqlite3_errmsg(db));
		if (stmt) sqlite3_finalize(stmt);
		return -3;
	}
#endif	

	SCL_IED* scl_ied;
	for (scl_ied = sclInfo->lIEDHead; scl_ied != NULL; scl_ied = (SCL_IED *)list_get_next(sclInfo->lIEDHead, scl_ied)) {
		userLog("==============================GetDataset IED NAME: %s==============================", scl_ied->iedName);
		for (scl_acpoint = scl_ied->accessPointHead; scl_acpoint != NULL; scl_acpoint = (SCL_ACCESSPOINT *)list_get_next(scl_ied->accessPointHead, scl_acpoint))
		{
			userLog("<AccessPoint name=\"%s\">", scl_acpoint->name);
			for (scl_ld = scl_acpoint->ldHead; scl_ld != NULL; scl_ld = (SCL_LD *)list_get_next(scl_acpoint->ldHead, scl_ld)){
				userLog (" <LDevice inst=\"%s\" desc=\"%s\">", scl_ld->inst, scl_ld->desc);
				for (scl_ln = scl_ld->lnHead; scl_ln != NULL; scl_ln = (SCL_LN *)list_get_next(scl_ld->lnHead, scl_ln)) {
					if (strcasecmp(scl_ln->varName, "LLN0")) continue; 	//非LLN0跳过

					SCL_DATASET* ds;
					// SCL_DATASET* pDataSet;
					userLog ("   <LN0 desc=\"%s\" lnType=\"%s\" lnClass=\"%s\" inst="">", scl_ln->desc, scl_ln->lnType, scl_ln->lnClass);

					for(ds = scl_ln->datasetHead; ds != NULL; ds = (SCL_DATASET *)list_get_next(scl_ln->datasetHead, ds))
					{
						SCL_FCDA* fcda;
						userLog("    <DataSet name=\"%s\" Desc=\"%s\">", ds->name, ds->desc); //
						for(fcda = ds->fcdaHead; fcda != NULL; fcda = (SCL_FCDA*)list_get_next(ds->fcdaHead, fcda)) 
						{
							userLog ("      <FCDA ldInst=\"%s\" prefix=\"%s\" lnClass=\"%s\" doName=\"%s\" lnInst=\"%s\" daName=\"%s\" fc=\"%s\" Ref=\"%s\" sAddr=\"%s\" desc=\"%s\"/>",
									fcda->ldInst, 
									fcda->prefix, 
									fcda->lnClass, 
									fcda->doName, 
									fcda->lnInst,
									fcda->daName,  
									fcda->fc,  
									fcda->domName, 
									fcda->doRefsAddr, 
									fcda->doRefDesc);
#ifdef DB_SQLITE3								
							sqlite3_reset(stmt);
							sqlite3_bind_text(stmt, 1, scl_ied->iedName, strlen(scl_ied->iedName), NULL);
							sqlite3_bind_text(stmt, 2, fcda->ldInst, strlen(fcda->ldInst), NULL);
							sqlite3_bind_text(stmt, 3, ds->name, strlen(ds->name), NULL);
							sqlite3_bind_text(stmt, 4, fcda->prefix, strlen(fcda->prefix), NULL);
							sqlite3_bind_text(stmt, 5, fcda->lnClass, strlen(fcda->lnClass), NULL);
							sqlite3_bind_text(stmt, 6, fcda->lnInst, strlen(fcda->lnInst), NULL);
							sqlite3_bind_text(stmt, 7, fcda->doName, strlen(fcda->doName), NULL);
							sqlite3_bind_text(stmt, 8, fcda->daName, strlen(fcda->daName), NULL);
							sqlite3_bind_text(stmt, 9, fcda->fc, strlen(fcda->fc), NULL);
							sqlite3_bind_text(stmt, 10, fcda->domName, strlen(fcda->domName), NULL);
							sqlite3_bind_text(stmt, 11, fcda->doRefDesc, strlen(fcda->doRefDesc), NULL);
							sqlite3_bind_text(stmt, 12, fcda->doRefsAddr, strlen(fcda->doRefsAddr), NULL);

							sqlite3_step(stmt); 
#endif						
						}
					}				
				}
			}
		}
	}
	
#ifdef DB_SQLITE3	
	sqlite3_finalize(stmt); 
	if (SQLITE_OK != sqlite3_exec(db,"commit;",0,0,&zErrMsg))
	{
		SLOG_ERROR("SQL error: %s", zErrMsg);	
		sqlite3_free(zErrMsg);
			
		return -3;	
	}
	//test proformence
	gettimeofday(&end, NULL);

	int cost = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	printf ("scdGetDataSetInfo succeess cost time: %d ms\n", cost/1000);
#endif		

	
	return 0;
}

/**
 * @Description: 
 * see:   
    "id integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    urcbName varchar(64) NOT NULL,\
    rptId varchar(64) NOT NULL,\
    dataset varchar(64) NOT NULL, \
    confRev integer NOT NULL,\
    trgOps integer NOT NULL,\
    optFlds integer NOT NULL,\
    rptEna BOOLEAN NOT NULL", 
 */
ST_RET sclGetUrcbElements(SCL_INFO* sclInfo, void* database) {
	if (!sclInfo) {
		SLOG_ERROR("Empty sclInfo");
		return -1;
	}
	SCL_ACCESSPOINT *scl_acpoint;
	SCL_LD *scl_ld;
	SCL_LN *scl_ln;

	if (list_get_sizeof(sclInfo->lIEDHead) == 0){
		SLOG_ERROR("Empty IED");
		
		return -2;
	}
	
	
#ifdef DB_SQLITE3
	sqlite3* db = (sqlite3*)database;
	char* zErrMsg = 0;
	int errCode;
	struct timeval start, end;
	char tabsql[1024 + 1] = {0};	
	sqlite3_exec(db,"begin;",0,0,0);
	sqlite3_stmt *stmt;
	gettimeofday(&start, NULL); 
	//total 11 elements
	strcpy(tabsql, "insert into iec_ied_urcb(urcbName,rptId,dataset,confRev,trgOps,optFlds,rptEna) values (?,?,?,?,?,?,0)");
	if ((errCode = sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt,0)) != SQLITE_OK) {
		SLOG_ERROR("sclGetUrcbElements error sqlite3_prepare_v2 %s", sqlite3_errmsg(db));
		if (stmt) sqlite3_finalize(stmt);
		
		return -3;
	}
#endif	
	SCL_IED* scl_ied;
	for (scl_ied = sclInfo->lIEDHead; scl_ied != NULL; scl_ied = (SCL_IED *)list_get_next(sclInfo->lIEDHead, scl_ied)) {
		userLog("==============================GetUrcb IED NAME: %s==============================", scl_ied->iedName);
		for (scl_acpoint = scl_ied->accessPointHead; scl_acpoint != NULL; scl_acpoint = (SCL_ACCESSPOINT *)list_get_next(scl_ied->accessPointHead, scl_acpoint)) {
			for (scl_ld = scl_acpoint->ldHead; scl_ld != NULL; scl_ld = (SCL_LD *)list_get_next(scl_acpoint->ldHead, scl_ld)){
				for (scl_ln = scl_ld->lnHead; scl_ln != NULL; scl_ln = (SCL_LN *)list_get_next(scl_ld->lnHead, scl_ln)) {
					if (strcasecmp(scl_ln->varName, "LLN0")) continue; 	//非LLN0跳过
					SCL_RCB* rcb;
					//找到
					for (rcb = scl_ln->rcbHead; rcb != NULL; rcb = (SCL_RCB *)list_get_next(scl_ln->rcbHead, rcb)) {
						if (rcb->buffered == 1) continue;
						if (rcb->maxClient < 1) continue;	//至少有一个RptMax
						
						ST_UINT8 i;
						for (i = 1; i <= rcb->maxClient; i++){
							ST_CHAR rcbName[MAX_IDENT_LEN+1] = {0};
							ST_CHAR rcbDataSet[MAX_IDENT_LEN+1] = {0};
							sprintf(rcbName, "%s/LLN0$RP$%s%d", scl_ld->domName,rcb->rptID,i);
							sprintf(rcbDataSet, "%s/LLN0$%s", scl_ld->domName,rcb->datSet);
							userLog("	Urcb rptName=\"%s\" datSet=\"%s\" intgPd=\"%d\" rptID=\"%s\" confRev=\"%d\" buffered=\"%d\" bufTime=\"%d\" TrgOps=\"%d\" OptFlds=\"%d\">", 
								rcbName,  rcbDataSet,  rcb->intgPd, rcb->rptID, rcb->confRev, rcb->buffered, rcb->bufTime, rcb->TrgOps[0], (rcb->OptFlds[1] << 8) + rcb->OptFlds[0]);
	#ifdef DB_SQLITE3	

							sqlite3_reset(stmt);
							sqlite3_bind_text(stmt, 1, scl_ied->iedName, strlen(scl_ied->iedName), NULL);
							sqlite3_bind_text(stmt, 2, rcbName, strlen(rcbName), NULL);
							sqlite3_bind_text(stmt, 3, rcb->rptID, strlen(rcb->rptID), NULL);
							sqlite3_bind_text(stmt, 4, rcbDataSet, strlen(rcbDataSet), NULL);
							sqlite3_bind_int(stmt, 5, rcb->confRev);
							sqlite3_bind_int(stmt, 6, rcb->TrgOps[0]);
							sqlite3_bind_int(stmt, 7, (rcb->OptFlds[1] << 8) + rcb->OptFlds[0]);

							sqlite3_step(stmt); 
	#endif							
						}

					}
				}
			}
		}
	}
#ifdef DB_SQLITE3	
	sqlite3_finalize(stmt); 
	if (SQLITE_OK != sqlite3_exec(db,"commit;",0,0,&zErrMsg))
	{
		SLOG_ERROR("SQL error: %s", zErrMsg);	
		sqlite3_free(zErrMsg);	
		
		return -3;	
	}
	//test proformence
	gettimeofday(&end, NULL);

	int cost = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	printf ("sclGetUrcbElements succeess cost time: %d us\n", cost);
#endif

	
	
	return 0;
}

/**
 * @Description: 
 * see:   
	id integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    brcbName varchar(64) NOT NULL,\
    rptId varchar(64) NOT NULL,\
    dataset varchar(64) NOT NULL, \
    bufferTm integer,\
    confRev integer NOT NULL,\
    trgOps integer NOT NULL,\
    optFlds integer NOT NULL,\
    rptEna BOOLEAN NOT NULL
 */
ST_RET sclGetBrcbElements(SCL_INFO* sclInfo, void* database) {
	if (!sclInfo) {
		SLOG_ERROR("Empty sclInfo");
		
		return -1;		
	}
	SCL_ACCESSPOINT *scl_acpoint;
	SCL_LD *scl_ld;
	SCL_LN *scl_ln;

	if (list_get_sizeof(sclInfo->lIEDHead) == 0){
		SLOG_ERROR("Empty IED");
		return -2;
	}
	
#ifdef DB_SQLITE3
	sqlite3* db = (sqlite3*)database;
	char* zErrMsg = 0;
	int errCode;
	struct timeval start, end;
	char tabsql[1024 + 1] = {0};	
	sqlite3_exec(db,"begin;",0,0,0);
	sqlite3_stmt *stmt;
	gettimeofday(&start, NULL); 
	//total 11 elements
	strcpy(tabsql, "insert into iec_ied_brcb(brcbName,rptId,dataset, bufferTm, confRev,trgOps,optFlds,rptEna) values (?,?,?,?,?,?,?,0)");
	if ((errCode = sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt,0)) != SQLITE_OK) {
		SLOG_ERROR("sclGetUrcbElements error sqlite3_prepare_v2 %s", sqlite3_errmsg(db));
		if (stmt) sqlite3_finalize(stmt);
		
		return -3;
	}
#endif	
	SCL_IED* scl_ied;
	for (scl_ied = sclInfo->lIEDHead; scl_ied != NULL; scl_ied = (SCL_IED *)list_get_next(sclInfo->lIEDHead, scl_ied)) {
		userLog("==============================GetBrcb IED NAME: %s==============================", scl_ied->iedName);
		for (scl_acpoint = scl_ied->accessPointHead; scl_acpoint != NULL; scl_acpoint = (SCL_ACCESSPOINT *)list_get_next(scl_ied->accessPointHead, scl_acpoint)) {
			for (scl_ld = scl_acpoint->ldHead; scl_ld != NULL; scl_ld = (SCL_LD *)list_get_next(scl_acpoint->ldHead, scl_ld)){
				for (scl_ln = scl_ld->lnHead; scl_ln != NULL; scl_ln = (SCL_LN *)list_get_next(scl_ld->lnHead, scl_ln)) {
					if (strcasecmp(scl_ln->varName, "LLN0")) continue; 	//非LLN0跳过
					SCL_RCB* rcb;
					//找到
					for (rcb = scl_ln->rcbHead; rcb != NULL; rcb = (SCL_RCB *)list_get_next(scl_ln->rcbHead, rcb)) {
						if (rcb->buffered == 0) continue;	//跳过urcb
						if (rcb->maxClient < 1) continue;	//至少有一个RptMax
						
						ST_UINT8 i;
						for (i = 1; i <= rcb->maxClient; i++){
							ST_CHAR rcbName[MAX_IDENT_LEN+1] = {0};
							ST_CHAR rcbDataSet[MAX_IDENT_LEN+1] = {0};
							sprintf(rcbName, "%s/LLN0$RP$%s%d", scl_ld->domName,rcb->rptID,i);
							sprintf(rcbDataSet, "%s/LLN0$%s", scl_ld->domName,rcb->datSet);
							userLog ("	BRCB name=\"%s\" datSet=\"%s\" intgPd=\"%d\" rptID=\"%s\" confRev=\"%d\" buffered=\"%d\" bufTime=\"%d\" TrgOps=\"%d\" OptFlds=\"%d\">", 
							rcbName,  rcbDataSet,  rcb->intgPd, rcb->rptID, rcb->confRev, rcb->buffered, rcb->bufTime, rcb->TrgOps[0], (rcb->OptFlds[1] << 8) + rcb->OptFlds[0]);
	#ifdef DB_SQLITE3	

							sqlite3_reset(stmt);
							// sqlite3_bind_text(stmt, 1, sclInfo->lIEDHead->iedName, strlen(sclInfo->lIEDHead->iedName), NULL);
							sqlite3_bind_text(stmt, 1, rcbName, strlen(rcbName), NULL);
							sqlite3_bind_text(stmt, 2, rcb->rptID, strlen(rcb->rptID), NULL);
							sqlite3_bind_text(stmt, 3, rcbDataSet, strlen(rcbDataSet), NULL);
							sqlite3_bind_int(stmt, 4, rcb->bufTime);
							sqlite3_bind_int(stmt, 5, rcb->confRev);
							sqlite3_bind_int(stmt, 6, rcb->TrgOps[0]);
							sqlite3_bind_int(stmt, 7, (rcb->OptFlds[1] << 8) + rcb->OptFlds[0]);

							sqlite3_step(stmt); 
	#endif							
						}

					}
				}
			}
		}
	}
#ifdef DB_SQLITE3	
	sqlite3_finalize(stmt); 
	if (SQLITE_OK != sqlite3_exec(db,"commit;",0,0,&zErrMsg))
	{
		SLOG_ERROR("SQL error: %s", zErrMsg);	
		sqlite3_free(zErrMsg);	
		
		return -3;	
	}
	//test proformence
	gettimeofday(&end, NULL);

	int cost = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	printf ("sclGetBrcbElements succeess cost time: %d us\n", cost);
#endif
	
	return 0;
}


/**
 * @Description: 获取LLN0 LogControl的配置信息
 */
ST_RET sclGetLogControlBlock(SCL_INFO* sclInfo, void* database) {
	
	if (!sclInfo) {
		SLOG_ERROR("Empty sclInfo");
		return -1;		
	}
	SCL_ACCESSPOINT *scl_acpoint;
	SCL_LD *scl_ld;
	SCL_LN *scl_ln;

	if (list_get_sizeof(sclInfo->lIEDHead) == 0){
		SLOG_ERROR("Empty IED");
		return -2;
	}

#ifdef DB_SQLITE3
	sqlite3* db = (sqlite3*)database;
	char* zErrMsg = 0;
	int errCode;
	struct timeval start, end;
	char tabsql[1024 + 1] = {0};	
	sqlite3_exec(db,"begin;",0,0,0);
	sqlite3_stmt *stmt;
	gettimeofday(&start, NULL); 
	//total 11 elements
	strcpy(tabsql, "insert into iec_ied_logControlBlock(iedname,logRef,logDataSet,lcbDesc,logEna,intgPd,trgOps) values (?,?,?,?,?,?,?)");
	if ((errCode = sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt,0)) != SQLITE_OK) {
		SLOG_ERROR("error sqlite3_prepare_v2 %s", sqlite3_errmsg(db));
		if (stmt) sqlite3_finalize(stmt);
		
		return -3;
	}
#endif	
	ST_CHAR iedLDhead[96] = {0};
	ST_CHAR lcbRef[128] = {0};
	ST_CHAR lcbDataSet[128] = {0};
	SCL_IED* scl_ied;
	for (scl_ied = sclInfo->lIEDHead; scl_ied != NULL; scl_ied = (SCL_IED *)list_get_next(sclInfo->lIEDHead, scl_ied)) {		
		userLog("==============================GetLogControlBlock IED NAME: %s==============================", scl_ied->iedName);
		for (scl_acpoint = scl_ied->accessPointHead; scl_acpoint != NULL; scl_acpoint = (SCL_ACCESSPOINT *)list_get_next(scl_ied->accessPointHead, scl_acpoint)) {
			for (scl_ld = scl_acpoint->ldHead; scl_ld != NULL; scl_ld = (SCL_LD *)list_get_next(scl_acpoint->ldHead, scl_ld)){
				memset(lcbRef, 0, sizeof(lcbRef));
				memset(lcbDataSet, 0, sizeof(lcbDataSet));
				memset(iedLDhead, 0, sizeof(ST_CHAR) * 96);
				if (strlen(scl_ied->iedName) < 1) {
					strncpy_safe(iedLDhead, "UNKNOW_IED_NAME", 128);
				} else {
					strncpy_safe(iedLDhead, scl_ied->iedName, 128);
				}				
				strcat(iedLDhead, scl_ld->inst);
				strcat(lcbRef, iedLDhead);
				strcat(lcbRef, "/");
				strcat(lcbRef, scl_ld->inst);
				
				strcat(lcbDataSet, iedLDhead);
				strcat(lcbDataSet, "/LLN0$");
				for (scl_ln = scl_ld->lnHead; scl_ln != NULL; scl_ln = (SCL_LN *)list_get_next(scl_ld->lnHead, scl_ln)) {
					if (strcasecmp(scl_ln->varName, "LLN0")) continue; 	//非LLN0跳过
					SCL_LCB* lcb;
					for (lcb = scl_ln->lcbHead; lcb != NULL; lcb = (SCL_LCB *)list_get_next(scl_ln->lcbHead, lcb)) {
						strcat(lcbDataSet, lcb->datSet);
						userLog ("   <LogControl name=\"%s\" desc=\"%s\" datSet=\"%s\" intgPd=\"%d\" TrgOps=\"0x%x\" logEna=\"%d\">", 
							lcbRef, lcb->desc,  lcbDataSet, lcb->intgPd, lcb->TrgOps[0], lcb->logEna);
						
						// 	scl_rptCtlget(lcb->TrgOps,"dchg"), scl_rptCtlget(lcb->TrgOps,"qchg"), scl_rptCtlget(lcb->TrgOps,"dupd"), scl_rptCtlget(lcb->TrgOps,"period"));
	#ifdef DB_SQLITE3								
						sqlite3_reset(stmt);
						sqlite3_bind_text(stmt, 1, scl_ied->iedName, strlen(scl_ied->iedName), NULL);
						sqlite3_bind_text(stmt, 2, lcbRef, strlen(lcbRef), NULL);
						sqlite3_bind_text(stmt, 3, lcbDataSet, strlen(lcbDataSet), NULL);
						sqlite3_bind_text(stmt, 4, lcb->desc, strlen(lcb->desc), NULL);
						sqlite3_bind_int(stmt, 5, lcb->logEna);
						sqlite3_bind_int(stmt, 6, lcb->intgPd);
						sqlite3_bind_int(stmt, 7, lcb->TrgOps[0]);

						sqlite3_step(stmt); 
	#endif							
					}
				}
			}
		}
	}
#ifdef DB_SQLITE3	
	sqlite3_finalize(stmt); 
	if (SQLITE_OK != sqlite3_exec(db,"commit;",0,0,&zErrMsg))
	{
		SLOG_ERROR("sqlite3_exec error: %s", zErrMsg);	
		sqlite3_free(zErrMsg);	
		
		return -3;	
	}
	//test proformence
	gettimeofday(&end, NULL);

	int cost = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	printf ("sclGetLogControlBack success cost time: %d us\n", cost);
#endif

	
	return 0;
}