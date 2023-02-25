static char sccsid[] = "@(#)31	1.14.1.1  src/bos/usr/sbin/lvm/lcreatevg.c, cmdlvm, bos411, 9428A410j 1/18/94 10:05:35";

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
 * FILE: lcreatevg.c
 *
 * FILE DESCRIPTION: Source file for lcreatevg program.  "lcreatevg" provides
 *     a command level user interface to the lvm low level function lvm_createvg.
 *
 *     For additional information on this program and the other lvm low level command
 *     programs see the specification "Logical Volume Manager Low-level Command
 *     Interface" and also the interface documentation to the lvm library functions.
 *
 * FUNCTION_NAMES: getflg, ck_required, setdef, main, output
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
#include "cmdlvm_def.h"
#include "cmdlvm_msg.h"
#include <locale.h>


/*
 * EXTERNAL PROCEDURES CALLED: NONE
 */

/* local library functions */
extern int setflg();
extern char *lvm_msg();


/*
 * GLOBAL VARIABLES
 */

/* flags for required values */
#define VGMG	1     /* error return code if vg major #  not specified on command line */
#define NAME	2     /* error return code if name not specified on command line	*/
#define MXLV	3     /* error return code if max lv's not specified on command line    */
#define SVGD	4     /* error return code if vgd size not specified on command line	*/
#define PPSZ	5     /* error return code if pp size not specified on command line	*/
#define MID     6     /* error return code if module id pointer not specified on command line     */
#define VGNAME  7     /* error return code if vgname not specified on command line     */

/* external variables */
extern int optind;		 /* index to next argv argument - incremented by getopt      */
extern char *optarg;		 /* pointer to command line argument string - set by getopt  */
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog;			 /* points to name of the current Program */

/* Pointer to command line parameters associated with each command line option.   */
/* These variables only contain values if corresponding flag is true - see below. */
static char *a_value;
static char *V_value;
static char *N_value;
static char *n_value;
static char *D_value;
static char *s_value;

/* Each flag corresponds to a command option - flag true if option specified on command line */
static int a_flag;
static int V_flag;
static int N_flag;
static int n_flag;
static int D_flag;
static int s_flag;
static int f_flag;
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
	while ((rc = getopt(argc, argv, "a:V:N:n:D:s:ft")) != EOF) {
		switch (rc) {
		case 'a':
			a_flag = 1;
			a_value = optarg;
			break;
		case 'V':
			V_flag = 1;
			V_value = optarg;
			break;
		case 'N':
			N_flag = 1;
			N_value = optarg;
			break;
		case 'n':
			n_flag = 1;
			n_value = optarg;
			break;
		case 'D':
			D_flag = 1;
			D_value = optarg;
			break;
		case 's':
			s_flag = 1;
			s_value = optarg;
			break;
		case 'f':
			f_flag = 1;
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
	if (!a_flag) {
		fprintf(stderr, lvm_msg(MISSING_VGNAME), Prog);
		Usage_error();
		return(VGNAME);
	}
	if (!V_flag) {
		fprintf(stderr, lvm_msg(MISSING_MAJ), Prog);
		Usage_error();
		return(VGMG);
	}
	if (!N_flag) {
		fprintf(stderr, lvm_msg(MISSING_PVNM), Prog);
		Usage_error();
		return(NAME);
	}
	if (!n_flag) {
		fprintf(stderr, lvm_msg(MISSING_MAXLV), Prog);
		Usage_error();
		return(MXLV);
	}
	if (!D_flag) {
		fprintf(stderr, lvm_msg(MISSING_DASZ), Prog);
		Usage_error();
		return(SVGD);
	}
	if (!s_flag) {
		fprintf(stderr, lvm_msg(MISSING_PPSZ), Prog);
		Usage_error();
		return(PPSZ);
	}
	return(0);
}


/*
 * NAME: output
 *
 * FUNCTION: Send requested data to stdout - if tag has been selected then tag the data.
 *
 * RETURN VALUE DESCRIPTION: No return value.
 */

static int output(vgid)
struct unique_id *vgid;    /* lvm structure for creating a volume group */
{
	int tag = 12;

	if (t_flag) printf("%-*.*s\t",tag,tag,"VGid:");
	printf("%08x%08x\n",vgid->word1,vgid->word2);
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
	fprintf(stderr, lvm_msg(CRVG_USAGE), Prog);
}


/*
 * NAME: main
 *
 * FUNCTION: lcreatevg reads command line input from the user,
 *    parses it, performs basic validation, formats it, and calls
 *    lvm_createvg passing the formatted values.  If an error is
 *    encountered then a message is displayed.
 *
 * RETURN VALUE DESCRIPTION: Zero if no error. Non-zero otherwise.
 */

main(argc,argv)

int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
	int retval;	   /* return code */
	struct createvg createvg;    /* lvm record for creating volume group def */

	int audit_status;     /* status for security auditing */
	char audit_buf[256];  /* buffer for audit log */

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

	/* Initialize Prog for use in usage_error function if usage error occurs */
	Prog = argv[0];

	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,NULL)) < 0) exit(lvm_rc(retval));

	if ((retval=ck_required()) == 0) {  /* ck_required confirms required parameters were supplied */

		/* Load the createvg structure with input values and pass to lvm_createvg. */
		createvg.vgname = a_value;
		createvg.vg_major = atoi(V_value);
		createvg.pvname = N_value;
		createvg.maxlvs = atoi(n_value);
		createvg.vgda_size = atoi(D_value);
		createvg.ppsize = atoi(s_value);

		if( retval = getkmid(&createvg.kmid) )
		{
		  exit(lvm_rc(retval));
		}
		
		if (f_flag) createvg.override = TRUE;
		else createvg.override = FALSE;

		/*
		 * acquire privilege
		 */
		privilege(PRIV_ACQUIRE);
		setgidx(ID_EFFECTIVE,getgidx(ID_SAVED));
		audit_status = AUDIT_OK;
		build_audit_string(audit_buf);
		
		if (retval = lvm_createvg(&createvg)) {
			errmsg(retval);
			audit_status = AUDIT_FAIL;
			if (retval == LVM_VGMEMBER || retval == LVM_MEMACTVVG)
                            fprintf(stderr,"%08x%08x\n", createvg.vg_id.word1,createvg.vg_id.word2);
		}
		else output(&createvg.vg_id);
	}

	/* write audit log and drop privilege*/
	auditlog("LVM_CreateVG",audit_status,audit_buf,strlen(audit_buf));
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
  
  /* Volume Group major number */
  strcat(astring," Volume Group # =");
  strcat(astring,V_value);

  /* PV name */
  strcat(astring," Physical Volume = ");
  strcat(astring,N_value);
  
  /* Max LV's */
  strcat(astring," Max LV's = ");
  strcat(astring,n_value);
  
  /* VG Descriptor Area */
  strcat(astring," VG descriptor size  = ");
  strcat(astring,D_value);
  
  /* PP size */
  strcat(astring," PP size = ");
  strcat(astring,s_value);
  
}
