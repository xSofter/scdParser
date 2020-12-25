/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*   (c) Copyright Systems Integration Specialists Company, Inc.,       */
/*	2004-2006, All Rights Reserved					*/
/*                                                                      */
/* MODULE NAME : sclparse.c                                             */
/* PRODUCT(S)  :                                                        */
/*                                                                      */
/* MODULE DESCRIPTION : This routine parses XML files conforming to the */
/*                      SCL object model.			        */
/*                                                                      */
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :                            */
/*                                                                      */
/* MODIFICATION LOG :                                                   */
/*  Date     Who   Rev          Comments                                */
/* --------  ---  ------   -------------------------------------------  */
/* 05/06/08  JRB    23     Chk ReportControl indexed attr (must be true)*/

/************************************************************************/

#include "glbtypes.h"
#include "sysincs.h"
#include "mem_chk.h"
#include "sx_defs.h"
#include "str_util.h"
#include "scl.h"	/* SCL file processing structs & functions	*/
#include "slog.h"
/************************************************************************/
/* For debug version, use a static pointer to avoid duplication of      */
/* __FILE__ strings.                                                    */

#ifdef DEBUG_SISCO
SD_CONST static ST_CHAR *SD_CONST thisFileName = __FILE__;
#endif

#define SCL_ATTR_OPTIONAL 	0	/* attribute is optional	*/
#define SCL_ATTR_REQUIRED 	1	/* attribute is required	*/

/* Bit numbers in OptFlds bitstring (configured by SmvOpts in SCL file)	*/
#define SVOPT_BITNUM_SMPRATE	0
#define SVOPT_BITNUM_REFRTM	1
#define SVOPT_BITNUM_SMPSYNCH	2

/* TrgOps bit numbers for IEC-61850.					*/
/* Use "bit" macros (BSTR_BIT_*) to access each individual bit.		*/
#define TRGOPS_BITNUM_RESERVED			0
#define TRGOPS_BITNUM_DATA_CHANGE		1	/* "dchg" in some specs	*/
#define TRGOPS_BITNUM_QUALITY_CHANGE		2	/* "qchg" in some specs	*/
#define TRGOPS_BITNUM_DATA_UPDATE		3	/* "dupd" in some specs	*/
#define TRGOPS_BITNUM_INTEGRITY			4	/* "period" in 61850-6	*/
#define TRGOPS_BITNUM_GENERAL_INTERROGATION	5

/* OptFlds bit numbers for IEC-61850.					*/
/* Use "bit" macros (BSTR_BIT_*) to access each individual bit.		*/
/* NOTE: DATSETNAME in IEC-61850 same as OUTDAT in UCA.			*/
/* NOTE: bit numbers 1 thru 4 correspond to the masks MVLU_SQNUM_MASK,	*/
/*       MVLU_RPTTIM_MASK, MVLU_REASONS_MASK, MVLU_OUTDAT_MASK above.	*/
/*	 Bit masks don't work well beyond 8 bits.			*/
#define OPTFLD_BITNUM_RESERVED		0
#define OPTFLD_BITNUM_SQNUM		1
#define OPTFLD_BITNUM_TIMESTAMP		2
#define OPTFLD_BITNUM_REASON		3
#define OPTFLD_BITNUM_DATSETNAME	4
#define OPTFLD_BITNUM_DATAREF		5
#define OPTFLD_BITNUM_BUFOVFL		6
#define OPTFLD_BITNUM_ENTRYID		7
#define OPTFLD_BITNUM_CONFREV		8
#define OPTFLD_BITNUM_SUBSEQNUM		9	/* segmentation in 61850-8-1*/


/* Macros to access each individual bit of any bitstring.		*/
#define BSTR_BIT_SET_ON(ptr,bitnum) \
	( ((ST_UINT8 *)(ptr))[(bitnum)/8] |= (0x80>>((bitnum)&7)) )
	
#define BSTR_BIT_SET_OFF(ptr,bitnum) \
	( ((ST_UINT8 *)(ptr))[(bitnum)/8] &= ~(0x80>>((bitnum)&7)) )

/* BSTR_BIT_GET returns 0 if bit is clear, 1 if bit is set.	*/
#define BSTR_BIT_GET(ptr,bitnum) \
	(( ((ST_UINT8 *)(ptr))[(bitnum)/8] &  (0x80>>((bitnum)&7)) ) ? "true" : "false" )

typedef struct scl_dec_ctrl
{
	ST_CHAR iedName[MAX_IDENT_LEN+1];
	ST_CHAR accessPointName[MAX_IDENT_LEN+1];
	ST_BOOLEAN accessPointFound;	/* SD_TRUE if IED and AccessPoint found	*/
	ST_BOOLEAN iedNameMatched;	
	ST_BOOLEAN accessPointMatched;
	SCL_ACCESSPOINT *scl_apInfo;
	SCL_INFO *sclInfo;	/* save scl info for user*/
	SCL_CAP* scl_cap;	//申请cap的访问指针,该指针指向新申请的地址
	SCL_GSE *scl_gse;	/* Used for "GSE" in "Communication" section	*/
	SCL_SMV *scl_smv;	/* Used for "SMV" in "Communication" section	*/
	SCL_PORT* scl_ports;	//用于访问申请的地址
	SCL_ADDRESS *scl_addr;	/* 站控层地址*/
	SCL_LD *scl_ld;	/* Used for "LDevice"				*/
	SCL_LN *scl_ln;	/* Used for "LN" (Logical Node)			*/
	SCL_RCB *scl_rcb;	/* alloc to store ReportControl info		*/
	SCL_LCB *scl_lcb;	/* alloc to store LogControl info		*/
	ST_UINT8 TrgOps[1];	/* Used for ReportControl or LogControl.	*/
	/* Copied to SCL_RCB or SCL_LCB.		*/
	SCL_SVCB *scl_svcb;	/* Used for "SampledValueControl".	*/
	SCL_ENUMVAL *scl_enumval;	/* Used for "EnumVal".			*/
	SCL_DAI *scl_dai;	/* Used for "DAI".				*/
	SCL_DA *scl_da;	/* Used for "DA".				*/
	SCL_BDA *scl_bda;	/* Used for "BDA".				*/
	ST_CHAR flattened[MAX_FLAT_LEN + 1];	/* Created by concatenating values*/
	/* from DOI, SDI, and DAI elements*/
} SCL_DEC_CTRL;

/************************************************************************/
static ST_VOID _SCL_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _Header_SFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _Substation_CrcFun(SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _Communication_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _Communication_PortMapFun(SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _SubNetwork_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _ConnectedAP_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _GSE_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _GSE_Address_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _GSE_MinTime_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _GSE_MaxTime_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _GSE_Address_P_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _S1_ConnPortFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _SMV_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _SMV_Address_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _SMV_Address_P_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _Address_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _Address_P_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _Connection_P_Port_SEFun (SX_DEC_CTRL *sxDecCtrl);

static ST_VOID _IED_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _AccessPoint_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _AccessPoint_PrivateFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _Server_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _LDevice_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _LN_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _DataSet_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _FCDA_SFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _ReportControl_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _LogControl_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _GSEControl_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _SettingControl_SFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _TrgOps_SFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _OptFlds_SFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _RptEnabled_SFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _DOI_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _SDI_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _DOI_DAI_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _SDI_SDI_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _SDI_DAI_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _DAI_Val_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _DataTypeTemplates_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _LNodeType_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _DO_SFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _DOType_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _DA_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _SDO_SFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _DAType_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _DA_Val_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _BDA_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _BDA_Val_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _EnumType_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _EnumVal_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_RET _scl_unknown_el_start (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *tag);
static ST_RET _scl_unknown_el_end (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *tag);
static ST_VOID _SampledValueControl_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _SmvOpts_SFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _IEDName_EFun (SX_DEC_CTRL *sxDecCtrl); //for gsecontrol
static ST_VOID _IEDName2_EFun (SX_DEC_CTRL *sxDecCtrl); //for smvcontrol

static ST_VOID _Substation_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _LNode_SFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _PowerTransformer_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _GeneralEquipment_SEFun(SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _VoltageLevel_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _Function_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _TransformerWinding_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _Terminal_SFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _SubEquipment_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _TapChanger_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _Voltage_SFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _Bay_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _ConductingEquipment_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _ConnectivityNode_SEFun (SX_DEC_CTRL *sxDecCtrl);
static ST_VOID _SubFunction_SEFun (SX_DEC_CTRL *sxDecCtrl);

static int HexStrToInt(const ST_CHAR *p);
/************************************************************************/
/************************************************************************/
/* Only the elements we need to extract are listed here.		*/
/* The rest are handled by "unknown" element handler.			*/
SX_ELEMENT sclStartElements[] = 
{
	/*name			elementFlags			funcPtr			user	notused*/
	{"SCL", 		SX_ELF_CSTARTEND,		_SCL_SEFun, 	NULL, 	0}
};
/************************************************************************/
/* Tables for mapping "SCL" elements.				*/
/************************************************************************/
SX_ELEMENT SCLElements[] = 
{
	/*name					elementFlags						funcPtr			user	notused*/
	{"Private",           	SX_ELF_CSTARTEND|SX_ELF_OPTRPT, 	_Substation_CrcFun, NULL, 0},
	{"Header",           	SX_ELF_CSTART|SX_ELF_OPT, 			_Header_SFun, NULL, 0},
	// {"Substation",          SX_ELF_CSTARTEND|SX_ELF_OPTRPT, 	_Substation_SEFun, NULL, 0},
	{"Communication",    	SX_ELF_CSTARTEND|SX_ELF_OPT, 		_Communication_SEFun, NULL, 0},	//next CommunicationElements
	{"IED",            		SX_ELF_CSTARTEND|SX_ELF_OPTRPT,		_IED_SEFun, NULL, 0},
	{"DataTypeTemplates", 	SX_ELF_CSTARTEND|SX_ELF_OPT, 		_DataTypeTemplates_SEFun, NULL, 0}
};

/************************************************************************/
/* Tables for mapping "Substation" elements.				*/
/************************************************************************/
SX_ELEMENT SubstationElements[] = 
{
	/*name					elementFlags					funcPtr				user	notused*/
	{"LNode",      			SX_ELF_CSTART|SX_ELF_OPTRPT,	_LNode_SFun,			 NULL, 0},
	{"PowerTransformer",    SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_PowerTransformer_SEFun, NULL, 0},
	//{"GeneralEquipment",    SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_GeneralEquipment_SEFun, NULL, 0},
	{"VoltageLevel",      	SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_VoltageLevel_SEFun, 	 NULL, 0},
	//{"Function",      	SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_Function_SEFun, NULL, 0}
};

SX_ELEMENT PowerTransformerElements[] = 
{
	{"LNode",      	SX_ELF_CSTART|SX_ELF_OPTRPT,	_LNode_SFun, NULL, 0},
	{"TransformerWinding",     SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_TransformerWinding_SEFun, NULL, 0}
};

SX_ELEMENT TransformerWindingElements[] = 
{
	{"LNode",      	SX_ELF_CSTART|SX_ELF_OPTRPT,	_LNode_SFun, NULL, 0},
	//{"Terminal",     SX_ELF_CSTART|SX_ELF_OPTRPT,	_Terminal_SFun, NULL, 0},
	//{"SubEquipment",     SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_SubEquipment_SEFun, NULL, 0},
	//{"TapChanger",     SX_ELF_CSTARTEND|SX_ELF_OPT,	_TapChanger_SEFun, NULL, 0}
};

SX_ELEMENT SubEquipmentElements[] = 
{
	{"LNode",      	SX_ELF_CSTART|SX_ELF_OPTRPT,	_LNode_SFun, NULL, 0}
};

SX_ELEMENT TapChangerElements[] = 
{
	{"LNode",      	SX_ELF_CSTART|SX_ELF_OPTRPT,	_LNode_SFun, NULL, 0}
};


SX_ELEMENT GeneralEquipmentElements[] = 
{
	{"LNode",      	SX_ELF_CSTART|SX_ELF_OPTRPT,	_LNode_SFun, NULL, 0}
};

SX_ELEMENT VoltageLevelElements[] = 
{
	{"LNode",      	SX_ELF_CSTART|SX_ELF_OPTRPT,	_LNode_SFun, NULL, 0},
	{"PowerTransformer",    SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_PowerTransformer_SEFun, NULL, 0},
	//{"GeneralEquipment",    SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_GeneralEquipment_SEFun, NULL, 0},
	{"Voltage",      	SX_ELF_CSTART|SX_ELF_OPT,	_Voltage_SFun, NULL, 0},
	{"Bay",      	SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_Bay_SEFun, NULL, 0}
};

SX_ELEMENT BayElements[] = 
{
	{"LNode",      	SX_ELF_CSTART|SX_ELF_OPTRPT,	_LNode_SFun, NULL, 0},
	{"PowerTransformer",    SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_PowerTransformer_SEFun, NULL, 0},
	//{"GeneralEquipment",    SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_GeneralEquipment_SEFun, NULL, 0},
	{"ConductingEquipment",     SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_ConductingEquipment_SEFun, NULL, 0},
	//{"ConnectivityNode",      	SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_ConnectivityNode_SEFun, NULL, 0}
};

SX_ELEMENT ConductingEquipmentElements[] = 
{
	{"LNode",      			SX_ELF_CSTART|SX_ELF_OPTRPT,	_LNode_SFun, NULL, 0},
	//{"Terminal",     SX_ELF_CSTART|SX_ELF_OPTRPT,	_Terminal_SFun, NULL, 0},
	//{"SubEquipment",     SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_SubEquipment_SEFun, NULL, 0}
};

SX_ELEMENT ConnectivityNodeElements[] = 
{
	{"LNode",      			 SX_ELF_CSTART|SX_ELF_OPTRPT,	_LNode_SFun, NULL, 0}
};

SX_ELEMENT FunctionElements[] = 
{
	{"LNode",      			 SX_ELF_CSTART|SX_ELF_OPTRPT,	_LNode_SFun, NULL, 0},
	{"SubFunction",     	 SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_SubFunction_SEFun, NULL, 0},
	{"GeneralEquipment",     SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_GeneralEquipment_SEFun, NULL, 0}
};

SX_ELEMENT SubFunctionElements[] = 
{
	{"LNode",      			 SX_ELF_CSTART|SX_ELF_OPTRPT,	_LNode_SFun, NULL, 0},
	{"GeneralEquipment",     SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_GeneralEquipment_SEFun, NULL, 0}
};
/************************************************************************/
/* Tables for mapping "Communication" elements.				*/
/************************************************************************/
SX_ELEMENT CommunicationElements[] = 
{
	{"Private",   			SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_Communication_PortMapFun, NULL, 0},
	{"SubNetwork",			SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_SubNetwork_SEFun, NULL, 0}
};

SX_ELEMENT SubNetworkElements[] = 
{
	/* NOTE: "bitRate" and "Text" elements ignored.	*/
	{"ConnectedAP",      	SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_ConnectedAP_SEFun, NULL, 0}
};

SX_ELEMENT ConnectedAPElements[] = 
{
	/* DEBUG: add "Address". */
	{"Address",	    		SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_Address_SEFun, NULL, 0},
	{"GSE",	      			SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_GSE_SEFun, NULL, 0},
	{"SMV",	      			SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_SMV_SEFun, NULL, 0},
	/* add PhyConn and connections 获取端口信息*/
	{"PhysConn",	    	SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_S1_ConnPortFun, NULL, 0},
};

SX_ELEMENT AddressElements[] = 
{
	{"P",      				SX_ELF_CSTARTEND|SX_ELF_OPTRPT, 	_Address_P_SEFun, NULL, 0}
};

SX_ELEMENT GSEElements[] = 
{
	{"Address",      		SX_ELF_CSTARTEND|SX_ELF_OPT, 	_GSE_Address_SEFun, NULL, 0},
	{"MinTime",      		SX_ELF_CEND|SX_ELF_OPT, 		_GSE_MinTime_SEFun, NULL, 0},
	{"MaxTime",      		SX_ELF_CEND|SX_ELF_OPT, 	 	_GSE_MaxTime_SEFun, NULL, 0}
};

SX_ELEMENT GSEAddressElements[] = 
{
	{"P",      				SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_GSE_Address_P_SEFun, NULL, 0}
};

SX_ELEMENT SMVElements[] = 
{
	{"Address",      		SX_ELF_CSTARTEND|SX_ELF_OPT, 	_SMV_Address_SEFun, NULL, 0}
};

SX_ELEMENT SMVAddressElements[] = 
{
	{"P",      				SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_SMV_Address_P_SEFun, NULL, 0}
};

SX_ELEMENT ConnectionElements[] = 
{
	{"P",      				SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_Connection_P_Port_SEFun, NULL, 0}
};
/************************************************************************/
/* Tables for mapping "IED" elements.					*/
/************************************************************************/
SX_ELEMENT IEDElements[] = 
{
	//private 包括IED的CRC校验码,以及南网signal map
	{"Private",        		SX_ELF_CSTARTEND|SX_ELF_OPTRPT, _AccessPoint_PrivateFun, NULL, 0},
	//regardless <Services>,直接从<AccessPoint >开始
	{"AccessPoint",    		SX_ELF_CSTARTEND|SX_ELF_RPT, 	_AccessPoint_SEFun, NULL, 0}
};
//===========================AccessPoint Start==========================================//
SX_ELEMENT AccessPointElements[] = 
{
	{"Server",      		SX_ELF_CSTARTEND|SX_ELF_OPT, 	_Server_SEFun, NULL, 0}
};

SX_ELEMENT ServerElements[] = 
{
	{"LDevice",      		SX_ELF_CSTARTEND|SX_ELF_RPT,	_LDevice_SEFun, NULL, 0}
};

SX_ELEMENT LDeviceElements[] = 
{
	{"LN0",      			SX_ELF_CSTARTEND,				_LN_SEFun, NULL, 0},
	{"LN",      			SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_LN_SEFun, NULL, 0}
};

SX_ELEMENT LN0Elements[] = 
{
	{"DataSet",  			SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_DataSet_SEFun, NULL, 0},
	{"ReportControl",		SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_ReportControl_SEFun, NULL, 0},
	{"DOI",					SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_DOI_SEFun, NULL, 0}, 
	{"SampledValueControl",	SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_SampledValueControl_SEFun, NULL, 0},
	{"LogControl",			SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_LogControl_SEFun, NULL, 0},
	{"SettingControl",		SX_ELF_CSTART|SX_ELF_OPTRPT,	_SettingControl_SFun, NULL, 0},
	{"GSEControl",			SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_GSEControl_SEFun, NULL, 0}
};

SX_ELEMENT GSEControlElements[] =
{
	{"IEDName",  			SX_ELF_CEND|SX_ELF_OPTRPT,		_IEDName_EFun, NULL, 0}
};

//modify by tangkai LN节点下,不会出现DOI以外的内容,暂时屏蔽处理
SX_ELEMENT LNElements[] = 
{
	// {"DataSet",  			SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_DataSet_SEFun, NULL, 0},
	// {"ReportControl",		SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_ReportControl_SEFun, NULL, 0},
	{"DOI",					SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_DOI_SEFun, NULL, 0}, 
	// {"SampledValueControl",	SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_SampledValueControl_SEFun, NULL, 0},
	// {"LogControl",			SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_LogControl_SEFun, NULL, 0}
};

SX_ELEMENT DataSetElements[] = 
{
	{"FCDA",  				SX_ELF_CSTART|SX_ELF_RPT,		_FCDA_SFun, NULL, 0}
};

SX_ELEMENT ReportControlElements[] = 
{
	{"TrgOps",  			SX_ELF_CSTART|SX_ELF_OPT,		_TrgOps_SFun, 	NULL, 0},
	{"OptFields",			SX_ELF_CSTART,					_OptFlds_SFun, 	NULL, 0},
	{"RptEnabled",			SX_ELF_CSTART|SX_ELF_OPT,		_RptEnabled_SFun, NULL, 0}
};

SX_ELEMENT LogControlElements[] = 
{
	{"TrgOps",  			SX_ELF_CSTART|SX_ELF_OPT,		_TrgOps_SFun, NULL, 0}
};

SX_ELEMENT SampledValueControlElements[] = 
{
	{"IEDName",  			SX_ELF_CEND|SX_ELF_OPTRPT,		_IEDName2_EFun, NULL, 0},
	{"SmvOpts",  			SX_ELF_CSTART,					_SmvOpts_SFun, NULL, 0}
};

SX_ELEMENT DOIElements[] = 
{
	{"SDI",  				SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_SDI_SEFun, NULL, 0},
	{"DAI",  				SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_DOI_DAI_SEFun, NULL, 0}
};

/* SDI can be nested under itself indefinitely */
/* SDI->SDI DAI or SDI->SDI->SDI->DAI*/
/* new add SDI_SDI_SEFun 增加顶层SDI下处理SDI和DAI的*/
SX_ELEMENT SDIElements[] = 
{
	{"SDI",  				SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_SDI_SDI_SEFun, NULL, 0},
	{"DAI",  				SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_SDI_DAI_SEFun, NULL, 0}
};

SX_ELEMENT DAIElements[] = 
{
	{"Val",  				SX_ELF_CSTARTEND|SX_ELF_OPT,	_DAI_Val_SEFun, NULL, 0}
};

/************************************************************************/
/* Tables for mapping "DataTypeTemplates" elements.			*/
/************************************************************************/
SX_ELEMENT DataTypeTemplatesElements[] = 
{
	{"LNodeType",  			SX_ELF_CSTARTEND|SX_ELF_RPT,	_LNodeType_SEFun, NULL, 0},
	{"DOType",  			SX_ELF_CSTARTEND|SX_ELF_RPT,	_DOType_SEFun, NULL, 0},
	{"DAType",  			SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_DAType_SEFun, NULL, 0},
	{"EnumType", 			SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_EnumType_SEFun, NULL, 0}
};

SX_ELEMENT LNodeTypeElements[] = 
{
	{"DO",  				SX_ELF_CSTART|SX_ELF_RPT,	_DO_SFun, NULL, 0}
};

SX_ELEMENT DOTypeElements[] = 
{
	{"DA",  				SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_DA_SEFun, NULL, 0},
	{"SDO",  				SX_ELF_CSTART|SX_ELF_OPTRPT,	_SDO_SFun, NULL, 0}
};

SX_ELEMENT DAElements[] = 
{
	{"Val",  				SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_DA_Val_SEFun, NULL, 0}
};

SX_ELEMENT DATypeElements[] = 
{
	{"BDA",  				SX_ELF_CSTARTEND|SX_ELF_RPT,	_BDA_SEFun, NULL, 0}
};

SX_ELEMENT BDAElements[] = 
{
	{"Val",  				SX_ELF_CSTARTEND|SX_ELF_OPTRPT,	_BDA_Val_SEFun, NULL, 0}
};

SX_ELEMENT EnumTypeElements[] = 
{
	{"EnumVal",  			SX_ELF_CSTARTEND|SX_ELF_RPT,	_EnumVal_SEFun, NULL, 0}
};

/************************************************************************/
/*			convert_mac					*/
/* Converts MAC string read from SCL file (like 01-02-03-04-05-06)	*/
/* to 6 byte hex MAC address.						*/
/* modify becaues macaddress shows up in string
/************************************************************************/
#define MAX_MAC_STRING_LEN	30
static ST_RET convert_mac (ST_CHAR *dst, ST_CHAR *src)
{
	ST_RET retcode;
	ST_CHAR tmpbuf [MAX_MAC_STRING_LEN+1];	//ascii string
	ST_CHAR *tmpptr;
	ST_UINT dstlen;
	/* Input string may include extra blanks, so allow for fairly long string.*/
	if (strlen (src) > MAX_MAC_STRING_LEN)
		retcode = SD_FAILURE;
	else
	{
		/* Just replace each '-' with ' '. Then use ascii_to_hex_str to convert.*/
		tmpptr = dst;
		/* Copy until NULL terminator but ignore '-' and spaces.	*/
		for ( ;  *src;  src++)
		{
			if (*src != '-' && (!isspace(*src)))
				*tmpptr++ = *src;
		}
		*tmpptr = '\0';	/* NULL terminate temp buffer	*/

		//modify 直接使用char
		// retcode = ascii_to_hex_str (dst, &dstlen, 6, tmpbuf);
		// if (retcode == SD_SUCCESS && dstlen != 6)
		// 	retcode = SD_FAILURE; 
	}
	return (SD_SUCCESS);
}

/************************************************************************/
/*			log_notfound_attr				*/
/************************************************************************/

// static ST_VOID log_notfound_attr (ST_CHAR *attrName)
#define log_notfound_attr(attrName) do { \
	SLOG_ERROR ("SCL PARSE: Required attribute '%s' not found.", attrName); \
} while(0)

/************************************************************************/
/*			log_attr_str					*/
/************************************************************************/

// static ST_VOID log_attr_str (ST_CHAR *name, ST_CHAR *value) 
#define log_attr_str(name, value) do { \
	SLOG_DEBUG ("SCL PARSE: Found attribute '%s', value is '%s'", name, value); \
} while(0)
/************************************************************************/
/*			log_attr_int					*/
/************************************************************************/

// static ST_VOID log_attr_int (ST_CHAR *name, ST_INT value)
#define log_attr_int(name, value) do { \
	SLOG_DEBUG ("SCL PARSE: Found attribute '%s', value is '%d'", name, value); \
} while(0)

