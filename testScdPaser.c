/*
 * @Date: 2020-12-07 14:26
 * @LastEditTime: 2020-12-16 16:24
 * @LastEditors: tangkai3
 * @Description: 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <QDebug>
// #include <QDateTime>
#include "sclPub.h"
#include "storage.h"
#include "str_util.h"
#include <sys/time.h>

/**
 * @Description: 保存通信配置表
 */
void testScdSaveCommuncation(SCL_INFO* sclInfo, SCL_USER* userInfo, sqlite3* db) {
	if (!sclInfo || !userInfo) return;
	if (scdGetCommunicationInfo(sclInfo, userInfo))
	{
		return;
	}

	char* zErrMsg = 0;
	char tabsql[1024 + 1] = {0};
	struct timeval start, end;
	SCL_COMM* pComm = userInfo->pCommHead;	

	gettimeofday(&start, NULL);

	sqlite3_exec(db,"begin;",0,0,0);
	sqlite3_stmt *stmt1, *stmt2, *stmt3; 

	strcpy(tabsql, "insert into iec_ied_commu(iedname,substation,apName,ipAddress,ipSubNet) values (?,?,?,?,?)");
	if (SQLITE_OK != sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt1,0)) {
		printf("sqlite3_prepare_v2 %s error\n", tabsql);
		if (stmt1) sqlite3_finalize(stmt1);
	}	

	strcpy(tabsql, "insert into iec_ied_commu_gse(subnetId,mac_address,vlan_id,vlan_priority,appid,mintime,maxtime) values (?,?,?,?,?,?,?)");
	if (SQLITE_OK != sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt2,0)) {
		printf("sqlite3_prepare_v2 %s error\n", tabsql);
		if (stmt2) sqlite3_finalize(stmt2);
	}
	strcpy(tabsql, "insert into iec_ied_commu_phyConn(subnetId,port) values (?,?)");
	if (SQLITE_OK != sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt3,0)) {
		printf("sqlite3_prepare_v2 %s error\n", tabsql);
		if (stmt3) sqlite3_finalize(stmt3);
	}
	int subNetworkid = 1;
	for (pComm = userInfo->pCommHead; pComm != NULL; pComm = (SCL_COMM *)list_get_next(userInfo->pCommHead, pComm)) {
		// printf ("<SubNetwork name=\"%s\" type=\"%s\">\n",pComm->subNetWorkName, pComm->subNetWorkType);
		sqlite3_reset(stmt1);
		sqlite3_bind_text(stmt1, 1, sclInfo->lIEDHead->iedName, strlen(sclInfo->lIEDHead->iedName), NULL);
		sqlite3_bind_text(stmt1, 2, pComm->subNetWorkName, strlen(pComm->subNetWorkName), NULL);
		sqlite3_bind_text(stmt1, 3, pComm->apName, strlen(pComm->apName), NULL);

		if (pComm->address.IP && pComm->address.IPSUBNET) {
			sqlite3_bind_text(stmt1, 4, pComm->address.IP, strlen(pComm->address.IP), NULL);
			sqlite3_bind_text(stmt1, 5, pComm->address.IPSUBNET, strlen(pComm->address.IPSUBNET), NULL);
		}
		sqlite3_step(stmt1); 
	
		//renew gse
		SCL_GSE* gse;
		for (gse = pComm->gseHead; gse != NULL; gse = (SCL_GSE *)list_get_next(pComm->gseHead, gse)) {
			// printf("  GSE: ldInst: %s cbName %s\n  MAC %s APPID %d VLAN %d VLANPORI %d min %d max %d\n", 
			// 			gse->ldInst, gse->cbName, 
			// 			gse->MAC,  gse->APPID,  gse->VLANID,  gse->VLANPRI, gse->minTime, gse->maxTime);
			sqlite3_reset(stmt2);
			sqlite3_bind_int(stmt2, 1,subNetworkid);
			sqlite3_bind_text(stmt2, 2, gse->MAC, strlen(gse->MAC), NULL);
			sqlite3_bind_int(stmt2, 3, gse->VLANID);
			sqlite3_bind_int(stmt2, 4, gse->VLANPRI);
			sqlite3_bind_int(stmt2, 5, gse->APPID);
			sqlite3_bind_int(stmt2, 6, gse->minTime);
			sqlite3_bind_int(stmt2, 7, gse->maxTime);
			sqlite3_step(stmt2); 
		}

		SCL_PORT* port;
		for (port = pComm->portHead; port != NULL; port = (SCL_PORT *)list_get_next(pComm->portHead, port)) {
			// printf("    Port: %s\n", port->portCfg);
			sqlite3_reset(stmt3);
			sqlite3_bind_int(stmt3, 1,subNetworkid);
			sqlite3_bind_text(stmt3, 2, port->portCfg, strlen(port->portCfg), NULL);
			sqlite3_step(stmt3); 
		}
		subNetworkid++;
	}

	sqlite3_finalize(stmt1);  
	sqlite3_finalize(stmt2);
	sqlite3_finalize(stmt3);
	if (SQLITE_OK != sqlite3_exec(db,"commit;",0,0,&zErrMsg))
	{
		printf("SQL error: %s\n", zErrMsg);	
		sqlite3_free(zErrMsg);		
	}
	//test proformence
	gettimeofday(&end, NULL);

	int cost = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	printf ("Succeess count time: %d ms\n", cost/1000);		
	return;	
}

