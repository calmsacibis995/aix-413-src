static char sccsid[] = "@(#)24	1.13  src/bos/usr/sbin/lvm/intercmd/getlvname.c, cmdlvm, bos41J, 9522A_all 5/29/95 23:57:25";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *	getflg,main
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
 * FILE NAME: getlvname  
 *
 * FILE DESCRIPTION: Source file for getlvname command.  "Getlvname" provides an
 *	intermediate level of command interface.  
 *	"Getlvname" generates or checks a logical volume name.
 *
 *	For additional information on this program and the other 
 *	lvm immediate level command programs see the specification 
 *	"Logical Volume Manager Intermediate Level Command Interface"
 *	and also the interface documentation to the lvm library functions.
 *         
 * RETURN VALUE DESCRIPTION:
 *			0   Command Successfully completed
 *		        1   Command Unsuccesful 
 *
 * EXTERNAL PROCEDURES CALLED: setflg, genseq, get_prefix
 * 			       checkname, addcuat2,
 * 		               odm_initialize, odm_terminate.       
 *
 * EXTERNAL VARIABLES: optind, optarg.
 *
 * 
 * FUNCTION_NAMES: getflg,  main.
 *
 */

/* Header and Include files */
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>
#include <odmi.h>
#include <sys/cfgodm.h>
#include "cmdlvm.h"
#include "cmdlvm_msg.h"		/* include files for message tests */

/*
 * EXTERNAL PROCEDURES CALLED:
 */
extern int genseq(); 		/* cfg routine to generate sequence number */
extern odm_initialize();	/* cfg routine to initialize ODM */
extern odm_terminate();		/* cfg routine to terminate ODM */
extern char * odm_set_path();	/* odm rountine to set config path */

/* EXTERNAL VARIABLES USED: */

/* local library functions */
extern int setflg();		/* lvm routine for parsing command line */

#define ODMERROR -1		/* error return code from cfg routines */

/*
 * GLOBAL VARIABLES:
 */
extern char *Prog;			/* points to name of the current Program */
extern nl_catd  scmc_catd;   		/* Cat descriptor for scmc conversion */

/* external variables */
extern int optind;		/* index to next argv argument */
				/* - incremented by getopt      */
extern char *optarg;		/* pointer to command line argument string */
				/* - set by getopt  */

/* Pointer to command line parameters associated with each command */
/* line option. This variable will only contain a value if */
/* corresponding flag is true - see below. */
static char *n_value;		/* pointer to hold user defined name */
static char *Y_value;		/* pointer to hold user defined prefix */

/* Each flag corresponds to a command option */
/* - flag true if option specified on command line */
static int n_flag;		/* flag for user defined name */
static int Y_flag;		/* flag for user defined prefix */

/*
 * NAME: getflg
 *
 * FUNCTION: Parse command line options until the first 
 *	non-option is encountered.
 *
 * (NOTES:) For each option in the command line, a global 
 *	flag is set and a global
 *	value is loaded with any accompaning command argument.
 *
 * RETURN VALUE DESCRIPTION: Return negative value (-1) if illegal option found.
 *	Return positive value (1) if options parsed correctly.
 *	Return zero if no command options found.
 */
int getflg(argc,argv)
int argc;	 /* number of command line string in referenced by argv */
char **argv;	 /* array of command line strings */
{
	int rc; 		/* getopt return value */

	/* if no command line options to parse then return */
	if (!argc) return(0);	

	/* switch on each option contained in the command line */
	while ((rc = getopt(argc, argv, "n:R:Y:")) != EOF) {
		switch (rc) {
		case 'n':
			n_flag = 1;
			n_value = optarg;
			break;
		case 'Y':
			Y_flag = 1;
			Y_value = optarg;
			break;
		default:
		case '?':
			fprintf(stderr,lvm_msg(LVNAME_USE),Prog);
			return(-1);
		}
	}
	return(1);
}
 
