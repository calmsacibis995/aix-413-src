static char sccsid[] = "@(#)85	1.6.1.1  src/bos/usr/sbin/lvm/intercmd/putlvcb.c, cmdlvm, bos411, 9428A410j 3/4/94 17:34:01";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: main getflg
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
 * putlvcb.c
 * Writes logical volume control block paramters into the
 * the logical volume control block.
 */

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "cmdlvm.h"


/*
 * EXTERNAL PROCEDURES CALLED: NONE
 */

/* local library functions */
extern int usage_error();

/*
 * GLOBAL VARIABLES
 */

/* external variables */

extern int optind;   /* index to next argv argument - incremented by getopt */
extern char *optarg; /* pointer to command line arg string - set by getopt  */


/* global variables */
char *Prog="";


/* Pointer to command line parameter associated with command line option.         */
/* This variable only contain a value if corresponding flag is true - see below.  */
static char *a_value;
static char *c_value;
static char *e_value;
static char *f_value;
static char *i_value;
static char *L_value;
static char *n_value;
static char *O_value;
static char *r_value;
static char *s_value;
static char *S_value;
static char *t_value;
static char *u_value;
static char *v_value;
static char *x_value;


/* Each flag corresponds to a command option - flag true if option specified on command line */
static int a_flag = 0;
static int c_flag = 0;
static int e_flag = 0;
static int f_flag = 0;
static int i_flag = 0;
static int L_flag = 0;
static int n_flag = 0;
static int O_flag = 0;
static int r_flag = 0;
static int s_flag = 0;
static int S_flag = 0;
static int t_flag = 0;
static int u_flag = 0;
static int v_flag = 0;
static int x_flag = 0;
static int N_flag = 0;



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

	if (!argc) return(0);   /* return if no options to parse  */

	/* switch on each option contained in the command line */
	while ((rc = getopt(argc,argv,"a:c:e:f:i:L:n:O:r:s:S:t:u:v:x:N")) != EOF)
	{
		switch (rc) {
		case 'a':                        /* intrapolicy */
			a_flag = 1;
			a_value = optarg;
			break;
		case 'c':                        /* copies */
			c_flag = 1;
			c_value = optarg;
			break;
		case 'e':                        /* interpolicy */
			e_flag = 1;
			e_value = optarg;
			break;
		case 'f':                        /* fs(/etc/filesystems) */
			f_flag = 1;
			f_value = optarg;
			break;
		case 'i':                        /* lvid */
			i_flag = 1;
			i_value = optarg;
			break;
		case 'L':                        /* Label */
			L_flag = 1;
			L_value = optarg;
			break;
		case 'n':                        /* number of LPs */
			n_flag = 1;
			n_value = optarg;
			break;
		case 'O':                        /* stripe width */
			O_flag = 1;
			O_value = optarg;
			break;
		case 'r':                        /* relocatable flag */
			r_flag = 1;
			r_value = optarg;
			break;
		case 's':                        /* strictness flag */
			s_flag = 1;
			s_value = optarg;
			break;
		case 'S':                        /* stripe blk exp */
			S_flag = 1;
			S_value = optarg;
			break;
		case 't':                        /* LV type */
			t_flag = 1;
			t_value = optarg;
			break;
		case 'u':                        /* upperbound */
			u_flag = 1;
			u_value = optarg;
			break;
		case 'v':                        /* vgname */
			v_flag = 1;
			v_value = optarg;
			break;
		case 'x':                        /* vg auto-on flag */
			x_flag = 1;
			x_value = optarg;
			break;
	        case 'N':
			N_flag = 1;              /* New control Block */
			break;
		default:
		case '?':
			return(-1);
		}
	}
	return(1);
}




/*
 * NAME: main
 *
 * FUNCTION:
 *
 */
int main (argc, argv)
int argc;                       /* number of command line arguments (INPUT) */
char *argv[];                   /* array of command line argument strings (INPUT) */
{
	STRING
		*type,          /* logical volume type */
		*lvname,        /* logical volume name */
		*vgname,        /* volume group name */
	        *fs,            /* fs (/etc/filesystem entry) */
		*lvid;
	int
		retval,            /* return code */
		*num_lps,
		*upperbound,
		*copies,
		*striping_width,    /* stripe width */
		*stripe_exp;  /* stripe blk in exponent */
	char
		**cmdarg,          /* pointer to command argument string */
		*interpolicy,
		*intrapolicy,
		*strict,
		*relocatable,
		*auto_on,
	        *label;
	struct LVCB
		*lvcb;
	RETURN_CODE
		rc;

	struct LVCB *build_lvcb();
	RETURN_CODE write_lvcb();


	cmdarg = (char **) get_zeroed_mem (argc, sizeof(char *));
	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,cmdarg)) < 0) exit(retval);

	intrapolicy = a_flag ? a_value : (char *) NULL;
	interpolicy = e_flag ? e_value : (char *) NULL;
	fs = f_flag ? f_value : (char *) NULL;
	lvid = i_flag ? i_value : (char *) NULL;
	relocatable = r_flag ? r_value : (char *) NULL;
	strict = s_flag ? s_value : (char *) NULL;
	type = t_flag ? t_value : (char *) NULL;
	vgname = v_flag ? v_value : (char *) NULL;
	auto_on = x_flag ? x_value : (char *) NULL;
	label = L_flag ? L_value : (char *) NULL;
	
	if (n_flag) {
		num_lps = (int *) getmem(sizeof(int));
		*num_lps = atoi(n_value);
	}
	else
		num_lps = (int *) NULL;

	if (c_flag) {
		copies = (int *) getmem(sizeof(int));
		*copies = atoi(c_value);
	}
	else
		copies = (int *) NULL;

	if (O_flag) {
		striping_width = (int *) getmem(sizeof(int));
		*striping_width = atoi(O_value);
	}
	else
		striping_width = (int *) NULL;

	if (u_flag) {
		upperbound = (int *) getmem(sizeof(int));
		*upperbound = atoi(u_value);
	}
	else
		upperbound = (int *) NULL;

	if (S_flag) {
		stripe_exp = (int *) getmem(sizeof(int));
		*stripe_exp = atoi(S_value);
	}
	else
		stripe_exp = (int *) NULL;

	if (cmdarg[0] == (char *) NULL) {
		rc = FAILURE;
	}
	else {
		lvname = cmdarg[0];
		lvcb = build_lvcb (type,
				   lvname,
				   vgname,
				   lvid,
				   num_lps,
				   interpolicy,
				   intrapolicy,
				   upperbound,
				   strict,
				   copies,
				   relocatable,
				   auto_on,
				   N_flag,
				   label,
				   fs,
				   striping_width,
				   stripe_exp);

		if (lvcb != (struct LVCB *) NULL)
			rc = write_lvcb (lvname, lvcb);
		else
			rc = FAILURE;
	}
	return rc;
}


