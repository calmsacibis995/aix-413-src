static char sccsid[] = "@(#)84	1.6.1.2  src/bos/usr/sbin/lvm/intercmd/getlvcb.c, cmdlvm, bos411, 9428A410j 3/10/94 16:29:30";

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
 * getlvcb.c
 * Reads parameters from the logical control block.
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
extern int setflg();

/*
 * GLOBAL VARIABLES
 */

/* external variables */

extern int optind;   /* index to next argv argument - incremented by getopt */
extern char *optarg; /* pointer to command line arg string - set by getopt  */


/* global variables */
char *Prog="";


/* Each flag corresponds to a command option
 - flag true if option specified on command line */
static int a_flag = 0;
static int c_flag = 0;
static int e_flag = 0;
static int f_flag = 0;
static int i_flag = 0;
static int l_flag = 0;
static int m_flag = 0;
static int n_flag = 0;
static int P_flag = 0;
static int r_flag = 0;
static int s_flag = 0;
static int S_flag = 0;
static int t_flag = 0;
static int u_flag = 0;
static int x_flag = 0;
static int y_flag = 0;
static int L_flag = 0;
static int tags   = 0;


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

	if (!argc) return(0);   /* return if no options to parse */

	/* switch on each option contained in the command line */
	while ((rc = getopt(argc, argv, "acefilLmnrstuxyATSP")) != EOF) {
		switch (rc) {
		case 'a':                        /* intrapolicy */
			a_flag = 1;
			break;
		case 'c':                        /* copies */
			c_flag = 1;
			break;
		case 'e':                        /* interpolicy */
			e_flag = 1;
			break;
	        case 'f':                        /* fs (/etc/filesystem) */
			f_flag = 1;
			break;
		case 'i':                        /* lv id */
			i_flag = 1;
			break;
		case 'l':                        /* lvname */
			l_flag = 1;
			break;
		case 'L':                        /* Label */
			L_flag = 1;
			break;
		case 'm':                        /* machine id */
			m_flag = 1;
			break;
		case 'n':                        /* number of LPs */
			n_flag = 1;
			break;
		case 'P':                        /* stripe width */
			P_flag = 1;
			break;
		case 'r':                        /* relocatable flag */
			r_flag = 1;
			break;
		case 's':                        /* strictness flag */
			s_flag = 1;
			break;
		case 'S':                        /* stripe blk in exponent */
			S_flag = 1;
			break;
		case 't':                        /* type */
			t_flag = 1;
			break;
		case 'u':                        /* upperbound */
			u_flag = 1;
			break;
		case 'x':                        /* date/time created */
			x_flag = 1;
			break;
		case 'y':                        /* date/time modified */
			y_flag = 1;
			break;
		case 'A':                        /* All */
			a_flag = 1;
			c_flag = 1;
			e_flag = 1;
			f_flag = 1;
			i_flag = 1;
			l_flag = 1;
			L_flag = 1;
			m_flag = 1;
			n_flag = 1;
			P_flag = 1;
			r_flag = 1;
			s_flag = 1;
			S_flag = 1;
			t_flag = 1;
			u_flag = 1;
			x_flag = 1;
			y_flag = 1;
			break;
	      case 'T':
			tags = 1;
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
int argc;
char *argv[];
{
	int
	    retval,            /* return code */
	    intrapolicy,
	    copies,
	    interpolicy,
	    fs,
	    lvid,
	    label,
	    lvname_flag,
	    mod_machine_id,
	    num_lps,
	    relocatable,
	    strict,
	    type,
	    upperbound,
	    created_time,
	    modified_time,
	    striping_width,
	    stripe_exp;

	char
		*lvname,
		**cmdarg;          /* pointer to command argument string */

	struct LVCB
		lvcb;
	RETURN_CODE
		rc;

	cmdarg = (char **) get_zeroed_mem (argc, sizeof(char *));
	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,cmdarg)) < 0) exit(retval);

	intrapolicy    = a_flag;
	copies         = c_flag;
	interpolicy    = e_flag;
	fs             = f_flag;
	lvid           = i_flag;
	lvname_flag    = l_flag;
	label          = L_flag;
	mod_machine_id = m_flag;
	num_lps        = n_flag;
	striping_width   = P_flag;
	relocatable    = r_flag;
	strict         = s_flag;
	stripe_exp = S_flag;
	type           = t_flag;
	upperbound     = u_flag;
	created_time   = x_flag;
	modified_time  = y_flag;

	if (cmdarg[0] == (char *) NULL) {
		rc = FAILURE;
	}
	else {
		lvname = cmdarg[0];
		rc = read_lvcb (lvname, &lvcb);
	}

	if ( rc == SUCCESS)
	{
	  if (tags) printf ("\t %s\n", lvcb.validity_tag);

	  if (intrapolicy)
	  {
	    if(tags) printf("\t intrapolicy = ");
	    printf("%c ",lvcb.intrapolicy);
	    if(tags) printf("\n");
	  }

	  if (copies)
	  {
	    if(tags) printf("\t copies = ");
	    printf("%d ",lvcb.copies);
	    if(tags) printf("\n");
	  }
	  
	  if (interpolicy)
	  {
	    if(tags) printf("\t interpolicy = ");
	    printf("%c ",lvcb.interpolicy);
	    if(tags) printf("\n");
	  }
	  
	  if (lvid)
	  {
	    if(tags) printf("\t lvid = ");
	    printf("%s ",lvcb.lvid);
	    if(tags) printf("\n");
	  }

	  if (lvname_flag)
	  {
	    if(tags) printf("\t lvname = ");
	    printf("%s ",lvcb.lvname);
	    if(tags) printf("\n");
	  }

	  if (label)
	  {
	    if(tags) printf("\t label = ");
	    printf("%s ",lvcb.label);
	    if(tags) printf("\n");
	  }

	  if (mod_machine_id)
	  {
	    if(tags) printf("\t machine id = ");
	    printf("%s ",lvcb.mod_machine_id);
	    if(tags) printf("\n");
	  }

	  if (num_lps)
	  {
	    if(tags) printf("\t number lps = ");
	    printf("%d ",lvcb.num_lps);
	    if(tags) printf("\n");
	  }
	  
	  if (relocatable)
	  {
	    if(tags) printf("\t relocatable = ");
	    printf("%c ",lvcb.relocatable);
	    if(tags) printf("\n");
	  }
	  
	  if (strict)
	  {
	    if(tags) printf("\t strict = ");
	    printf("%c ",lvcb.strict);
	    if(tags) printf("\n");
	  }
	  
	  if (striping_width)
	  {
	    if(tags) printf("\t stripe width = ");
	    printf("%d ",lvcb.striping_width);
	    if(tags) printf("\n");
	  }
	  
	  if (stripe_exp)
	  {
	    if(tags) printf("\t stripe size in exponent = ");
	    printf("%d ",lvcb.stripe_exp);
	    if(tags) printf("\n");
	  }
	  
	  if (type)
	  {
	    if(tags) printf("\t type = ");
	    printf("%s ",lvcb.type);
	    if(tags) printf("\n");
	  }
	  
	  if (upperbound)
	  {
	    if(tags) printf("\t upperbound = ");
	    printf("%d ",lvcb.upperbound);
	    if(tags) printf("\n");
	  }
	  
	  if (fs)
	  {
	    if(tags) printf("\t fs = ");
	    printf("%s ",lvcb.fs);
	    if(tags) printf("\n");
	  }
	  
	  if (created_time)
	  {
	    if(tags) printf("\t time created  = ");
	    printf("%s ",lvcb.created_time);
	  }
	  
	  if (modified_time)
	  {
	    if(tags) printf("\t time modified = ");
	    printf("%s ",lvcb.modified_time);
	  }
	  printf("\n");  /* make sure there is a newline at the end */
	}
	return rc;
}