/************************************************************************/
/*			log_attr_uint					*/
/************************************************************************/

#define log_attr_uint(name, value) do { \
	SLOG_DEBUG("SCL PARSE: Found attribute '%s', value is '%u'", name, value); \
} while(0)

/************************************************************************/
/*			log_returned_failure				*/
/************************************************************************/

// static ST_VOID log_returned_failure (ST_CHAR *funcName, ST_LONG ret)
#define log_returned_failure(funcName, ret) do { \
	SLOG_WARN("SCL PARSE: %s returned failure, err=%ld", funcName, ret); \
} while(0)

/************************************************************************/
/*			scl_stop_parsing				*/
/************************************************************************/

static ST_VOID scl_stop_parsing (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *offender,
								 ST_RET errCode)
{
	sxDecCtrl->errCode = errCode;
	sxDecCtrl->termFlag = SD_TRUE;
	if (errCode == SX_REQUIRED_TAG_NOT_FOUND)
	{
		log_notfound_attr (offender);
	}
	else /* SX_USER_ERROR */
	{
		log_returned_failure (offender, errCode);
	}
}

/************************************************************************/
/*			scl_get_attr_ptr				*/
/* Get a pointer to an attr string stored in SX_DEC_CTRL.		*/
/* If attr found, SD_SUCCESS returned & "*value" points to string.	*/
/* NOTE: The pointer returned in "*value" might not be valid later	*/
/*	when parsing continues.						*/
/************************************************************************/

static ST_RET scl_get_attr_ptr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name,
								ST_CHAR **value, ST_BOOLEAN required)
{
	ST_RET ret;

	ret = sx_get_attr_ptr (sxDecCtrl, value, name);
	if (ret != SD_SUCCESS)
	{
		*value = NULL;	/* make sure ptr is NULL on error (better than garbage)*/
		if (required)
			scl_stop_parsing (sxDecCtrl, name, SX_REQUIRED_TAG_NOT_FOUND);
	}
	else
		log_attr_str (name, *value);

	return (ret);
}

/************************************************************************/
/*			scl_get_attr_copy				*/
/* Get a pointer to an attr string stored in SX_DEC_CTRL.		*/
/* If strlen <= maxValueLen, copy string, else return error.		*/
/************************************************************************/

static ST_RET scl_get_attr_copy (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name,
								 ST_CHAR *value, ST_UINT maxValueLen, ST_BOOLEAN required)
{
	ST_RET ret;
	ST_CHAR *pValue;

	ret = scl_get_attr_ptr (sxDecCtrl, name, &pValue, required);
	if (ret == SD_SUCCESS)
	{
		if (strlen (pValue) <= maxValueLen)
			strcpy (value, pValue);	/* copy string to caller's buffer	*/
		else
		{
			SLOG_ERROR ("Attribute Value='%s' exceeds max len '%d' for attribute '%s'", pValue, maxValueLen, name);
			scl_stop_parsing (sxDecCtrl, name, SX_USER_ERROR);
			ret = SD_FAILURE;
		}
	}

	return (ret);
}

/************************************************************************/
/*			scl_get_int_attr				*/
/************************************************************************/

static ST_RET scl_get_int_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name,
								ST_INT *value, ST_BOOLEAN required)
{
	ST_RET ret;

	ret = sx_get_int_attr (sxDecCtrl, name, value);
	if (ret != SD_SUCCESS)
	{
		if (required)
			scl_stop_parsing (sxDecCtrl, name, SX_REQUIRED_TAG_NOT_FOUND);
	}
	else
		log_attr_int (name, *value);

	return (ret);
}

/************************************************************************/
/*			scl_get_uint_attr				*/
/************************************************************************/

static ST_RET scl_get_uint_attr (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *name,
								 ST_UINT *value, ST_BOOLEAN required)
{
	ST_RET ret;

	ret = sx_get_uint_attr (sxDecCtrl, name, value);
	if (ret != SD_SUCCESS)
	{
		if (required)
			scl_stop_parsing (sxDecCtrl, name, SX_REQUIRED_TAG_NOT_FOUND);
	}
	else
		log_attr_uint (name, *value);

	return (ret);
}

/************************************************************************/
/*			 construct_flattened				*/
/* Construct a flattened variable name from DOI, SDI, DAI names.	*/
/************************************************************************/
ST_RET construct_flattened (ST_CHAR *flattened, size_t maxlen, ST_CHAR *name, ST_CHAR *ix)
{
	size_t ixlen;
	ST_RET retCode;

	/* Calc space needed for optional [ix]	*/
	if (ix)
		ixlen = strlen(ix)+2;	/* string plus brackets	*/
	else
		ixlen = 0;
	/* Make sure there is room for [ix] and "$"	*/
	if (strlen (flattened) + strlen(name) + ixlen + 1 <= maxlen)
	{
		/* If flattened is now empty, just copy name, else add "$" then name.*/
		if (strlen(flattened) == 0)
			strcpy (flattened, name);
		else
		{
			strcat (flattened, "$");
			strcat (flattened, name);
		}
		if (ix)
		{      /* Add 'ix' to flattened if necessary.	*/
			strcat (flattened, "[");
			strcat (flattened, ix);
			strcat (flattened, "]");
		}
		retCode = SD_SUCCESS;
	}
	else
	{	/* flattened is big, so this error should never occur with normal SCL.*/
		SLOG_WARN ("ERROR: not enough space to add name '%s' to flattened name '%s'", name, flattened);
		retCode = SD_FAILURE;
	}
	return (retCode);
}

/************************************************************************/
/*			scl_parse					*/
/************************************************************************/

ST_RET scl_parse (const ST_CHAR *xmlFileName, const ST_CHAR *iedName, 
				  const ST_CHAR *accessPointName, SCL_INFO *sclInfo)
{
	ST_RET ret;
	SCL_DEC_CTRL sclDecCtrl;	/* start with clean struct.	*/
	memset(&sclDecCtrl,0,sizeof(SCL_DEC_CTRL));

	memset (sclInfo, 0, sizeof (SCL_INFO));    /* CRITICAL: start with clean struct*/

	/* Save iedName in sclInfo for later use.	*/
//	strncpy_safe (sclInfo->iedName, iedName, MAX_IDENT_LEN);

//	sclDecCtrl.iedName = iedName;
//	sclDecCtrl.accessPointName = accessPointName;
	// strncpy(sclDecCtrl.accessPointName, accessPointName, sizeof(sclDecCtrl.accessPointName));
//	sclDecCtrl.accessPointFound = SD_FALSE;
	sclDecCtrl.sclInfo = sclInfo;

	ret = sx_parseExx_mt (xmlFileName, 
		sizeof (sclStartElements)/sizeof(SX_ELEMENT), sclStartElements,
		&sclDecCtrl, _scl_unknown_el_start, _scl_unknown_el_end);
	if (ret == SD_SUCCESS) {
		//遍历完scl之后,再做其他工作
		scl_get_dataSet_sAddr(sclInfo);
	}
	SLOG_DEBUG ("sx_parseExx_mt finished result %d", ret);
	/* NOTE: sx_parseEx_mt doesn't log error if file open fails, so log here*/
	/* It may not log some other errors, so log any other error here too.	*/
	if (ret == SX_FILE_NOT_FOUND)
	{SLOG_ERROR ("XML File (%s) Open Error",xmlFileName);}
	else if (ret != SD_SUCCESS)
	{SLOG_ERROR ("Error 0x%X parsing SCL file (%s)", ret, xmlFileName);}

	return (ret);
}

/************************************************************************/
/************************************************************************/
/*			SCL_SEFun					*/
/************************************************************************/

static ST_VOID _SCL_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	//SCL 层无事可做,下一层压栈
	if (sxDecCtrl->reason == SX_ELEMENT_START)
		sx_push (sxDecCtrl, sizeof(SCLElements)/sizeof(SX_ELEMENT), SCLElements, SD_FALSE);
	else
	{
		while (sxDecCtrl->itemStackLevel > 0)
			sx_pop (sxDecCtrl);
	}
}

