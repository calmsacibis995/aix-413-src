static char sccsid[] = "@(#)35	1.4  src/bos/usr/sbin/lvm/intercmd/lvlstmajor.c, cmdlvm, bos411, 9428A410j 1/18/94 09:58:01";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *	main,compare
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
 * FILE NAME: lvlstmajor.c 
 *
 * FILE DESCRIPTION: Source file for lvlstmajor command.  
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
 * EXTERNAL PROCEDURES CALLED: lstmajor, odm_initialize, odm_terminate.                                  
 *
 * EXTERNAL VARIABLES: None
 * 
 * FUNCTION_NAMES: main,compare.
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

extern int lstmajor();		/* cfg routine to list major numbers */
extern odm_initialize();	/* routine to initialize ODM */
extern odm_terminate();		/* routine to terminate ODM gracefully */
extern char * odm_set_path();		/* odm rountine to set config path */

/*
 * GLOBAL VARIABLES:
 */
extern char *Prog;			/* points to name of the current Program */
extern nl_catd  scmc_catd;   		/* Cat descriptor for scmc conversion */

#define ODMERROR -1		/* error return code from cfg routines */

/* 
 * GLOBAL STRUCTURES:
 */ 
struct sigaction action;		/* parameters for sigaction */
struct sigaction oldaction;		/* parameters for sigaction */

/*
 * NAME: compare
 *
 * FUNCTION: compares two integers, for qsort(3).
 *    
 * INPUT: int *first, int *second
 *
 * RETURN VALUE DESCRIPTION:-1 - First is less than second
 * 		 	     0 - First and second are equal
 *                           1 - First is greater than second.
 */
int
compare(int *first,int *second) {
  if(*first < *second)
    return(-1);
  else if(*first > *second)
    return(1);
  else
    return(0);
  }

/*
 * NAME: main
 *
 * FUNCTION: lvlstmajor command returns a list of major numbers 
 *    
 * INPUT: None
 *
 * RETURN VALUE DESCRIPTION: 0 - SUCCESS                         
 *                           1 - FAILURE 
 */

main(argc,argv)
int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
	int rc = 0;		/* return code */
	int *listp;		/* pointer to list of major numbers */

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
 
	if (argc != 1) {
		fprintf(stderr,lvm_msg(606),Prog);
		fprintf(stderr,lvm_msg(LSTMAJ_USE),Prog);
		exit(ILLEGALSYNTAX);
	}

	listp = lstmajor();

	odm_terminate();

	if (listp == ODMERROR) {
		fprintf(stderr,lvm_msg(LSTMAJ_ERROR),Prog);
		rc = FAILURE;
	}
	else {
		int i,j;

		for(i=0;;i++)
		  if(-1 == listp[i])
		    break; 

		qsort((void *)listp,i,sizeof(int),compare);

		j = 0;
		for(i=0;;i++)
		  if(j == listp[i])
		    j++;
		  else if(-1 == listp[i]) {
		    printf("%d...\n",j);
		    break;
		    }
		  else {
		    printf("%d",j);
		    if(j+1 == listp[i])
		      printf(",");
		    else if(j+2 == listp[i])
		      printf(",%d,",j+1);
		    else
		      printf("..%d,",listp[i]-1);
		    j = listp[i]+1;
		    }

		free(listp);
		}

	exit(rc);
}

