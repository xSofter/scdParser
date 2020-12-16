/*
 * @Date: 2020-12-07 09:27
 * @LastEditTime: 2020-12-15 10:39
 * @LastEditors: tangkai3
 * @Description: 
 */
#ifndef SCLPUB_H
#define SCLPUB_H

#ifdef __cplusplus
extern "C" {
#endif
#include "glbtypes.h"
#include "mem_chk.h"
#include "slog.h"
#include "scl.h"


int load_scd_file(const char* fileName, SCL_INFO* sclInfo);
void release_scd_file(SCL_INFO* sclInfo);
int scdGetCommunicationInfo(SCL_INFO* sclInfo, SCL_USER* user);
int scdGetDataSetInfo(SCL_INFO* sclInfo, SCL_USER* user);
int sclGetDoiNameValue(SCL_INFO* sclInfo, SCL_USER* user);
int sclGetDOInfoByLnType(SCL_INFO* sclInfo, const char* lnType, SCL_DO** lnInfo);
int sclGetDAListByDoType(SCL_INFO* sclInfo, const char* DoType, SCL_DA** pDAInfo);
int sclGetBdaByDaType(SCL_INFO* sclInfo, const char* DaType, SCL_BDA** pBdaHead);
int sclGetEnumValByDaType(SCL_INFO* sclInfo, const char* DaType, SCL_ENUMVAL** enumvalHead);
void release_scd_userInfo(SCL_USER* userInfo);
#ifdef __cplusplus
}
#endif
#endif /* SCLPUB_H */