/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*      	  1986 - 2006, All Rights Reserved.		        */
/*									*/
/*		    PROPRIETARY AND CONFIDENTIAL			*/
/*									*/
/* MODULE NAME : glbtypes.h						*/
/* PRODUCT(S)  : MMSEASE						*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 03/18/08  JRB     30    Use endian.h to set SD_BYTE_ORDER on linux.	*/
/* 01/27/06  EJV     29    _WIN32: chg ST_UINT64 to unsigned __int64.	*/
/* 08/01/05  JRB     28    Disable SD_CONST for ALL platforms.		*/
/* 02/12/04  JRB     27    Disable ST_CONST for LINUX.			*/
/* 01/08/04  EJV     26    Checked SD_BIG_ENDIAN on sun, rem pragma msg.*/
/* 12/09/03  JRB     25    Add LYNX, don't def ST_CONST for LYNX.	*/
/* 02/19/03  JRB     24    Define SD_BYTE_ORDER for each system.	*/
/* 02/19/03  JRB     23    Del VAX-VMS & PSOS code.			*/
/* 02/17/03  CRM     22    Added "defined(linux)" code. 		*/
/* 02/17/03  JRB     21    Del unsupported __IC86__	*/
/*			   Del obsolete DEBUG_MMS, DEBUG_SUIC, etc.	*/
/*			   Del NEW_SYSTEM.. defines (not very useful).	*/
/* 02/27/02  JRB     20    Include compiler option file "glbopt.h".	*/
/* 10/05/01  EJV     19    sun supports ST_INT64 (long long)		*/
/* 01/02/01  EJV     18    disabled SD_CONST macro for _AIX		*/
/*			   enabled  SD_CONST macro for QNX		*/
/* 06/16/00  JRB     17    Define "*INT64" only if INT64_SUPPORT defined*/
/* 03/13/00  MDE     16    disabled SD_CONST macro for WIN32            */
/* 09/24/99  JRB     15    disabled SD_CONST macro for QNX              */
/* 09/13/99  MDE     14    Added SD_CONST modifiers			*/
/* 04/14/99  MDE     13    Changed SYSTEM_SEL defines			*/
/* 01/19/99  EJV     12    AIX: Added 64 bit integer support		*/
/* 08/26/98  EJV     11    Deleted CPU_SEL and all releated defines.	*/
/*			   __hpux supports 64-bit integers.		*/
/*			   Eliminated spaces at the end of lines.	*/
/* 02/09/98  RKR     10    Removed PRODUCT_ONLY_VMS_ALPHA		*/
/* 01/23/98  EJV     09    Digital UNIX: revised.			*/
/* 12/11/97  JRB     08    Cleaned up PSOS ifdefs. Define PSOS_SYSTEM	*/
/*			   to compile. Use new SYSTEM_SEL=SYS_PSOS.	*/
/* 12/11/97  JRB     07    Deleted _MRI ifdef (for Modicon/MICROTEC)	*/
/*			   Deleted EMBEDDED_6800 SYSTEM_SEL.		*/
/* 10/27/97  EJV     06    AIX: revised types, deleted pragma message()	*/
/* 09/23/97  JRB     05    QNX: pragma stops unused symbol warnings.	*/
/* 08/16/97  EJV     04    For QNX corrected ST_INT16, ST_UINT16.	*/
/* 08/15/97  JRB     03    Use MOTO_68000 consistently.			*/
/* 08/13/97  EJV     02    Changed #if (A && B) statements to avoid	*/
/*                         warning if A or B has not been defined.	*/
/* 06/20/97  MDE     01    Added 64 bit integer support for 95/NT	*/
/* 04/02/97  DTL   7.00    MMSEASE 7.0 release. See MODL70.DOC for	*/
/*			   history.					*/
/************************************************************************/

#ifndef GBLTYPES_INCLUDED
#define GBLTYPES_INCLUDED

//#include "glbopt.h"	/* Global compiler options.			*/

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/************************************************************************/
/************************************************************************/
/* General purpose defines, same for all platforms			*/

#define SD_TRUE		1
#define SD_FALSE	0		
#define SD_SUCCESS 	0
#define SD_FAILURE 	1
#define SD_BIG_ENDIAN		0
#define SD_LITTLE_ENDIAN	1
#define TRUE 1
#define FALSE 0

