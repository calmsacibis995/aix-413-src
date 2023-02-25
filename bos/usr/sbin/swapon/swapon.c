static char sccsid[] = "@(#)64	1.3.1.2  src/bos/usr/sbin/swapon/swapon.c, cmdps, bos411, 9428A410j 5/5/94 18:06:44";
/*
 * COMPONENT_NAME: (CMDPS) paging space commands
 *
 * FUNCTIONS: swapon
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * char copyright[] =
 *  " Copyright (c) 1980 Regents of the University of California.\n\
 * All rights reserved.\n";
 * static char sccsid[] = "swapon.c	5.2 (Berkeley) 3/4/86";
 */

#include <stdio.h>
#include <errno.h>
#include <IN/AFdefs.h>
#include "cmdps.h"
#include <lvm.h>

/* Limits.
 */
#define MAXDEV	16
#define	MAXSIZE	80
#define MAXREC	500
#define	MAXATTR	100

/* Common variables.
 */
static char *cmd = "";		/* command name		*/
static char *stanzafile;

/* Internal routines.
 */
static int get_devnames (int *num, char names[][MAXSIZE]);
static void usage(int msgnum);
static int check_dev(char *name);

/* Externs.
 */
extern char *basename();

main(int argc, char *argv[])
{
	int stat = 0;		/* exit status				*/
	int aflg = 0;		/* command line option flag		*/
	int opt;		/* command line option			*/
	char devname[MAXDEV][MAXSIZE];	/* array of device names	*/
	char errstr[MAXSIZE];	/* error string				*/
	int cnt, devcnt;	/* count of device names		*/
	
	(void) setlocale (LC_ALL, "");

	nls_catd = catopen(MF_CMDPS,NL_CAT_LOCALE);

	/* Determine command name.
	 */
	cmd = basename(argv[0]);

	/* Parse command line flags.
	 */
	while ((opt = getopt(argc, argv, "a")) != EOF)
	{
		switch (opt)
		{
		case 'a':	/* automatic */
			aflg++;
			break;

		default:	/* bad parameter */
			usage(SWAPON_USAGE);
			break;
		}
	}
     
	/* Get list of device names to swapon.
	 */
	if (aflg)
	{
		/* Get device names from automatic list.
		 */
		if ((argc - optind) != 0)
		{
			/* Parameters in addition to -a specified.
			 */
			usage(SWAPON_USAGE);
		}

		if (get_devnames(&devcnt, devname))
		{
			/* Could not access automatic list.
			 */
			fprintf(stderr, MSGSTR(WARN_AUTO),
				cmd, stanzafile);
			exit(1);
		}
	}
	else
	{
		/* Get device names from command line.
		 */
		devcnt = argc - optind;
		if (devcnt == 0)
		{
			/* Neither -a nor device name specified.
			 */
			fprintf(stderr, MSGSTR(SWAPON_AORDN), cmd);
			usage(SWAPON_USAGE);
		}

		for (cnt=0; cnt<devcnt; cnt++)
			strcpy(devname[cnt], argv[optind+cnt]);
	}

	/* Now issue swapon for each device.
	 */
	for (cnt=0; cnt<devcnt; cnt++)
	{
		if (check_dev(devname[cnt]))
		{
			stat = 1;
			sprintf(errstr, MSGSTR(SWAPON_FAIL),
				cmd, devname[cnt]);

		}
		else if (swapon(devname[cnt]) == -1)
		{
			/* Set exit status to failure.
			 */
			stat = 1;

			/* Display errno message.
			 */
			sprintf(errstr, MSGSTR(SWAPON_FAIL),
				cmd, devname[cnt]);
			perror(errstr);
		}
		else
		{
			/* If swapon -a, display device name activated.
			 */
			if (aflg)
				fprintf(stdout, MSGSTR(SWAPON_ADD),
					cmd, devname[cnt]);
		}
	}

	exit(stat);
}

/*
 * NAME: get_devnames
 *
 * FUNCTION: Retrieve paging space device names from automatic list
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION):
 *
 * (DATA STRUCTURES):
 *
 * RETURNS:
 *
 *   0 if successful
 *   1 if not successful
 */
