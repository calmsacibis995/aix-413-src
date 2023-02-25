static char sccsid[] = "@(#)28  1.19  src/bos/usr/sbin/lvm/intercmd/allocp.c, cmdlvm, bos41J, 9507A 2/2/95 17:05:04";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: main, formatted_output
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * FILE DESCRIPTION: Contains main functions for allocp (logical volume
 * partition allocation command). The remaining allocp files are
 *
 * best_alloc.c map_alloc.c mapread.c place1copy.c pvbypv.c
 * max_alloc.c placelps.c
 * 
 * ****  SEE USER DOCUMENTATION FOR A FUNCTIONAL OVERVIEW OF ALLOCATION ****
 */


#include <stdio.h>
#include <lvm.h>
#include "allocp.h"

/* include files for NLS message texts */
#include "cmdlvm_msg.h"
#include <locale.h>

#include <unistd.h>
#include <ctype.h>
#include <string.h>

FILE           *debug;

extern char     *lvm_msg();
extern char     *Prog="allocp";    
 /* NLS Catalog descriptor for scmc conversion */
extern nl_catd  scmc_catd;        

int      mapread(),parsecmd();

extern  LVPP     *rmcopy();
extern LVPP      *max_alloc(), *map_alloc(), *best_alloc();


/* 
 * NAME: convert_lvid
 * 
 * FUNCTION: convert an string representing an lvid to a 
 *    struct lv_id;
 *
 * INPUT: 
 *    char           *ascii_lvid;
 *
 * OUTPUT:
 *    struct lv_id   *bin_lvid;
 *
 * RETURN VALUES: 
 *       0 :  indicates sucess;
 * non zero:  indicates an error; the valu corresponds to an error
 *            message.
 *
 */

int   convert_lvid (ascii_lvid, bin_lvid)

          char           *ascii_lvid;
          struct lv_id   *bin_lvid;
{
  char   *ptr;

  ptr = ascii_lvid;
  if (ptr = strtok (ascii_lvid, "."))
      get_uniq_id (ptr, &bin_lvid->vg_id);
      if (ptr = strtok (NULL, "."))
      {
	bin_lvid->minor_num = (long) atoi (ptr);
	return (0);
      }
  return (-1);
}

/*
 * FILE NAME: parsecmd.c
 *
 * FILE DESCRIPTION: Functions to parse command line parameters for
 * allocp (partition allocation command).
 */

/*
 * EXTERNAL PROCEDURES CALLED: NONE
 */

/* local library functions */
extern int setflg();
extern int usage_error();


/*
 * GLOBAL VARIABLES
 */

/* external variables */
extern char  *lvm_msg();
extern char *Prog;    /* program name */
extern int optind;   /* index to next argv argument - incremented by getopt      */
extern char *optarg;        /* pointer to command line argument string - set by getopt  */
extern nl_catd  scmc_catd;  /* NLS Catalog descriptor for scmc conversion */

/* global variables */

/* Pointer to command line parameter associated with command line option.     */
/* This variable only contains a value if corresponding flag is true - see below.  */
static char *i_value,*t_value,*c_value,*s_value,*u_value,*e_value,*a_value;

/* Each flag corresponds to a command option - flag true if option specified on command line */
static int i_flag = 0, t_flag = 0, c_flag = 0, s_flag = 0, k_flag = 0,
           u_flag = 0, e_flag = 0, a_flag = 0;
static int p_flag = 0;

/*
 * pvids to delete from in the UNCOPY case...
 */
char **Delete_pvids;
int Delete_pvidct = 0;

/*
 * NAME: getflg
 *
 * FUNCTION: Parse command line options until the first non-option 
 *           is encountered. (Called from setflag.)
 *
 * (NOTES:) For each option in the command line, a global flag is set and 
 *     a global value is loaded with any accompaning command argument.
 *
 * RETURN VALUE DESCRIPTION: Return negative value (-1) if illegal option found.
 *                           Return (1) if options parsed correctly.
 *                           Return zero if no command options found.
 */

