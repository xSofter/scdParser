/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*	(c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*	2004-2006 All Rights Reserved					*/
/*									*/
/* MODULE NAME : sclstore.c						*/
/* PRODUCT(S)  : MMS-EASE-LITE						*/
/*									*/
/* MODULE DESCRIPTION :							*/
/*	Functions to store information parsed from SCL file.		*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*			scl_lntype_create				*/
/*			scl_lntype_add_do				*/
/*			scl_dotype_create				*/
/*			scl_dotype_add_da				*/
/*			scl_dotype_add_sdo				*/
/*			scl_datype_create				*/
/*			scl_datype_add_bda				*/
/*			scl_enumtype_create				*/
/*			scl_enumtype_add_enumval			*/
/*			scl_fcda_add					*/
/*			scl_dai_add					*/
/*			scl_dataset_add					*/
/*			scl_rcb_add					*/
/*			scl_lcb_add					*/
/*			scl_gcb_add					*/
/*			scl_ln_add					*/
/*			scl_ld_create					*/
/*			scl_info_destroy				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev     Comments					*/
/* --------  ---  ------   -------------------------------------------  */
/* 04/23/08  JRB    11     Add scl_sgcb_add.				*/
/************************************************************************/
#include "glbtypes.h"
#include "sysincs.h"
#include "scl.h"
#include "str_util.h"
#include "mem_chk.h"
#include "slog.h"

#ifdef DEBUG_SISCO
SD_CONST static ST_CHAR *SD_CONST thisFileName = __FILE__;
#endif


/* NOTE:
* Functions with "scl_" prefix used while parsing SCL file to
* store data in linked lists.
* Functions with "scl2_" prefix (see sclproc.c) used to read info from
* linked lists and create MMS objects (Domains, variables, NVLs, etc.).
*/

/************************************************************************/
/*			scl_lntype_create				*/
/* Begin creation of a Logical Node Type (LNodeType).			*/
/* RETURNS:	SD_SUCCESS or error code				*/
/************************************************************************/
SCL_LNTYPE *scl_lntype_create (
							   SCL_INFO *scl_info)	/* struct to store all SCL info	*/
{
	SCL_LNTYPE *scl_lntype;

	scl_lntype = (SCL_LNTYPE *) chk_calloc (1, sizeof (SCL_LNTYPE));
	/* Add LNType to front of LNType List.	*/
	list_add_first (&scl_info->lnTypeHead, scl_lntype);
	return (scl_lntype);
}

/************************************************************************/
/*			scl_lntype_add_do				*/
/* Add a Data Object (DO) to a Logical Node Type (LNodeType).		*/
/* Adds to lntype created by most recent call to "scl_lntype_create".	*/
/* RETURNS:	SD_SUCCESS or error code				*/
/************************************************************************/
SCL_DO *scl_lntype_add_do (
						   SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_DO *scl_do = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->lnTypeHead)
	{
		scl_do = (SCL_DO *) chk_calloc (1, sizeof (SCL_DO));
		/* Add DO to front of DO List in first entry of LNType list	*/
		list_add_first (&scl_info->lnTypeHead->doHead, scl_do);
	}
	else
	{
		SLOG_ERROR ("Cannot add DO to NULL LNTYPE");
	}
	return (scl_do);
}

/************************************************************************/
/*			scl_dotype_create				*/
/* Begin creation of a Data Object Type (DOType).			*/
/* RETURNS:	SD_SUCCESS or error code				*/
/************************************************************************/
SCL_DOTYPE *scl_dotype_create (
							   SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_DOTYPE *scl_dotype;

	scl_dotype = (SCL_DOTYPE *) chk_calloc (1, sizeof (SCL_DOTYPE));
	/* Add DOType to front of DOType List.	*/
	list_add_first (&scl_info->doTypeHead, scl_dotype);
	return (scl_dotype);
}