/*
 * NAME: main
 *
 * FUNCTION: getlvname command reads data from the command line, then
 *	according to what options were used checks a logical volume name for
 *	existance in the CuDv class and checks the last character for a 
 *	number or it creates a new logical volume name with a sequence 
 *	number. 
 *
 * RETURN VALUE DESCRIPTION: Returns the logical volume name if no error. 
 *	One if illegal syntax or ODM error. 
 */


main(argc,argv)

int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
char **argbuf;		/* buffer for the lvtype (option field in setflg) */
char prefix[DBNAMESIZE];	/* string for prefix */
char lvtype[DBNAMESIZE];	/* string for logical volume type */
char lvname[DBNAMESIZE];	/* string for logical volume name */
int rc = 0;		/* return code */
int seqnum;		/* integer for return from genseq */
char seqchar[2];	/* array to store ascii form of sequence number */
int found;        	/* flag for return from get_prefix */
struct CuAt *cuatp;	/* Customized Attribute class pointer */
struct CuDv *cudvp;	/* Customized Device class pointer */
struct listinfo stat_info; 	/* return structure for odm_get_list */
char crit[CRITSIZE];			/* search criteria string */
int	lock_word;

	/* Initialize Prog */
	Prog = argv[0];

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);
 
	/* argbuf will be pointer to command line argument */ 
        argbuf = (char **) getmem(argc * sizeof(char *));
  
	/* setflg sets command option flags and command option values. */
	/* If error then exit. */
	if ((setflg(argc,argv,argbuf)) < 0) exit(FAILURE);

       	odm_initialize();

	/* set the config database path for the classes */
	odm_set_path(CFGPATH);
        lock_word = odm_lock("/etc/objrepos/lvm_lock",ODM_WAIT);

	/* if user defined name then check name for existance */
	/* in CuDv class or number as last character then */
	/* print name out if an error has not occured */
	if (n_flag) {
		bzero(lvname,DBNAMESIZE);
		strncpy(lvname,n_value,DEVNAMESIZE);
		if (strlen(n_value) > DEVNAMESIZE)
			fprintf(stderr,lvm_msg(NAME_TRUNC),Prog,lvname);
		checkname(lvname,Prog);
		printf("%s\n",lvname);
	}
	/* else create a logical volume name with optional */
	/* type or prefix defined */
	else {

		/* if the Y-flag was specified then a prefix */
		/* was specified and it will be */
		/* used to generate the new logical volume name */
		if (Y_flag) {
			bzero(prefix,DBNAMESIZE);
			strncpy(prefix,Y_value,PREFIXSZ);
			if (strlen(Y_value) > DEVNAMESIZE)
				fprintf(stderr,lvm_msg(NAME_TRUNC),Prog,prefix);
		}
	
		/* else check if type was specified by user and */
		/* if not use default type */
		else {
			if (*argbuf != NULL) 
				sprintf(lvtype,"%s",*argbuf);
			else
				/* filesysystem is default type */
				sprintf(lvtype,"%s",FILESYSTEM_TYPE);	
	
			/* check if specified type has a prefix in ODM */
			found = get_prefix(lvtype,prefix);
	
			/* if the specified type does not have a default */
			/* prefix in ODM */
			/* then use the logical volume default prefix */
			if (!found)
				found = get_prefix(LVDEFAULT_TYPE,prefix);
				/* if the default prefix is not found */
				/* use 'lv' or the default prefix */
				if (!found) 
					sprintf(prefix,"%s",LVDEFAULT_PREFIX);
		}
	
		/* call genseq to get new sequence number for */
		/* logical volume name */
		seqnum = genseq(prefix);

		/* if a -1 returned then an ODM error has occured */
		/* and a sequence number cannot */
		/* be generated.  Return FAILURE. */
		if (seqnum == ODMERROR) {
			fprintf(stderr,lvm_msg(SEQ_ERROR),Prog);
			rc = FAILURE;
		}
	
		/* else concatenate the prefix and sequence number to */
		/* form the new logical volume name */
		else {
			sprintf(lvname,"%s%.2d",prefix,seqnum);
			/* print new logical volume name */
			printf("%s\n",lvname);
		}
			
	}

        odm_unlock(lock_word);
	odm_terminate();	/* terminate ODM gracefully */

	exit(rc);

}

