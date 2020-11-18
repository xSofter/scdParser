/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,       */
/*	   1988-2000, All Rights Reserved   				*/
/*									*/
/* MODULE NAME : gen_list.h						*/
/* PRODUCT(S)  : General Use						*/
/*									*/
/* MODULE DESCRIPTION :                                                 */
/*	This module contains the definitions as required for 		*/
/*	manipulation of double-linked circular lists			*/
/*									*/
/* MODIFICATION LOG :                                                   */
/*  Date     Who   Rev      Comments                                    */
/* --------  ---  ------   -------------------------------------------  */
/* 01/04/06  EJV     04    Added DLLs incompatibility comments.		*/
/*			   history.					*/
/************************************************************************/

#ifndef GEN_LIST_INCLUDED
#define GEN_LIST_INCLUDED

#include "glbtypes.h"
#include "scdParse_export.h"

#ifdef __cplusplus
extern "C" {
#endif

/*#define FASTLIST */

/************************************************************************/
/* LINKED LIST MECHANISM						*/
/************************************************************************/
/* This following structure is used in all doubly linked circular lists */
/* as the first component in the structure.  This allows one set of list*/
/* manipulation primitives to be used with any linked structure		*/
/* containing it.							*/

typedef struct dbl_lnk
  {
  /* Note: adding/removing fields to/from this struct will make older	*/
  /* !!!   applications incompatible with new security DLLs.		*/
  struct dbl_lnk *next;
  struct dbl_lnk *prev;
  } DBL_LNK;


/************************************************************************/
/* The variable below can be used to do integrity checking of any list	*/
/* manipulated in the generic queuing functions by setting it to SD_TRUE	*/

//extern ST_BOOLEAN list_debug_sel;

/* For compatibility with older code only				*/
#define list_sLogCtrl sLogCtrl

/************************************************************************/

#ifdef FASTLIST
#define list_get_next(h,p)  (((DBL_LNK *)p)->next == (DBL_LNK *)h ? NULL : ((DBL_LNK *)p)->next) 
#define list_get_first(h)   *(h);list_unlink (h,*h)
#else
/* NOTE: I_AM_THE_TRUE_GEN_LIST is only defined in the module 		*/
/* genlists.c so it will compile.					*/
#if defined(I_AM_THE_TRUE_GEN_LIST)
SCDPAESE_API ST_VOID *list_get_first (DBL_LNK **);		
SCDPAESE_API ST_VOID * list_get_next  (DBL_LNK *, DBL_LNK *);
#else
SCDPAESE_API ST_VOID * list_get_first (ST_VOID *);		
SCDPAESE_API ST_VOID * list_get_next (ST_VOID *, ST_VOID *);
#endif
#endif	/* FASTLIST */


/************************************************************************/
/* Primitive functions for generic queue handling			*/

#if defined(I_AM_THE_TRUE_GEN_LIST)
ST_RET	list_unlink         (DBL_LNK **, DBL_LNK *);
ST_RET	list_add_first      (DBL_LNK **, DBL_LNK *);
ST_RET	list_add_last       (DBL_LNK **, DBL_LNK *);	
ST_RET 	list_move_to_first  (DBL_LNK **, DBL_LNK **, DBL_LNK *);
ST_RET 	list_find_node      (DBL_LNK *,  DBL_LNK *);
ST_RET 	list_add_node_after (DBL_LNK *,  DBL_LNK *);
ST_INT  list_get_sizeof     (DBL_LNK *);
ST_VOID *list_get_last       (DBL_LNK **);
#else
/* NOTE: these prototypes provide very little argument type checking.	*/
/* They allow you to pass almost any argument without casting.		*/
/* ANSI compilers automatically cast the arguments to (ST_VOID *).	*/
/*   This is not a great loss, because if the "real" prototypes were	*/
/* used, most code would have to cast arguments to (DBL_LNK *) or	*/
/* (DBL_LNK **), which would disable the argument type checking anyway.	*/

ST_RET	list_unlink         (ST_VOID *pphol, ST_VOID *pnode);
ST_RET	list_add_first      (ST_VOID *pphol, ST_VOID *pnode);
ST_RET	list_add_last       (ST_VOID *pphol, ST_VOID *pnode);	
ST_RET 	list_move_to_first  (ST_VOID *pphol1, ST_VOID *pphol2, ST_VOID *pnode);
ST_RET 	list_find_node      (ST_VOID *pphol, ST_VOID *pnode);
ST_RET 	list_add_node_after (ST_VOID *pnode1, ST_VOID *pnode2);
ST_INT	list_get_sizeof     (ST_VOID *phol);
ST_VOID   *list_get_last       (ST_VOID *pphol);
#endif

/* New functions. Abandon the "I_AM_THE_TRUE_GEN_LIST" casting business.*/
ST_VOID *list_find_prev (DBL_LNK *list_head_ptr, DBL_LNK *cur_node);
ST_VOID *list_find_last (DBL_LNK *list_head_ptr);

#ifdef __cplusplus
}
#endif

#endif
