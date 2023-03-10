static char sccsid[] = "@(#)08	1.3  src/tcpip/usr/lib/libisode/log_tai.c, isodelib7, tcpip411, GOLD410 4/5/93 13:58:26";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: log_tai
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/log_tai.c
 */

/* log_tai.c - system tailoring routines */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/log_tai.c,v 1.2 93/02/05 17:05:13 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/log_tai.c,v 1.2 93/02/05 17:05:13 snmp Exp $
 *
 *
 * $Log:	log_tai.c,v $
 * Revision 1.2  93/02/05  17:05:13  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:30  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:23:16  mrose
 * Release 6.0
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
#include <string.h>
#include <isode/cmd_srch.h>
#include <isode/general.h>
#include <isode/manifest.h>
#include <isode/logger.h>

/*  */

#define	LG_FILE         1
#define	LG_SFLAGS       2
#define	LG_DFLAGS       3
#define	LG_LEVEL        4
#define	LG_SIZE         5
#define	LG_DLEVEL       6

static CMD_TABLE log_tbl[] = {
	"FILE",         LG_FILE,
	"SFLAGS",       LG_SFLAGS,
	"DFLAGS",       LG_DFLAGS,
	"LEVEL",        LG_LEVEL,
	"SLEVEL",       LG_LEVEL,
	"DLEVEL",       LG_DLEVEL,
	"SIZE",         LG_SIZE,
	0,              -1,
};

static CMD_TABLE log_lvltbl[] = {
	"NONE",		LLOG_NONE,
	"FATAL",        LLOG_FATAL,
	"EXCEPTIONS",   LLOG_EXCEPTIONS,
	"NOTICE",       LLOG_NOTICE,
	"TRACE",        LLOG_TRACE,
	"DEBUG",        LLOG_DEBUG,
	"PDUS",         LLOG_PDUS,
	"ALL",		LLOG_ALL,
	0,      -1
};

static CMD_TABLE log_flgtbl[] = {
	"CLOSE",        LLOGCLS,
	"CREATE",       LLOGCRT,
	"ZERO",         LLOGZER,
	"TTY",          LLOGTTY,
	0,              -1
};

/*  */

log_tai(lgptr, av, ac) /* for now only alter the level - files etc later */
LLog    *lgptr;
char    **av;
int     ac;
{
	register int i;
	register char *p;
	int	 val;

	for(i = 0; i < ac; i++)
	{
		if((p = index(av[i], '=')) == NULLCP)
			continue;
		*p++ = NULL;
		switch(cmd_srch(av[i], log_tbl))
		{
			case LG_LEVEL:
				val = cmd_srch(p,log_lvltbl);
				if (val != -1)
					lgptr->ll_events |= val;
				break;
			case LG_DLEVEL:
				val = cmd_srch(p,log_lvltbl);
				if (val != -1)
					lgptr->ll_events &= ~ val;
				break;
			case LG_FILE:
				lgptr->ll_file = strdup (p);
				break;
			case LG_SFLAGS:
				val = cmd_srch(p,log_flgtbl);
				if (val != -1)
					lgptr->ll_stat |= val;
				break;
			case LG_DFLAGS:
				val = cmd_srch(p,log_flgtbl);
				if (val != -1)
					lgptr->ll_stat &= ~ val;
				break;
			case LG_SIZE:
				lgptr->ll_msize = atoi(p);
				break;
		}
	}
}
