static char sccsid[] = "@(#)36	1.2.1.3  src/bos/usr/sbin/lvm/intercmd/lvchkmajor.c, cmdlvm, bos411, 9428A410j 1/18/94 09:57:55";

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
 * FILE NAME: lvchkmajor.c 
 *
 * FILE DESCRIPTION: Source file for lvchkmajor command.  
 *	"lvchkmajor" provides an
 *	intermediate level of command interface.  
 *	"lvchkmajor" checks and returns a major number.
 *
 *	For additional information on this program and the other 
 *	lvm immediate level command 
 *	programs see the specification "Logical Volume 
 *	Manager Intermediate Level Command Interface" and also 
 *	the interface documentation to the lvm library functions.
 *         
 * RETURN VALUE DESCRIPTION:
 *				0   Command Successfully completed
 *			        1   Unsuccessful return
 *
 * EXTERNAL PROCEDURES CALLED: chkmajor, odm_initialize, odm_terminate.                                  
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

extern int chkmajor();		/* cfg routine to check a major number */
extern odm_initialize();	/* routine to initialize ODM */
extern odm_terminate();		/* routine to terminate ODM gracefully */
extern char * odm_set_path();		/* odm rountine to set config path */

/*
 * GLOBAL VARIABLES:
 */
extern char *Prog;			/* points to name of the current Program */
extern nl_catd  scmc_catd;   		/* Cat descriptor for scmc conversion */

static char *major_number_string;

#define ODMERROR -1		/* error return code from cfg routines */

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

	/* if too few command line options to parse then return */
	if(!argc) 
          return(0);	

	/* switch on each option contained in the command line */
	while ((rc = getopt(argc, argv, "")) != EOF) {
		switch (rc) {
		case '?':
			fprintf(stderr,lvm_msg(CHKMAJ_USE),Prog);
			return(-1);
		}
	}

	return(1);
}
/*
 * NAME: main
 *
 * FUNCTION: lvchkmajor command checks and returns a major number 
 *           for a volume group.
 *    
 * INPUT: volume group name, proposed major number.
 *
 * RETURN VALUE DESCRIPTION: 0 - SUCCESS                         
 *                           1 - FAILURE 
 */

main(argc,argv)
int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
int rc = 0;		/* return code */
int majornum;		/* major number */
char **argbuf;

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

	/* Initialize Prog */
	Prog = argv[0];

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);
 
	if (argc != 3) {
		fprintf(stderr,lvm_msg(606),Prog);
		fprintf(stderr,lvm_msg(CHKMAJ_USE),Prog);
		exit(ILLEGALSYNTAX);
	}

	/* vgname will be pointer to command line argument */ 
        argbuf = (char **) getmem(argc * sizeof(char *));
  
	/* setflg sets command option flags and command option values. */
	/* If error then exit. */
	if ((setflg(argc,argv,argbuf)) < 0) exit(FAILURE);

	/* ck_required confirms required parameters were supplied. */
	if(ck_required(argbuf))
		exit(ILLEGALSYNTAX);

        /* major number is first argument */
        majornum = atoi(argbuf[0]);

	/* call chkmajor to check a major number */
	/* if chkmajor fails then an ODM error (-1) has occured and */
	/* it will be printed and return code set to FAILURE */

	majornum = chkmajor(argbuf[1],majornum);

	if (majornum == ODMERROR)
		rc = FAILURE;

	printf("%d\n", majornum);

	odm_terminate();

	exit(rc);
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
		fprintf(stderr,lvm_msg(CHKMAJ_PARMS),Prog);
		fprintf(stderr,lvm_msg(CHKMAJ_USE),Prog);
		return(ILLEGALSYNTAX);
	}
	/* If there was no argument on the command line then error */
	if (argbuf[1] == NULL) { 
		fprintf(stderr,lvm_msg(CHKMAJ_PARMS),Prog);
		fprintf(stderr,lvm_msg(CHKMAJ_USE),Prog);
		return(ILLEGALSYNTAX);
	}
	return(SUCCESS);
}

