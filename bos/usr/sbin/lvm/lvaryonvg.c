static char sccsid[] = "@(#)38	1.17.1.1  src/bos/usr/sbin/lvm/lvaryonvg.c, cmdlvm, bos411, 9428A410j 1/18/94 10:06:18";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *	getflg,ck_required,getnams,Usage_error,main
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
 * FILE: lvaryonvg.c
 *
 * FILE DESCRIPTION: Source file for lvaryonvg program.  "lvaryonvg" provides
 *     a command level user interface to the lvm low level function lvm_varyonvg.
 *
 *     For additional information on this program and the other lvm low level command
 *     programs see the specification "Logical Volume Manager Low-level Command
 *     Interface" and also the interface documentation to the lvm library functions.
 *
 * FUNCTION_NAMES: getflg, ck_required, setdef, getnams, main
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
extern int get_uniq_id();
extern char *lvm_msg();


/*
 * GLOBAL VARIABLES
 */

/* flags for required values */
#define VGMG	1     /* error return code if vg major # not specified on command line */
#define VGID	2     /* error return code if vg id not specified on command line      */
#define FNAM    4     /* error return code if name is not specified on command line    */
#define VGNAME  5     /* error return code if vgname is not specified on command line  */


/* external variables */
extern int optind;    /* index to next argv argument - incremented by getopt	  */
extern char *optarg;  /* pointer to command line argument string - set by getopt  */
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog;			 /* points to name of the current Program  */

/* Pointer to command line parameters associated with command line options. */
static char *a_value;
static char *V_value;
static char *g_value;

/* Flags corresponding to command option - flag true if option specified on command line */
static int a_flag;
static int V_flag;
static int g_flag;
static int o_flag;
static int r_flag;
static int n_flag;
static int p_flag;
static int f_flag;
static int t_flag;

