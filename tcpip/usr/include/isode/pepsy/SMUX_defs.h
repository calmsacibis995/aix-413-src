/* @(#)40	1.4  src/tcpip/usr/include/isode/pepsy/SMUX_defs.h, isodelib7, tcpip411, GOLD410 4/5/93 13:22:52 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: decode_SMUX_ClosePDU decode_SMUX_OpenPDU decode_SMUX_PDUs 
 *    decode_SMUX_RReqPDU decode_SMUX_RRspPDU decode_SMUX_SOutPDU 
 *    decode_SMUX_SimpleOpen encode_SMUX_ClosePDU encode_SMUX_OpenPDU 
 *    encode_SMUX_PDUs encode_SMUX_RReqPDU encode_SMUX_RRspPDU 
 *    encode_SMUX_SOutPDU encode_SMUX_SimpleOpen print_SMUX_ClosePDU 
 *    print_SMUX_OpenPDU print_SMUX_PDUs print_SMUX_RReqPDU print_SMUX_RRspPDU 
 *    print_SMUX_SOutPDU print_SMUX_SimpleOpen
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/SMUX_defs.h
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

#ifndef _SMUX_defs_
#define _SMUX_defs_

#include <isode/pepsy/SMUX_pre_defs.h>

#ifndef	lint
#define encode_SMUX_PDUs(pe, top, len, buffer, parm) \
    enc_f(_ZPDUsSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMUX_PDUs(pe, top, len, buffer, parm) \
    dec_f(_ZPDUsSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char **) parm)

#define print_SMUX_PDUs(pe, top, len, buffer, parm) \
    prnt_f(_ZPDUsSMUX, &_ZSMUX_mod, pe, top, len, buffer)
#define print_SMUX_PDUs_P    _ZPDUsSMUX, &_ZSMUX_mod

#define encode_SMUX_OpenPDU(pe, top, len, buffer, parm) \
    enc_f(_ZOpenPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMUX_OpenPDU(pe, top, len, buffer, parm) \
    dec_f(_ZOpenPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char **) parm)

#define print_SMUX_OpenPDU(pe, top, len, buffer, parm) \
    prnt_f(_ZOpenPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer)
#define print_SMUX_OpenPDU_P    _ZOpenPDUSMUX, &_ZSMUX_mod

#define encode_SMUX_SimpleOpen(pe, top, len, buffer, parm) \
    enc_f(_ZSimpleOpenSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMUX_SimpleOpen(pe, top, len, buffer, parm) \
    dec_f(_ZSimpleOpenSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char **) parm)

#define print_SMUX_SimpleOpen(pe, top, len, buffer, parm) \
    prnt_f(_ZSimpleOpenSMUX, &_ZSMUX_mod, pe, top, len, buffer)
#define print_SMUX_SimpleOpen_P    _ZSimpleOpenSMUX, &_ZSMUX_mod

#define encode_SMUX_ClosePDU(pe, top, len, buffer, parm) \
    enc_f(_ZClosePDUSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMUX_ClosePDU(pe, top, len, buffer, parm) \
    dec_f(_ZClosePDUSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char **) parm)

#define print_SMUX_ClosePDU(pe, top, len, buffer, parm) \
    prnt_f(_ZClosePDUSMUX, &_ZSMUX_mod, pe, top, len, buffer)
#define print_SMUX_ClosePDU_P    _ZClosePDUSMUX, &_ZSMUX_mod

#define encode_SMUX_RReqPDU(pe, top, len, buffer, parm) \
    enc_f(_ZRReqPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMUX_RReqPDU(pe, top, len, buffer, parm) \
    dec_f(_ZRReqPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char **) parm)

#define print_SMUX_RReqPDU(pe, top, len, buffer, parm) \
    prnt_f(_ZRReqPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer)
#define print_SMUX_RReqPDU_P    _ZRReqPDUSMUX, &_ZSMUX_mod

#define encode_SMUX_RRspPDU(pe, top, len, buffer, parm) \
    enc_f(_ZRRspPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMUX_RRspPDU(pe, top, len, buffer, parm) \
    dec_f(_ZRRspPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char **) parm)

#define print_SMUX_RRspPDU(pe, top, len, buffer, parm) \
    prnt_f(_ZRRspPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer)
#define print_SMUX_RRspPDU_P    _ZRRspPDUSMUX, &_ZSMUX_mod

#define encode_SMUX_SOutPDU(pe, top, len, buffer, parm) \
    enc_f(_ZSOutPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char *) parm)

#define decode_SMUX_SOutPDU(pe, top, len, buffer, parm) \
    dec_f(_ZSOutPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer, (char **) parm)

#define print_SMUX_SOutPDU(pe, top, len, buffer, parm) \
    prnt_f(_ZSOutPDUSMUX, &_ZSMUX_mod, pe, top, len, buffer)
#define print_SMUX_SOutPDU_P    _ZSOutPDUSMUX, &_ZSMUX_mod


#endif   /* lint */

#endif /* _SMUX_defs_ */
