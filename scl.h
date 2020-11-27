#ifndef SCL_INCLUDED
#define SCL_INCLUDED
/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*	(c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*	2004-2004 All Rights Reserved					*/
/*									*/
/* MODULE NAME : scl.h							*/
/* PRODUCT(S)  : MMS-EASE-LITE						*/
/*									*/
/* MODULE DESCRIPTION :							*/
/*	SCL main header file.						*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev     Comments					*/
/* --------  ---  ------   -------------------------------------------  */
/* 07/24/08  JRB    17     Fix len of appID in SCL_GCB (maps to GoID).	*/
/* 04/23/08  JRB    16     Add SCL_SGCB, scl_sgcb_add.			*/
/* 07/03/07  JRB    15     Add scl_gse_find.				*/
/* 06/12/07  JRB    14     Add VLANID in SCL_GSE & SCL_SMV.		*/
/* 05/21/07  RKR    13     Changed MAC type to ST_UCHAR for GSE and SMV */
/* 10/30/06  JRB    12     scl2_datatype_create_all: all args changed.	*/
/*			   scl2_ld_create_all: add vmd_ctrl, is_client args.*/
/*			   Add type_id to SCL_LN, SCL_LNTYPE.		*/
/*			   Chg SCL2_BTYPE.				*/
/* 08/04/06  MDE    11     Added scl_debug_mode, scl2_add_btype, etc.	*/
/* 07/26/06  JRB    10     Add structs,functs for parsing Communication.*/
/* 04/19/06  JRB    09	   Add maxClient to SCL_RCB for "RptEnabled max".*/
/* 03/16/06  JRB    08	   Add SCL_SVCB struct & scl_svcb_add proto.	*/
/* 03/15/06  JRB    07     Chg almost all function prototypes.		*/
/*			   Del scl_dotype_add_da_val,			*/
/*			    scl_datype_add_bda_val, scl_header_save.	*/
/*			   Add sGroup to SCL_DAI.			*/
/*			   Chg desc to ptr (allocated during parse).	*/
/*			   Increase MAX_CDC_LEN for user-defined CDCs.	*/
/* 07/25/05  JRB    12     scl2_ld_create_all: Add brcb_bufsize arg.	*/
/*			   SCL_INFO: add brcb_bufsize element.		*/
/* 06/28/05  JRB    11     Del scl2_ld_destroy_all. Use mvl_vmd_destroy.*/
/* 06/24/05  JRB    10     Chg "ord" from unsigned to signed value.	*/
/* 05/27/05  CRM    09	   Add scl_info_destroy & scl2_ld_destroy_all.	*/
/* 03/22/05  JRB    08     Add ifdef __cplusplus to work with C++ code.	*/
/* 02/15/05  JRB    07     Add iedName to SCL_INFO.			*/
/*			   Add domName to SCL_LD & SCL_FCDA.		*/
/*			   Add varName to SCL_LN.			*/
/*			   Define SCL_HEADER struct & add it to SCL_INFO.*/
/*			   Add scl_header_save.				*/
/* 08/06/04  JRB    06     Add scl_parse.				*/
/*			   Move mapping functions to user header.	*/
/* 07/19/04  JRB    05     Add out_filename arg to datamap_cfg_read.	*/
/*			   Add any_cfg_count proto.			*/
/* 07/15/04  JRB    04     Clean up SCL_GCB, scl_gcb_add.		*/
/* 07/09/04  JRB    03     scl2_ld_create_all: add reportScanRate arg.	*/
/* 07/02/04  JRB    02     Add SCL_LCB, SCL_GCB, scl_lcb_add, scl_gcb_add.*/
/* 06/08/04  JRB    01     Initial Revision.				*/
/************************************************************************/
#include "gen_list.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
	FIX_STR_LEN_1 = 1,
	FIX_STR_LEN_2,
	FIX_STR_LEN_3,
	FIX_STR_LEN_4,
	FIX_STR_LEN_5
} enum_str_lenth;

#define MAX_IDENT_LEN 100
#define MVL61850_MAX_RPTID_LEN 64
#define CLNP_MAX_LEN_MAC 12	/* 采用12字节显示,010CCD010000 */
#define MAX_CRC32_LEN 8
#define MAX_VALKIND_LEN	4	/* Spec, Conf, RO, or Set		*/
#define MAX_CDC_LEN	50	/* SPS, DPS, etc. (CURVE is longest	*/
	/* predefined CDC but user may define others)*/
#define MAX_FC_LEN	2	/* ST, MX, etc.				*/
#define VERSION_LEN 5 // V1.00

	/* These defines used in SCL_DA struct to differentiate between structs	*/
	/* containing DA info and structs containing SDO info.			*/
#define SCL_OBJTYPE_DA	0
#define SCL_OBJTYPE_SDO	1

	/* This def used for flattened leaf names (longer to allow array indices)*/
