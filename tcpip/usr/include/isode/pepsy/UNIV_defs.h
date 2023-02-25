/* @(#)43	1.4  src/tcpip/usr/include/isode/pepsy/UNIV_defs.h, isodelib7, tcpip411, GOLD410 4/5/93 13:24:16 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: decode_UNIV_CharacterString decode_UNIV_EXTERNAL 
 *    decode_UNIV_GeneralString decode_UNIV_GeneralisedTime 
 *    decode_UNIV_GeneralizedTime decode_UNIV_GraphicString 
 *    decode_UNIV_IA5String decode_UNIV_ISO646String decode_UNIV_NumericString 
 *    decode_UNIV_ObjectDescriptor decode_UNIV_PrintableString 
 *    decode_UNIV_T61String decode_UNIV_TeletexString decode_UNIV_UTCTime 
 *    decode_UNIV_UniversalTime decode_UNIV_VideotexString 
 *    decode_UNIV_VisibleString encode_UNIV_CharacterString 
 *    encode_UNIV_EXTERNAL encode_UNIV_GeneralString 
 *    encode_UNIV_GeneralisedTime encode_UNIV_GeneralizedTime 
 *    encode_UNIV_GraphicString encode_UNIV_IA5String encode_UNIV_ISO646String 
 *    encode_UNIV_NumericString encode_UNIV_ObjectDescriptor 
 *    encode_UNIV_PrintableString encode_UNIV_T61String 
 *    encode_UNIV_TeletexString encode_UNIV_UTCTime encode_UNIV_UniversalTime 
 *    encode_UNIV_VideotexString encode_UNIV_VisibleString 
 *    print_UNIV_CharacterString print_UNIV_EXTERNAL print_UNIV_GeneralString 
 *    print_UNIV_GeneralisedTime print_UNIV_GeneralizedTime 
 *    print_UNIV_GraphicString print_UNIV_IA5String print_UNIV_ISO646String 
 *    print_UNIV_NumericString print_UNIV_ObjectDescriptor 
 *    print_UNIV_PrintableString print_UNIV_T61String print_UNIV_TeletexString 
 *    print_UNIV_UTCTime print_UNIV_UniversalTime print_UNIV_VideotexString 
 *    print_UNIV_VisibleString
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/UNIV_defs.h
 */

#ifndef _UNIV_defs_
#define _UNIV_defs_

#include <isode/pepsy/UNIV_pre_defs.h>