/* Define used for 'const' modifier 					*/
/* DEBUG: someday if all code is changed to consistently use SD_CONST,	*/
/*        this define may be replaced with the following:		*/
/* #define SD_CONST const						*/
#define SD_CONST  const
#define OX_LINUX
/************************************************************************/
/************************************************************************/
/************************************************************************/
/************************************************************************/

/* SYSTEM_SEL defines - bit masked					*/
#define	SYSTEM_SEL_MSOFT	0x0001
#define SYSTEM_SEL_OS2		0x0008
#define SYSTEM_SEL_OPEN_VMS	0x0010
#define SYSTEM_SEL_SYS_5	0x0020
#define SYSTEM_SEL_SYS_BSD	0x0040
#define SYSTEM_SEL_QNX_C86	0x0100
#define SYSTEM_SEL_SYSVXWORKS	0x0800
#define SYSTEM_SEL_SYS_QNX4	0x1000

/* For backwards compatibility only, do not use. Will be deleted soon.	*/
#if !defined(MSOFT)
#define	MSOFT		SYSTEM_SEL_MSOFT       
#endif
#if !defined(OS2)
#define	OS2		SYSTEM_SEL_OS2	       
#endif
#if !defined(OPEN_VMS)
#define	OPEN_VMS	SYSTEM_SEL_OPEN_VMS    
#endif
#if !defined(SYS_5)
#define	SYS_5		SYSTEM_SEL_SYS_5       
#endif
#if !defined(SYS_BSD)
#define	SYS_BSD		SYSTEM_SEL_SYS_BSD     
#endif
#if !defined(QNX_C86)
#define	QNX_C86		SYSTEM_SEL_QNX_C86     
#endif
#if !defined(SYSVXWORKS)
#define	SYSVXWORKS	SYSTEM_SEL_SYSVXWORKS  
#endif
#if !defined(SYS_QNX4)
#define	SYS_QNX4	SYSTEM_SEL_SYS_QNX4    
#endif

/************************************************************************/
/************************************************************************/
/* SYSTEM and CPU select defines. These are based on built in compiler	*/
/* defines which allow automatic detection of the compiler.		*/
/************************************************************************/

/************************************************************************/
/* MS-DOS and WINDOWS							*/
/************************************************************************/
#if defined(MSDOS) || defined(__MSDOS__)	/* Microsoft or Borland */

#if !defined(_WINDOWS) && !defined(_Windows)
#define PLAIN_DOS		/* straight and pure DOS */
#endif

#define SD_BYTE_ORDER	SD_LITTLE_ENDIAN
#define SYSTEM_SEL   	SYSTEM_SEL_MSOFT
#define SD_END_STRUCT

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int

/* We need specific sizes for these types				*/
#define ST_INT8   signed char     	
#define ST_INT16  signed short     	
#define ST_INT32  signed long     	
#define ST_UINT8  unsigned char     	
#define ST_UINT16 unsigned short    	
#define ST_UINT32 unsigned long    	

/* SD_TRUE or SD_FALSE only						*/
#define ST_BOOLEAN  unsigned char		


/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED
	
#endif

/************************************************************************/
/* WINDOWS 95/NT							*/
/************************************************************************/
#if defined(_WIN32)				/* VC++, 32-Bit		*/

#define SD_BYTE_ORDER	SD_LITTLE_ENDIAN
#define SYSTEM_SEL   	SYSTEM_SEL_MSOFT
#define SD_END_STRUCT	

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int		

/* We need specific sizes for these types				*/
#define ST_INT8     signed char     	
#define ST_INT16    signed short     	
#define ST_INT32    signed long     	
#define ST_INT64    __int64
#define ST_UINT8    unsigned char     	
#define ST_UINT16   unsigned short    	
#define ST_UINT32   unsigned long    	
#define ST_UINT64   unsigned __int64
#define ST_BOOLEAN  unsigned char		

/* This define shows that we really have support for 64 bit integers	*/
#define INT64_SUPPORT

/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED

#endif

/************************************************************************/
/* OS/2 - IBM C/SET2 and C/SET++    					*/
/************************************************************************/
#if defined(__OS2__) 		 		/* IBM C Set/2 	*/

