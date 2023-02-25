static char sccsid[] = "@(#)30	1.11.1.2  src/bos/usr/sbin/lvm/lcreatelv.c, cmdlvm, bos411, 9428A410j 3/4/94 17:33:25";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *	getflg,ck_required,setdef,Usage_error,main
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
 * FILE: lcreatelv.c
 *
 * FILE DESCRIPTION: Source file for lcreatelv program.  "lcreatelv" provides
 *     a command level user interface to the lvm low level function lvm_createlv.
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
extern int get_uniq_id();
extern char *lvm_msg();


/*
 * GLOBAL VARIABLES
 */

/* flags for required values */
#define NAME	1     /* error return code if name not specified on command line	*/
#define VGID	2     /* error return code if vg id not specified on command line	*/
#define MNUM	3     /* error return code if minor #  not specified on command line   */

/* default values */
#define DEF_MAXLPS  128   /* default value of max LPs used to create a new LV if not specified */

/* external variables */
extern int optind;		 /* index to next argv argument - incremented by getopt      */
extern char *optarg;		 /* pointer to command line argument string - set by getopt  */
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog;			 /* points to name of the current Program */

/* Pointer to command line parameters associated with each command line option.   */
/* These variables only contain values if corresponding flag is true - see below. */
static char *N_value;
static char *g_value;
static char *n_value;
static char *M_value;
static char *w_value;
static char *s_value;
static char *S_value;
static char *p_value;
static char *O_value;
static char *r_value;
static char *v_value;

/* Each flag corresponds to a command option - flag true if option specified on command line */
static int N_flag;
static int g_flag;
static int n_flag;
static int M_flag;
static int w_flag;
static int s_flag;
static int S_flag;
static int p_flag;
static int O_flag;
static int r_flag;
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
	while ((rc = getopt(argc, argv, "N:g:n:M:w:s:S:O:p:r:v:")) != EOF) {
		switch (rc) {
		case 'N':
			N_flag = 1;
			N_value = optarg;
			break;
		case 'g':
			g_flag = 1;
			g_value = optarg;
			break;
		case 'n':
			n_flag = 1;
			n_value = optarg;
			break;
		case 'w':
			w_flag = 1;
			w_value = optarg;
			break;
		case 'M':
			M_flag = 1;
			M_value = optarg;
			break;
		case 's':
			s_flag = 1;
			s_value = optarg;
			break;
		case 'S':
			S_flag = 1;
			S_value = optarg;
			break;
		case 'O':
			O_flag = 1;
			O_value = optarg;
			break;
		case 'p':
			p_flag = 1;
			p_value = optarg;
			break;
		case 'r':
			r_flag = 1;
			r_value = optarg;
			break;
		case 'v':
			v_flag = 1;
			v_value = optarg;
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
	if (!N_flag) {
		fprintf(stderr, lvm_msg(MISSING_LVNM), Prog);
		Usage_error();
		return(NAME);
	}
	if (!g_flag) {
		fprintf(stderr, lvm_msg(MISSING_VGID), Prog);
		Usage_error();
		return(VGID);
	}
	if (!n_flag) {
		fprintf(stderr, lvm_msg(MISSING_MINOR), Prog);
		Usage_error();
		return(MNUM);
	}
	return(0);
}


/*
 * NAME: setdef
 *
 * FUNCTION: Set change structure to default values.
 *
 * (NOTES:) The default value is not a real value but a flag for the lvm library
 *    routines to use the internal default value.  This enables setdef to set the
 *    default values without doing a query first to find out what the values are.
 *
 * RETURN VALUE DESCRIPTION: NONE
 */

static setdef(createlv)

struct createlv *createlv;    /* the create values in lvm format */
{
	createlv->maxsize = DEF_MAXLPS;
	createlv->mirror_policy = LVM_PARALLEL;
	createlv->permissions = LVM_RDWR;
	createlv->bb_relocation = LVM_RELOC;
	createlv->write_verify = LVM_NOVERIFY;
	createlv->mirwrt_consist = LVM_NOCONSIST;
	createlv->stripe_exp =  0; 
	createlv->striping_width =  0; 
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
	fprintf(stderr, lvm_msg(CRLV_USAGE), Prog);
}


/*
 * NAME: main
 *
 * FUNCTION: lcreatelv reads command line input from the user,
 *    parses it, performs basic validation, formats it, and calls
 *    lvm_createlv passing the formatted values.  If an error is
 *    encountered then a message is displayed.
 *
 * RETURN VALUE DESCRIPTION: Zero if no error. Non-zero otherwise.
 */

main(argc,argv)

int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
	int retval;	   /* return code */
	struct createlv createlv;    /* lvm record for creating logical volume def */

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

	/* Initialize Prog and Usage for use in usage_error function if usage error occurs */
	Prog = argv[0];

	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,NULL)) < 0) exit(lvm_rc(retval));

	if ((retval=ck_required()) == 0) {  /* ck_required confirms required parameters were supplied */

		setdef(&createlv);    /* set create structure to default values */

		/* Load the changelv structure with input values and pass to lvm_changelv. */
		createlv.lvname = N_value;
		get_uniq_id(g_value, &createlv.vg_id);
		createlv.minor_num = atoi(n_value);

		/* If input parameter was not required on the command line then only	   */
		/* load if its flag signals a new value was input on the command line.	   */
		if (M_flag) createlv.mirror_policy = atoi(M_value);
		if (w_flag) createlv.mirwrt_consist = atoi(w_value);
		if (s_flag) createlv.maxsize = atoi(s_value);
		if (S_flag) createlv.stripe_exp = atoi(S_value);
		if (p_flag) createlv.permissions = atoi(p_value);
		if (O_flag) createlv.striping_width = atoi(O_value);
		if (r_flag) createlv.bb_relocation = atoi(r_value);
		if (v_flag) createlv.write_verify = atoi(v_value);

		/*
		 * acquire privilege
		 */
		privilege(PRIV_ACQUIRE);
		setgidx(ID_EFFECTIVE,getgidx(ID_SAVED));
		audit_status = AUDIT_OK;
		build_audit_string(audit_buf);
		
		if (retval = lvm_createlv(&createlv)) {
			errmsg(retval);
			audit_status = AUDIT_FAIL;
		}
	}

	/* write audit log and drop privilege*/
	auditlog("LVM_CreateLV",audit_status,audit_buf,strlen(audit_buf));
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
  
  /* lvname */
  strcat(astring,"Logical Volume Name =");
  strcat(astring,N_value);

  /* mirror policy */
  if (M_flag)
  {
    strcat(astring," Mirror policy = ");
    strcat(astring,M_value);
  }
  
  /* mirror write consistency */
  if (w_flag)
  {
    strcat(astring," Mirror Write Consistency = ");
    strcat(astring,w_value);
  }
  
  /* Max number of Logical partions */
  if (s_flag)
  {
    strcat(astring," Max LP's = ");
    strcat(astring,s_value);
  }
  
  /* Stripe block size */
  if (S_flag)
  {
    strcat(astring," Stripe size = ");
    strcat(astring,S_value);
  }
  
  /* permissions */
  if (p_flag)
  {
    strcat(astring," Permissions = ");
    strcat(astring,p_value);
  }
  
  /* Stripe block size */
  if (O_flag)
  {
    strcat(astring," Stripe Width = ");
    strcat(astring,O_value);
  }
  
  /* Relocation */
  if (r_flag)
  {
    strcat(astring," Relocation = ");
    strcat(astring,r_value);
  }
  
  /* Write verify */
  if (v_flag)
  {
    strcat(astring," Write veryify = ");
    strcat(astring,v_value);
  }
}
