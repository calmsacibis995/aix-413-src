/* @(#)82	1.1  src/tcpip/usr/sbin/ipreport/spray.h, tcpras, tcpip411, GOLD410 10/24/91 18:15:21 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * (#) from SUN 1.5
 * 
 */

/* (#)spray.h	1.1 12/13/88 17:47:03 */
/*	(#)spray.h	1.2 88/05/08 4.0NFSSRC SMI	*/
/* (#)95	1.3  spray.h, cmdnfs, nfs320 9/21/89 13:40:29 */


#define SPRAYPROG 100012
#define SPRAYPROC_SPRAY 1
#define SPRAYPROC_GET 2
#define SPRAYPROC_CLEAR 3
#define SPRAYVERS_ORIG 1
#define SPRAYVERS 1

#define SPRAYOVERHEAD 86	/* size of rpc packet when size=0 */
#define SPRAYMAX 8845		/* related to max udp packet of 9000 */

int xdr_sprayarr();
int xdr_spraycumul();

struct spraycumul {
	unsigned counter;
	struct timeval clock;
};

struct sprayarr {
	int *data;
	int lnth;
};
