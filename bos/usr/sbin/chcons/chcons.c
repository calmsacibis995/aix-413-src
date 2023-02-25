static char sccsid[] = "@(#)68	1.15  src/bos/usr/sbin/chcons/chcons.c, cfgmethods, bos411, 9428A410j 1/5/94 11:57:05";
/*
 * COMPONENT_NAME: (CFGMETHODS) chcons.c - Change Console
 *
 * FUNCTIONS: main(), parse(), errexit()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	This command changes the current value of the default console
	in the ODM database. It will also save the current value in
	a "previous value" field in the database. 
	
	Interface:

	chcons pathname
		where pathname is the character device special file that 
		is to be the new default console.

*/


#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/mode.h>
#include <unistd.h>
#include <sys/types.h>
#include <nl_types.h>
#include <sys/cfgdb.h>
#include <sys/sysmacros.h>
#include <sys/devinfo.h>
#include <sys/device.h>
#include <sys/lft_ioctl.h>
#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <sys/console.h>
#include <locale.h>

#define CONMAJOR 	4
#define CONMINOR 	0

#include "console_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str)  catgets(catd, MS_CONSOLE, Num, Str)

nl_catd	catd;

main(argc,argv)
int	argc;
char	*argv[];
{
	char	prev_path[256];		/* previous console path */
	struct 	stat	new_stat;
	struct	CuAt	*cuat;		/* ptr to customized attribute Class */
	struct	PdAt	*pdatp; 	/* pointer to Predefined */
	struct listinfo stat_info;
	extern	int optind;
	extern	char *optarg;
	extern  int  opterr;		/* suppress getopt error messages */
	char	*atname, *atvalue;	/* attribute name and value */
	int	fildes;			/* file descriptor */
	int	rc, attr;		/* return code, attribute flag */
	int	i,c, errflg;
	char	*out_ptr = NULL;	/* savebase output buffer */
	char	*err_ptr = NULL;	/* savebase error buffer */
	int	runtime;		/* runtime flag for savebase */
	char	crit[128];		/* search criteria */
	struct  qry_devsw qdevsw;

	setlocale(LC_ALL, "");

	catd = catopen(MF_CONSOLE, NL_CAT_LOCALE);
	/*
	  Parse the command string. 
	*/
	opterr = 0;
	errflg = 0;
	attr = FALSE;
	while ((c = getopt(argc, argv, "a:")) != EOF)
	{
		switch (c)
		{
			case 'a':
			{
				if ((atname = strtok(optarg,"=")) != NULL) 
				{
				if ((strcmp(atname, "login") == 0) &&
					((atvalue = strtok(NULL," ")) != NULL))
				{
					
					if ((strcmp(atvalue, "enable") != 0) && 
					    (strcmp(atvalue, "disable") != 0))
					{	
						errflg++;
						break;
					}	
				}	
				else
				{	
					errflg++;
					break;
				}	
				attr = TRUE;
				break;
				}
			}	

			case '?':
				errflg++;
		}
	}	
#ifdef CFGDBG
	printf ("optind = %d \n", optind);
	printf ("attr = %d \n", attr);
	printf ("errflg = %d \n", errflg);
#endif
	if (errflg || ((attr == FALSE) && (argv[optind] == NULL)))
	{
		printf(MSGSTR(USAGECHC," usage: chcons [-a login=enable | disable] [pathname] \n"));
		exit(1);
	}
	/*
	  Initialize the ODM.
	*/
	if ( odm_initialize() < 0 )
	{
		fprintf(stderr,MSGSTR(CHCONE04,"chcons: failure accessing the device database (ODM) \n"));
		exit(1);
	}

	/* check on runtime mode for doing savebase when complete */
     	sprintf(crit,PHASE1_DISALLOWED);
	pdatp = (struct PdAt *)odm_get_list(PdAt_CLASS,crit,&stat_info,1,1);
	if ((int)pdatp == -1)
		errexit(MSGSTR(CHCONE04,"chcons: failure accessing the device database ODM \n"));
	else
		if ((int)pdatp != NULL)
			runtime = TRUE;
		else
			runtime = FALSE;

	/* get console device pathname */
	if (( cuat = getattr("sys0", "syscons", FALSE, &rc )) == NULL )
		errexit(MSGSTR(CHCONE06,"chcons: failure accessing the sys0 object in ODM \n"));

	strcpy(prev_path, cuat->value);
	/*
	  Get default console attribute.
	 */
	if (( cuat = getattr("sys0", "conslogin", FALSE, &rc ))
	    == NULL )
		errexit(MSGSTR(CHCONE06,"chcons: failure accessing the sys0 object in ODM \n"));
#ifdef CFGDBG
	printf ("old conslogin = %s \n", cuat->value);
#endif
	if (attr == TRUE)
	{	
		/*
		  Update device object.
		 */
		strcpy(cuat->value, atvalue);
		/*
		   If login is being enabled, ensure target is valid 
		 */
		if (strcmp(cuat->value, "enable") == 0)   
		{
			if (argv[optind] == NULL)
 				rc = chk_console(prev_path);
			else
 				rc = chk_console(argv[optind]);
			if (rc != 0)
				errexit(MSGSTR(CHCONE09,"chcons: Console login cannot be enabled because the console device \n\tis not an available lft or tty. \n"));
		}
		if (( rc = putattr(cuat)) < 0 )
		{
			errexit(MSGSTR(CHCONE06,"chcons: failure accessing the sys0 object in ODM \n"));
		}
		printf(MSGSTR(CHCONLOGIN,"chcons: console login: %sd, effective on next system boot\n"), atvalue);
	}	
/* if no pathname passed in then we are finished here */		
	if (argv[optind] == NULL)
	{	

		if (runtime == TRUE)
		{
#ifdef CFGDBG
	printf ("running savebase\n");
#endif
   			if ( rc = odm_run_method( "/usr/sbin/savebase", "", &out_ptr, 
				&err_ptr ))
			{
				errexit(MSGSTR(CHCONE07,"chcons: savebase of sys0 object failed \n"));
			}
		}
		odm_terminate();
		exit(0);
	}	
#ifdef CFGDBG
	printf ("pathname = %s \n", argv[optind]);
#endif
	/*
	  Test the validity of the pathname passed in to this command.
	*/
	if (strncmp(argv[optind], "/", 1) != 0)
	{
		errexit(MSGSTR(CHCONE01,"chcons: device or file pathname must be fully qualified \n"));	
	}
	new_stat.st_rdev = 0;
	rc = stat(argv[optind], &new_stat);
	if (rc != 0 && errno != ENOENT)
	{
		fprintf(stderr,MSGSTR(CHCONE02,"chcons: stat on %s failed, errno = %d\n"), argv[1], errno);
		odm_terminate();
		exit(1);
	}

	/* Ensure that the target is not a block device */

	if (new_stat.st_rdev != 0)	
	{
		qdevsw.devno = new_stat.st_rdev; 	
		sysconfig (SYS_QDVSW, &qdevsw, sizeof(qdevsw)); 
		if (qdevsw.status & DSW_BLOCK)
		{
			fprintf(stderr,MSGSTR(CHCONE10,"chcons: invalid request - the console cannot be redirected\n\t\t to a block device \n"));
			exit(1);
		}
	}
	/*
  	  Make sure the target device is not the console redirector itself.
	*/
	
	if (makedev(CONMAJOR, CONMINOR) == new_stat.st_rdev)
	{
		errexit(MSGSTR(CHCONE03,"chcons: invalid request - the console cannot be redirected to itself \n"));
	}
	if ((strcmp(cuat->value, "enable") == 0) &&  
 		(rc = chk_console(argv[optind]) != 0))
		errexit(MSGSTR(CHCONE08,"chcons: invalid request - Console login is enabled and \n\tthe specified device is not an available lft or tty. \n"));

	/*	
	  Get default console attribute.
	*/
	if (( cuat = getattr("sys0", "syscons", FALSE, &rc )) == NULL )
		errexit(MSGSTR(CHCONE06,"chcons: failure accessing the sys0 object in ODM \n"));

	strcpy(prev_path, cuat->value);
	strcpy(cuat->value, argv[optind]);
	
	/*
	  Update device object.
	*/
	if (( rc = putattr(cuat)) < 0 )
	{
		errexit(MSGSTR(CHCONE06,"chcons: failure accessing the sys0 object in ODM \n"));
	}

	/*
	  Get "previous console" attribute.
	*/
	if (( cuat = getattr("sys0", "sysconsp", FALSE, &rc )) == NULL )
		errexit(MSGSTR(CHCONE06,"chcons: failure accessing the sys0 object in ODM \n"));

	strcpy(cuat->value, prev_path);
	
	/*
	  Update device object.
	*/
	if (( rc = putattr(cuat)) < 0 )
	{
		errexit(MSGSTR(CHCONE06,"chcons: failure accessing the sys0 object in ODM \n"));
	}

	printf(MSGSTR(CHCONPATH,"chcons: console assigned to: %s, effective on next system boot\n"), argv[optind]);

	/*
	  Clean up and return successful.
	*/

	if (runtime == TRUE)
	{
#ifdef CFGDBG
	printf ("running savebase\n");
#endif
   		if ( rc = odm_run_method( "/usr/sbin/savebase", "", &out_ptr, 
			&err_ptr ))
		{
			errexit(MSGSTR(CHCONE07,"savebase of sys0 object failed"));
		}
	}
	odm_terminate();
	exit(0);

}



errexit(errstring)
char	*errstring;
{
	fprintf(stderr, errstring);

	/*
	  Terminate the ODM.
	*/
	odm_terminate();
	exit(1);
}



int
chk_console(path)
char *path;
{
	int	rc, return_code;
	char	crit[128];		/* ODM search criteria */
	struct  CuDv cudv;		/* customized device object storage */
	struct  Class cudv_class;	/* Class object */

	/* if console login is enabled check for validity of new target */
	return_code = 0; /* assume success */
	if ((strncmp(path, "/dev/lft", 8) == 0) || 
		(strncmp(path, "/dev/tty", 8) == 0)) 
	{
		 /* check status in database */
		sprintf(crit, "name = '%s'",path+5);
		rc = odm_get_obj(CuDv_CLASS, crit, &cudv, ODM_FIRST);
		if ( rc == -1 )
			errexit(MSGSTR(CHCONE04,"chcons: failure accessing the device database ODM \n"));

		if ( (rc == 0) || (cudv.status != AVAILABLE))
			return_code = -1; /* check failed */
	}
	else
 		return_code = -1; /* check failed */
	return(return_code);
}
