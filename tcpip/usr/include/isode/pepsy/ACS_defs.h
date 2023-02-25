/* @(#)53	1.4  src/tcpip/usr/include/isode/pepsy/ACS_defs.h, isodelib7, tcpip411, GOLD410 4/5/93 13:14:00 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: decode_ACS_AARE__apdu decode_ACS_AARQ__apdu decode_ACS_ABRT__apdu
 *    decode_ACS_ABRT__source decode_ACS_ACSE__apdu 
 *    decode_ACS_AE__invocation__id decode_ACS_AE__qualifier 
 *    decode_ACS_AE__title decode_ACS_AP__invocation__id decode_ACS_AP__title 
 *    decode_ACS_Application__context__name decode_ACS_Associate__result 
 *    decode_ACS_Associate__source__diagnostic 
 *    decode_ACS_Association__information decode_ACS_Implementation__data 
 *    decode_ACS_RLRE__apdu decode_ACS_RLRQ__apdu 
 *    decode_ACS_Release__request__reason decode_ACS_Release__response__reason 
 *    encode_ACS_AARE__apdu encode_ACS_AARQ__apdu encode_ACS_ABRT__apdu 
 *    encode_ACS_ABRT__source encode_ACS_ACSE__apdu 
 *    encode_ACS_AE__invocation__id encode_ACS_AE__qualifier 
 *    encode_ACS_AE__title encode_ACS_AP__invocation__id encode_ACS_AP__title 
 *    encode_ACS_Application__context__name encode_ACS_Associate__result 
 *    encode_ACS_Associate__source__diagnostic 
 *    encode_ACS_Association__information encode_ACS_Implementation__data 
 *    encode_ACS_RLRE__apdu encode_ACS_RLRQ__apdu 
 *    encode_ACS_Release__request__reason encode_ACS_Release__response__reason 
 *    print_ACS_AARE__apdu print_ACS_AARQ__apdu print_ACS_ABRT__apdu 
 *    print_ACS_ABRT__source print_ACS_ACSE__apdu print_ACS_AE__invocation__id 
 *    print_ACS_AE__qualifier print_ACS_AE__title print_ACS_AP__invocation__id 
 *    print_ACS_AP__title print_ACS_Application__context__name 
 *    print_ACS_Associate__result print_ACS_Associate__source__diagnostic 
 *    print_ACS_Association__information print_ACS_Implementation__data 
 *    print_ACS_RLRE__apdu print_ACS_RLRQ__apdu 
 *    print_ACS_Release__request__reason print_ACS_Release__response__reason
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/ACS_defs.h
 */

#ifndef _ACS_defs_
#define _ACS_defs_

#include <isode/pepsy/ACS_pre_defs.h>

