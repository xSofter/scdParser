/*
 * @Date: 2020-12-15 11:18
 * @LastEditTime: 2020-12-22 14:11
 * @LastEditors: tangkai3
 * @Description: 
 */
#ifndef __MYSQLDB_H__
#define __MYSQLDB_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <sqlite3.h>

enum table_id
{
	IEC_IED_STRUCT = 0,
    IEC_IED_DATA,
	IEC_IED_COMM_PHYCONN, 
	IEC_IED_COMM_GSE,
	IEC_IED_COMM,
	IEC_IED_DATASET,
	IEC_IED_LCB,
	IEC_IED_URCB,
	IEC_IED_BRCB,

	IEC_IED_STRUCT_SCD,
	IEC_IED_COMMU_SCD,
	IEC_IED_DATASET_SCD,
    
    NEW_TABLE_MAX_NUM
};

// 外部使用函数
int createDatabase(sqlite3 **ppDb);
extern const char* mySqlTableName[];
extern const char* create_tab_statment[];
#ifdef __cplusplus
}
#endif

#endif