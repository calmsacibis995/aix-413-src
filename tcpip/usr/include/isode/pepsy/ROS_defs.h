/* @(#)98	1.4  src/tcpip/usr/include/isode/pepsy/ROS_defs.h, isodelib7, tcpip411, GOLD410 5/26/93 15:35:27 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: decode_ROS_Error decode_ROS_GeneralProblem decode_ROS_Invoke 
 *     decode_ROS_InvokeIDType decode_ROS_InvokeProblem decode_ROS_OPDU 
 *     decode_ROS_Operation decode_ROS_ROERapdu decode_ROS_ROIVapdu 
 *     decode_ROS_RORJapdu decode_ROS_RORSapdu decode_ROS_ROSEapdus 
 *     decode_ROS_Reject decode_ROS_ReturnError decode_ROS_ReturnErrorProblem 
 *     decode_ROS_ReturnResult decode_ROS_ReturnResultProblem encode_ROS_Error 
 *     encode_ROS_GeneralProblem encode_ROS_Invoke encode_ROS_InvokeIDType 
 *     encode_ROS_InvokeProblem encode_ROS_OPDU encode_ROS_Operation 
 *     encode_ROS_ROERapdu encode_ROS_ROIVapdu encode_ROS_RORJapdu 
 *     encode_ROS_RORSapdu encode_ROS_ROSEapdus encode_ROS_Reject 
 *     encode_ROS_ReturnError encode_ROS_ReturnErrorProblem 
 *     encode_ROS_ReturnResult encode_ROS_ReturnResultProblem print_ROS_Error 
 *     print_ROS_GeneralProblem print_ROS_Invoke print_ROS_InvokeIDType 
 *     print_ROS_InvokeProblem print_ROS_OPDU print_ROS_Operation 
 *     print_ROS_ROERapdu print_ROS_ROIVapdu print_ROS_RORJapdu 
 *     print_ROS_RORSapdu print_ROS_ROSEapdus print_ROS_Reject 
 *     print_ROS_ReturnError print_ROS_ReturnErrorProblem 
 *     print_ROS_ReturnResult print_ROS_ReturnResultProblem
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/ROS_defs.h
 */

#ifndef _ROS_defs_
#define _ROS_defs_

#include <isode/pepsy/ROS_pre_defs.h>