int getflg(argc,argv)

int argc;        /* number of command line string in referenced by argv */
char **argv;     /* array of command line strings */
{
	int rc;                 /* getopt return value */
	char *p;		/* generic pointer */

	if (!argc) return(0);   /* if no command line options to parse then return */

	/* switch on each option contained in the command line */
	while ((rc = getopt(argc, argv, "i:t:c:s:ku:l:e:a:p:")) != EOF) {
		switch (rc) {
		case 'i':
			i_flag = 1;
			i_value = optarg;
			break;
		case 't':
			t_flag = 1;
			t_value = optarg;
			break;
		case 'c':
			c_flag = 1;
			c_value = optarg;
			break;
		case 's':
			s_flag = 1;
			s_value = optarg;
			break;
		case 'k':
			k_flag = 1;
			break;
		case 'u':
			u_flag = 1;
			u_value = optarg;
			break;
		case 'e':
			e_flag = 1;
			e_value = optarg;
			break;
		case 'a':
			a_flag = 1;
			a_value = optarg;
			break;
		case 'p':		/* delete from these pvids */
			/*
			 * save pvid(s)
			 */
			for (p=optarg; (p=strtok(p, " ,")) != NULL; p=NULL) {
				Delete_pvids = (Delete_pvidct == 0) ?
				  (char **) malloc(sizeof(char *)) :
				  (char **) realloc((void *) Delete_pvids,
					(Delete_pvidct+1) * sizeof(char *));
				*(Delete_pvids + Delete_pvidct++) =
					strdup(p);
				}

			p_flag = 1;
			break;
		default:
		case '?':
			fprintf(stderr, lvm_msg(ALLOCP_USAGE), Prog);
			return(-1);
		}
	}
	return(1);
}





/*
 * NAME: parsecmd
 *
 * FUNCTION: Parsecmd reads command line input from the user, parses it, and performs basic validation.  If an error
 *           is encountered, the subroutine will print a message and exit the program.  All command line values
 *           are returned to the caller.
 */

int parsecmd (argc, argv, type, size, lvchars)

int
    argc;          /* number of command line strings in argc (INPUT) */
char
    **argv;        /* array of command line strings - from user (INPUT) */
TYPE
    *type;         /* allocation type (OUTPUT) */
