static char sccsid[] = "@(#)29	1.11.1.1  src/bos/usr/sbin/lvm/lchangepv.c, cmdlvm, bos411, 9428A410j 1/18/94 10:05:29";

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
 * FILE: lchangepv.c
 *
 * FILE DESCRIPTION: Source code for lchangepv program.  "lchangepv" provides
 *     a user command to prepare data and call the lvm_changepv() function.
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
#include <sys/audit.h>
#include <sys/priv.h>
#include <sys/id.h>

/*              include file for message texts          */
#include "cmdlvm_msg.h"
#include "cmdlvm_def.h"
#include <locale.h>

/*
 * EXTERNAL PROCEDURES CALLED:
 */

/* local library functions */
extern int setflg();
extern int get_uniq_id();
extern char *lvm_msg();

/*
 * GLOBAL VARIABLES:
 */

/* flags for required values */
#define VGID	1		 /* error return code if vg id not specified on command line */
#define PVID	2		 /* error return code if pv id not specified on command line */

/* external variables */
extern int optind;		 /* index to next argv argument - incremented by getopt      */
extern char *optarg;		 /* pointer to command line argument string - set by getopt  */
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog;			 /* points to name of the current Program */


/* Pointer to command line parameters associated with each command line option.   */
/* These variables only contain values if corresponding flag is true - see below. */
static char *g_value;
static char *p_value;
static char *r_value;
static char *a_value;

/* Each flag corresponds to a command option - flag true if option specified on command line */
static int g_flag;
static int p_flag;
static int r_flag;
static int a_flag;


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
	while ((rc = getopt(argc, argv, "g:p:r:a:")) != EOF) {
		switch (rc) {
		case 'g':
			g_flag = 1;
			g_value = optarg;
			break;
		case 'p':
			p_flag = 1;
			p_value = optarg;
			break;
		case 'r':
			r_flag = 1;
			r_value = optarg;
			break;
		case 'a':
			a_flag = 1;
			a_value = optarg;
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
 *     have been received.
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
	fprintf(stderr,  lvm_msg(CHPV_USAGE) , Prog);
}


/*
 * NAME: main
 *
 * FUNCTION: lchangepv reads command line input from the user,
 *    parses it, performs basic validation, formats it, and calls
 *    lvm_changepv passing the formatted values.  If an error is
 *    encountered then a message is displayed.
 *
 * RETURN VALUE DESCRIPTION: Zero if no error. Non-zero otherwise.
 */

main(argc,argv)

int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
	int retval;	   /* return code */
	struct changepv changepv;    /* lvm record for changing physical volume def */

	int audit_status;     /* status for security auditing */
	char audit_buf[128];  /* buffer for audit log */
	
	/*
	 * suspend regular auditing and lapse privilege
	 */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);
	privilege(PRIV_LAPSE);
	setgidx(ID_EFFECTIVE,getgidx(ID_REAL));

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

	/* Initialize Prog for usage_error function */
	Prog = argv[0];

	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,NULL)) < 0) exit(lvm_rc(retval));

	if ((retval=ck_required()) == 0) {  /* ck_required confirms required parameters were supplied */

		/* convert input data to correct format and store in lvm record */
		get_uniq_id(g_value, &changepv.vg_id);
		get_uniq_id(p_value, &changepv.pv_id);

		if (r_flag) changepv.rem_ret = atoi(r_value);
		else  changepv.rem_ret = LVM_NOCHNG;

		if (a_flag) changepv.allocation = atoi(a_value);
		else  changepv.allocation = LVM_NOCHNG;

		/*
		 * acquire privilege
		 */
		privilege(PRIV_ACQUIRE);
		setgidx(ID_EFFECTIVE,getgidx(ID_SAVED));
		audit_status = AUDIT_OK;
		build_audit_string(audit_buf);
		
		/* pass the record to lvm to change the physical volume definition */
		if (retval = lvm_changepv(&changepv)) {
                    if (retval < 0) {    
			audit_status = AUDIT_FAIL;
			errmsg(retval);
                    }
                    else {
			errmsg(retval);
                    }
		}
	}

	/* write audit log and drop privilege*/
	auditlog("LVM_ChangeVG",audit_status,audit_buf,strlen(audit_buf));
	privilege(PRIV_DROP);

	exit(lvm_rc(retval));
}

/*
 * Build the audit trail
 */
build_audit_string(astring)
     char * astring;
{  
  astring[0] = '\0';   /*  init null astring */
  
  /* pvname */
  strcat(astring," Physical Volume ID = ");
  strcat(astring,g_value);

  /* allocation */
  if (a_flag)
  {
    strcat(astring," Allocation = ");
    strcat(astring,a_value);
  }
  
  /* Max number of Logical partions */
  if (r_flag)
  {
    strcat(astring," State = ");
    strcat(astring,r_value);
  }
}
