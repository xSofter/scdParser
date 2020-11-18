/*
 * @Date: 2020-11-16 08:56
 * @LastEditTime: 2020-11-16 09:57
 * @LastEditors: tangkai3
 * @Description: 
 */
#ifndef __SCDPARSE_EXPORT_H__
#define __SCDPARSE_EXPORT_H__
// __declspec(dllexport) for win32
#if defined(_WIN32)	
#define SCDPAESE_API  __declspec(dllexport)
#endif

#if defined(linux)
#define SCDPAESE_API
#endif

#endif