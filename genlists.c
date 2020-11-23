/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*              1986-2005  All Rights Reserved                   	*/
/*									*/
/* MODULE NAME : gen_list.c						*/
/* PRODUCT(S)  : general list handling functions			*/
/*									*/
/* MODULE DESCRIPTION : 						*/
/*	This module contains generic que manipulation routines		*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 10/04/07  MDE     07    Tweaked LOGCFG_VALUE_GROUP/LOGCFGX_VALUE_MAP	*/
/*			   history.					*/
/************************************************************************/

#include "glbtypes.h"
#include "sysincs.h"
#define  I_AM_THE_TRUE_GEN_LIST 
#include "gen_list.h"
#include "slog.h"

/************************************************************************/
/************************************************************************/
/* For debug version, use a static pointer to avoid duplication of 	*/
/* __FILE__ strings.							*/
/************************************************************************/

#ifdef DEBUG_SISCO
static ST_CHAR *thisFileName = __FILE__;
#endif

/************************************************************************/
/* The variable below is used in selecting generic queuing debug 	*/
/* checking.  When this variable is set to SD_TRUE all data strucutres 	*/
/* using the list manipulation primitives will under go this checking	*/

ST_BOOLEAN list_debug_sel;


/************************************************************************/
/*				list_unlink				*/
/* Primitive to unlink a node out of a circular double linked list	*/
/************************************************************************/

ST_RET list_unlink (DBL_LNK **list_head_ptr, DBL_LNK *node_ptr)
{
#ifdef DEBUG_SISCO
	if (*list_head_ptr == NULL) 
	{
		SLOGALWAYS ("GENLIST: ATTEMPT TO UNLINK A NODE FROM A NULL LIST");
		return (SD_FAILURE);
	}

	if (node_ptr == NULL)
	{
		SLOGALWAYS ("GENLIST: ATTEMPT TO UNLINK A NULL ADDRESS FROM A LIST");
		return (SD_FAILURE);
	}

	if (list_debug_sel == SD_TRUE)
	{
		if (list_find_node (*list_head_ptr, node_ptr) == SD_FAILURE)
		{
			SLOGALWAYS ("GENLIST: NODE NOT FOUND IN LIST");
			return (SD_FAILURE);
		}
	}
#endif

	/* If list checking is enabled and we get to here we have valid 	*/
	/* arguments and a valid list						*/

	if ((node_ptr -> next == *list_head_ptr) && 
		(node_ptr == *list_head_ptr))		/* only node in list?	*/
	{
		*list_head_ptr = NULL;
	}
	else 
	{					       
		if (node_ptr == *list_head_ptr)    	/* first node in a multi*/
			*list_head_ptr = node_ptr->next;		/* node list		*/
		(node_ptr->next)->prev = node_ptr->prev;	/* link cur next to prev*/
		(node_ptr->prev)->next = node_ptr->next;	/* link cur prev to next*/
	}
	return (SD_SUCCESS);
}


/************************************************************************/
/*				list_add_first				*/
/* Primitive to add a node as the first node of a circular double 	*/
/* linked list								*/
/************************************************************************/

ST_RET list_add_first (DBL_LNK **list_head_ptr, DBL_LNK *node_ptr)
{
	DBL_LNK *list_tail_ptr = NULL;

#ifdef DEBUG_SISCO
	if (node_ptr == NULL)
	{
		SLOGALWAYS ("GENLIST: ATTEMPT TO ADD A NULL ADDRESS TO A LIST");
		return (SD_FAILURE);
	}
#endif

	/* If list checking is enabled and we get to here we have valid 	*/
	/* arguments 								*/

	if (*list_head_ptr == NULL) 	/* will this be the only node?	*/
	{
		node_ptr->next = node_ptr;
		node_ptr->prev = node_ptr;
	}
	else
	{
		list_tail_ptr = (*list_head_ptr)->prev;
		node_ptr->next = *list_head_ptr;
		node_ptr->prev = list_tail_ptr;
		list_tail_ptr->next = node_ptr;
		(*list_head_ptr)->prev = node_ptr;
	}
	*list_head_ptr = node_ptr;		/* assign the new head of list	*/
	return (SD_SUCCESS);

}


/************************************************************************/
/*				list_add_last				*/
/* Primitive to add a node as the last node of a circular double 	*/
/* linked list								*/
/************************************************************************/

