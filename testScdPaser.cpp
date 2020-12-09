/*
 * @Date: 2020-12-07 14:26
 * @LastEditTime: 2020-12-09 11:18
 * @LastEditors: tangkai3
 * @Description: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <QDebug>
// #include <QDateTime>
#include "sclPub.h"
#include "str_util.h"

#ifdef DEBUG_SISCO
SD_CONST static ST_CHAR *SD_CONST thisFileName = __FILE__;
#endif

int main(int argc, char* argv[])
{
	//thisFileName = __FILE__;

	ST_CHAR *xmlFileName = "Template.icd";//"jilindong.scd"; //"TXJD01.cid";
	ST_CHAR *iedName = "C5011";
	ST_CHAR *accessPointName = "S1";
	int rc;

	
	SCL_INFO sclInfo;
	memset(&sclInfo, 0, sizeof(SCL_INFO));

	SCL_LD* ld;
	SCL_LN* ln;
	SCL_DOI* doi;
	// SCL_DAI* dai;
	SCL_SDI* sdi;

	SCL_SUBNET *net;
	SCL_CAP *cap;
	SCL_GSE *gse;

	SCL_LNTYPE *lnt;
	SCL_DO* don;

	SCL_DOTYPE *dot;
	SCL_DA *dan;

	// int i=0;

	//test for log 
	// setlocale(LC_ALL, "");  
	slog_start(SX_LOG_ALWAY, LOG_FILE_EN, "scdpase.log");
	rc = scl_parse(xmlFileName, iedName, accessPointName, &sclInfo);	
	// rc = load_scd_file(xmlFileName, &sclInfo);
	// if (rc != 0 )
	// {
	// 	printf("return error %d\n", rc);
	// 	return -1;
	// }


	//����communication,
	scdGetCommunicationInfo(&sclInfo);
	// scdGetDataSetInfo(&sclInfo);
#if 0
	SCL_LNTEMPLATE lnInfo;
	memset(&lnInfo, 0, sizeof(SCL_LNTEMPLATE));
	SCL_DO* pDo;
	
	sclGetDOInfoByLnType(&sclInfo, "GDNR_NSR-3620A-DG-N_V1_CSWI", &lnInfo.doHead);
	for (pDo = lnInfo.doHead; pDo != NULL; pDo = (SCL_DO *)list_get_next(lnInfo.doHead, pDo)) {
		if (strcmpi(pDo->name, "Mod") == 0) continue;
		if (strcmpi(pDo->name, "Beh") == 0) continue;
		if (strcmpi(pDo->name, "Health") == 0) continue;
		if (strcmpi(pDo->name, "NamPlt") == 0) continue;
		printf ("  <DO name=\"%s\" type=\"%s\" desc=\"%s\"/>\n", pDo->name, pDo->type, pDo->desc);
		if (sclGetDAInfoByDoType(&sclInfo, pDo->type, &lnInfo.daHead))
		{
			//没有找到,跳过
			continue;
		}
		SCL_DA* pDa;
		// printf ("  DOType id=\"%s\" cdc=\"%s\">\n", pDo->id, pDo->cdc);			
		for (pDa = lnInfo.daHead; pDa != NULL; pDa = (SCL_DA*)list_get_next(lnInfo.daHead, pDa)) {
			printf ("	<DA name=\"%s\" bType=\"%s\" type=\"%s\" dchg=\"%d\" fc=\"%s\"/>\n", pDa->name, pDa->bType, pDa->type, pDa->dchg, pDa->fc);
			if (strcmpi(pDa->bType, "struct") == 0) {
				//��DA��ʼ�ݹ�Ѱ��
				int i = 0;
				if (sclGetBdaByDaType(&sclInfo, pDa->type, &(lnInfo.badInfoHead[i]))) continue;
				SCL_BDA* pBda;
				for (pBda =  lnInfo.badInfoHead[i]; pBda != NULL; pBda = (SCL_BDA*)list_get_next( lnInfo.badInfoHead[i], pBda)) {
					printf ("		<BDA name=\"%s\" bType=\"%s\" type=\"%s\"/>\n",pBda->name, pBda->bType, pBda->type);
					SCL_BDA* curBdaAddr;
					if (strcmpi(pBda->bType, "struct") == 0) {
						++i;
						curBdaAddr = pBda;
						// printf ("1		======= i=%d pBda %p %s\n", i, pBda, pBda->type);
						if (sclGetBdaByDaType(&sclInfo, pBda->type, &(lnInfo.badInfoHead[i]))) continue;
						for (pBda =  lnInfo.badInfoHead[i]; pBda != NULL; pBda = (SCL_BDA*)list_get_next( lnInfo.badInfoHead[i], pBda)) { 
							printf ("		    <BDA name=\"%s\" bType=\"%s\" type=\"%s\"/>\n",pBda->name, pBda->bType, pBda->type);

							if (strcmpi(pBda->bType, "Enum") == 0) {
								SCL_ENUMVAL* pEnum;
								if (sclGetEnumValByDaType(&sclInfo, pBda->type, &(lnInfo.enumvalHead))) continue;
								for (pEnum =  lnInfo.enumvalHead; pEnum != NULL; pEnum = (SCL_ENUMVAL*)list_get_next( lnInfo.enumvalHead, pEnum)) {
									printf ("		        <EnumVal ord=\"%d\">%s</EnumVal>\n",pEnum->ord, pEnum->EnumVal);					
								}
							}	
							if (strcmpi(pBda->bType, "struct") == 0) {
								++i;
								// curBdaAddr = pBda;
								printf("name %s, type %s \n",pBda->name, pBda->type);
								if (sclGetBdaByDaType(&sclInfo, pBda->type, &(lnInfo.badInfoHead[i]))) continue;
								for (pBda =  lnInfo.badInfoHead[i]; pBda != NULL; pBda = (SCL_BDA*)list_get_next( lnInfo.badInfoHead[i], pBda)) { 
									printf ("				<BDA name=\"%s\" bType=\"%s\" type=\"%s\"/>\n",pBda->name, pBda->bType, pBda->type);
								}
								--i;			
								// pBda = curBdaAddr;
							}									
						}
						--i;			
						pBda = curBdaAddr;
					} 
					
					if (strcmpi(pBda->bType, "Enum") == 0) {
						SCL_ENUMVAL* pEnum;
						if (sclGetEnumValByDaType(&sclInfo, pBda->type, &(lnInfo.enumvalHead))) continue;
						for (pEnum =  lnInfo.enumvalHead; pEnum != NULL; pEnum = (SCL_ENUMVAL*)list_get_next( lnInfo.enumvalHead, pEnum)) {
							printf ("		    <EnumVal ord=\"%d\">%s</EnumVal>\n",pEnum->ord, pEnum->EnumVal);					
						}
					}
					
				}
			} 

			if (strcmpi(pDa->bType, "Enum") == 0) {
				int i = 0;
				if (sclGetEnumValByDaType(&sclInfo, pDa->type, &(lnInfo.enumvalHead))) continue;
				SCL_ENUMVAL* pEnum;
				for (pEnum =  lnInfo.enumvalHead; pEnum != NULL; pEnum = (SCL_ENUMVAL*)list_get_next( lnInfo.enumvalHead, pEnum)) {
					printf ("		<EnumVal ord=\"%d\">%s</EnumVal>\n",pEnum->ord, pEnum->EnumVal);					
				}
			} 			
		}		
	}
	
#endif
	
	release_scd_file(&sclInfo);
	
	printf("Parse scl file %s Result=%d\n", xmlFileName, rc);
	

	//slog_start(SX_LOG_ALWAY, LOG_MEM_EN, NULL);
	slog_end();
	//slog("main: parse %s, rc=%d\n", xmlFileName, rc);

#ifdef WIN32
	system("pause");
#endif

	return 0;
}