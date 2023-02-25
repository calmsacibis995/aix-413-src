/* @(#)67	1.4  src/tcpip/usr/include/isode/pepsy/PS_defs.h, isodelib7, tcpip411, GOLD410 5/26/93 15:34:44 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: decode_PS_ACA__PPDU decode_PS_AC__PPDU decode_PS_ARP__PPDU 
 *    decode_PS_ARU__PPDU decode_PS_Abort__reason decode_PS_Abort__type 
 *    decode_PS_Abstract__syntax__name decode_PS_Addition__list 
 *    decode_PS_Addition__result__list decode_PS_CPA__type decode_PS_CPC__type 
 *    decode_PS_CPR__type decode_PS_CP__type 
 *    decode_PS_Called__presentation__selector 
 *    decode_PS_Calling__presentation__selector decode_PS_Context__list 
 *    decode_PS_Context__name decode_PS_Context__result 
 *    decode_PS_Definition__list decode_PS_Definition__result__list 
 *    decode_PS_Deletion__list decode_PS_Deletion__result__list 
 *    decode_PS_Event__identifier decode_PS_Fully__encoded__data 
 *    decode_PS_Identifier decode_PS_Identifier__list decode_PS_Mode__selector 
 *    decode_PS_PDV__list decode_PS_Presentation__requirements 
 *    decode_PS_Protocol__version decode_PS_Provider__reason 
 *    decode_PS_RSA__PPDU decode_PS_RS__PPDU 
 *    decode_PS_Responding__presentation__selector decode_PS_Result 
 *    decode_PS_Result__list decode_PS_Selector
 *    decode_PS_Simply__encoded__data decode_PS_Transfer__syntax__name 
 *    decode_PS_Typed__data__type decode_PS_User__data 
 *    decode_PS_User__session__requirements encode_PS_ACA__PPDU 
 *    encode_PS_AC__PPDU encode_PS_ARP__PPDU encode_PS_ARU__PPDU 
 *    encode_PS_Abort__reason encode_PS_Abort__type 
 *    encode_PS_Abstract__syntax__name encode_PS_Addition__list 
 *    encode_PS_Addition__result__list encode_PS_CPA__type encode_PS_CPC__type 
 *    encode_PS_CPR__type encode_PS_CP__type 
 *    encode_PS_Called__presentation__selector 
 *    encode_PS_Calling__presentation__selector encode_PS_Context__list 
 *    encode_PS_Context__name encode_PS_Context__result 
 *    encode_PS_Definition__list encode_PS_Definition__result__list 
 *    encode_PS_Deletion__list encode_PS_Deletion__result__list 
 *    encode_PS_Event__identifier encode_PS_Fully__encoded__data 
 *    encode_PS_Identifier encode_PS_Identifier__list encode_PS_Mode__selector 
 *    encode_PS_PDV__list encode_PS_Presentation__requirements 
 *    encode_PS_Protocol__version encode_PS_Provider__reason 
 *    encode_PS_RSA__PPDU encode_PS_RS__PPDU 
 *    encode_PS_Responding__presentation__selector encode_PS_Result 
 *    encode_PS_Result__list encode_PS_Selector encode_PS_Simply__encoded__data
 *    encode_PS_Transfer__syntax__name encode_PS_Typed__data__type 
 *    encode_PS_User__data encode_PS_User__session__requirements
 *    print_PS_ACA__PPDU print_PS_AC__PPDU print_PS_ARP__PPDU 
 *    print_PS_ARU__PPDU print_PS_Abort__reason print_PS_Abort__type 
 *    print_PS_Abstract__syntax__name print_PS_Addition__list 
 *    print_PS_Addition__result__list print_PS_CPA__type print_PS_CPC__type 
 *    print_PS_CPR__type print_PS_CP__type 
 *    print_PS_Called__presentation__selector 
 *    print_PS_Calling__presentation__selector print_PS_Context__list 
 *    print_PS_Context__name print_PS_Context__result print_PS_Definition__list
 *    print_PS_Definition__result__list print_PS_Deletion__list 
 *    print_PS_Deletion__result__list print_PS_Event__identifier 
 *    print_PS_Fully__encoded__data print_PS_Identifier 
 *    print_PS_Identifier__list print_PS_Mode__selector print_PS_PDV__list 
 *    print_PS_Presentation__requirements print_PS_Protocol__version 
 *    print_PS_Provider__reason print_PS_RSA__PPDU print_PS_RS__PPDU 
 *    print_PS_Responding__presentation__selector print_PS_Result 
 *    print_PS_Result__list print_PS_Selector print_PS_Simply__encoded__data 
 *    print_PS_Transfer__syntax__name print_PS_Typed__data__type 
 *    print_PS_User__data print_PS_User__session__requirements
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/PS_defs.h
 */