/* Filename is non-zero if file was specified on command line, zero if no file specified */
static int Filename = 1;     /* used in getnams and ck_required */


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
	while ((rc = getopt(argc, argv, "a:V:g:ornpft")) != EOF) {
		switch (rc) {
		case 'a':
			a_flag = 1;
			a_value = optarg;
			break;
		case 'V':
			V_flag = 1;
			V_value = optarg;
			break;
		case 'g':
			g_flag = 1;
			g_value = optarg;
			break;
		case 'o':
			o_flag = 1;
			break;
		case 'r':
			r_flag = 1;
			break;
		case 'n':
			n_flag = 1;
			break;
		case 'p':
			p_flag = 1;
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
 *     have been received.  Print error message if missing field is found.
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
	if (!g_flag) {
		fprintf(stderr, lvm_msg(MISSING_VGID), Prog);
		Usage_error();
		return(VGID);
	}
	if (!Filename) {
		fprintf(stderr, lvm_msg(MISSING_FN), Prog);
		Usage_error();
		return(FNAM);
	}
	return(0);
}


/*
 * NAME: getnams
 *
 * FUNCTION: Read PV ID and names from input file and store in the input buffer.
 *
 * RETURN VALUE DESCRIPTION: If zero then no error.  If not zero then error.
 *
 */

static int getnams(argv,v)

char **argv;	   /* array of command line strings - from user */
struct varyonvg *v;
{
	register i=0;	       /* index into vect buffer */
	char line[500];        /* line buffer */
	char temp[20];         /* temporary buffer */
	char *ret;	       /* return code from fgets */
	char *ptr;             /* pointer to line buffer */
	FILE *fp;	       /* file pointer */

	if (*argv) {				       /* if command line parameter exist */
		if (!strcmp("-",*argv)) fp = stdin;    /* if "-" then read from stardard in */

		else {				       /* ...otherwise the parm is a filename */
			if (!access(*argv,R_OK)) {     /* is there read permission on file */
				if ((fp = fopen(*argv,"r")) == NULL) {  /* open the file */
					perror(*argv); /* file open error so print message */
					return(-1);    /* exit since cannot open the file */
				}
			}
			else {			       /* file does not give read permission */
				perror(*argv);	       /* no permission so print error message */
				return(-1);	       /* exit since cannot read file */
			}
		}
	}
	else {			/* no filename specified */
		Filename = 0;	/* no file so print error message */
		return(0);	/* exit since a file is required */
	}

	do {	/* the input file is now open for reading names */
		if (ret=fgets(ptr = line,500,fp)) {    /* get an input line string */
			v->vvg_in.pv[i].pvname = (char *) getmem (strlen(line) + 1); /* allocate memory to store names */
			sscanf(ptr, "%s%s", temp, v->vvg_in.pv[i].pvname);
			get_uniq_id(temp, &v->vvg_in.pv[i++].pv_id);
		}
	} while ((ret != NULL) && (i<LVM_MAXPVS));    /* while not end-of-file & list not full */

	v->vvg_in.num_pvs = i;

	return(0);		 /* all the names read with no error so return success */
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
	fprintf(stderr, lvm_msg(VON_USAGE), Prog);
}


/*
 * NAME: main
 *
 * FUNCTION: lvaryonvg reads command line input from the user,
 *    parses it, performs basic validation, formats it, and calls
 *    lvm_varyonvg passing the formatted values.  If an error is
 *    encountered then a message is displayed.
 *
 * RETURN VALUE DESCRIPTION: Zero if no error. Non-zero otherwise.
 */

main(argc,argv)

int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
	int retval;	   /* return code */
	register i;	   /* index to the pv_ids */
	int tag=12;	   /* printf format parameter */
	char *ptr;	   /* tag display string */
	char **vect_argv;  /* buffer for the file name (non-option field in setflg */
	struct varyonvg varyonvg;  /* record to pass to lvm to vary the volume group */


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

	/* Initialize Prog for use in usage_error function */
	Prog = argv[0];

	/* allocate memory so setflag can store the map info */
	vect_argv = (char **) getmem(argc * sizeof(char *));

	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,vect_argv)) < 0) exit(lvm_rc(retval));
	argv = vect_argv;

	/* get pv ids and pv names from input file or standard input */
	/* and store them in input structure to pass to lvm subroutines */
	if (retval = getnams(argv,&varyonvg))
		exit(lvm_rc(retval));

	if ((retval=ck_required()) == 0) {  /* ck_required confirms required parameters were supplied */

		/* format input values and call lvm to vary the volume group on-line */
		varyonvg.vgname = a_value;
		varyonvg.vg_major = atoi(V_value);

                if (retval = getkmid(&varyonvg.kmid) ) {
                    exit(lvm_rc(retval));
                }

		get_uniq_id(g_value,&varyonvg.vg_id);

		if (!o_flag) varyonvg.noopen_lvs = TRUE;
		else varyonvg.noopen_lvs = FALSE;

		if (r_flag) varyonvg.auto_resync = TRUE;
		else varyonvg.auto_resync = FALSE;

		if (p_flag) varyonvg.misspv_von = TRUE;
		else varyonvg.misspv_von = FALSE;

		if (n_flag) varyonvg.missname_von = TRUE;
		else varyonvg.missname_von = FALSE;

		if (f_flag) varyonvg.override = TRUE;
		else varyonvg.override = FALSE;

		/*
		 * acquire privilege
		 */
		privilege(PRIV_ACQUIRE);
		setgidx(ID_EFFECTIVE,getgidx(ID_SAVED));
		audit_status = AUDIT_OK;
		audit_buf[0] = '\0';

		if (retval = lvm_varyonvg(&varyonvg))  {

			ptr="PV Status:";    /* tag display string */
			for (i=0;i<varyonvg.vvg_out.num_pvs;++i) {
				if (t_flag) printf ("%-*.*s\t",tag,tag,ptr);

				printf("%s\t",varyonvg.vvg_out.pv[i].pvname);
				printf("%08x%08x\t",
					varyonvg.vvg_out.pv[i].pv_id.word1,varyonvg.vvg_out.pv[i].pv_id.word2);

				switch (varyonvg.vvg_out.pv[i].pv_status) {
				case LVM_PVACTIVE:
					printf("PVACTIVE\n");
					break;
				case LVM_PVMISSING:
					printf("PVMISSING\n");
					break;
				case LVM_PVREMOVED:
					printf("PVREMOVED\n");
					break;
				case LVM_INVPVID:
					printf("INVPVID\n");
					break;
				case LVM_DUPPVID:
					printf("DUPPVID\n");
					break;
				case LVM_LVMRECNMTCH:
					printf("LVMRECNMTCH\n");
					break;
				case LVM_NONAME:
					printf("NONAME\n");
					break;
				case LVM_NAMIDNMTCH:
					printf("NAMIDNMTCH\n");
					break;
				case LVM_PVNOTFND:
					printf("PVNOTFND\n");
					break;
				case LVM_PVNOTINVG:
					printf("PVNOTINVG\n");
					break;
				case LVM_PVINVG:
					printf("PVINVG\n");
					break;
				default:
					printf("UNKNOWN\n");
					break;
				}

				ptr = "";    /* tag field only printed once */
			}		     /* end of for loop */

			if (retval < 0) {
				errmsg(retval);
				audit_status = AUDIT_FAIL;
			}
			else {
				errmsg(retval);
			}


		}
	}

	/* write audit log and drop privilege*/
	auditlog("LVM_VaryonVG",audit_status,audit_buf,strlen(audit_buf));
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
  
  /* Volume Group ID */
  strcat(astring," Volume Group ID =");
  strcat(astring,g_value);

  if ( o_flag ) strcat(astring," System Management Mode ");
}
