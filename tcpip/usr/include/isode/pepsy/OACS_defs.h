/* @(#)66	1.4  src/tcpip/usr/include/isode/pepsy/OACS_defs.h, isodelib7, tcpip411, GOLD410 5/26/93 15:34:35 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: encode_OACS_AbortInformation encode_OACS_AbortReason 
 *    encode_OACS_AdditionalReferenceInformation 
 *    encode_OACS_CallingSSUserReference encode_OACS_CommonReference 
 *    encode_OACS_ConnectionData encode_OACS_DataTransferSyntax 
 *    encode_OACS_PAccept encode_OACS_PConnect encode_OACS_PRefuse 
 *    encode_OACS_Priority encode_OACS_RefuseReason encode_OACS_SSAPAddress 
 *    encode_OACS_SessionConnectionIdentifier parse_OACS_AbortInformation 
 *    parse_OACS_AbortReason parse_OACS_AdditionalReferenceInformation 
 *    parse_OACS_CallingSSUserReference parse_OACS_CommonReference 
 *    parse_OACS_ConnectionData parse_OACS_DataTransferSyntax 
 *    parse_OACS_PAccept parse_OACS_PConnect parse_OACS_PRefuse 
 *    parse_OACS_Priority parse_OACS_RefuseReason parse_OACS_SSAPAddress 
 *    parse_OACS_SessionConnectionIdentifier print_OACS_AbortInformation 
 *    print_OACS_AbortReason print_OACS_AdditionalReferenceInformation 
 *    print_OACS_CallingSSUserReference print_OACS_CommonReference 
 *    print_OACS_ConnectionData print_OACS_DataTransferSyntax 
 *    print_OACS_PAccept print_OACS_PConnect print_OACS_PRefuse 
 *    print_OACS_Priority print_OACS_RefuseReason print_OACS_SSAPAddress 
 *    print_OACS_SessionConnectionIdentifier
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/OACS_defs.h
 */

#ifndef _OACS_defs_
#define _OACS_defs_

#include <isode/pepsy/OACS_pre_defs.h>