/************************************************************************/
/*		_Substation_CrcFun	 处理scl 框架的Private like as: 			*/
/*		<Private type="portMap">2-A;1-A</Private>
		<Private type="portMap">2-B;1-B</Private>
/************************************************************************/
static ST_VOID _Substation_CrcFun(SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *typeStrValue;
	ST_RET ret;
	SCL_INFO *sclInfo;
	ST_CHAR *strOut;
	ST_INT strLen;
	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	sclInfo = sclDecCtrl->sclInfo;
	if (sxDecCtrl->reason == SX_ELEMENT_END)
	{
		if (scl_get_attr_ptr (sxDecCtrl, "type", &typeStrValue, SCL_ATTR_OPTIONAL) == SD_SUCCESS)
		{
			if (strcmp (typeStrValue, "Substation virtual terminal conection CRC") != 0)
			{
				SLOG_ERROR ("Private attribute type='%s' not allowed. Assuming type desc='Substation virtual terminal conection CRC' ", typeStrValue);
			}
			
			if (sx_get_string_ptr (sxDecCtrl, &strOut, &strLen) == SD_SUCCESS)
			{
				if (!strLen) return;
				strncpy_safe (sclInfo->Header.sclCrc, strOut, MAX_CRC32_LEN);
				SLOG_DEBUG("virtual terminal conection CRC %s strlen: %d", sclInfo->Header.sclCrc, strLen);
			}
		}
	}
	return;
}
/************************************************************************/
/*			_Header_SFun					*/
/************************************************************************/

static ST_VOID _Header_SFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *nameStructure;
	ST_RET ret;
	SCL_INFO *sclInfo;
	SLOG_DEBUG("Paser _Header_SFun");
	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	sclInfo = sclDecCtrl->sclInfo;

	/* Get required attributes	*/
	//获取ID
	ret = scl_get_attr_copy (sxDecCtrl, "id", sclInfo->Header.id,
		(sizeof(sclInfo->Header.id)-1), SCL_ATTR_REQUIRED);
	if (ret != SD_SUCCESS)
	{	
		SLOG_ERROR("Get scl_get_attr_copy ID error.");
		return;	/* At least one required attr not found. Stop now.	*/
	}
	/* Handle optional "nameStructure".	*/
	if (scl_get_attr_ptr (sxDecCtrl, "nameStructure", &nameStructure, SCL_ATTR_REQUIRED) == SD_SUCCESS)
	{	
		if (strcasecmp (nameStructure, "IEDName") != 0)
		{
			SLOG_ERROR ("Header attribute nameStructure='%s' not allowed. Assuming nameStructure='IEDName' (i.e. 'Product Naming')", nameStructure);
		}
	}
	/* Always assume nameStructure="IEDName" (i.e. "Product Naming")	*/
	sclInfo->Header.nameStructure = SCL_NAMESTRUCTURE_IEDNAME;
}

/************************************************************************/
/*			_Communication_SEFun				*/
/************************************************************************/
static ST_VOID _Communication_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sx_push (sxDecCtrl, sizeof(CommunicationElements)/sizeof(SX_ELEMENT), 
			CommunicationElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}

/************************************************************************/
/*_Communication_PortMapFun	 处理Communication 自定义PortMap			*/
/************************************************************************/
static ST_VOID _Communication_PortMapFun(SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *typeStrValue;
	ST_RET ret;
	SCL_INFO *sclInfo;
	SLOG_DEBUG("Paser _Communication_PortMapFun");
	ST_CHAR *strOut;
	ST_INT strLen;
	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	sclInfo = sclDecCtrl->sclInfo;

	if (scl_get_attr_ptr (sxDecCtrl, "type", &typeStrValue, SCL_ATTR_OPTIONAL) == SD_SUCCESS)
	{
		if (strcmpi (typeStrValue, "portMap") != 0)
		{
			SLOG_ERROR ("Private attribute type='%s' not allowed. Assuming type desc='portMap' or 'portmap'..etc ", typeStrValue);
		}
		
		if (sx_get_string_ptr (sxDecCtrl, &strOut, &strLen) == SD_SUCCESS)
		{
			if (!strLen) return;
			// strncpy_safe (sclInfo->Header.sclCrc, strOut, MAX_CRC32_LEN);
			SLOG_DEBUG("portMap: %s", strOut);
		}			
	}	
	

	return;
}
/************************************************************************/
/*			_SubNetwork_SEFun				*/
/************************************************************************/
static ST_VOID _SubNetwork_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_RET ret;
	ST_CHAR *desc;
	//parse <SubNetwork name="SubNetwork_Stationbus" type="xxxxxx">
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SCL_SUBNET *scl_subnet;
		scl_subnet = scl_subnet_create (sclDecCtrl->sclInfo);
		if (scl_subnet == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_subnet_create", SX_USER_ERROR);
			return;
		}
		/* Get required attributes.	*/
		ret = scl_get_attr_copy (sxDecCtrl, "name", scl_subnet->name, (sizeof(scl_subnet->name)-1), SCL_ATTR_REQUIRED);
		if (ret)
		{
			scl_stop_parsing (sxDecCtrl, "SubNetwork", SX_USER_ERROR);
			return;
		}
		/* Get optional attributes.	*/
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			scl_subnet->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
		ret = scl_get_attr_copy (sxDecCtrl, "type", scl_subnet->type, (sizeof(scl_subnet->type)-1), SCL_ATTR_OPTIONAL);

		sx_push (sxDecCtrl, sizeof(SubNetworkElements)/sizeof(SX_ELEMENT), 
			SubNetworkElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}
/************************************************************************/
/*			_ConnectedAP_SEFun				*/
/************************************************************************/
static ST_VOID _ConnectedAP_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_RET ret;
	ST_CHAR *desc;
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SCL_CAP *scl_cap;
		scl_cap = scl_cap_add (sclDecCtrl->sclInfo);
		if (scl_cap == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_cap_add", SX_USER_ERROR);
			return;
		}
		/* Get required attributes	*/
		ret = scl_get_attr_copy (sxDecCtrl, "iedName", scl_cap->iedName, (sizeof(scl_cap->iedName)-1), SCL_ATTR_REQUIRED);
		ret |= scl_get_attr_copy (sxDecCtrl, "apName", scl_cap->apName, (sizeof(scl_cap->apName)-1), SCL_ATTR_REQUIRED);
		if (ret)
		{
			scl_stop_parsing (sxDecCtrl, "ConnectedAP", SX_USER_ERROR);
			return;
		}
		/* Get optional attributes.	*/
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			scl_cap->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/

		sx_push (sxDecCtrl, sizeof(ConnectedAPElements)/sizeof(SX_ELEMENT), 
			ConnectedAPElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}
/************************************************************************/
/*			_GSE_SEFun					*/
/************************************************************************/
static ST_VOID _GSE_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_RET ret;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		/* NOTE: save ptr in sclDecCtrl->scl_gse to use later in parsing.	*/
		sclDecCtrl->scl_gse = scl_gse_add (sclDecCtrl->sclInfo);	//使用链表存放
		if (sclDecCtrl->scl_gse == NULL)
		{
			SLOG_ERROR ("STOP scl_gse_add because scl_gse is NULL");
			scl_stop_parsing (sxDecCtrl, "scl_gse_add", SX_USER_ERROR);
			return;
		}
		ret = scl_get_attr_copy (sxDecCtrl, "ldInst", sclDecCtrl->scl_gse->ldInst, (sizeof(sclDecCtrl->scl_gse->ldInst)-1), SCL_ATTR_REQUIRED);    
		ret |= scl_get_attr_copy (sxDecCtrl, "cbName", sclDecCtrl->scl_gse->cbName, (sizeof(sclDecCtrl->scl_gse->cbName)-1), SCL_ATTR_REQUIRED); 

		SLOG_DEBUG ("GSE Name: %s", sclDecCtrl->sclInfo->subnetHead->capHead->apName);
		SLOG_DEBUG("ldInst: %s", sclDecCtrl->scl_gse->ldInst);
		SLOG_DEBUG("cbName: %s", sclDecCtrl->scl_gse->cbName);
		if (ret)	//error
		{
			SLOG_ERROR ("GSE scl_get_attr_copy error.");
			scl_stop_parsing (sxDecCtrl, "GSE", SX_USER_ERROR);
			return;
		}
		else
			sx_push (sxDecCtrl, sizeof(GSEElements)/sizeof(SX_ELEMENT), 
			GSEElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}
/************************************************************************/
/*			_GSE_Address_SEFun				*/
/************************************************************************/
static ST_VOID _GSE_Address_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sx_push (sxDecCtrl, sizeof(GSEAddressElements)/sizeof(SX_ELEMENT), 
			GSEAddressElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}

/************************************************************************/
/*			_GSE_MinTime_SEFun				*/
/************************************************************************/
static ST_VOID _GSE_MinTime_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_RET ret;
	ST_CHAR *strOut;
	ST_INT strLen;

	ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
	if (ret == SD_SUCCESS)
		sclDecCtrl->scl_gse->minTime = atoi(strOut);

}

/************************************************************************/
/*			_GSE_MaxTime_SEFun				*/
/************************************************************************/
static ST_VOID _GSE_MaxTime_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_RET ret;
	ST_CHAR *strOut;
	ST_INT strLen;

	ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
	if (ret == SD_SUCCESS)
		sclDecCtrl->scl_gse->maxTime = atoi(strOut);

}
/************************************************************************/
/*			_GSE_Address_P_SEFun				*/
/************************************************************************/
static ST_VOID _GSE_Address_P_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_CHAR *str;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	ST_CHAR *strOut;
	ST_INT strLen;

	if (sxDecCtrl->reason == SX_ELEMENT_END)
	{
		ret = scl_get_attr_ptr (sxDecCtrl, "type", &str, required);
		if (!strcasecmp(str,"MAC-Address"))
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS)
			{
				if (convert_mac (sclDecCtrl->scl_gse->MAC,strOut))
				{
					SLOG_ERROR ("Illegal MAC Address '%s'", strOut);
					scl_stop_parsing (sxDecCtrl, "_GSE_Address_P_SEFun", SX_USER_ERROR);
				}
			}
		}
		else if (!strcasecmp(str,"APPID"))
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS)
				sclDecCtrl->scl_gse->APPID = HexStrToInt(strOut);
		}
		else if (!strcasecmp(str,"VLAN-PRIORITY"))
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS)
				sclDecCtrl->scl_gse->VLANPRI = atoi(strOut);
		}
		else if (!strcasecmp(str,"VLAN-ID"))
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS)
				sclDecCtrl->scl_gse->VLANID = atoi(strOut);
		}
	}
}
/************************************************************************/
/*			_S1_ConnPortFun					*/
/************************************************************************/
static ST_VOID _S1_ConnPortFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_RET ret;
	ST_CHAR *typeStr;
	ST_INT strLen;
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		/* NOTE: save ptr in sclDecCtrl->scl_smv to use later in parsing.	*/
		// sclDecCtrl->scl_smv = scl_smv_add (sclDecCtrl->sclInfo);
		// if (sclDecCtrl->scl_smv == NULL)
		// {
		// 	scl_stop_parsing (sxDecCtrl, "scl_smv_add", SX_USER_ERROR);
		// 	return;
		// }
		// ret = scl_get_attr_copy (sxDecCtrl, "ldInst", sclDecCtrl->scl_smv->ldInst, (sizeof(sclDecCtrl->scl_smv->ldInst)-1), SCL_ATTR_REQUIRED);    
		// ret |= scl_get_attr_copy (sxDecCtrl, "cbName", sclDecCtrl->scl_smv->cbName, (sizeof(sclDecCtrl->scl_smv->cbName)-1), SCL_ATTR_REQUIRED);    
		// if (ret)
		// {
		// 	scl_stop_parsing (sxDecCtrl, "SMV", SX_USER_ERROR);
		// 	return;
		// }
		// SLOG_DEBUG("Parse _S1_ConnPortFun");
		ret = scl_get_attr_ptr (sxDecCtrl, "type", &typeStr, SCL_ATTR_REQUIRED);
		if (ret != SD_SUCCESS) {
			scl_stop_parsing (sxDecCtrl, "type", SX_USER_ERROR);
			return;
		}

		SLOG_DEBUG("typeStr %s", typeStr);
		sclDecCtrl->scl_ports = scl_port_add(sclDecCtrl->sclInfo);
		if ( NULL ==  sclDecCtrl->scl_ports)
		{
			scl_stop_parsing (sxDecCtrl, "scl_port", SX_USER_ERROR);
			return;
		}
		//寻找port
		sx_push (sxDecCtrl, sizeof(ConnectionElements)/sizeof(SX_ELEMENT), 
			ConnectionElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}


/**
 * @description: 
 * @param {*}
 * @return {*}
 */
static ST_VOID _SMV_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_RET ret;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		/* NOTE: save ptr in sclDecCtrl->scl_smv to use later in parsing.	*/
		sclDecCtrl->scl_smv = scl_smv_add (sclDecCtrl->sclInfo);
		if (sclDecCtrl->scl_smv == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_smv_add", SX_USER_ERROR);
			return;
		}
		ret = scl_get_attr_copy (sxDecCtrl, "ldInst", sclDecCtrl->scl_smv->ldInst, (sizeof(sclDecCtrl->scl_smv->ldInst)-1), SCL_ATTR_REQUIRED);    
		ret |= scl_get_attr_copy (sxDecCtrl, "cbName", sclDecCtrl->scl_smv->cbName, (sizeof(sclDecCtrl->scl_smv->cbName)-1), SCL_ATTR_REQUIRED);    
		if (ret)
		{
			scl_stop_parsing (sxDecCtrl, "SMV", SX_USER_ERROR);
			return;
		}
		else
			sx_push (sxDecCtrl, sizeof(SMVElements)/sizeof(SX_ELEMENT), 
			SMVElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}
/************************************************************************/
/*			_SMV_Address_SEFun				*/
/************************************************************************/
static ST_VOID _SMV_Address_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sx_push (sxDecCtrl, sizeof(SMVAddressElements)/sizeof(SX_ELEMENT), 
			SMVAddressElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}
/************************************************************************/
/*			_SMV_Address_P_SEFun				*/
/************************************************************************/
static ST_VOID _SMV_Address_P_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_CHAR *str;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	ST_CHAR *strOut;
	ST_INT strLen;

	if (sxDecCtrl->reason == SX_ELEMENT_END)
	{
		ret = scl_get_attr_ptr (sxDecCtrl, "type", &str, required);
		if (!strcmpi(str,"MAC-Address"))
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS)
			{
				if (convert_mac (sclDecCtrl->scl_smv->MAC,strOut))
				{
					SLOG_ERROR ("Illegal MAC Address '%s'", strOut);
					scl_stop_parsing (sxDecCtrl, "_SMV_Address_P_SEFun", SX_USER_ERROR);
				}
			}
		}
		else if (!strcmpi(str,"APPID"))
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS)
				sclDecCtrl->scl_smv->APPID = HexStrToInt(strOut);
		}
		else if (!strcmpi(str,"VLAN-PRIORITY"))
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS)
				sclDecCtrl->scl_smv->VLANPRI = atoi(strOut);
		}
		else if (!strcmpi(str,"VLAN-ID"))
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS)
				sclDecCtrl->scl_smv->VLANID = atoi(strOut);
		}
	}
}

/************************************************************************/
/*			_Connection_P_Port_SEFun				*/
/************************************************************************/
static ST_VOID _Connection_P_Port_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_CHAR *str;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	ST_CHAR *strOut;
	ST_INT strLen;
	SCL_PORT *scl_port = sclDecCtrl->scl_ports;

	if (!scl_port) {
		SLOG_ERROR ("Null scl_port address");
		return;
	}
	if (sxDecCtrl->reason == SX_ELEMENT_END)
	{
		if (scl_get_attr_ptr(sxDecCtrl, "type", &str, required) != SD_SUCCESS)
		{
			return;
		}	
	
		if (strcasecmp(str,"Port") == 0)
		{
			/* Get required attributes	*/		
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);	
			if (ret == SD_SUCCESS) {
				strncpy_safe (scl_port->portValue, strOut, MAX_IDENT_LEN);
				// SLOG_DEBUG ("GSE ports :%s", scl_port->portValue);
			}
		}	
		else if (strcasecmp(str,"Plug") == 0)
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS) {
				strncpy_safe (scl_port->portPlug, strOut, MAX_IDENT_LEN);
			}

		}
		else if (strcasecmp(str,"Type") == 0)
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS) {
				strncpy_safe (scl_port->portType, strOut, MAX_IDENT_LEN);
			}

		}
		else if (strcasecmp(str,"Cable") == 0)
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS) {
				strncpy_safe (scl_port->portCable, strOut, MAX_IDENT_LEN);
			}
		}		
	}
}
/************************************************************************/
/*			IED_SEFun					*/
/************************************************************************/