static
int
get_devnames (int *num, char names[][MAXSIZE])
{
	ATTR_t	stanza;		/* stanza in attribute file		*/
	AFILE_t	af;		/* attribute file			*/
	char *attrval;		/* value of attribute			*/

	/* Initialize to no names found.
	 */
	*num = 0;

	/* Set stanza file pathname from environment variable
	 * or default.
	 */
	if (!(stanzafile = (char *)getenv(ENVSWAP)))
		stanzafile = ETCSWAP;

	/* Open stanza file.
	 */
	if ((af = (AFILE_t) AFopen(stanzafile,MAXREC,MAXATTR)) == NULL)
		return(1);

	/* Get 'dev = ' attribute from each stanza.
	 */
	while (stanza = AFnxtrec(af))
	{
		if (attrval = AFgetatr(stanza, "dev"))
		{
			strncpy(names[*num], attrval, MAXSIZE);
			(*num)++;
		}
	}

	AFclose(af);
	return(0);
}

/*
 * NAME: usage
 *
 * FUNCTION: display specified usage message and exit
 */
static
void
usage (int msgnum)
{
	fprintf(stderr, MSGSTR(msgnum), cmd);
	exit(1);
}

static int check_dev(char *name)
{
    int rc;
    struct queryvgs *vgs_query;
    int vg_num;
    struct unique_id vg_id;
    struct queryvg *vg_query;
    struct lv_array *lv, *lv_end;
    struct lv_id lv_id;
    struct querylv *lv_query;
    struct pp *pp, *pp_end;
    struct pv_array *pv, *pv_end;
    int mirror;
    int kmid;

    /*
     * Find the kmid of the logical volume manager
     */
    if ((kmid = loadext("/etc/drivers/hd_pin", 0, 1)) < 0)
	return 0;

    /*
     * Find all volume groups in system.
     */
    if (lvm_queryvgs(&vgs_query, kmid) < 0)
	return 0;

    /*
     * For each volume group
     */
    for (vg_num = 0; vg_num < vgs_query->num_vgs; ++vg_num) {
	/*
	 * Query to get volume group info.
	 */
	vg_id = vgs_query->vgs[vg_num].vg_id;
	if (lvm_queryvg(&vg_id, &vg_query, (char *)0) < 0)
	    continue;

	/*
	 * Search logical volumes for the one we want
	 */
	for (lv_end = (lv = vg_query->lvs) + vg_query->num_lvs;
	     lv < lv_end && strcmp(name, lv->lvname);
	     ++lv);

	/*
	 * If we did not find it then just continue.
	 */
	if (lv == lv_end)
	    continue;

	/*
	 * For the logical volume we want, get the logical volume
	 * info.
	 */
	lv_id = lv->lv_id;
	if (lvm_querylv(&lv_id, &lv_query, (char *)0) < 0)
	    continue;

	/*
	 * For each mirror in the logical volume.
	 */
	for (mirror = 0; mirror < LVM_NUMCOPIES; ++mirror) {
	    /*
	     * If the mirror does not exist, then just continue to
	     * next mirror.
	     */
	    if (!(pp = lv_query->mirrors[mirror]))
		continue;

	    /*
	     * For each pp in mirror.
	     */
	    for (pp_end = pp + lv_query->currentsize; pp < pp_end; ++pp) {
		/*
		 * Find the pv (physical volume) for this pp
		 */
		for (pv_end = (pv = vg_query->pvs) + vg_query->num_pvs;
		     pv < pv_end &&
		     (pv->pv_id.word1 != pp->pv_id.word1 ||
		      pv->pv_id.word2 != pp->pv_id.word2 ||
		      pv->pv_id.word3 != pp->pv_id.word3 ||
		      pv->pv_id.word4 != pp->pv_id.word4);
		     ++pv);
		/*
		 * If we find the pv, make sure it is active.  If it
		 * isn't we quit with an unhappy return because not
		 * all the pv's that are needed are active at this
		 * time.
		 */
		if (pv != pv_end && pv->state != LVM_PVACTIVE)
		    return 1;
	    }
	}
	/*
	 * If we make it to this point, we have found the logical
	 * volume and we have checked all of the partitions we need
	 * and they are all on an active physical volume so we can
	 * return happy at this point.  There is no need to check the
	 * other logical volumes or volume groups.
	 */
	return 0;
    }
    return 0;
}