#define MAX_FLAT_LEN	(MAX_IDENT_LEN*2)

	/************************************************************************/
	/* Structures to contain information from "substation" section of SCL.	*/
	/************************************************************************/
	typedef struct scl_lnode
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_lnode *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_lnode *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR lnClass[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		ST_CHAR *lnInst;
		ST_CHAR *iedName;
		ST_CHAR *ldInst;
		ST_CHAR *prefix;
		ST_CHAR *lnType;
		ST_CHAR flattened[MAX_FLAT_LEN + 1];	/* Created by concatenating values*/
	} SCL_LNODE;

	typedef struct scl_terminal
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_terminal *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_terminal *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR connectivityNode[MAX_IDENT_LEN+1];
		ST_CHAR substationName[MAX_IDENT_LEN+1];
		ST_CHAR voltageLevelName[MAX_IDENT_LEN+1];
		ST_CHAR bayName[MAX_IDENT_LEN+1];
		ST_CHAR cNodeName[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
	} SCL_TERMINAL;
	typedef struct scl_subequipment
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_subequipment *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_subequipment *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		ST_CHAR *phase;
		ST_CHAR bvirtual;
		/* may be long so allocate if present*/
	//	SCL_LNODE *lnHead;		/* head of list of LNode defs	*/
	} SCL_SUBEQUIPMENT;
	typedef struct scl_tapchanger
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_tapchanger *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_tapchanger *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR type[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		ST_CHAR bvirtual;
		/* may be long so allocate if present*/
	//	SCL_LNODE *lnHead;		/* head of list of LNode defs	*/
	} SCL_TAPCHANGER;

	typedef struct scl_transformerwinding
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_transformerwinding *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_transformerwinding *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR type[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		ST_CHAR bvirtual;
		/* may be long so allocate if present*/
	//	SCL_LNODE *lnHead;		/* head of list of LNode defs	*/
		SCL_TERMINAL *tlHead;
		SCL_SUBEQUIPMENT *seHead;
		SCL_TAPCHANGER *tcHead;
	} SCL_TRANSFORMERWINDING;
	typedef struct scl_powertransformer
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_powertransformer *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_powertransformer *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR type[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		ST_CHAR bvirtual;
		ST_CHAR flattened[MAX_FLAT_LEN + 1];	/* Created by concatenating values*/
		ST_UCHAR wdcounts; //绕组数
		/* may be long so allocate if present*/
	//	SCL_LNODE *lnHead;		/* head of list of LNode defs	*/
		SCL_TRANSFORMERWINDING *twHead;
	} SCL_POWERTRANSFORMER;

	typedef struct scl_generalequipment
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_generalequipment *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_generalequipment *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR type[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		ST_CHAR bvirtual;
		/* may be long so allocate if present*/
	//	SCL_LNODE *lnHead;		/* head of list of LNode defs	*/
	} SCL_GENERALEQUIPMENT;

	typedef struct scl_conductingequipment
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_conductingequipment *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_conductingequipment *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR type[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		ST_CHAR bvirtual;
		ST_CHAR flattened[MAX_FLAT_LEN + 1];	/* Created by concatenating values*/
		/* may be long so allocate if present*/
	//	SCL_LNODE *lnHead;		/* head of list of LNode defs	*/
		SCL_TERMINAL *tlHead;
		SCL_SUBEQUIPMENT *seHead;
	} SCL_CONDUCTINGEQUIPMENT;

	typedef struct scl_connectivitynode
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_connectivitynode *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_connectivitynode *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR pathName[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		/* may be long so allocate if present*/
	//	SCL_LNODE *lnHead;		/* head of list of LNode defs	*/
	} SCL_CONNECTIVITYNODE;

	typedef struct scl_bay
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_bay *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_bay *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		/* may be long so allocate if present*/
	//	SCL_LNODE *lnHead;		/* head of list of LNode defs	*/
	//	SCL_POWERTRANSFORMER *ptHead;
	//	SCL_GENERALEQUIPMENT *geHead;
	//	SCL_CONDUCTINGEQUIPMENT *ceHead;
		SCL_CONNECTIVITYNODE *cnHead;
	} SCL_BAY;

	typedef struct scl_voltagelevel
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_voltagelevel *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_voltagelevel *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		ST_CHAR *multi;
		ST_CHAR *unit;
		double volVal;
		/* may be long so allocate if present*/
	//	SCL_LNODE *lnHead;		/* head of list of LNode defs	*/
	//	SCL_POWERTRANSFORMER *ptHead;
	//	SCL_GENERALEQUIPMENT *geHead;
		SCL_BAY *bayHead;
	} SCL_VOLTAGELEVEL;

	typedef struct scl_subfunction
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_subfunction *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_subfunction *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		/* may be long so allocate if present*/
	//	SCL_LNODE *lnHead;		/* head of list of LNode defs	*/
	//	SCL_GENERALEQUIPMENT *geHead;
	} SCL_SUBFUNCTION;

	typedef struct scl_function
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_function *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_function *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		/* may be long so allocate if present*/
	//	SCL_LNODE *lnHead;		/* head of list of LNode defs	*/
		SCL_SUBFUNCTION *sfHead;
	//	SCL_GENERALEQUIPMENT *geHead;
	} SCL_FUNCTION;

	/* Data from "Substation" element	*/
	typedef struct scl_substation
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_substation *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_substation *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		/* may be long so allocate if present*/
		SCL_LNODE *lnHead;		/* head of list of LNode defs	*/
		SCL_POWERTRANSFORMER *ptHead;  //变压器链表
		SCL_GENERALEQUIPMENT *geHead;
		SCL_VOLTAGELEVEL *vlHead;
		SCL_FUNCTION *ftHead;
		SCL_CONDUCTINGEQUIPMENT *ceHead; //其它 一次设备链表
	} SCL_SUBSTATION;

	/************************************************************************/
	/* Structures to contain information from "Header" section of SCL.	*/
	/************************************************************************/
	typedef struct
	{
		/* NOTE: only required elements included here. Add optional elements as needed.*/
		ST_CHAR id            [MAX_IDENT_LEN+1];
		ST_CHAR sclCrc		  [MAX_CRC32_LEN+1];
		/* Defined values for "nameStructure" attribute	*/
#define SCL_NAMESTRUCTURE_IEDNAME	0	/* value="IEDName"	*/
#define SCL_NAMESTRUCTURE_FUNCNAME	1	/* value="FuncName"	*/
		ST_INT nameStructure;
	} SCL_HEADER;

	/************************************************************************/
	/* Structures to contain information from "Communications" section of SCL.*/
	/************************************************************************/
	/* Data from "GSE" element (inside "ConnectedAP" element)	*/
	typedef struct scl_gse
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_gse *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_gse *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR ldInst[MAX_IDENT_LEN+1];
		ST_CHAR cbName[MAX_IDENT_LEN+1];
		ST_CHAR MAC[CLNP_MAX_LEN_MAC+1];	/* Multicast MAC address like 010CCD010000*/
		ST_UINT APPID;
		ST_UINT VLANPRI;
		ST_UINT VLANID;
		ST_UINT minTime;
		ST_UINT maxTime;
	} SCL_GSE;

	/* Data from "SMV" element (inside "ConnectedAP" element)	*/
	typedef struct scl_smv
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_smv *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_smv *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR ldInst[MAX_IDENT_LEN+1];
		ST_CHAR cbName[MAX_IDENT_LEN+1];
		ST_CHAR MAC[CLNP_MAX_LEN_MAC+1];	/* Multicast MAC address	*/
		ST_UINT APPID;
		ST_UINT VLANPRI;
		ST_UINT VLANID;
	} SCL_SMV;

	typedef struct scl_address
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		ST_CHAR IP[20+1]; /* IP address	*/
		ST_CHAR IPSUBNET[20+1];
		ST_CHAR IPGATEWAY[21];	
	} SCL_ADDRESS;

	typedef struct scl_ports
	{
		struct scl_ports *next;
		struct scl_ports *prev;
		ST_CHAR portCfg[MAX_IDENT_LEN+1];
	} SCL_PORT;
	
	/* Data from "ConnectedAP" element	*/
	typedef struct scl_cap
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_cap *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_cap *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR iedName[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		/* may be long so allocate if present*/
		ST_CHAR apName[MAX_IDENT_LEN+1];
		SCL_PORT *portHead;
		SCL_ADDRESS *addr;
		SCL_GSE *gseHead;      /* head of list of GSE defs	*/
		SCL_SMV *smvHead;      /* head of list of SMV defs	*/
	} SCL_CAP;

	/* Data from "Subnetwork" element	*/
	typedef struct scl_subnet
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_subnet *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_subnet *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		/* may be long so allocate if present*/
		ST_CHAR type[MAX_IDENT_LEN+1];
		SCL_CAP *capHead;		/* head of list of ConnectedAP defs	*/
	} SCL_SUBNET;

	/************************************************************************/
	/* Structures to contain information from "AccessPoint" section of SCL.*/
	/************************************************************************/

	/* "scl_fcda_add" allocates this struct, fills it in,		*/
	/* and adds it to the linked list "fcdaHead" in SCL_DATASET.		*/
	typedef struct
	{
		DBL_LNK l;
		ST_CHAR domName[MAX_IDENT_LEN+1];	/* domain name (constructed)	*/
		ST_CHAR ldInst  [MAX_IDENT_LEN+1];
		ST_CHAR prefix  [MAX_IDENT_LEN+1];
		ST_CHAR lnInst  [MAX_IDENT_LEN+1];
		ST_CHAR lnClass [MAX_IDENT_LEN+1];
		ST_CHAR doName  [MAX_IDENT_LEN+1];
		ST_CHAR daName  [MAX_IDENT_LEN+1];
		ST_CHAR fc      [MAX_FC_LEN+1];	/* ST, MX, etc.			*/
	} SCL_FCDA;

	/* "scl_dai_add" allocates this struct, fills it in,			*/
	/* and adds it to the linked list "daiHead" in SCL_LN.			*/
	/* The "flattened" name must be constructed from the "name" & "ix"	*/
	/* attribute of the DOI and DAI and possibly the intervening SDI,	*/
	/* where "ix" is an array index (we'll need some new flattened name	*/
	/* syntax to handle the array index).					*/
	/* The "accessControl" attr of DOI is ignored (don't know what it means).*/
	/* The "desc" attr of DOI, SDI, & DAI are ignored (not useful).		*/
	typedef struct
	{
		DBL_LNK l;
		ST_CHAR flattened[MAX_FLAT_LEN+1];	/* flattened attribute name	相当于DAI的name*/
		ST_CHAR *desc;			/* description (optional)*/
		/* constructed from "name" & "ix"*/
		/* from DOI, SDI, & DAI		*/
		ST_CHAR *Val;				/* attribute value text		*/
		/* allocate appropriate size buffer*/
		ST_UINT sGroup;			/* optional Setting Group Number*/
		ST_CHAR sAddr[MAX_IDENT_LEN+1];	/* from DAI			*/
		ST_CHAR valKind[MAX_VALKIND_LEN+1];	/* from DAI			*/
	} SCL_DAI;

	typedef struct
	{
		DBL_LNK l;
		ST_CHAR flattened[MAX_FLAT_LEN+1];	/*  name		*/
		ST_CHAR *desc;			/* description (optional)*/
		//SCL_SDI *ssdiHead;
		SCL_DAI *sdaiHead;
	} SCL_SDI;

	typedef struct
	{
		DBL_LNK l;
		ST_CHAR name[MAX_IDENT_LEN+1];	/*  name		*/
		ST_CHAR *desc;			/* description (optional)*/
		ST_UINT idx;
		SCL_SDI *sdiHead;
		SCL_DAI *daiHead;
	} SCL_DOI;

	/* "scl_dataset_add" allocates this struct,			*/
	/* and adds it to the linked list "datasetHead" in SCL_LN.	*/
	typedef struct
	{
		DBL_LNK l;
		ST_CHAR name[MAX_IDENT_LEN+1];	/* dataset name		*/
		ST_CHAR *desc;			/* description (optional)*/
		/* may be long so allocate if present*/
		SCL_FCDA *fcdaHead;			/* head of list of FCDA	*/
	} SCL_DATASET;

	/* "scl_rcb_add" allocates this struct, fills it in,		*/
	/* and adds it to the linked list "rcbHead" in SCL_LN.		*/
	typedef struct
	{
		DBL_LNK l;
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		/* may be long so allocate if present*/
		ST_CHAR datSet[MAX_IDENT_LEN+1];
		ST_UINT intgPd;
		ST_CHAR rptID[MAX_IDENT_LEN+1];
		ST_UINT confRev;
		ST_BOOLEAN buffered;		/* TRUE if this is buffered RCB	*/
		ST_UINT bufTime;
		ST_UINT8 TrgOps [1];		/* 8-bit bitstring			*/
		/* Boolean vals from SCL file		*/
		/* (dchg, qchg, dupd, & period)		*/
		/* used to set bits in TrgOps bitstring	*/
		ST_UINT8 OptFlds [2];		/* 9-bit bitstring			*/
		/* Boolean vals from SCL file		*/
		/* (seqNum, timeStamp, dataSet,		*/
		/* reasonCode, dataRef, bufOvfl,	*/
		/* entryID, configRef)			*/
		/* segmentation boolean is ignored	*/
		/* used to set bits in OptFlds bitstring*/
		ST_UINT maxClient;		/* value of "RptEnabled max" attr.	*/
	} SCL_RCB;			/* Report Control Block	*/

	typedef struct
	{
		/* Counters of different types of BRCB.	*/
		ST_UINT brcb_count_complex;
		ST_UINT brcb_count_ST;
		ST_UINT brcb_count_MX;
		ST_UINT brcb_count_CO;
		ST_UINT brcb_count_SP;
		ST_UINT brcb_count_SG;
		ST_UINT brcb_count_SE;
		ST_UINT brcb_count_SV;
		ST_UINT brcb_count_CF;
		ST_UINT brcb_count_DC;
		ST_UINT brcb_count_EX;

		/* Counters of different types of URCB.	*/
		ST_UINT urcb_count_complex;
		ST_UINT urcb_count_ST;
		ST_UINT urcb_count_MX;
		ST_UINT urcb_count_CO;
		ST_UINT urcb_count_SP;
		ST_UINT urcb_count_SG;
		ST_UINT urcb_count_SE;
		ST_UINT urcb_count_SV;
		ST_UINT urcb_count_CF;
		ST_UINT urcb_count_DC;
		ST_UINT urcb_count_EX;
	} SCL_RCB_COUNTERS;

	/************************************************************************/
	/*			LOG Control Block				*/
	/* "scl_lcb_add" allocates this struct, fills it in,			*/
	/* and adds it to the linked list "lcbHead" in SCL_LN.			*/
	typedef struct
	{
		DBL_LNK l;
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		/* may be long so allocate if present*/
		ST_CHAR datSet[MAX_IDENT_LEN+1];
		ST_UINT intgPd;
		ST_CHAR logName[MAX_IDENT_LEN+1];
		ST_BOOLEAN logEna;
		ST_BOOLEAN reasonCode;
		ST_UINT8 TrgOps [1];		/* 8-bit bitstring			*/
		/* Boolean vals from SCL file		*/
		/* (dchg, qchg, dupd, & period)		*/
		/* used to set bits in TrgOps bitstring	*/
	} SCL_LCB;

	typedef struct
	{
		DBL_LNK l;
		ST_CHAR IEDName[MAX_IDENT_LEN+1];
	}SCL_IEDNAME;

	/************************************************************************/
	/*			GOOSE Control Block				*/
	/* "scl_gcb_add" allocates this struct, fills it in,			*/
	/* and adds it to the linked list "gcbHead" in SCL_LN.			*/
	typedef struct
	{
		DBL_LNK l;
		ST_CHAR name[MAX_IDENT_LEN+1];	/* Name of CB. Used to construct*/
		/* GoCBRef or GsCBRef		*/
		ST_CHAR *desc;			/* description (optional)*/
		/* may be long so allocate if present*/
		ST_CHAR datSet[MAX_IDENT_LEN+1];	/* for GOOSE only	*/
		/* used to construct GOOSE DatSet*/
		ST_UINT confRev;			/* for GOOSE only	*/
		ST_BOOLEAN isGoose;	/* SD_TRUE if "GOOSE", SD_FALSE if "GSSE"*/
		ST_CHAR appID[MVL61850_MAX_RPTID_LEN+1];	/* for GOOSE only	*/
		/* maps to GoID in 61850-7-2	*/
		/* The SCL file may also contain one or more "IEDName" elements to	*/
		/* indicate IEDs that should subscribe for GOOSE data. We have no	*/
		/* way to use this information, so it is ignored.			*/
		SCL_IEDNAME *iedNHead; //接收此GOOSE 的设备 列表
	} SCL_GCB;

	/************************************************************************/
	/*			Sampled Value Control Block			*/
	/* "scl_parse" allocates this struct and fills it in.			*/
	/* "scl_svcb_add" adds it to the linked list "svcbHead" in SCL_LN.	*/

	typedef struct
	{
		DBL_LNK l;
		ST_CHAR name[MAX_IDENT_LEN+1];
		ST_CHAR *desc;		/* description (optional)*/
		/* may be long so allocate if present*/
		ST_CHAR datSet[MAX_IDENT_LEN+1];
		ST_CHAR smvID[MAX_IDENT_LEN+1];
		ST_UINT smpRate;
		ST_UINT nofASDU;
		ST_UINT confRev;
		ST_BOOLEAN multicast;		/* TRUE if this is MsvCB		*/
		ST_UINT8 OptFlds [1];		/* 8-bit bitstring			*/
		/* Boolean vals from "SmvOpts" in SCL	*/
		/* (sampleRate, etc.)			*/
		/* used to set bits in this bitstring	*/
		ST_BOOLEAN securityPres;	/* SmvOpts security flag	*/
		ST_BOOLEAN dataRefPres;	/* SmvOpts dataRef flag		*/
		SCL_IEDNAME *iedNHead; //接收此MU 的设备 列表
	} SCL_SVCB;			/* Sampled Value Control Block	*/

	/************************************************************************/
	/*			Setting Group Control Block			*/
	typedef struct
	{
		/* NOTE: no DBL_LNK here. Only 2 allowed so never put on a linked list.*/
		ST_CHAR *desc;		/* description (optional)		*/
		/* may be long so allocate if present	*/
		ST_UINT numOfSGs;		/* mandatory	*/
		ST_UINT actSG;		/* optional	*/
	} SCL_SGCB;

	/************************************************************************/
	/*			Logical Node structure 				*/
	/* "scl_ln_add" allocates this struct, fills it in,		*/
	/* and adds it to the linked list "lnHead" in SCL_LD.		*/
	typedef struct
	{
		DBL_LNK l;
		ST_CHAR varName[MAX_IDENT_LEN+1];	/* variable name (constructed)	*/
		ST_CHAR *desc;			/* description (optional)*/
		/* may be long so allocate if present*/
		ST_CHAR lnType[MAX_IDENT_LEN+1];	/* LN Type name		*/
		ST_CHAR lnClass[MAX_IDENT_LEN+1];	/* LN Class name	*/
		/* for LN0, must be "LLN0"	*/
		ST_CHAR inst[MAX_IDENT_LEN+1];	/* LN inst name			*/
		/* for LN0, must be "" (empty string)*/
		ST_CHAR prefix[MAX_IDENT_LEN+1];	/* LN prefix name	*/
		/* for LNO, ignored	*/
		SCL_DOI     *doiHead;		/* head of list of DAI	*/
		SCL_DATASET *datasetHead;	/* head of list of DataSet	*/
		SCL_RCB     *rcbHead;		/* head of list of RCB (Report Control)	*/
		SCL_LCB     *lcbHead;		/* head of list of LCB (Log Control)	*/
		SCL_GCB     *gcbHead;		/* head of list of GCB (GOOSE Control)	*/
		SCL_SVCB    *svcbHead;	/* head of list of SVCB (Sampled Value Control)*/
		SCL_SGCB    *sgcb;		/* SGCB (Setting Group Control)(only 1 allowed)*/
		/* NOTE: In LN or LN0: Inputs ignored		*/
		/* NOTE: In LN0: SCLControl ignored		*/
	} SCL_LN;			/* Logical Node (LN or LN0 in SCL)	*/

	/************************************************************************/
	/*			Logical Device structure 			*/
	/* "scl_ld_create" allocates this struct			*/
	/* and adds it to the linked list "ldHead" in SCL_INFO.		*/
	typedef struct
	{
		DBL_LNK l;
		ST_CHAR domName[MAX_IDENT_LEN+1];	/* ied name (constructed)	*/
		ST_CHAR apName[MAX_IDENT_LEN+1]; //
		ST_CHAR *desc;			/* description (optional)*/
		/* may be long so allocate if present*/
		ST_CHAR inst[MAX_IDENT_LEN+1];	/* LD inst name		*/
		SCL_LN *lnHead;			/* head of list of LN	*/
		/* NOTE: AccessControl in LDevice is ignored	*/
	} SCL_LD;			/* Logical Device (LDevice in SCL)*/


	typedef struct
	{
		DBL_LNK l;
		ST_CHAR iedName[MAX_IDENT_LEN+1];	/* ied name (constructed)	*/
		ST_CHAR *desc;			/* description (optional)*/
		ST_CHAR *manufacturer;
		ST_CHAR *iedType;
		ST_CHAR configVersion[VERSION_LEN + 1];
		ST_CHAR iedDeviceCrc [MAX_CRC32_LEN+1];
	} SCL_IED;			/* IED*/

	/************************************************************************/
	/* Structures to contain information from "DataTypeTemplates" section of SCL.*/
	/************************************************************************/
	/* This structure should be allocated and filled in by the function	*/
	/* "scl_lntype_add_do".							*/
	typedef struct scl_do
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_do *next;			/* CRITICAL: DON'T MOVE.	*/
		struct scl_do *prev;			/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR name[MAX_IDENT_LEN+1];	/* data object name		*/
		ST_CHAR type[MAX_IDENT_LEN+1];	/* data object type		*/
		ST_CHAR *desc;
	} SCL_DO;

	typedef struct scl_lntype
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_lntype *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_lntype *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR id[MAX_IDENT_LEN+1];			/* name used to reference this LN Type*/
		ST_CHAR lnClass[MAX_IDENT_LEN+1];		/* logical node class		*/
		SCL_DO *doHead;		/* head of list of DO	*/
		/* scl_lntype_add_do adds to list	*/
		ST_INT type_id;		/* Initialized by "scl2_datatype_create_all"*/
	} SCL_LNTYPE;


	/* This structure should be allocated and filled in by the function	*/
	/* "scl_dotype_add_da" OR "scl_dotype_add_sdo", and possibly modified by the optional	*/
	/* function "scl_dotype_add_da_val".					*/
	/* NOTE: the same structure must be used for DA or SDO because each must	*/
	/* be put on the same linked list in the order they are read from the SCL file.*/
	/* Most of the parameters are relevant only for DA elements. They are	*/
	/* ignored if this is an SDO (i.e. objtype=SCL_OBJTYPE_SDO).		*/
	typedef struct
	{
		DBL_LNK l;
		ST_CHAR name[MAX_IDENT_LEN+1];	/* DA or SDO name		*/
		ST_CHAR *desc;			/* description (optional)*/
		/* may be long so allocate if present*/
		ST_CHAR sAddr[MAX_IDENT_LEN+1];	/* for DA only: DA sAddr	*/
		ST_CHAR bType[MAX_IDENT_LEN+1];	/* for DA only: DA bType	*/
		ST_CHAR valKind[MAX_VALKIND_LEN+1];	/* for DA only: Spec, Conf, RO, or Set	*/
		ST_CHAR type[MAX_IDENT_LEN+1];	/* for DA: needed if bType="Struct" or "Enum"*/
		/* for SDO: required		*/
		ST_UINT count;			/* for DA only: num array entries*/
		ST_CHAR fc[MAX_FC_LEN+1];		/* for DA only: functional constraint	*/
		ST_BOOLEAN dchg;			/* for DA only: TrgOp (data change)	*/
		ST_BOOLEAN qchg;			/* for DA only: TrgOp (quality change)	*/
		ST_BOOLEAN dupd;			/* for DA only: TrgOp (data update)	*/

		/* The "Val" and "sGroup" parameters are only set if the SCL file contains the
		* optional "Val" element, in which case "scl_dotype_add_da_val" is called.
		*/
		ST_CHAR *Val;				/* for DA only: attribute value text	*/
		/* allocate appropriate size buffer*/
		ST_UINT sGroup;			/* for DA only: optional Setting Group Number*/
	} SCL_DA;

	typedef struct scl_dotype
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		struct scl_dotype *next;		/* CRITICAL: DON'T MOVE.	*/
		struct scl_dotype *prev;		/* CRITICAL: DON'T MOVE.	*/
		ST_CHAR id[MAX_IDENT_LEN+1];	/* name used to reference this DO Type	*/
		ST_CHAR cdc[MAX_CDC_LEN+1];	/* CDC name				*/
		ST_CHAR *desc;
		SCL_DA *daHead;		/* head of list of DA 		*/
		SCL_DO *sdoHead;
		/* scl_dotype_add_da OR			*/
		/* scl_dotype_add_sdo adds to list	*/
	} SCL_DOTYPE;

	/* This structure should be allocated and filled in by the function	*/
	/* "scl_datype_add_bda".						*/
	typedef struct
	{
		DBL_LNK l;
		ST_CHAR name[MAX_IDENT_LEN+1];		/* data attribute name		*/
		ST_CHAR *desc;			/* description (optional)*/
		/* may be long so allocate if present*/
		ST_CHAR sAddr[MAX_IDENT_LEN+1];		/* for DA only: DA sAddr	*/
		ST_CHAR bType[MAX_IDENT_LEN+1];		/* data attribute type		*/
		ST_CHAR valKind[MAX_VALKIND_LEN+1];		/* Spec, Conf, RO, or Set	*/
		ST_CHAR type[MAX_IDENT_LEN+1];		/* only used if btype="Struct" or "Enum"*/
		ST_UINT count;	/* for DA only: num array entries*/

		/* The "Val" and "sGroup" parameters are only set if the SCL file contains the
		* optional "Val" element, in which case "scl_datype_add_bda_val" is called.
		*/
		ST_CHAR *Val;				/* attribute value text		*/
		/* allocate appropriate size buffer*/
		ST_UINT sGroup;			/* optional Setting Group Number*/
	} SCL_BDA;			/* Basic Data Attribute		*/
	typedef struct
	{
		DBL_LNK l;
		ST_CHAR id[MAX_IDENT_LEN+1];			/* name used to reference this DA Type*/
		SCL_BDA *bdaHead;		/* head of list of BDA	*/
		/* scl_datype_add_bda adds to list	*/
	} SCL_DATYPE;


	/* This structure should be allocated and filled in by the function	*/
	/* "scl_enumtype_add_enumval".						*/
	typedef struct
	{
		DBL_LNK l;
		ST_INT ord;				/* ord attribute	*/
		ST_CHAR EnumVal[MAX_IDENT_LEN+1];	/* EnumVal element		*/
		/* TRUNCATED if longer than buffer	*/
	} SCL_ENUMVAL;
	typedef struct
	{
		DBL_LNK l;
		ST_CHAR id[MAX_IDENT_LEN+1];			/* name used to reference this DA Type*/
		SCL_ENUMVAL *enumvalHead;	/* head of list of EnumVal	*/
		/* scl_enumtype_add_enumval adds to list*/
	} SCL_ENUMTYPE;

	/* Data from "Subnetwork" element	*/
	typedef struct scl_accesspoint
	{
		/* CRITICAL: First 2 parameters used to add this struct to linked	*/
		/* lists using list_add_last, etc.					*/
		DBL_LNK l;
		ST_CHAR name[FIX_STR_LEN_3+1];		//S1 G1 M1
		ST_CHAR *desc;		/* description (optional)*/
		/* may be long so allocate if present*/
		ST_CHAR router[FIX_STR_LEN_5+1];	//only choose True or false
		ST_CHAR clock[FIX_STR_LEN_5+1];		//only choose True or false
		/* Logical Device (MMS Domain) definitions (from AccessPoint section)	*/
		SCL_LD *ldHead;		/* head of list of LDevice defs		*/
	} SCL_ACCESSPOINT;
	/************************************************************************/
	/*			SCL_INFO					*/
	/* This structure contains all information extracted from the SCL file	*/
	/* to be used for MMS-EASE Lite initialization.				*/
	/************************************************************************/
	typedef struct
	{
        SCL_IED *lIEDHead;

		SCL_HEADER Header;		/* Info from "Header" section of SCL file*/

		SCL_SUBSTATION *substationHead;
		/* SubNetwork definitions from (from Communication section)		*/
		SCL_SUBNET *subnetHead;	/* head of list of SubNetwork defs	*/

		SCL_ACCESSPOINT *accessPointHead;
		/* Logical Node Type definitions (from DataTypeTemplates section)	*/
		SCL_LNTYPE *lnTypeHead;	/* head of list	of LNodeType defs	*/
		SCL_DOTYPE *doTypeHead;	/* head of list of DOType defs		*/
		SCL_DATYPE *daTypeHead;	/* head of list of DAType defs		*/
		SCL_ENUMTYPE *enumTypeHead;	/* head of list of EnumType defs	*/

	} SCL_INFO;

	/************************************************************************/
	/************************************************************************/
	/* FUNCTIONS to store SCL info in "SCL_INFO" structure.			*/
	/************************************************************************/
	/************************************************************************/

	SCL_LNTYPE *scl_lntype_create (
		SCL_INFO *scl_info);

	SCL_DO *scl_lntype_add_do (
		SCL_INFO *scl_info);

	SCL_DOTYPE *scl_dotype_create (
		SCL_INFO *scl_info);

	SCL_DA *scl_dotype_add_da (
		SCL_INFO *scl_info);

	SCL_DO *scl_dotype_add_sdo (
		SCL_INFO *scl_info);

	SCL_DATYPE *scl_datype_create (
		SCL_INFO *scl_info);

	SCL_BDA *scl_datype_add_bda (
		SCL_INFO *scl_info);

	SCL_ENUMTYPE *scl_enumtype_create (
		SCL_INFO *scl_info);

	SCL_ENUMVAL *scl_enumtype_add_enumval (
		SCL_INFO *scl_info);

	SCL_FCDA *scl_fcda_add (
		SCL_INFO *scl_info);

	SCL_DOI *scl_doi_add (
		SCL_INFO *scl_info);

	SCL_SDI *scl_sdi_add (
		SCL_INFO *scl_info);

	SCL_DAI *scl_dai_add (
		SCL_INFO *scl_info);
		
	SCL_DAI *scl_sdi_dai_add(
		SCL_INFO *scl_info);

	SCL_DATASET *scl_dataset_add (
		SCL_INFO *scl_info);

	SCL_RCB *scl_rcb_add (
		SCL_INFO *scl_info);

	SCL_LCB *scl_lcb_add (
		SCL_INFO *scl_info);

	SCL_GCB *scl_gcb_add (
		SCL_INFO *scl_info);

	SCL_SGCB *scl_sgcb_add (
		SCL_INFO *scl_info);

	SCL_SVCB *scl_svcb_add (
		SCL_INFO *scl_info);

	SCL_LN *scl_ln_add (
		SCL_INFO *scl_info);

	SCL_LD *scl_ld_create (
		SCL_INFO *scl_info);

	SCL_SUBNET *scl_subnet_add (
		SCL_INFO *scl_info);

	SCL_CAP *scl_cap_add (
		SCL_INFO *scl_info);

	SCL_GSE *scl_gse_add (
		SCL_INFO *scl_info);

	SCL_SMV *scl_smv_add (SCL_INFO *scl_info);

	SCL_ADDRESS *scl_address_add (SCL_INFO *scl_info);

	SCL_PORT *scl_port_add (SCL_INFO *scl_info);

	SCL_ACCESSPOINT *scl_accesspoint_add (SCL_INFO *scl_info);
	/************************************************************************/
	/*			scl_parse					*/
	/* Parses SCL file and stores extracted info in SCL_INFO structure.	*/
	/************************************************************************/
	SCDPAESE_API ST_RET scl_parse (ST_CHAR *xmlFileName, ST_CHAR *iedName, 
		ST_CHAR *accessPointName, SCL_INFO *sclInfo);
	SCDPAESE_API ST_VOID scl_log_all (SCL_INFO *scl_info);

	/************************************************************************/
	/*			scl_info_destroy				*/
	/* Destroy all info stored in the SCL_INFO structure by "scl_parse".	*/
	/************************************************************************/
	SCDPAESE_API ST_VOID scl_info_destroy (SCL_INFO *scl_info);

	/************************************************************************/
	/************************************************************************/

	extern ST_BOOLEAN scl_debug_mode;
	extern ST_INT scl_debug_mode_error_count;

	/************************************************************************/
	/* Functions to find objects in SCL_INFO.				*/
	/************************************************************************/
	SCL_GSE *scl_gse_find (SCL_INFO *scl_info, SCL_LD *scl_ld, SCL_GCB *scl_gcb);

#ifdef __cplusplus
}
#endif

#endif	/* !SCL_INCLUDED	*/