int *size;         /* number of pp's for allocation (OUTPUT) */
LVCHRS *lvchars;   /* OUTPUT */
{
	int retval;               /* return code */
	char *ptr;                /* pointer to command string token - the id */
	int   failed = 0;         /* true if error parsing lv id */
        LVID  lvid;

	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,NULL)) < 0) exit(retval);

	if (t_flag) /* set type */
		switch (t_value[0]) {
		case ('c') : 
			*type = COPY; 
			break;
		case ('u') : 
			*type = UNCOPY; 
			break;
		case ('x') : 
			*type = EXPLICIT; 
			break;
		case ('e') :
		default    : 
			*type = EXTEND; 
			break;
		}
	else    /* default */
		*type = EXTEND;


	if (e_flag) /* set spread */
		switch (e_value[0]) {
		case ('x') : 
			lvchars->spread = MAX; 
			break;
		case ('p') : 
			lvchars->spread = MAP; 
			break;
		case ('m') :
		default    : 
			lvchars->spread = MIN; 
			break;
		}
	else    /* default */
		lvchars->spread = MIN;


	if (a_flag) /* set region */
		switch (a_value[0]) {
		case ('e') : 
			lvchars->region = OUTEREDGE; 
			break;
		case ('c') : 
			lvchars->region = CENTER; 
			break;
                case ('i') :
                        if (a_value[1] == 'e' ) lvchars->region = INNEREDGE;
                        else lvchars->region = INNERMIDDLE;
                        break;
		case ('m') :
		default    : 
			lvchars->region = OUTERMIDDLE; 
			break;
		}
	else    /* default */
		lvchars->region = OUTERMIDDLE;


	if (i_flag) /* set lvid */
                {
                  convert_lvid( i_value,&(lvchars->lvid));
                } 
	else { 
         /* default to no pvid (a value that will not be used in real system) */
                     lvchars->lvid.vg_id.word1=-1 ;
                     lvchars->lvid.minor_num = -1;
        }

	if (c_flag) /* set number of copies */
		sscanf(c_value,"%d", &(lvchars->copies));
	else    /* default */
		lvchars->copies = 1;
	if (lvchars->copies>3 || lvchars->copies<1) {
		fprintf(stderr, lvm_msg(ALLOCP_PARSECOPY), Prog);
		exit (-1);
	}


	if (s_flag) /* set size of allocation */
		sscanf(s_value,"%d", size);
	else    /* default */
		*size = 0;
	if (*size<0 || *size>MAXPP_PER_VG) {
		fprintf(stderr, lvm_msg(ALLOCP_PARSESIZE), Prog);
		exit (-1);
	}


	if (k_flag) /* set allocation strictness */
		lvchars->strict = 0;
	else    /* default to strict */
		lvchars->strict = 1;


	if (u_flag) /* set upper bound */
		sscanf(u_value,"%d", &(lvchars->upperbound));
	else    /* default to known value that will be later replaced with total number of pv's - once they are known */
		lvchars->upperbound = LVM_MAXPVS;
	if (lvchars->upperbound<1 || lvchars->upperbound>LVM_MAXPVS)
        {
		fprintf(stderr, lvm_msg(ALLOCP_PARSEBOUND), Prog);
		exit (-1);
	}


	/* insure no incompatable types were given on the command line */
	if (
	     /* ((*type==EXPLICIT) && (c_flag || s_flag)) ||  */
	     (*type==COPY && s_flag) ||
/*	     ((*spread==map) && (u_flag || k_flag || a_flag)) ||     */
	     (*type != UNCOPY && p_flag) ||
	     (*type==UNCOPY && (s_flag || k_flag || u_flag | e_flag))
	   ) {
		fprintf(stderr, lvm_msg(ALLOCP_PARMCOMB), Prog);
		exit (-1);
	}

}

/*
 * NAME: formatted_output
 *
 * FUNCTION: Writes the allocation map generated by allocp to standard out
 * in the correct format.
 *
 * OUTPUT: Allocation data to stdout  (format: pvid  pp-number  lp-number)
 *
 * RETURN CODE: none
 */

void formatted_output (newsize, alloc_result)
int newsize;                /* number of partitions allocated (INPUT) */
LVPP alloc_result[]; /* pps allocated - lp,pp,pvid tuples (INPUT) */
{
    char pvid[20];          /* ascii pvid */
    int  p;                  /* loop counter and index */

  if (alloc_result != NULL) {
    for (p=0; p<newsize; p++) {
      if (debug || alloc_result[p].pp_num != -1) {
	sprintf(pvid,"%8.8x%8.8x",alloc_result[p].pv_id.word1,
		alloc_result[p].pv_id.word2);
	printf("%16s         %5d         %5d\n", pvid,
	    alloc_result[p].pp_num, alloc_result[p].lp_num);
     }
    }
  }
}

/*
 * NAME: main
 *
 * FUNCTION: Main function for allocp program. Parses the command line
 * arguments and routes them to the appropriate major component/subfunction.
 *
 * INPUT: 1) command line options
 *           [-i lvid] [-t type] [-c copies] [-s size] [-k]
 *           [-u upperbound] [-e inter-policy] [-a intra-policy]
 *        2) allocation maps from standard-in
 *           These maps are usually generated by lquerylv or lquerypv.
 *           Maps may also contain MODIFY tokens.
 *
 * OUTPUT: 1) allocation map to standard-out (pvid pp-number lp-number)
 *         2) error messages to standard-error
 *
 * RETURN CODE:
 *         zero      ==   successful
 *         non-zero  ==   error
 */