/************************************************************************/
/*			scl_dotype_add_da				*/
/* Add a Data Attribute (DA) to a Data Object Type (DOType).		*/
/* RETURNS:	SD_SUCCESS or error code				*/
/************************************************************************/
SCL_DA *scl_dotype_add_da (
						   SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_DA *scl_da = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->doTypeHead)
	{
		scl_da = (SCL_DA *) chk_calloc (1, sizeof (SCL_DA));
		/* Add DA to front of DA List in first entry of DOType list	*/
		list_add_first (&scl_info->doTypeHead->daHead, scl_da);
	}
	else
	{
		SLOG_ERROR ("Cannot add DA to NULL DO");
	}
	return (scl_da);
}

/************************************************************************/
/*			scl_dotype_add_sdo				*/
/* Add a (SDO) to a Data Object Type (DOType).				*/
/* RETURNS:	SD_SUCCESS or error code				*/
/************************************************************************/
SCL_DO *scl_dotype_add_sdo (
							SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_DO *scl_da = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->doTypeHead)
	{
		scl_da = (SCL_DO *) chk_calloc (1, sizeof (SCL_DO));
		/* CRITICAL: DA and SDO use same struct, "objtype" tells which one it is.
		* NOTE: Because this is SDO, the sAddr, bType, and valKind members of
		* the scl_da structure are NOT used.
		*/
	
		/* Add DA to front of DA List in first entry of DOType list	*/
		list_add_first (&scl_info->doTypeHead->sdoHead, scl_da);
	}
	else
	{
		SLOG_ERROR ("Cannot add SDO to NULL DO");
	}
	return (scl_da);
}

/************************************************************************/
/*			scl_datype_create				*/
/* Begin creation of a Data Attribute Type (DAType).			*/
/* RETURNS:	SD_SUCCESS or error code				*/
/************************************************************************/
SCL_DATYPE *scl_datype_create (
							   SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_DATYPE *scl_datype;

	scl_datype = (SCL_DATYPE *) chk_calloc (1, sizeof (SCL_DATYPE));
	/* Add DAType to front of DAType List.	*/
	list_add_first (&scl_info->daTypeHead, scl_datype);
	return (scl_datype);
}
/************************************************************************/
/*			scl_datype_add_bda				*/
/* Add a Basic Data Attribute (BDA) to a Data Attribute Type (DAType).	*/
/* RETURNS:	SD_SUCCESS or error code				*/
/************************************************************************/
SCL_BDA *scl_datype_add_bda (
							 SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_BDA *scl_bda = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->daTypeHead)
	{
		scl_bda = (SCL_BDA *) chk_calloc (1, sizeof (SCL_BDA));
		/* Add BDA to front of BDA List in first entry of DAType list	*/
		list_add_first (&scl_info->daTypeHead->bdaHead, scl_bda);
	}
	else
	{
		SLOG_ERROR ("Cannot add BDA to NULL DATYPE");
	}
	return (scl_bda);
}

/************************************************************************/
/*			scl_enumtype_create				*/
/* Begin creation of an EnumType.					*/
/* RETURNS:	SD_SUCCESS or error code				*/
/************************************************************************/
SCL_ENUMTYPE *scl_enumtype_create (
								   SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_ENUMTYPE *scl_enumtype;

	scl_enumtype = (SCL_ENUMTYPE *) chk_calloc (1, sizeof (SCL_ENUMTYPE));
	/* Add EnumType to front of EnumType List.	*/
	list_add_first (&scl_info->enumTypeHead, scl_enumtype);
	return (scl_enumtype);
}


/************************************************************************/
/*			scl_enumtype_add_enumval			*/
/* Add an EnumVal to an EnumType.					*/
/* Add to EnumType created by most recent call to scl_enumtype_create.	*/
/* RETURNS:	SD_SUCCESS or error code				*/
/************************************************************************/
SCL_ENUMVAL *scl_enumtype_add_enumval (
									   SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_ENUMVAL *scl_enumval = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->enumTypeHead)
	{
		scl_enumval = (SCL_ENUMVAL *) chk_calloc (1, sizeof (SCL_ENUMVAL));
		/* Add EnumVal to front of EnumVal List in first entry of EnumType list	*/
		list_add_first (&scl_info->enumTypeHead->enumvalHead, scl_enumval);
	}
	else
	{
		SLOG_ERROR ("Cannot add ENUMVAL to NULL ENUMTYPE");
	}
	return (scl_enumval);
}


