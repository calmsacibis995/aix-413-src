static char sccsid[] = "@(#)68	1.3  src/tcpip/usr/lib/libisode/OACS_tables.c, isodelib7, tcpip411, GOLD410 4/5/93 13:34:08";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: decode_OACS_AbortInformation decode_OACS_AbortReason 
 *    decode_OACS_AdditionalReferenceInformation 
 *    decode_OACS_CallingSSUserReference decode_OACS_CommonReference 
 *    decode_OACS_ConnectionData decode_OACS_DataTransferSyntax 
 *    decode_OACS_PAccept decode_OACS_PConnect decode_OACS_PRefuse 
 *    decode_OACS_Priority decode_OACS_RefuseReason decode_OACS_SSAPAddress 
 *    decode_OACS_SessionConnectionIdentifier
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/OACS_tables.c
 */

#include <stdio.h>
#include "OACS-types.h"


#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static tpe et_PConnectOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_OACS_PConnect, member_OACS_0), _ZDataTransferSyntaxOACS, FL_CONTEXT },
	{ SET_START, OFFSET(struct type_OACS_PConnect, pUserData), 1, FL_CONTEXT },
	{ OPTL, OFFSET(struct member_OACS_1, optionals), 0, 0 },
	{ DFLT_F,	0,	0,	0 },
	{ INTEGER, OFFSET(struct member_OACS_1, checkpointSize), 0, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_F,	3,	0,	0 },
	{ INTEGER, OFFSET(struct member_OACS_1, windowSize), 1, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_F,	0,	0,	0 },
	{ INTEGER, OFFSET(struct member_OACS_1, dialogueMode), 2, FL_CONTEXT|FL_DEFAULT },
	{ ETAG, 0, 3, FL_CONTEXT },
	{ OBJECT, OFFSET(struct member_OACS_1, member_OACS_2), _ZConnectionDataOACS, FL_UNIVERSAL },
	{ DFLT_F,	1,	0,	0 },
	{ INTEGER, OFFSET(struct member_OACS_1, applicationProtocol), 4, FL_CONTEXT|FL_DEFAULT },
	{ INTEGER, OFFSET(struct member_OACS_1, protocolVersion), 5, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_PAcceptOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_OACS_PAccept, member_OACS_3), _ZDataTransferSyntaxOACS, FL_CONTEXT },
	{ SET_START, OFFSET(struct type_OACS_PAccept, pUserData), 1, FL_CONTEXT },
	{ DFLT_F,	0,	0,	0 },
	{ INTEGER, OFFSET(struct member_OACS_4, checkpointSize), 0, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_F,	3,	0,	0 },
	{ INTEGER, OFFSET(struct member_OACS_4, windowsize), 1, FL_CONTEXT|FL_DEFAULT },
	{ ETAG, 0, 2, FL_CONTEXT },
	{ OBJECT, OFFSET(struct member_OACS_4, member_OACS_5), _ZConnectionDataOACS, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_PRefuseOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ SOBJECT, 0, _ZRefuseReasonOACS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_DataTransferSyntaxOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_OACS_DataTransferSyntax, parm), 0, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ConnectionDataOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_OACS_ConnectionData, offset), 0, 0 },
	{ ETAG, 0, 0, FL_CONTEXT },
	{ ANY, OFFSET(struct type_OACS_ConnectionData, un.open), -1, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_OACS_ConnectionData, un.recover), _ZSessionConnectionIdentifierOACS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_SessionConnectionIdentifierOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_OACS_SessionConnectionIdentifier, element_OACS_0), _ZCallingSSUserReferenceOACS, 0 },
	{ OBJECT, OFFSET(struct type_OACS_SessionConnectionIdentifier, element_OACS_1), _ZCommonReferenceOACS, 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_OACS_SessionConnectionIdentifier, element_OACS_2), _ZAdditionalReferenceInformationOACS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_CallingSSUserReferenceOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZSSAPAddressOACS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_CommonReferenceOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 23, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AdditionalReferenceInformationOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 20, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_SSAPAddressOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 20, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RefuseReasonOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_OACS_RefuseReason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AbortInformationOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_OACS_AbortInformation, member_OACS_6), _ZAbortReasonOACS, FL_CONTEXT|FL_OPTIONAL },
	{ BITSTRING, OFFSET(struct type_OACS_AbortInformation, reflectedParameter), 1, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AbortReasonOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_OACS_AbortReason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_PriorityOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_OACS_Priority, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_PConnectOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_OACS_PConnect), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_OACS_PConnect, member_OACS_0), _ZDataTransferSyntaxOACS, FL_CONTEXT },
	{ SET_START, OFFSET(struct type_OACS_PConnect, pUserData), 1, FL_CONTEXT },
	{ MEMALLOC, 0, sizeof (struct member_OACS_1), 0 },
	{ OPTL, OFFSET(struct member_OACS_1, optionals), 0, 0 },
	{ INTEGER, OFFSET(struct member_OACS_1, checkpointSize), 0, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	0,	0,	0 },
	{ INTEGER, OFFSET(struct member_OACS_1, windowSize), 1, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	3,	0,	0 },
	{ INTEGER, OFFSET(struct member_OACS_1, dialogueMode), 2, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	0,	0,	0 },
	{ ETAG, 0, 3, FL_CONTEXT },
	{ OBJECT, OFFSET(struct member_OACS_1, member_OACS_2), _ZConnectionDataOACS, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct member_OACS_1, applicationProtocol), 4, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	1,	0,	0 },
	{ INTEGER, OFFSET(struct member_OACS_1, protocolVersion), 5, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_PAcceptOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_OACS_PAccept), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_OACS_PAccept, member_OACS_3), _ZDataTransferSyntaxOACS, FL_CONTEXT },
	{ SET_START, OFFSET(struct type_OACS_PAccept, pUserData), 1, FL_CONTEXT },
	{ MEMALLOC, 0, sizeof (struct member_OACS_4), 0 },
	{ INTEGER, OFFSET(struct member_OACS_4, checkpointSize), 0, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	0,	0,	0 },
	{ INTEGER, OFFSET(struct member_OACS_4, windowsize), 1, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	3,	0,	0 },
	{ ETAG, 0, 2, FL_CONTEXT },
	{ OBJECT, OFFSET(struct member_OACS_4, member_OACS_5), _ZConnectionDataOACS, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_PRefuseOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_OACS_PRefuse), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT },
	{ SOBJECT, 0, _ZRefuseReasonOACS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_DataTransferSyntaxOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_OACS_DataTransferSyntax), 0 },
	{ INTEGER, OFFSET(struct type_OACS_DataTransferSyntax, parm), 0, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ConnectionDataOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_OACS_ConnectionData), 0 },
	{ SCTRL, OFFSET(struct type_OACS_ConnectionData, offset), 0, 0 },
	{ ETAG, 0, 0, FL_CONTEXT },
	{ ANY, OFFSET(struct type_OACS_ConnectionData, un.open), -1, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_OACS_ConnectionData, un.recover), _ZSessionConnectionIdentifierOACS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_SessionConnectionIdentifierOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_OACS_SessionConnectionIdentifier), 0 },
	{ OBJECT, OFFSET(struct type_OACS_SessionConnectionIdentifier, element_OACS_0), _ZCallingSSUserReferenceOACS, 0 },
	{ OBJECT, OFFSET(struct type_OACS_SessionConnectionIdentifier, element_OACS_1), _ZCommonReferenceOACS, 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_OACS_SessionConnectionIdentifier, element_OACS_2), _ZAdditionalReferenceInformationOACS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_CallingSSUserReferenceOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOBJECT, 0, _ZSSAPAddressOACS, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_CommonReferenceOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 23, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AdditionalReferenceInformationOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 20, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_SSAPAddressOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 20, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RefuseReasonOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_OACS_RefuseReason), 0 },
	{ INTEGER, OFFSET(struct type_OACS_RefuseReason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AbortInformationOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_OACS_AbortInformation), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_OACS_AbortInformation, member_OACS_6), _ZAbortReasonOACS, FL_CONTEXT|FL_OPTIONAL },
	{ BITSTRING, OFFSET(struct type_OACS_AbortInformation, reflectedParameter), 1, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AbortReasonOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_OACS_AbortReason), 0 },
	{ INTEGER, OFFSET(struct type_OACS_AbortReason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_PriorityOACS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_OACS_Priority), 0 },
	{ INTEGER, OFFSET(struct type_OACS_Priority, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static ptpe pt_PConnectOACS[] = {
	{ PE_START, 0, 0, 0, "PConnect" },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT, "DataTransferSyntax" },
	{ OBJECT, 0, _ZDataTransferSyntaxOACS, FL_CONTEXT, "DataTransferSyntax" },
	{ SET_START, -1, 1, FL_CONTEXT, "pUserData" },
	{ INTEGER, -1, 0, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "checkpointSize" },
	{ DFLT_B,	0,	0,	0 },
	{ INTEGER, -1, 1, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "windowSize" },
	{ DFLT_B,	3,	0,	0 },
	{ INTEGER, -1, 2, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "dialogueMode" },
	{ DFLT_B,	0,	0,	0 },
	{ ETAG, -1, 3, FL_CONTEXT, "ConnectionData" },
	{ OBJECT, 0, _ZConnectionDataOACS, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 4, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "applicationProtocol" },
	{ DFLT_B,	1,	0,	0 },
	{ INTEGER, -1, 5, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "protocolVersion" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_PAcceptOACS[] = {
	{ PE_START, 0, 0, 0, "PAccept" },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT, "DataTransferSyntax" },
	{ OBJECT, 0, _ZDataTransferSyntaxOACS, FL_CONTEXT, "DataTransferSyntax" },
	{ SET_START, -1, 1, FL_CONTEXT, "pUserData" },
	{ INTEGER, -1, 0, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "checkpointSize" },
	{ DFLT_B,	0,	0,	0 },
	{ INTEGER, -1, 1, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "windowsize" },
	{ DFLT_B,	3,	0,	0 },
	{ ETAG, -1, 2, FL_CONTEXT, "ConnectionData" },
	{ OBJECT, 0, _ZConnectionDataOACS, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_PRefuseOACS[] = {
	{ PE_START, 0, 0, 0, "PRefuse" },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT, "RefuseReason" },
	{ SOBJECT, 0, _ZRefuseReasonOACS, FL_CONTEXT, "RefuseReason" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_DataTransferSyntaxOACS[] = {
	{ PE_START, 0, 0, 0, "DataTransferSyntax" },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 0, FL_CONTEXT, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ConnectionDataOACS[] = {
	{ PE_START, 0, 0, 0, "ConnectionData" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ ETAG, -1, 0, FL_CONTEXT, "open" },
	{ ANY, -1, -1, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 1, FL_CONTEXT, "recover" },
	{ OBJECT, 0, _ZSessionConnectionIdentifierOACS, FL_CONTEXT, "recover" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_SessionConnectionIdentifierOACS[] = {
	{ PE_START, 0, 0, 0, "SessionConnectionIdentifier" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ OBJECT, 0, _ZCallingSSUserReferenceOACS, 0, "CallingSSUserReference" },
	{ OBJECT, 0, _ZCommonReferenceOACS, 0, "CommonReference" },
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_OPTIONAL, "AdditionalReferenceInformation" },
	{ OBJECT, 0, _ZAdditionalReferenceInformationOACS, FL_CONTEXT|FL_OPTIONAL, "AdditionalReferenceInformation" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_CallingSSUserReferenceOACS[] = {
	{ PE_START, 0, 0, 0, "CallingSSUserReference" },
	{ SOBJECT, 0, _ZSSAPAddressOACS, 0, "SSAPAddress" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_CommonReferenceOACS[] = {
	{ PE_START, 0, 0, 0, "CommonReference" },
	{ SOCTETSTRING, 0, 23, 0, "UTCTime" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AdditionalReferenceInformationOACS[] = {
	{ PE_START, 0, 0, 0, "AdditionalReferenceInformation" },
	{ SOCTETSTRING, 0, 20, 0, "T61String" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_SSAPAddressOACS[] = {
	{ PE_START, 0, 0, 0, "SSAPAddress" },
	{ SOCTETSTRING, 0, 20, 0, "T61String" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RefuseReasonOACS[] = {
	{ PE_START, 0, 0, 0, "RefuseReason" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AbortInformationOACS[] = {
	{ PE_START, 0, 0, 0, "AbortInformation" },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_OPTIONAL, "AbortReason" },
	{ OBJECT, 0, _ZAbortReasonOACS, FL_CONTEXT|FL_OPTIONAL, "AbortReason" },
	{ BITSTRING, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "reflectedParameter" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AbortReasonOACS[] = {
	{ PE_START, 0, 0, 0, "AbortReason" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_PriorityOACS[] = {
	{ PE_START, 0, 0, 0, "Priority" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static tpe *etabl[] = {
	et_PConnectOACS,
	et_PAcceptOACS,
	et_PRefuseOACS,
	et_DataTransferSyntaxOACS,
	et_ConnectionDataOACS,
	et_SessionConnectionIdentifierOACS,
	et_CallingSSUserReferenceOACS,
	et_CommonReferenceOACS,
	et_AdditionalReferenceInformationOACS,
	et_SSAPAddressOACS,
	et_RefuseReasonOACS,
	et_AbortInformationOACS,
	et_AbortReasonOACS,
	et_PriorityOACS,
	};

static tpe *dtabl[] = {
	dt_PConnectOACS,
	dt_PAcceptOACS,
	dt_PRefuseOACS,
	dt_DataTransferSyntaxOACS,
	dt_ConnectionDataOACS,
	dt_SessionConnectionIdentifierOACS,
	dt_CallingSSUserReferenceOACS,
	dt_CommonReferenceOACS,
	dt_AdditionalReferenceInformationOACS,
	dt_SSAPAddressOACS,
	dt_RefuseReasonOACS,
	dt_AbortInformationOACS,
	dt_AbortReasonOACS,
	dt_PriorityOACS,
	};

static ptpe *ptabl[] = {
	pt_PConnectOACS,
	pt_PAcceptOACS,
	pt_PRefuseOACS,
	pt_DataTransferSyntaxOACS,
	pt_ConnectionDataOACS,
	pt_SessionConnectionIdentifierOACS,
	pt_CallingSSUserReferenceOACS,
	pt_CommonReferenceOACS,
	pt_AdditionalReferenceInformationOACS,
	pt_SSAPAddressOACS,
	pt_RefuseReasonOACS,
	pt_AbortInformationOACS,
	pt_AbortReasonOACS,
	pt_PriorityOACS,
	};


/* Pointer table 0 entries */
static caddr_t _ZptrtabOACS[] = {
    (caddr_t ) 0,
};

modtyp _ZOACS_mod = {
	"OACS",
	14,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabOACS,
	};


#ifdef	lint

#undef encode_OACS_PConnect
int	encode_OACS_PConnect(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_PConnect *parm;
{
  return (enc_f(_ZPConnectOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_PConnect
int	decode_OACS_PConnect(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_PConnect **parm;
{
  return (dec_f(_ZPConnectOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_PConnect
/* ARGSUSED */
int	print_OACS_PConnect(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_PConnect *parm;
{
  return (prnt_f(_ZPConnectOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_PAccept
int	encode_OACS_PAccept(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_PAccept *parm;
{
  return (enc_f(_ZPAcceptOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_PAccept
int	decode_OACS_PAccept(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_PAccept **parm;
{
  return (dec_f(_ZPAcceptOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_PAccept
/* ARGSUSED */
int	print_OACS_PAccept(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_PAccept *parm;
{
  return (prnt_f(_ZPAcceptOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_PRefuse
int	encode_OACS_PRefuse(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_PRefuse *parm;
{
  return (enc_f(_ZPRefuseOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_PRefuse
int	decode_OACS_PRefuse(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_PRefuse **parm;
{
  return (dec_f(_ZPRefuseOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_PRefuse
/* ARGSUSED */
int	print_OACS_PRefuse(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_PRefuse *parm;
{
  return (prnt_f(_ZPRefuseOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_DataTransferSyntax
int	encode_OACS_DataTransferSyntax(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_DataTransferSyntax *parm;
{
  return (enc_f(_ZDataTransferSyntaxOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_DataTransferSyntax
int	decode_OACS_DataTransferSyntax(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_DataTransferSyntax **parm;
{
  return (dec_f(_ZDataTransferSyntaxOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_DataTransferSyntax
/* ARGSUSED */
int	print_OACS_DataTransferSyntax(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_DataTransferSyntax *parm;
{
  return (prnt_f(_ZDataTransferSyntaxOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_ConnectionData
int	encode_OACS_ConnectionData(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_ConnectionData *parm;
{
  return (enc_f(_ZConnectionDataOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_ConnectionData
int	decode_OACS_ConnectionData(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_ConnectionData **parm;
{
  return (dec_f(_ZConnectionDataOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_ConnectionData
/* ARGSUSED */
int	print_OACS_ConnectionData(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_ConnectionData *parm;
{
  return (prnt_f(_ZConnectionDataOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_SessionConnectionIdentifier
int	encode_OACS_SessionConnectionIdentifier(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_SessionConnectionIdentifier *parm;
{
  return (enc_f(_ZSessionConnectionIdentifierOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_SessionConnectionIdentifier
int	decode_OACS_SessionConnectionIdentifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_SessionConnectionIdentifier **parm;
{
  return (dec_f(_ZSessionConnectionIdentifierOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_SessionConnectionIdentifier
/* ARGSUSED */
int	print_OACS_SessionConnectionIdentifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_SessionConnectionIdentifier *parm;
{
  return (prnt_f(_ZSessionConnectionIdentifierOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_CallingSSUserReference
int	encode_OACS_CallingSSUserReference(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_CallingSSUserReference *parm;
{
  return (enc_f(_ZCallingSSUserReferenceOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_CallingSSUserReference
int	decode_OACS_CallingSSUserReference(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_CallingSSUserReference **parm;
{
  return (dec_f(_ZCallingSSUserReferenceOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_CallingSSUserReference
/* ARGSUSED */
int	print_OACS_CallingSSUserReference(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_CallingSSUserReference *parm;
{
  return (prnt_f(_ZCallingSSUserReferenceOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_CommonReference
int	encode_OACS_CommonReference(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_CommonReference *parm;
{
  return (enc_f(_ZCommonReferenceOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_CommonReference
int	decode_OACS_CommonReference(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_CommonReference **parm;
{
  return (dec_f(_ZCommonReferenceOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_CommonReference
/* ARGSUSED */
int	print_OACS_CommonReference(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_CommonReference *parm;
{
  return (prnt_f(_ZCommonReferenceOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_AdditionalReferenceInformation
int	encode_OACS_AdditionalReferenceInformation(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_AdditionalReferenceInformation *parm;
{
  return (enc_f(_ZAdditionalReferenceInformationOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_AdditionalReferenceInformation
int	decode_OACS_AdditionalReferenceInformation(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_AdditionalReferenceInformation **parm;
{
  return (dec_f(_ZAdditionalReferenceInformationOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_AdditionalReferenceInformation
/* ARGSUSED */
int	print_OACS_AdditionalReferenceInformation(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_AdditionalReferenceInformation *parm;
{
  return (prnt_f(_ZAdditionalReferenceInformationOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_SSAPAddress
int	encode_OACS_SSAPAddress(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_SSAPAddress *parm;
{
  return (enc_f(_ZSSAPAddressOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_SSAPAddress
int	decode_OACS_SSAPAddress(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_SSAPAddress **parm;
{
  return (dec_f(_ZSSAPAddressOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_SSAPAddress
/* ARGSUSED */
int	print_OACS_SSAPAddress(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_SSAPAddress *parm;
{
  return (prnt_f(_ZSSAPAddressOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_RefuseReason
int	encode_OACS_RefuseReason(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_RefuseReason *parm;
{
  return (enc_f(_ZRefuseReasonOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_RefuseReason
int	decode_OACS_RefuseReason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_RefuseReason **parm;
{
  return (dec_f(_ZRefuseReasonOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_RefuseReason
/* ARGSUSED */
int	print_OACS_RefuseReason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_RefuseReason *parm;
{
  return (prnt_f(_ZRefuseReasonOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_AbortInformation
int	encode_OACS_AbortInformation(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_AbortInformation *parm;
{
  return (enc_f(_ZAbortInformationOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_AbortInformation
int	decode_OACS_AbortInformation(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_AbortInformation **parm;
{
  return (dec_f(_ZAbortInformationOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_AbortInformation
/* ARGSUSED */
int	print_OACS_AbortInformation(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_AbortInformation *parm;
{
  return (prnt_f(_ZAbortInformationOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_AbortReason
int	encode_OACS_AbortReason(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_AbortReason *parm;
{
  return (enc_f(_ZAbortReasonOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_AbortReason
int	decode_OACS_AbortReason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_AbortReason **parm;
{
  return (dec_f(_ZAbortReasonOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_AbortReason
/* ARGSUSED */
int	print_OACS_AbortReason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_AbortReason *parm;
{
  return (prnt_f(_ZAbortReasonOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#undef encode_OACS_Priority
int	encode_OACS_Priority(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_OACS_Priority *parm;
{
  return (enc_f(_ZPriorityOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_OACS_Priority
int	decode_OACS_Priority(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_Priority **parm;
{
  return (dec_f(_ZPriorityOACS, &_ZOACS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_OACS_Priority
/* ARGSUSED */
int	print_OACS_Priority(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_OACS_Priority *parm;
{
  return (prnt_f(_ZPriorityOACS, &_ZOACS_mod, pe, top, len, buffer));
}

#endif	/* lint */
