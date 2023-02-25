static char sccsid[] = "@(#)09  1.4.1.7  src/bos/usr/sbin/install/sysck/sysck.c, cmdinstl, bos411, 9428A410j 2/17/94 17:22:58";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/fullstat.h>
#include <sys/types.h>
#include <sys/audit.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <locale.h>
#include <usersec.h>
#include "sysdbio.h"
#include "sysmsg.h"
#include "sysaudit.h"

/*
 * Sysck - System configuration checker
 *
 *	Define and check proper system installation and configuration.
 *
 */

/*
 * Program names
 *
 * The Checksum program may be overriden by a value in the TCB file
 */

char	*Aclget = "/bin/aclget";
char	*Aclput = "/bin/aclput";
char	*Pclget = "/bin/privget";
char	*Pclput = "/bin/privput";
char	*Checksum = "/bin/sum -r < ";
int	BuiltInSum = 1;

/*
 * Argument flags and optional arguments
 */

int     Dont_Remove_ACL = FALSE;
int	iflg = 0;	/* install a stanza file of objects                 */
int	Rflg = 0;	/* use RootPath instead of "/"                      */
int	Nflg = 0;	/* Do not update the swvpd database                 */
int	uflg = 0;	/* uninstall a stanza file of objects               */
int     sflg = 0;       /* save odm inventory entries that were changed     */
int	rootflg = 0;	/* use the root part of the vpd                     */
int	usrflg = 0;	/* use the usr part of the vpd                      */
int	shareflg = 0;	/* use the share part of the vpd                    */
int	vflg = 0;	/* If set check the checksums			    */

char	*RootPath;
int	RootPathLen;
int	verbose = 1;
int	query = 1;	/* -t was specified                                 */

/*
 * Forward declarations and external functions
 */

extern	int	optind;
extern	char	*optarg;

/*
 * Global data
 */

struct	tcbent	**tcb_table;	/* array of structures describing the TCB    */
char	**novfs;		/* list of file systems to ignore            */
gid_t	*setgids;		/* group IDs to test for                     */
uid_t	*setuids;		/* user IDs to test for                      */
int	tcb_cnt;		/* number of valid entries in tcb_table      */
int	tcb_max;		/* number of slots in tcb_table              */