/************************************************************************/
/*			scl_fcda_add				*/
/* Allocates a SCL_FCDA struct						*/
/* and adds it to the linked list "fcdaHead" in SCL_DATASET.		*/
/************************************************************************/
SCL_FCDA *scl_fcda_add (
						SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_FCDA *scl_fcda = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->ldHead
		&& scl_info->ldHead->lnHead
		&& scl_info->ldHead->lnHead->datasetHead)
	{
		scl_fcda = (SCL_FCDA *) chk_calloc (1, sizeof (SCL_FCDA));
		/* Add FCDA to front of FCDA List.	*/
		list_add_first (&scl_info->ldHead->lnHead->datasetHead->fcdaHead, scl_fcda);
	}
	else
	{
		SLOG_ERROR ("Cannot add FCDA to NULL DATASET");
	}

	return (scl_fcda);
}

/************************************************************************/
/*			scl_dai_add					*/
/* Allocates a SCL_DAI struct						*/
/* and adds it to the linked list "daiHead" in SCL_LN.			*/
/************************************************************************/
SCL_DAI *scl_dai_add (
					  SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_DAI *scl_dai = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->ldHead
		&& scl_info->ldHead->lnHead)
	{
		scl_dai = (SCL_DAI *) chk_calloc (1, sizeof (SCL_DAI));
		/* Add DAI to front of DAI List.	*/
		list_add_first (&scl_info->ldHead->lnHead->doiHead->daiHead, scl_dai);
	}
	else
	{
		SLOG_ERROR ("Cannot add DAI to NULL LN");
	}
	return (scl_dai);
}

/************************************************************************/
/*			scl_dataset_add				*/
/* Allocates a SCL_DATASET struct					*/
/* and adds it to the linked list "datasetHead" in SCL_LN.		*/
/************************************************************************/
SCL_DATASET *scl_dataset_add (
							  SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
							  /* TRUNCATED if longer than buffer	*/
{
	SCL_DATASET *scl_dataset = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->ldHead
		&& scl_info->ldHead->lnHead)
	{
		scl_dataset = (SCL_DATASET *) chk_calloc (1, sizeof (SCL_DATASET));
		/* Add DATASET to front of DATASET List.	*/
		list_add_first (&scl_info->ldHead->lnHead->datasetHead, scl_dataset);
	}
	else
	{
		SLOG_ERROR ("Cannot add DATASET to NULL LN");
	}
	return (scl_dataset);
}

/************************************************************************/
/*			scl_rcb_add					*/
/* Alloc & add SCL_RCB struct to the linked list "rcbHead" in SCL_LN.	*/
/* NOTE: struct is not filled in yet.					*/
/* NOTE: the RptEnabled element of the SCL file is ignored. SISCO software*/
/*	does not need to know which clients may access the RCB.		*/
/************************************************************************/
SCL_RCB *scl_rcb_add (
					  SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_RCB *scl_rcb = NULL;

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->ldHead
		&& scl_info->ldHead->lnHead)
	{
		scl_rcb = (SCL_RCB *) chk_calloc (1, sizeof (SCL_RCB));
		/* Add RCB to front of RCB List.	*/
		list_add_first (&scl_info->ldHead->lnHead->rcbHead, scl_rcb);
	}
	else
	{
		SLOG_ERROR ("Cannot add RCB to NULL LN");
	}
	return (scl_rcb);
}

/************************************************************************/
/*			scl_lcb_add					*/
/* Alloc & add a SCL_LCB struct to the linked list "lcbHead" in SCL_LN.	*/
/* NOTE: struct is not filled in yet.					*/
/************************************************************************/
SCL_LCB *scl_lcb_add (
					  SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_LCB *scl_lcb = NULL;

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->ldHead
		&& scl_info->ldHead->lnHead)
	{
		scl_lcb = (SCL_LCB *) chk_calloc (1, sizeof (SCL_LCB));
		/* Add LCB to front of LCB List.	*/
		list_add_first (&scl_info->ldHead->lnHead->lcbHead, scl_lcb);
	}
	else
	{
		SLOG_ERROR ("Cannot add LCB to NULL LN");
	}
	return (scl_lcb);
}

