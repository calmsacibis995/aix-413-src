static char sccsid[] = "@(#)48	1.3  src/tcpip/usr/lib/libisode/RTS_tables.c, isodelib7, tcpip411, GOLD410 4/5/93 13:36:27";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: AOFFSET OFFSET
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/RTS_tables.c
 */

#include <stdio.h>
#include "RTS-types.h"


# line 33 "rts.py"

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/RTS_tables.c,v 1.2 93/02/05 17:02:34 snmp Exp $";
#endif

#include <stdio.h>
#include <isode/rtpkt.h>


int	rtsap_priority;

/*  */

#define OFFSET(t,f)	((int ) &(((t *)0)->f))


#define AOFFSET(t,f)	((int ) (((t *)0)->f))

static tpe et_RTSE_apdusRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_RTS_RTSE__apdus, offset), 0, 0 },
	{ IMP_OBJ, 0, 16, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTSE__apdus, un.rtorq__apdu), _ZRTORQapduRTS, FL_CONTEXT },
	{ IMP_OBJ, 0, 17, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTSE__apdus, un.rtoac__apdu), _ZRTOACapduRTS, FL_CONTEXT },
	{ IMP_OBJ, 0, 18, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTSE__apdus, un.rtorj__apdu), _ZRTORJapduRTS, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTSE__apdus, un.rttp__apdu), _ZRTTPapduRTS, 0 },
	{ OBJECT, OFFSET(struct type_RTS_RTSE__apdus, un.rttr__apdu), _ZRTTRapduRTS, 0 },
	{ IMP_OBJ, 0, 22, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTSE__apdus, un.rtab__apdu), _ZRTABapduRTS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RTORQapduRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ OPTL, OFFSET(struct type_RTS_RTORQapdu, optionals), 0, 0 },
	{ DFLT_F,	0,	0,	0 },
	{ INTEGER, OFFSET(struct type_RTS_RTORQapdu, checkpointSize), 0, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_F,	3,	0,	0 },
	{ INTEGER, OFFSET(struct type_RTS_RTORQapdu, windowSize), 1, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_F,	0,	0,	0 },
	{ INTEGER, OFFSET(struct type_RTS_RTORQapdu, dialogueMode), 2, FL_CONTEXT|FL_DEFAULT },
	{ ETAG, 0, 3, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTORQapdu, connectionDataRQ), _ZConnectionDataRTS, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_RTS_RTORQapdu, applicationProtocol), 4, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RTOACapduRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ DFLT_F,	0,	0,	0 },
	{ INTEGER, OFFSET(struct type_RTS_RTOACapdu, checkpointSize), 0, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_F,	3,	0,	0 },
	{ INTEGER, OFFSET(struct type_RTS_RTOACapdu, windowSize), 1, FL_CONTEXT|FL_DEFAULT },
	{ ETAG, 0, 2, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTOACapdu, connectionDataAC), _ZConnectionDataRTS, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RTORJapduRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ EXTOBJ, OFFSET(struct type_RTS_RTORJapdu, refuseReason), _ZRefuseReasonOACS, FL_CONTEXT|FL_OPTIONAL },
	{ EXTMOD, 0, 0, 0 },
	{ ETAG, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ ANY, OFFSET(struct type_RTS_RTORJapdu, userDataRJ), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RTTPapduRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_RTS_RTTPapdu, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RTTRapduRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 4, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_RTABapduRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_RTS_RTABapdu, abortReason), _ZAbortReasonRTS, FL_CONTEXT|FL_OPTIONAL },
	{ BITSTRING, OFFSET(struct type_RTS_RTABapdu, reflectedParameter), 1, FL_CONTEXT|FL_OPTIONAL },
	{ ETAG, 0, 2, FL_CONTEXT|FL_OPTIONAL },
	{ ANY, OFFSET(struct type_RTS_RTABapdu, userdataAB), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_ConnectionDataRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_RTS_ConnectionData, offset), 0, 0 },
	{ ETAG, 0, 0, FL_CONTEXT },
	{ ANY, OFFSET(struct type_RTS_ConnectionData, un.open), -1, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_ConnectionData, un.recover), _ZSessionConnectionIdentifierRTS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_SessionConnectionIdentifierRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ OBJECT, OFFSET(struct type_RTS_SessionConnectionIdentifier, element_RTS_0), _ZCallingSSuserReferenceRTS, 0 },
	{ OBJECT, OFFSET(struct type_RTS_SessionConnectionIdentifier, element_RTS_1), _ZCommonReferenceRTS, 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_RTS_SessionConnectionIdentifier, element_RTS_2), _ZAdditionalReferenceInformationRTS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_CallingSSuserReferenceRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ SCTRL, OFFSET(struct type_RTS_CallingSSuserReference, offset), 0, 0 },
	{ OCTETSTRING, OFFSET(struct type_RTS_CallingSSuserReference, un.choice_RTS_0), 20, 0 },
	{ OCTETSTRING, OFFSET(struct type_RTS_CallingSSuserReference, un.choice_RTS_1), 4, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_CommonReferenceRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 23, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AdditionalReferenceInformationRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 20, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe et_AbortReasonRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ INTEGER, OFFSET(struct type_RTS_AbortReason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RTSE_apdusRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_RTS_RTSE__apdus), 0 },
	{ SCTRL, OFFSET(struct type_RTS_RTSE__apdus, offset), 0, 0 },
	{ IMP_OBJ, 0, 16, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTSE__apdus, un.rtorq__apdu), _ZRTORQapduRTS, FL_CONTEXT },
	{ IMP_OBJ, 0, 17, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTSE__apdus, un.rtoac__apdu), _ZRTOACapduRTS, FL_CONTEXT },
	{ IMP_OBJ, 0, 18, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTSE__apdus, un.rtorj__apdu), _ZRTORJapduRTS, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTSE__apdus, un.rttp__apdu), _ZRTTPapduRTS, 0 },
	{ OBJECT, OFFSET(struct type_RTS_RTSE__apdus, un.rttr__apdu), _ZRTTRapduRTS, 0 },
	{ IMP_OBJ, 0, 22, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTSE__apdus, un.rtab__apdu), _ZRTABapduRTS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RTORQapduRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_RTS_RTORQapdu), 0 },
	{ OPTL, OFFSET(struct type_RTS_RTORQapdu, optionals), 0, 0 },
	{ INTEGER, OFFSET(struct type_RTS_RTORQapdu, checkpointSize), 0, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	0,	0,	0 },
	{ INTEGER, OFFSET(struct type_RTS_RTORQapdu, windowSize), 1, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	3,	0,	0 },
	{ INTEGER, OFFSET(struct type_RTS_RTORQapdu, dialogueMode), 2, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	0,	0,	0 },
	{ ETAG, 0, 3, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTORQapdu, connectionDataRQ), _ZConnectionDataRTS, FL_UNIVERSAL },
	{ INTEGER, OFFSET(struct type_RTS_RTORQapdu, applicationProtocol), 4, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RTOACapduRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_RTS_RTOACapdu), 0 },
	{ INTEGER, OFFSET(struct type_RTS_RTOACapdu, checkpointSize), 0, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	0,	0,	0 },
	{ INTEGER, OFFSET(struct type_RTS_RTOACapdu, windowSize), 1, FL_CONTEXT|FL_DEFAULT },
	{ DFLT_B,	3,	0,	0 },
	{ ETAG, 0, 2, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_RTOACapdu, connectionDataAC), _ZConnectionDataRTS, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RTORJapduRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_RTS_RTORJapdu), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ EXTOBJ, OFFSET(struct type_RTS_RTORJapdu, refuseReason), _ZRefuseReasonOACS, FL_CONTEXT|FL_OPTIONAL },
	{ EXTMOD, 0, 0, 0 },
	{ ETAG, 0, 1, FL_CONTEXT|FL_OPTIONAL },
	{ ANY, OFFSET(struct type_RTS_RTORJapdu, userDataRJ), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RTTPapduRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_RTS_RTTPapdu), 0 },
	{ INTEGER, OFFSET(struct type_RTS_RTTPapdu, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RTTRapduRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 4, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_RTABapduRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSET_START, 0, 17, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_RTS_RTABapdu), 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_RTS_RTABapdu, abortReason), _ZAbortReasonRTS, FL_CONTEXT|FL_OPTIONAL },
	{ BITSTRING, OFFSET(struct type_RTS_RTABapdu, reflectedParameter), 1, FL_CONTEXT|FL_OPTIONAL },
	{ ETAG, 0, 2, FL_CONTEXT|FL_OPTIONAL },
	{ ANY, OFFSET(struct type_RTS_RTABapdu, userdataAB), -1, FL_UNIVERSAL|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_ConnectionDataRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_RTS_ConnectionData), 0 },
	{ SCTRL, OFFSET(struct type_RTS_ConnectionData, offset), 0, 0 },
	{ ETAG, 0, 0, FL_CONTEXT },
	{ ANY, OFFSET(struct type_RTS_ConnectionData, un.open), -1, FL_UNIVERSAL },
	{ IMP_OBJ, 0, 1, FL_CONTEXT },
	{ OBJECT, OFFSET(struct type_RTS_ConnectionData, un.recover), _ZSessionConnectionIdentifierRTS, FL_CONTEXT },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_SessionConnectionIdentifierRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SSEQ_START, 0, 16, FL_UNIVERSAL },
	{ MEMALLOC, 0, sizeof (struct type_RTS_SessionConnectionIdentifier), 0 },
	{ OBJECT, OFFSET(struct type_RTS_SessionConnectionIdentifier, element_RTS_0), _ZCallingSSuserReferenceRTS, 0 },
	{ OBJECT, OFFSET(struct type_RTS_SessionConnectionIdentifier, element_RTS_1), _ZCommonReferenceRTS, 0 },
	{ IMP_OBJ, 0, 0, FL_CONTEXT|FL_OPTIONAL },
	{ OBJECT, OFFSET(struct type_RTS_SessionConnectionIdentifier, element_RTS_2), _ZAdditionalReferenceInformationRTS, FL_CONTEXT|FL_OPTIONAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_CallingSSuserReferenceRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SCHOICE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_RTS_CallingSSuserReference), 0 },
	{ SCTRL, OFFSET(struct type_RTS_CallingSSuserReference, offset), 0, 0 },
	{ OCTETSTRING, OFFSET(struct type_RTS_CallingSSuserReference, un.choice_RTS_0), 20, 0 },
	{ OCTETSTRING, OFFSET(struct type_RTS_CallingSSuserReference, un.choice_RTS_1), 4, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_CommonReferenceRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 23, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AdditionalReferenceInformationRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ SOCTETSTRING, 0, 20, 0 },
	{ PE_END, 0, 0, 0 }
	};

