static char sccsid[] = "@(#)24  1.3  src/bos/usr/sbin/lvm/highcmd/parselslv.c, cmdlvm, bos411, 9428A410j 6/15/90 21:56:20";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: getflg, parselslv
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
 * FILENAME: parselslv.c
 *
 * FILE DESCRIPTION: Functions to parse command line parameters for lslv (logical volume list command).
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "cmdlvm.h"
#include "cmdlvm_msg.h"

/*
 * EXTERNAL PROCEDURES CALLED: NONE
 */

/* local library functions */
extern int setflg();

/*
 * GLOBAL VARIABLES
 */

/* external variables */
extern int optind;             /* index to next argv argument - incremented by getopt      */
extern char *optarg;           /* pointer to command line argument string - set by getopt  */

extern nl_catd  scmc_catd;     /* Cat descriptor for scmc conversion */
extern char *Prog;             /* program name */

/* global variables */
char *Usage = " [-l | -m] [-n pvname] lvdescript\n       lslv  [-n pvname] -p pvname [lvdescript]";

/* Pointer to command line parameter associated with command line option.         */
/* This variable only contain a value if corresponding flag is true - see below.  */
static char *n_value = NULL;
static char *p_value = NULL;

/* Each flag corresponds to a command option - flag true if option specified on command line */
static int l_flag = 0;
static int m_flag = 0;
static int n_flag = 0;
static int p_flag = 0;





/*
 * NAME: getflg
 *
 * FUNCTION: Parse command line options until the first non-option is encountered. (Called from setflag.)
 *
 * (NOTES:) For each option in the command line, a global flag is set and a global
 *     value is loaded with any accompaning command argument.
 *
 * RETURN VALUE DESCRIPTION: Return negative value (-1) if illegal option found.
 *                           Return positive value (1) if options parsed correctly.
 *                           Return zero if no command options found.
 */

int getflg(argc,argv)

int argc;        /* number of command line string in referenced by argv */
char **argv;     /* array of command line strings */
{
	int rc;                 /* getopt return value */

	if (!argc) return(0);   /* if no command line options to parse then return */

	/* switch on each option contained in the command line */
	while ((rc = getopt(argc, argv, "lmn:p:")) != EOF) {
		switch (rc) {
		case 'l':
			l_flag = 1;
			break;
		case 'm':
			m_flag = 1;
			break;
		case 'n':
			n_flag = 1;
			n_value = optarg;
			break;
		case 'p':
			p_flag = 1;
			p_value = optarg;
			break;
		default:
		case '?':
			fprintf(stderr, lvm_msg(LS_LVUSAGE), Prog, Prog);
			return(-1);
		}
	}
	return(1);
}




/*
 * NAME: parselslv
 *
 * FUNCTION: Parselslv reads command line input from the user, parses it, and performs basic validation.
 *           If an error is encountered, the subroutine will print a message and exit the program.  All
 *           command line values are returned to the caller.
 */

void parselslv(argc, argv, loption, moption, poption, pvname, noption, sourcepv, lvdescript)
int argc;
char **argv;
int *loption, *moption, *poption, *noption;
char *pvname, *sourcepv, *lvdescript;
{
	int retval;                /* return code */
	char **cmdarg;             /* pointer to command argument string */

	cmdarg = (char **) get_zeroed_mem (argc, sizeof(char *));
	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,cmdarg)) < 0) exit(retval);

	*loption = l_flag;
	*moption = m_flag;
	if (*noption = n_flag)
		strcpy (sourcepv, n_value);
	if (*poption = p_flag)
		strcpy (pvname, p_value);

	if (*cmdarg != NULL)
		strcpy (lvdescript, *cmdarg);
	else
		*lvdescript = '\0';

	if (!(*poption) && *cmdarg==NULL) {
		fprintf(stderr, lvm_msg(602), Prog);
		fprintf(stderr, lvm_msg(LS_LVUSAGE), Prog, Prog);
		exit (-1);
	}

	if (*poption && (*loption || *moption)) {
		fprintf(stderr, lvm_msg(LS_BADOPS), Prog);
		fprintf(stderr, lvm_msg(LS_LVUSAGE), Prog, Prog);
		exit (-1);
	}

	if (*loption && *moption) {
		fprintf(stderr, lvm_msg(LS_BADOPS), Prog);
		fprintf(stderr, lvm_msg(LS_LVUSAGE), Prog, Prog);
		exit (-1);
	}

}


