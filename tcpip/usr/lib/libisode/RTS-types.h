/* @(#)45	1.3  src/tcpip/usr/lib/libisode/RTS-types.h, isodelib7, tcpip411, GOLD410 4/5/93 13:36:14 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: free_RTS_AbortReason free_RTS_CallingSSuserReference 
 *    free_RTS_ConnectionData free_RTS_RTABapdu free_RTS_RTOACapdu 
 *    free_RTS_RTORJapdu free_RTS_RTORQapdu free_RTS_RTSE__apdus 
 *    free_RTS_RTTPapdu free_RTS_SessionConnectionIdentifier
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/RTS-types.h
 */

/* automatically generated by pepsy 6.0 #108 (oilers.netmgmt.austin.ibm.com), do not edit! */

#ifndef	_module_RTS_defined_
#define	_module_RTS_defined_

#ifndef	PEPSY_VERSION
#define	PEPSY_VERSION		2
#endif

#include <isode/psap.h>
#include <isode/pepsy.h>
#include <isode/pepsy/UNIV-types.h>

#include <isode/pepsy/RTS_defs.h>

#include "OACS-types.h"

#define	type_RTS_RTTRapdu	qbuf
#define	free_RTS_RTTRapdu	qb_free

#define	type_RTS_CommonReference	type_UNIV_UTCTime
#define	free_RTS_CommonReference	free_UNIV_UTCTime

#define	type_RTS_AdditionalReferenceInformation	type_UNIV_T61String
#define	free_RTS_AdditionalReferenceInformation	free_UNIV_T61String

struct type_RTS_RTSE__apdus {
    int         offset;
#define	type_RTS_RTSE__apdus_rtorq__apdu	1
#define	type_RTS_RTSE__apdus_rtoac__apdu	2
#define	type_RTS_RTSE__apdus_rtorj__apdu	3
#define	type_RTS_RTSE__apdus_rttp__apdu	4
#define	type_RTS_RTSE__apdus_rttr__apdu	5
#define	type_RTS_RTSE__apdus_rtab__apdu	6

    union {
        struct type_RTS_RTORQapdu *rtorq__apdu;

        struct type_RTS_RTOACapdu *rtoac__apdu;

        struct type_RTS_RTORJapdu *rtorj__apdu;

        struct type_RTS_RTTPapdu *rttp__apdu;

        struct type_RTS_RTTRapdu *rttr__apdu;

        struct type_RTS_RTABapdu *rtab__apdu;
    }       un;
};
#define	free_RTS_RTSE__apdus(parm)\
	(void) fre_obj((char *) parm, _ZRTS_mod.md_dtab[_ZRTSE_apdusRTS], &_ZRTS_mod, 1)

struct type_RTS_RTORQapdu {
    integer     optionals;
#define	opt_RTS_RTORQapdu_applicationProtocol (000000001)

    integer     checkpointSize;

    integer     windowSize;

    integer     dialogueMode;
#define	int_RTS_dialogueMode_monologue	0
#define	int_RTS_dialogueMode_twa	1

    struct type_RTS_ConnectionData *connectionDataRQ;

    integer     applicationProtocol;
};
#define	free_RTS_RTORQapdu(parm)\
	(void) fre_obj((char *) parm, _ZRTS_mod.md_dtab[_ZRTORQapduRTS], &_ZRTS_mod, 1)

struct type_RTS_RTOACapdu {
    integer     checkpointSize;

    integer     windowSize;

    struct type_RTS_ConnectionData *connectionDataAC;
};
#define	free_RTS_RTOACapdu(parm)\
	(void) fre_obj((char *) parm, _ZRTS_mod.md_dtab[_ZRTOACapduRTS], &_ZRTS_mod, 1)

struct type_RTS_RTORJapdu {
    struct type_OACS_RefuseReason *refuseReason;

    PE      userDataRJ;
};
#define	free_RTS_RTORJapdu(parm)\
	(void) fre_obj((char *) parm, _ZRTS_mod.md_dtab[_ZRTORJapduRTS], &_ZRTS_mod, 1)

struct type_RTS_RTTPapdu {
    integer     parm;
};
#define	free_RTS_RTTPapdu(parm)\
	(void) fre_obj((char *) parm, _ZRTS_mod.md_dtab[_ZRTTPapduRTS], &_ZRTS_mod, 1)

struct type_RTS_RTABapdu {
    struct type_RTS_AbortReason *abortReason;

    PE      reflectedParameter;

    PE      userdataAB;
};
#define	free_RTS_RTABapdu(parm)\
	(void) fre_obj((char *) parm, _ZRTS_mod.md_dtab[_ZRTABapduRTS], &_ZRTS_mod, 1)

struct type_RTS_ConnectionData {
    int         offset;
#define	type_RTS_ConnectionData_open	1
#define	type_RTS_ConnectionData_recover	2

    union {
        PE      open;

        struct type_RTS_SessionConnectionIdentifier *recover;
    }       un;
};
#define	free_RTS_ConnectionData(parm)\
	(void) fre_obj((char *) parm, _ZRTS_mod.md_dtab[_ZConnectionDataRTS], &_ZRTS_mod, 1)

struct type_RTS_SessionConnectionIdentifier {
    struct type_RTS_CallingSSuserReference *element_RTS_0;

    struct type_RTS_CommonReference *element_RTS_1;

    struct type_RTS_AdditionalReferenceInformation *element_RTS_2;
};
#define	free_RTS_SessionConnectionIdentifier(parm)\
	(void) fre_obj((char *) parm, _ZRTS_mod.md_dtab[_ZSessionConnectionIdentifierRTS], &_ZRTS_mod, 1)

struct type_RTS_CallingSSuserReference {
    int         offset;
#define	type_RTS_CallingSSuserReference_1	1
#define	type_RTS_CallingSSuserReference_2	2

    union {
        struct	qbuf	*choice_RTS_0;

        struct qbuf *choice_RTS_1;
    }       un;
};
#define	free_RTS_CallingSSuserReference(parm)\
	(void) fre_obj((char *) parm, _ZRTS_mod.md_dtab[_ZCallingSSuserReferenceRTS], &_ZRTS_mod, 1)

struct type_RTS_AbortReason {
    integer     parm;
#define	int_RTS_AbortReason_localSystemProblem	0
#define	int_RTS_AbortReason_invalidParameter	1
#define	int_RTS_AbortReason_unrecognizedActivity	2
#define	int_RTS_AbortReason_temporaryProblem	3
#define	int_RTS_AbortReason_protocolError	4
#define	int_RTS_AbortReason_permanentProblem	5
#define	int_RTS_AbortReason_userError	6
#define	int_RTS_AbortReason_transferCompleted	7
};
#define	free_RTS_AbortReason(parm)\
	(void) fre_obj((char *) parm, _ZRTS_mod.md_dtab[_ZAbortReasonRTS], &_ZRTS_mod, 1)
#endif
