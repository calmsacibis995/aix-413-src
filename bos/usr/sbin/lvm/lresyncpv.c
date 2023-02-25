static char sccsid[] = "@(#)91	1.6.1.1  src/bos/usr/sbin/lvm/lresyncpv.c, cmdlvm, bos411, 9428A410j 1/18/94 10:06:11";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *	getflg,ck_required,Usage_error,main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * FILE: lresyncpv.c
 *
 * FILE DESCRIPTION: Source file for lresyncpv program.  "lresyncpv" provides
 *     a command level user interface to the lvm low level function lvm_resyncpv.
 *
 *     For additional information on this program and the other lvm low level command
 *     programs see the specification "Logical Volume Manager Low-level Command
 *     Interface" and also the interface documentation to the lvm library functions.
 *
 * FUNCTION_NAMES: getflg, ck_required, setdef, main
 *
 */



#include <lvm.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

/*              include file for message texts          */
#include "cmdlvm_def.h"
#include "cmdlvm_msg.h"
#include <locale.h>


/*
 * EXTERNAL PROCEDURES CALLED: NONE
 */

/* local library functions */
extern int setflg();
extern int get_uniq_id();
extern char *lvm_msg();

/*
 * GLOBAL VARIABLES
 */

/* flags for required values */
#define VGID	1     /* error return code if lv id not specified on command line	*/
#define PVID	2     /* error return code if pv id not specified on command line	*/

/* external variables */
extern int optind;   /* index to next argv argument - incremented by getopt	 */
extern char *optarg; /* pointer to command line argument string - set by getopt  */
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog;			 /* points to name of the current Program  */

/* Pointer to command line parameters associated with command line options. */
static char *g_value;
static char *p_value;

/* Flags corresponding to command option - flag true if option specified on command line */
static int f_flag;
static int g_flag;
static int p_flag;


/*
 * NAME: getflg
 *
 * FUNCTION: Parse command line options until the first non-option is encountered.
 *
 * (NOTES:) For each option in the command line, a global flag is set and a global
 *     value is loaded with any accompaning command argument.
 *
 * RETURN VALUE DESCRIPTION: Return negative value (-1) if illegal option found.
 *			     Return positive value (1) if options parsed correctly.
 *			     Return zero if no command options found.
 */

int getflg(argc,argv)

int argc;	 /* number of command line string in referenced by argv */
char **argv;	 /* array of command line strings */
{
	int rc; 		/* getopt return value */

	if (!argc) return(0);	/* if no command line options to parse then return */

	/* switch on each option contained in the command line */
	while ((rc = getopt(argc, argv, "fg:p:")) != EOF) {
		switch (rc) {
		case 'f':
			f_flag = 1;
			break;
		case 'g':
			g_flag = 1;
			g_value = optarg;
			break;
		case 'p':
			p_flag = 1;
			p_value = optarg;
			break;
		default:
		case '?':
			Usage_error();
			return(-1);
		}
	}
	return(1);
}


/*
 * NAME: ck_required
 *
 * FUNCTION: Check input flag indicators to determine if all required fields
 *     have been received.  Print error message if missing field is found.
 *
 * RETURN VALUE DESCRIPTION: If zero then no error.  If not zero then error (the
 *    non-zero value indicates which required flag was not set).
 *
 */

static int ck_required()
{
	if (!g_flag) {
		fprintf(stderr, lvm_msg(MISSING_VGID), Prog);
		Usage_error();
		return(VGID);
	}
	if (!p_flag) {
		fprintf(stderr, lvm_msg(MISSING_PVID), Prog);
		Usage_error();
		return(PVID);
	}
	return(0);
}


/*
 * NAME: Usage_error
 *
 * FUNCTION: routine for displaying usage error message.
 *
 * RETURN VALUE DESCRIPTION: NONE
 */

int Usage_error()
{
	fprintf(stderr, lvm_msg(RSPV_USAGE), Prog);
}

/*
 * NAME: main
 *
 * FUNCTION: lresyncpv reads command line input from the user,
 *    parses it, performs basic validation, formats it, and calls
 *    lvm_resyncpv passing the formatted values.  If an error is
 *    encountered then a message is displayed.
 *
 * RETURN VALUE DESCRIPTION: Zero if no error. Non-zero otherwise.
 */

main(argc,argv)

int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
	int retval;	   /* return code */
	int force;     /* force flag to pass to the subroutine */
	struct unique_id vg_id,pv_id;  /* lvm volume group id and physical volume id */

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

	/* Initialize Prog for usage_error function */
	Prog = argv[0];

	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,NULL)) < 0) exit(lvm_rc(retval));

	if ((retval=ck_required()) == 0) {  /* ck_required confirms required parameters were supplied */

		/* format ids and call lvm to resync the physical volume */
		get_uniq_id(g_value, &vg_id);
		get_uniq_id(p_value, &pv_id);
		if (f_flag) force = TRUE;
		else force = FALSE;       /* default is False */
		if (retval = lvm_resyncpv(&vg_id,&pv_id,force)) {
			errmsg(retval);
			exit(lvm_rc(retval));
		}
	}
	exit(lvm_rc(retval));
}
