static char sccsid[] = "@(#)33	1.12.2.3  src/bos/usr/sbin/lvm/lextendlv.c, cmdlvm, bos411, 9428A410j 3/22/94 23:01:49";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:  getflg, ck_required, setdef, getmaps, main
 *	
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
 * FILE: lextendlv.c
 *
 * FILE DESCRIPTION: Source file for lextendlv program.  "lextendlv" provides
 *     a command level user interface to the lvm low level function lvm_extendlv.
 *
 *     For additional information on this program and the other lvm low level command
 *     programs see the specification "Logical Volume Manager Low-level Command
 *     Interface" and also the interface documentation to the lvm library functions.
 *
 * FUNCTION_NAMES: getflg, ck_required, setdef, getmaps, main
 *
 */


#define _ILS_MACROS	/* performance enhancement for isspace(), etc. call */
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
#include <ctype.h>


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
#define LVID	1     /* error return code if lv id not specified on command line	*/
#define SIZE    2     /* error return code if size not specified on command line   */
#define FNAM    3     /* error return code if file name not specified on command line   */
#define PMAP    4     /* error return code if pp map not specified on command line      */

/* flags for error messages */
#define NOLP	-1     /* incomplete map triplet, missing lp_num */
#define NOPPLP	-2     /* incomplete map triplet, missing pp_num and lp_num */
#define BLNK	-3     /* incomplete map triplet, blank line */
#define NOTFND  -4     /* expected map triplet not found */

/* external variables */
extern int optind;     /* index to next argv argument - incremented by getopt	   */
extern char *optarg;   /* pointer to command line argument string - set by getopt  */
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog;			 /* points to name of the current Program  */

/* Pointer to command line parameters associated with command line options. */
static char *l_value;
static char *s_value;

/* Flags corresponding to command option - flag true if option specified on command line */
static int l_flag;
static int s_flag;
static int Filename = 1;