static ST_VOID _IED_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *str;	/* ptr set by scl_get_attr_ptr	*/
	ST_CHAR *configver;
	ST_CHAR *desc, *iedType, *manufacturer;

	ST_RET ret;
	// ST_BOOLEAN required = SD_FALSE;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		//创建IED链表
		SCL_IED *scl_lIED = (SCL_IED *) chk_calloc (1, sizeof (SCL_IED));
		list_add_last (&sclDecCtrl->sclInfo->lIEDHead, scl_lIED);

		/* start required attributes */
		// required = SD_TRUE;

		ret = scl_get_attr_ptr (sxDecCtrl, "name", &str, SCL_ATTR_REQUIRED);
		ret |= scl_get_attr_ptr (sxDecCtrl, "configVersion", &configver, SCL_ATTR_OPTIONAL);
		if (ret != SD_SUCCESS)
		{
			return;
		}

		strncpy_safe (scl_lIED->iedName, str, MAX_IDENT_LEN);
		strncpy_safe (scl_lIED->configVersion, configver, VERSION_LEN);
//        strncpy_safe (sclDecCtrl->sclInfo->iedName, str, MAX_IDENT_LEN);
		strncpy_safe (sclDecCtrl->iedName, str, MAX_IDENT_LEN);
		//SLOG_DEBUG ("SCL PARSE: IED 'name' match found: %s", str);
		//sclDecCtrl->iedNameMatched = SD_TRUE;

		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_REQUIRED);
		if (ret == SD_SUCCESS)
			scl_lIED->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
		ret = scl_get_attr_ptr (sxDecCtrl, "type", &iedType, SCL_ATTR_REQUIRED);
		if (ret == SD_SUCCESS)
			scl_lIED->iedType = chk_strdup (iedType);	/* Alloc & copy desc string	*/			
		ret = scl_get_attr_ptr (sxDecCtrl, "manufacturer", &manufacturer, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			scl_lIED->manufacturer = chk_strdup (manufacturer);	/* Alloc & copy desc string	*/	

		sx_push (sxDecCtrl, sizeof(IEDElements)/sizeof(SX_ELEMENT), IEDElements, SD_FALSE);
	
		/* end required attributes */
	}
	else
	{
	//	if (sclDecCtrl->iedNameMatched == SD_TRUE)
	//	{
	//		sclDecCtrl->iedNameMatched = SD_FALSE;
			sx_pop (sxDecCtrl);
	//	}
	}
}
/**
 * @description: 处理IED下层private信息函数
 * @param {*} 
 */
static ST_VOID _AccessPoint_PrivateFun  (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_CHAR *str;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	ST_CHAR *strOut;
	ST_INT strLen;

	SLOG_DEBUG("Parse _AccessPoint_PrivateFun");

	if (sxDecCtrl->reason == SX_ELEMENT_END)
	{
		ret = scl_get_attr_ptr (sxDecCtrl, "type", &str, required);
		if (ret != SD_SUCCESS)
		{
			return;
		}		
		if (strcasecmp(str, "IED virtual terminal conection CRC") == 0)
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS)
			{
				
				if (sclDecCtrl->sclInfo->lIEDHead != NULL) {
					strncpy_safe (sclDecCtrl->sclInfo->lIEDHead->iedDeviceCrc, strOut, MAX_CRC32_LEN);
					SLOG_DEBUG ("IED virtual terminal conection CRC: %s", sclDecCtrl->sclInfo->lIEDHead->iedDeviceCrc);
				} else {
					SLOG_ERROR ("lIEDHead pointer is NUll");
				}
			}
		}			
	}

}
/************************************************************************/
/*			_AccessPoint_SEFun				*/
/************************************************************************/

static ST_VOID _AccessPoint_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *name, *desc;	/* ptr set by scl_get_attr_ptr	*/
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		/* start required attributes */
		
		SCL_ACCESSPOINT *scl_acpoint = scl_accesspoint_add(sclDecCtrl->sclInfo);
		if (scl_acpoint == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_accesspoint_add", SX_USER_ERROR);
			return;
		}
		required = SD_TRUE;
		ret = scl_get_attr_ptr (sxDecCtrl, "name", &name, SCL_ATTR_REQUIRED);
		if (strlen(name) > FIX_STR_LEN_3)
		{
			SLOG_ERROR("AccessPoint name= %s is illegal.", name);
			scl_stop_parsing (sxDecCtrl, "name", SX_REQUIRED_TAG_NOT_FOUND);
			return;
		}		
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
	
		scl_acpoint->desc =  chk_strdup (desc); /* Alloc & copy desc string	*/
		strncpy_safe(scl_acpoint->name, name, FIX_STR_LEN_3);
		strncpy_safe(sclDecCtrl->accessPointName, name, FIX_STR_LEN_3);
		//原函数传参没有用处,目前需要考虑多个Accesspoint
		// SLOG_DEBUG ("SCL PARSE: AccessPoint name: %s", sclDecCtrl->accessPointName);
		SLOG_DEBUG ("SCL PARSE: AccessPoint name: %s desc: %s", scl_acpoint->name, scl_acpoint->desc);
		sclDecCtrl->accessPointFound = SD_TRUE;	/*NOTE: only get here if IED also found*/
		sclDecCtrl->accessPointMatched = SD_TRUE;
		sx_push (sxDecCtrl, sizeof(AccessPointElements)/sizeof(SX_ELEMENT), AccessPointElements, SD_FALSE);

		/* end required attributes */
	}
	else
	{
		//if (sclDecCtrl->accessPointMatched == SD_TRUE)
		//{
		//	sclDecCtrl->accessPointMatched = SD_FALSE;
			sx_pop (sxDecCtrl);
		//}
	}
}

/************************************************************************/
/*			_Server_SEFun					*/
/************************************************************************/

static ST_VOID _Server_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{	
		sx_push (sxDecCtrl, sizeof(ServerElements)/sizeof(SX_ELEMENT), ServerElements, SD_FALSE);
	}
	else
		sx_pop (sxDecCtrl);
}

/************************************************************************/
/*			_LDevice_SEFun					*/
/************************************************************************/
static ST_VOID _LDevice_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	SCL_INFO *scl_info;
	SCL_LD* scl_ld;
	ST_CHAR* desc;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
		//添加LDevice 链表
		scl_ld = sclDecCtrl->scl_ld = scl_ld_add (sclDecCtrl->sclInfo);
		if (scl_ld == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_ld_add", SX_USER_ERROR);
			return;
		}

		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, required);
		if (ret == SD_SUCCESS)
			scl_ld->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
		/* end optional attributes */

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_copy (sxDecCtrl, "inst", scl_ld->inst, (sizeof(scl_ld->inst)-1), required);
		if (ret != SD_SUCCESS)
			return;	/* At least one required attr not found. Stop now.	*/
		/* end required attributes */
		SLOG_DEBUG ("LD device: %s, desc: %s", scl_ld->inst, scl_ld->desc);
		sx_push (sxDecCtrl, sizeof(LDeviceElements)/sizeof(SX_ELEMENT), LDeviceElements, SD_FALSE);
	}
	else
	{		/* reason == SX_ELEMENT_END	*/
		sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
		scl_info = sclDecCtrl->sclInfo;
		scl_ld = sclDecCtrl->scl_ld;
		/* Construct MMS Domain name from scl info.	*/
		/* ASSUME nameStructure="IEDName" (domain name = IED name + LDevice inst)*/
		/* nameStructure="FuncName" is OBSOLETE.				*/
		if (strlen(sclDecCtrl->iedName) + strlen(scl_ld->inst) <= MAX_IDENT_LEN)
		{
			strcpy (scl_ld->domName, sclDecCtrl->iedName);	/* construct domain name*/
			strcat (scl_ld->domName, scl_ld->inst);
			strcpy(scl_ld->apName,sclDecCtrl->accessPointName);
		}
		else
		{
			SLOG_ERROR ("Cannot create LD: constructed domain name too long");
			scl_stop_parsing (sxDecCtrl, "_LDevice_SEFun", SX_USER_ERROR);
		}
		sx_pop (sxDecCtrl);	 
	}
}

/************************************************************************/
/*			_LN_SEFun					*/
/************************************************************************/

static ST_VOID _LN_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	SCL_LN *scl_ln;
	ST_CHAR *desc;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
		//创建LN链表
		scl_ln = sclDecCtrl->scl_ln = scl_ln_add (sclDecCtrl->sclInfo);
		if (scl_ln == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_ln_add", SX_USER_ERROR);
			return;
		}

		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, required);
		if (ret == SD_SUCCESS)
			scl_ln->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
		ret = scl_get_attr_copy (sxDecCtrl, "prefix", scl_ln->prefix, (sizeof(scl_ln->prefix)-1), required);
		/* end optional attributes */

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_copy (sxDecCtrl, "lnType", scl_ln->lnType, (sizeof(scl_ln->lnType)-1), required);
		ret |= scl_get_attr_copy (sxDecCtrl, "inst", scl_ln->inst, (sizeof(scl_ln->inst)-1), required);
		ret |= scl_get_attr_copy (sxDecCtrl, "lnClass", scl_ln->lnClass, (sizeof(scl_ln->lnClass)-1), required);

		if (ret != SD_SUCCESS)
			return;

		//LN0的inClass必须为LLN0
		if (stricmp(sxDecCtrl->sxDecElInfo.tag, "LN0") == 0 && 
			stricmp(scl_ln->lnClass, "LLN0") != 0)
		{
			sxDecCtrl->errCode = SX_USER_ERROR;
			sxDecCtrl->termFlag = SD_TRUE;
			SLOG_ERROR ("SCL PARSE: Attribute 'lnClass' of element 'LN0' has a value other then 'LLN0' (schema violation).");
			return;
		}
		/* end required attributes */

		if (stricmp(sxDecCtrl->sxDecElInfo.tag, "LN0") == 0)
			sx_push (sxDecCtrl, sizeof(LN0Elements)/sizeof(SX_ELEMENT), LN0Elements, SD_FALSE);
		else
			sx_push (sxDecCtrl, sizeof(LNElements)/sizeof(SX_ELEMENT), LNElements, SD_FALSE);
	}
	else
	{		/* reason == SX_ELEMENT_END	*/
		sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
		scl_ln = sclDecCtrl->scl_ln;
		/* Construct MMS Variable name from scl info.	*/
		if (strlen (scl_ln->lnClass) != 4)
		{
			SLOG_ERROR ("Illegal lnClass='%s'. Must be exactly 4 char",
				scl_ln->lnClass);
			scl_stop_parsing (sxDecCtrl, "_LN_SEFun", SX_USER_ERROR);
		}
		else if (strlen (scl_ln->prefix) + strlen (scl_ln->inst) > 11)
		{
			/* NOTE: standard only allows max=7, but we want to be more forgiving.*/
			SLOG_ERROR ("Illegal definition for lnClass='%s': prefix (%s) plus inst (%s) > 11 char.",
				scl_ln->lnClass, scl_ln->prefix, scl_ln->inst);
			scl_stop_parsing (sxDecCtrl, "_LN_SEFun", SX_USER_ERROR);
		}
		else
		{
			strcpy (scl_ln->varName, scl_ln->prefix);
			strcat (scl_ln->varName, scl_ln->lnClass);
			strcat (scl_ln->varName, scl_ln->inst);
		}
		sx_pop (sxDecCtrl);
	}
}

/************************************************************************/
/*			_DataSet_SEFun					*/
/************************************************************************/

static ST_VOID _DataSet_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	SCL_DATASET *scl_dataset;
	ST_CHAR *desc;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
		SCL_INFO *scl_info = sclDecCtrl->sclInfo;
		
		//检查DataSet 出现位置的合法性,不应该出现在LN节点中
		// SCL_LN *curLn = list_find_last(scl_info->accessPointHead->ldHead->lnHead);
		// if (strcasecmp(curLn->lnClass, "LLN0") != 0) {
		// 	SLOG_WARN ("DataSet must not occur LN Element return");
		// 	// sx_pop (sxDecCtrl);
		// 	return;
		// }
		
		scl_dataset = scl_dataset_add (scl_info);
		if (scl_dataset == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_dataset_add", SX_USER_ERROR);
			return;
		}
		
		//dataSet 一般包括name和desc
		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, required);
		if (ret == SD_SUCCESS)
			scl_dataset->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
		/* end optional attributes */

		/* start required attributes */
		required = SD_TRUE;
		//scl_dataset 为最新地址
		ret = scl_get_attr_copy (sxDecCtrl, "name", scl_dataset->name, (sizeof(scl_dataset->name)-1), required);
		if (ret != SD_SUCCESS)
			return;
		/* end required attributes */
		sx_push (sxDecCtrl, sizeof(DataSetElements)/sizeof(SX_ELEMENT), DataSetElements, SD_FALSE);
	}
	else
		sx_pop (sxDecCtrl);
}

/************************************************************************/
/*			_FCDA_SFun					*/
/************************************************************************/

static ST_VOID _FCDA_SFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	SCL_INFO *scl_info;
	SCL_FCDA *scl_fcda;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	scl_info = sclDecCtrl->sclInfo;
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{		
		scl_fcda = scl_fcda_add (scl_info);
		if (scl_fcda == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_fcda_add", SX_USER_ERROR);
			return;
		}

		/* start optional attributes */
		ret = scl_get_attr_copy (sxDecCtrl, "ldInst",  scl_fcda->ldInst,  (sizeof(scl_fcda->ldInst)-1), required);
		ret = scl_get_attr_copy (sxDecCtrl, "prefix",  scl_fcda->prefix,  (sizeof(scl_fcda->prefix)-1), required);
		ret = scl_get_attr_copy (sxDecCtrl, "lnInst",  scl_fcda->lnInst,  (sizeof(scl_fcda->lnInst)-1), required);
		ret = scl_get_attr_copy (sxDecCtrl, "lnClass", scl_fcda->lnClass, (sizeof(scl_fcda->lnClass)-1), required);
		ret = scl_get_attr_copy (sxDecCtrl, "doName",  scl_fcda->doName,  (sizeof(scl_fcda->doName)-1), required);
		ret = scl_get_attr_copy (sxDecCtrl, "daName",  scl_fcda->daName,  (sizeof(scl_fcda->daName)-1), required);
		/* end optional attributes */

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_copy (sxDecCtrl, "fc", scl_fcda->fc, (sizeof(scl_fcda->fc)-1), required);
		if (ret != SD_SUCCESS)
			return;
		/* end required attributes */

		/* Construct domain name from SCL info	*/
		/* ASSUME nameStructure="IEDName" (domain name = IED name + LDevice inst)*/
		//nameStructure="FuncName" is OBSOLETE.
		SCL_IED* ied = list_find_last((DBL_LNK *)scl_info->lIEDHead);
		if (!ied) {
			SLOG_ERROR ("Null IED");
			return ;
		}
		//TEMPLATECTRL/GOOUTGGIO2.Ind5.stVal.[ST] or 
		//TEMPLATEPROT/WarnGGIO1.Alm19[ST]
		if (strlen(scl_fcda->ldInst) + strlen(scl_fcda->prefix)+ strlen(scl_fcda->lnClass) + 
		   strlen(scl_fcda->lnInst) + strlen(scl_fcda->doName) + strlen(scl_fcda->daName) <= MAX_IDENT_LEN)
		{
			if  (!ied->iedName) {
				SLOG_ERROR ("There is no IedName");
				strcpy (scl_fcda->domName, scl_fcda->ldInst);
			} 
			else
			{
				strcpy (scl_fcda->domName, ied->iedName);
				// strcpy (scl_fcda->domName, scl_info->lIEDHead->iedName);
			}
			
			strcat (scl_fcda->domName, scl_fcda->ldInst);
			strcat (scl_fcda->domName, "/");
			if (strlen(scl_fcda->prefix)) 
			{
				strcat (scl_fcda->domName, scl_fcda->prefix);
				// strcat (scl_fcda->domName, "/");
			}
			strcat (scl_fcda->domName, scl_fcda->lnClass);
			// strcat (scl_fcda->domName, "/");
			strcat (scl_fcda->domName, scl_fcda->lnInst);
			strcat (scl_fcda->domName, ".");
			strcat (scl_fcda->domName, scl_fcda->doName);
			if (strlen(scl_fcda->daName)) 
			{
				strcat (scl_fcda->domName, ".");
				strcat (scl_fcda->domName, scl_fcda->daName);
			}
			strcat (scl_fcda->domName, "[");
			strcat (scl_fcda->domName, scl_fcda->fc);
			strcat (scl_fcda->domName, "]");
			

		}
		else
		{
			SLOG_ERROR ("Cannot add FCDA: constructed domain name too long");
			scl_stop_parsing (sxDecCtrl, "_FCDA_SFun", SX_USER_ERROR);
		}

	}
}

/************************************************************************/
/*			_ReportControl_SEFun				*/
/************************************************************************/

