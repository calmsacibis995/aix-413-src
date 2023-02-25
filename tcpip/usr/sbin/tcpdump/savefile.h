/* @(#)67	1.1  src/tcpip/usr/sbin/tcpdump/savefile.h, tcpip, tcpip411, GOLD410 8/10/93 17:11:56 */
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: SWAPLONG
 *		SWAPSHORT
 *
 *   ORIGINS: 26,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 * Header for offline storage info.
 * Extraction/creation by Jeffrey Mogul, DECWRL.
 *
 * Used to save the received packet headers, after filtering, to
 * a file, and then read them later.
 */

#ifndef _L_H_SAVEFILE
#define _L_H_SAVEFILE

/*
 * Each packet in the dump file is prepended with this generic header.
 * This gets around the problem of different headers for different
 * packet interfaces.
 */
struct packet_header {
	struct timeval ts;	/* time stamp */
	u_long len;		/* length this packet (off wire) */
	u_long caplen;		/* length of portion present */
};

/* true if the contents of the savefile being read are byte-swapped */
extern int sf_swapped;

/* macros for when sf_swapped is true: */
/*
 * We use the "receiver-makes-right" approach to byte order,
 * because time is at a premium when we are writing the file.
 * In other words, the file_header and packet_header records
 * are written in host byte order.
 * Note that the packets are always written in network byte order.
 *
 * ntoh[ls] aren't sufficient because we might need to swap on a big-endian
 * machine (if the file was written in little-end order).
 */
#define	SWAPLONG(y) \
((((y)&0xff)<<24) | (((y)&0xff00)<<8) | (((y)&0xff0000)>>8) | (((y)>>24)&0xff))
#define	SWAPSHORT(y) \
	( (((y)&0xff)<<8) | (((y)&0xff00)>>8) )


extern FILE *sf_readfile;	/* dump file being read from */
extern FILE *sf_writefile;	/* dump file being written to */

int sf_read_init();
int sf_read();
int sf_next_packet();
void sf_write_init();
void sf_write();
void sf_err();

#define SFERR_TRUNC		1
#define SFERR_BADVERSION	2
#define SFERR_BADF		3
#define SFERR_EOF		4 /* not really an error, just a status */

#endif _L_H_SAVEFILE
