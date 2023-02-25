static char sccsid[] = "@(#)25	1.7  src/bos/usr/sbin/lvm/highcmd/parselspv.c, cmdlvm, bos411, 9428A410j 5/1/91 16:35:16";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: getflg, parselspv
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
 * FILENAME: parselspv.c
 *
 * FILE DESCRIPTION: Functions to parse command line parameters for lslv (logical volume list command).
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
extern int optind;           /* index to next argv argument - incremented by getopt      */
extern char *optarg;         /* pointer to command line argument string - set by getopt  */
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog;           /* program name */

/* global variables */
char *Prog="lspv";               /* initialize error message variables */
char *Usage = "[-M | -l | -p] [-n pvname] [-v vgid] [pvdescript]";

/* Pointer to command line parameter associated with command line option.         */
/* This variable only contain a value if corresponding flag is true - see below.  */
static char *n_value = NULL;
static char *v_value = NULL;

/* Each flag corresponds to a command option - flag true if option specified on command line */
static int l_flag = 0;
static int M_flag = 0;
static int n_flag = 0;
static int p_flag = 0;
static int v_flag = 0;

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
	while ((rc = getopt(argc, argv, "lMn:pv:")) != EOF) {
		switch (rc) {
		case 'l':
			l_flag = 1;
			break;
		case 'M':
			M_flag = 1;
			break;
		case 'n':
			n_flag = 1;
			n_value = optarg;
			break;
		case 'p':
			p_flag = 1;
			break;
		case 'v':
			v_flag = 1;
			v_value = optarg;
			break;
		default:
		case '?':
			fprintf(stderr, lvm_msg(NLS_PVUSAGE), Prog);
			return(-1);
		}
	}
	return(1);
}




/*
 * NAME: parselspv
 *
 * FUNCTION: Parselslv reads command line input from the user, parses it, and performs basic validation.
 *           If an error is encountered, the subroutine will print a message and exit the program.  All
 *           command line values are returned to the caller.
 */

void parselspv(argc, argv, loption, poption, Moption, voption, vgid, noption, sourcepv, pvdescript)
int argc;
char **argv;
int *loption, *Moption, *poption, *noption, *voption;
char *vgid, *sourcepv, *pvdescript;
{
	int retval;                /* return code */
	char **cmdarg;             /* pointer to command argument string */
	char *getalloc();


	cmdarg = (char **) get_zeroed_mem (argc, sizeof(char *));
	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,cmdarg)) < 0) exit(retval);

	*loption = l_flag;
	if (*noption = n_flag)
		strcpy (sourcepv, n_value);
	*poption = p_flag;
	*Moption = M_flag;
	if (*voption = v_flag)
		strcpy (vgid, v_value);

	if (*cmdarg != NULL)
		strcpy (pvdescript, *cmdarg);
	else
		pvdescript = NULL;

    if ((l_flag || p_flag || M_flag) && (pvdescript == NULL)){
	   fprintf(stderr, lvm_msg(NLS_PVUSAGE), Prog);
	   exit(-1);
    }

	if (l_flag && p_flag || l_flag && M_flag || p_flag && M_flag) {
		fprintf(stderr, lvm_msg(LS_BADOPS), Prog);
		fprintf(stderr, lvm_msg(NLS_PVUSAGE), Prog);
		exit (-1);
	}

}