static ST_VOID _ReportControl_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *str;	/* ptr set by scl_get_attr_ptr	*/
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	ST_CHAR *desc;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	SCL_INFO * scl_info = sclDecCtrl->sclInfo;
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		//检查ReportControl 出现位置的合法性,不应该出现在LN节点中
		// SCL_LN *curLn = list_find_last(scl_info->accessPointHead->ldHead->lnHead);
		// SLOG_WARN("curLn->lnClass is %s", curLn->lnClass);
		// if (strcasecmp(curLn->lnClass, "LLN0") != 0) {
		// 	SLOG_WARN ("ReportControl must not occur LN Element return");
		// 	// sx_pop (sxDecCtrl);
		// 	return;
		// }		
		/* Alloc struct, save ptr in sclDecCtrl, & set local ptr to it.	*/
		SCL_RCB *scl_rcb = sclDecCtrl->scl_rcb = scl_rcb_add (scl_info);
		if (scl_rcb == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_rcb_add", SX_USER_ERROR);
			return;
		}

		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, required);
		if (ret == SD_SUCCESS)
			scl_rcb->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
		ret = scl_get_attr_copy (sxDecCtrl, "datSet", scl_rcb->datSet, (sizeof(scl_rcb->datSet)-1), required);
		ret = scl_get_uint_attr (sxDecCtrl, "intgPd", &scl_rcb->intgPd, required);
		ret = scl_get_uint_attr (sxDecCtrl, "bufTime", &scl_rcb->bufTime, required);
		ret = scl_get_attr_ptr (sxDecCtrl, "buffered", &str, required);

		scl_rcb->buffered = SD_FALSE;  /* default */
		if (ret == SD_SUCCESS)
		{
			if (stricmp(str, "true") == 0)
				scl_rcb->buffered = SD_TRUE;
		}
		/* NOTE: we only accept default value of indexed="true".	*/
		ret = scl_get_attr_ptr (sxDecCtrl, "indexed", &str, required);
		if (ret == SD_SUCCESS && stricmp(str, "false") == 0)
		{
			SLOG_ERROR ("ReportControl attribute indexed='false' not supported");
			scl_stop_parsing (sxDecCtrl, "scl_rcb_add", SX_USER_ERROR);
			return;
		}
		/* end optional attributes */

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_copy (sxDecCtrl, "name", scl_rcb->name, (sizeof(scl_rcb->name)-1), required);
		ret |= scl_get_attr_copy (sxDecCtrl, "rptID", scl_rcb->rptID, (sizeof(scl_rcb->rptID)-1), required);
		ret |= scl_get_uint_attr (sxDecCtrl, "confRev", &scl_rcb->confRev, required);
		if (ret != SD_SUCCESS)
			return;
		/* end required attributes */

		sx_push (sxDecCtrl, sizeof(ReportControlElements)/sizeof(SX_ELEMENT), ReportControlElements, SD_FALSE);
	}
	else /* reason = SX_ELEMENT_END */
	{
		/* CRITICAL: Copy TrgOps to scl_rcb.	*/
		sclDecCtrl->scl_rcb->TrgOps[0] = sclDecCtrl->TrgOps[0];
		/* If "RptEnabled max" not configured, set default value*/
		if (sclDecCtrl->scl_rcb->maxClient == 0)
			sclDecCtrl->scl_rcb->maxClient = 1;	/* default	*/
		/* NOTE: scl_rcb is all filled in now	*/
		sx_pop (sxDecCtrl);
	}
}

/************************************************************************/
/*			_LogControl_SEFun				*/
/************************************************************************/

static ST_VOID _LogControl_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *str;	/* ptr set by scl_get_attr_ptr	*/
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	ST_CHAR *desc;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		/* Alloc struct, save ptr in sclDecCtrl, & set local ptr to it.	*/
		SCL_LCB *scl_lcb = sclDecCtrl->scl_lcb = scl_lcb_add (sclDecCtrl->sclInfo);
		if (scl_lcb == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_lcb_add", SX_USER_ERROR);
			return;
		}

		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, required);
		if (ret == SD_SUCCESS)
			scl_lcb->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
		ret = scl_get_uint_attr (sxDecCtrl, "intgPd", &scl_lcb->intgPd, required);
		ret = scl_get_attr_copy (sxDecCtrl, "datSet", scl_lcb->datSet, (sizeof(scl_lcb->datSet)-1), required);
		ret = scl_get_attr_ptr (sxDecCtrl, "logEna", &str, required);
		scl_lcb->logEna = SD_FALSE;  /* default */
		if (ret == SD_SUCCESS)
		{
			if (stricmp(str, "true") == 0)
				scl_lcb->logEna = SD_TRUE;
		}

		ret = scl_get_attr_ptr (sxDecCtrl, "reasonCode", &str, required);
		scl_lcb->reasonCode = SD_FALSE;  /* default */
		if (ret == SD_SUCCESS)
		{
			if (stricmp(str, "true") == 0)
				scl_lcb->reasonCode = SD_TRUE;
		}
		/* end optional attributes */

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_copy (sxDecCtrl, "name", scl_lcb->name, (sizeof(scl_lcb->name)-1), required);
		ret |= scl_get_attr_copy (sxDecCtrl, "logName", scl_lcb->logName, (sizeof(scl_lcb->logName)-1), required);
		if (ret != SD_SUCCESS)
			return;
		/* end required attributes */

		sx_push (sxDecCtrl, sizeof(LogControlElements)/sizeof(SX_ELEMENT), LogControlElements, SD_FALSE);
	}
	else /* reason = SX_ELEMENT_END */
	{
		/* CRITICAL: Copy TrgOps to scl_lcb.	*/
		sclDecCtrl->scl_lcb->TrgOps[0] = sclDecCtrl->TrgOps[0];
		/* NOTE: scl_lcb is all filled in now	*/
		sx_pop (sxDecCtrl);
	}
}

/************************************************************************/
/*			_GSEControl_SEFun				*/
/************************************************************************/

static ST_VOID _GSEControl_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	ST_CHAR *type;	/* ptr set by scl_get_attr_ptr	*/
	SCL_GCB *scl_gcb;
	ST_CHAR *desc;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		scl_gcb = scl_gcb_add (sclDecCtrl->sclInfo);
		if (scl_gcb == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_gcb_add", SX_USER_ERROR);
			return;
		}

		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, required);
		if (ret == SD_SUCCESS)
			scl_gcb->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
		ret = scl_get_uint_attr (sxDecCtrl, "confRev", &scl_gcb->confRev, required);
		ret = scl_get_attr_copy (sxDecCtrl, "datSet", scl_gcb->datSet, (sizeof(scl_gcb->datSet)-1), required);
		ret = scl_get_attr_ptr (sxDecCtrl, "type", &type, required);
		if (ret == SD_SUCCESS && strcmp(type, "GSSE") == 0)
			scl_gcb->isGoose = SD_FALSE;
		else
			scl_gcb->isGoose = SD_TRUE;
		/* end optional attributes */

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_copy  (sxDecCtrl, "name",   scl_gcb->name,   (sizeof(scl_gcb->name)-1), required);
		ret |= scl_get_attr_copy (sxDecCtrl, "appID",  scl_gcb->appID,  (sizeof(scl_gcb->appID)-1), required);
		if (ret != SD_SUCCESS)
			return;
		/* end required attributes */

		sx_push (sxDecCtrl, sizeof(GSEControlElements)/sizeof(SX_ELEMENT), GSEControlElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}

static ST_VOID _IEDName_EFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_RET ret;
	ST_INT strLen;
	ST_CHAR *Val;
	ret = sx_get_string_ptr (sxDecCtrl, &Val, &strLen);
	if (ret==SD_SUCCESS)
	{
		SCL_IEDNAME *iedNm=(SCL_IEDNAME *) chk_calloc (1, sizeof (SCL_IEDNAME));
		
		SCL_IED *ied = list_get_last (sclDecCtrl->sclInfo->lIEDHead);
		if (!ied) {
			chk_free(iedNm);
			return;
		}
		SCL_LD *ld = ied->accessPointHead->ldHead;
		if (ld && ld->lnHead && ld->lnHead->gcbHead)
		{
			list_add_first (&ld->lnHead->gcbHead->iedNHead, iedNm);
		}
		else
		{
			chk_free(iedNm);
			return;
		}
		
		if (strlen (Val) < sizeof(iedNm->IEDName))
			strcpy (iedNm->IEDName, Val);	/* copy string to caller's buffer	*/
		else
		{
			scl_stop_parsing (sxDecCtrl, "_IEDName_EFun", SX_USER_ERROR);
			ret = SD_FAILURE;
		}

		sclDecCtrl->scl_dai->Val = chk_strdup (Val);	/* alloc & store Val*/
	}
	else
		scl_stop_parsing (sxDecCtrl, "_IEDName_EFun", SX_USER_ERROR);
};


static ST_VOID _IEDName2_EFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_RET ret;
	ST_INT strLen;
	ST_CHAR *Val;
	ret = sx_get_string_ptr (sxDecCtrl, &Val, &strLen);
	if (ret==SD_SUCCESS)
	{
		SCL_IED *ied = sclDecCtrl->sclInfo->lIEDHead;
		if (!ied) {
			return;
		}
		
		SCL_LD *ld = ied->accessPointHead->ldHead;
		SCL_IEDNAME *iedNm = NULL;
		if (ld && ld->lnHead && ld->lnHead->svcbHead)
		{
			iedNm = (SCL_IEDNAME *) chk_calloc (1, sizeof (SCL_IEDNAME));
			list_add_first (&ld->lnHead->svcbHead->iedNHead, iedNm);
		}
		else
		{
			return;
		}

		if (strlen (Val) < sizeof(iedNm->IEDName))
			strcpy (iedNm->IEDName, Val);	/* copy string to caller's buffer	*/
		else
		{
			scl_stop_parsing (sxDecCtrl, "_IEDName_EFun", SX_USER_ERROR);
			ret = SD_FAILURE;
		}

		sclDecCtrl->scl_dai->Val = chk_strdup (Val);	/* alloc & store Val*/
	}
	else
		scl_stop_parsing (sxDecCtrl, "_IEDName_EFun", SX_USER_ERROR);
};
/************************************************************************/
/*			_SettingControl_SFun				*/
/************************************************************************/

static ST_VOID _SettingControl_SFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_BOOLEAN required;
	SCL_SGCB *scl_sgcb;
	ST_CHAR *desc;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	scl_sgcb = scl_sgcb_add (sclDecCtrl->sclInfo);
	if (scl_sgcb == NULL)
	{
		scl_stop_parsing (sxDecCtrl, "scl_sgcb_add", SX_USER_ERROR);
		return;
	}

	/* start optional attributes */
	required = SD_FALSE;
	ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, required);
	if (ret == SD_SUCCESS)
		scl_sgcb->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
	ret = scl_get_uint_attr (sxDecCtrl, "actSG", &scl_sgcb->actSG, required);
	if (ret)
		scl_sgcb->actSG = 1;  /* default value */
	/* end optional attributes */

	/* start required attributes */
	required = SD_TRUE;
	ret = scl_get_uint_attr (sxDecCtrl, "numOfSGs", &scl_sgcb->numOfSGs, required);
	/* end required attributes */
}

/************************************************************************/
/*			_TrgOps_SFun					*/
/* Save all TrgOps bits in sclDecCtrl->TrgOps.				*/
/************************************************************************/

static ST_VOID _TrgOps_SFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *str;	/* ptr set by scl_get_attr_ptr	*/
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	sclDecCtrl->TrgOps[0] = 0;	/* Start with all bits=0	*/

	/* start optional attributes */
	ret = scl_get_attr_ptr (sxDecCtrl, "dchg", &str, required);
	if (ret == SD_SUCCESS)
	{
		if (stricmp(str, "true") == 0)
		{
			BSTR_BIT_SET_ON(sclDecCtrl->TrgOps, TRGOPS_BITNUM_DATA_CHANGE);	//1
		}

	}

	ret = scl_get_attr_ptr (sxDecCtrl, "qchg", &str, required);
	if (ret == SD_SUCCESS)
	{
		if (stricmp(str, "true") == 0)
		{
			BSTR_BIT_SET_ON(sclDecCtrl->TrgOps, TRGOPS_BITNUM_QUALITY_CHANGE);	//2
		}
	}

	ret = scl_get_attr_ptr (sxDecCtrl, "dupd", &str, required);
	if (ret == SD_SUCCESS)
	{
		if (stricmp(str, "true") == 0)
		{
			BSTR_BIT_SET_ON(sclDecCtrl->TrgOps, TRGOPS_BITNUM_DATA_UPDATE);	//3
		}
	}

	ret = scl_get_attr_ptr (sxDecCtrl, "period", &str, required);		
	if (ret == SD_SUCCESS)
	{
		if (stricmp(str, "true") == 0)
		{
			BSTR_BIT_SET_ON(sclDecCtrl->TrgOps, TRGOPS_BITNUM_INTEGRITY);	//4
		}
	}
	/* end optional attributes */
}

/************************************************************************/
/*			_OptFlds_SFun					*/
/* Save all OptFlds bits in sclDecCtrl->scl_rcb->OptFlds.		*/
/************************************************************************/

static ST_VOID _OptFlds_SFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *str;	/* ptr set by scl_get_attr_ptr	*/
	ST_RET ret;
	SCL_RCB *scl_rcb;
	ST_BOOLEAN required = SD_FALSE;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	scl_rcb = sclDecCtrl->scl_rcb;

	/* start optional attributes */
	ret = scl_get_attr_ptr (sxDecCtrl, "seqNum", &str, required);
	if (ret == SD_SUCCESS)
	{
		if (stricmp(str, "true") == 0)
		{
			BSTR_BIT_SET_ON(scl_rcb->OptFlds, OPTFLD_BITNUM_SQNUM);
		}
	}

	ret = scl_get_attr_ptr (sxDecCtrl, "timeStamp", &str, required);
	if (ret == SD_SUCCESS)
	{
		if (stricmp(str, "true") == 0)
		{
			BSTR_BIT_SET_ON(scl_rcb->OptFlds, OPTFLD_BITNUM_TIMESTAMP);
		}
	}

	ret = scl_get_attr_ptr (sxDecCtrl, "dataSet", &str, required);
	if (ret == SD_SUCCESS)
	{
		if (stricmp(str, "true") == 0)
		{
			BSTR_BIT_SET_ON(scl_rcb->OptFlds, OPTFLD_BITNUM_DATSETNAME);
		}
	}

	ret = scl_get_attr_ptr (sxDecCtrl, "reasonCode", &str, required);
	if (ret == SD_SUCCESS)
	{
		if (stricmp(str, "true") == 0)
		{
			BSTR_BIT_SET_ON(scl_rcb->OptFlds, OPTFLD_BITNUM_REASON);
		}
	}

	ret = scl_get_attr_ptr (sxDecCtrl, "dataRef", &str, required);
	if (ret == SD_SUCCESS)
	{
		if (stricmp(str, "true") == 0)
		{
			BSTR_BIT_SET_ON(scl_rcb->OptFlds, OPTFLD_BITNUM_DATAREF);
		}
	}

	ret = scl_get_attr_ptr (sxDecCtrl, "bufOvfl", &str, required);
	if (ret == SD_SUCCESS)
	{
		if (stricmp(str, "true") == 0)
		{
			BSTR_BIT_SET_ON(scl_rcb->OptFlds, OPTFLD_BITNUM_BUFOVFL);
		}
	}

	ret = scl_get_attr_ptr (sxDecCtrl, "entryID", &str, required);
	if (ret == SD_SUCCESS)
	{
		if (stricmp(str, "true") == 0)
		{
			BSTR_BIT_SET_ON(scl_rcb->OptFlds, OPTFLD_BITNUM_ENTRYID);
		}
	}

	ret = scl_get_attr_ptr (sxDecCtrl, "configRef", &str, required);
	if (ret == SD_SUCCESS)
	{
		if (stricmp(str, "true") == 0)
		{
			BSTR_BIT_SET_ON(scl_rcb->OptFlds, OPTFLD_BITNUM_CONFREV);
		}
	}
	/* end optional attributes */
}

/************************************************************************/
/*			_RptEnabled_SFun				*/
/* Save RptEnabled info in sclDecCtrl->scl_rcb.				*/
/************************************************************************/
static ST_VOID _RptEnabled_SFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_RET ret;
	SCL_RCB *scl_rcb;

	assert (sxDecCtrl->reason == SX_ELEMENT_START);
	scl_rcb = sclDecCtrl->scl_rcb;

	/* start optional attributes */
	ret = scl_get_uint_attr (sxDecCtrl, "max", &scl_rcb->maxClient, SCL_ATTR_OPTIONAL);
	/* If configured, check for legal value.	*/
	if (ret == SD_SUCCESS)
	{
		if (scl_rcb->maxClient <= 0 || scl_rcb->maxClient > 99)
		{
			SLOG_ERROR ("RptEnabled max=%d is not valid. Must be value between 1 and 99", scl_rcb->maxClient);
			scl_stop_parsing (sxDecCtrl, "RptEnabled", SX_USER_ERROR);
			scl_rcb->maxClient = 1;	/* set to default just in case user ignores error*/
		}
	}
}

