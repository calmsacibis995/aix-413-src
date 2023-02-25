static char sccsid[] = "@(#)97	1.4.1.1  src/bos/usr/sbin/lvm/highcmd/parselsvg.c, cmdlvm, bos411, 9428A410j 1/13/93 18:51:58";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: getflg, parselsvg
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
 * FILENAME: parselsvg.c
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

extern nl_catd  scmc_catd;     /* Cat descriptor for scmc conversion */
extern char *Prog;             /* program name */

/*
 * GLOBAL VARIABLES
 */

/* external variables */
extern int optind;               /* index to next argv argument - incremented by getopt      */
extern char *optarg;             /* pointer to command line argument string - set by getopt  */

/* global variables */
char *Usage = "\n       lsvg -o [-n pvname]\n       lsvg [-i] [-M | -l | -p] [-n pvname] vgname...";

/* Pointer to command line parameter associated with command line option.         */
/* This variable only contain a value if corresponding flag is true - see below.  */
static char *n_value = NULL;

/* Each flag corresponds to a command option - flag true if option specified on command line */
static int i_flag = 0;
static int o_flag = 0;
static int l_flag = 0;
static int M_flag = 0;
static int n_flag = 0;
static int p_flag = 0;

void addvg(char *, char ***, int *, int *);

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
	while ((rc = getopt(argc, argv, "on:ilMp")) != EOF) {
		switch (rc) {
		case 'i':
			i_flag = 1;
			break;
		case 'o':
			o_flag = 1;
			break;
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
		default:
		case '?':
			fprintf(stderr, lvm_msg(NLS_VGUSAGE), Prog, Prog);
			return(-1);
		}
	}
	return(1);
}

/*
 * NAME: parselsvg
 *
 * FUNCTION: Parselsvg reads command line input from the user, parses it, and performs basic validation.
 *           If an error is encountered, the subroutine will print a message and exit the program.  All
 *           command line values are returned to the caller.
 */
void parselsvg(argc, argv, ooption, loption, Moption, poption, noption, sourcepv, vgdescript, num_vgs)
int argc;
char **argv;
int *ooption, *loption, *Moption, *poption, *noption;
char *sourcepv, ***vgdescript;
int *num_vgs;
{
	int retval;                /* return code */
	char **cmdarg;             /* pointer to command argument string */
	int vg_allocated;
	char vgtmp[STRSIZE];


	cmdarg = (char **) get_zeroed_mem (argc, sizeof(char *));
	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,cmdarg)) < 0) exit(retval);

	*ooption = o_flag;
	*loption = l_flag;
	*Moption = M_flag;
	*poption = p_flag;
	if (*noption = n_flag)
		strcpy (sourcepv, n_value);

	vg_allocated = *num_vgs = 0;

	if (i_flag)  /* read the vgnames from stdin into vgname array */
		while (scanf("%s", vgtmp) != EOF)
			addvg(vgtmp, vgdescript, num_vgs, &vg_allocated);
	else  /* move the vgnames from the command line into vgname array - if any exist */
		for (*num_vgs = 0; cmdarg[*num_vgs] != NULL; )
			addvg(cmdarg[*num_vgs], vgdescript,
				num_vgs, &vg_allocated);

	if (o_flag   &&   (i_flag || l_flag || p_flag || M_flag || *num_vgs != 0)) {
		fprintf(stderr, lvm_msg(LS_BADOPS), Prog);
		fprintf(stderr, lvm_msg(NLS_VGUSAGE), Prog, Prog);
		exit (-1);
	}

	if (*num_vgs == 0   &&   (i_flag || l_flag || p_flag || M_flag)) {
		fprintf(stderr, lvm_msg(LS_BADOPS), Prog);
		fprintf(stderr, lvm_msg(NLS_VGUSAGE), Prog, Prog);
		exit (-1);
	}

	if (*num_vgs != 0   &&   n_flag) {
		fprintf(stderr, lvm_msg(LS_BADOPS), Prog);
		fprintf(stderr, lvm_msg(NLS_VGUSAGE), Prog, Prog);
		exit (-1);
	}
}


/*
 *	add 'newvg' to the array of pointers 'vglist'.
 *
 *	'vgct' is a pointer to actual number we've saved so far, and
 *	'vg_allocated' is a pointer to how many pointers we've allocated
 *	so far (we'll allocate the pointers in clusters.  maybe that
 *	will gain us some performance.  maybe not)
 */
void
addvg(char *newvg, char ***vglist, int *vgct, int *vgallocated)
{
	register char **list;
	register int ct, allocated;
	register unsigned newsize;

	/*
	 * xfer to local vars (less redirections required :-)
	 */
	list = *vglist;
	ct = *vgct;
	allocated = *vgallocated;

	if (ct >= allocated) {
		/*
		 * grow table (in clusters of 16)
		 */
		allocated += 16;

		newsize = allocated * sizeof(*list);	/* how much we need */

		list = (ct == 0) ? (char **) malloc(newsize) :
				   (char **) realloc((void *) list, newsize);
		}

	/*
	 * save new vg.
	 */
	if (list != NULL && (list[ct++] = strdup(newvg)) == NULL)
		list = NULL;

	if (list == NULL) {
		fprintf(stderr, lvm_msg(ALLOCERR_LVM), Prog);
		exit(1);
		}

	/*
	 * xfer back
	 */
	*vglist = list;
	*vgct = ct;
	*vgallocated = allocated;
}
