static char sccsid[] = "@(#)31	1.3  src/tcpip/usr/lib/libisode/rydsblock.c, isodelib7, tcpip411, GOLD410 4/5/93 16:22:36";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: finddsblk freedsblk losedsblk newdsblk
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rydsblock.c
 */

/* rydsblock.c - manage dispatch blocks */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rydsblock.c,v 1.2 93/02/05 17:10:43 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rydsblock.c,v 1.2 93/02/05 17:10:43 snmp Exp $
 *
 *
 * $Log:	rydsblock.c,v $
 * Revision 1.2  93/02/05  17:10:43  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:41:56  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:48  mrose
 * Release 5.0
 * 
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include <stdio.h>
#include <isode/rosy.h>

/*    DATA */

static int  once_only = 0;
static struct dspblk dspque;
static struct dspblk *DSHead = &dspque;

/*    DISPATCH BLOCKS */

struct dspblk  *newdsblk (sd, ryo)
int	sd;
struct RyOperation *ryo;
{
    register struct dspblk *dsb;

    dsb = (struct dspblk   *) calloc (1, sizeof *dsb);
    if (dsb == NULL)
	return NULL;

    dsb -> dsb_fd = sd;
    dsb -> dsb_ryo = ryo;

    if (once_only == 0) {
	DSHead -> dsb_forw = DSHead -> dsb_back = DSHead;
	once_only++;
    }

    insque (dsb, DSHead -> dsb_back);

    return dsb;
}

/*  */

freedsblk (dsb)
register struct dspblk *dsb;
{
    if (dsb == NULL)
	return;

    remque (dsb);

    free ((char *) dsb);
}

/*  */

struct dspblk   *finddsblk (sd, op)
register int	sd,
		op;
{
    register struct dspblk *dsb;

    if (once_only == 0)
	return NULL;

    for (dsb = DSHead -> dsb_forw; dsb != DSHead; dsb = dsb -> dsb_forw)
	if (dsb -> dsb_fd == sd && dsb -> dsb_ryo -> ryo_op == op)
	    return dsb;

    return NULL;
}

/*  */

losedsblk (sd)
register int	sd;
{
    register struct dspblk *dsb,
                           *ds2;

    if (once_only == 0)
	return;

    for (dsb = DSHead -> dsb_forw; dsb != DSHead; dsb = ds2) {
	ds2 = dsb -> dsb_forw;

	if (dsb -> dsb_fd == sd)
	    freedsblk (dsb);
    }
}