#ifndef	lint
#define encode_ROS_Operation(pe, top, len, buffer, parm) \
    enc_f(_ZOperationROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_Operation(pe, top, len, buffer, parm) \
    dec_f(_ZOperationROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_Operation(pe, top, len, buffer, parm) \
    prnt_f(_ZOperationROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_Operation_P    _ZOperationROS, &_ZROS_mod

#define encode_ROS_Error(pe, top, len, buffer, parm) \
    enc_f(_ZErrorROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_Error(pe, top, len, buffer, parm) \
    dec_f(_ZErrorROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_Error(pe, top, len, buffer, parm) \
    prnt_f(_ZErrorROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_Error_P    _ZErrorROS, &_ZROS_mod

#define encode_ROS_ROSEapdus(pe, top, len, buffer, parm) \
    enc_f(_ZROSEapdusROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_ROSEapdus(pe, top, len, buffer, parm) \
    dec_f(_ZROSEapdusROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_ROSEapdus(pe, top, len, buffer, parm) \
    prnt_f(_ZROSEapdusROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_ROSEapdus_P    _ZROSEapdusROS, &_ZROS_mod

#define encode_ROS_ROIVapdu(pe, top, len, buffer, parm) \
    enc_f(_ZROIVapduROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_ROIVapdu(pe, top, len, buffer, parm) \
    dec_f(_ZROIVapduROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_ROIVapdu(pe, top, len, buffer, parm) \
    prnt_f(_ZROIVapduROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_ROIVapdu_P    _ZROIVapduROS, &_ZROS_mod

#define encode_ROS_InvokeIDType(pe, top, len, buffer, parm) \
    enc_f(_ZInvokeIDTypeROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_InvokeIDType(pe, top, len, buffer, parm) \
    dec_f(_ZInvokeIDTypeROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_InvokeIDType(pe, top, len, buffer, parm) \
    prnt_f(_ZInvokeIDTypeROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_InvokeIDType_P    _ZInvokeIDTypeROS, &_ZROS_mod

#define encode_ROS_RORSapdu(pe, top, len, buffer, parm) \
    enc_f(_ZRORSapduROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_RORSapdu(pe, top, len, buffer, parm) \
    dec_f(_ZRORSapduROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_RORSapdu(pe, top, len, buffer, parm) \
    prnt_f(_ZRORSapduROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_RORSapdu_P    _ZRORSapduROS, &_ZROS_mod

#define encode_ROS_ROERapdu(pe, top, len, buffer, parm) \
    enc_f(_ZROERapduROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_ROERapdu(pe, top, len, buffer, parm) \
    dec_f(_ZROERapduROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_ROERapdu(pe, top, len, buffer, parm) \
    prnt_f(_ZROERapduROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_ROERapdu_P    _ZROERapduROS, &_ZROS_mod

#define encode_ROS_RORJapdu(pe, top, len, buffer, parm) \
    enc_f(_ZRORJapduROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_RORJapdu(pe, top, len, buffer, parm) \
    dec_f(_ZRORJapduROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_RORJapdu(pe, top, len, buffer, parm) \
    prnt_f(_ZRORJapduROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_RORJapdu_P    _ZRORJapduROS, &_ZROS_mod

#define encode_ROS_GeneralProblem(pe, top, len, buffer, parm) \
    enc_f(_ZGeneralProblemROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_GeneralProblem(pe, top, len, buffer, parm) \
    dec_f(_ZGeneralProblemROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_GeneralProblem(pe, top, len, buffer, parm) \
    prnt_f(_ZGeneralProblemROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_GeneralProblem_P    _ZGeneralProblemROS, &_ZROS_mod

#define encode_ROS_InvokeProblem(pe, top, len, buffer, parm) \
    enc_f(_ZInvokeProblemROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_InvokeProblem(pe, top, len, buffer, parm) \
    dec_f(_ZInvokeProblemROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_InvokeProblem(pe, top, len, buffer, parm) \
    prnt_f(_ZInvokeProblemROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_InvokeProblem_P    _ZInvokeProblemROS, &_ZROS_mod

#define encode_ROS_ReturnResultProblem(pe, top, len, buffer, parm) \
    enc_f(_ZReturnResultProblemROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_ReturnResultProblem(pe, top, len, buffer, parm) \
    dec_f(_ZReturnResultProblemROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_ReturnResultProblem(pe, top, len, buffer, parm) \
    prnt_f(_ZReturnResultProblemROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_ReturnResultProblem_P    _ZReturnResultProblemROS, &_ZROS_mod

#define encode_ROS_ReturnErrorProblem(pe, top, len, buffer, parm) \
    enc_f(_ZReturnErrorProblemROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_ReturnErrorProblem(pe, top, len, buffer, parm) \
    dec_f(_ZReturnErrorProblemROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_ReturnErrorProblem(pe, top, len, buffer, parm) \
    prnt_f(_ZReturnErrorProblemROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_ReturnErrorProblem_P    _ZReturnErrorProblemROS, &_ZROS_mod

#define encode_ROS_OPDU(pe, top, len, buffer, parm) \
    enc_f(_ZOPDUROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_OPDU(pe, top, len, buffer, parm) \
    dec_f(_ZOPDUROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_OPDU(pe, top, len, buffer, parm) \
    prnt_f(_ZOPDUROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_OPDU_P    _ZOPDUROS, &_ZROS_mod

#define encode_ROS_Invoke(pe, top, len, buffer, parm) \
    enc_f(_ZInvokeROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_Invoke(pe, top, len, buffer, parm) \
    dec_f(_ZInvokeROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_Invoke(pe, top, len, buffer, parm) \
    prnt_f(_ZInvokeROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_Invoke_P    _ZInvokeROS, &_ZROS_mod

#define encode_ROS_ReturnResult(pe, top, len, buffer, parm) \
    enc_f(_ZReturnResultROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_ReturnResult(pe, top, len, buffer, parm) \
    dec_f(_ZReturnResultROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_ReturnResult(pe, top, len, buffer, parm) \
    prnt_f(_ZReturnResultROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_ReturnResult_P    _ZReturnResultROS, &_ZROS_mod

#define encode_ROS_ReturnError(pe, top, len, buffer, parm) \
    enc_f(_ZReturnErrorROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_ReturnError(pe, top, len, buffer, parm) \
    dec_f(_ZReturnErrorROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_ReturnError(pe, top, len, buffer, parm) \
    prnt_f(_ZReturnErrorROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_ReturnError_P    _ZReturnErrorROS, &_ZROS_mod

#define encode_ROS_Reject(pe, top, len, buffer, parm) \
    enc_f(_ZRejectROS, &_ZROS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ROS_Reject(pe, top, len, buffer, parm) \
    dec_f(_ZRejectROS, &_ZROS_mod, pe, top, len, buffer, (char **) parm)

#define print_ROS_Reject(pe, top, len, buffer, parm) \
    prnt_f(_ZRejectROS, &_ZROS_mod, pe, top, len, buffer)
#define print_ROS_Reject_P    _ZRejectROS, &_ZROS_mod


#endif   /* lint */

#endif /* _ROS_defs_ */
