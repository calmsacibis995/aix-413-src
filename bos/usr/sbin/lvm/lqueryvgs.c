static char sccsid[] = "@(#)21	1.9.1.2  src/bos/usr/sbin/lvm/lqueryvgs.c, cmdlvm, bos411, 9428A410j 1/18/94 09:57:14";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *	getflg,ck_required,output,Usage_error,main
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
 * FILE: lqueryvgs.c
 *
 * FILE DESCRIPTION: Source file for lqueryvgs program.  "lqueryvgs" provides
 *     a command level user interface to the lvm low level function lvm_queryvgs.
 *
 *     For additional information on this program and the other lvm low level command
 *     programs see the specification "Logical Volume Manager Low-level Command
 *     Interface" and also the interface documentation to the lvm library functions.
 *
 * FUNCTION_NAMES: getflg, setdef, main, output
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


/* external variables */
extern int optind; /* index to next argv argument - incremented by getopt */
extern char *optarg; /*pointer to command line argument string - set by getopt*/
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog;           /* points to name of the current Program */


/* Each flag corresponds to a command option - flag true if option specified on command line */
static int N_flag;
static int G_flag;
static int A_flag;
static int t_flag;


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
	while ((rc = getopt(argc, argv, "NGAt")) != EOF) {
		switch (rc) {
		case 'N':
			N_flag = 1;
			break;
		case 'G':
			G_flag = 1;
			break;
		case 'A':
			A_flag = 1;
			break;
		case 't':
			t_flag = 1;
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
 * NAME: output
 *
 * FUNCTION: Send requested data to stdout - if tag has been selected then tag the data.
 *
 * (NOTES): The flags specify what data was received from lvm - this is the data to
 *          be output.  The "A" flag represents a group of data fields.
 *
 *          A tag field is output with each value if t_flag is true.
 *
 * RETURN VALUE DESCRIPTION: No return value.
 */

static int output(q)
struct queryvgs *q;    /* lvm structure containing the query data to be output */
{
	int tag = 23;       /* printf format value - used for string width and precision */
	register i;         /* index into volume group ids */
	char *ptr;          /* pointer to the tag field    */


	/* if no query flags were set then use the default - the "A" flag */
	if (!A_flag && !N_flag && !G_flag) A_flag = 1;

	if (A_flag || N_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"VG count:");
		printf("%1d\n",q->num_vgs);
	}
	if (A_flag || G_flag) {     /* output the volume group ids */
		ptr = "VG ids & major numbers:";
		for (i=0;i<q->num_vgs;++i) {    /* for each volume group */
			if (t_flag) printf("%-*.*s\t",tag,tag,ptr);
			printf("%08x%08x %ld\n",q->vgs[i].vg_id.word1,
			  q->vgs[i].vg_id.word2, q->vgs[i].major_num);
			ptr = "";
		}
	}
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
	fprintf(stderr, lvm_msg(QVGS_USAGE), Prog);
}


/*
 * NAME: main
 *
 * FUNCTION: lqueryvgs reads command line input from the user,
 *    parses it, performs basic validation, formats it, and calls
 *    lvm_queryvgs passing the formatted values.  If an error is
 *    encountered then a message is displayed.
 *
 * RETURN VALUE DESCRIPTION: Zero if no error. Non-zero otherwise.
 */

main(argc,argv)

int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
	int retval;                 /* return code */
	struct unique_id vg_id;     /* lvm volume group id */
	struct queryvgs *queryvgs;
	mid_t kmid;

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

	/* Initialize Prog for use in usage_error function if usage error occurs */
	Prog = argv[0];

	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,NULL)) < 0) exit(lvm_rc(retval));

	if( retval = getkmid(&kmid) )
	{
	  exit(lvm_rc(retval));
	}
		

	/* call lvm to query the volume groups */
	if (retval = lvm_queryvgs(&queryvgs,kmid)) {
		errmsg(retval);
		exit(lvm_rc(retval));
	}
	else {
		output(queryvgs);
		lvm_freebuf(queryvgs);
	}
	
	exit(lvm_rc(retval));
}
