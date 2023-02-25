/* @(#)84	1.4  src/tcpip/usr/include/isode/pepsy/RONOT_defs.h, isodelib7, tcpip411, GOLD410 5/26/93 15:35:15 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: decode_RONOT_BindArgumentValue decode_RONOT_BindErrorValue 
 *    decode_RONOT_BindResultValue decode_RONOT_UnBindArgumentValue 
 *    decode_RONOT_UnBindErrorValue decode_RONOT_UnBindResultValue 
 *    encode_RONOT_BindArgumentValue encode_RONOT_BindErrorValue 
 *    encode_RONOT_BindResultValue encode_RONOT_UnBindArgumentValue 
 *    encode_RONOT_UnBindErrorValue encode_RONOT_UnBindResultValue 
 *    print_RONOT_BindArgumentValue print_RONOT_BindErrorValue 
 *    print_RONOT_BindResultValue print_RONOT_UnBindArgumentValue 
 *    print_RONOT_UnBindErrorValue print_RONOT_UnBindResultValue
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/RONOT_defs.h
 */

#ifndef _RONOT_defs_
#define _RONOT_defs_

#include <isode/pepsy/RONOT_pre_defs.h>

#ifndef	lint
#define encode_RONOT_BindArgumentValue(pe, top, len, buffer, parm) \
    enc_f(_ZBindArgumentValueRONOT, &_ZRONOT_mod, pe, top, len, buffer, (char *) parm)

#define decode_RONOT_BindArgumentValue(pe, top, len, buffer, parm) \
    dec_f(_ZBindArgumentValueRONOT, &_ZRONOT_mod, pe, top, len, buffer, (char **) parm)

#define print_RONOT_BindArgumentValue(pe, top, len, buffer, parm) \
    prnt_f(_ZBindArgumentValueRONOT, &_ZRONOT_mod, pe, top, len, buffer)
#define print_RONOT_BindArgumentValue_P    _ZBindArgumentValueRONOT, &_ZRONOT_mod

#define encode_RONOT_BindResultValue(pe, top, len, buffer, parm) \
    enc_f(_ZBindResultValueRONOT, &_ZRONOT_mod, pe, top, len, buffer, (char *) parm)

#define decode_RONOT_BindResultValue(pe, top, len, buffer, parm) \
    dec_f(_ZBindResultValueRONOT, &_ZRONOT_mod, pe, top, len, buffer, (char **) parm)

#define print_RONOT_BindResultValue(pe, top, len, buffer, parm) \
    prnt_f(_ZBindResultValueRONOT, &_ZRONOT_mod, pe, top, len, buffer)
#define print_RONOT_BindResultValue_P    _ZBindResultValueRONOT, &_ZRONOT_mod

#define encode_RONOT_BindErrorValue(pe, top, len, buffer, parm) \
    enc_f(_ZBindErrorValueRONOT, &_ZRONOT_mod, pe, top, len, buffer, (char *) parm)

#define decode_RONOT_BindErrorValue(pe, top, len, buffer, parm) \
    dec_f(_ZBindErrorValueRONOT, &_ZRONOT_mod, pe, top, len, buffer, (char **) parm)

#define print_RONOT_BindErrorValue(pe, top, len, buffer, parm) \
    prnt_f(_ZBindErrorValueRONOT, &_ZRONOT_mod, pe, top, len, buffer)
#define print_RONOT_BindErrorValue_P    _ZBindErrorValueRONOT, &_ZRONOT_mod

#define encode_RONOT_UnBindArgumentValue(pe, top, len, buffer, parm) \
    enc_f(_ZUnBindArgumentValueRONOT, &_ZRONOT_mod, pe, top, len, buffer, (char *) parm)

#define decode_RONOT_UnBindArgumentValue(pe, top, len, buffer, parm) \
    dec_f(_ZUnBindArgumentValueRONOT, &_ZRONOT_mod, pe, top, len, buffer, (char **) parm)

#define print_RONOT_UnBindArgumentValue(pe, top, len, buffer, parm) \
    prnt_f(_ZUnBindArgumentValueRONOT, &_ZRONOT_mod, pe, top, len, buffer)
#define print_RONOT_UnBindArgumentValue_P    _ZUnBindArgumentValueRONOT, &_ZRONOT_mod

#define encode_RONOT_UnBindResultValue(pe, top, len, buffer, parm) \
    enc_f(_ZUnBindResultValueRONOT, &_ZRONOT_mod, pe, top, len, buffer, (char *) parm)

#define decode_RONOT_UnBindResultValue(pe, top, len, buffer, parm) \
    dec_f(_ZUnBindResultValueRONOT, &_ZRONOT_mod, pe, top, len, buffer, (char **) parm)

#define print_RONOT_UnBindResultValue(pe, top, len, buffer, parm) \
    prnt_f(_ZUnBindResultValueRONOT, &_ZRONOT_mod, pe, top, len, buffer)
#define print_RONOT_UnBindResultValue_P    _ZUnBindResultValueRONOT, &_ZRONOT_mod

#define encode_RONOT_UnBindErrorValue(pe, top, len, buffer, parm) \
    enc_f(_ZUnBindErrorValueRONOT, &_ZRONOT_mod, pe, top, len, buffer, (char *) parm)

#define decode_RONOT_UnBindErrorValue(pe, top, len, buffer, parm) \
    dec_f(_ZUnBindErrorValueRONOT, &_ZRONOT_mod, pe, top, len, buffer, (char **) parm)

#define print_RONOT_UnBindErrorValue(pe, top, len, buffer, parm) \
    prnt_f(_ZUnBindErrorValueRONOT, &_ZRONOT_mod, pe, top, len, buffer)
#define print_RONOT_UnBindErrorValue_P    _ZUnBindErrorValueRONOT, &_ZRONOT_mod


#endif   /* lint */

#endif /* _RONOT_defs_ */
