static char sccsid[] = "@(#)27	1.11.1.2  src/bos/usr/sbin/lvm/intercmd/lvgenminor.c, cmdlvm, bos41J, bai15 3/27/95 00:09:49";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *	getflg,ck_required,main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*
 * FILE NAME: lvgenminor.c 
 *
 * FILE DESCRIPTION: Source file for lvgenminor command.  
 *	"lvgenminor" provides an intermediate level of command interface.  
 *	"Lvgenminor" returns a minor number.
 *
 *	For additional information on this program and the other 
 *	lvm immediate level command programs see the specification 
 *	"Logical Volume Manager Intermediate Level Command Interface" 
 *	and also the interface documentation to the lvm library functions.
 *         
 * RETURN VALUE DESCRIPTION:
 *				0   Command Successfully completed
 *			        1   Unsuccessful return
 *
 * EXTERNAL PROCEDURES CALLED: genminor, odm_initialize, odm_terminate.                                  
 *
 * EXTERNAL VARIABLES: optind, optarg.
 * 
 * FUNCTION_NAMES: main, getflg.
 *
 */

/* Header and Include Files */
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>
#include <sys/signal.h>
#include <odmi.h>
#include "cmdlvm.h"
#include "cmdlvm_msg.h"		/* include files for message tests */

extern int *genminor();		/* cfg routine to generate a minor number */
extern odm_initialize();	/* cfg routine to initialize ODM database */
extern odm_terminate();		/* cfg routine to terminate ODM gracefully */
extern char * odm_set_path();	/* odm rountine to set config path */


/*
 * GLOBAL VARIABLES:
 */
extern char *Prog;			/* points to name of the current Program */
extern nl_catd  scmc_catd;   		/* Cat descriptor for scmc conversion */

/* external variables */
extern int optind;		/* index to next argv argument */
				/* - incremented by getopt      */
extern char *optarg;		/* pointer to command line argument string */
				/* - set by getopt  */

#define ODMERROR -1

/* Pointer to command line parameters associated with */
/* each command line option.   */
static char *p_value = " ";
 
/* Each flag corresponds to a command option */
/* - flag true if option specified on command line */
static int p_flag = FALSE;

/* 
 * GLOBAL STRUCTURES:
 */ 
struct sigaction action;		/* parameters for sigaction */
struct sigaction oldaction;		/* parameters for sigaction */

/*
 * NAME: getflg
 *
 * FUNCTION: Parse command line options until the first 
 *	non-option is encountered.
 *
 * (NOTES:) For each option in the command line, a global 
 *	flag is set and a global
 *	value is loaded with any accompaning command argument.
 *
 * RETURN VALUE DESCRIPTION: Return negative value (-1) if illegal option found.
 *		Return positive value (1) if options parsed correctly.
 *		Return zero if no command options found.
 */

int getflg(argc,argv)

int argc;	 /* number of command line string in referenced by argv */
char **argv;	 /* array of command line strings */
{
	int rc; 		/* getopt return value */

	/* if no command line options to parse then return */
	if (!argc) return(0);	

	/* switch on each option contained in the command line */
	while ((rc = getopt(argc, argv, "p:")) != EOF) {
		switch (rc) {
		case 'p':
			p_flag = TRUE;
			p_value = optarg;
			break;
		case '?':
			fprintf(stderr,lvm_msg(GENMIN_USE),Prog);
			return(-1);
		}
	}
	return(1);
}
/*
 * NAME: ck_required
 *
 * DESCRIPTION: Check input flag indicators to determine if all required fields
 *	have been received.
 *
 * RETURN VALUE DESCRIPTION: If zero then no error.  If not zero then error 
 *	(the non-zero value indicates which required flag was not set).
 *
 * INPUT: Argument buffer.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			-1:  If illegal option found.
 *			 0:  No error found.               
 */

static int ck_required(argbuf)
char **argbuf;
{

	/* If there was no argument on the command line then error */
	if (argbuf[0] == NULL) { 
		fprintf(stderr,lvm_msg(MISSING_PARMS),Prog);
		fprintf(stderr,lvm_msg(GENMIN_USE),Prog);
		return(ILLEGALSYNTAX);
	}
	/* If there was no argument on the command line then error */
	if (argbuf[1] == NULL) { 
		fprintf(stderr,lvm_msg(MISSING_MAJNUM),Prog);
		fprintf(stderr,lvm_msg(GENMIN_USE),Prog);
		return(ILLEGALSYNTAX);
	}
	return(SUCCESS);
}
/*
 * NAME: main
 *
 * FUNCTION: lvgenminor command returns a minor number for a device, either a
 *	logical volume or volume group.
 *
 * RETURN VALUE DESCRIPTION: 0 - SUCCESS                         
 *                           1 - FAILURE 
 */

main(argc,argv)
int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
char **argbuf;
int rc = 0;	 	/* return code */
int *minornum;		/* minor number */
int majornum;		/* major number is required for to generate */
			/* a minor number */
int pref_num = -1;	/* if a preferred minor number is not specfied */
			/* then default value */
int minors_in_grp = 1;	/* default number of minor numbers for one */
			/* major number */
int inc_within_grp = 1; /* increment the minor numbers by one */
int inc_btwn_grp = 1;   /* increment between minor numbers */
int lock_word;


	/* Initialize Prog */
	Prog = argv[0];

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);
 
	/* ignore these signals */
	memset(&action, '\0', sizeof (struct sigaction));
	action.sa_handler = SIG_IGN;
	sigaction (SIGQUIT, &action, &oldaction);
	action.sa_handler = SIG_IGN;
	sigaction (SIGINT, &action, &oldaction);
	action.sa_handler = SIG_IGN;
	sigaction (SIGTERM, &action, &oldaction);
	action.sa_handler = SIG_IGN;
	sigaction (SIGHUP, &action, &oldaction);

	/* initialize ODM */
	odm_initialize();

	/* set the config database path for the classes */
	odm_set_path("/etc/objrepos");

	/* argbuf will be pointer to command line argument */ 
        argbuf = (char **) getmem(argc * sizeof(char *));
  
	/* setflg sets command option flags and command option values */
	/* If error then exit */
	if ((setflg(argc,argv,argbuf)) < 0) exit(FAILURE);

	/* ck_required confirms required parameters were supplied */
	if (ck_required(argbuf)) {	 

			exit(ILLEGALSYNTAX);
	}

	/* if preferred minor number specified then set pref_num */
	if (p_flag) {
		pref_num = atoi(p_value);
	}

	/* major number is first argument */
	majornum = atoi(argbuf[0]);

	/* call genminor to generate minor number using argbuf[1], */
	/* the logical volume name and the major number */
	/* if a -1 is returned then an ODM error has occured so */
	/* print error message and set return code */
	lock_word = odm_lock("/etc/objrepos/lvm_lock",ODM_WAIT);
	minornum = genminor(argbuf[1],majornum,pref_num,minors_in_grp,
				inc_within_grp,inc_btwn_grp);

	if (minornum == NULL) {
		fprintf(stderr,lvm_msg(GENMINOR_ERR),Prog);
		rc = FAILURE;
	}
	else
		printf("%d\n",*minornum);

	odm_unlock(lock_word);
	/* terminate ODM */
	odm_terminate();

	exit(rc);
}

