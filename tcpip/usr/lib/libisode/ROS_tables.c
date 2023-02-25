static char sccsid[] = "@(#)00	1.3  src/tcpip/usr/lib/libisode/ROS_tables.c, isodelib7, tcpip411, GOLD410 4/5/93 13:35:55";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: AOFFSET OFFSET
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ROS_tables.c
 */

#include <stdio.h>
#include "ROS-types.h"


# line 36 "ros.py"

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ROS_tables.c,v 1.2 93/02/05 17:02:29 snmp Exp $";
#endif

#include <stdio.h>
#include <isode/ropkt.h>


int	rosap_operation;
int	rosap_error;
int	rosap_type;
int	rosap_id;
int	rosap_null;
int	rosap_linked;
int	rosap_lnull;
PE	rosap_data;
int	rosap_reason;

/*  */

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static tpe et_OperationROS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ROS_Operation, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ErrorROS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ROS_Error, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ROSEapdusROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_ROS_ROSEapdus, offset), 0, 0 },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_ROSEapdus, un.roiv__apdu), _ZROIVapduROS, FL_CONTEXT },
	{ IMP_OBJ, 0, 2, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_ROSEapdus, un.rors__apdu), _ZRORSapduROS, FL_CONTEXT },
	{ IMP_OBJ, 0, 3, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_ROSEapdus, un.roer__apdu), _ZROERapduROS, FL_CONTEXT },
	{ IMP_OBJ, 0, 4, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_ROSEapdus, un.rorj__apdu), _ZRORJapduROS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ROIVapduROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ OPTL, OFFSET(struct type_ROS_ROIVapdu, optionals), 0, 0 },
	{ OBJECT, OFFSET(struct type_ROS_ROIVapdu, invokeID), _ZInvokeIDTypeROS, 0 },
	{ INTEGER, OFFSET(struct type_ROS_ROIVapdu, linked__ID), 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ROS_ROIVapdu, operation__value), _ZOperationROS, 0 },
	{ ANY, OFFSET(struct type_ROS_ROIVapdu, argument), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_InvokeIDTypeROS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ROS_InvokeIDType, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RORSapduROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_ROS_RORSapdu, invokeID), _ZInvokeIDTypeROS, 0 },
	{ SEQ_START, OFFSET(struct type_ROS_RORSapdu, element_ROS_0), 16, FL_UNIVERSAL|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct element_ROS_1, operation__value), _ZOperationROS, 0 },
	{ ANY, OFFSET(struct element_ROS_1, result), -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ROERapduROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_ROS_ROERapdu, invokeID), _ZInvokeIDTypeROS, 0 },
	{ OBJECT, OFFSET(struct type_ROS_ROERapdu, error__value), _ZErrorROS, 0 },
	{ ANY, OFFSET(struct type_ROS_ROERapdu, parameter), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RORJapduROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ CHOICE_START, OFFSET(struct type_ROS_RORJapdu, invokeID), 0, 0 },
	{ SCTRL, OFFSET(struct choice_ROS_0, offset), 0, 0 },
	{ OBJECT, OFFSET(struct choice_ROS_0, un.choice_ROS_1), _ZInvokeIDTypeROS, 0 },
	{ T_NULL, 0, 5, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ CHOICE_START, OFFSET(struct type_ROS_RORJapdu, problem), 0, 0 },
	{ SCTRL, OFFSET(struct choice_ROS_3, offset), 0, 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct choice_ROS_3, un.choice_ROS_4), _ZGeneralProblemROS, FL_CONTEXT },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct choice_ROS_3, un.choice_ROS_5), _ZInvokeProblemROS, FL_CONTEXT },
	{ IMP_OBJ, 0, 2, FL_CONTEXT },
	{ OBJECT, OFFSET(struct choice_ROS_3, un.choice_ROS_6), _ZReturnResultProblemROS, FL_CONTEXT },
	{ IMP_OBJ, 0, 3, FL_CONTEXT },
	{ OBJECT, OFFSET(struct choice_ROS_3, un.choice_ROS_7), _ZReturnErrorProblemROS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_GeneralProblemROS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ROS_GeneralProblem, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_InvokeProblemROS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ROS_InvokeProblem, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ReturnResultProblemROS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ROS_ReturnResultProblem, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ReturnErrorProblemROS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_ROS_ReturnErrorProblem, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_OPDUROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_ROS_OPDU, offset), 0, 0 },
	{ ETAG, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_OPDU, un.choice_ROS_8), _ZInvokeROS, FL_UNIVERSAL },
	{ ETAG, 0, 2, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_OPDU, un.choice_ROS_9), _ZReturnResultROS, FL_UNIVERSAL },
	{ ETAG, 0, 3, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_OPDU, un.choice_ROS_10), _ZReturnErrorROS, FL_UNIVERSAL },
	{ ETAG, 0, 4, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_OPDU, un.choice_ROS_11), _ZRejectROS, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_InvokeROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_ROS_Invoke, invokeID), 2, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_ROS_Invoke, element_ROS_2), _ZOperationROS, 0 },
	{ ANY, OFFSET(struct type_ROS_Invoke, argument), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ReturnResultROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_ROS_ReturnResult, invokeID), _ZInvokeIDTypeROS, 0 },
	{ ANY, OFFSET(struct type_ROS_ReturnResult, result), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ReturnErrorROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_ROS_ReturnError, invokeID), 2, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_ROS_ReturnError, element_ROS_3), _ZErrorROS, 0 },
	{ ANY, OFFSET(struct type_ROS_ReturnError, parameter), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RejectROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZRORJapduROS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_OperationROS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ROS_Operation), 0 },
	{ INTEGER, OFFSET(struct type_ROS_Operation, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ErrorROS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ROS_Error), 0 },
	{ INTEGER, OFFSET(struct type_ROS_Error, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ROSEapdusROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ROS_ROSEapdus), 0 },
	{ SCTRL, OFFSET(struct type_ROS_ROSEapdus, offset), 0, 0 },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_ROSEapdus, un.roiv__apdu), _ZROIVapduROS, FL_CONTEXT },
	{ IMP_OBJ, 0, 2, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_ROSEapdus, un.rors__apdu), _ZRORSapduROS, FL_CONTEXT },
	{ IMP_OBJ, 0, 3, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_ROSEapdus, un.roer__apdu), _ZROERapduROS, FL_CONTEXT },
	{ IMP_OBJ, 0, 4, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_ROSEapdus, un.rorj__apdu), _ZRORJapduROS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ROIVapduROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_ROS_ROIVapdu), 0 },
	{ OPTL, OFFSET(struct type_ROS_ROIVapdu, optionals), 0, 0 },
	{ OBJECT, OFFSET(struct type_ROS_ROIVapdu, invokeID), _ZInvokeIDTypeROS, 0 },
	{ INTEGER, OFFSET(struct type_ROS_ROIVapdu, linked__ID), 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_ROS_ROIVapdu, operation__value), _ZOperationROS, 0 },
	{ ANY, OFFSET(struct type_ROS_ROIVapdu, argument), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_InvokeIDTypeROS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ROS_InvokeIDType), 0 },
	{ INTEGER, OFFSET(struct type_ROS_InvokeIDType, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RORSapduROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_ROS_RORSapdu), 0 },
	{ OBJECT, OFFSET(struct type_ROS_RORSapdu, invokeID), _ZInvokeIDTypeROS, 0 },
	{ SEQ_START, OFFSET(struct type_ROS_RORSapdu, element_ROS_0), 16, FL_UNIVERSAL|FL_OPTIONAL },
	{ MEMALLOC, 0, sizeof (struct element_ROS_1), 0 },
	{ OBJECT, OFFSET(struct element_ROS_1, operation__value), _ZOperationROS, 0 },
	{ ANY, OFFSET(struct element_ROS_1, result), -1, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ROERapduROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_ROS_ROERapdu), 0 },
	{ OBJECT, OFFSET(struct type_ROS_ROERapdu, invokeID), _ZInvokeIDTypeROS, 0 },
	{ OBJECT, OFFSET(struct type_ROS_ROERapdu, error__value), _ZErrorROS, 0 },
	{ ANY, OFFSET(struct type_ROS_ROERapdu, parameter), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RORJapduROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_ROS_RORJapdu), 0 },
	{ CHOICE_START, OFFSET(struct type_ROS_RORJapdu, invokeID), 0, 0 },
	{ MEMALLOC, 0, sizeof (struct choice_ROS_0), 0 },
	{ SCTRL, OFFSET(struct choice_ROS_0, offset), 0, 0 },
	{ OBJECT, OFFSET(struct choice_ROS_0, un.choice_ROS_1), _ZInvokeIDTypeROS, 0 },
	{ T_NULL, 0, 5, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ CHOICE_START, OFFSET(struct type_ROS_RORJapdu, problem), 0, 0 },
	{ MEMALLOC, 0, sizeof (struct choice_ROS_3), 0 },
	{ SCTRL, OFFSET(struct choice_ROS_3, offset), 0, 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct choice_ROS_3, un.choice_ROS_4), _ZGeneralProblemROS, FL_CONTEXT },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct choice_ROS_3, un.choice_ROS_5), _ZInvokeProblemROS, FL_CONTEXT },
	{ IMP_OBJ, 0, 2, FL_CONTEXT },
	{ OBJECT, OFFSET(struct choice_ROS_3, un.choice_ROS_6), _ZReturnResultProblemROS, FL_CONTEXT },
	{ IMP_OBJ, 0, 3, FL_CONTEXT },
	{ OBJECT, OFFSET(struct choice_ROS_3, un.choice_ROS_7), _ZReturnErrorProblemROS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_GeneralProblemROS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ROS_GeneralProblem), 0 },
	{ INTEGER, OFFSET(struct type_ROS_GeneralProblem, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_InvokeProblemROS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ROS_InvokeProblem), 0 },
	{ INTEGER, OFFSET(struct type_ROS_InvokeProblem, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ReturnResultProblemROS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ROS_ReturnResultProblem), 0 },
	{ INTEGER, OFFSET(struct type_ROS_ReturnResultProblem, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ReturnErrorProblemROS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ROS_ReturnErrorProblem), 0 },
	{ INTEGER, OFFSET(struct type_ROS_ReturnErrorProblem, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_OPDUROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_ROS_OPDU), 0 },
	{ SCTRL, OFFSET(struct type_ROS_OPDU, offset), 0, 0 },
	{ ETAG, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_OPDU, un.choice_ROS_8), _ZInvokeROS, FL_UNIVERSAL },
	{ ETAG, 0, 2, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_OPDU, un.choice_ROS_9), _ZReturnResultROS, FL_UNIVERSAL },
	{ ETAG, 0, 3, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_OPDU, un.choice_ROS_10), _ZReturnErrorROS, FL_UNIVERSAL },
	{ ETAG, 0, 4, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_ROS_OPDU, un.choice_ROS_11), _ZRejectROS, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_InvokeROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_ROS_Invoke), 0 },
	{ INTEGER, OFFSET(struct type_ROS_Invoke, invokeID), 2, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_ROS_Invoke, element_ROS_2), _ZOperationROS, 0 },
	{ ANY, OFFSET(struct type_ROS_Invoke, argument), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ReturnResultROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_ROS_ReturnResult), 0 },
	{ OBJECT, OFFSET(struct type_ROS_ReturnResult, invokeID), _ZInvokeIDTypeROS, 0 },
	{ ANY, OFFSET(struct type_ROS_ReturnResult, result), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ReturnErrorROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_ROS_ReturnError), 0 },
	{ INTEGER, OFFSET(struct type_ROS_ReturnError, invokeID), 2, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_ROS_ReturnError, element_ROS_3), _ZErrorROS, 0 },
	{ ANY, OFFSET(struct type_ROS_ReturnError, parameter), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RejectROS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZRORJapduROS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static ptpe pt_OperationROS[] = {
	{ PE_START, 0, 0, 0, "Operation" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ErrorROS[] = {
	{ PE_START, 0, 0, 0, "Error" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ROSEapdusROS[] = {
	{ PE_START, 0, 0, 0, "ROSEapdus" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ IMP_OBJ, -1, 1, FL_CONTEXT, "roiv-apdu" },
	{ OBJECT, 0, _ZROIVapduROS, FL_CONTEXT, "roiv-apdu" },
	{ IMP_OBJ, -1, 2, FL_CONTEXT, "rors-apdu" },
	{ OBJECT, 0, _ZRORSapduROS, FL_CONTEXT, "rors-apdu" },
	{ IMP_OBJ, -1, 3, FL_CONTEXT, "roer-apdu" },
	{ OBJECT, 0, _ZROERapduROS, FL_CONTEXT, "roer-apdu" },
	{ IMP_OBJ, -1, 4, FL_CONTEXT, "rorj-apdu" },
	{ OBJECT, 0, _ZRORJapduROS, FL_CONTEXT, "rorj-apdu" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ROIVapduROS[] = {
	{ PE_START, 0, 0, 0, "ROIVapdu" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ OBJECT, 0, _ZInvokeIDTypeROS, 0, "invokeID" },
	{ INTEGER, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "linked-ID" },
	{ OBJECT, 0, _ZOperationROS, 0, "operation-value" },
	{ ANY, -1, -1, FL_UNIVERSAL|FL_OPTIONAL, "argument" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_InvokeIDTypeROS[] = {
	{ PE_START, 0, 0, 0, "InvokeIDType" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RORSapduROS[] = {
	{ PE_START, 0, 0, 0, "RORSapdu" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ OBJECT, 0, _ZInvokeIDTypeROS, 0, "invokeID" },
	{ SEQ_START, -1, 16, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ OBJECT, 0, _ZOperationROS, 0, "operation-value" },
	{ ANY, -1, -1, FL_UNIVERSAL, "result" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ROERapduROS[] = {
	{ PE_START, 0, 0, 0, "ROERapdu" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ OBJECT, 0, _ZInvokeIDTypeROS, 0, "invokeID" },
	{ OBJECT, 0, _ZErrorROS, 0, "error-value" },
	{ ANY, -1, -1, FL_UNIVERSAL|FL_OPTIONAL, "parameter" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RORJapduROS[] = {
	{ PE_START, 0, 0, 0, "RORJapdu" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ CHOICE_START, -1, 0, 0, "invokeID" },
	{ SCTRL, 0, 0, 0, "invokeID" },
	{ OBJECT, 0, _ZInvokeIDTypeROS, 0, "InvokeIDType" },
	{ T_NULL, -1, 5, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ CHOICE_START, -1, 0, 0, "problem" },
	{ SCTRL, 0, 0, 0, "problem" },
	{ IMP_OBJ, -1, 0, FL_CONTEXT, "GeneralProblem" },
	{ OBJECT, 0, _ZGeneralProblemROS, FL_CONTEXT, "GeneralProblem" },
	{ IMP_OBJ, -1, 1, FL_CONTEXT, "InvokeProblem" },
	{ OBJECT, 0, _ZInvokeProblemROS, FL_CONTEXT, "InvokeProblem" },
	{ IMP_OBJ, -1, 2, FL_CONTEXT, "ReturnResultProblem" },
	{ OBJECT, 0, _ZReturnResultProblemROS, FL_CONTEXT, "ReturnResultProblem" },
	{ IMP_OBJ, -1, 3, FL_CONTEXT, "ReturnErrorProblem" },
	{ OBJECT, 0, _ZReturnErrorProblemROS, FL_CONTEXT, "ReturnErrorProblem" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_GeneralProblemROS[] = {
	{ PE_START, 0, 0, 0, "GeneralProblem" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_InvokeProblemROS[] = {
	{ PE_START, 0, 0, 0, "InvokeProblem" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ReturnResultProblemROS[] = {
	{ PE_START, 0, 0, 0, "ReturnResultProblem" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ReturnErrorProblemROS[] = {
	{ PE_START, 0, 0, 0, "ReturnErrorProblem" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_OPDUROS[] = {
	{ PE_START, 0, 0, 0, "OPDU" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ ETAG, -1, 1, FL_CONTEXT, "Invoke" },
	{ OBJECT, 0, _ZInvokeROS, FL_UNIVERSAL, NULL},
	{ ETAG, -1, 2, FL_CONTEXT, "ReturnResult" },
	{ OBJECT, 0, _ZReturnResultROS, FL_UNIVERSAL, NULL},
	{ ETAG, -1, 3, FL_CONTEXT, "ReturnError" },
	{ OBJECT, 0, _ZReturnErrorROS, FL_UNIVERSAL, NULL},
	{ ETAG, -1, 4, FL_CONTEXT, "Reject" },
	{ OBJECT, 0, _ZRejectROS, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_InvokeROS[] = {
	{ PE_START, 0, 0, 0, "Invoke" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 2, FL_UNIVERSAL, "invokeID" },
	{ OBJECT, 0, _ZOperationROS, 0, "Operation" },
	{ ANY, -1, -1, FL_UNIVERSAL|FL_OPTIONAL, "argument" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ReturnResultROS[] = {
	{ PE_START, 0, 0, 0, "ReturnResult" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ OBJECT, 0, _ZInvokeIDTypeROS, 0, "invokeID" },
	{ ANY, -1, -1, FL_UNIVERSAL|FL_OPTIONAL, "result" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ReturnErrorROS[] = {
	{ PE_START, 0, 0, 0, "ReturnError" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 2, FL_UNIVERSAL, "invokeID" },
	{ OBJECT, 0, _ZErrorROS, 0, "Error" },
	{ ANY, -1, -1, FL_UNIVERSAL|FL_OPTIONAL, "parameter" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RejectROS[] = {
	{ PE_START, 0, 0, 0, "Reject" },
	{ SOBJECT, 0, _ZRORJapduROS, 0, "RORJapdu" },
	{ PE_END, 0, 0, 0, NULL }
	};

static tpe *etabl[] = {
	et_OperationROS,
	et_ErrorROS,
	et_ROSEapdusROS,
	et_ROIVapduROS,
	et_InvokeIDTypeROS,
	et_RORSapduROS,
	et_ROERapduROS,
	et_RORJapduROS,
	et_GeneralProblemROS,
	et_InvokeProblemROS,
	et_ReturnResultProblemROS,
	et_ReturnErrorProblemROS,
	et_OPDUROS,
	et_InvokeROS,
	et_ReturnResultROS,
	et_ReturnErrorROS,
	et_RejectROS,
	};

static tpe *dtabl[] = {
	dt_OperationROS,
	dt_ErrorROS,
	dt_ROSEapdusROS,
	dt_ROIVapduROS,
	dt_InvokeIDTypeROS,
	dt_RORSapduROS,
	dt_ROERapduROS,
	dt_RORJapduROS,
	dt_GeneralProblemROS,
	dt_InvokeProblemROS,
	dt_ReturnResultProblemROS,
	dt_ReturnErrorProblemROS,
	dt_OPDUROS,
	dt_InvokeROS,
	dt_ReturnResultROS,
	dt_ReturnErrorROS,
	dt_RejectROS,
	};

static ptpe *ptabl[] = {
	pt_OperationROS,
	pt_ErrorROS,
	pt_ROSEapdusROS,
	pt_ROIVapduROS,
	pt_InvokeIDTypeROS,
	pt_RORSapduROS,
	pt_ROERapduROS,
	pt_RORJapduROS,
	pt_GeneralProblemROS,
	pt_InvokeProblemROS,
	pt_ReturnResultProblemROS,
	pt_ReturnErrorProblemROS,
	pt_OPDUROS,
	pt_InvokeROS,
	pt_ReturnResultROS,
	pt_ReturnErrorROS,
	pt_RejectROS,
	};


/* Pointer table 0 entries */
static caddr_t _ZptrtabROS[] = {
    (caddr_t ) 0,
};

modtyp _ZROS_mod = {
	"ROS",
	17,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabROS,
	};


#ifdef	lint

#undef encode_ROS_Operation
int	encode_ROS_Operation(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_Operation *parm;
{
  return (enc_f(_ZOperationROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_Operation
int	decode_ROS_Operation(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_Operation **parm;
{
  return (dec_f(_ZOperationROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_Operation
/* ARGSUSED */
int	print_ROS_Operation(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_Operation *parm;
{
  return (prnt_f(_ZOperationROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_Error
int	encode_ROS_Error(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_Error *parm;
{
  return (enc_f(_ZErrorROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_Error
int	decode_ROS_Error(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_Error **parm;
{
  return (dec_f(_ZErrorROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_Error
/* ARGSUSED */
int	print_ROS_Error(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_Error *parm;
{
  return (prnt_f(_ZErrorROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_ROSEapdus
int	encode_ROS_ROSEapdus(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_ROSEapdus *parm;
{
  return (enc_f(_ZROSEapdusROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_ROSEapdus
int	decode_ROS_ROSEapdus(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ROSEapdus **parm;
{
  return (dec_f(_ZROSEapdusROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_ROSEapdus
/* ARGSUSED */
int	print_ROS_ROSEapdus(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ROSEapdus *parm;
{
  return (prnt_f(_ZROSEapdusROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_ROIVapdu
int	encode_ROS_ROIVapdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_ROIVapdu *parm;
{
  return (enc_f(_ZROIVapduROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_ROIVapdu
int	decode_ROS_ROIVapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ROIVapdu **parm;
{
  return (dec_f(_ZROIVapduROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_ROIVapdu
/* ARGSUSED */
int	print_ROS_ROIVapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ROIVapdu *parm;
{
  return (prnt_f(_ZROIVapduROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_InvokeIDType
int	encode_ROS_InvokeIDType(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_InvokeIDType *parm;
{
  return (enc_f(_ZInvokeIDTypeROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_InvokeIDType
int	decode_ROS_InvokeIDType(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_InvokeIDType **parm;
{
  return (dec_f(_ZInvokeIDTypeROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_InvokeIDType
/* ARGSUSED */
int	print_ROS_InvokeIDType(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_InvokeIDType *parm;
{
  return (prnt_f(_ZInvokeIDTypeROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_RORSapdu
int	encode_ROS_RORSapdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_RORSapdu *parm;
{
  return (enc_f(_ZRORSapduROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_RORSapdu
int	decode_ROS_RORSapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_RORSapdu **parm;
{
  return (dec_f(_ZRORSapduROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_RORSapdu
/* ARGSUSED */
int	print_ROS_RORSapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_RORSapdu *parm;
{
  return (prnt_f(_ZRORSapduROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_ROERapdu
int	encode_ROS_ROERapdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_ROERapdu *parm;
{
  return (enc_f(_ZROERapduROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_ROERapdu
int	decode_ROS_ROERapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ROERapdu **parm;
{
  return (dec_f(_ZROERapduROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_ROERapdu
/* ARGSUSED */
int	print_ROS_ROERapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ROERapdu *parm;
{
  return (prnt_f(_ZROERapduROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_RORJapdu
int	encode_ROS_RORJapdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_RORJapdu *parm;
{
  return (enc_f(_ZRORJapduROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_RORJapdu
int	decode_ROS_RORJapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_RORJapdu **parm;
{
  return (dec_f(_ZRORJapduROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_RORJapdu
/* ARGSUSED */
int	print_ROS_RORJapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_RORJapdu *parm;
{
  return (prnt_f(_ZRORJapduROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_GeneralProblem
int	encode_ROS_GeneralProblem(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_GeneralProblem *parm;
{
  return (enc_f(_ZGeneralProblemROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_GeneralProblem
int	decode_ROS_GeneralProblem(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_GeneralProblem **parm;
{
  return (dec_f(_ZGeneralProblemROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_GeneralProblem
/* ARGSUSED */
int	print_ROS_GeneralProblem(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_GeneralProblem *parm;
{
  return (prnt_f(_ZGeneralProblemROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_InvokeProblem
int	encode_ROS_InvokeProblem(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_InvokeProblem *parm;
{
  return (enc_f(_ZInvokeProblemROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_InvokeProblem
int	decode_ROS_InvokeProblem(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_InvokeProblem **parm;
{
  return (dec_f(_ZInvokeProblemROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_InvokeProblem
/* ARGSUSED */
int	print_ROS_InvokeProblem(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_InvokeProblem *parm;
{
  return (prnt_f(_ZInvokeProblemROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_ReturnResultProblem
int	encode_ROS_ReturnResultProblem(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_ReturnResultProblem *parm;
{
  return (enc_f(_ZReturnResultProblemROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_ReturnResultProblem
int	decode_ROS_ReturnResultProblem(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ReturnResultProblem **parm;
{
  return (dec_f(_ZReturnResultProblemROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_ReturnResultProblem
/* ARGSUSED */
int	print_ROS_ReturnResultProblem(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ReturnResultProblem *parm;
{
  return (prnt_f(_ZReturnResultProblemROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_ReturnErrorProblem
int	encode_ROS_ReturnErrorProblem(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_ReturnErrorProblem *parm;
{
  return (enc_f(_ZReturnErrorProblemROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_ReturnErrorProblem
int	decode_ROS_ReturnErrorProblem(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ReturnErrorProblem **parm;
{
  return (dec_f(_ZReturnErrorProblemROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_ReturnErrorProblem
/* ARGSUSED */
int	print_ROS_ReturnErrorProblem(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ReturnErrorProblem *parm;
{
  return (prnt_f(_ZReturnErrorProblemROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_OPDU
int	encode_ROS_OPDU(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_OPDU *parm;
{
  return (enc_f(_ZOPDUROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_OPDU
int	decode_ROS_OPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_OPDU **parm;
{
  return (dec_f(_ZOPDUROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_OPDU
/* ARGSUSED */
int	print_ROS_OPDU(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_OPDU *parm;
{
  return (prnt_f(_ZOPDUROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_Invoke
int	encode_ROS_Invoke(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_Invoke *parm;
{
  return (enc_f(_ZInvokeROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_Invoke
int	decode_ROS_Invoke(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_Invoke **parm;
{
  return (dec_f(_ZInvokeROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_Invoke
/* ARGSUSED */
int	print_ROS_Invoke(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_Invoke *parm;
{
  return (prnt_f(_ZInvokeROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_ReturnResult
int	encode_ROS_ReturnResult(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_ReturnResult *parm;
{
  return (enc_f(_ZReturnResultROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_ReturnResult
int	decode_ROS_ReturnResult(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ReturnResult **parm;
{
  return (dec_f(_ZReturnResultROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_ReturnResult
/* ARGSUSED */
int	print_ROS_ReturnResult(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ReturnResult *parm;
{
  return (prnt_f(_ZReturnResultROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_ReturnError
int	encode_ROS_ReturnError(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_ReturnError *parm;
{
  return (enc_f(_ZReturnErrorROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_ReturnError
int	decode_ROS_ReturnError(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ReturnError **parm;
{
  return (dec_f(_ZReturnErrorROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_ReturnError
/* ARGSUSED */
int	print_ROS_ReturnError(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_ReturnError *parm;
{
  return (prnt_f(_ZReturnErrorROS, &_ZROS_mod, pe, top, len, buffer));
}

#undef encode_ROS_Reject
int	encode_ROS_Reject(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_ROS_Reject *parm;
{
  return (enc_f(_ZRejectROS, &_ZROS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_ROS_Reject
int	decode_ROS_Reject(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_Reject **parm;
{
  return (dec_f(_ZRejectROS, &_ZROS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_ROS_Reject
/* ARGSUSED */
int	print_ROS_Reject(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_ROS_Reject *parm;
{
  return (prnt_f(_ZRejectROS, &_ZROS_mod, pe, top, len, buffer));
}

#endif	/* lint */
