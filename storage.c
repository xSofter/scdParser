/*
 * @Date: 2020-12-15 11:19
 * @LastEditTime: 2020-12-22 19:06
 * @LastEditors: tangkai3
 * @Description: 
 */

#include "storage.h"
#include <stdio.h>
#include <string.h>

const char* mySqlTableName[] = {
    //for single ied use
    "iec_ied_struct",
    "iec_ied_data",
    "iec_ied_commu_phyConn",
    "iec_ied_commu_gse",
    "iec_ied_commu",
	"iec_ied_dataset",
    "iec_ied_logControlBlock",
    "iec_ied_urcb",
    "iec_ied_brcb",
    //for SCD use
	"iec_ied_struct_from_scd",
	"iec_ied_commu_from_scd",
	"iec_ied_dataset_from_scd"
};

const char* create_tab_statment[] = 
{
    //iec_ied_struct
    "iedname varchar(32) NOT NULL PRIMARY KEY,\
    type varchar(32),\
    desc varchar(64),\
    manufacturer varchar(32),\
    configVersion varchar(8),\
    iedCrc varchar(8),\
    substationCrc varchar(8)",
    
    //iec_ied_data
    "id integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    iedname varchar(32) NOT NULL,\
    LD varchar(64)  NOT NULL, \
    LN varchar(64)  NOT NULL, \
    FC varchar(8)  NOT NULL, \
    DOI varchar(64)  NOT NULL, \
    DAI varchar(64)  NOT NULL, \
    dataset varchar(32), \
    fcda varchar(128) ,\
    fcda_sqnum int ,\
    cb varchar(32) , \
    cb_type varchar(8), \
    ref varchar(128)  NOT NULL,\
    sAddr varchar(128), \
    ref_flag int(32)  default 0,\
    ref_type int  NOT NULL,\
    ref_size int  NOT NULL,\
    val varchar(256),\
    val_type int,\
    val_size int",

    //iec_ied_commu_phyConn
    "portId integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    subnetId integer,\
    apname varchar(4),\
    port varchar(64),\
    FOREIGN KEY(subnetId) REFERENCES iec_ied_commu(subnetId) on delete cascade",

    //iec_ied_commu_gse
    "gseId integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    subnetId integer,\
    mac_address varchar(32), \
    vlan_id integer,\
    vlan_priority integer ,\
    appid integer,\
    mintime integer,\
    maxtime integer,\
    FOREIGN KEY(subnetId) REFERENCES iec_ied_commu(subnetId) on delete cascade",
    
    //iec_ied_commu
    "subnetId integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    iedname varchar(32) NOT NULL,\
    substation varchar(32) NOT NULL,\
    apname varchar(4) NOT NULL,\
    ipAddress varchar(64), \
    ipSubNet varchar(64)",

    //iec_ied_dataset
    "id integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    iedname varchar(32) NOT NULL,\
    ldInst varchar(32)  NOT NULL,\
    dsName varchar(32) NOT NULL,\
    prefix varchar(32) ,\
    lnClass varchar(32),\
    lnInst varchar(32), \
    doName varchar(32), \
    daName varchar(32), \
    fc     varchar(4), \
    fcda varchar(128),\
    desc varchar(128),\
    sAddr varchar(128) ",

    //iec_ied_logControlBlock
    "id integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    iedname varchar(32) NOT NULL,\
    logRef varchar(64) NOT NULL,\
    logDataSet varchar(128) NOT NULL,\
    lcbDesc varchar(64), \
    logEna BOOLEAN NOT NULL,\
    intgPd integer NOT NULL,\
    trgOps int8 NOT NULL",

    //iec_ied_urcb
    "id integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    urcbName varchar(64) NOT NULL,\
    rptId varchar(64) NOT NULL,\
    dataset varchar(64) NOT NULL, \
    confRev integer NOT NULL,\
    trgOps integer NOT NULL,\
    optFlds integer NOT NULL,\
    rptEna BOOLEAN NOT NULL",    

    //iec_ied_brcb
    "id integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    brcbName varchar(64) NOT NULL,\
    rptId varchar(64) NOT NULL,\
    dataset varchar(64) NOT NULL, \
    bufferTm integer,\
    confRev integer NOT NULL,\
    trgOps integer NOT NULL,\
    optFlds integer NOT NULL,\
    rptEna BOOLEAN NOT NULL",     

    //iec_ied_struct_from_scd
    "id integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    iedname varchar(32) NOT NULL,\
    LD varchar(64)  NOT NULL, \
    LN varchar(64)  NOT NULL, \
    FC varchar(8)  NOT NULL, \
    DOI varchar(64)  NOT NULL, \
    DAI varchar(64)  NOT NULL, \
    dataset varchar(32) ,\
    fcda varchar(128) ,\
    fcda_sqnum int ,\
    cb varchar(32) , \
    cb_type varchar(8), \
    ref varchar(128)  NOT NULL,\
    ref_flag int(32)  default 0,\
    ref_type int  NOT NULL,\
    ref_size int  NOT NULL,\
    val varchar(256),\
    val_type varchar(32),\
    val_size int",

    //iec_ied_commu_from_scd
    "id integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    substation varchar(32) NOT NULL,\
    iedname varchar(32) NOT NULL,\
    ip varchar(64)",

    //iec_ied_dataset_from_scd
    "id integer NOT NULL PRIMARY KEY AUTOINCREMENT,\
    iedname varchar(32) NOT NULL,\
    LD varchar(64)  NOT NULL,\
    dataset varchar(32) ,\
    fcda varchar(128) ",
};

