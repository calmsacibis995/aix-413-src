/* @(#)72	1.1  src/nfs/usr/sbin/netdisk/xdr_custom.h, cmdnfs, nfs411, GOLD410 10/26/89 15:27:51 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */

struct futureslop { char dummy; };
extern bool_t xdr_futureslop();

struct nomedia { char dummy; };
extern bool_t xdr_nomedia();
