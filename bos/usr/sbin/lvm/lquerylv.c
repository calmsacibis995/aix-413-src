static char sccsid[] = "@(#)34	1.16.2.4  src/bos/usr/sbin/lvm/lquerylv.c, cmdlvm, bos411, 9428A410j 3/4/94 17:33:21";

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
 * FILE: lquerylv.c
 *
 * FILE DESCRIPTION: Source file for lquerylv program.  "lquerylv" provides
 *     a command level user interface to the lvm low level function lvm_querylv.
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
#define LVID	1     /* error return code if lv id not specified on command line */


/* DUMMY VALUE TO BE REPLACED LATER */
static char ODM_type[]= "ODMtype";



/* external variables */
extern int optind;		 /* index to next argv argument - incremented by getopt      */
extern char *optarg;		 /* pointer to command line argument string - set by getopt  */
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog;				       /* points to name of the current Program */

/* This variable only contain a value if corresponding flag is true - see below.  */
static char *L_value;
static char *p_value;

/* Each flag corresponds to a command option - flag true if option specified on command line */
static int L_flag;
static int p_flag;
static int N_flag;
static int G_flag;
static int n_flag;
static int M_flag;
static int S_flag;
static int c_flag;
static int s_flag;
static int P_flag;
static int R_flag;
static int v_flag;
static int w_flag;
static int o_flag;
static int a_flag;
static int d_flag;
static int l_flag;
static int A_flag;
static int r_flag;
static int t_flag;
static int b_flag;
static int B_flag;


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
	while ((rc = getopt(argc, argv, "L:p:NGnMScsPRvwoadlArtbB")) != EOF) {
		switch (rc) {
		case 'L':
			L_flag = 1;
			L_value = optarg;
			break;
		case 'p':
			p_flag = 1;
			p_value = optarg;
			break;
		case 'N':
			N_flag = 1;
			break;
		case 'G':
			G_flag = 1;
			break;
		case 'n':
			n_flag = 1;
			break;
		case 'M':
			M_flag = 1;
			break;
		case 'S':
			S_flag = 1;
			break;
		case 'c':
			c_flag = 1;
			break;
		case 's':
			s_flag = 1;
			break;
		case 'P':
			P_flag = 1;
			break;
		case 'R':
			R_flag = 1;
			break;
		case 'v':
			v_flag = 1;
			break;
		case 'w':
			w_flag = 1;
			break;
		case 'o':
			o_flag = 1;
			break;
		case 'a':
			a_flag = 1;
			break;
		case 'd':
			d_flag = 1;
			break;
		case 'l':
			l_flag = 1;
			break;
		case 'A':
			A_flag = 1;
			break;
		case 'r':
			r_flag = 1;
			break;
		case 't':
			t_flag = 1;
			break;
		case 'b':		/* stripe block in exponent */
			b_flag = 1;
			break;
		case 'B':		/* stripe width */
			B_flag = 1;
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
	if (!L_flag) {
		fprintf(stderr, lvm_msg(MISSING_LVID), Prog);
		Usage_error();
		return(LVID);
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

static int output(q,lv_id)

struct querylv *q;     /* lvm structure containing the query data to be output */
struct lv_id *lv_id;
{
	int tag = 12;  /* printf format value - used for string width and precision */
	register i;     /* offset into map */
	struct pp *pp;  /* map partition record */
	char *ptr;      /* pointer to tag field */

	/* if no query flags were set then use the default - the "a" flag */
	if (!A_flag && !d_flag && !a_flag && !N_flag && !G_flag && !n_flag
	 && !M_flag && !S_flag && !c_flag && !s_flag && !P_flag && !w_flag
	 && !R_flag && !v_flag && !o_flag && !r_flag && !b_flag && !B_flag) 
	 a_flag = 1;

	if (A_flag || a_flag || N_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"LVname:");
		printf("%-*.*s\n",0,LVM_NAMESIZ,q->lvname);
	}
	if (A_flag || a_flag || G_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"VGid:");
		printf("%0x%x\n",q->vg_id.word1,q->vg_id.word2);
	}
	if (A_flag || a_flag || n_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"MaxLP:");
		printf("%1d\n",q->maxsize);
	}
	if (A_flag || a_flag || M_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"MPolicy:");
		printf("%1d\n",q->mirror_policy);
	}
	if (A_flag || a_flag || w_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"MWrtConsist:");
		printf("%1d\n",q->mirwrt_consist);
	}
	if (A_flag || a_flag || S_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"LVstate:");
		printf("%1d\n",q->lv_state);
	}
	if (A_flag || a_flag || c_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"Csize:");
		printf("%1d\n",q->currentsize);
	}
	if (A_flag || a_flag || s_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"PPsize:");
		printf("%1d\n",q->ppsize);
	}
	if (A_flag || a_flag || P_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"Permissions:");
		printf("%1d\n",q->permissions);
	}
	if (A_flag || a_flag || R_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"Relocation:");
		printf("%1d\n",q->bb_relocation);
	}
	if (A_flag || a_flag || v_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"WrVerify:");
		printf("%1d\n",q->write_verify);
	}
	if (A_flag || a_flag || o_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"open_close:");
		printf("%1d\n",q->open_close);
	}
	if (A_flag || a_flag || b_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"stripe_exp:");
		printf("%1d\n",q->stripe_exp);
	}
	if (A_flag || a_flag || B_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"striping_width:");
		printf("%1d\n",q->striping_width);
	}
	if (A_flag || d_flag) {                      /* output the logical partition maps  */
		if (q->mirrors[0] != (struct pp *)0) {  /* if the primary map is not empty    */
			ptr = "LVMAP: ";
			/* output the primary map */
			for (i=0;i<q->currentsize;++i) {
				if (t_flag) printf("%s\t",ptr);
				pp = q->mirrors[0] + i;
				printf("%08x%08x:%-4d ",pp->pv_id.word1,pp->pv_id.word2,pp->pp_num);
				printf("%1d %-8s ",pp->ppstate,ODM_type);
				printf("%08x%08x.%-3d ",lv_id->vg_id.word1,lv_id->vg_id.word2,lv_id->minor_num);
				printf("%-5d ",pp->lp_num);

				if (q->mirrors[1] != (struct pp *)0) {  /* if second pointer not NULL */
				      pp = q->mirrors[1] + i;
				      if (l_flag) {
					 printf("\n");
					 if (t_flag) printf("%s\t",ptr);
					 printf("%08x%08x:%-4d ",pp->pv_id.word1,pp->pv_id.word2,pp->pp_num);
					 printf("%1d %-8s ",pp->ppstate,ODM_type);
					 printf("%08x%08x.%-3d ",lv_id->vg_id.word1,lv_id->vg_id.word2,lv_id->minor_num);
					 printf("%-5d\n",pp->lp_num);

					 if (q->mirrors[2] != (struct pp *)0) {  /* if third pointer not NULL */
					    pp = q->mirrors[2] + i;
					    if (t_flag) printf("%s\t",ptr);
					    printf("%08x%08x:%-4d ",pp->pv_id.word1,pp->pv_id.word2,pp->pp_num);
					    printf("%1d %-8s ",pp->ppstate,ODM_type);
					    printf("%08x%08x.%-3d ",lv_id->vg_id.word1,lv_id->vg_id.word2,lv_id->minor_num);
					    printf("%-5d\n",pp->lp_num);
					 }

				      }
				      else {                    /* no l_flag */
					 printf("%08x%08x:%-4d ",pp->pv_id.word1,pp->pv_id.word2,pp->pp_num);
					 if (q->mirrors[2] != (struct pp *)0) {  /* if third pointer not NULL */
					    pp = q->mirrors[2] + i;
					    printf("%08x%08x:%-4d\n",pp->pv_id.word1,pp->pv_id.word2,pp->pp_num);
					 }                       /* only singly-mirrored */
					 else printf("\n");      /* third pointer is NULL */
				      }

				}                       /* non-mirrored */
				else printf("\n");      /* second pointer is NULL */
			}
		}
	}
	if (r_flag) {        /* output the logical partition maps in the lreducelv input format  */
		if (q->mirrors[0] != (struct pp *)0) {  /* if the primary map is not empty    */
			ptr = "LVMAP: ";
			/* output the primary map */
			for (i=0;i<q->currentsize;++i) {
				pp = q->mirrors[0] + i;
				if (pp->pp_num) {       /* if pp_numm not zero then print out entry */
					if (t_flag) printf("%s\t",ptr);
					printf("%08x%08x %-4d ",pp->pv_id.word1,pp->pv_id.word2,pp->pp_num);
					printf("%-5d\n",pp->lp_num);
				}

				if (q->mirrors[1] != (struct pp *)0) {  /* if second pointer not NULL */
				      pp = q->mirrors[1] + i;
				      if (pp->pp_num) {       /* if pp_numm not zero then print out entry */
					    if (t_flag) printf("%s\t",ptr);
					    printf("%08x%08x %-4d ",pp->pv_id.word1,pp->pv_id.word2,pp->pp_num);
					    printf("%-5d\n",pp->lp_num);
				      }

				      if (q->mirrors[2] != (struct pp *)0) {  /* if third pointer not NULL */
					    pp = q->mirrors[2] + i;
					    if (pp->pp_num) {       /* if pp_numm not zero then print out entry */
						   if (t_flag) printf("%s\t",ptr);
						   printf("%08x%08x %-4d ",pp->pv_id.word1,pp->pv_id.word2,pp->pp_num);
						   printf("%-5d\n",pp->lp_num);                                             }
				      }

				}                       /* non-mirrored */
			}               /* end of for loop */
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
	fprintf(stderr, lvm_msg(QLV_USAGE), Prog);
}


/*
 * NAME: main
 *
 * FUNCTION: lquerylv reads command line input from the user,
 *    parses it, performs basic validation, formats it, and calls
 *    lvm_changelv passing the formatted values.  If an error is
 *    encountered then a message is displayed.
 *
 * RETURN VALUE DESCRIPTION: Zero if no error. Non-zero otherwise.
 */

main(argc,argv)

int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
	int retval;		   /* return code */
	char *ptr;		   /* pointer to command string token - the id */
	int failed = 0; 	   /* true if error parsing lv id */
	struct lv_id lv_id;	   /* lvm logical volume id record */
	struct querylv *querylv;   /* lvm record for querying lvm database */
	char *pvname;

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

	/* Initialize Prog for use in usage_error function if usage error occurs */
	Prog = argv[0];

	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,NULL)) < 0) exit(lvm_rc(retval));

	if ((retval=ck_required()) == 0) {  /* ck_required confirms required parameters were supplied */

		if (ptr = strtok(L_value,".")) {       /* get first token - VG id */
			get_uniq_id(ptr, &lv_id.vg_id);
			if (ptr = strtok(NULL,".")) {  /* get next token - minor number */
				lv_id.minor_num = atoi(ptr);
				if (strtok(NULL,".")) failed = -1;  /* error if more tokens */
			}
			else failed = -1;	       /* unable to parse lv id */
		}
		else failed = -1;		       /* lv id not found */

		if (failed) {
			fprintf(stderr, lvm_msg(BADLVID), Prog);
			exit(lvm_rc(-1));
		}

		if (p_flag) pvname = p_value;
		else pvname = NULL;

		/* pass lv id to lvm to query the lv info */
		if (retval = lvm_querylv(&lv_id,&querylv,pvname)) {
			errmsg(retval);
			exit(lvm_rc(retval));
		}
		else {
			output(querylv,&lv_id);   /* output the received lv info */
			lvm_freebuf(querylv);
		}
	}
	exit(lvm_rc(retval));
}