/************************************************************************/
/*			scl_gcb_add					*/
/* Add a GOOSE Control Block (GCB).					*/
/* Allocates a SCL_GCB struct						*/
/* and adds it to the linked list "gcbHead" in SCL_LN.			*/
/* NOTE: The SCL file may also contain one or more "IEDName" elements to*/
/*       indicate IEDs that should subscribe for GOOSE data. We have no	*/
/*       way to use this information, so it is ignored.			*/
/************************************************************************/
SCL_GCB *scl_gcb_add (
					  SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_GCB *scl_gcb = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->ldHead
		&& scl_info->ldHead->lnHead)
	{
		scl_gcb = (SCL_GCB *) chk_calloc (1, sizeof (SCL_GCB));
		/* Add GCB to front of GCB List.	*/
		list_add_first (&scl_info->ldHead->lnHead->gcbHead, scl_gcb);
	}
	else
	{
		SLOG_ERROR ("Cannot add GCB (GOOSE Control Block) to NULL LN");
	}
	return (scl_gcb);
}

/************************************************************************/
/*			scl_sgcb_add					*/
/* Add a Setting Group Control Block (SGCB).				*/
/* Allocates a SCL_SGCB struct, stores ptr in "sgcb" in SCL_LN.		*/
/************************************************************************/
SCL_SGCB *scl_sgcb_add (
						SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_SGCB *scl_sgcb = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->ldHead
		&& scl_info->ldHead->lnHead)
	{
		/* Only one SGCB allowed. Make sure not already set.	*/
		if (scl_info->ldHead->lnHead->sgcb != NULL)
		{
			SLOG_ERROR ("Duplicate SGCB (Setting Group Control Block) not allowed");
			return (NULL);
		}
		else  
			scl_info->ldHead->lnHead->sgcb = scl_sgcb = (SCL_SGCB *) chk_calloc (1, sizeof (SCL_SGCB));
	}
	else
	{
		SLOG_ERROR ("Cannot add SGCB (Setting Group Control Block) to NULL LN");
	}
	return (scl_sgcb);
}

/************************************************************************/
/*			scl_svcb_add					*/
/* Alloc & add SCL_SVCB struct to the linked list "svcbHead" in SCL_LN.	*/
/* NOTE: struct is not filled in yet.					*/
/************************************************************************/
SCL_SVCB *scl_svcb_add (
						SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_SVCB *scl_svcb = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->ldHead
		&& scl_info->ldHead->lnHead)
	{
		scl_svcb = (SCL_SVCB *) chk_calloc (1, sizeof(SCL_SVCB));
		/* Add to front of list.	*/
		list_add_first (&scl_info->ldHead->lnHead->svcbHead, scl_svcb);
	}
	else
	{
		SLOG_ERROR ("Cannot add SVCB to NULL LN");
	}
	return (scl_svcb);
}

/************************************************************************/
/*			scl_ln_add					*/
/* Allocates a SCL_LN struct						*/
/* and adds it to the linked list "lnHead" in SCL_LD.			*/
/************************************************************************/
SCL_LN *scl_ln_add (
					SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_LN *scl_ln = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->ldHead)
	{
		scl_ln = (SCL_LN *) chk_calloc (1, sizeof (SCL_LN));
		/* Add LN to front of LN List.	*/
		list_add_first (&scl_info->ldHead->lnHead, scl_ln);
	}
	else
	{
		SLOG_ERROR ("Cannot add LN to NULL LD");
	}
	return (scl_ln);
}

/************************************************************************/
/*			scl_ld_create					*/
/* Allocates SCL_LD struct						*/
/* and adds it to the linked list "ldHead" in SCL_INFO.			*/
/************************************************************************/
SCL_LD *scl_ld_create (
					   SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_LD *scl_ld = NULL;	/* assume failure	*/

	scl_ld = (SCL_LD *) chk_calloc (1, sizeof (SCL_LD));
	/* Add LD to front of LD List.	*/
	list_add_first (&scl_info->ldHead, scl_ld);

	return (scl_ld);
}

