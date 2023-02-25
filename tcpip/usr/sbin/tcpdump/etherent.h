/* @(#)23	1.2  src/tcpip/usr/sbin/tcpdump/etherent.h, tcpip, tcpip411, GOLD410 10/13/93 09:50:04 */
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: none
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
 */
#ifndef _L_H_ETHERENT
#define _L_H_ETHERENT

/* File name of ethernet address data base. */

#ifdef _AIX
#define ETHERS_FILE "/etc/no_such_file_called_ethers_in_aix"
#else
#define ETHERS_FILE "/etc/ethers"
#endif /* _AIX */

struct etherent {
	u_char addr[6];
	char name[122];
};

struct etherent *next_etherent();

#endif _L_H_ETHERENT