main (argc, argv)
int argc;     		/* number of command line arguments (INPUT) */
char **argv;  		/* array of command line arguments (INPUT) */
{
int      size;         /* number of lp's to extend lv */
TYPE     type;         /* allocation policy type (e.g. extend) */
int      ret_code, newsize=0;
LVPP     *layout;
int      numfinput,numlvpps,nummodpps,maxlp=0;
LVCHRS   lvchars;  /* lv allocation characteristics */

struct lvlist   
           lvlist[3] = { NULL,NULL,0,0,NULL,NULL,0,0,NULL,NULL,0,0 } ;

PV                *pv[LVM_MAXPVS];
struct lv_id       lv_id;
int                numofpvs = 0;
int                ret,i;
RANGE             *ptr;
int                j;
LVMAP             *lpptr;
char              *debug_string=NULL;
int                numofcopies;



debug_string = getenv("_DEBUG_ALLOCP");

if (debug_string != NULL) debug = fopen("/tmp/DEBUG","w") ;

ret_code = 0;
setlocale(LC_ALL,"");  /* set up for NLS */
scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);  /* open message catalog */

parsecmd (argc, argv, &type, &size, &lvchars ) ;

/*    Read the partition maps from standard-in.  These maps define the
 *     state-of-the-world for allocp.  */

mapread(lv_id,&numofpvs, pv, &numlvpps,lvlist,&nummodpps, lvchars.spread) ;

numofcopies=0;
for (i=0; i<3; i++) {
       if (lvlist[i].head != NULL) { numofcopies ++; }
}

  
if (debug) {
 fprintf (debug,"INPUT \n numofcopies=%d\n",numofcopies) ; 
 for (i=0; i < numofpvs; i++) {
  
  fprintf (debug,"pv:%s\n",pv[i]->asciipvid) ; 
  fprintf (debug,"number of free pps = %d\n",pv[i]->free_pp_count);
  fprintf (debug,"number of free sections = %d\n",pv[i]->free_section_count);

  if ( pv[i]->free_range_head != NULL){
       ptr = pv[i]->free_range_head;
       for (j=0; j<pv[i]->free_section_count; j++) {
          fprintf(debug,"start=%d, length=%d  region=%d \n", ptr->start,ptr->length,ptr->region_length);
          ptr = ptr->next_range;
       }
  } 
 }

 for (i=0; i<3; i++) {
  if (lvlist[i].head != NULL) {
    lpptr = lvlist[i].head;
    fprintf(debug,"max_lp_num = %d \n",lvlist[i].max_lp_num);
    fprintf(debug,"num_of_lps = %d \n",lvlist[i].num_of_lps);
    for (;lpptr; lpptr = lpptr->next) {
       fprintf(debug, "pvindex:%d ppnum:%d lpnum:%d \n ",lpptr->pvindex,lpptr->ppnum,lpptr->lpnum);
    }
    fprintf(debug,"\n");
  }
 }
 fprintf(debug,"END OF INPUT \n");
 fflush(debug);
}

if (type == UNCOPY)  /* generate deallocation map for copies */

	layout = rmcopy(lvchars.copies,pv,lvlist, &newsize);

else 
if (lvchars.spread == MAP)  { /* user specified allocation by map */

	layout = map_alloc(type,lvchars.copies,size,numofpvs,pv,
                  numlvpps,lvlist,&newsize,&ret_code);
}

else if (lvchars.spread == MAX) {

	layout = max_alloc(type, lvchars.copies, lvchars.strict,
                         size, numofpvs, pv, numlvpps, lvlist, lvchars.region,
                         lvchars.upperbound, &newsize, &ret_code);
}

else  {  /* optimize allocation */
       
       layout = best_alloc(type,lvchars.copies,lvchars.strict,
                         size,numofpvs,pv, numlvpps,lvlist, lvchars.region,
                         lvchars.spread, lvchars.upperbound,&newsize,&ret_code);

} 

if (ret_code==0) formatted_output (newsize, layout);                      

exit(ret_code);  /* since no abnormal exit, no error occurred */
}