/**
 * @Description: 保存dataSet表
 */
void testScdSaveDataSet(SCL_INFO* sclInfo, SCL_USER* userInfo, sqlite3* db) {
	if (!sclInfo || !userInfo) return;
	if (scdGetDataSetInfo(sclInfo, userInfo)) return;
	char* zErrMsg = 0;
	char tabsql[1024 + 1] = {0};	
	
	SCL_DATASET* dataSet = userInfo->pDataSet;
	SCL_FCDA* fcda;
	struct timeval start, end;

	gettimeofday(&start, NULL);
	sqlite3_exec(db,"begin;",0,0,0);
	sqlite3_stmt *stmt; 
	//total 11 elements
	strcpy(tabsql, "insert into iec_ied_dataset(iedname,ldInst,prefix,lnClass,lnInst,doName,daName,fc,fcda,desc,sAddr) values (?,?,?,?,?,?,?,?,?,?,?)");
	if (SQLITE_OK != sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt,0)) {
		printf("error sqlite3_prepare_v2\n");
		if (stmt) sqlite3_finalize(stmt);
	}
	for (dataSet = userInfo->pDataSet; dataSet != NULL; dataSet = (SCL_DATASET *)list_get_next(userInfo->pDataSet, dataSet)) {
		// printf("<DataSet: name %s Desc %s>\n", dataSet->name, dataSet->desc);
		for (fcda= dataSet->fcdaHead; fcda != NULL; fcda = (SCL_FCDA *)list_get_next(dataSet->fcdaHead, fcda)) {
			sqlite3_reset(stmt);
			sqlite3_bind_text(stmt, 1, sclInfo->lIEDHead->iedName, strlen(sclInfo->lIEDHead->iedName), NULL);
			sqlite3_bind_text(stmt, 2, fcda->ldInst, strlen(fcda->ldInst), NULL);
			sqlite3_bind_text(stmt, 3, fcda->prefix, strlen(fcda->prefix), NULL);
			sqlite3_bind_text(stmt, 4, fcda->lnClass, strlen(fcda->lnClass), NULL);
			sqlite3_bind_text(stmt, 5, fcda->lnInst, strlen(fcda->lnInst), NULL);
			sqlite3_bind_text(stmt, 6, fcda->doName, strlen(fcda->doName), NULL);
			sqlite3_bind_text(stmt, 7, fcda->daName, strlen(fcda->daName), NULL);
			sqlite3_bind_text(stmt, 8, fcda->fc, strlen(fcda->fc), NULL);
			sqlite3_bind_text(stmt, 9, fcda->domName, strlen(fcda->domName), NULL);
			sqlite3_bind_text(stmt, 10, fcda->doRefDesc, strlen(fcda->doRefDesc), NULL);
			sqlite3_bind_text(stmt, 11, fcda->doRefsAddr, strlen(fcda->doRefsAddr), NULL);
			
			sqlite3_step(stmt); 
		}
	}
	sqlite3_finalize(stmt);  
	if (SQLITE_OK != sqlite3_exec(db,"commit;",0,0,&zErrMsg))
	{
		printf("SQL error: %s\n", zErrMsg);	
		sqlite3_free(zErrMsg);		
	}
	//test proformence
	gettimeofday(&end, NULL);

	int cost = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	printf ("Succeess count time: %d ms\n", cost/1000);	

}

