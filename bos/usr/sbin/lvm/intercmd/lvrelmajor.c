static char sccsid[] = "@(#)28	1.8.1.2  src/bos/usr/sbin/lvm/intercmd/lvrelmajor.c, cmdlvm, bos41J, bai15 3/27/95 00:09:51";

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
 * FILE NAME: lvrelmajor.c 
 *
 * FILE DESCRIPTION: Source file for lvrelmajor command. 
 *	"Lvrelmajor" provides an intermediate level of command interface.
 *	"Lvrelmajor" returns a major number.
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
 * EXTERNAL PROCEDURES CALLED: relmajor, odm_initialize, odm_terminate.                                  
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

#define ODMERROR -1		/* error return code from cfg routines */

extern int relmajor();		/* cfg routine to release minor number */
extern odm_initialize();	/* rountine to initialize ODM */  
extern odm_terminate();		/* routine to terminate ODM gracefully */
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
			fprintf(stderr,lvm_msg(GENMAJ_USE),Prog);
			return(-1);
		}
	}
	return(1);
}
/*
 * NAME: main
 *
 * FUNCTION: lvrelmajor command releases a major number.                    
 *
 * INPUT: vgname.
 *
 * OUTPUT: none.
 *
 * RETURN CODE DESCRIPTION: 0 - SUCCESS                         
 *                          1 - FAILURE 
 */

main(argc,argv)
int argc;	   	/* number of command line strings in argc */
char **argv;	   	/* array of command line strings - from user */
{
char **agrbuf;		/* pointer for major number arguement */
int rc = SUCCESS;	/* return code */
int retval;	   	/* return code */
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

	/* typelv will be pointer to command line argument */ 
        agrbuf = (char **) getmem(argc * sizeof(char *));
  
	/* setflg sets command option flags and command option values. */
	/* If error then exit. */
	if ((setflg(argc,argv,agrbuf)) < 0) exit(FAILURE);

	if (argc != 2) {
		fprintf(stderr,lvm_msg(606),Prog);
		fprintf(stderr,lvm_msg(GENMAJ_USE),Prog);
		exit(ILLEGALSYNTAX);
	}

	/* the cfg routine is called with *argbuf which is a */
	/* pointer to a vgname. If the return code from relmajor */
	/* is -1 then an ODM error has occured */
	/* and a FAILURE (1) return value will be set */

	lock_word = odm_lock("/etc/objrepos/lvm_lock",ODM_WAIT);
	retval = relmajor(*agrbuf);

	if (retval == ODMERROR) {
		fprintf(stderr,lvm_msg(RELSEQ_ERR),Prog);
		rc = FAILURE;
	}

	odm_unlock(lock_word);
	/* terminate ODM */
	odm_terminate();

	exit(rc);
}