/************************************************************************/
/*			scl_subnet_add					*/
/* Allocates SCL_SUBNET struct						*/
/* and adds it to the linked list "subnetHead" in SCL_INFO.		*/
/************************************************************************/
SCL_SUBNET *scl_subnet_add (
							SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_SUBNET *scl_subnet = NULL;	/* assume failure	*/

	scl_subnet = (SCL_SUBNET *) chk_calloc (1, sizeof (SCL_SUBNET));
	/* Add to front of list.	*/
	list_add_first (&scl_info->subnetHead, scl_subnet);

	return (scl_subnet);
}

/************************************************************************/
/*			scl_cap_add					*/
/* Allocates a SCL_CAP struct						*/
/* and adds it to the linked list "capHead" in SCL_SUBNET.		*/
/************************************************************************/
SCL_CAP *scl_cap_add (
					  SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_CAP *scl_cap = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->subnetHead)
	{
		scl_cap = (SCL_CAP *) chk_calloc (1, sizeof (SCL_CAP));
		/* Add to front of list.	*/
		list_add_first (&scl_info->subnetHead->capHead, scl_cap);
	}
	else
	{
		SLOG_ERROR ("Cannot add CAP to NULL SUBNET");
	}
	return (scl_cap);
}

/************************************************************************/
/*			scl_gse_add					*/
/* Allocates a SCL_GSE struct						*/
/* and adds it to the linked list "gseHead" in SCL_CAP.			*/
/************************************************************************/
SCL_GSE *scl_gse_add (
					  SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_GSE *scl_gse = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->subnetHead->capHead)
	{
		scl_gse = (SCL_GSE *) chk_calloc (1, sizeof (SCL_GSE));
		/* Add to front of list.	*/
		list_add_first (&scl_info->subnetHead->capHead->gseHead, scl_gse);
	}
	else
	{
		SLOG_ERROR ("Cannot add GSE to NULL CAP");
	}
	return (scl_gse);
}

/************************************************************************/
/*			scl_smv_add					*/
/* Allocates a SCL_SMV struct						*/
/* and adds it to the linked list "smvHead" in SCL_CAP.			*/
/************************************************************************/
SCL_SMV *scl_smv_add (
					  SCL_INFO *scl_info)	/* main struct where all SCL info stored*/
{
	SCL_SMV *scl_smv = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->subnetHead->capHead)
	{
		scl_smv = (SCL_SMV *) chk_calloc (1, sizeof (SCL_SMV));
		/* Add to front of list.	*/
		list_add_first (&scl_info->subnetHead->capHead->smvHead, scl_smv);
	}
	else
	{
		SLOG_ERROR ("Cannot add SMV to NULL CAP");
	}
	return (scl_smv);
}