void testScdGetLnInfo(SCL_INFO* sclInfo, SCL_USER* userInfo, sqlite3* db) {
	if (!sclInfo || !userInfo) return;
	SCL_LNINFO* lninfo;
	char* zErrMsg = 0;
	char tabsql[1024 + 1] = {0};
	int rc = sclGetDoiNameValue(sclInfo, userInfo);
	if ( rc != 0){
		printf ("Error sclGetDoiNameValue failed rc %d\n", rc);
		return;
	}
	
	struct timeval start, end;

	gettimeofday(&start, NULL);
	sqlite3_exec(db,"begin;",0,0,0);
	sqlite3_stmt *stmt; 

	strcpy(tabsql, "insert into iec_ied_struct(iedname,LD,LN,FC,DOI,DAI,ref,sAddr,val,val_type,val_size,ref_type,ref_size) values (?,?,?,?,?,?,?,?,?,?,'0',0,-255)");
	if (SQLITE_OK != sqlite3_prepare_v2(db,tabsql,strlen(tabsql),&stmt,0)) {
		printf("error sqlite3_prepare_v2\n");
		if (stmt) sqlite3_finalize(stmt);
	}
	for (lninfo = userInfo->pLnInfo; lninfo != NULL; lninfo = (SCL_LNINFO *)list_get_next(userInfo->pLnInfo, lninfo)) {
		// printf ("index %d LNInfo %-5s %-10s %-10s %-10s %-10s %-5s %-5s %-5s %-5s %-5s %-5s\n", 
		// index, lninfo->ldinst, lninfo->lnVarName, lninfo->lnDesc, lninfo->doiName, 
		// lninfo->doiDesc, lninfo->daiName, lninfo->daisAddr, lninfo->daiVal, 
		// lninfo->daiType, lninfo->ref, lninfo->fc);
		// index++;
		sqlite3_reset(stmt);
		sqlite3_bind_text(stmt, 1, sclInfo->lIEDHead->iedName, strlen(sclInfo->lIEDHead->iedName), NULL);
		sqlite3_bind_text(stmt, 2, lninfo->ldinst, strlen(lninfo->ldinst), NULL);
		sqlite3_bind_text(stmt, 3, lninfo->lnVarName, strlen(lninfo->lnVarName), NULL);
		sqlite3_bind_text(stmt, 4, lninfo->fc, strlen(lninfo->fc), NULL);
		sqlite3_bind_text(stmt, 5, lninfo->doiName, strlen(lninfo->doiName), NULL);
		sqlite3_bind_text(stmt, 6, lninfo->daiName, strlen(lninfo->daiName), NULL);
		sqlite3_bind_text(stmt, 7, lninfo->ref, strlen(lninfo->ref), NULL);
		sqlite3_bind_text(stmt, 8, lninfo->daisAddr , strlen(lninfo->daisAddr), NULL);
		if (lninfo->daiVal){
			sqlite3_bind_text(stmt, 9, lninfo->daiVal, strlen(lninfo->daiVal), NULL);
		} else{
			sqlite3_bind_text(stmt, 9, "null", strlen("null"), NULL);
		}
		
		sqlite3_bind_text(stmt, 10, lninfo->daiType, strlen(lninfo->daiType), NULL);
		sqlite3_step(stmt);  

		// snprintf(tabsql, 1024, "insert into %s(%s) values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',0,-255)",
		// 		mySqlTableName[IEC_IED_STRUCT],
		// 		"iedname,LD,LN,FC,DOI,DAI,ref,sAddr,val,val_type,val_size,ref_type,ref_size",
		// 		sclInfo->lIEDHead->iedName,
		// 		lninfo->ldinst, 
		// 		lninfo->lnVarName, 
		// 		lninfo->fc,
		// 		lninfo->doiName, 
		// 		lninfo->daiName, 
		// 		lninfo->ref,
		// 		lninfo->daisAddr,
		// 		lninfo->daiVal, 
		// 		lninfo->daiType,
		// 		"0"
		// );
		
		// rc = sqlite3_exec(db, tabsql,0,0,0);
		// if( rc != SQLITE_OK ){
		// 	// fprintf(stderr, "SQL error: %s\n", zErrMsg);
		// 	printf("SQL error: %s\n", zErrMsg);
		// 	sqlite3_free(zErrMsg);
		// } 	
			
	}
	sqlite3_finalize(stmt);  
	if (SQLITE_OK != sqlite3_exec(db,"commit;",0,0,&zErrMsg))
	{
		printf("SQL error: %s\n", zErrMsg);	
		sqlite3_free(zErrMsg);		
	}
	//test proformence
	gettimeofday(&end, NULL);

	int cost = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	printf ("Succeess count time: %d ms\n", cost/1000);
}


int main(int argc, char* argv[])
{
	const char *xmlFileName = "Template.icd";//"jilindong.scd"; //"TXJD01.cid";
	// const char *iedName = "C5011";
	// const char *accessPointName = "S1";
	int rc;

	SCL_INFO sclInfo;
	SCL_USER userInfo;
	memset(&sclInfo, 0, sizeof(SCL_INFO));
	memset(&userInfo, 0, sizeof(SCL_USER));
	sqlite3* db;
	rc = load_scd_file(xmlFileName, &sclInfo);
	
	if (rc != 0 )
	{
		printf("Parse scl file %s Error. Result=%d\n", xmlFileName, rc);
		return -1;
	}

	rc = createDatabase(&db);
	if (rc != 0)
	{
		printf("Create database failed %d\n", rc);
		return -1;
	}

	testScdSaveCommuncation(&sclInfo, &userInfo, db);
	testScdSaveDataSet(&sclInfo, &userInfo, db);
	testScdGetLnInfo(&sclInfo, &userInfo, db);

	release_scd_file(&sclInfo);
	release_scd_userInfo(&userInfo);
	printf("Parse scl file %s Result %s\n", xmlFileName, rc == 0 ? "Success": "Failed");
	
	//slog_start(SX_LOG_ALWAY, LOG_MEM_EN, NULL);
	// slog_end();
	//slog("main: parse %s, rc=%d\n", xmlFileName, rc);
	sqlite3_close(db);
#ifdef WIN32
	system("pause");
#endif

	return 0;
}

