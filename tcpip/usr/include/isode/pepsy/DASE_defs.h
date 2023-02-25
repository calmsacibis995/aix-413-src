/* @(#)57	1.4  src/tcpip/usr/include/isode/pepsy/DASE_defs.h, isodelib7, tcpip411, GOLD410 5/26/93 15:34:00 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: decode_DASE_Callback__REQ decode_DASE_Callback__RSP 
 *    decode_DASE_Environment decode_DASE_Message decode_DASE_Pair 
 *    decode_DASE_Provider__RSP decode_DASE_Query__REQ decode_DASE_Query__RSP 
 *    encode_DASE_Callback__REQ encode_DASE_Callback__RSP 
 *    encode_DASE_Environment encode_DASE_Message encode_DASE_Pair 
 *    encode_DASE_Provider__RSP encode_DASE_Query__REQ encode_DASE_Query__RSP 
 *    print_DASE_Callback__REQ print_DASE_Callback__RSP print_DASE_Environment 
 *    print_DASE_Message print_DASE_Pair print_DASE_Provider__RSP 
 *    print_DASE_Query__REQ print_DASE_Query__RSP
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/DASE_defs.h
 */

#ifndef _DASE_defs_
#define _DASE_defs_

#include <isode/pepsy/DASE_pre_defs.h>

#ifndef	lint
#define encode_DASE_Query__REQ(pe, top, len, buffer, parm) \
    enc_f(_ZQuery_REQDASE, &_ZDASE_mod, pe, top, len, buffer, (char *) parm)

#define decode_DASE_Query__REQ(pe, top, len, buffer, parm) \
    dec_f(_ZQuery_REQDASE, &_ZDASE_mod, pe, top, len, buffer, (char **) parm)

#define print_DASE_Query__REQ(pe, top, len, buffer, parm) \
    prnt_f(_ZQuery_REQDASE, &_ZDASE_mod, pe, top, len, buffer)
#define print_DASE_Query__REQ_P    _ZQuery_REQDASE, &_ZDASE_mod

#define encode_DASE_Environment(pe, top, len, buffer, parm) \
    enc_f(_ZEnvironmentDASE, &_ZDASE_mod, pe, top, len, buffer, (char *) parm)

#define decode_DASE_Environment(pe, top, len, buffer, parm) \
    dec_f(_ZEnvironmentDASE, &_ZDASE_mod, pe, top, len, buffer, (char **) parm)

#define print_DASE_Environment(pe, top, len, buffer, parm) \
    prnt_f(_ZEnvironmentDASE, &_ZDASE_mod, pe, top, len, buffer)
#define print_DASE_Environment_P    _ZEnvironmentDASE, &_ZDASE_mod

#define encode_DASE_Callback__REQ(pe, top, len, buffer, parm) \
    enc_f(_ZCallback_REQDASE, &_ZDASE_mod, pe, top, len, buffer, (char *) parm)

#define decode_DASE_Callback__REQ(pe, top, len, buffer, parm) \
    dec_f(_ZCallback_REQDASE, &_ZDASE_mod, pe, top, len, buffer, (char **) parm)

#define print_DASE_Callback__REQ(pe, top, len, buffer, parm) \
    prnt_f(_ZCallback_REQDASE, &_ZDASE_mod, pe, top, len, buffer)
#define print_DASE_Callback__REQ_P    _ZCallback_REQDASE, &_ZDASE_mod

#define encode_DASE_Pair(pe, top, len, buffer, parm) \
    enc_f(_ZPairDASE, &_ZDASE_mod, pe, top, len, buffer, (char *) parm)

#define decode_DASE_Pair(pe, top, len, buffer, parm) \
    dec_f(_ZPairDASE, &_ZDASE_mod, pe, top, len, buffer, (char **) parm)

#define print_DASE_Pair(pe, top, len, buffer, parm) \
    prnt_f(_ZPairDASE, &_ZDASE_mod, pe, top, len, buffer)
#define print_DASE_Pair_P    _ZPairDASE, &_ZDASE_mod

#define encode_DASE_Callback__RSP(pe, top, len, buffer, parm) \
    enc_f(_ZCallback_RSPDASE, &_ZDASE_mod, pe, top, len, buffer, (char *) parm)

#define decode_DASE_Callback__RSP(pe, top, len, buffer, parm) \
    dec_f(_ZCallback_RSPDASE, &_ZDASE_mod, pe, top, len, buffer, (char **) parm)

#define print_DASE_Callback__RSP(pe, top, len, buffer, parm) \
    prnt_f(_ZCallback_RSPDASE, &_ZDASE_mod, pe, top, len, buffer)
#define print_DASE_Callback__RSP_P    _ZCallback_RSPDASE, &_ZDASE_mod

#define encode_DASE_Query__RSP(pe, top, len, buffer, parm) \
    enc_f(_ZQuery_RSPDASE, &_ZDASE_mod, pe, top, len, buffer, (char *) parm)

#define decode_DASE_Query__RSP(pe, top, len, buffer, parm) \
    dec_f(_ZQuery_RSPDASE, &_ZDASE_mod, pe, top, len, buffer, (char **) parm)

#define print_DASE_Query__RSP(pe, top, len, buffer, parm) \
    prnt_f(_ZQuery_RSPDASE, &_ZDASE_mod, pe, top, len, buffer)
#define print_DASE_Query__RSP_P    _ZQuery_RSPDASE, &_ZDASE_mod

#define encode_DASE_Message(pe, top, len, buffer, parm) \
    enc_f(_ZMessageDASE, &_ZDASE_mod, pe, top, len, buffer, (char *) parm)

#define decode_DASE_Message(pe, top, len, buffer, parm) \
    dec_f(_ZMessageDASE, &_ZDASE_mod, pe, top, len, buffer, (char **) parm)

#define print_DASE_Message(pe, top, len, buffer, parm) \
    prnt_f(_ZMessageDASE, &_ZDASE_mod, pe, top, len, buffer)
#define print_DASE_Message_P    _ZMessageDASE, &_ZDASE_mod

#define encode_DASE_Provider__RSP(pe, top, len, buffer, parm) \
    enc_f(_ZProvider_RSPDASE, &_ZDASE_mod, pe, top, len, buffer, (char *) parm)

#define decode_DASE_Provider__RSP(pe, top, len, buffer, parm) \
    dec_f(_ZProvider_RSPDASE, &_ZDASE_mod, pe, top, len, buffer, (char **) parm)

#define print_DASE_Provider__RSP(pe, top, len, buffer, parm) \
    prnt_f(_ZProvider_RSPDASE, &_ZDASE_mod, pe, top, len, buffer)
#define print_DASE_Provider__RSP_P    _ZProvider_RSPDASE, &_ZDASE_mod


#endif   /* lint */

#endif /* _DASE_defs_ */