/************************************************************************/
/*			_DOI_SEFun					*/
/************************************************************************/

static ST_VOID _DOI_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *ix;
	ST_CHAR *desc;
	ST_RET ret;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	SCL_ACCESSPOINT *apHead = sclDecCtrl->sclInfo->lIEDHead->accessPointHead;
	if (apHead == NULL)
	{
		scl_stop_parsing (sxDecCtrl, "_DOI_SEFun", SX_INTERNAL_NULL_POINTER);
		return;		
	}

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{

		SCL_DOI *doi; 
		if ( (doi = scl_doi_add(sclDecCtrl->sclInfo)) == NULL) {
			scl_stop_parsing (sxDecCtrl, "_DOI_SEFun", SX_INTERNAL_NULL_POINTER);
			return;
		}
		/* start required attributes */
		ret = scl_get_attr_copy (sxDecCtrl, "name", doi->name, (sizeof(doi->name)-1), SCL_ATTR_REQUIRED);
		if (ret)
		{
			scl_stop_parsing (sxDecCtrl, "_DOI_SEFun", SX_USER_ERROR);
			return;
		}
		/* end required attributes */

		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "ix", &ix, SCL_ATTR_OPTIONAL);
		if (ret !=SD_SUCCESS)
			ix = NULL;
		else
			doi->idx=atoi(ix);

		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			doi->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
		/* end optional attributes */

		/* Start creation of flattened name */
		sclDecCtrl->flattened[0] = '\0';	/* CRITICAL: start with empty flatname*/
		
		sx_push (sxDecCtrl, sizeof(DOIElements)/sizeof(SX_ELEMENT), DOIElements, SD_FALSE);    
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}

/************************************************************************/
/*			_SDI_SEFun					*/
/************************************************************************/

static ST_VOID _SDI_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *ix;
	ST_CHAR *name;
	ST_CHAR *desc;
	ST_RET ret;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SCL_SDI *sdi =scl_sdi_add(sclDecCtrl->sclInfo);
		if (sdi == NULL) {
			scl_stop_parsing  (sxDecCtrl, "_SDI_SEFun", SX_INTERNAL_NULL_POINTER);
			return;
		}
		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			sdi->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/

		ret = scl_get_attr_ptr (sxDecCtrl, "ix", &ix, SCL_ATTR_OPTIONAL);
		if (ret)
			ix = NULL;


		/* end optional attributes */

		/* start required attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "name", &name, SCL_ATTR_REQUIRED);
		if (ret != SD_SUCCESS)
		{
			return;
		}
		sdi->name = chk_strdup (name);	/* Alloc & copy desc string	*/
		/* end required attributes */

		/* Continue creation of flattened name 地址嵌套*/
		if (construct_flattened (sclDecCtrl->flattened, sizeof(sclDecCtrl->flattened), name, NULL)
			!= SD_SUCCESS)
		{	/* error already logged.	*/
			scl_stop_parsing (sxDecCtrl, "_SDI_SEFun", SX_USER_ERROR);
			return;
		}
		strncpy_safe(sdi->flattened, sclDecCtrl->flattened, MAX_FLAT_LEN);
		SLOG_DEBUG ("_SDI_SEFun flattened : %s", sdi->flattened);
		//continue searching dai elements
		sx_push (sxDecCtrl, sizeof(SDIElements)/sizeof(SX_ELEMENT), SDIElements, SD_FALSE);    
	}
	else /* reason = SX_ELEMENT_END */
	{
		/* Remove the last item from the flattened string */
		ST_CHAR *p = strrchr(sclDecCtrl->flattened, '$');
		if (p != NULL)
			*p = 0;
		else
			sclDecCtrl->flattened[0] = 0; //added by luolinglu
		SLOG_DEBUG ("SCL PARSE: Removed last item from flattened variable: '%s'", sclDecCtrl->flattened);
		sx_pop (sxDecCtrl);
	}
}

/************************************************************************/
/*			_DOI_DAI_SEFun 处理DOI节点下的DAI					*/
/************************************************************************/

static ST_VOID _DOI_DAI_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *ix;
	ST_CHAR *name;
	ST_CHAR *desc;
	ST_RET ret;
	ST_CHAR *p;
	ST_BOOLEAN required = SD_FALSE;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SX_ELEMENT_TBL_CTRL *itemTblCtrl;
		itemTblCtrl = &sxDecCtrl->items[sxDecCtrl->itemStackLevel];
			
		SCL_DAI *scl_dai;
		if ((scl_dai = sclDecCtrl->scl_dai = scl_dai_add (sclDecCtrl->sclInfo)) == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "_DOI_DAI_SEFun", SX_INTERNAL_NULL_POINTER);
			return;
		}
		
		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			scl_dai->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/

		ret = scl_get_attr_ptr (sxDecCtrl, "ix", &ix, SD_FALSE);
		if (ret)
			ix = NULL;
		ret = scl_get_attr_copy (sxDecCtrl, "sAddr", scl_dai->sAddr, (sizeof(scl_dai->sAddr)-1), required);
		
		if (ret)
			strcpy (scl_dai->valKind, "Set"); /* default */
		/* end optional attributes */

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_ptr (sxDecCtrl, "name", &name, required);
		if (ret != SD_SUCCESS)
		{
			return;
		}
		scl_dai->name = chk_strdup (name);	/* Alloc & copy name string	*/
		/* end required attributes */

		/* Continue creation of flattened name */
		if (construct_flattened (sclDecCtrl->flattened, sizeof(sclDecCtrl->flattened), name, desc)
			!= SD_SUCCESS)
		{	/* error already logged.	*/
			scl_stop_parsing (sxDecCtrl, "_DOI_DAI_SEFun", SX_USER_ERROR);
			return;
		}

		strncpy_safe (scl_dai->flattened, sclDecCtrl->flattened, MAX_FLAT_LEN);
		SLOG_DEBUG(" _DOI_DAI_SEFun name %s flattened %s sAddr %s", name, scl_dai->flattened, scl_dai->sAddr);

		//只有DOI的DAI element需要解析
		sx_push (sxDecCtrl, sizeof(DAIElements)/sizeof(SX_ELEMENT), DAIElements, SD_FALSE);    
		
	}
	else /* reason = SX_ELEMENT_END */
	{
		/* Remove the last item from the flattened string */
		p = strrchr(sclDecCtrl->flattened, '$');
		if (p != NULL)
			*p = 0;
		else
			sclDecCtrl->flattened[0] = '\0'; //added by luolinglu

		SLOG_DEBUG ("SCL PARSE: Removed last item from flattened variable: '%s'", sclDecCtrl->flattened);
		sx_pop (sxDecCtrl);
	}
}
/************************************************************************/
/*			_SDI_SDI_SEFun 处理SDI节点下的SDI					*/
/************************************************************************/
static ST_VOID _SDI_SDI_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *ix;
	ST_CHAR *name;
	ST_CHAR *desc;
	ST_RET ret;
	ST_CHAR *p;
	ST_BOOLEAN required = SD_FALSE;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SCL_SDI *sdi =scl_sdi_sdi_add(sclDecCtrl->sclInfo);
		if (sdi == NULL) {
			scl_stop_parsing  (sxDecCtrl, "_SDI_SDI_SEFun", SX_INTERNAL_NULL_POINTER);
			return;
		}
		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			sdi->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/

		ret = scl_get_attr_ptr (sxDecCtrl, "ix", &ix, SCL_ATTR_OPTIONAL);
		if (ret)
			ix = NULL;

		/* end optional attributes */

		/* start required attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "name", &name, SCL_ATTR_REQUIRED);
		if (ret != SD_SUCCESS)
		{
			return;
		}
		sdi->name = chk_strdup (name);	/* Alloc & copy desc string	*/
		/* end required attributes */

		/* Continue creation of flattened name 地址嵌套*/
		if (construct_flattened (sclDecCtrl->flattened, sizeof(sclDecCtrl->flattened), name, NULL)
			!= SD_SUCCESS)
		{	/* error already logged.	*/
			scl_stop_parsing (sxDecCtrl, "_SDI_SDI_SEFun", SX_USER_ERROR);
			return;
		}
		strncpy_safe(sdi->flattened, sclDecCtrl->flattened, MAX_FLAT_LEN);
		SLOG_DEBUG ("_SDI_SDI_SEFun name: %s flattened : %s", sdi->name, sdi->flattened);
		//continue searching dai elements
		sx_push (sxDecCtrl, sizeof(SDIElements)/sizeof(SX_ELEMENT), SDIElements, SD_FALSE);    
	}
	else /* reason = SX_ELEMENT_END */
	{
		/* Remove the last item from the flattened string */
		ST_CHAR *p = strrchr(sclDecCtrl->flattened, '$');
		if (p != NULL)
			*p = 0;
		else
			sclDecCtrl->flattened[0] = 0; //added by luolinglu
		SLOG_DEBUG ("SCL PARSE: Removed last item from flattened variable: '%s'", sclDecCtrl->flattened);
		sx_pop (sxDecCtrl);
	}
}

/************************************************************************/
/*			_SDI_DAI_SEFun 处理SDI节点下的DAI					*/
/************************************************************************/

static ST_VOID _SDI_DAI_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *ix;
	ST_CHAR *name;
	ST_CHAR *desc;
	ST_RET ret;
	ST_CHAR *p;
	ST_BOOLEAN required = SD_FALSE;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SX_ELEMENT_TBL_CTRL *itemTblCtrl;
		itemTblCtrl = &sxDecCtrl->items[sxDecCtrl->itemStackLevel];
			
		SCL_DAI *scl_dai;
		//添加子SDI
		if ((scl_dai = sclDecCtrl->scl_dai = scl_sdi_dai_add (sclDecCtrl->sclInfo)) == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "_SDI_DAI_SEFun", SX_INTERNAL_NULL_POINTER);
			return;
		}
		
		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			scl_dai->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/

		ret = scl_get_attr_ptr (sxDecCtrl, "ix", &ix, required);
		if (ret)
			ix = NULL;
		ret = scl_get_attr_copy (sxDecCtrl, "sAddr", scl_dai->sAddr, (sizeof(scl_dai->sAddr)-1), required);
		ret = scl_get_attr_copy (sxDecCtrl, "valKind", scl_dai->valKind, (sizeof(scl_dai->valKind)-1), required);
		if (ret)
			strcpy (scl_dai->valKind, "Set"); /* default */
		/* end optional attributes */

		/* start required attributes */
		// required = SD_TRUE;
		ret = scl_get_attr_ptr (sxDecCtrl, "name", &name, SCL_ATTR_REQUIRED);
		if (ret != SD_SUCCESS)
		{
			return;
		}
		scl_dai->name = chk_strdup (name);	/* Alloc & copy desc string	*/
		/* end required attributes */

		/* Continue creation of flattened name */
		if (construct_flattened (sclDecCtrl->flattened, sizeof(sclDecCtrl->flattened), name, NULL)
			!= SD_SUCCESS)
		{	/* error already logged.	*/
			scl_stop_parsing (sxDecCtrl, "_SDI_DAI_SEFun", SX_USER_ERROR);
			return;
		}

		strncpy_safe (scl_dai->flattened, sclDecCtrl->flattened, MAX_FLAT_LEN);
		// SLOG_DEBUG(" _SDI_DAI_SEFun flattened %s sAddr %s", scl_dai->flattened, scl_dai->sAddr);
	
		sx_push (sxDecCtrl, sizeof(DAIElements)/sizeof(SX_ELEMENT), DAIElements, SD_FALSE);    
		
	}
	else /* reason = SX_ELEMENT_END */
	{
		/* Remove the last item from the flattened string */
		p = strrchr(sclDecCtrl->flattened, '$');
		if (p != NULL)
			*p = 0;
		else
			sclDecCtrl->flattened[0] = '\0'; //added by luolinglu

		SLOG_DEBUG ("SCL PARSE: Removed last item from flattened variable: '%s'", sclDecCtrl->flattened);
		sx_pop (sxDecCtrl);
	}
}
/************************************************************************/
/*			_DAI_Val_SEFun					*/
/* Sets "sGroup" and "Val" in sclDecCtrl->scl_dai.			*/
/************************************************************************/

static ST_VOID _DAI_Val_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
		/* start optional attributes (don't care about return)	*/
		scl_get_uint_attr (sxDecCtrl, "sGroup", &sclDecCtrl->scl_dai->sGroup, SCL_ATTR_OPTIONAL);
		/* end optional attributes */
	}
	else /* reason = SX_ELEMENT_END */
	{
		SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
		ST_RET ret;
		ST_INT strLen;
		ST_CHAR *Val;
		ret = sx_get_string_ptr (sxDecCtrl, &Val, &strLen);
		if (ret==SD_SUCCESS)
			sclDecCtrl->scl_dai->Val = chk_strdup (Val);	/* alloc & store Val*/
		else
			scl_stop_parsing (sxDecCtrl, "DAI Val", SX_USER_ERROR);
	}
}

/************************************************************************/
/*			_DataTypeTemplates_SEFun			*/
/************************************************************************/

static ST_VOID _DataTypeTemplates_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
		sx_push (sxDecCtrl, sizeof(DataTypeTemplatesElements)/sizeof(SX_ELEMENT), DataTypeTemplatesElements, SD_FALSE);    
	else
		sx_pop (sxDecCtrl);
}

/************************************************************************/
/*			_LNodeType_SEFun				*/
/************************************************************************/

static ST_VOID _LNodeType_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	SCL_LNTYPE *scl_lntype;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

		scl_lntype = scl_lntype_create (sclDecCtrl->sclInfo);
		if (scl_lntype == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_lntype_create", SX_USER_ERROR);
			return;
		}

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_copy (sxDecCtrl, "id", scl_lntype->id, (sizeof(scl_lntype->id)-1), required);
		ret |= scl_get_attr_copy (sxDecCtrl, "lnClass", scl_lntype->lnClass, (sizeof(scl_lntype->lnClass)-1), required);
		if (ret != SD_SUCCESS)
		{
			return;
		}
		/* end required attributes */

		sx_push (sxDecCtrl, sizeof(LNodeTypeElements)/sizeof(SX_ELEMENT), LNodeTypeElements, SD_FALSE);    
	}
	else
		sx_pop (sxDecCtrl);
}

/************************************************************************/
/*			_DO_SFun					*/
/************************************************************************/

static ST_VOID _DO_SFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	SCL_DO *scl_do;
	ST_CHAR *desc;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	scl_do = scl_lntype_add_do (sclDecCtrl->sclInfo);
	if (scl_do == NULL)
	{
		scl_stop_parsing (sxDecCtrl, "scl_lntype_add_do", SX_USER_ERROR);
		return;
	}
	ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, required);
	if (ret == SD_SUCCESS)
		scl_do->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/

	/* start required attributes */
	required = SD_TRUE;
	ret = scl_get_attr_copy (sxDecCtrl, "name", scl_do->name, (sizeof(scl_do->name)-1), required);
	ret |= scl_get_attr_copy (sxDecCtrl, "type", scl_do->type, (sizeof(scl_do->type)-1), required);
	if (ret != SD_SUCCESS)
	{
		return;
	}
	/* end required attributes */
}

/************************************************************************/
/*			_DOType_SEFun					*/
/************************************************************************/

static ST_VOID _DOType_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	SCL_DOTYPE *scl_dotype;
	ST_CHAR *desc;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

		scl_dotype = scl_dotype_create (sclDecCtrl->sclInfo);
		if (scl_dotype == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_dotype_create", SX_USER_ERROR);
			return;
		}

		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, required);
		if (ret == SD_SUCCESS)
			scl_dotype->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_copy (sxDecCtrl, "id", scl_dotype->id, (sizeof(scl_dotype->id)-1), required);
		ret |= scl_get_attr_copy (sxDecCtrl, "cdc", scl_dotype->cdc, (sizeof(scl_dotype->cdc)-1), required);
		if (ret != SD_SUCCESS)
		{
			return;
		}
		/* end required attributes */

		sx_push (sxDecCtrl, sizeof(DOTypeElements)/sizeof(SX_ELEMENT), DOTypeElements, SD_FALSE);    
	}
	else
		sx_pop (sxDecCtrl);
}

/************************************************************************/
/*			_DA_SEFun					*/
/************************************************************************/