#ifndef	lint
#define encode_OACS_PConnect(pe, top, len, buffer, parm) \
    enc_f(_ZPConnectOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_PConnect(pe, top, len, buffer, parm) \
    dec_f(_ZPConnectOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_PConnect(pe, top, len, buffer, parm) \
    prnt_f(_ZPConnectOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_PConnect_P    _ZPConnectOACS, &_ZOACS_mod

#define encode_OACS_PAccept(pe, top, len, buffer, parm) \
    enc_f(_ZPAcceptOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_PAccept(pe, top, len, buffer, parm) \
    dec_f(_ZPAcceptOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_PAccept(pe, top, len, buffer, parm) \
    prnt_f(_ZPAcceptOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_PAccept_P    _ZPAcceptOACS, &_ZOACS_mod

#define encode_OACS_PRefuse(pe, top, len, buffer, parm) \
    enc_f(_ZPRefuseOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_PRefuse(pe, top, len, buffer, parm) \
    dec_f(_ZPRefuseOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_PRefuse(pe, top, len, buffer, parm) \
    prnt_f(_ZPRefuseOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_PRefuse_P    _ZPRefuseOACS, &_ZOACS_mod

#define encode_OACS_DataTransferSyntax(pe, top, len, buffer, parm) \
    enc_f(_ZDataTransferSyntaxOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_DataTransferSyntax(pe, top, len, buffer, parm) \
    dec_f(_ZDataTransferSyntaxOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_DataTransferSyntax(pe, top, len, buffer, parm) \
    prnt_f(_ZDataTransferSyntaxOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_DataTransferSyntax_P    _ZDataTransferSyntaxOACS, &_ZOACS_mod

#define encode_OACS_ConnectionData(pe, top, len, buffer, parm) \
    enc_f(_ZConnectionDataOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_ConnectionData(pe, top, len, buffer, parm) \
    dec_f(_ZConnectionDataOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_ConnectionData(pe, top, len, buffer, parm) \
    prnt_f(_ZConnectionDataOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_ConnectionData_P    _ZConnectionDataOACS, &_ZOACS_mod

#define encode_OACS_SessionConnectionIdentifier(pe, top, len, buffer, parm) \
    enc_f(_ZSessionConnectionIdentifierOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_SessionConnectionIdentifier(pe, top, len, buffer, parm) \
    dec_f(_ZSessionConnectionIdentifierOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_SessionConnectionIdentifier(pe, top, len, buffer, parm) \
    prnt_f(_ZSessionConnectionIdentifierOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_SessionConnectionIdentifier_P    _ZSessionConnectionIdentifierOACS, &_ZOACS_mod

#define encode_OACS_CallingSSUserReference(pe, top, len, buffer, parm) \
    enc_f(_ZCallingSSUserReferenceOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_CallingSSUserReference(pe, top, len, buffer, parm) \
    dec_f(_ZCallingSSUserReferenceOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_CallingSSUserReference(pe, top, len, buffer, parm) \
    prnt_f(_ZCallingSSUserReferenceOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_CallingSSUserReference_P    _ZCallingSSUserReferenceOACS, &_ZOACS_mod

#define encode_OACS_CommonReference(pe, top, len, buffer, parm) \
    enc_f(_ZCommonReferenceOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_CommonReference(pe, top, len, buffer, parm) \
    dec_f(_ZCommonReferenceOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_CommonReference(pe, top, len, buffer, parm) \
    prnt_f(_ZCommonReferenceOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_CommonReference_P    _ZCommonReferenceOACS, &_ZOACS_mod

#define encode_OACS_AdditionalReferenceInformation(pe, top, len, buffer, parm) \
    enc_f(_ZAdditionalReferenceInformationOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_AdditionalReferenceInformation(pe, top, len, buffer, parm) \
    dec_f(_ZAdditionalReferenceInformationOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_AdditionalReferenceInformation(pe, top, len, buffer, parm) \
    prnt_f(_ZAdditionalReferenceInformationOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_AdditionalReferenceInformation_P    _ZAdditionalReferenceInformationOACS, &_ZOACS_mod

#define encode_OACS_SSAPAddress(pe, top, len, buffer, parm) \
    enc_f(_ZSSAPAddressOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_SSAPAddress(pe, top, len, buffer, parm) \
    dec_f(_ZSSAPAddressOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_SSAPAddress(pe, top, len, buffer, parm) \
    prnt_f(_ZSSAPAddressOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_SSAPAddress_P    _ZSSAPAddressOACS, &_ZOACS_mod

#define encode_OACS_RefuseReason(pe, top, len, buffer, parm) \
    enc_f(_ZRefuseReasonOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_RefuseReason(pe, top, len, buffer, parm) \
    dec_f(_ZRefuseReasonOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_RefuseReason(pe, top, len, buffer, parm) \
    prnt_f(_ZRefuseReasonOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_RefuseReason_P    _ZRefuseReasonOACS, &_ZOACS_mod

#define encode_OACS_AbortInformation(pe, top, len, buffer, parm) \
    enc_f(_ZAbortInformationOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_AbortInformation(pe, top, len, buffer, parm) \
    dec_f(_ZAbortInformationOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_AbortInformation(pe, top, len, buffer, parm) \
    prnt_f(_ZAbortInformationOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_AbortInformation_P    _ZAbortInformationOACS, &_ZOACS_mod

#define encode_OACS_AbortReason(pe, top, len, buffer, parm) \
    enc_f(_ZAbortReasonOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_AbortReason(pe, top, len, buffer, parm) \
    dec_f(_ZAbortReasonOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_AbortReason(pe, top, len, buffer, parm) \
    prnt_f(_ZAbortReasonOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_AbortReason_P    _ZAbortReasonOACS, &_ZOACS_mod

#define encode_OACS_Priority(pe, top, len, buffer, parm) \
    enc_f(_ZPriorityOACS, &_ZOACS_mod, pe, top, len, buffer, (char *) parm)

#define parse_OACS_Priority(pe, top, len, buffer, parm) \
    dec_f(_ZPriorityOACS, &_ZOACS_mod, pe, top, len, buffer, (char **) parm)

#define print_OACS_Priority(pe, top, len, buffer, parm) \
    prnt_f(_ZPriorityOACS, &_ZOACS_mod, pe, top, len, buffer)
#define print_OACS_Priority_P    _ZPriorityOACS, &_ZOACS_mod


#endif   /* lint */

#endif /* _OACS_defs_ */