SCL_ADDRESS *scl_address_add (
						  SCL_INFO *scl_info)
{
	SCL_ADDRESS *scl_addr = NULL;	/* assume failure	*/

	/* All higher level linked lists must be initialized.	*/
	if (scl_info->subnetHead->capHead)
	{
		scl_addr = (SCL_ADDRESS *) chk_calloc (1, sizeof (SCL_ADDRESS));
		/* Add to front of list.	*/
		scl_info->subnetHead->capHead->addr=scl_addr;
	}
	else
	{
		SLOG_ERROR ("Cannot add ADDRESS to NULL CAP");
	}
	return (scl_addr);
}
/************************************************************************/
/*			scl_info_destroy				*/
/* Destroy all info stored in the SCL_INFO structure by "scl_parse".	*/
/* NOTE: most buffers were allocated by functions in this module. The	*/
/*   elements "Val" and "desc" in several structures were allocated	*/
/*   by functions in "sclparse.c".					*/
/************************************************************************/
ST_VOID scl_info_destroy (SCL_INFO *scl_info)
{
	SCL_LNTYPE *lnType;	
	SCL_DO *scl_do;
	SCL_LD *scl_ld;
	SCL_LN *scl_ln;
	SCL_RCB *scl_rcb;
	SCL_LCB *scl_lcb;
	SCL_GCB *scl_gcb;
	SCL_SVCB *scl_svcb;
	SCL_DAI *scl_dai;
	SCL_DATASET *scl_dataset;
	SCL_FCDA *scl_fcda;
	SCL_DATYPE * scl_daType;
	SCL_BDA *scl_bda;
	SCL_ENUMTYPE *scl_enum;
	SCL_ENUMVAL *scl_enumval;
	SCL_DOTYPE *scl_doType;	
	SCL_DA *scl_da;	
	SCL_SUBNET *scl_subnet;
	SCL_CAP *scl_cap;
	SCL_GSE *scl_gse;
	SCL_SMV *scl_smv;
	SCL_DOI *scl_doi;
	SCL_SDI *scl_sdi;
	SCL_IED *scl_ied;

	SCL_SUBSTATION *subStn;
	SCL_LNODE *lnode;
	SCL_POWERTRANSFORMER *trfm;
	SCL_GENERALEQUIPMENT *gnet;
	SCL_FUNCTION *fntn;
	SCL_VOLTAGELEVEL *vllv;
	SCL_CONDUCTINGEQUIPMENT *cdet;

	while ((subStn = (SCL_SUBSTATION *) list_get_first(&scl_info->substationHead)) != NULL)
	{
		while ((lnode = (SCL_LNODE *) list_get_first(&subStn->lnHead)) != NULL)
		{
			if (lnode->desc)
				chk_free (lnode->desc);
			if (lnode->lnInst)
				chk_free (lnode->lnInst);
			if (lnode->iedName)
				chk_free (lnode->iedName);
			if (lnode->ldInst)
				chk_free (lnode->ldInst);
			if (lnode->prefix)
				chk_free (lnode->prefix);
			if (lnode->lnType)
				chk_free (lnode->lnType);
			chk_free (lnode);
		}
		while ((trfm = (SCL_POWERTRANSFORMER *) list_get_first(&subStn->ptHead)) != NULL)
		{
			SCL_TRANSFORMERWINDING *tnwd;
			while ((tnwd = (SCL_TRANSFORMERWINDING *) list_get_first(&trfm->twHead)) != NULL)
			{
				SCL_TERMINAL *trml;
                SCL_SUBEQUIPMENT *sbet;
				SCL_TAPCHANGER *tpcg;
				while ((trml = (SCL_TERMINAL *) list_get_first(&tnwd->tlHead)) != NULL)
				{
					if (trml->desc)
						chk_free (trml->desc);
					chk_free(trml);
				}
				while ((sbet = (SCL_SUBEQUIPMENT *) list_get_first(&tnwd->seHead)) != NULL)
				{
					if (sbet->desc)
						chk_free (sbet->desc);
					if (sbet->phase)
						chk_free (sbet->phase);
					chk_free(sbet);
				}
				
				while ((tpcg = (SCL_TAPCHANGER *) list_get_first(&tnwd->tcHead)) != NULL)
				{
					if (tpcg->desc)
						chk_free (tpcg->desc);
					chk_free(tpcg);
				}
				if (tnwd->desc)
					chk_free (tnwd->desc);
				chk_free (tnwd);
			}
			if (trfm->desc)
				chk_free (trfm->desc);
			chk_free (trfm);
		}
		while ((gnet = (SCL_GENERALEQUIPMENT *) list_get_first(&subStn->geHead)) != NULL)
		{
			if (gnet->desc)
				chk_free (gnet->desc);
			chk_free (gnet);
		}
		while ((fntn = (SCL_FUNCTION *) list_get_first(&subStn->ftHead)) != NULL)
		{
			SCL_SUBFUNCTION *sbfn;
			while ((sbfn = (SCL_SUBFUNCTION *) list_get_first(&fntn->sfHead)) != NULL)
			{
				if (sbfn->desc)
					chk_free (sbfn->desc);
				chk_free (sbfn);
			}
			if (fntn->desc)
				chk_free (fntn->desc);
			chk_free (fntn);
		}
		while ((vllv = (SCL_VOLTAGELEVEL *) list_get_first(&subStn->vlHead)) != NULL)
		{
			SCL_BAY *bay;
			while ((bay = (SCL_BAY *) list_get_first(&vllv->bayHead)) != NULL)
			{
				SCL_CONNECTIVITYNODE *cnnd;
				while ((cnnd = (SCL_CONNECTIVITYNODE *) list_get_first(&bay->cnHead)) != NULL)
				{
					if (cnnd->desc)
						chk_free (cnnd->desc);
					chk_free(cnnd);
				}

				if (bay->desc)
					chk_free (bay->desc);
				chk_free (bay);
			}
			if (vllv->unit)
				chk_free (vllv->unit);
			if (vllv->multi)
				chk_free (vllv->multi);
			if (vllv->desc)
				chk_free (vllv->desc);
			chk_free (vllv);
		}
		while ((cdet = (SCL_CONDUCTINGEQUIPMENT *) list_get_first(&subStn->ceHead)) != NULL)
		{
			SCL_TERMINAL *trml;
			SCL_SUBEQUIPMENT *sbet;
			while ((trml = (SCL_TERMINAL *) list_get_first(&cdet->tlHead)) != NULL)
			{
				if (trml->desc)
					chk_free (trml->desc);
				chk_free(trml);
			}
			while ((sbet = (SCL_SUBEQUIPMENT *) list_get_first(&cdet->seHead)) != NULL)
			{
				if (sbet->desc)
					chk_free (sbet->desc);
				if (sbet->phase)
					chk_free (sbet->phase);
				chk_free(sbet);
			}
			if (cdet->desc)
				chk_free (cdet->desc);
			chk_free (cdet);
		}

		if (subStn->desc)
			chk_free (subStn->desc);
		chk_free (subStn);
	}

	while ((scl_subnet = (SCL_SUBNET *) list_get_first(&scl_info->subnetHead)) != NULL)
	{
		while ((scl_cap = (SCL_CAP *) list_get_first(&scl_subnet->capHead)) != NULL)
		{
			while ((scl_gse = (SCL_GSE *) list_get_first(&scl_cap->gseHead)) != NULL)
			{
				chk_free(scl_gse);
			}
			while ((scl_smv = (SCL_SMV *) list_get_first(&scl_cap->smvHead)) != NULL)
			{
				chk_free(scl_smv);
			}
			if(scl_cap->addr != NULL)
			{
				chk_free(scl_cap->addr);
			}

			if (scl_cap->desc)
				chk_free (scl_cap->desc);
			chk_free (scl_cap);
		}
		if (scl_subnet->desc)
			chk_free (scl_subnet->desc);
		chk_free (scl_subnet);
	}

	while ((scl_daType = (SCL_DATYPE *) list_get_first (&scl_info->daTypeHead)) != NULL)
	{	
		while ((scl_bda = (SCL_BDA *) list_get_first (&scl_daType->bdaHead)) != NULL)
		{	
			if (scl_bda->desc)
				chk_free (scl_bda->desc);
			if (scl_bda->Val)
				chk_free (scl_bda->Val);
			chk_free (scl_bda);
		}
		chk_free (scl_daType);
	}

	while ((lnType = (SCL_LNTYPE *) list_get_first (&scl_info->lnTypeHead)) != NULL)
	{	
		while ((scl_do = (SCL_DO *) list_get_first (&lnType->doHead)) != NULL)
		{	
			if (scl_do->desc)
				chk_free (scl_do->desc);
			chk_free (scl_do);
		}
		chk_free (lnType);
	}
	while ((scl_doType = (SCL_DOTYPE *) list_get_first (&scl_info->doTypeHead)) != NULL)
	{	
		SCL_DO *scl_sdo;
		while ((scl_sdo = (SCL_DO *) list_get_first (&scl_doType->sdoHead)) != NULL)
		{	
			if (scl_sdo->desc)
				chk_free (scl_sdo->desc);
			chk_free (scl_sdo);
		}
		while ((scl_da = (SCL_DA *) list_get_first (&scl_doType->daHead)) != NULL)
		{	
			if (scl_da->desc)
				chk_free (scl_da->desc);
			if (scl_da->Val)
				chk_free (scl_da->Val);
			chk_free (scl_da);
		}
		if (scl_doType->desc)
			chk_free (scl_doType->desc);
		chk_free (scl_doType);
	}
	while ((scl_enum = (SCL_ENUMTYPE *) list_get_first (&scl_info->enumTypeHead)) != NULL)
	{	
		while ((scl_enumval = (SCL_ENUMVAL *) list_get_first (&scl_enum->enumvalHead)) != NULL)
		{	
			chk_free (scl_enumval);
		}
		chk_free (scl_enum);
	}
	while ((scl_ld = (SCL_LD *) list_get_first (&scl_info->ldHead)) != NULL)
	{	
		while ((scl_ln = (SCL_LN *) list_get_first (&scl_ld->lnHead)) != NULL)
		{	
			while ((scl_doi = (SCL_DOI *) list_get_first (&scl_ln->doiHead)) != NULL)
			{	
				while ((scl_dai = (SCL_DAI *) list_get_first (&scl_doi->daiHead)) != NULL)
				{	
					if (scl_dai->Val)
						chk_free (scl_dai->Val);

					if (scl_dai->desc)
						chk_free (scl_dai->desc);

					chk_free (scl_dai);
				}
				while ((scl_sdi = (SCL_SDI *) list_get_first (&scl_doi->sdiHead)) != NULL)
				{	
					if (scl_sdi->desc)
						chk_free (scl_sdi->desc);

					chk_free (scl_sdi);
				}

				if (scl_doi->desc)
					chk_free (scl_doi->desc);
				chk_free (scl_doi);	
			}

			while ((scl_dataset = (SCL_DATASET *) list_get_first (&scl_ln->datasetHead)) != NULL)
			{	
				while ((scl_fcda = (SCL_FCDA *) list_get_first (&scl_dataset->fcdaHead)) != NULL)
				{	
					chk_free (scl_fcda);
				}
				if (scl_dataset->desc)
					chk_free (scl_dataset->desc);
				chk_free (scl_dataset);
			}

			while ((scl_rcb = (SCL_RCB *) list_get_first (&scl_ln->rcbHead)) != NULL)
			{	
				if (scl_rcb->desc)
					chk_free (scl_rcb->desc);
				chk_free (scl_rcb);
			}

			while ((scl_lcb = (SCL_LCB *) list_get_first (&scl_ln->lcbHead)) != NULL)
			{	
				if (scl_lcb->desc)
					chk_free (scl_lcb->desc);
				chk_free (scl_lcb);
			}

			while ((scl_gcb = (SCL_GCB *) list_get_first (&scl_ln->gcbHead)) != NULL)
			{	
				SCL_IEDNAME *iedNm;
				while ((iedNm = (SCL_IEDNAME *) list_get_first (&scl_gcb->iedNHead)) != NULL)
				{	
					chk_free (iedNm);
				}
				if (scl_gcb->desc)
					chk_free (scl_gcb->desc);
				chk_free (scl_gcb);
			}
			while ((scl_svcb = (SCL_SVCB *) list_get_first (&scl_ln->svcbHead)) != NULL)
			{	
				SCL_IEDNAME *iedNm;
				while ((iedNm = (SCL_IEDNAME *) list_get_first (&scl_svcb->iedNHead)) != NULL)
				{	
					chk_free (iedNm);
				}
				if (scl_svcb->desc)
					chk_free (scl_svcb->desc);
				chk_free (scl_svcb);
			}

			/* Only one SGCB allowed (no linked list)	*/
			if (scl_ln->sgcb)
			{	
				if (scl_ln->sgcb->desc)
					chk_free (scl_ln->sgcb->desc);
				chk_free (scl_ln->sgcb);
			}
			if (scl_ln->desc)
				chk_free (scl_ln->desc);
			chk_free (scl_ln);
		}
		if (scl_ld->desc)
			chk_free (scl_ld->desc);
		chk_free (scl_ld);
	}

	while ((scl_ied = (SCL_IED *) list_get_first (&scl_info->lIEDHead)) != NULL)
	{	
		if (scl_ied->desc)
			chk_free (scl_ied->desc);
		chk_free (scl_ied);
	}
}