#ifndef _PS_defs_
#define _PS_defs_

#include <isode/pepsy/PS_pre_defs.h>

#ifndef	lint
#define encode_PS_CP__type(pe, top, len, buffer, parm) \
    enc_f(_ZCP_typePS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_CP__type(pe, top, len, buffer, parm) \
    dec_f(_ZCP_typePS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_CP__type(pe, top, len, buffer, parm) \
    prnt_f(_ZCP_typePS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_CP__type_P    _ZCP_typePS, &_ZPS_mod

#define encode_PS_CPC__type(pe, top, len, buffer, parm) \
    enc_f(_ZCPC_typePS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_CPC__type(pe, top, len, buffer, parm) \
    dec_f(_ZCPC_typePS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_CPC__type(pe, top, len, buffer, parm) \
    prnt_f(_ZCPC_typePS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_CPC__type_P    _ZCPC_typePS, &_ZPS_mod

#define encode_PS_CPA__type(pe, top, len, buffer, parm) \
    enc_f(_ZCPA_typePS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_CPA__type(pe, top, len, buffer, parm) \
    dec_f(_ZCPA_typePS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_CPA__type(pe, top, len, buffer, parm) \
    prnt_f(_ZCPA_typePS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_CPA__type_P    _ZCPA_typePS, &_ZPS_mod

#define encode_PS_CPR__type(pe, top, len, buffer, parm) \
    enc_f(_ZCPR_typePS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_CPR__type(pe, top, len, buffer, parm) \
    dec_f(_ZCPR_typePS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_CPR__type(pe, top, len, buffer, parm) \
    prnt_f(_ZCPR_typePS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_CPR__type_P    _ZCPR_typePS, &_ZPS_mod

#define encode_PS_Abort__type(pe, top, len, buffer, parm) \
    enc_f(_ZAbort_typePS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Abort__type(pe, top, len, buffer, parm) \
    dec_f(_ZAbort_typePS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Abort__type(pe, top, len, buffer, parm) \
    prnt_f(_ZAbort_typePS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Abort__type_P    _ZAbort_typePS, &_ZPS_mod

#define encode_PS_ARU__PPDU(pe, top, len, buffer, parm) \
    enc_f(_ZARU_PPDUPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_ARU__PPDU(pe, top, len, buffer, parm) \
    dec_f(_ZARU_PPDUPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_ARU__PPDU(pe, top, len, buffer, parm) \
    prnt_f(_ZARU_PPDUPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_ARU__PPDU_P    _ZARU_PPDUPS, &_ZPS_mod

#define encode_PS_ARP__PPDU(pe, top, len, buffer, parm) \
    enc_f(_ZARP_PPDUPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_ARP__PPDU(pe, top, len, buffer, parm) \
    dec_f(_ZARP_PPDUPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_ARP__PPDU(pe, top, len, buffer, parm) \
    prnt_f(_ZARP_PPDUPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_ARP__PPDU_P    _ZARP_PPDUPS, &_ZPS_mod

#define encode_PS_Typed__data__type(pe, top, len, buffer, parm) \
    enc_f(_ZTyped_data_typePS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Typed__data__type(pe, top, len, buffer, parm) \
    dec_f(_ZTyped_data_typePS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Typed__data__type(pe, top, len, buffer, parm) \
    prnt_f(_ZTyped_data_typePS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Typed__data__type_P    _ZTyped_data_typePS, &_ZPS_mod

#define encode_PS_AC__PPDU(pe, top, len, buffer, parm) \
    enc_f(_ZAC_PPDUPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_AC__PPDU(pe, top, len, buffer, parm) \
    dec_f(_ZAC_PPDUPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_AC__PPDU(pe, top, len, buffer, parm) \
    prnt_f(_ZAC_PPDUPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_AC__PPDU_P    _ZAC_PPDUPS, &_ZPS_mod

#define encode_PS_ACA__PPDU(pe, top, len, buffer, parm) \
    enc_f(_ZACA_PPDUPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_ACA__PPDU(pe, top, len, buffer, parm) \
    dec_f(_ZACA_PPDUPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_ACA__PPDU(pe, top, len, buffer, parm) \
    prnt_f(_ZACA_PPDUPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_ACA__PPDU_P    _ZACA_PPDUPS, &_ZPS_mod

#define encode_PS_RS__PPDU(pe, top, len, buffer, parm) \
    enc_f(_ZRS_PPDUPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_RS__PPDU(pe, top, len, buffer, parm) \
    dec_f(_ZRS_PPDUPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_RS__PPDU(pe, top, len, buffer, parm) \
    prnt_f(_ZRS_PPDUPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_RS__PPDU_P    _ZRS_PPDUPS, &_ZPS_mod

#define encode_PS_RSA__PPDU(pe, top, len, buffer, parm) \
    enc_f(_ZRSA_PPDUPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_RSA__PPDU(pe, top, len, buffer, parm) \
    dec_f(_ZRSA_PPDUPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_RSA__PPDU(pe, top, len, buffer, parm) \
    prnt_f(_ZRSA_PPDUPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_RSA__PPDU_P    _ZRSA_PPDUPS, &_ZPS_mod

#define encode_PS_Abort__reason(pe, top, len, buffer, parm) \
    enc_f(_ZAbort_reasonPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Abort__reason(pe, top, len, buffer, parm) \
    dec_f(_ZAbort_reasonPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Abort__reason(pe, top, len, buffer, parm) \
    prnt_f(_ZAbort_reasonPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Abort__reason_P    _ZAbort_reasonPS, &_ZPS_mod

#define encode_PS_Abstract__syntax__name(pe, top, len, buffer, parm) \
    enc_f(_ZAbstract_syntax_namePS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Abstract__syntax__name(pe, top, len, buffer, parm) \
    dec_f(_ZAbstract_syntax_namePS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Abstract__syntax__name(pe, top, len, buffer, parm) \
    prnt_f(_ZAbstract_syntax_namePS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Abstract__syntax__name_P    _ZAbstract_syntax_namePS, &_ZPS_mod

#define encode_PS_Called__presentation__selector(pe, top, len, buffer, parm) \
    enc_f(_ZCalled_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Called__presentation__selector(pe, top, len, buffer, parm) \
    dec_f(_ZCalled_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Called__presentation__selector(pe, top, len, buffer, parm) \
    prnt_f(_ZCalled_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Called__presentation__selector_P    _ZCalled_presentation_selectorPS, &_ZPS_mod

#define encode_PS_Calling__presentation__selector(pe, top, len, buffer, parm) \
    enc_f(_ZCalling_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Calling__presentation__selector(pe, top, len, buffer, parm) \
    dec_f(_ZCalling_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Calling__presentation__selector(pe, top, len, buffer, parm) \
    prnt_f(_ZCalling_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Calling__presentation__selector_P    _ZCalling_presentation_selectorPS, &_ZPS_mod

#define encode_PS_Context__list(pe, top, len, buffer, parm) \
    enc_f(_ZContext_listPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Context__list(pe, top, len, buffer, parm) \
    dec_f(_ZContext_listPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Context__list(pe, top, len, buffer, parm) \
    prnt_f(_ZContext_listPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Context__list_P    _ZContext_listPS, &_ZPS_mod

#define encode_PS_Context__name(pe, top, len, buffer, parm) \
    enc_f(_ZContext_namePS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Context__name(pe, top, len, buffer, parm) \
    dec_f(_ZContext_namePS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Context__name(pe, top, len, buffer, parm) \
    prnt_f(_ZContext_namePS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Context__name_P    _ZContext_namePS, &_ZPS_mod

#define encode_PS_Context__result(pe, top, len, buffer, parm) \
    enc_f(_ZContext_resultPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Context__result(pe, top, len, buffer, parm) \
    dec_f(_ZContext_resultPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Context__result(pe, top, len, buffer, parm) \
    prnt_f(_ZContext_resultPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Context__result_P    _ZContext_resultPS, &_ZPS_mod

#define encode_PS_Event__identifier(pe, top, len, buffer, parm) \
    enc_f(_ZEvent_identifierPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Event__identifier(pe, top, len, buffer, parm) \
    dec_f(_ZEvent_identifierPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Event__identifier(pe, top, len, buffer, parm) \
    prnt_f(_ZEvent_identifierPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Event__identifier_P    _ZEvent_identifierPS, &_ZPS_mod

#define encode_PS_Mode__selector(pe, top, len, buffer, parm) \
    enc_f(_ZMode_selectorPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Mode__selector(pe, top, len, buffer, parm) \
    dec_f(_ZMode_selectorPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Mode__selector(pe, top, len, buffer, parm) \
    prnt_f(_ZMode_selectorPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Mode__selector_P    _ZMode_selectorPS, &_ZPS_mod

#define encode_PS_Addition__list(pe, top, len, buffer, parm) \
    enc_f(_ZAddition_listPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Addition__list(pe, top, len, buffer, parm) \
    dec_f(_ZAddition_listPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Addition__list(pe, top, len, buffer, parm) \
    prnt_f(_ZAddition_listPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Addition__list_P    _ZAddition_listPS, &_ZPS_mod

#define encode_PS_Addition__result__list(pe, top, len, buffer, parm) \
    enc_f(_ZAddition_result_listPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Addition__result__list(pe, top, len, buffer, parm) \
    dec_f(_ZAddition_result_listPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Addition__result__list(pe, top, len, buffer, parm) \
    prnt_f(_ZAddition_result_listPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Addition__result__list_P    _ZAddition_result_listPS, &_ZPS_mod

#define encode_PS_Definition__list(pe, top, len, buffer, parm) \
    enc_f(_ZDefinition_listPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Definition__list(pe, top, len, buffer, parm) \
    dec_f(_ZDefinition_listPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Definition__list(pe, top, len, buffer, parm) \
    prnt_f(_ZDefinition_listPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Definition__list_P    _ZDefinition_listPS, &_ZPS_mod

#define encode_PS_Definition__result__list(pe, top, len, buffer, parm) \
    enc_f(_ZDefinition_result_listPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Definition__result__list(pe, top, len, buffer, parm) \
    dec_f(_ZDefinition_result_listPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Definition__result__list(pe, top, len, buffer, parm) \
    prnt_f(_ZDefinition_result_listPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Definition__result__list_P    _ZDefinition_result_listPS, &_ZPS_mod

#define encode_PS_Deletion__list(pe, top, len, buffer, parm) \
    enc_f(_ZDeletion_listPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Deletion__list(pe, top, len, buffer, parm) \
    dec_f(_ZDeletion_listPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Deletion__list(pe, top, len, buffer, parm) \
    prnt_f(_ZDeletion_listPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Deletion__list_P    _ZDeletion_listPS, &_ZPS_mod

#define encode_PS_Deletion__result__list(pe, top, len, buffer, parm) \
    enc_f(_ZDeletion_result_listPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Deletion__result__list(pe, top, len, buffer, parm) \
    dec_f(_ZDeletion_result_listPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Deletion__result__list(pe, top, len, buffer, parm) \
    prnt_f(_ZDeletion_result_listPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Deletion__result__list_P    _ZDeletion_result_listPS, &_ZPS_mod

#define encode_PS_Identifier(pe, top, len, buffer, parm) \
    enc_f(_ZIdentifierPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Identifier(pe, top, len, buffer, parm) \
    dec_f(_ZIdentifierPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Identifier(pe, top, len, buffer, parm) \
    prnt_f(_ZIdentifierPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Identifier_P    _ZIdentifierPS, &_ZPS_mod

#define encode_PS_Identifier__list(pe, top, len, buffer, parm) \
    enc_f(_ZIdentifier_listPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Identifier__list(pe, top, len, buffer, parm) \
    dec_f(_ZIdentifier_listPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Identifier__list(pe, top, len, buffer, parm) \
    prnt_f(_ZIdentifier_listPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Identifier__list_P    _ZIdentifier_listPS, &_ZPS_mod

#define encode_PS_Presentation__requirements(pe, top, len, buffer, parm) \
    enc_f(_ZPresentation_requirementsPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Presentation__requirements(pe, top, len, buffer, parm) \
    dec_f(_ZPresentation_requirementsPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Presentation__requirements(pe, top, len, buffer, parm) \
    prnt_f(_ZPresentation_requirementsPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Presentation__requirements_P    _ZPresentation_requirementsPS, &_ZPS_mod

#define encode_PS_Selector(pe, top, len, buffer, parm) \
    enc_f(_ZSelectorPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Selector(pe, top, len, buffer, parm) \
    dec_f(_ZSelectorPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Selector(pe, top, len, buffer, parm) \
    prnt_f(_ZSelectorPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Selector_P    _ZSelectorPS, &_ZPS_mod

#define encode_PS_Protocol__version(pe, top, len, buffer, parm) \
    enc_f(_ZProtocol_versionPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Protocol__version(pe, top, len, buffer, parm) \
    dec_f(_ZProtocol_versionPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Protocol__version(pe, top, len, buffer, parm) \
    prnt_f(_ZProtocol_versionPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Protocol__version_P    _ZProtocol_versionPS, &_ZPS_mod

#define encode_PS_Provider__reason(pe, top, len, buffer, parm) \
    enc_f(_ZProvider_reasonPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Provider__reason(pe, top, len, buffer, parm) \
    dec_f(_ZProvider_reasonPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Provider__reason(pe, top, len, buffer, parm) \
    prnt_f(_ZProvider_reasonPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Provider__reason_P    _ZProvider_reasonPS, &_ZPS_mod

#define encode_PS_Responding__presentation__selector(pe, top, len, buffer, parm) \
    enc_f(_ZResponding_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Responding__presentation__selector(pe, top, len, buffer, parm) \
    dec_f(_ZResponding_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Responding__presentation__selector(pe, top, len, buffer, parm) \
    prnt_f(_ZResponding_presentation_selectorPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Responding__presentation__selector_P    _ZResponding_presentation_selectorPS, &_ZPS_mod

#define encode_PS_Result(pe, top, len, buffer, parm) \
    enc_f(_ZResultPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Result(pe, top, len, buffer, parm) \
    dec_f(_ZResultPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Result(pe, top, len, buffer, parm) \
    prnt_f(_ZResultPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Result_P    _ZResultPS, &_ZPS_mod

#define encode_PS_Result__list(pe, top, len, buffer, parm) \
    enc_f(_ZResult_listPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Result__list(pe, top, len, buffer, parm) \
    dec_f(_ZResult_listPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Result__list(pe, top, len, buffer, parm) \
    prnt_f(_ZResult_listPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Result__list_P    _ZResult_listPS, &_ZPS_mod

#define encode_PS_Transfer__syntax__name(pe, top, len, buffer, parm) \
    enc_f(_ZTransfer_syntax_namePS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Transfer__syntax__name(pe, top, len, buffer, parm) \
    dec_f(_ZTransfer_syntax_namePS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Transfer__syntax__name(pe, top, len, buffer, parm) \
    prnt_f(_ZTransfer_syntax_namePS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Transfer__syntax__name_P    _ZTransfer_syntax_namePS, &_ZPS_mod

#define encode_PS_User__data(pe, top, len, buffer, parm) \
    enc_f(_ZUser_dataPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_User__data(pe, top, len, buffer, parm) \
    dec_f(_ZUser_dataPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_User__data(pe, top, len, buffer, parm) \
    prnt_f(_ZUser_dataPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_User__data_P    _ZUser_dataPS, &_ZPS_mod

#define encode_PS_Simply__encoded__data(pe, top, len, buffer, parm) \
    enc_f(_ZSimply_encoded_dataPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Simply__encoded__data(pe, top, len, buffer, parm) \
    dec_f(_ZSimply_encoded_dataPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Simply__encoded__data(pe, top, len, buffer, parm) \
    prnt_f(_ZSimply_encoded_dataPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Simply__encoded__data_P    _ZSimply_encoded_dataPS, &_ZPS_mod

#define encode_PS_Fully__encoded__data(pe, top, len, buffer, parm) \
    enc_f(_ZFully_encoded_dataPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_Fully__encoded__data(pe, top, len, buffer, parm) \
    dec_f(_ZFully_encoded_dataPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_Fully__encoded__data(pe, top, len, buffer, parm) \
    prnt_f(_ZFully_encoded_dataPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_Fully__encoded__data_P    _ZFully_encoded_dataPS, &_ZPS_mod

#define encode_PS_PDV__list(pe, top, len, buffer, parm) \
    enc_f(_ZPDV_listPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_PDV__list(pe, top, len, buffer, parm) \
    dec_f(_ZPDV_listPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_PDV__list(pe, top, len, buffer, parm) \
    prnt_f(_ZPDV_listPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_PDV__list_P    _ZPDV_listPS, &_ZPS_mod

#define encode_PS_User__session__requirements(pe, top, len, buffer, parm) \
    enc_f(_ZUser_session_requirementsPS, &_ZPS_mod, pe, top, len, buffer, (char *) parm)

#define decode_PS_User__session__requirements(pe, top, len, buffer, parm) \
    dec_f(_ZUser_session_requirementsPS, &_ZPS_mod, pe, top, len, buffer, (char **) parm)

#define print_PS_User__session__requirements(pe, top, len, buffer, parm) \
    prnt_f(_ZUser_session_requirementsPS, &_ZPS_mod, pe, top, len, buffer)
#define print_PS_User__session__requirements_P    _ZUser_session_requirementsPS, &_ZPS_mod


#endif   /* lint */

#endif /* _PS_defs_ */