#ifndef	lint
#define encode_ACS_ACSE__apdu(pe, top, len, buffer, parm) \
    enc_f(_ZACSE_apduACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_ACSE__apdu(pe, top, len, buffer, parm) \
    dec_f(_ZACSE_apduACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_ACSE__apdu(pe, top, len, buffer, parm) \
    prnt_f(_ZACSE_apduACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_ACSE__apdu_P    _ZACSE_apduACS, &_ZACS_mod

#define encode_ACS_AARQ__apdu(pe, top, len, buffer, parm) \
    enc_f(_ZAARQ_apduACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_AARQ__apdu(pe, top, len, buffer, parm) \
    dec_f(_ZAARQ_apduACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_AARQ__apdu(pe, top, len, buffer, parm) \
    prnt_f(_ZAARQ_apduACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_AARQ__apdu_P    _ZAARQ_apduACS, &_ZACS_mod

#define encode_ACS_AARE__apdu(pe, top, len, buffer, parm) \
    enc_f(_ZAARE_apduACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_AARE__apdu(pe, top, len, buffer, parm) \
    dec_f(_ZAARE_apduACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_AARE__apdu(pe, top, len, buffer, parm) \
    prnt_f(_ZAARE_apduACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_AARE__apdu_P    _ZAARE_apduACS, &_ZACS_mod

#define encode_ACS_RLRQ__apdu(pe, top, len, buffer, parm) \
    enc_f(_ZRLRQ_apduACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_RLRQ__apdu(pe, top, len, buffer, parm) \
    dec_f(_ZRLRQ_apduACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_RLRQ__apdu(pe, top, len, buffer, parm) \
    prnt_f(_ZRLRQ_apduACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_RLRQ__apdu_P    _ZRLRQ_apduACS, &_ZACS_mod

#define encode_ACS_RLRE__apdu(pe, top, len, buffer, parm) \
    enc_f(_ZRLRE_apduACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_RLRE__apdu(pe, top, len, buffer, parm) \
    dec_f(_ZRLRE_apduACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_RLRE__apdu(pe, top, len, buffer, parm) \
    prnt_f(_ZRLRE_apduACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_RLRE__apdu_P    _ZRLRE_apduACS, &_ZACS_mod

#define encode_ACS_ABRT__apdu(pe, top, len, buffer, parm) \
    enc_f(_ZABRT_apduACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_ABRT__apdu(pe, top, len, buffer, parm) \
    dec_f(_ZABRT_apduACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_ABRT__apdu(pe, top, len, buffer, parm) \
    prnt_f(_ZABRT_apduACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_ABRT__apdu_P    _ZABRT_apduACS, &_ZACS_mod

#define encode_ACS_ABRT__source(pe, top, len, buffer, parm) \
    enc_f(_ZABRT_sourceACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_ABRT__source(pe, top, len, buffer, parm) \
    dec_f(_ZABRT_sourceACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_ABRT__source(pe, top, len, buffer, parm) \
    prnt_f(_ZABRT_sourceACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_ABRT__source_P    _ZABRT_sourceACS, &_ZACS_mod

#define encode_ACS_Application__context__name(pe, top, len, buffer, parm) \
    enc_f(_ZApplication_context_nameACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_Application__context__name(pe, top, len, buffer, parm) \
    dec_f(_ZApplication_context_nameACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_Application__context__name(pe, top, len, buffer, parm) \
    prnt_f(_ZApplication_context_nameACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_Application__context__name_P    _ZApplication_context_nameACS, &_ZACS_mod

#define encode_ACS_AP__title(pe, top, len, buffer, parm) \
    enc_f(_ZAP_titleACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_AP__title(pe, top, len, buffer, parm) \
    dec_f(_ZAP_titleACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_AP__title(pe, top, len, buffer, parm) \
    prnt_f(_ZAP_titleACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_AP__title_P    _ZAP_titleACS, &_ZACS_mod

#define encode_ACS_AE__qualifier(pe, top, len, buffer, parm) \
    enc_f(_ZAE_qualifierACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_AE__qualifier(pe, top, len, buffer, parm) \
    dec_f(_ZAE_qualifierACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_AE__qualifier(pe, top, len, buffer, parm) \
    prnt_f(_ZAE_qualifierACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_AE__qualifier_P    _ZAE_qualifierACS, &_ZACS_mod

#define encode_ACS_AE__title(pe, top, len, buffer, parm) \
    enc_f(_ZAE_titleACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_AE__title(pe, top, len, buffer, parm) \
    dec_f(_ZAE_titleACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_AE__title(pe, top, len, buffer, parm) \
    prnt_f(_ZAE_titleACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_AE__title_P    _ZAE_titleACS, &_ZACS_mod

#define encode_ACS_AE__invocation__id(pe, top, len, buffer, parm) \
    enc_f(_ZAE_invocation_idACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_AE__invocation__id(pe, top, len, buffer, parm) \
    dec_f(_ZAE_invocation_idACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_AE__invocation__id(pe, top, len, buffer, parm) \
    prnt_f(_ZAE_invocation_idACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_AE__invocation__id_P    _ZAE_invocation_idACS, &_ZACS_mod

#define encode_ACS_AP__invocation__id(pe, top, len, buffer, parm) \
    enc_f(_ZAP_invocation_idACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_AP__invocation__id(pe, top, len, buffer, parm) \
    dec_f(_ZAP_invocation_idACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_AP__invocation__id(pe, top, len, buffer, parm) \
    prnt_f(_ZAP_invocation_idACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_AP__invocation__id_P    _ZAP_invocation_idACS, &_ZACS_mod

#define encode_ACS_Associate__result(pe, top, len, buffer, parm) \
    enc_f(_ZAssociate_resultACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_Associate__result(pe, top, len, buffer, parm) \
    dec_f(_ZAssociate_resultACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_Associate__result(pe, top, len, buffer, parm) \
    prnt_f(_ZAssociate_resultACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_Associate__result_P    _ZAssociate_resultACS, &_ZACS_mod

#define encode_ACS_Associate__source__diagnostic(pe, top, len, buffer, parm) \
    enc_f(_ZAssociate_source_diagnosticACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_Associate__source__diagnostic(pe, top, len, buffer, parm) \
    dec_f(_ZAssociate_source_diagnosticACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_Associate__source__diagnostic(pe, top, len, buffer, parm) \
    prnt_f(_ZAssociate_source_diagnosticACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_Associate__source__diagnostic_P    _ZAssociate_source_diagnosticACS, &_ZACS_mod

#define encode_ACS_Association__information(pe, top, len, buffer, parm) \
    enc_f(_ZAssociation_informationACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_Association__information(pe, top, len, buffer, parm) \
    dec_f(_ZAssociation_informationACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_Association__information(pe, top, len, buffer, parm) \
    prnt_f(_ZAssociation_informationACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_Association__information_P    _ZAssociation_informationACS, &_ZACS_mod

#define encode_ACS_Implementation__data(pe, top, len, buffer, parm) \
    enc_f(_ZImplementation_dataACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_Implementation__data(pe, top, len, buffer, parm) \
    dec_f(_ZImplementation_dataACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_Implementation__data(pe, top, len, buffer, parm) \
    prnt_f(_ZImplementation_dataACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_Implementation__data_P    _ZImplementation_dataACS, &_ZACS_mod

#define encode_ACS_Release__request__reason(pe, top, len, buffer, parm) \
    enc_f(_ZRelease_request_reasonACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_Release__request__reason(pe, top, len, buffer, parm) \
    dec_f(_ZRelease_request_reasonACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_Release__request__reason(pe, top, len, buffer, parm) \
    prnt_f(_ZRelease_request_reasonACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_Release__request__reason_P    _ZRelease_request_reasonACS, &_ZACS_mod

#define encode_ACS_Release__response__reason(pe, top, len, buffer, parm) \
    enc_f(_ZRelease_response_reasonACS, &_ZACS_mod, pe, top, len, buffer, (char *) parm)

#define decode_ACS_Release__response__reason(pe, top, len, buffer, parm) \
    dec_f(_ZRelease_response_reasonACS, &_ZACS_mod, pe, top, len, buffer, (char **) parm)

#define print_ACS_Release__response__reason(pe, top, len, buffer, parm) \
    prnt_f(_ZRelease_response_reasonACS, &_ZACS_mod, pe, top, len, buffer)
#define print_ACS_Release__response__reason_P    _ZRelease_response_reasonACS, &_ZACS_mod


#endif   /* lint */

#endif /* _ACS_defs_ */