ST_RET list_add_last (DBL_LNK **list_head_ptr, DBL_LNK *node_ptr)
{
	DBL_LNK *list_tail_ptr;

#ifdef DEBUG_SISCO
	if (node_ptr == NULL)
	{
		SLOG_ERROR ("GENLIST: ATTEMPT TO ADD A NULL ADDRESS TO A LIST");
		return (SD_FAILURE);
	}
#endif

	/* If list checking is enabled and we get to here we have valid 	*/
	/* arguments 								*/

	if (*list_head_ptr == NULL) 	/* will this be the only node?	*/
	{
		node_ptr->next = node_ptr;
		node_ptr->prev = node_ptr;
		*list_head_ptr = node_ptr;
	}
	else
	{
		list_tail_ptr = (*list_head_ptr)->prev;
		list_tail_ptr->next = node_ptr;
		node_ptr->prev = list_tail_ptr;
		node_ptr->next = *list_head_ptr;
		(*list_head_ptr)->prev = node_ptr;
	}
	return (SD_SUCCESS);
}

/************************************************************************/
/*				list_get_first				*/
/* Primitive to unlink the first node out of the list and return it's	*/
/* address								*/
/************************************************************************/

#ifndef FASTLIST

ST_VOID *list_get_first (DBL_LNK **list_head_ptr)
{
	DBL_LNK *node_ptr;

	if (*list_head_ptr == NULL)
		return (NULL);

	node_ptr = *list_head_ptr;
	list_unlink (list_head_ptr, node_ptr);
	return (node_ptr);

}
#endif

/************************************************************************/
/*				list_get_last				*/
/* Primitive to unlink the last node out of the list and return it's	*/
/* address								*/
/************************************************************************/


ST_VOID *list_get_last (DBL_LNK **list_head_ptr)
{
	DBL_LNK *node_ptr;

	if (*list_head_ptr == NULL)
		return (NULL);

	node_ptr = *list_head_ptr;
	node_ptr = node_ptr->prev;
	list_unlink (list_head_ptr, node_ptr);
	return (node_ptr);
}


/************************************************************************/
/*		           list_move_to_first				*/
/* Primitive to unlink the first node out of the list and return it's	*/
/* address								*/
/************************************************************************/

ST_RET list_move_to_first (DBL_LNK **src_list_head_ptr, 
						   DBL_LNK **dest_list_head_ptr, 
						   DBL_LNK *node_ptr)
{
	ST_RET ret_code;

	if (!(ret_code = list_unlink (src_list_head_ptr, node_ptr)))
	{
		list_add_first (dest_list_head_ptr, node_ptr);
	}
	return (ret_code);
}

/************************************************************************/
/*		           list_find_node				*/
/* Primitive to verify that a node is in a list, returns SD_SUCCESS if it	*/
/* is; SD_FAILURE otherwise.						*/
/************************************************************************/

ST_RET list_find_node (DBL_LNK *list_head_ptr, DBL_LNK *node_ptr)
{
	DBL_LNK	  *temp_ptr;
	DBL_LNK	  *list_tail_ptr;
	ST_RET	ret_code;

	if ((list_head_ptr == NULL) || (node_ptr == NULL))
		ret_code = SD_FAILURE;
	else
	{
		temp_ptr = list_head_ptr;
		list_tail_ptr = list_head_ptr->prev;

		/* search forward from the begining to the end of the list for our node	   */

		while ((temp_ptr != list_tail_ptr) && (temp_ptr != node_ptr))
		{
			temp_ptr = temp_ptr->next;
		}
		if (temp_ptr == node_ptr)
			ret_code = SD_SUCCESS;
		else
			ret_code = SD_FAILURE;
	}
	return (ret_code);
}


/************************************************************************/
/*		           list_print_links				*/
/************************************************************************/

#if 0

ST_VOID list_print_links (DBL_LNK *list_head_ptr)
{
	DBL_LNK	*temp_ptr;
	DBL_LNK	*list_tail_ptr;
	ST_INT i;

	i = 1;
	if (list_head_ptr == NULL)
	{
		printf ("\nThe list is empty");
	}
	else
	{
		temp_ptr = list_head_ptr;
		list_tail_ptr = list_head_ptr->prev;
		printf ("\npointer to the head of the list is: %lx",list_head_ptr);
		printf ("\n");
		printf ("\npointer to node number %d is: %lx",i, temp_ptr);
		printf ("\nnode number:  next ptr is: %lx",temp_ptr->next);
		printf ("\n              prev ptr is: %lx",temp_ptr->prev);
		while ((temp_ptr != list_tail_ptr))
		{
			temp_ptr = temp_ptr->next;
			i++;
			printf ("\npointer to node number %d is: %lx",i, temp_ptr);
			printf ("\nnode number:  next ptr is: %lx",temp_ptr->next);
			printf ("\n              prev ptr is: %lx",temp_ptr->prev);
		}
	}
}
#endif

