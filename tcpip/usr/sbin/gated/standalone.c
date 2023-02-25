static char sccsid[] = "@(#)28	1.1  src/tcpip/usr/sbin/gated/standalone.c, tcprouting, tcpip411, GOLD410 12/6/93 17:27:44";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF2
 *		__PF3
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  standalone.c,v 1.1 1993/03/08 16:50:33 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */

#include "include.h"

/* Support for stand-alone programs (ripquery, gdc) */

struct gtime timer_time;

const bits ll_type_bits[] = {
    { LL_OTHER,		"Unknown" },
    { LL_8022,		"802.2" },
    { LL_X25,		"X.25" },
    { LL_PRONET,	"ProNET" },
    { LL_HYPER,		"HyperChannel" }
};


void
task_assert __PF2(file, const char *,
		  line, int)
{
    fprintf(stderr,
	    "Assertion failed: file \"%s\", line %d",
	    file,
	    line);

    /* Exit with a core dump */
    abort();
}


void_t
task_mem_malloc __PF2(tp, task *,
		      size, size_t)
{
    void_t p;

    p = (void_t) malloc(size);
    if (!p) {
	(void) fprintf(stderr,
		       "malloc: Can not malloc(%d)",
		       size);
	abort();
    }

    return p;
}


void_t
task_mem_calloc __PF3(tp, task *,
		      number, u_int,
		      size, size_t)
{
    void_t p;

    p = (void_t) calloc(number, size);
    if (!p) {
	(void) fprintf(stderr,
		       "calloc: Can not calloc(%d, %d)",
		       number,
		       size);
	abort();
    }

    return p;
}


/*ARGSUSED*/
void
task_mem_free __PF2(tp, task *,
		    p, void_t)
{
    if (p) {
	free((caddr_t) p);
    }
}


/*
 * ------------------------------------------------------------------------
 * 
 * 	GateD, Release 3
 * 
 * 	Copyright (c) 1990,1991,1992,1993 by Cornell University
 * 	    All rights reserved.
 * 
 * 	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY
 * 	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * 	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * 	AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	Royalty-free licenses to redistribute GateD Release
 * 	3 in whole or in part may be obtained by writing to:
 * 
 * 	    GateDaemon Project
 * 	    Information Technologies/Network Resources
 * 	    200 CCC, Garden Avenue
 * 	    Cornell University
 * 	    Ithaca, NY  14853-2601  USA
 * 
 * 	GateD is based on Kirton's EGP, UC Berkeley's routing
 * 	daemon	 (routed), and DCN's HELLO routing Protocol.
 * 	Development of GateD has been supported in part by the
 * 	National Science Foundation.
 * 
 * 	Please forward bug fixes, enhancements and questions to the
 * 	gated mailing list: gated-people@gated.cornell.edu.
 * 
 * 	Authors:
 * 
 * 		Jeffrey C Honig <jch@gated.cornell.edu>
 * 		Scott W Brim <swb@gated.cornell.edu>
 * 
 * ------------------------------------------------------------------------
 * 
 *       Portions of this software may fall under the following
 *       copyrights:
 * 
 * 	Copyright (c) 1988 Regents of the University of California.
 * 	All rights reserved.
 * 
 * 	Redistribution and use in source and binary forms are
 * 	permitted provided that the above copyright notice and
 * 	this paragraph are duplicated in all such forms and that
 * 	any documentation, advertising materials, and other
 * 	materials related to such distribution and use
 * 	acknowledge that the software was developed by the
 * 	University of California, Berkeley.  The name of the
 * 	University may not be used to endorse or promote
 * 	products derived from this software without specific
 * 	prior written permission.  THIS SOFTWARE IS PROVIDED
 * 	``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * 	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * 	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
