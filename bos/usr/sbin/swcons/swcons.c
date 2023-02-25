static char sccsid[] = "@(#)66 1.11  src/bos/usr/sbin/swcons/swcons.c, cfgmethods, bos411, 9428A410j 93/11/12 15:53:08";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: MSGSTR
 *		main
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
  
  Description: This high level command is used to change the current
			   setting for the default console. The change is made
			   via a sysconfig() call.

  Syntax:	swcons pathname
				where pathname is the full pathname of the desired 
				file.
  
*/   

#include <stdio.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/mode.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <nl_types.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/console.h>
#include <sys/device.h>

#define CONMAJOR	4
#define CONMINOR	0

#include "console_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str)  catgets(catd, MS_CONSOLE, Num, Str)

main(argc,argv)
int	argc;
char	*argv[];
{
	int rc;
	struct cons_config config;
	char format[255];								/* for messages */
	struct cfg_dd  cfgdata;			
	struct  qry_devsw qdevsw;
	struct stat change_stat;			/* file statistics */
	struct CuAt	*cuat;								/* customized attribute */
	int  fildes;
	char	current_default[255];			/* device pathnames */
	char	change_default[255];

	setlocale(LC_ALL, "");
	catd = catopen(MF_CONSOLE, NL_CAT_LOCALE);
	cfgdata.kmid = 0;
	cfgdata.devno = makedev(CONMAJOR,CONMINOR);
	cfgdata.cmd = CONSOLE_CFG;
	cfgdata.ddsptr = (char *) (&config);
	cfgdata.ddslen = sizeof(config);

	--argc;				/* skip progname */
	++argv;
	if (!strcmp(argv[0], "-c")) {	/* no controlling tty */
	    config.cmd = CONS_NCTTY;
	    rc = sysconfig(SYS_CFGDD, &cfgdata, sizeof(struct cfg_dd)); 
	    if (rc) {
		perror("sysconfig");
		return 1;
	    }
	    return 0;
	}

	if (!strcmp(argv[0], "+c")) {	/* may be controlling tty */
	    config.cmd = CONS_CTTY;
	    rc = sysconfig(SYS_CFGDD, &cfgdata, sizeof(struct cfg_dd)); 
	    if (rc) {
		perror("sysconfig");
		return 1;
	    }
	    return 0;
	}

	/*
	 Check argument list for validity.
	*/

	for ( ; argc > 0 && argv[0][0] == '-'; argc--, argv++)
	{
		printf(MSGSTR(USAGESW," usage: swcons [pathname] \n"));
		exit(1);
	}
	if ( argc > 1  )
	{
		printf(MSGSTR(USAGESW," usage: swcons [pathname] \n"));
		exit(1);
	}

/* process any input pathname */
	if (argc == 1)
	{
		strcpy(change_default, argv[0]);
		if (change_default[0] != '/')
		{
			fprintf(stderr,MSGSTR(SWCONE01,"swcons: device or file pathname must be fully qualified \n"));
			exit(1);
		}
		change_stat.st_rdev = 0;
		rc = stat(change_default, &change_stat);
		if ( rc != 0 && errno != ENOENT)
		{
			fprintf(stderr,MSGSTR(SWCONE02,"swcons: stat  on %s failed, errno = %d\n"), change_default, errno);
			exit(-1);
		}

		/* Ensure that the target is not a block device */

		if (change_stat.st_rdev != 0)	
		{
			qdevsw.devno = change_stat.st_rdev; 	
			sysconfig (SYS_QDVSW, &qdevsw, sizeof(qdevsw)); 
			if (qdevsw.status & DSW_BLOCK)
			{
				fprintf(stderr,MSGSTR(SWCONE06,"swcons: invalid request - the console cannot be redirected\n\t\t to a block device \n"));
				exit(1);
			}
		}

		/*
	  	Compare device numbers to ensure that the console is not being
	  	redirected to itself.
		*/

		if (makedev(CONMAJOR,CONMINOR) == change_stat.st_rdev)
		{
			fprintf(stderr,MSGSTR(SWCONE03,"swcons: invalid request - the console cannot be redirected to itself \n"));
			exit(1);
		}
	} /* end of if path name was provided */

		/*
	  	Set the current default console to the new default.
		Or query the driver for the default console if no arguments
		were provided.
		*/
	
	config.path = change_default;
	if (argc == 1)
		config.cmd = CONSETPRIM;
	else
		config.cmd = CONSGETDFLT;

	rc = sysconfig(SYS_CFGDD, &cfgdata, sizeof(struct cfg_dd)); 
	if ( rc != 0 )
	{
		fprintf(stderr,MSGSTR(SWCONE04,"swcons: switch command to driver failed, errno = %d\n"), errno);
		exit(1);
	}


	/*
	  Activate the new console.
	*/
	if (argc == 1)
	{
		config.cmd = CONS_ACTPRIM;
	}
	else
	{
		config.cmd = CONS_ACTDFLT;
	}
	rc = sysconfig(SYS_CFGDD, &cfgdata, sizeof(struct cfg_dd)); 
	if ( rc != 0 )
	{
		if (config.cmd == CONS_ACTPRIM)
		{
			config.cmd = CONSGETCURR;
			rc = sysconfig(SYS_CFGDD, &cfgdata,
			 sizeof(struct cfg_dd)); 
			if ( rc != 0 )
				fprintf(stderr,MSGSTR(SWCONE04,"swcons: switch command to driver failed, errno = %d\n"), errno);
			else
				fprintf(stderr,MSGSTR(SWCONE05,"swcons: unable to access new console selection \n\t returning console to: %s\n"), change_default);
			exit(1);
		}
		else
			fprintf(stderr,MSGSTR(SWCONE04,"swcons: switch command to driver failed, errno = %d\n"), errno);
		exit(1);
	} /* no error */

		if (config.cmd == CONS_ACTPRIM)
			printf(MSGSTR(SWCONPRIM,"swcons: console output redirected to: %s\n"), change_default);
		else
			printf(MSGSTR(SWCONDFLT,"swcons: returning console to: %s\n"), change_default);

	return(0);
}
