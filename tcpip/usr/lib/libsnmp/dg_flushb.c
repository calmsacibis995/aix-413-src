static char sccsid[] = "@(#)99	1.3  src/tcpip/usr/lib/libsnmp/dg_flushb.c, snmp, tcpip411, GOLD410 3/2/93 18:22:16";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: dg_flushb()
 *
 * ORIGINS: 27 60
 *
 * (C) Copyright International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/dg_flushb.c
 */

/* 
 * Contributed by NYSERNet Inc.  This work was partially supported by the
 * U.S. Defense Advanced Research Projects Agency and the Rome Air Development
 * Center of the U.S. Air Force Systems Command under contract number
 * F30602-88-C-0016.
 *
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */


#include <isode/psap.h>

/* ...from isode/psap/dg2ps.c */

struct ps_dg {
    int	    ps_fd;
    int	    ps_maxsize;

    struct ps_inout {
	struct qbuf *pio_qb;
	char   *pio_ptr;
	int	pio_cnt;

	IFP	pio_fnx;
    }	    ps_input,
	    ps_output;

    IFP	    ps_check;
};

/* 
 * flush the output buffers without attempting a sendto (like dg_flush())
 * returns OK if there was nothing to flush, or DONE if it flushed the
 * buffer.
 */
int
dg_flushb (ps)
PS ps;
{
    register struct ps_dg *pt = (struct ps_dg *) ps -> ps_addr;
    register struct ps_inout *po = &pt -> ps_output;
    register struct qbuf *qb = po -> pio_qb;

    if ((qb -> qb_len = po -> pio_ptr - qb -> qb_data) <= 0)
	return OK;

    po -> pio_ptr = qb -> qb_data, po -> pio_cnt = pt -> ps_maxsize;
    return DONE;
}