static ST_VOID _DA_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SCL_DEC_CTRL *sclDecCtrl;
		ST_CHAR *str;	/* use for dchg, qchg, dupd	*/
		ST_RET ret;
		ST_BOOLEAN required = SD_FALSE;
		SCL_DA *scl_da;
		ST_CHAR *desc;

		sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

		scl_da = sclDecCtrl->scl_da = scl_dotype_add_da (sclDecCtrl->sclInfo);
		if (scl_da == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_dotype_add_da", SX_USER_ERROR);
			return;
		}

		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, required);
		if (ret == SD_SUCCESS)
			scl_da->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
		ret = scl_get_attr_copy (sxDecCtrl, "sAddr", scl_da->sAddr, (sizeof(scl_da->sAddr)-1), required);
		ret = scl_get_attr_copy (sxDecCtrl, "valKind", scl_da->valKind, (sizeof(scl_da->valKind)-1), required);
		if (ret)
			strcpy (scl_da->valKind, "Set"); /* default */
		ret = scl_get_attr_copy (sxDecCtrl, "type", scl_da->type, (sizeof(scl_da->type)-1), required);
		ret = scl_get_uint_attr (sxDecCtrl, "count", &scl_da->count, required);
		ret = scl_get_attr_ptr (sxDecCtrl, "dchg", &str, required);
		if (ret == SD_SUCCESS  &&  stricmp(str, "true") == 0)
			scl_da->dchg = SD_TRUE;

		ret = scl_get_attr_ptr (sxDecCtrl, "qchg", &str, required);
		if (ret == SD_SUCCESS  &&  stricmp(str, "true") == 0)
			scl_da->qchg = SD_TRUE;

		ret = scl_get_attr_ptr (sxDecCtrl, "dupd", &str, required);
		if (ret == SD_SUCCESS  &&  stricmp(str, "true") == 0)
			scl_da->dupd = SD_TRUE;
		/* end optional attributes */

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_copy (sxDecCtrl, "name", scl_da->name, (sizeof(scl_da->name)-1), required);
		ret |= scl_get_attr_copy (sxDecCtrl, "bType", scl_da->bType, (sizeof(scl_da->bType)-1), required);
		ret |= scl_get_attr_copy (sxDecCtrl, "fc", scl_da->fc, (sizeof(scl_da->fc)-1), required);
		if (ret != SD_SUCCESS)
		{
			return;
		}
		/* end required attributes */
		sx_push (sxDecCtrl, sizeof(DAElements)/sizeof(SX_ELEMENT), DAElements, SD_FALSE);
	}
	else
		sx_pop (sxDecCtrl);
}

/************************************************************************/
/*			_SDO_SFun					*/
/************************************************************************/

static ST_VOID _SDO_SFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	SCL_DO *scl_da;
	ST_CHAR *desc;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	scl_da = scl_dotype_add_sdo (sclDecCtrl->sclInfo);
	if (scl_da == NULL)
	{
		scl_stop_parsing (sxDecCtrl, "scl_dotype_add_sdo", SX_USER_ERROR);
		return;
	}

	/* start optional attributes */
	ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, required);
	if (ret == SD_SUCCESS)
		scl_da->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
	/* end optional attributes */

	/* start required attributes */
	required = SD_TRUE;
	ret = scl_get_attr_copy (sxDecCtrl, "name", scl_da->name, (sizeof(scl_da->name)-1), required);
	ret |= scl_get_attr_copy (sxDecCtrl, "type", scl_da->type, (sizeof(scl_da->type)-1), required);
	if (ret != SD_SUCCESS)
	{
		return;
	}
	/* end required attributes */
}

/************************************************************************/
/*			_DA_Val_SEFun					*/
/************************************************************************/

static ST_VOID _DA_Val_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
		/* start optional attributes (don't care about return)	*/
		scl_get_uint_attr (sxDecCtrl, "sGroup", &sclDecCtrl->scl_da->sGroup, SCL_ATTR_OPTIONAL);
		/* end optional attributes */
	}
	else /* reason = SX_ELEMENT_END */
	{
		SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
		ST_RET ret;
		ST_INT strLen;
		ST_CHAR *Val;
		ret = sx_get_string_ptr (sxDecCtrl, &Val, &strLen);
		if (ret==SD_SUCCESS)
			sclDecCtrl->scl_da->Val = chk_strdup (Val);	/* alloc & store Val*/
		else
			scl_stop_parsing (sxDecCtrl, "DA Val", SX_USER_ERROR);
	}
}

/************************************************************************/
/*			_DAType_SEFun					*/
/************************************************************************/

static ST_VOID _DAType_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	SCL_DATYPE *scl_datype;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

		scl_datype = scl_datype_create (sclDecCtrl->sclInfo);
		if (scl_datype == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_datype_create", SX_USER_ERROR);
			return;
		}

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_copy (sxDecCtrl, "id", scl_datype->id, (sizeof(scl_datype->id)-1), required);
		if (ret != SD_SUCCESS)
		{
			return;
		}
		/* end required attributes */

		sx_push (sxDecCtrl, sizeof(DATypeElements)/sizeof(SX_ELEMENT), DATypeElements, SD_FALSE);    
	}
	else
		sx_pop (sxDecCtrl);
}

/************************************************************************/
/*			_BDA_SEFun					*/
/************************************************************************/

static ST_VOID _BDA_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SCL_DEC_CTRL *sclDecCtrl;
		ST_RET ret;
		ST_BOOLEAN required = SD_FALSE;
		SCL_BDA *scl_bda;
		ST_CHAR *desc;

		sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

		scl_bda = sclDecCtrl->scl_bda = scl_datype_add_bda (sclDecCtrl->sclInfo);
		if (scl_bda == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_datype_add_bda", SX_USER_ERROR);
			return;
		}

		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, required);
		if (ret == SD_SUCCESS)
			scl_bda->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
		ret = scl_get_attr_copy (sxDecCtrl, "sAddr", scl_bda->sAddr, (sizeof(scl_bda->sAddr)-1), required);
		ret = scl_get_attr_copy (sxDecCtrl, "valKind", scl_bda->valKind, (sizeof(scl_bda->valKind)-1), required);
		if (ret)
			strcpy (scl_bda->valKind, "Set"); /* default */
		ret = scl_get_attr_copy (sxDecCtrl, "type", scl_bda->type, (sizeof(scl_bda->type)-1), required);
		ret = scl_get_uint_attr (sxDecCtrl, "count", &scl_bda->count, required);
		/* end optional attributes */

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_copy (sxDecCtrl, "name", scl_bda->name, (sizeof(scl_bda->name)-1), required);
		ret |= scl_get_attr_copy (sxDecCtrl, "bType", scl_bda->bType, (sizeof(scl_bda->bType)-1), required);
		if (ret != SD_SUCCESS)
		{
			return;
		}
		/* end required attributes */
		sx_push (sxDecCtrl, sizeof(BDAElements)/sizeof(SX_ELEMENT), BDAElements, SD_FALSE);
	}
	else
		sx_pop (sxDecCtrl);
}

/************************************************************************/
/*			_BDA_Val_SEFun					*/
/************************************************************************/

static ST_VOID _BDA_Val_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
		/* start optional attributes (don't care about return)	*/
		scl_get_uint_attr (sxDecCtrl, "sGroup", &sclDecCtrl->scl_bda->sGroup, SCL_ATTR_OPTIONAL);
		/* end optional attributes */
	}
	else /* reason = SX_ELEMENT_END */
	{
		SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
		ST_RET ret;
		ST_INT strLen;
		ST_CHAR *Val;
		ret = sx_get_string_ptr (sxDecCtrl, &Val, &strLen);
		if (ret==SD_SUCCESS)
			sclDecCtrl->scl_bda->Val = chk_strdup (Val);	/* alloc & store Val*/
		else
			scl_stop_parsing (sxDecCtrl, "BDA Val", SX_USER_ERROR);
	}
}

/************************************************************************/
/*			_EnumType_SEFun					*/
/************************************************************************/

static ST_VOID _EnumType_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	SCL_ENUMTYPE *scl_enumtype;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

		if ((scl_enumtype = scl_enumtype_create (sclDecCtrl->sclInfo)) == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_enumtype_create", SX_USER_ERROR);
			return;
		}

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_attr_copy (sxDecCtrl, "id", scl_enumtype->id, (sizeof(scl_enumtype->id)-1), required);
		if (ret != SD_SUCCESS)
		{
			return;
		}
		/* end required attributes */

		sx_push (sxDecCtrl, sizeof(EnumTypeElements)/sizeof(SX_ELEMENT), EnumTypeElements, SD_FALSE);    
	}
	else
		sx_pop (sxDecCtrl);
}

/************************************************************************/
/*			_EnumVal_SEFun					*/
/************************************************************************/

static ST_VOID _EnumVal_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_RET ret;
	ST_INT strLen;
	ST_BOOLEAN required = SD_FALSE;
	SCL_ENUMVAL *scl_enumval;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		if ((scl_enumval = sclDecCtrl->scl_enumval = scl_enumtype_add_enumval (sclDecCtrl->sclInfo)) == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_enumtype_add_enumval", SX_USER_ERROR);
			return;
		}

		/* start required attributes */
		required = SD_TRUE;
		ret = scl_get_int_attr (sxDecCtrl, "ord", &scl_enumval->ord, required);
		if (ret != SD_SUCCESS)
		{
			return;
		}
		/* end required attributes */
	}
	else /* reason = SX_ELEMENT_END */
	{
		scl_enumval = sclDecCtrl->scl_enumval;
		/* CRITICAL: Init strLen = max len. After sx_get_string, strLen = actual len*/
		strLen = sizeof(scl_enumval->EnumVal)-1;
		ret = sx_get_string (sxDecCtrl, scl_enumval->EnumVal, &strLen);
		if (ret != SD_SUCCESS)
			scl_stop_parsing (sxDecCtrl, "_EnumVal_SEFun", SX_USER_ERROR);
	}
}

/************************************************************************/
/*			_scl_unknown_el_start				*/
/************************************************************************/

static ST_RET _scl_unknown_el_start (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *tag)
{
	SLOG_DEBUG ("SCL PARSE: Unneeded or unknown element '%s'", tag);
	return (SD_SUCCESS);
}

/************************************************************************/
/*			_scl_unknown_el_end				*/
/************************************************************************/

static ST_RET _scl_unknown_el_end (SX_DEC_CTRL *sxDecCtrl, ST_CHAR *tag)
{
	return (SD_SUCCESS);
}

/************************************************************************/
/*			_SampledValueControl_SEFun			*/
/* DEBUG: if parser called separate start and end functions, the lower	*/
/*   functs could be called directly & this funct would not be needed.	*/
/************************************************************************/
static ST_VOID _SampledValueControl_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *str;		/* ptr set by scl_get_attr_ptr	*/
	ST_RET ret;
	SCL_SVCB *scl_svcb;
	ST_CHAR *desc;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		/* Alloc struct, save in sclDecCtrl, & set local ptr to it.	*/
		scl_svcb = sclDecCtrl->scl_svcb = scl_svcb_add (sclDecCtrl->sclInfo);
		if (scl_svcb == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_svcb_add", SX_USER_ERROR);
			return;
		}

		/* start required attributes */
		ret = scl_get_attr_copy (sxDecCtrl, "name", scl_svcb->name, (sizeof(scl_svcb->name)-1), SCL_ATTR_REQUIRED);
		ret |= scl_get_attr_copy (sxDecCtrl, "smvID", scl_svcb->smvID, (sizeof(scl_svcb->smvID)-1), SCL_ATTR_REQUIRED);
		ret |= scl_get_uint_attr (sxDecCtrl, "smpRate", &scl_svcb->smpRate, SCL_ATTR_REQUIRED);
		ret |= scl_get_uint_attr (sxDecCtrl, "nofASDU", &scl_svcb->nofASDU, SCL_ATTR_REQUIRED);
		if (ret != SD_SUCCESS)
			return;	/* At least one required attr not found. Stop now.	*/
		/* end required attributes */

		/* start optional attributes */
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			scl_svcb->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
		ret = scl_get_attr_copy (sxDecCtrl, "datSet", scl_svcb->datSet, (sizeof(scl_svcb->datSet)-1), SCL_ATTR_OPTIONAL);
		ret = scl_get_uint_attr (sxDecCtrl, "confRev", &scl_svcb->confRev, SCL_ATTR_OPTIONAL);
		ret = scl_get_attr_ptr (sxDecCtrl, "multicast", &str, SCL_ATTR_OPTIONAL);	/* chk "str" below*/
		if (ret == SD_SUCCESS  &&  stricmp(str, "false") == 0)
			scl_svcb->multicast = SD_FALSE;
		else
			scl_svcb->multicast = SD_TRUE;  /* default value */
		/* end optional attributes */

		sx_push (sxDecCtrl, sizeof(SampledValueControlElements)/sizeof(SX_ELEMENT), SampledValueControlElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}

/************************************************************************/
/*			_SmvOpts_SFun					*/
/************************************************************************/
static ST_VOID _SmvOpts_SFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl;
	ST_CHAR *str;		/* ptr set by scl_get_attr_ptr	*/
	ST_RET ret;
	SCL_SVCB *scl_svcb;

	sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	scl_svcb = sclDecCtrl->scl_svcb;

	/* start optional attributes */
	ret = scl_get_attr_ptr (sxDecCtrl, "sampleRate", &str, SCL_ATTR_OPTIONAL);
	if (ret == SD_SUCCESS  &&  stricmp(str, "true") == 0)
	{
		BSTR_BIT_SET_ON(scl_svcb->OptFlds, SVOPT_BITNUM_SMPRATE);
	}

	ret = scl_get_attr_ptr (sxDecCtrl, "refreshTime", &str, SCL_ATTR_OPTIONAL);
	if (ret == SD_SUCCESS  &&  stricmp(str, "true") == 0)
	{
		BSTR_BIT_SET_ON(scl_svcb->OptFlds, SVOPT_BITNUM_REFRTM);
	}

	ret = scl_get_attr_ptr (sxDecCtrl, "sampleSynchronized", &str, SCL_ATTR_OPTIONAL);
	if (ret == SD_SUCCESS  &&  stricmp(str, "true") == 0)
	{
		BSTR_BIT_SET_ON(scl_svcb->OptFlds, SVOPT_BITNUM_SMPSYNCH);
	}

	ret = scl_get_attr_ptr (sxDecCtrl, "security", &str, SCL_ATTR_OPTIONAL);
	if (ret == SD_SUCCESS  &&  stricmp(str, "true") == 0)
		scl_svcb->securityPres = SD_TRUE;	/* scl_svcb calloced so init val is FALSE*/

	/* NOTE: SCL calls this "dataRef", but 7-2 & 9-2 call it "DatSet".	*/
	ret = scl_get_attr_ptr (sxDecCtrl, "dataRef", &str, SCL_ATTR_OPTIONAL);
	if (ret == SD_SUCCESS  &&  stricmp(str, "true") == 0)
		scl_svcb->dataRefPres = SD_TRUE;	/* scl_svcb calloced so init val is FALSE*/
	/* end optional attributes */
}

/************************************************************************/
/*			_Substation_SEFun				*/
/************************************************************************/
static ST_VOID _Substation_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	ST_RET ret;
	ST_CHAR *desc;
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SCL_SUBSTATION *substation = (SCL_SUBSTATION *) chk_calloc (1, sizeof (SCL_SUBSTATION));
		list_add_first (&sclDecCtrl->sclInfo->substationHead, substation);

		/* Get required attributes.	*/
		ret = scl_get_attr_copy (sxDecCtrl, "name", substation->name, (sizeof(substation->name)-1), SCL_ATTR_REQUIRED);
		if (ret)
		{
			scl_stop_parsing (sxDecCtrl, "Substation", SX_USER_ERROR);
			return;
		}
		/* Get optional attributes.	*/
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			substation->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/

		/* Start creation of flattened name */
		if (sizeof(sclDecCtrl->flattened) <= strlen(substation->name))
		{	/* error already logged.	*/
			scl_stop_parsing (sxDecCtrl, "_Substation_SEFun", SX_USER_ERROR);
			return;
		}
		strcpy(sclDecCtrl->flattened,substation->name);

		sx_push (sxDecCtrl, sizeof(SubstationElements)/sizeof(SX_ELEMENT), 
			SubstationElements, SD_FALSE);
	}
	else
	{
		sclDecCtrl->flattened[0] = '\0';
		sx_pop (sxDecCtrl);
	}
}