/************************************************************************/
/*		           list_add_node_after(list insert node prim)	*/
/* Primitive to add a node to a list after the current node.  This 	*/
/* function assumes that we are working with a non NIL list.  SD_SUCCESS 	*/
/* is returned if the node gets added to the list; SD_FAILURE otherwise.	*/
/************************************************************************/

ST_RET list_add_node_after (DBL_LNK *cur_node, DBL_LNK *new_node)
{
	DBL_LNK	*next_node;
	ST_RET	ret_val;

	ret_val = SD_FAILURE;
	if (new_node == NULL)
	{
		SLOGALWAYS ("GENLIST: ATTEMPT TO ADD A NULL ADDRESS TO A LIST");
	}
	else if (cur_node == NULL)
	{
		SLOGALWAYS ("GENLIST: ATTEMPT TO ADD A NODE TO A NULL LIST");
	}
	else			/* this is a good node and good predecessor	*/
	{
		next_node = cur_node -> next;
		new_node -> next = next_node;
		new_node -> prev = cur_node;
		next_node -> prev = new_node;
		cur_node -> next = new_node;
		ret_val = SD_SUCCESS;
	}
	return (ret_val);
}    

/************************************************************************/
/*		           list_get_next				*/
/* Primitive to get the node pointed to by the next component of the	*/
/* current node.  This function returns NULL if the list_head_ptr is	*/
/* NULL, the address of the current node is NULL or the next node in the*/
/* list is the list_head_ptr(the list has wrapped around).  This 	*/
/* function is useful for traversing a doubly linked circular list	*/
/* from begining to the end as if the list were NULL terminated.	*/
/************************************************************************/

#ifndef FASTLIST

ST_VOID *list_get_next (DBL_LNK *list_head_ptr, DBL_LNK *cur_node)
{
	DBL_LNK *next_node = NULL;

	if (list_head_ptr == NULL)
	{
		SLOGALWAYS ("GENLIST: ATTEMPT TO REFERENCE A NULL LIST");
	}
	else if (cur_node == NULL)
	{
		SLOGALWAYS ("GENLIST: ATTEMPT TO REFERENCE THROUGH A NULL PTR");
	}
	else if (cur_node->next != list_head_ptr)
	{
		next_node = cur_node->next;
	}
	return (next_node);
}
#endif

/************************************************************************/
/*		           list_get_sizeof				*/
/* Primitive to return the number of nodes in this generic list.	*/
/************************************************************************/

ST_INT list_get_sizeof (DBL_LNK *list_head_ptr)
{
	ST_INT count;
	DBL_LNK *cur_node;

	count = 0;
	cur_node = list_head_ptr;

	while (cur_node != NULL) 
	{
		count ++;
		cur_node = (DBL_LNK *) list_get_next (list_head_ptr, cur_node);
	}

	return (count);
}

/************************************************************************/
/*		           list_find_prev				*/
/* Primitive to find the node "before" the current node in the linked	*/
/* list. This function returns NULL if the list_head_ptr is		*/
/* NULL, the current node is NULL, or the current node			*/
/* is the head of the linked list (i.e. does not wrap around).		*/
/************************************************************************/

ST_VOID *list_find_prev (DBL_LNK *list_head_ptr, DBL_LNK *cur_node)
{
	DBL_LNK *prev_node = NULL;

	if (list_head_ptr == NULL)
	{
		SLOGALWAYS ("GENLIST: ATTEMPT TO REFERENCE A NULL LIST");
	}
	else if (cur_node == NULL)
	{
		SLOGALWAYS ("GENLIST: ATTEMPT TO REFERENCE THROUGH A NULL PTR");
	}
	else if (cur_node != list_head_ptr)
	{
		prev_node = cur_node->prev;
	}
	return (prev_node);
}

/************************************************************************/
/*				list_find_last				*/
/* Primitive to find the last node in the linked list and return it's	*/
/* address.								*/
/************************************************************************/

ST_VOID *list_find_last (DBL_LNK *list_head_ptr)
{
	DBL_LNK *last_node;

	if (list_head_ptr)
		last_node = list_head_ptr->prev;
	else
	{	/* head is NULL (i.e. list is empty) so last is NULL too.	*/
		last_node = NULL;
	}

	return (last_node);
}

