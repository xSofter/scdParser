/*
 * @Date: 2020-12-07 14:26
 * @LastEditTime: 2020-12-22 14:13
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

// #define DEBUG_PRINT


int main(int argc, char* argv[])
{
	const char *xmlFileName = "Template_3620.icd";// "Template_3620.icd"
	// const char *iedName = "C5011";
	// const char *accessPointName = "S1";
	// char* xmlFileName = "Template_3620.icd";
	int rc;
	argc = argc;
	argv = argv;

	SCL_INFO sclInfo;
	
	memset(&sclInfo, 0, sizeof(SCL_INFO));
	// memset(&userInfo, 0, sizeof(SCL_USER));
	struct timeval start, end;
	gettimeofday(&start, NULL);	
	
	rc = load_scd_file(xmlFileName, &sclInfo);
	if (rc != 0 )
	{
		printf("Parse scl file %s Error. Result=%d\n", xmlFileName, rc);
		return -1;
	}
	
	gettimeofday(&end, NULL);		
	int cost = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
	printf("Parse scl file %s Result %s cost time %d ms\n", xmlFileName, rc == 0 ? "Success": "Failed" , cost/1000);
	
#ifdef DB_SQLITE3
	sqlite3* db;
	rc = createDatabase(&db);
	if (rc != 0)
	{
		printf("Create database failed %d\n", rc);
		return -1;
	}
#endif

	scdGetIedStructInfo(&sclInfo, db);
	scdGetCommuncationInfo(&sclInfo, db);
	scdGetDataSetInfo(&sclInfo, db);
	sclGetDoiNameValue(&sclInfo, db);
	sclGetUrcbElements(&sclInfo, db);
	sclGetBrcbElements(&sclInfo, db);
	sclGetLogControlBack(&sclInfo, db);

	release_scd_file(&sclInfo);
	// slog_end();

#ifdef DB_SQLITE3	
	sqlite3_close(db);
#endif

#ifdef WIN32
	system("pause");
#endif

	return 0;
}