static tpe dt_AbortReasonRTS[] = {
	{ PE_START, 0, 0, 0 },
	{ MEMALLOC, 0, sizeof (struct type_RTS_AbortReason), 0 },
	{ INTEGER, OFFSET(struct type_RTS_AbortReason, parm), 2, FL_UNIVERSAL },
	{ PE_END, 0, 0, 0 }
	};

static ptpe pt_RTSE_apdusRTS[] = {
	{ PE_START, 0, 0, 0, "RTSE-apdus" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ IMP_OBJ, -1, 16, FL_CONTEXT, "rtorq-apdu" },
	{ OBJECT, 0, _ZRTORQapduRTS, FL_CONTEXT, "rtorq-apdu" },
	{ IMP_OBJ, -1, 17, FL_CONTEXT, "rtoac-apdu" },
	{ OBJECT, 0, _ZRTOACapduRTS, FL_CONTEXT, "rtoac-apdu" },
	{ IMP_OBJ, -1, 18, FL_CONTEXT, "rtorj-apdu" },
	{ OBJECT, 0, _ZRTORJapduRTS, FL_CONTEXT, "rtorj-apdu" },
	{ OBJECT, 0, _ZRTTPapduRTS, 0, "rttp-apdu" },
	{ OBJECT, 0, _ZRTTRapduRTS, 0, "rttr-apdu" },
	{ IMP_OBJ, -1, 22, FL_CONTEXT, "rtab-apdu" },
	{ OBJECT, 0, _ZRTABapduRTS, FL_CONTEXT, "rtab-apdu" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RTORQapduRTS[] = {
	{ PE_START, 0, 0, 0, "RTORQapdu" },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 0, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "checkpointSize" },
	{ DFLT_B,	0,	0,	0 },
	{ INTEGER, -1, 1, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "windowSize" },
	{ DFLT_B,	3,	0,	0 },
	{ INTEGER, -1, 2, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "dialogueMode" },
	{ DFLT_B,	0,	0,	0 },
	{ ETAG, -1, 3, FL_CONTEXT, "connectionDataRQ" },
	{ OBJECT, 0, _ZConnectionDataRTS, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 4, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "applicationProtocol" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RTOACapduRTS[] = {
	{ PE_START, 0, 0, 0, "RTOACapdu" },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULL},
	{ INTEGER, -1, 0, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "checkpointSize" },
	{ DFLT_B,	0,	0,	0 },
	{ INTEGER, -1, 1, FL_CONTEXT|FL_DEFAULT|FL_PRTAG, "windowSize" },
	{ DFLT_B,	3,	0,	0 },
	{ ETAG, -1, 2, FL_CONTEXT, "connectionDataAC" },
	{ OBJECT, 0, _ZConnectionDataRTS, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RTORJapduRTS[] = {
	{ PE_START, 0, 0, 0, "RTORJapdu" },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "refuseReason" },
	{ EXTOBJ, 0, _ZRefuseReasonOACS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "refuseReason" },
	{ EXTMOD, 0, 0, 0, NULL },
	{ ETAG, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "userDataRJ" },
	{ ANY, -1, -1, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RTTPapduRTS[] = {
	{ PE_START, 0, 0, 0, "RTTPapdu" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RTTRapduRTS[] = {
	{ PE_START, 0, 0, 0, "RTTRapdu" },
	{ SOCTETSTRING, -1, 4, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_RTABapduRTS[] = {
	{ PE_START, 0, 0, 0, "RTABapdu" },
	{ SSET_START, -1, 17, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "abortReason" },
	{ OBJECT, 0, _ZAbortReasonRTS, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "abortReason" },
	{ BITSTRING, -1, 1, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "reflectedParameter" },
	{ ETAG, -1, 2, FL_CONTEXT|FL_OPTIONAL|FL_PRTAG, "userdataAB" },
	{ ANY, -1, -1, FL_UNIVERSAL|FL_OPTIONAL, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_ConnectionDataRTS[] = {
	{ PE_START, 0, 0, 0, "ConnectionData" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ ETAG, -1, 0, FL_CONTEXT, "open" },
	{ ANY, -1, -1, FL_UNIVERSAL, NULL},
	{ IMP_OBJ, -1, 1, FL_CONTEXT, "recover" },
	{ OBJECT, 0, _ZSessionConnectionIdentifierRTS, FL_CONTEXT, "recover" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_SessionConnectionIdentifierRTS[] = {
	{ PE_START, 0, 0, 0, "SessionConnectionIdentifier" },
	{ SSEQ_START, -1, 16, FL_UNIVERSAL, NULL},
	{ OBJECT, 0, _ZCallingSSuserReferenceRTS, 0, "CallingSSuserReference" },
	{ OBJECT, 0, _ZCommonReferenceRTS, 0, "CommonReference" },
	{ IMP_OBJ, -1, 0, FL_CONTEXT|FL_OPTIONAL, "AdditionalReferenceInformation" },
	{ OBJECT, 0, _ZAdditionalReferenceInformationRTS, FL_CONTEXT|FL_OPTIONAL, "AdditionalReferenceInformation" },
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_CallingSSuserReferenceRTS[] = {
	{ PE_START, 0, 0, 0, "CallingSSuserReference" },
	{ SCHOICE_START, -1, 0, 0, NULL},
	{ SCTRL, 0, 0, 0, NULL },
	{ OCTETSTRING, 0, 20, 0, "T61String" },
	{ OCTETSTRING, -1, 4, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_CommonReferenceRTS[] = {
	{ PE_START, 0, 0, 0, "CommonReference" },
	{ SOCTETSTRING, 0, 23, 0, "UTCTime" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AdditionalReferenceInformationRTS[] = {
	{ PE_START, 0, 0, 0, "AdditionalReferenceInformation" },
	{ SOCTETSTRING, 0, 20, 0, "T61String" },
	{ PE_END, 0, 0, 0, NULL }
	};

static ptpe pt_AbortReasonRTS[] = {
	{ PE_START, 0, 0, 0, "AbortReason" },
	{ INTEGER, -1, 2, FL_UNIVERSAL, NULL},
	{ PE_END, 0, 0, 0, NULL }
	};

static tpe *etabl[] = {
	et_RTSE_apdusRTS,
	et_RTORQapduRTS,
	et_RTOACapduRTS,
	et_RTORJapduRTS,
	et_RTTPapduRTS,
	et_RTTRapduRTS,
	et_RTABapduRTS,
	et_ConnectionDataRTS,
	et_SessionConnectionIdentifierRTS,
	et_CallingSSuserReferenceRTS,
	et_CommonReferenceRTS,
	et_AdditionalReferenceInformationRTS,
	et_AbortReasonRTS,
	};

static tpe *dtabl[] = {
	dt_RTSE_apdusRTS,
	dt_RTORQapduRTS,
	dt_RTOACapduRTS,
	dt_RTORJapduRTS,
	dt_RTTPapduRTS,
	dt_RTTRapduRTS,
	dt_RTABapduRTS,
	dt_ConnectionDataRTS,
	dt_SessionConnectionIdentifierRTS,
	dt_CallingSSuserReferenceRTS,
	dt_CommonReferenceRTS,
	dt_AdditionalReferenceInformationRTS,
	dt_AbortReasonRTS,
	};

static ptpe *ptabl[] = {
	pt_RTSE_apdusRTS,
	pt_RTORQapduRTS,
	pt_RTOACapduRTS,
	pt_RTORJapduRTS,
	pt_RTTPapduRTS,
	pt_RTTRapduRTS,
	pt_RTABapduRTS,
	pt_ConnectionDataRTS,
	pt_SessionConnectionIdentifierRTS,
	pt_CallingSSuserReferenceRTS,
	pt_CommonReferenceRTS,
	pt_AdditionalReferenceInformationRTS,
	pt_AbortReasonRTS,
	};


/* Pointer table 1 entries */
static caddr_t _ZptrtabRTS[] = {
    (caddr_t ) &_ZOACS_mod,
};

modtyp _ZRTS_mod = {
	"RTS",
	13,
	etabl,
	dtabl,
	ptabl,
	0,
	0,
	0,
	_ZptrtabRTS,
	};


#ifdef	lint

#undef encode_RTS_RTSE__apdus
int	encode_RTS_RTSE__apdus(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_RTSE__apdus *parm;
{
  return (enc_f(_ZRTSE_apdusRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_RTSE__apdus
int	decode_RTS_RTSE__apdus(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTSE__apdus **parm;
{
  return (dec_f(_ZRTSE_apdusRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_RTSE__apdus
/* ARGSUSED */
int	print_RTS_RTSE__apdus(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTSE__apdus *parm;
{
  return (prnt_f(_ZRTSE_apdusRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#undef encode_RTS_RTORQapdu
int	encode_RTS_RTORQapdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_RTORQapdu *parm;
{
  return (enc_f(_ZRTORQapduRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_RTORQapdu
int	decode_RTS_RTORQapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTORQapdu **parm;
{
  return (dec_f(_ZRTORQapduRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_RTORQapdu
/* ARGSUSED */
int	print_RTS_RTORQapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTORQapdu *parm;
{
  return (prnt_f(_ZRTORQapduRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#undef encode_RTS_RTOACapdu
int	encode_RTS_RTOACapdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_RTOACapdu *parm;
{
  return (enc_f(_ZRTOACapduRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_RTOACapdu
int	decode_RTS_RTOACapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTOACapdu **parm;
{
  return (dec_f(_ZRTOACapduRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_RTOACapdu
/* ARGSUSED */
int	print_RTS_RTOACapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTOACapdu *parm;
{
  return (prnt_f(_ZRTOACapduRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#undef encode_RTS_RTORJapdu
int	encode_RTS_RTORJapdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_RTORJapdu *parm;
{
  return (enc_f(_ZRTORJapduRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_RTORJapdu
int	decode_RTS_RTORJapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTORJapdu **parm;
{
  return (dec_f(_ZRTORJapduRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_RTORJapdu
/* ARGSUSED */
int	print_RTS_RTORJapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTORJapdu *parm;
{
  return (prnt_f(_ZRTORJapduRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#undef encode_RTS_RTTPapdu
int	encode_RTS_RTTPapdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_RTTPapdu *parm;
{
  return (enc_f(_ZRTTPapduRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_RTTPapdu
int	decode_RTS_RTTPapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTTPapdu **parm;
{
  return (dec_f(_ZRTTPapduRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_RTTPapdu
/* ARGSUSED */
int	print_RTS_RTTPapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTTPapdu *parm;
{
  return (prnt_f(_ZRTTPapduRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#undef encode_RTS_RTTRapdu
int	encode_RTS_RTTRapdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_RTTRapdu *parm;
{
  return (enc_f(_ZRTTRapduRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_RTTRapdu
int	decode_RTS_RTTRapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTTRapdu **parm;
{
  return (dec_f(_ZRTTRapduRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_RTTRapdu
/* ARGSUSED */
int	print_RTS_RTTRapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTTRapdu *parm;
{
  return (prnt_f(_ZRTTRapduRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#undef encode_RTS_RTABapdu
int	encode_RTS_RTABapdu(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_RTABapdu *parm;
{
  return (enc_f(_ZRTABapduRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_RTABapdu
int	decode_RTS_RTABapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTABapdu **parm;
{
  return (dec_f(_ZRTABapduRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_RTABapdu
/* ARGSUSED */
int	print_RTS_RTABapdu(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_RTABapdu *parm;
{
  return (prnt_f(_ZRTABapduRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#undef encode_RTS_ConnectionData
int	encode_RTS_ConnectionData(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_ConnectionData *parm;
{
  return (enc_f(_ZConnectionDataRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_ConnectionData
int	decode_RTS_ConnectionData(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_ConnectionData **parm;
{
  return (dec_f(_ZConnectionDataRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_ConnectionData
/* ARGSUSED */
int	print_RTS_ConnectionData(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_ConnectionData *parm;
{
  return (prnt_f(_ZConnectionDataRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#undef encode_RTS_SessionConnectionIdentifier
int	encode_RTS_SessionConnectionIdentifier(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_SessionConnectionIdentifier *parm;
{
  return (enc_f(_ZSessionConnectionIdentifierRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_SessionConnectionIdentifier
int	decode_RTS_SessionConnectionIdentifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_SessionConnectionIdentifier **parm;
{
  return (dec_f(_ZSessionConnectionIdentifierRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_SessionConnectionIdentifier
/* ARGSUSED */
int	print_RTS_SessionConnectionIdentifier(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_SessionConnectionIdentifier *parm;
{
  return (prnt_f(_ZSessionConnectionIdentifierRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#undef encode_RTS_CallingSSuserReference
int	encode_RTS_CallingSSuserReference(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_CallingSSuserReference *parm;
{
  return (enc_f(_ZCallingSSuserReferenceRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_CallingSSuserReference
int	decode_RTS_CallingSSuserReference(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_CallingSSuserReference **parm;
{
  return (dec_f(_ZCallingSSuserReferenceRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_CallingSSuserReference
/* ARGSUSED */
int	print_RTS_CallingSSuserReference(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_CallingSSuserReference *parm;
{
  return (prnt_f(_ZCallingSSuserReferenceRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#undef encode_RTS_CommonReference
int	encode_RTS_CommonReference(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_CommonReference *parm;
{
  return (enc_f(_ZCommonReferenceRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_CommonReference
int	decode_RTS_CommonReference(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_CommonReference **parm;
{
  return (dec_f(_ZCommonReferenceRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_CommonReference
/* ARGSUSED */
int	print_RTS_CommonReference(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_CommonReference *parm;
{
  return (prnt_f(_ZCommonReferenceRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#undef encode_RTS_AdditionalReferenceInformation
int	encode_RTS_AdditionalReferenceInformation(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_AdditionalReferenceInformation *parm;
{
  return (enc_f(_ZAdditionalReferenceInformationRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_AdditionalReferenceInformation
int	decode_RTS_AdditionalReferenceInformation(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_AdditionalReferenceInformation **parm;
{
  return (dec_f(_ZAdditionalReferenceInformationRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_AdditionalReferenceInformation
/* ARGSUSED */
int	print_RTS_AdditionalReferenceInformation(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_AdditionalReferenceInformation *parm;
{
  return (prnt_f(_ZAdditionalReferenceInformationRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#undef encode_RTS_AbortReason
int	encode_RTS_AbortReason(pe, top, len, buffer, parm)
PE     *pe;
int	top,
	len;
char   *buffer;
struct type_RTS_AbortReason *parm;
{
  return (enc_f(_ZAbortReasonRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char *) parm));
}

#undef decode_RTS_AbortReason
int	decode_RTS_AbortReason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_AbortReason **parm;
{
  return (dec_f(_ZAbortReasonRTS, &_ZRTS_mod, pe, top, len, buffer,
		(char **) parm));
}

#undef print_RTS_AbortReason
/* ARGSUSED */
int	print_RTS_AbortReason(pe, top, len, buffer, parm)
PE	pe;
int	top,
       *len;
char  **buffer;
struct type_RTS_AbortReason *parm;
{
  return (prnt_f(_ZAbortReasonRTS, &_ZRTS_mod, pe, top, len, buffer));
}

#endif	/* lint */