static ST_VOID _LNode_SFun (SX_DEC_CTRL *sxDecCtrl)
{
	ST_RET ret;
	ST_CHAR *desc;

	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	SCL_LNODE *lnd = (SCL_LNODE *) chk_calloc (1, sizeof (SCL_LNODE));
	list_add_first (&sclDecCtrl->sclInfo->substationHead->lnHead, lnd);

	/* Get required attributes.	*/
	ret = scl_get_attr_copy (sxDecCtrl, "lnClass", lnd->lnClass, (sizeof(lnd->lnClass)-1), SCL_ATTR_REQUIRED);
	if (ret)
	{
		scl_stop_parsing (sxDecCtrl, "_LNode_SFun", SX_USER_ERROR);
		return;
	}

	/* Get optional attributes.	*/
	ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
	if (ret == SD_SUCCESS)
		lnd->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/

	ret = scl_get_attr_ptr (sxDecCtrl, "lnInst", &desc, SCL_ATTR_OPTIONAL);
	if (ret == SD_SUCCESS)
		lnd->lnInst = chk_strdup (desc);	/* Alloc & copy desc string	*/

	ret = scl_get_attr_ptr (sxDecCtrl, "iedName", &desc, SCL_ATTR_OPTIONAL);
	if (ret == SD_SUCCESS)
		lnd->iedName = chk_strdup (desc);	/* Alloc & copy desc string	*/

	ret = scl_get_attr_ptr (sxDecCtrl, "ldInst", &desc, SCL_ATTR_OPTIONAL);
	if (ret == SD_SUCCESS)
		lnd->ldInst = chk_strdup (desc);	/* Alloc & copy desc string	*/

	ret = scl_get_attr_ptr (sxDecCtrl, "prefix", &desc, SCL_ATTR_OPTIONAL);
	if (ret == SD_SUCCESS)
		lnd->prefix = chk_strdup (desc);	/* Alloc & copy desc string	*/

	ret = scl_get_attr_ptr (sxDecCtrl, "lnType", &desc, SCL_ATTR_OPTIONAL);
	if (ret == SD_SUCCESS)
		lnd->lnType = chk_strdup (desc);	/* Alloc & copy desc string	*/

	strcpy(lnd->flattened,sclDecCtrl->flattened); //拷贝内容
}


static ST_VOID _PowerTransformer_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	ST_RET ret;
	ST_CHAR *desc;

	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		int uv=0;
		SCL_POWERTRANSFORMER *pwtn = (SCL_POWERTRANSFORMER *) chk_calloc (1, sizeof (SCL_POWERTRANSFORMER));
		list_add_first (&sclDecCtrl->sclInfo->substationHead->ptHead, pwtn);

		/* Get required attributes.	*/
		ret = scl_get_attr_copy (sxDecCtrl, "name", pwtn->name, (sizeof(pwtn->name)-1), SCL_ATTR_REQUIRED);
		if (ret)
		{
			scl_stop_parsing (sxDecCtrl, "PowerTransformer", SX_USER_ERROR);
			return;
		}
		ret = scl_get_attr_copy (sxDecCtrl, "type", pwtn->type, (sizeof(pwtn->type)-1), SCL_ATTR_REQUIRED);
		if (ret)
		{
			scl_stop_parsing (sxDecCtrl, "PowerTransformer", SX_USER_ERROR);
			return;
		}

		/* Get optional attributes.	*/
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			pwtn->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/
	
		scl_get_int_attr(sxDecCtrl, "virtual", &uv, SCL_ATTR_OPTIONAL);
		pwtn->bvirtual= (uv==0)?0:1;

		strcpy(pwtn->flattened,sclDecCtrl->flattened); //����·��


		/* Start creation of flattened name */
		if (construct_flattened (sclDecCtrl->flattened, sizeof(sclDecCtrl->flattened), pwtn->name, NULL)
			!= SD_SUCCESS)
		{	/* error already logged.	*/
			scl_stop_parsing (sxDecCtrl, "_PowerTransformer_SEFun", SX_USER_ERROR);
			return;
		}

		sx_push (sxDecCtrl, sizeof(PowerTransformerElements)/sizeof(SX_ELEMENT), 
			PowerTransformerElements, SD_FALSE);
	}
	else
	{
		ST_CHAR *p = strrchr(sclDecCtrl->flattened, '$');
		if (p != NULL)
			*p = 0;
		sx_pop (sxDecCtrl);
	}
}


static ST_VOID _GeneralEquipment_SEFun(SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sx_push (sxDecCtrl, sizeof(GeneralEquipmentElements)/sizeof(SX_ELEMENT), 
			GeneralEquipmentElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}


static ST_VOID _VoltageLevel_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	ST_RET ret;
	ST_CHAR *desc;

	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SCL_VOLTAGELEVEL *vollev = (SCL_VOLTAGELEVEL *) chk_calloc (1, sizeof (SCL_VOLTAGELEVEL));
		list_add_first (&sclDecCtrl->sclInfo->substationHead->vlHead, vollev);

		/* Get required attributes.	*/
		ret = scl_get_attr_copy (sxDecCtrl, "name", vollev->name, (sizeof(vollev->name)-1), SCL_ATTR_REQUIRED);
		if (ret)
		{
			scl_stop_parsing (sxDecCtrl, "Substation", SX_USER_ERROR);
			return;
		}
		/* Get optional attributes.	*/
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			vollev->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/

		/* Start creation of flattened name */
		if (construct_flattened (sclDecCtrl->flattened, sizeof(sclDecCtrl->flattened), vollev->name, NULL)
			!= SD_SUCCESS)
		{	/* error already logged.	*/
			scl_stop_parsing (sxDecCtrl, "_VoltageLevel_SEFun", SX_USER_ERROR);
			return;
		}
		sx_push (sxDecCtrl, sizeof(VoltageLevelElements)/sizeof(SX_ELEMENT), 
			VoltageLevelElements, SD_FALSE);
	}
	else
	{
		ST_CHAR *p = strrchr(sclDecCtrl->flattened, '$');
		if (p != NULL)
			*p = 0;

		sx_pop (sxDecCtrl);
	}
}


static ST_VOID _Function_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sx_push (sxDecCtrl, sizeof(FunctionElements)/sizeof(SX_ELEMENT), 
			FunctionElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}


static ST_VOID _TransformerWinding_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
	    SCL_POWERTRANSFORMER *powerfm =(SCL_POWERTRANSFORMER *)sclDecCtrl->sclInfo->substationHead->ptHead;
		powerfm->wdcounts++;

		sx_push (sxDecCtrl, sizeof(TransformerWindingElements)/sizeof(SX_ELEMENT), 
			TransformerWindingElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}

static ST_VOID _Terminal_SFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		
	}
	
}

static ST_VOID _SubEquipment_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sx_push (sxDecCtrl, sizeof(SubEquipmentElements)/sizeof(SX_ELEMENT), 
			SubEquipmentElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}


static ST_VOID _TapChanger_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sx_push (sxDecCtrl, sizeof(TapChangerElements)/sizeof(SX_ELEMENT), 
			TapChangerElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}

static ST_VOID _Voltage_SFun (SX_DEC_CTRL *sxDecCtrl)
{
	ST_RET ret;
	ST_CHAR *desc;

	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	SCL_VOLTAGELEVEL *vtlv=(SCL_VOLTAGELEVEL *)sclDecCtrl->sclInfo->substationHead->vlHead;
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		/* Get optional attributes.	*/
		ret = scl_get_attr_ptr (sxDecCtrl, "unit", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			vtlv->unit = chk_strdup (desc);	/* Alloc & copy desc string	*/

		/* Get optional attributes.	*/
		ret = scl_get_attr_ptr (sxDecCtrl, "multiplier", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			vtlv->multi = chk_strdup (desc);	/* Alloc & copy desc string	*/
	}
	else
	{
		ST_RET ret;
		ST_INT strLen;
		ST_CHAR *Val;
		ret = sx_get_string_ptr (sxDecCtrl, &Val, &strLen);
		if (ret==SD_SUCCESS)
			vtlv->volVal = atof(Val);	/* alloc & store Val*/
		else
			scl_stop_parsing (sxDecCtrl, "voltage Val", SX_USER_ERROR);
	}
}

static ST_VOID _Bay_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	ST_RET ret;
	ST_CHAR *desc;

	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		SCL_BAY *bay = (SCL_BAY *) chk_calloc (1, sizeof (SCL_BAY));
		list_add_first (&sclDecCtrl->sclInfo->substationHead->vlHead->bayHead, bay);

		/* Get required attributes.	*/
		ret = scl_get_attr_copy (sxDecCtrl, "name", bay->name, (sizeof(bay->name)-1), SCL_ATTR_REQUIRED);
		if (ret)
		{
			scl_stop_parsing (sxDecCtrl, "Substation", SX_USER_ERROR);
			return;
		}
		/* Get optional attributes.	*/
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			bay->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/

		/* Start creation of flattened name */
		if (construct_flattened (sclDecCtrl->flattened, sizeof(sclDecCtrl->flattened), bay->name, NULL)
			!= SD_SUCCESS)
		{	/* error already logged.	*/
			scl_stop_parsing (sxDecCtrl, "_Bay_SEFun", SX_USER_ERROR);
			return;
		}

		sx_push (sxDecCtrl, sizeof(BayElements)/sizeof(SX_ELEMENT), 
			BayElements, SD_FALSE);
	}
	else
	{
		ST_CHAR *p = strrchr(sclDecCtrl->flattened, '$');
		if (p != NULL)
			*p = 0;

		sx_pop (sxDecCtrl);
	}
}

static ST_VOID _ConductingEquipment_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	ST_RET ret;
	ST_CHAR *desc;

	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		int uv=0;
		SCL_CONDUCTINGEQUIPMENT *cndt = (SCL_CONDUCTINGEQUIPMENT *) chk_calloc (1, sizeof (SCL_CONDUCTINGEQUIPMENT));
		list_add_first (&sclDecCtrl->sclInfo->substationHead->ceHead, cndt);

		/* Get required attributes.	*/
		ret = scl_get_attr_copy (sxDecCtrl, "name", cndt->name, (sizeof(cndt->name)-1), SCL_ATTR_REQUIRED);
		if (ret)
		{
			scl_stop_parsing (sxDecCtrl, "_ConductingEquipment_SEFun", SX_USER_ERROR);
			return;
		}
		ret = scl_get_attr_copy (sxDecCtrl, "type", cndt->type, (sizeof(cndt->type)-1), SCL_ATTR_REQUIRED);
		if (ret)
		{
			scl_stop_parsing (sxDecCtrl, "_ConductingEquipment_SEFun", SX_USER_ERROR);
			return;
		}

		/* Get optional attributes.	*/
		ret = scl_get_attr_ptr (sxDecCtrl, "desc", &desc, SCL_ATTR_OPTIONAL);
		if (ret == SD_SUCCESS)
			cndt->desc = chk_strdup (desc);	/* Alloc & copy desc string	*/

		scl_get_int_attr(sxDecCtrl, "virtual", &uv, SCL_ATTR_OPTIONAL);
		cndt->bvirtual= (uv==0)?0:1;

		strcpy(cndt->flattened,sclDecCtrl->flattened); //����·��

		/* Start creation of flattened name */
		if (construct_flattened (sclDecCtrl->flattened, sizeof(sclDecCtrl->flattened), cndt->name, NULL)
			!= SD_SUCCESS)
		{	/* error already logged.	*/
			scl_stop_parsing (sxDecCtrl, "_ConductingEquipment_SEFun", SX_USER_ERROR);
			return;
		}

		sx_push (sxDecCtrl, sizeof(ConductingEquipmentElements)/sizeof(SX_ELEMENT), 
			ConductingEquipmentElements, SD_FALSE);
	}
	else
	{
		ST_CHAR *p = strrchr(sclDecCtrl->flattened, '$');
		if (p != NULL)
			*p = 0;
		sx_pop (sxDecCtrl);
	}
}

static ST_VOID _ConnectivityNode_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sx_push (sxDecCtrl, sizeof(ConnectivityNodeElements)/sizeof(SX_ELEMENT), 
			ConnectivityNodeElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}


static ST_VOID _SubFunction_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		sx_push (sxDecCtrl, sizeof(SubFunctionElements)/sizeof(SX_ELEMENT), 
			SubFunctionElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}

int HexStrToInt(const ST_CHAR *p)
{
	int iRst=0;
	int iMul=1;
	const ST_CHAR *p1=p+strlen(p)-1;
	for (;p1>=p;p1--)
	{
		switch (*p1)
		{
		case '1':
			iRst+=1*iMul;
			break;
		case '2':
			iRst+=2*iMul;
			break;
		case '3':
			iRst+=3*iMul;
			break;
		case '4':
			iRst+=4*iMul;
			break;
		case '5':
			iRst+=5*iMul;
			break;
		case '6':
			iRst+=6*iMul;
			break;
		case '7':
			iRst+=7*iMul;
			break;
		case '8':
			iRst+=8*iMul;
			break;
		case '9':
			iRst+=9*iMul;
			break;
		case 'A':
			iRst+=10*iMul;
			break;
		case 'B':
			iRst+=11*iMul;
			break;
		case 'C':
			iRst+=12*iMul;
			break;
		case 'D':
			iRst+=13*iMul;
			break;
		case 'E':
			iRst+=14*iMul;
			break;
		case 'F':
			iRst+=15*iMul;
			break;
		default:
			break;
		}
		iMul*=16;
	}
	return iRst;
}

static ST_VOID _Address_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;

	if (sxDecCtrl->reason == SX_ELEMENT_START)
	{
		/* NOTE: save ptr in sclDecCtrl->scl_gse to use later in parsing.	*/
		sclDecCtrl->scl_addr = scl_address_add (sclDecCtrl->sclInfo);
		if (sclDecCtrl->scl_addr == NULL)
		{
			scl_stop_parsing (sxDecCtrl, "scl_address_add", SX_USER_ERROR);
			return;
		}
	
		sx_push (sxDecCtrl, sizeof(AddressElements)/sizeof(SX_ELEMENT), 
			AddressElements, SD_FALSE);
	}
	else
	{
		sx_pop (sxDecCtrl);
	}
}

/************************************************************************/
/*			_Address_P_SEFun				*/
/************************************************************************/
static ST_VOID _Address_P_SEFun (SX_DEC_CTRL *sxDecCtrl)
{
	SCL_DEC_CTRL *sclDecCtrl = (SCL_DEC_CTRL *) sxDecCtrl->usr;
	ST_CHAR *str;
	ST_RET ret;
	ST_BOOLEAN required = SD_FALSE;
	ST_CHAR *strOut;
	ST_INT strLen;

	if (sxDecCtrl->reason == SX_ELEMENT_END)
	{
		ret = scl_get_attr_ptr (sxDecCtrl, "type", &str, required);
		if (!strcmpi(str,"IP"))
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS)
			{
				strncpy_safe (sclDecCtrl->scl_addr->IP, strOut, 20);
			}
		}
		else if (!strcmpi(str,"IP-SUBNET"))
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS)
				strncpy_safe (sclDecCtrl->scl_addr->IPSUBNET, strOut, 20);
		}
		else if (!strcmpi(str,"IP-GATEWAY"))
		{
			ret = sx_get_string_ptr (sxDecCtrl, &strOut, &strLen);
			if (ret == SD_SUCCESS)
				strncpy_safe (sclDecCtrl->scl_addr->IPGATEWAY, strOut, 20);
		}
	}
}

/************************************************************************/
/*			scl_rptCtlget										
/* 				对外服务函数,提取rpt里面的config
/************************************************************************/
ST_CHAR *scl_rptCtlget(ST_UINT8* rptPtr, SD_CONST ST_CHAR *field)
{
	if (rptPtr == NULL || field == NULL)
	{
		return NULL;
	}
	unsigned int i;
	typedef struct  {
		ST_CHAR* str;
		ST_UINT8 code;
	}STFieldCode;

	SD_CONST STFieldCode fieldArr[] = 
	{
		{"dchg", 	TRGOPS_BITNUM_DATA_CHANGE},
		{"qchg", 	TRGOPS_BITNUM_QUALITY_CHANGE},
		{"dupd", 	TRGOPS_BITNUM_DATA_UPDATE},
		{"period", 	TRGOPS_BITNUM_INTEGRITY},
		{"seqNum",		OPTFLD_BITNUM_SQNUM},
		{"timeStamp", 	OPTFLD_BITNUM_TIMESTAMP},
		{"dataSet", 	OPTFLD_BITNUM_REASON},
		{"reasonCode", 	OPTFLD_BITNUM_DATSETNAME},
		{"dataRef", 	OPTFLD_BITNUM_DATAREF},
		{"bufOvfl", 	OPTFLD_BITNUM_BUFOVFL},
		{"entryID", 	OPTFLD_BITNUM_ENTRYID},
		{"configRef", 	OPTFLD_BITNUM_CONFREV}
	};
	// ST_UINT8 i;
	for ( i = 0; i < sizeof(fieldArr)/sizeof(fieldArr[0]); i++) 
	{
		if ( !strcmp(fieldArr[i].str, field) ) {
			return BSTR_BIT_GET(rptPtr, fieldArr[i].code);
		}
	}

	return NULL;
}