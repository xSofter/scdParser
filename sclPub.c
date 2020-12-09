/*
 * @Date: 2020-12-07 09:25
 * @LastEditTime: 2020-12-09 11:58
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
int load_scd_file(char* fileName, SCL_INFO* sclInfo)
{
	//ST_CHAR *xmlFileName = "TXJD01.cid";//"jilindong.scd"; //"TXJD01.cid";
	ST_CHAR *iedName = "C5011";
	ST_CHAR *accessPointName = "S1";
	ST_RET rc = 0;

	slog_start(SX_LOG_ALWAY, LOG_FILE_EN, LOG_FILE_NAME); //SX_LOG_ERROR  SX_LOG_ALWAY
	rc = scl_parse(fileName, iedName, accessPointName, sclInfo);
	printf ("main: parse %s, rc=%d\n", fileName, rc);
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
 * @Description: 获取IED的通信配置头
 * return -1 failed 0 successful
 */
int scdGetCommunicationInfo(SCL_INFO* sclInfo)
{
    if (!sclInfo || !sclInfo->subnetHead)
    {
        return -1;
    }
    SCL_SUBNET *net;
    SCL_CAP *cap;
    SCL_GSE *gse;
	for(net = sclInfo->subnetHead; net!=NULL; net = (SCL_SUBNET*)list_get_next (sclInfo->subnetHead, net))
	{
		printf ("<SubNetwork name=\"%s\" type=\"%s\">\n",net->name, net->type);
		
		for(cap = net->capHead; cap!=NULL; cap = (SCL_CAP *)list_get_next (net->capHead, cap))
		{
			if (cap->addr) 
			{
				printf ("IP Address %s IP-Subnet %s\n", cap->addr->IP, cap->addr->IPSUBNET);
			}
			SCL_PORT *port;

			for (gse = cap->gseHead; gse != NULL; gse = (SCL_GSE *)list_get_next (cap->gseHead, gse)) {
				printf("GSE: ldInst: %s cbName %s\n  Name %s MAC %s APPID %d min %d max %d\n", 
					gse->ldInst, gse->cbName, gse->cbName, gse->MAC, gse->APPID, gse->minTime, gse->maxTime);
			}
			
			for (port = cap->portHead; port != NULL; port = (SCL_PORT *)list_get_next (cap->portHead, port)) {
				printf("\tPort: %s\n", port->portCfg);
			}
		}
	}    

	return 0;
}

/******************************************
 * @Description: 获取LN0里面的dataset信息
 ******************************************/
int scdGetDataSetInfo(SCL_INFO* sclInfo) {
	if (!sclInfo){
		SLOG_ERROR("null sclInfo");
		return -1;
	} 

	SCL_ACCESSPOINT *scl_acpoint;
	SCL_LD *scl_ld;
	SCL_LN *scl_ln;

	if (list_get_sizeof(sclInfo->accessPointHead) == 0){
		SLOG_ERROR("Error: Empty accessPoint");
		return -2;
	}
	
	for (scl_acpoint = sclInfo->accessPointHead; scl_acpoint != NULL; scl_acpoint = (SCL_ACCESSPOINT *)list_get_next(sclInfo->accessPointHead, scl_acpoint))
	{
		for (scl_ld = scl_acpoint->ldHead; scl_ld != NULL; scl_ld = (SCL_LD *)list_get_next(scl_acpoint->ldHead, scl_ld)){
			printf("<LDevice inst= %s desc=%s apName=%s>\n", scl_ld->inst, scl_ld->desc, scl_ld->apName);
			
			for (scl_ln = scl_ld->lnHead; scl_ln != NULL; scl_ln = (SCL_LN *)list_get_next(scl_ld->lnHead, scl_ln)) {
				if (strcmpi(scl_ln->varName, "LLN0")) continue;
				// ST_UCHAR dsCount = scl_ln->datasetCount;
				printf("  <LN VarName=\"%s\" desc=\"%s\" lnType=\"%s\" lnClass=\"%s\" prefix=\"%s\" inst=\"%s\">\n",
								 scl_ln->varName, scl_ln->desc, scl_ln->lnType, scl_ln->lnClass, scl_ln->prefix, scl_ln->inst);
				//如果是dataset
				SCL_DATASET* ds;
				for(ds = scl_ln->datasetHead; ds != NULL; ds = (SCL_DATASET *)list_get_next(scl_ln->datasetHead, ds))
				{
					SCL_FCDA* fcda;
					printf("    <DataSet: name %s Desc %s>\n", ds->name, ds->desc); //
					for(fcda = ds->fcdaHead; fcda != NULL; fcda = (SCL_FCDA*)list_get_next(ds->fcdaHead, fcda)) 
					{
						printf("      <FCDA ldInst=%s prefix=%s lnClass=%s lnInst=%s doName=%s daName=%s fc=%s desc=%s sAddr=%s>\n", 
											fcda->ldInst,
											fcda->prefix, 
											fcda->lnClass,
											fcda->lnInst,
											fcda->doName,
											fcda->daName, 
											fcda->fc, 
											fcda->doRefDesc,
											fcda->doRefsAddr); 
																		
					}
				}				
			}
		}
	
	}

	return 0;
}
/**
 * @Description: 根据传入的LN类型,抓取doi的配置
 * 入参 sclInfo 解析的scl文件信息 lntype 想要寻找的lntype
 */
int sclGetDOInfoByLnType(SCL_INFO* sclInfo, const char* lnType, SCL_DO** lnInfo) {
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
 * pDaInfo返回找到的Dotype节点地址
 */
int sclGetDAInfoByDoType(SCL_INFO* sclInfo, const char* DoType, SCL_DA** pDAInfo) {
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
		*pDAInfo = ldoType->daHead;
		return 0;
	}
	
	return -3;
}


/**
 * @Description: 根据DA info取出的type信息,获取Datype信息首地址
 */
int sclGetBdaByDaType(SCL_INFO* sclInfo, const char* DaType, SCL_BDA** pBdaHead) {
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
		if (strcmpi(daTypeHead->id, DaType) != 0) continue;
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
int sclGetEnumValByDaType(SCL_INFO* sclInfo, const char* DaType, SCL_ENUMVAL** enumvalHead) {
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