static struct ext_redlv extendlv;  /* Record to pass to lvm_extendlv to perform the extension */
				   /* ...this record is filled by the getmaps function.       */

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
	while ((rc = getopt(argc, argv, "l:s:")) != EOF) {
		switch (rc) {
		case 'l':
			l_flag = 1;
			l_value = optarg;
			break;
		case 's':
			s_flag = 1;
			s_value = optarg;
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
	if (!l_flag) {
		fprintf(stderr, lvm_msg(MISSING_LVID), Prog);
		Usage_error();
		return(LVID);
	}
	if (!s_flag) {
		fprintf(stderr, lvm_msg(MISSING_ESZ), Prog);
		Usage_error();
		return(SIZE);
	}
	if (!Filename) {
		fprintf(stderr, lvm_msg(MISSING_FN), Prog);
		Usage_error();
		return(FNAM);
	}
	if (!extendlv.parts) {
		fprintf(stderr, lvm_msg(MISSING_MAP), Prog);
		Usage_error();
		return(PMAP);
	}
	return(0);
}


/*
 * NAME: getmaps
 *
 * FUNCTION: Read map information from input file.  A map is composed of
 * multiple partition definitions. Each partition definition is defined by a
 * triplet (physical volume id, logical partition number, physical partition
 * number). Each line in the input file will contain one triplet.  Each triplet
 * is read one at a time, formatted, and stored in the global structure.
 *
 * RETURN VALUE DESCRIPTION: Zero if no error. Non-zero otherwise.
 */

static int getmaps(argv,size)

char **argv;	  /* pointer into the command line string - points to after the last option */
int size;         /* size of extension */
{
	register i;         /* indexes */
	int error = 0;	    /* error code for partition parsing */
	char line[500];     /* buffer to hold one partition definition (one triplet) */
	char temp[25];      /* temporary buffer to hold the pv id string */
	char *ptr;	    /* current position within line buffer */
	struct pp *sptr;    /* next slot to be filled with partition structure pp */
	struct pp *pp;	    /* the primary partition structures - upto 3 possible */
	FILE *fp;	    /* file pointer */


	if (*argv) {	    /* if there is a command field defined after the last option field */
		if (!strcmp("-",*argv)) fp = stdin;     /* if the field is "-", then then read */
							/* ...input from stdin instead of file */
		else {				  /* otherwise, read input from specified file */
			if (!access(*argv,R_OK)) {	/* does the file give read permission  */
				if ((fp = fopen(*argv,"r")) == NULL) {     /* open input file  */
					perror(*argv);	     /* if cannot open file then error */
					return(-1);
				}
			}
			else {		  /* input file does not give read permission so error */
				perror(*argv);
				return(-1);
			}
		}
	}
	else {	/* no map file was specified so exit */
		Filename = 0;
		return(0);
	}

	/* allocate memory for the partition map structures and init the structures */
	pp = (struct pp *)0;
	pp = (struct pp *) getmem(size*sizeof(struct pp));

	if (pp == (struct pp *)0) return(0);   /* if no storage allocated then exit */
	else bzero ((char *) pp,(size*sizeof(struct pp)));   /* zero out the newly allocated space */

	for (i=0;i<size;++i) {        /* for each line from map file */
	    if (fgets(ptr = line,500,fp)) {        /* for each line from map file */
		sptr = pp + i;                  /* get the current map slot */
		if (sscanf(ptr,"%s",temp)) {     /* put pv id in a temporary slot */
			get_uniq_id(temp, &sptr->pv_id);

			while (isspace(*ptr)) ++ptr;	   /* skip over spaces */
			/* scanf did not move ptr so skip past the pv_id str */
			ptr = &ptr[strspn(ptr,"1234567890abcdefABCDEF")];
			/* if more string then read the pp_num */
			if (strlen(ptr) && sscanf(ptr,"%d",&sptr->pp_num)) {

				while (isspace(*ptr)) ++ptr;
				/* scanf didn't move ptr - skip past pp_num str */
				ptr = &ptr[strspn(ptr,"1234567890")];
				/* if more str then read the lp_num */
				if (strlen(ptr) && sscanf(ptr,"%d",&sptr->lp_num));
				else error = NOLP;      /* incomplete map triplet, missing lp_num */
			}
			else error = NOPPLP;            /* incomplete map triplet, missing pp_num and lp_num */
		}
		else error = BLNK;                      /* incomplete map triplet, blank line?? */
	    }
	    else error = NOTFND;                        /* expected map triplet not found */

	    if (error) {          /* if error then print appropriate error message */
		if (error == NOTFND)
			fprintf(stderr, lvm_msg(EXPECT_MAP), Prog);
		else {
			fprintf(stderr, lvm_msg(EXPECT_INPT), Prog);
			fprintf(stderr, lvm_msg(RECEIVED),Prog,line);
			if (line[strlen(line)-1] != '\n') putc('\n',stderr);
		}
		fprintf(stderr, lvm_msg(RDERR_MAP), Prog);
		return(-1);
	    }
	    *line = '\0';                  /* reinit line buffer to empty */

	}			/* end of for loop */


	/* load the lvm's extension record with pointers to the new partitions */
	extendlv.parts = pp;	  /* the primary partition */

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
	fprintf(stderr, lvm_msg(EXLV_USAGE), Prog);
}


/*
 * NAME: main
 *
 * FUNCTION: lextendlv reads command line input from the user,
 *    parses it, performs basic validation, formats it, and calls
 *    lvm_extendlv passing the formatted values.  If an error is
 *    encountered then a message is displayed.
 *
 * RETURN VALUE DESCRIPTION: Zero if no error. Non-zero otherwise.
 */

main(argc,argv)

int argc;	   /* number of command line strings in argv */
char **argv;	   /* array of command line strings - from user */
{
	int retval;	   /* return code */
	char **vect_argv;
	char *ptr;	 /* pointer to command token */
	int failed = 0;  /* true if unable to correctly parse */
	struct lv_id lv_id;   /* lvm id for deleting logical volume */


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

	/* Initialize Prog for use in usage_error function if usage error occurs */
	Prog = argv[0];

	/* allocate memory so setflag can store the map info */
	vect_argv = (char **) getmem(argc * sizeof(char *));

	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,vect_argv)) < 0) exit(lvm_rc(retval));
	argv = vect_argv;

	/* Get the map values - at least one map must always be requested. */
	/* The lvm extendlv record is filled by getmaps.                   */
	if (retval = getmaps(argv,atoi(s_value)))
		exit(lvm_rc(retval));


	if ((retval=ck_required()) == 0) {  /* ck_required confirms required parameters were supplied */

		/* parse volume group id and minor number of id. They must be seperated   */
		/* by a period - so key off the period for parsing the values into tokens */
		if (ptr = strtok(l_value,".")) {
			get_uniq_id(ptr,&lv_id.vg_id);
			if (ptr = strtok(NULL,".")) {
				lv_id.minor_num = atoi(ptr);
				if (strtok(NULL,".")) failed = -1;
			}
			else failed = -1;
		}
		else failed = -1;

		if (failed) {
			fprintf(stderr, lvm_msg(BADLVID), Prog);
			exit(lvm_rc(-1));
		}


		extendlv.size = atoi(s_value);     /* number of addition partitions */


		/*
		 * acquire privilege
		 */
		privilege(PRIV_ACQUIRE);
		setgidx(ID_EFFECTIVE,getgidx(ID_SAVED));
		audit_status = AUDIT_OK;
		build_audit_string(audit_buf);

		/* call lvm to extend the logical volume */
		if (retval = lvm_extendlv(&lv_id,&extendlv)) {
			errmsg(retval);
			audit_status = AUDIT_FAIL;
		}
	}

	/* write audit log and drop privilege*/
	auditlog("LVM_ChangeLV",audit_status,audit_buf,strlen(audit_buf));
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
  
  /* lvid */
  strcat(astring,"Logical volume ID =");
  strcat(astring,l_value);

  /* Max LV size */
  strcat(astring," Max LV size = ");
  strcat(astring,s_value);

}
