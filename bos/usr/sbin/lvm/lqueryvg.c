static char sccsid[] = "@(#)36  1.10.1.2  src/bos/usr/sbin/lvm/lqueryvg.c, cmdlvm, bos411, 9428A410j 1/18/94 09:57:12";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: getflg, ck_required, setdef, main, output
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
 * FILE: lqueryvg.c
 *
 * FILE DESCRIPTION: Source file for lqueryvg program.  "lqueryvg" provides
 *     a command level user interface to the lvm low level function lvm_queryvg.
 *
 *     For additional information on this program and the other lvm low level command
 *     programs see the specification "Logical Volume Manager Low-level Command
 *     Interface" and also the interface documentation to the lvm library functions.
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
#define VGID    1     /* error return code if neither vg id nor pv name is specified on command line */

/* external variables */
extern int optind;		 /* index to next argv argument - incremented by getopt      */
extern char *optarg;		 /* pointer to command line argument string - set by getopt  */
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog;				      /* points to name of the current Program */


/* Pointer to command line parameter associated with command line option.	  */
/* This variable only contain a value if corresponding flag is true - see below.  */
static char *g_value;
static char *p_value;

/* Each flag corresponds to a command option - flag true if option specified on command line */
static int g_flag;
static int p_flag;
static int N_flag;
static int s_flag;
static int F_flag;
static int n_flag;
static int c_flag;
static int a_flag;
static int D_flag;
static int L_flag;
static int P_flag;
static int A_flag;
static int t_flag;
static int v_flag;


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
	while ((rc = getopt(argc, argv, "g:p:NsFncDaLPAtv")) != EOF) {
		switch (rc) {
		case 'g':
			g_flag = 1;
			g_value = optarg;
			break;
		case 'p':
			p_flag = 1;
			p_value = optarg;
			break;
		case 'N':
			N_flag = 1;
			break;
		case 's':
			s_flag = 1;
			break;
		case 'F':
			F_flag = 1;
			break;
		case 'n':
			n_flag = 1;
			break;
		case 'c':
			c_flag = 1;
			break;
		case 'D':
			D_flag = 1;
			break;
		case 'a':
			a_flag = 1;
			break;
		case 'L':
			L_flag = 1;
			break;
		case 'P':
			P_flag = 1;
			break;
		case 'A':
			A_flag = 1;
			break;
		case 't':
			t_flag = 1;
			break;
		case 'v':
			v_flag = 1;
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
	if (!g_flag && !p_flag) {
		fprintf(stderr, lvm_msg(MISSING_VGPV), Prog);
		Usage_error();
		return(VGID);
	}
	return(0);
}


/*
 * NAME: output
 *
 * FUNCTION: Send requested data to stdout - if tag has been selected then tag the data.
 *
 * (NOTES): The flags specify what data was received from lvm - this is the data to
 *          be output.  The "A" and "a" flags represent a group of data fields.
 *
 *          A tag field is output with each value if t_flag is true.
 *
 * RETURN VALUE DESCRIPTION: No return value.
 */

static int output(q,v)

struct queryvg *q;      /* lvm structure containing the query data to be output */
struct unique_id *v;    /* vg_id returned from the lvm subroutine */
{
	int tag = 12;   /* printf format value - used for string width and precision */
	register i;          /* offset into logical volume info */
	char *ptr;           /* pointer to tag field */
	struct lv_array *l;  /* pointer to list of lv id's and states */
	struct pv_array *p;  /* pointer to list of pv id's and states */


	/* if no query flags were set then use the default - the "a" flag */
	if (!A_flag && !L_flag && !P_flag && !a_flag && !N_flag && !s_flag
	 && !F_flag && !n_flag && !c_flag && !D_flag && !v_flag) a_flag = 1;

	if (A_flag || a_flag || N_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"Max LVs:");
		printf("%1d\n",q->maxlvs);
	}
	if (A_flag || a_flag || s_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"PP Size:");
		printf("%1d\n",q->ppsize);
	}
	if (A_flag || a_flag || F_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"Free PPs:");
		printf("%1d\n",q->freespace);
	}
	if (A_flag || a_flag || n_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"LV count:");
		printf("%1d\n",q->num_lvs);
	}
	if (A_flag || a_flag || c_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"PV count:");
		printf("%1d\n",q->num_pvs);
	}
	if (A_flag || a_flag || D_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"Total VGDAs:");
		printf("%1d\n",q->total_vgdas);
	}
	if (A_flag || L_flag)                        /* output logical volume ids & states */
		if (q->lvs != (struct lv_array *)0) {
			ptr = "Logical:";
			for (i=0;i<q->num_lvs;++i) {  /* for each logical volume */
				if (t_flag) printf("%-*.*s\t",tag,tag,ptr);
				l = q->lvs + i;
				printf("%08x%08x.%-3d ",
					l->lv_id.vg_id.word1,l->lv_id.vg_id.word2,l->lv_id.minor_num);
				printf("%s %-3d\n",
					l->lvname,l->state);
				ptr = "";
			}
		}
	if (A_flag || P_flag)                        /* output physical volume ids & states */
		if (q->pvs != (struct pv_array *)0) {
			ptr = "Physical:";
			for (i=0;i<q->num_pvs;++i) {            /* for each physical volume */
				if (t_flag) printf("%-*.*s\t",tag,tag,ptr);
				p = q->pvs + i;
				printf("%08x%08x %-3d %-3d\n",
					p->pv_id.word1,p->pv_id.word2,p->pvnum_vgdas,p->state);
				ptr = "";
			}
		}
	if (v_flag && p_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"VGid:");
		printf("%08x%08x\n", v->word1,v->word2);
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
	fprintf(stderr, lvm_msg(QVG_USAGE), Prog);
}


/*
 * NAME: main
 *
 * FUNCTION: lqueryvg reads command line input from the user,
 *    parses it, performs basic validation, formats it, and calls
 *    lvm_queryvg passing the formatted values.  If an error is
 *    encountered then a message is displayed.
 *
 * RETURN VALUE DESCRIPTION: Zero if no error. Non-zero otherwise.
 */

main(argc,argv)

int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
	int retval;                 /* return code */
	char *value0 = "0";
	struct unique_id vg_id;     /* volume group id */
	struct queryvg *queryvg;    /* lvm record for querying lvm database */
	char *pvname;               /* Physical Volume Name */

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

	/* Initialize Prog for use in usage_error function if usage error occurs */
	Prog = argv[0];

	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,NULL)) < 0) exit(lvm_rc(retval));

	if ((retval=ck_required()) == 0) {  /* ck_required confirms required parameters were supplied */

		if (p_flag) {                   /* is pv name specified? */
			pvname = p_value;
			g_value = value0;       /* zero out vg id if pvname is specified */
		}
		else pvname = NULL;

		get_uniq_id(g_value, &vg_id);

		if (retval = lvm_queryvg(&vg_id,&queryvg,pvname)) {
			errmsg(retval);
			exit(lvm_rc(retval));
		}
		else {
			output(queryvg,&vg_id);    /* output the query info received back from lvm */
			lvm_freebuf(queryvg);
		}
	}
	exit(lvm_rc(retval));
}
