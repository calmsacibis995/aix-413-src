/* @(#)61	1.4  src/tcpip/usr/include/isode/pepsy/DSE_defs.h, isodelib7, tcpip411, GOLD410 4/5/93 13:19:24 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: decode_DSE_PSAPaddr encode_DSE_PSAPaddr print_DSE_PSAPaddr
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/pepsy/DSE_defs.h
 */

#ifndef _DSE_defs_
#define _DSE_defs_

#include <isode/pepsy/DSE_pre_defs.h>


#ifndef	lint
#define encode_DSE_PSAPaddr(pe, top, len, buffer, parm) \
    enc_f(_ZPSAPaddrDSE, &_ZDSE_mod, pe, top, len, buffer, (char *) parm)

#define decode_DSE_PSAPaddr(pe, top, len, buffer, parm) \
    dec_f(_ZPSAPaddrDSE, &_ZDSE_mod, pe, top, len, buffer, (char **) parm)

#define print_DSE_PSAPaddr(pe, top, len, buffer, parm) \
    prnt_f(_ZPSAPaddrDSE, &_ZDSE_mod, pe, top, len, buffer)
#define print_DSE_PSAPaddr_P    _ZPSAPaddrDSE, &_ZDSE_mod


#endif   /* lint */

#endif /* _DSE_defs_ */
