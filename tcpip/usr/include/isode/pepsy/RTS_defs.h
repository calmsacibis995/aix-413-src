/* @(#)46	1.4  src/tcpip/usr/include/isode/pepsy/RTS_defs.h, isodelib7, tcpip411, GOLD410 5/26/93 15:35:56 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: decode_RTS_AbortReason decode_RTS_AdditionalReferenceInformation 
 *    decode_RTS_CallingSSuserReference decode_RTS_CommonReference 
 *    decode_RTS_ConnectionData decode_RTS_RTABapdu decode_RTS_RTOACapdu 
 *    decode_RTS_RTORJapdu decode_RTS_RTORQapdu decode_RTS_RTSE__apdus 
 *    decode_RTS_RTTPapdu decode_RTS_RTTRapdu 
 *    decode_RTS_SessionConnectionIdentifier encode_RTS_AbortReason 
 *    encode_RTS_AdditionalReferenceInformation 
 *    encode_RTS_CallingSSuserReference encode_RTS_CommonReference 
 *    encode_RTS_ConnectionData encode_RTS_RTABapdu encode_RTS_RTOACapdu 
 *    encode_RTS_RTORJapdu encode_RTS_RTORQapdu encode_RTS_RTSE__apdus 
 *    encode_RTS_RTTPapdu encode_RTS_RTTRapdu 
 *    encode_RTS_SessionConnectionIdentifier print_RTS_AbortReason 
 *    print_RTS_AdditionalReferenceInformation print_RTS_CallingSSuserReference
 *    print_RTS_CommonReference print_RTS_ConnectionData print_RTS_RTABapdu 
 *    print_RTS_RTOACapdu print_RTS_RTORJapdu print_RTS_RTORQapdu 
 *    print_RTS_RTSE__apdus print_RTS_RTTPapdu print_RTS_RTTRapdu 
 *    print_RTS_SessionConnectionIdentifier
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/RTS_defs.h
 */

#ifndef _RTS_defs_
#define _RTS_defs_

#include <isode/pepsy/RTS_pre_defs.h>

