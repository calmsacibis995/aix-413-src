static char sccsid[] = "@(#)51	1.17  src/bos/usr/sbin/netstat/drvstats.c, cmdnet, bos41J, 9520B_all 5/18/95 17:20:22";
/* 
 * COMPONENT_NAME: CMDNET Network Commands
 * 
 * FUNCTIONS: MSGSTR, drvstats 
 *
 * ORIGINS: 26 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <nl_types.h>
#include "netstat_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_NETSTAT,n,s)

#define HLEN	6
#define HISTENT 20

#include <sys/kinfo.h>
#include <sys/ndd.h>
#include <sys/errno.h>

extern int Zflag;

drvstats(nddname)
{
        int needed;
        int rlen;
        char * buf, tempchar[80], *tempc;
        int lim, kidpid;
        struct kinfo_ndd *knddp;
        char * next, nddcmd[80];
	extern int errno;

        if ((needed = getkerninfo(KINFO_NDD, 0, 0, nddname)) < 0) { 
		fprintf(stderr, 
			MSGSTR(ESTFAIL,
				"Could not retrieve network dd data.\n"));
		perror("getkerninfo-estimate"); exit(1);}

        if (needed == 0) { 
		fprintf(stderr, 
			MSGSTR(NONDD, 
				"Network Device Driver info not found\n"));
		exit(1);
	}
        if ((buf = malloc(needed)) == 0) {
		fprintf(stderr, MSGSTR(REQ_MEM_DENIED,"out of space\n"));
		exit(1);
	}
        if ((rlen = getkerninfo(KINFO_NDD, buf, &needed, nddname)) < 0) {
		fprintf(stderr, 
			MSGSTR(ACTFAIL,
				"Could not retrieve actual network dd data.\n")
			);
                perror("actual retrieval of ndd");
		exit(1);
	}

        lim = buf + needed;
        for (next = buf; next < lim; next += sizeof(struct kinfo_ndd)) {
                knddp = (struct kinfo_ndd *)next;

		if (knddp != NULL) {

			strcpy(&tempchar[0], knddp->ndd_name);
			tempc = (char *) &tempchar[0];
			while (!isdigit(*tempc))
				tempc++;
			*tempc = '\0';
			strcpy(nddcmd, "/usr/sbin/");
			strcat(nddcmd, (char *) &tempchar[0]);
			strcat(nddcmd, "stat");
			kidpid = fork();
			if (kidpid == 0) {
				if (Zflag)
					execl(nddcmd, nddcmd, "-r", knddp->ndd_name, 0);
				else 
					execl(nddcmd, nddcmd, knddp->ndd_name, 0);
				/*
			 	* Should not return from exec.  If it happens,
			 	* report an error.
			 	*/
				/* there is a scsi cdli interface that gets put
				** in the ndd table, so if we can't find a stat
				** program we don't want to alarm the user
				*/
				if ( errno != ENOENT ) {
					fprintf(stderr, MSGSTR(BADEXEC, 
						"Exec error for %s.  Errno = %d\n"),
						nddcmd, errno);
				}
				exit(1);
			}
			else if (kidpid > 0){
				wait(kidpid);
			}
			else {
				fprintf(stderr, 
					MSGSTR(BADFORK,
					"Fork for netstat command failed.\n"));
				perror("Fork for ndd stat command");
				exit(1);
			}
		}
        }
return(1);
}
