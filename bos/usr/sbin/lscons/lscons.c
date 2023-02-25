static char sccsid[] = "@(#)26 1.3  src/bos/usr/sbin/lscons/lscons.c, cfgmethods, bos411, 9428A410j 93/11/12 15:52:07";
/*
 * COMPONENT_NAME: (CFGMETHODS) list console
 *
 * FUNCTIONS: lscons [-s] [-b] [-d]
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 */

#include	<stdio.h>
#include	<locale.h>
#include 	<sys/cfgdb.h>
#include 	<sys/sysmacros.h>
#include 	<sys/device.h>
#include 	<sys/cfgodm.h>
#include 	<sys/console.h>
#include 	<sys/sysconfig.h>

#define CONMAJOR	4
#define CONMINOR	0

#include "console_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str)  catgets(catd, MS_CONSOLE, Num, Str)

char	*ttyname();
extern int	optind;
extern int 	opterr;

main(argc, argv)
int argc;
char **argv;
{
	register int	opt, silent=FALSE, getdefault = FALSE;
	register int	errflg = 0, getboot = FALSE;
	char	consname[512];		/* name for current console device */
	char	dfltname[512];		/* name for default console  device */
	struct	CuAt	*cuat;		/* ptr to customized attribute Class */
	char	*ttyp;
	struct cons_config config;
	struct cfg_dd  cfgdata;		
	uchar	exitcode;
	int	rc;

	setlocale(LC_ALL, "");
	catd = catopen(MF_CONSOLE, NL_CAT_LOCALE);

	exitcode = 0;
	while((opt = getopt(argc, argv, "sbd")) != EOF)
		switch(opt) {

		case 's':
			silent = TRUE;
			break;

		case 'b':
			if (getdefault != TRUE)
				getboot = TRUE;
			else
				errflg++;
			break;

		case 'd':
			if (getboot != TRUE) 
				getdefault = TRUE;
			else
				errflg++;
			break;

		case '?':
			errflg++;
		}
	if (errflg)
	{
		printf(MSGSTR(USAGELSC," usage: lscons [-s] [-b] [-d] \n"));
		exit(3);
	}	
	/*
	  Initialize the ODM.
	*/
	if (getboot == TRUE)	
	{
		if ( odm_initialize() < 0 )
		{
			fprintf(stderr,MSGSTR(LSCONE01,"chcons: failure accessing the device database (ODM) \n"));
			exit(4);
		}
		/*
		  Get console device name from database.
		 */
		if (( cuat = getattr("sys0", "syscons", FALSE, &rc ))
		    == NULL )
			fprintf(stderr,MSGSTR(LSCONE01,"lscons: failure accessing the sys0 object in ODM \n"));
		else
		{
			odm_terminate();
			if (!silent)
				 /* print next boot name */
				printf("%s\n",cuat->value);
		}	
	}
	/* get current tty name */
	if ((ttyp = ttyname(0)) == NULL)
		exitcode = 1;
	/* get current console path name */
	cfgdata.kmid = 0;
	cfgdata.devno = makedev(CONMAJOR,CONMINOR);
	cfgdata.cmd = CONSOLE_CFG;
	cfgdata.ddsptr = (char *) (&config);
	cfgdata.ddslen = sizeof(config);
	config.path = consname;
	config.cmd = CONSGETCURR;
	rc = sysconfig(SYS_CFGDD, &cfgdata, sizeof(struct cfg_dd)); 
	if ( rc != 0 )
	{
		fprintf(stderr,MSGSTR(LSCONE02,"lscons: system error occurred accessing console driver \n"));
		exit(4);
	}		

	/* get console name at last boot time */	
	config.path = dfltname;
	config.cmd = CONSGETDFLT;
	rc = sysconfig(SYS_CFGDD, &cfgdata,
		sizeof(struct cfg_dd)); 
	if ( rc != 0 )
	{
		fprintf(stderr,MSGSTR(LSCONE02,"lscons: system error occurred accessing console driver \n"));
		exit(4);
	}
	if (!silent && !getboot && !getdefault)
		printf("%s\n",consname);  /* print current console name */
	else
		if (!silent && getdefault)
			printf("%s\n",dfltname);

	if (exitcode == 0)
	{	
		if(strcmp(ttyp,consname) != 0)
			if(strcmp(ttyp,dfltname) == 0)
				exitcode = 2;
			else
				exitcode = 1;
	}	
	exit(exitcode);	
}