int createDatabase(sqlite3 **ppDb)
{
	char tabsql[1024] = {0};
    sqlite3 *database;
    int rc = sqlite3_open("sclParse.db", &database);
    if (rc != 0) {
        printf ("Open database failed\n");
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(database));
        return -1;
    }
    sqlite3_exec(database,"PRAGMA synchronous = OFF; ",0,0,0);  //关闭同步,加快写入速度
    sqlite3_exec(database,"PRAGMA foreign_keys = ON; ",0,0,0);  //支持外键表
    char *zErrMsg = 0;
  
    int i=0;
	for(i = 0; i < NEW_TABLE_MAX_NUM; i++)
	{
		memset(tabsql, 0, sizeof(tabsql));
		sprintf(tabsql, "drop table if exists %s", mySqlTableName[i]);		
		// db.exec(tabsql);
        rc = sqlite3_exec(database, tabsql, 0, 0, &zErrMsg);
        if( rc != SQLITE_OK ){
            printf ("%s oper SQL %s error: %s\n", mySqlTableName[i], tabsql, zErrMsg);
            sqlite3_free(zErrMsg);
            return -1;
        }           

		memset(tabsql, 0, sizeof(tabsql));
		sprintf(tabsql, "create table if not exists %s(%s)", mySqlTableName[i], create_tab_statment[i]);		
		rc = sqlite3_exec(database, tabsql, 0, 0, &zErrMsg);
        if( rc != SQLITE_OK ){
            printf ("%s oper SQL %s error: %s\n", mySqlTableName[i], tabsql, zErrMsg);
            sqlite3_free(zErrMsg);
            return -1;
        }         

		// memset(tabsql, 0, sizeof(tabsql));
		// sprintf(tabsql, "DELETE FROM sqlite_sequence where name = %s", mySqlTableName[i]);
		// rc = sqlite3_exec(database, tabsql, 0, 0, &zErrMsg);
        // if( rc != SQLITE_OK ){
        //     // fprintf(stderr, "SQL error: %s\n", zErrMsg);
        //     printf ("DELETE table error: %s\n", zErrMsg);
        //     sqlite3_free(zErrMsg);
        //     return -1;
        // }else{
        //     // fprintf(stdout, "Table created successfully\n");
        //     printf("create table %s successfully\n",mySqlTableName[i]);
        // }  
	}
    
    *ppDb = database;
    return 0;
}