/*
 * NAME:	main
 *
 * FUNCTION:	Process arguments for sysck command
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Invoked by another user process.  This is a command.
 * ENVIRONMENT VARIABLES:
 *	INUROOT if not null specifies an alternate root.
 *	INUTREE if not null specifies the part of the tree being checked.
 *	INUVERIFY An indicator to sysck do check checksums:
 *		1 - yes check checksums
 *		0 - do not check checksums
 *	INUNOVPD Indicates to sysck not to update the swvpd
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

main (argc, argv)
int	argc;
char	**argv;
{
	int	arg;
	int	rc;
	char	*inventory_file;
	char	*tmpenv;

	/*
	 * Turn off auditing for this process.  It will cut its
	 * own audit records.
	 */

	auditproc (0, AUDIT_STATUS, AUDIT_SUSPEND, 0);
	(void) setlocale (LC_ALL, "");

	/*
	 * Initialize the NLS message catalogs
	 */

        catd = catopen(MF_SYSCK,0);
        /*
         * Set sysck program name for messages
         */
        sysck_pn = argv[0];

	if (strchr ((char *)getenv ("SYSCK_FLAGS"), 'F') != (char *) 0)
	 {
	   /*
	    * Don't delete the ACL for a file who's checksum or
	    * size has changed.  This is useful when installp is
	    * doing a REJECT operation, since we may be restoring
	    * a file that is wrong, but it was wrong long ago.
	    * (The administrator may have changed this file after
	    * installing).  We don't want to disable this file
	    * completely, just warn the customer about it.
	    */

	Dont_Remove_ACL = TRUE;
	 }
	
	/*
	 * Process each argument in turn.
	 *
	 *	-i - test if next argument is -f, do -f processing for
	 *	     LPP installation.  Differs from -a version.
	 *	-f - read each stanza from next argument and add.  must
	 *	     occur with -a or -i option.
	 *	-u - test if next argument is -f, do -f processing for
	 *	     LPP uninstallation.  Differs from -d version.
         *      -s - save inventory entries after they are deleted.
	 *	-R - uses an alternate root supply by the user.
	 *	-N - Does not update the swvpd database.
	 * 	-v - Check the checksums of files.
         *           the default is to not check the checksums.
	 */

        while ((arg = getopt (argc, argv, "f:O:R:s:Niuvadpynt")) != EOF) {
		switch (arg) {

			case 'O':
				if (optarg[1] != '\0') {
/* Put note about r|u|s must be used */ 
                                        MSG_S(MSG_Usage,DEF_Usage,0,0,0,0);
                                        exit (EINVAL);
                                }
				if (optarg[0] == 'r')
					rootflg++;
				if (optarg[0] == 'u')
					usrflg++;
				if (optarg[0] == 's')
					shareflg++;
				break;
			/*
			 * Use the argument of the -R option as
                         * root instead of "/"
			 */

			case 'R':
				Rflg++;
				RootPath = optarg;
				if (validate_name(RootPath)) {
                                   MSG_S(MSG_Illegal_Root,DEF_Illegal_Root,
                                         RootPath,0,0,0);
                                   MSG_S(MSG_Usage,DEF_Usage,0,0,0,0);
                                   exit (1);
                                }

				RootPathLen = strlen(RootPath);
				break;

			/*
			 * Do not update the swvpd database
			 */

			case 'N':
				Nflg++;
				break;
			/*
			 * Install - install a collection of objects using
			 * the file after the -f flag as input.
			 */

			case 'i':

				iflg++;

				if (optind >= argc || argv[optind][0] != '-') {
                                        MSG_S(MSG_Usage,DEF_Usage,0,0,0,0);
                                        exit (EINVAL);
                                }

				break;

			/*
			 * Uninstall - remove a collection of objects
			 * which were added with the -i option earlier
			 */

			case 'u':

				uflg++;

				if (optind >= argc || argv[optind][0] != '-') {
                                     MSG_S(MSG_Usage,DEF_Usage,0,0,0,0);
                                        exit (EINVAL);
                                }

				break;

			/*
			 * Use a file - the next argument must be a file
			 * name of a file containing stanzas to be added
			 * to or installed in the TCB file.
			 */

			case 'f':
				inventory_file = optarg;
				break;

                        /* Save the odm inventory entries that were changed */
                        case 's':
                                save_file = optarg;
                                sflg++;
                                break;

			/*  Check the checksums for files */
		
			case 'v':
				vflg++;
				break;

/*******************************************************/
			case 'a':
			case 'd':
			case 'p':
			case 'y':
			case 'n':
			case 't':
				 if (optind == 2)
				 	execv("/bin/tcbck",argv);
                                 else {
                                        MSG_S(MSG_Usage,DEF_Usage,0,0,0,0);
                                        exit (EINVAL);
                                 }
				break;
/*******************************************************/

			default:	/* Unknown flag */
                                MSG_S(MSG_Usage,DEF_Usage,0,0,0,0);
                                exit (EINVAL);
		}
	}	/* end of parsing arguments */

	/*
 	 * Get any environment variables that are set
         */

	/* get INUROOT if -R was not specified on the command line */
	if (!Rflg) {
		RootPath = (char *)getenv("INUROOT");
		if (RootPath != (char *) NULL) {
			Rflg++;
			if (validate_name(RootPath)) {
                                   MSG_S(MSG_Illegal_Root,DEF_Illegal_Root,
                                         RootPath,0,0,0);
                                   exit (1);
                        }
			RootPathLen = strlen(RootPath);
		}
	}

	/* get INUTREE if -O was not specified on the command line */
	if (!rootflg && !usrflg && !shareflg) {
		tmpenv = (char *)getenv("INUTREE");	
		if (tmpenv != (char *) NULL) 
			if (tmpenv[0] == 'M')
				rootflg++;
			else if (tmpenv[0] == 'U')
				usrflg++;
			else if (tmpenv[0] == 'S')
				shareflg++;
	}

	/* get INUVERIFY if -v was not specified on the command line */
	if (!vflg) {
		tmpenv = (char *)getenv("INUVERIFY");
		if (tmpenv != (char *) NULL) 
			if (tmpenv[0] == '1')
				vflg++;
	}

	/* get INUNOVPD if -N was not specified on the command line */
	if (!Nflg) {
		tmpenv = (char *)getenv("INUNOVPD");
		if (tmpenv != (char *) NULL) 
			if (tmpenv[0] == '1')
				Nflg++;
	}
	/*
	 * Expect one more argument - the name of the LPP to install -
	 * for -i and possibly -u.  -u has an optional argument.
	 */
	if (iflg || uflg) {
		if (((iflg) && (optind + 1 != argc)) || 
		    ((uflg) && (argc - optind > 1)))  {
                       MSG_S(MSG_Usage,DEF_Usage,0,0,0,0);
                       exit (EINVAL);
                }
                if (iflg)
                        rc = install_tcbents (inventory_file,argv[optind]);
                else {
		     if ( argc == optind ) {
                        rc = del_tcbents (inventory_file, NULL);
		     }  else {
                        rc = del_tcbents (inventory_file, argv[optind]);
		     }
					  
		}
                exit(rc?rc:0);

        } else {
                MSG_S(MSG_Usage,DEF_Usage,0,0,0,0);
                exit (EINVAL);
       }
}
