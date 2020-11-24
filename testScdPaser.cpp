/************************************************************************/
/* auth: luolinglu
   fun:  test sclpase functon
*/
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <QDebug>
#include <QDateTime>
#include "scl.h"
#include "slog.h"

//static char *thisFileName;
#define debugMsg qDebug() << QString("[%1 %2: %3 %4]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(__FILE__).arg(__FUNCTION__).arg(__LINE__)
#define warnMsg qWarning() << QString("[%1 %2: %3 %4]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(__FILE__).arg(__FUNCTION__).arg(__LINE__)

#define errMsg qCritical() << QString("[%1 %2: %3 %4]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(__FILE__).arg(__FUNCTION__).arg(__LINE__)
#define panicMsg  qFatal() << QString("[%1 %2: %3 %4]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(__FILE__).arg(__FUNCTION__).arg(__LINE__)
#define LOG_FILE_NAME "scdpase.log"
int main(int argc, char* argv[])
{
	//thisFileName = __FILE__;

	ST_CHAR *xmlFileName = "Template.icd";//"jilindong.scd"; //"TXJD01.cid";
	ST_CHAR *iedName = "C5011";
	ST_CHAR *accessPointName = "S1";
	ST_RET rc;

	
	SCL_INFO sclInfo;
	memset(&sclInfo, 0, sizeof(SCL_INFO));

	SCL_LD* ld;
	SCL_LN* ln;
	SCL_DOI* doi;
	SCL_DAI* dai;
	SCL_SDI* sdi;

	SCL_SUBNET *net;
	SCL_CAP *cap;
	SCL_GSE *gse;

	SCL_LNTYPE *lnt;
	SCL_DO* don;

	SCL_DOTYPE *dot;
	SCL_DA *dan;

	int i=0;

	slog_start(SX_LOG_ALWAY, LOG_FILE_EN, LOG_FILE_NAME);
	rc = scl_parse(xmlFileName, iedName, accessPointName, &sclInfo);

#if 0
	for(ld = sclInfo.ldHead; ld!=NULL; ld = (SCL_LD*)list_get_next (sclInfo.ldHead, ld))
	{
		printf("ld:%s\n",ld->inst); //ld:MONT
		for(ln = ld->lnHead; ln!=NULL; ln = (SCL_LN*)list_get_next (ld->lnHead, ln))
		{
			printf("  Ln:%s, %s, %s\n",ln->lnClass, ln->lnType, ln->varName); // Ln:GGIO, GDNR_EVE_GGIO_NSR_3620_DS, AlmGGIO1
			for(doi = ln->doiHead; doi!=NULL; doi = (SCL_DOI*)list_get_next (ln->doiHead, doi))
			{
				printf("    DO:%s, %s\n",doi->name, doi->desc); // DO:COppm, CO一氧化碳浓度
				for(dai = doi->daiHead; dai!=NULL; dai = (SCL_DAI*)list_get_next (doi->daiHead, dai))
				{
					printf("      DAI:%s, %s, %s, %s, %d\n",dai->flattened, dai->Val, dai->desc, dai->sAddr, dai->sGroup); //DAI:mag$dU, CO一氧化碳度
				}
				for(sdi = doi->sdiHead; sdi!=NULL; sdi = (SCL_SDI*)list_get_next(doi->sdiHead, sdi))
				{
					printf("      SDI:%s, %s\n",sdi->flattened, sdi->desc); //SDI:mag, (null)
				}
			}

			for(SCL_DATASET* ds = ln->datasetHead; ds != NULL; ds = (SCL_DATASET*)list_get_next(ln->datasetHead, ds))
			{
				printf("  DataSet:%s, %s\n", ds->name, ds->desc); //DataSet:dsLog, 鏃ュ織璁板綍鏁版嵁闆?
				for(SCL_FCDA* fcda = ds->fcdaHead; fcda != NULL; fcda = (SCL_FCDA*)list_get_next(ds->fcdaHead, fcda)) 
				{
					printf("      FCDA:%s, %s, %s, %s\n",fcda->doName, fcda->daName, fcda->domName, fcda->fc); //FCDA:Alm32, , TEMPLATELD0, ST
				}
			}

			for(SCL_RCB *rcb = ln->rcbHead; rcb != NULL; rcb = (SCL_RCB *)list_get_next(ln->rcbHead, rcb))
			{
				printf("  rcb:%s, %s, %s, %d, %s\n", rcb->name, rcb->datSet, rcb->rptID, rcb->maxClient, rcb->desc); //rcb:brcbRelayEna, dsRelayEna, TEMPLATEPROT/LLN0$brcbRelayEna, 16, (null)
			}
		}
	}
#endif
	//解析communication,
	
	for(net = sclInfo.subnetHead; net!=NULL; net = (SCL_SUBNET*)list_get_next (sclInfo.subnetHead, net))
	{
		debugMsg << QString("net:%1 %2").arg(net->name).arg(net->desc);
	
		for(cap = net->capHead; cap!=NULL; cap = (SCL_CAP *)list_get_next (net->capHead, cap))
		{
			// if (cap->addr->IP == '\0' || cap->addr->IPSUBNET == '\0') continue;
			SCL_PORT *port;
			for (port = cap->portHead; port != NULL; port = (SCL_PORT *)list_get_next (cap->portHead, port)) {
				debugMsg << QString("Port: %1").arg(port->portCfg);
			}
			for (gse = cap->gseHead; gse != NULL; gse = (SCL_GSE *)list_get_next (cap->gseHead, gse)) {
				debugMsg << QString("   cap:Name %5 MAC %1 APPID %2 min %3 max %4").arg(gse->MAC).arg(gse->APPID).arg(gse->minTime).arg(gse->maxTime).arg(gse->cbName);
			}
			
		}
	}
	SCL_ACCESSPOINT *acPoint;
	
	for (acPoint = sclInfo.accessPointHead; acPoint != NULL; acPoint = (SCL_ACCESSPOINT *)list_get_next(sclInfo.accessPointHead, acPoint))
	{
		debugMsg << "==============================AccessPoint=================================" ;
		debugMsg << QString("AccessPoint Name= %1 desc=%2 ").arg(acPoint->name).arg(acPoint->desc);
		debugMsg << "==============================LDevice Start===============================" ;
		SCL_LD *scl_ld;
		SCL_LN *scl_ln;
		for (scl_ld = acPoint->ldHead; scl_ld != NULL; scl_ld = (SCL_LD *)list_get_next(acPoint->ldHead, scl_ld)){
			debugMsg << QString("LDevice inst= %1 desc=%2 apName=%3").arg(scl_ld->inst).arg(scl_ld->desc).arg(scl_ld->apName);
			// sclInfo.LLN0
			for (scl_ln = scl_ld->lnHead; scl_ln != NULL; scl_ln = (SCL_LN *)list_get_next(scl_ld->lnHead, scl_ln)) {
				debugMsg << QString("   LN:%1 %2 %3 %4 %5 %6").arg(scl_ln->varName).arg(scl_ln->desc).arg(scl_ln->lnType).arg(scl_ln->lnClass).arg(scl_ln->prefix).arg(scl_ln->inst);
				for(SCL_DATASET* ds = scl_ln->datasetHead; ds != NULL; ds = (SCL_DATASET*)list_get_next(scl_ln->datasetHead, ds))
				{
					debugMsg << QString("  DataSet: name %1 Desc %2").arg(ds->name).arg(ds->desc); //
					for(SCL_FCDA* fcda = ds->fcdaHead; fcda != NULL; fcda = (SCL_FCDA*)list_get_next(ds->fcdaHead, fcda)) 
					{
						debugMsg << QString("    FCDA ldInst=%1 prefix=%2 lnClass=%3 lnInst=%4 doName=%5 daName=%6 fc=%7").arg(fcda->ldInst).arg(fcda->prefix).arg(fcda->lnClass).arg(fcda->lnInst).arg(fcda->doName).arg(fcda->daName).arg(fcda->fc); //FCDA:Alm32, , TEMPLATELD0, ST
					}
				}
			}
		}
		debugMsg << "==============================LDevice End===================================" ;
	}
	
	
 #if 0
	for(dot = sclInfo.doTypeHead; dot!=NULL; dot = (SCL_DOTYPE*)list_get_next (sclInfo.doTypeHead, dot))
	{
		printf("do type:%s, %s\n",dot->id, dot->cdc);  //do type:CN_INC_Mod, INC
		for(dan = dot->daHead; dan!=NULL; dan = (SCL_DA*)list_get_next (dot->daHead, dan))
		{
			printf("  da:%s, %s, %s\n",dan->name, dan->bType, dan->fc); //da:ctlModel, Enum, CF
		}
		for(don = dot->sdoHead; don!=NULL; don = (SCL_DO*)list_get_next(dot->sdoHead, don))
		{
			printf("  do:%s, %s\n",don->name, don->type); //do:CmbuGasAlm, CN_SPS
		}
	}
#endif
	

	scl_info_destroy(&sclInfo);
	
	debugMsg << QString("main: parse %1, rc=%2").arg(xmlFileName).arg(rc);
	

	//slog_start(SX_LOG_ALWAY, LOG_MEM_EN, NULL);
	slog_end();
	//slog("main: parse %s, rc=%d\n", xmlFileName, rc);

#ifdef WIN32
	system("pause");
#endif

	return 0;
}