#pragma message("Please look over the OS/2 system detect & defines")

#pragma data_seg(alldata)

#define SD_BYTE_ORDER	SD_LITTLE_ENDIAN
#define SYSTEM_SEL   	SYSTEM_SEL_OS2
#define SD_END_STRUCT 

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int		

/* We need specific sizes for these types				*/
#define ST_INT8   signed char     	
#define ST_INT16  signed short     	
#define ST_INT32  signed long     	
#define ST_UINT8  unsigned char     	
#define ST_UINT16 unsigned short    	
#define ST_UINT32 unsigned long    	
#define ST_BOOLEAN  unsigned char		

/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED

#endif

/************************************************************************/
/* SUN									*/
/************************************************************************/
#if defined(sun) 				/* SUN		*/

#define SD_BYTE_ORDER	SD_BIG_ENDIAN
#define	SYSTEM_SEL   	SYS_5
#define SD_END_STRUCT long end_of;  /* force struct to quad word allign	*/

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int		

/* We need specific sizes for these types				*/
#define ST_INT8   signed char     	
#define ST_INT16  signed short     	
#define ST_INT32  signed long     	
#define ST_INT64  signed long long
#define ST_UINT8  unsigned char     	
#define ST_UINT16 unsigned short    	
#define ST_UINT32 unsigned long    	
#define ST_UINT64 unsigned long long
#define ST_BOOLEAN  unsigned char		

/* This define shows that we really have support for 64 bit integers	*/
#define INT64_SUPPORT

/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED

#endif

/************************************************************************/
/* AIX									*/
/************************************************************************/
#if defined(_AIX)				/* AIX		*/

#define SD_BYTE_ORDER	SD_BIG_ENDIAN
#define	SYSTEM_SEL   	SYS_5
#define SD_END_STRUCT long end_of;  /* force struct to quad word allign	*/

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int

/* We need specific sizes for these types				*/
#define ST_INT8   signed char     	
#define ST_INT16  signed short     	
#define ST_INT32  signed long     	
#define ST_INT64  signed long long
#define ST_UINT8  unsigned char     	
#define ST_UINT16 unsigned short    	
#define ST_UINT32 unsigned long    	
#define ST_UINT64 unsigned long long
#define ST_BOOLEAN  unsigned char		

/* This define shows that we really have support for 64 bit integers	*/
#define INT64_SUPPORT

/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED

#endif

/************************************************************************/
/* OpenVMS AXP                              				*/
/************************************************************************/
#if defined(__ALPHA) && defined(__VMS)   /* OpenVMS AXP uses DECC*/

#define SD_BYTE_ORDER	SD_LITTLE_ENDIAN
#define SYSTEM_SEL     SYSTEM_SEL_OPEN_VMS
#define SD_END_STRUCT  long end_of;

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int		

/* We need specific sizes for these types				*/
#define ST_INT8   signed char     	
#define ST_INT16  signed short     	
#define ST_INT32  signed long     	
#define ST_UINT8  unsigned char     	
#define ST_UINT16 unsigned short    	
#define ST_UINT32 unsigned long    	
#define ST_BOOLEAN  unsigned char		


/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED

#endif

/************************************************************************/
/* Digital UNIX                                                         */
/************************************************************************/

#if defined(__alpha) && !defined(__VMS)

#define SD_BYTE_ORDER	SD_LITTLE_ENDIAN
#define SYSTEM_SEL     SYSTEM_SEL_SYS_5
#define SD_END_STRUCT  long end_of;

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int		

/* We need specific sizes for these types				*/
#define ST_INT8   signed char     	
#define ST_INT16  signed short     	
#define ST_INT32  signed int     	
#define ST_INT64  signed long     	
#define ST_UINT8  unsigned char     	
#define ST_UINT16 unsigned short    	
#define ST_UINT32 unsigned int    	
#define ST_UINT64 unsigned long    	
#define ST_BOOLEAN  unsigned char		

/* This define shows that we really have support for 64 bit integers	*/
#define INT64_SUPPORT

/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED

#endif

/************************************************************************/
/* VXWORKS - VXWORKS on Motorola 680x0 processor			*/
/************************************************************************/

#if defined(VXWORKS) 

