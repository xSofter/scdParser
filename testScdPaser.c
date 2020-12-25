/*
 * @Date: 2020-12-07 14:26
 * @LastEditTime: 2020-12-25 08:36
 * @LastEditors: tangkai3
 * @Description: 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sclPub.h"
#include "storage.h"
#include "str_util.h"

int main(int argc, char* argv[])
{
	if(!argv[1] || argc < 2) {
		printf("Eroor: no input file.\nUsage: sclParse sample.icd, sample.cid or sample.scd...\n");
		exit(-1);
	}

	const char *xmlFileName = argv[1];

	int rc;

	SCL_INFO sclInfo;
#ifndef DB_SQLITE3	
	void* db = NULL;
#endif
	memset(&sclInfo, 0, sizeof(SCL_INFO));

	struct timeval start, end;
	gettimeofday(&start, NULL);	
	
	rc = load_scd_file(xmlFileName, &sclInfo);
	if (rc != 0 )
	{
		printf("Parse scl file %s Error. Result=%d\n", xmlFileName, rc);
		return -1;
	}
	
	gettimeofday(&end, NULL);		

	long cost = 1000 *(end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000);
	printf("Parse scl file %s Result %s cost time %ld ms\n", xmlFileName, rc == 0 ? "Success": "Failed" , cost);
	
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
	sclGetLogControlBlock(&sclInfo, db);

	release_scd_file(&sclInfo);

#ifdef DB_SQLITE3	
	sqlite3_close(db);
#endif

#ifdef WIN32
	system("pause");
#endif

	return 0;
}