#ifndef	lint
#define encode_UNIV_IA5String(pe, top, len, buffer, parm) \
    enc_f(_ZIA5StringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_IA5String(pe, top, len, buffer, parm) \
    dec_f(_ZIA5StringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_IA5String(pe, top, len, buffer, parm) \
    prnt_f(_ZIA5StringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_IA5String_P    _ZIA5StringUNIV, &_ZUNIV_mod

#define encode_UNIV_NumericString(pe, top, len, buffer, parm) \
    enc_f(_ZNumericStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_NumericString(pe, top, len, buffer, parm) \
    dec_f(_ZNumericStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_NumericString(pe, top, len, buffer, parm) \
    prnt_f(_ZNumericStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_NumericString_P    _ZNumericStringUNIV, &_ZUNIV_mod

#define encode_UNIV_PrintableString(pe, top, len, buffer, parm) \
    enc_f(_ZPrintableStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_PrintableString(pe, top, len, buffer, parm) \
    dec_f(_ZPrintableStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_PrintableString(pe, top, len, buffer, parm) \
    prnt_f(_ZPrintableStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_PrintableString_P    _ZPrintableStringUNIV, &_ZUNIV_mod

#define encode_UNIV_T61String(pe, top, len, buffer, parm) \
    enc_f(_ZT61StringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_T61String(pe, top, len, buffer, parm) \
    dec_f(_ZT61StringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_T61String(pe, top, len, buffer, parm) \
    prnt_f(_ZT61StringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_T61String_P    _ZT61StringUNIV, &_ZUNIV_mod

#define encode_UNIV_TeletexString(pe, top, len, buffer, parm) \
    enc_f(_ZTeletexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_TeletexString(pe, top, len, buffer, parm) \
    dec_f(_ZTeletexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_TeletexString(pe, top, len, buffer, parm) \
    prnt_f(_ZTeletexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_TeletexString_P    _ZTeletexStringUNIV, &_ZUNIV_mod

#define encode_UNIV_VideotexString(pe, top, len, buffer, parm) \
    enc_f(_ZVideotexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_VideotexString(pe, top, len, buffer, parm) \
    dec_f(_ZVideotexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_VideotexString(pe, top, len, buffer, parm) \
    prnt_f(_ZVideotexStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_VideotexString_P    _ZVideotexStringUNIV, &_ZUNIV_mod

#define encode_UNIV_GeneralizedTime(pe, top, len, buffer, parm) \
    enc_f(_ZGeneralizedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_GeneralizedTime(pe, top, len, buffer, parm) \
    dec_f(_ZGeneralizedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_GeneralizedTime(pe, top, len, buffer, parm) \
    prnt_f(_ZGeneralizedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_GeneralizedTime_P    _ZGeneralizedTimeUNIV, &_ZUNIV_mod

#define encode_UNIV_GeneralisedTime(pe, top, len, buffer, parm) \
    enc_f(_ZGeneralisedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_GeneralisedTime(pe, top, len, buffer, parm) \
    dec_f(_ZGeneralisedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_GeneralisedTime(pe, top, len, buffer, parm) \
    prnt_f(_ZGeneralisedTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_GeneralisedTime_P    _ZGeneralisedTimeUNIV, &_ZUNIV_mod

#define encode_UNIV_UTCTime(pe, top, len, buffer, parm) \
    enc_f(_ZUTCTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_UTCTime(pe, top, len, buffer, parm) \
    dec_f(_ZUTCTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_UTCTime(pe, top, len, buffer, parm) \
    prnt_f(_ZUTCTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_UTCTime_P    _ZUTCTimeUNIV, &_ZUNIV_mod

#define encode_UNIV_UniversalTime(pe, top, len, buffer, parm) \
    enc_f(_ZUniversalTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_UniversalTime(pe, top, len, buffer, parm) \
    dec_f(_ZUniversalTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_UniversalTime(pe, top, len, buffer, parm) \
    prnt_f(_ZUniversalTimeUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_UniversalTime_P    _ZUniversalTimeUNIV, &_ZUNIV_mod

#define encode_UNIV_GraphicString(pe, top, len, buffer, parm) \
    enc_f(_ZGraphicStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_GraphicString(pe, top, len, buffer, parm) \
    dec_f(_ZGraphicStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_GraphicString(pe, top, len, buffer, parm) \
    prnt_f(_ZGraphicStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_GraphicString_P    _ZGraphicStringUNIV, &_ZUNIV_mod

#define encode_UNIV_VisibleString(pe, top, len, buffer, parm) \
    enc_f(_ZVisibleStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_VisibleString(pe, top, len, buffer, parm) \
    dec_f(_ZVisibleStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_VisibleString(pe, top, len, buffer, parm) \
    prnt_f(_ZVisibleStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_VisibleString_P    _ZVisibleStringUNIV, &_ZUNIV_mod

#define encode_UNIV_ISO646String(pe, top, len, buffer, parm) \
    enc_f(_ZISO646StringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_ISO646String(pe, top, len, buffer, parm) \
    dec_f(_ZISO646StringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_ISO646String(pe, top, len, buffer, parm) \
    prnt_f(_ZISO646StringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_ISO646String_P    _ZISO646StringUNIV, &_ZUNIV_mod

#define encode_UNIV_GeneralString(pe, top, len, buffer, parm) \
    enc_f(_ZGeneralStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_GeneralString(pe, top, len, buffer, parm) \
    dec_f(_ZGeneralStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_GeneralString(pe, top, len, buffer, parm) \
    prnt_f(_ZGeneralStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_GeneralString_P    _ZGeneralStringUNIV, &_ZUNIV_mod

#define encode_UNIV_CharacterString(pe, top, len, buffer, parm) \
    enc_f(_ZCharacterStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_CharacterString(pe, top, len, buffer, parm) \
    dec_f(_ZCharacterStringUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_CharacterString(pe, top, len, buffer, parm) \
    prnt_f(_ZCharacterStringUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_CharacterString_P    _ZCharacterStringUNIV, &_ZUNIV_mod

#define encode_UNIV_EXTERNAL(pe, top, len, buffer, parm) \
    enc_f(_ZEXTERNALUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_EXTERNAL(pe, top, len, buffer, parm) \
    dec_f(_ZEXTERNALUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_EXTERNAL(pe, top, len, buffer, parm) \
    prnt_f(_ZEXTERNALUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_EXTERNAL_P    _ZEXTERNALUNIV, &_ZUNIV_mod

#define encode_UNIV_ObjectDescriptor(pe, top, len, buffer, parm) \
    enc_f(_ZObjectDescriptorUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char *) parm)

#define decode_UNIV_ObjectDescriptor(pe, top, len, buffer, parm) \
    dec_f(_ZObjectDescriptorUNIV, &_ZUNIV_mod, pe, top, len, buffer, (char **) parm)

#define print_UNIV_ObjectDescriptor(pe, top, len, buffer, parm) \
    prnt_f(_ZObjectDescriptorUNIV, &_ZUNIV_mod, pe, top, len, buffer)
#define print_UNIV_ObjectDescriptor_P    _ZObjectDescriptorUNIV, &_ZUNIV_mod


#endif   /* lint */

#endif /* _UNIV_defs_ */