#ifndef	lint
#define encode_RTS_RTSE__apdus(pe, top, len, buffer, parm) \
    enc_f(_ZRTSE_apdusRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_RTSE__apdus(pe, top, len, buffer, parm) \
    dec_f(_ZRTSE_apdusRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_RTSE__apdus(pe, top, len, buffer, parm) \
    prnt_f(_ZRTSE_apdusRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_RTSE__apdus_P    _ZRTSE_apdusRTS, &_ZRTS_mod

#define encode_RTS_RTORQapdu(pe, top, len, buffer, parm) \
    enc_f(_ZRTORQapduRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_RTORQapdu(pe, top, len, buffer, parm) \
    dec_f(_ZRTORQapduRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_RTORQapdu(pe, top, len, buffer, parm) \
    prnt_f(_ZRTORQapduRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_RTORQapdu_P    _ZRTORQapduRTS, &_ZRTS_mod

#define encode_RTS_RTOACapdu(pe, top, len, buffer, parm) \
    enc_f(_ZRTOACapduRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_RTOACapdu(pe, top, len, buffer, parm) \
    dec_f(_ZRTOACapduRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_RTOACapdu(pe, top, len, buffer, parm) \
    prnt_f(_ZRTOACapduRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_RTOACapdu_P    _ZRTOACapduRTS, &_ZRTS_mod

#define encode_RTS_RTORJapdu(pe, top, len, buffer, parm) \
    enc_f(_ZRTORJapduRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_RTORJapdu(pe, top, len, buffer, parm) \
    dec_f(_ZRTORJapduRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_RTORJapdu(pe, top, len, buffer, parm) \
    prnt_f(_ZRTORJapduRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_RTORJapdu_P    _ZRTORJapduRTS, &_ZRTS_mod

#define encode_RTS_RTTPapdu(pe, top, len, buffer, parm) \
    enc_f(_ZRTTPapduRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_RTTPapdu(pe, top, len, buffer, parm) \
    dec_f(_ZRTTPapduRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_RTTPapdu(pe, top, len, buffer, parm) \
    prnt_f(_ZRTTPapduRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_RTTPapdu_P    _ZRTTPapduRTS, &_ZRTS_mod

#define encode_RTS_RTTRapdu(pe, top, len, buffer, parm) \
    enc_f(_ZRTTRapduRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_RTTRapdu(pe, top, len, buffer, parm) \
    dec_f(_ZRTTRapduRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_RTTRapdu(pe, top, len, buffer, parm) \
    prnt_f(_ZRTTRapduRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_RTTRapdu_P    _ZRTTRapduRTS, &_ZRTS_mod

#define encode_RTS_RTABapdu(pe, top, len, buffer, parm) \
    enc_f(_ZRTABapduRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_RTABapdu(pe, top, len, buffer, parm) \
    dec_f(_ZRTABapduRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_RTABapdu(pe, top, len, buffer, parm) \
    prnt_f(_ZRTABapduRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_RTABapdu_P    _ZRTABapduRTS, &_ZRTS_mod

#define encode_RTS_ConnectionData(pe, top, len, buffer, parm) \
    enc_f(_ZConnectionDataRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_ConnectionData(pe, top, len, buffer, parm) \
    dec_f(_ZConnectionDataRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_ConnectionData(pe, top, len, buffer, parm) \
    prnt_f(_ZConnectionDataRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_ConnectionData_P    _ZConnectionDataRTS, &_ZRTS_mod

#define encode_RTS_SessionConnectionIdentifier(pe, top, len, buffer, parm) \
    enc_f(_ZSessionConnectionIdentifierRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_SessionConnectionIdentifier(pe, top, len, buffer, parm) \
    dec_f(_ZSessionConnectionIdentifierRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_SessionConnectionIdentifier(pe, top, len, buffer, parm) \
    prnt_f(_ZSessionConnectionIdentifierRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_SessionConnectionIdentifier_P    _ZSessionConnectionIdentifierRTS, &_ZRTS_mod

#define encode_RTS_CallingSSuserReference(pe, top, len, buffer, parm) \
    enc_f(_ZCallingSSuserReferenceRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_CallingSSuserReference(pe, top, len, buffer, parm) \
    dec_f(_ZCallingSSuserReferenceRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_CallingSSuserReference(pe, top, len, buffer, parm) \
    prnt_f(_ZCallingSSuserReferenceRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_CallingSSuserReference_P    _ZCallingSSuserReferenceRTS, &_ZRTS_mod

#define encode_RTS_CommonReference(pe, top, len, buffer, parm) \
    enc_f(_ZCommonReferenceRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_CommonReference(pe, top, len, buffer, parm) \
    dec_f(_ZCommonReferenceRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_CommonReference(pe, top, len, buffer, parm) \
    prnt_f(_ZCommonReferenceRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_CommonReference_P    _ZCommonReferenceRTS, &_ZRTS_mod

#define encode_RTS_AdditionalReferenceInformation(pe, top, len, buffer, parm) \
    enc_f(_ZAdditionalReferenceInformationRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_AdditionalReferenceInformation(pe, top, len, buffer, parm) \
    dec_f(_ZAdditionalReferenceInformationRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_AdditionalReferenceInformation(pe, top, len, buffer, parm) \
    prnt_f(_ZAdditionalReferenceInformationRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_AdditionalReferenceInformation_P    _ZAdditionalReferenceInformationRTS, &_ZRTS_mod

#define encode_RTS_AbortReason(pe, top, len, buffer, parm) \
    enc_f(_ZAbortReasonRTS, &_ZRTS_mod, pe, top, len, buffer, (char *) parm)

#define decode_RTS_AbortReason(pe, top, len, buffer, parm) \
    dec_f(_ZAbortReasonRTS, &_ZRTS_mod, pe, top, len, buffer, (char **) parm)

#define print_RTS_AbortReason(pe, top, len, buffer, parm) \
    prnt_f(_ZAbortReasonRTS, &_ZRTS_mod, pe, top, len, buffer)
#define print_RTS_AbortReason_P    _ZAbortReasonRTS, &_ZRTS_mod


#endif   /* lint */

#endif /* _RTS_defs_ */
