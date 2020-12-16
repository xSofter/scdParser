/*
 * @Date: 2020-12-15 11:18
 * @LastEditTime: 2020-12-16 10:10
 * @LastEditors: tangkai3
 * @Description: 
 */
#ifndef __MYSQLDB_H__
#define __MYSQLDB_H__
#ifdef __cplusplus
extern "C" {
#endif
#define SQLITE_3

#ifdef SQLITE_3
#include <sqlite3.h>
#endif
enum table_id
{
    IEC_IED_STRUCT = 0,
	IEC_IED_COMMU,
	IEC_IED_DATASET,

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