#define SD_BYTE_ORDER	SD_BIG_ENDIAN
#define SYSTEM_SEL   	SYSTEM_SEL_SYSVXWORKS
#define SD_END_STRUCT 

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int		

/* We need specific sizes for these types				*/
#define ST_INT8   signed char     	
#define ST_INT16  signed short     	
#define ST_INT32  signed long     	
#define ST_UINT8  unsigned char     	
#define ST_UINT16 unsigned short    	
#define ST_UINT32 unsigned long    	
#define ST_BOOLEAN  unsigned char		

/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED

#endif

/************************************************************************/
/* QNX									*/
/************************************************************************/
#ifdef __QNX__	/* This should be defined automatically by compiler.	*/

#pragma off (unreferenced)	/* don't warn about unused symbols.	*/

#define SD_BYTE_ORDER	SD_LITTLE_ENDIAN
#define SYSTEM_SEL	SYSTEM_SEL_SYS_QNX4
#define SD_END_STRUCT

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int		

/* We need specific sizes for these types				*/
#define ST_INT8   signed char     	
#define ST_INT16  signed short
#define ST_INT32  signed long     	
#define ST_UINT8  unsigned char     	
#define ST_UINT16 unsigned short
#define ST_UINT32 unsigned long    	
#define ST_BOOLEAN  unsigned char		

/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED

#endif

/************************************************************************/
/* HP-UX								*/
/************************************************************************/
#if defined(__hpux)			/* HP-UX		*/

#define SD_BYTE_ORDER	SD_BIG_ENDIAN
#define	SYSTEM_SEL   	SYS_5
#define SD_END_STRUCT long end_of;  /* force struct to quad word allign	*/


/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char
#define ST_INT     signed int
#define ST_LONG    signed long int
#define ST_UCHAR   unsigned char
#define ST_UINT    unsigned int
#define ST_ULONG   unsigned long
#define ST_VOID    void
#define ST_DOUBLE  double
#define ST_FLOAT   float

/* General purpose return code						*/
#define ST_RET signed int

/* We need specific sizes for these types				*/
#define ST_INT8   signed char
#define ST_INT16  signed short
#define ST_INT32  signed long
#define ST_INT64  signed long long
#define ST_UINT8  unsigned char
#define ST_UINT16 unsigned short
#define ST_UINT32 unsigned long
#define ST_UINT64 unsigned long long
#define ST_BOOLEAN  unsigned char

/* This define shows that we really have support for 64 bit integers	*/
#define INT64_SUPPORT

/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED

#endif

/************************************************************************/
/* LINUX SYSTEM								*/
/* OR LYNXOS SYSTEM (same types)					*/
/************************************************************************/
#if defined(linux) || defined(__LYNX) || defined(OX_LINUX)

/* NOTE: this may also work for setting SD_BYTE_ORDER on other		*/
/*       platforms that use the GNU C Library				*/
#include <endian.h>
#include <stdarg.h>
#if (__BYTE_ORDER ==__LITTLE_ENDIAN)
  #define SD_BYTE_ORDER	SD_LITTLE_ENDIAN
#elif (__BYTE_ORDER ==__BIG_ENDIAN)
  #define SD_BYTE_ORDER	SD_BIG_ENDIAN
#else
  #error unsupported byte order
#endif

#define SD_END_STRUCT 

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int		

/* We need specific sizes for these types				*/
#define ST_INT8   signed char     	
#define ST_INT16  signed short     	
#define ST_INT32  signed long     	
#define ST_INT64  signed long long
#define ST_UINT8  unsigned char     	
#define ST_UINT16 unsigned short    	
#define ST_UINT32 unsigned long    	
#define ST_UINT64 unsigned long long
#define ST_BOOLEAN  unsigned char		

/* This define shows that we really have support for 64 bit integers	*/
#define INT64_SUPPORT

/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED

#endif	/* linux	*/

/************************************************************************/
/************************************************************************/
/* Make sure that this module has identified the target system 		*/

#if !defined(_SISCOTYPES_DEFINED)
#error Warning: System not correctly identified by glbtypes.h
#endif

#if !defined(SD_BYTE_ORDER)
#error SD_BYTE_ORDER not defined
#endif

/************************************************************************/
#ifdef __cplusplus
}
#endif


#endif /* #ifndef GBLTYPES_INCLUDED */
