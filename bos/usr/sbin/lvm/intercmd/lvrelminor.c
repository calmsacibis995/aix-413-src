static char sccsid[] = "@(#)30	1.8.1.2  src/bos/usr/sbin/lvm/intercmd/lvrelminor.c, cmdlvm, bos41J, bai15 3/27/95 00:09:53";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *	getflg,main
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
 * FILE NAME: lvrelminor.c 
 *
 * FILE DESCRIPTION: Source file for lvrelminor command.
 *	"Lvrelminor" provides an intermediate level of command interface.
 *	"Lvrelminor" returns a minor number.
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
 * EXTERNAL PROCEDURES CALLED: relminor, odm_initialize, odm_terminate.                                  
 *
 * EXTERNAL VARIABLES: optind, optarg.
 * 
 * FUNCTION_NAMES: main.
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

#define ODMERROR -1		/* error return code from cfg routines */

extern int reldevno();		/* cfg routine to release minor numbers */ 
extern odm_initialize();	/* cfg routine to initialize ODM database */
extern odm_terminate();		/* cfg routine to terminate ODM gracefully */
extern char * odm_set_path();		/* odm rountine to set config path */

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
	while ((rc = getopt(argc, argv, "")) != EOF) {
		switch (rc) {
		case '?':
			fprintf(stderr,lvm_msg(RELMIN_USE),Prog);
			return(-1);
		}
	}
	return(1);
}
/*
 * NAME: main
 *
 * FUNCTION: lvrelminor command releases a minor number.                    
 *
 * INPUT: logical volume name.
 *
 * OUTPUT: none.
 *
 * RETURN CODE DESCRIPTION: 0 - SUCCESS                         
 *                          1 - FAILURE 
 */

main(argc,argv)
int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
int rc = 0;		/* return code */
int retval;	   	/* return code */
char **argbuf;		/* arguement buffer to hold lvname */
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
	/* If error then exit  */
	retval = setflg(argc,argv,argbuf);
	if (retval < 0) exit(FAILURE);

	/* if the argument count is not 2 then an syntax error has occured */
	if (argc != 2) { /* missing lvname */
		fprintf(stderr,lvm_msg(602),Prog);
		exit(ILLEGALSYNTAX);
	}

 	/* call reldevno to release minor number */
	/* if a -1 is returned from reldevno an ODM error has occured */
	/* so print message and set return code to FAILURE */
	lock_word = odm_lock("/etc/objrepos/lvm_lock",ODM_WAIT);
	retval = reldevno(*argbuf,FALSE);      
	if (retval == ODMERROR) {
		fprintf(stderr,lvm_msg(RELMINOR_ERR),Prog);
		rc = FAILURE;
	}

	odm_unlock(lock_word);
	/* terminate ODM gracefully */
	odm_terminate();

	exit(rc);
}

