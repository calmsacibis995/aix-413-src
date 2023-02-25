static char sccsid[] = "@(#)53	1.6  src/bos/usr/sbin/lvm/highcmd/varyonvg.c, cmdlvm, bos41B, 9504A 12/22/94 12:45:07";
/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: main, getflg, prtreport, get_config_pvnames, get_id_name,
 *            get_pvids, errorexit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*                                                                        *
 *                                                                        *
 * FILE : varyonvg.c                                                      *
 *                                                                        *
 * FILE DESCRIPTION: Source file for varyonvg command.                    * 
 *                                                                        *
 * RETURN VALUE DESCRIPTION:                                              *
 *                                                                        *
 * EXTERNAL PROCEDURES CALLED : odm_get_list,odm__initialize,             *
 *                              odm_terminate, lvm_querypv, lvm_varyonvg, *
 *                                                                        *
 * FUNCTIONS: get_config_pvnames, get_majornum,                           *
 *                                                                        *
 *                                                                        *
 **************************************************************************/

#include <stdio.h>
#include <sys/devinfo.h>
#include <ctype.h>
#include <sys/dir.h>
#include <string.h>
#include <lvm.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include "cmdlvm.h"
#include "cmdlvm_msg.h"
#include "../liblv/cmdlvm_def.h"
#include <locale.h>

/* EXTERNAL PROCEDURES CALLED */

extern odm_error ();
extern int odmerrno;
extern int lvm_querypv();
extern int lvm_varyonvg();
extern pid_t fork();
extern int execlp();

/* EXTERNAL DEFINTIONS */

extern char       *optarg;
extern int         optind;
extern nl_catd     scmc_catd;
extern char       *Prog;

/* ERROR DEFINITIONS */

#define VARYON_FAILURE    1
#define ODM_FAILURE       2
#define ODM_OBJNOTFOUND   3
#define MALLOC_FAILURE    5
#define CANTLOCK          6
#define USAGE_ERROR       7
#define NOKMID_FOUND     10
#define GENMAJOR_FAILURE 12
#define NO_ERROR          0

/* GLOBAL VARIABLES */
/* varyonvg flags */
static int n_flag=0;
static int s_flag=0;
static int p_flag=0;
static int f_flag=0;
static int b_flag=0;
static int u_flag=0;


char    vgname[32];
void    errorexit();

/*************************************************************************
*                                                                        *
* NAME: getflg                                                           *
*                                                                        *
* FUNCTION: Parse the command line flags and arguments                   *
*                                                                        *
* INPUT:                                                                 *
*            argc             An INTEGER containing the number of        *
*                             command line strings.                      *
*                                                                        *
*            argv             An array of command line strings.          *
*                                                                        *
* OUTPUT:                                                                *
*            NONE                                                        *
*                                                                        *
*                                                                        *
* RETURN VALUE DESCRIPTION:                                              *
*                          0 Function successfully completed             *
*                         -1 Unsucessful Completion                      *                                              *
*                                                                        *
**************************************************************************/

void getflg(argc,argv)

int    argc;        /* number of command line string in referenced by argv */
char **argv;     /* array of command line strings */
{
        int rc;                    /* getopt return value */

   while ((rc = getopt(argc, argv, "fnspbum:")) != EOF) {

	  switch (rc) {
		case 'f':   /*  override quorum */
			f_flag = 1;
			break;
		case 'n':   /*  do not sync mirrors */
			n_flag = 1;
			break;
		case 's':   /*  do not allow lv opens */
			s_flag = 1;
			break;
		case 'p':  /*  only with no missing pv's */
			p_flag = 1;
			break;
                case 'm':  /* ignore m flag not to break levels before 4.1 */
			break;
		case 'b':  /* break reservation of the disk */
			b_flag = 1;
			break;
		case 'u':  /* after the disk is open, keep unreserved */
			u_flag = 1;
			break;
		default:
			errorexit(USAGE_ERROR);
	}
   }
   if (optind < argc) {
       strcpy(vgname,argv[optind]) ;
   }
   else {
	errorexit(USAGE_ERROR);
   }
}

void 
prtreport(struct varyonvg *varyonvg)
{
static char  *status[12] = 
     /* position in the array determined by the values in 
        lvm.h */

     { "PVACTIVE", "PVMISSING", "UNKNOWN", "PVREMOVED",
       "INVPVID", "DUPPVID", "LVMRECNMTCH", "NONAME",
       "NAMIDMTCH", "PVNOTFND", "PVNOTINVG", "PVINVG" };

int    i;
int    tag=12; /* printf format parameter */
char   *ptr;
long   stat;

ptr="PV Status:";    /* tag display string */

for (i=0;i<varyonvg->vvg_out.num_pvs;++i) {

  printf ("%-*.*s\t",tag,tag,ptr);
  printf("%s\t",varyonvg->vvg_out.pv[i].pvname);
  printf("%08x%08x\t", varyonvg->vvg_out.pv[i].pv_id.word1,varyonvg->vvg_out.pv[i].pv_id.word2);
  stat = varyonvg->vvg_out.pv[i].pv_status ;
  if (stat > 0 && stat < 13 ) printf("%s\n", status[stat-1]);
  else printf("UNKNOWN\n");
  ptr = "";    /* tag field only printed once */
}
}

/*************************************************************************
*                                                                        *
* NAME: get_config_pvnames                                               *
*                                                                        *
* FUNCTION: Get a list of all configured physical volumes (names)        *
*           from ODM.                                                    *
*                                                                        *
* INPUT:    NONE                                                         *
*                                                                        *
* OUTPUT:                                                                *
*            pvlist_ptr       A POINTER to a POINTER of character        *
*                             strings used to store the physical.        *
*                             volume names.                              *
*                                                                        *
*            num_of_pvnames   An INTEGER used to store the number        *
*                             of configured physical volumes.            *
*                                                                        *
* RETURN VALUE DESCRIPTION:                                              *
*                          0 Function successfully completed             *
*                          2 ODM Access Error                            *
*                          3 Object not found in ODM                     *
*                          5 MALLOC Failure                              *
*                                                                        *
*                                                                        *
*                                                                        *
**************************************************************************/

void get_config_pvnames (pvlist_ptr, num_of_pvnames )

struct pvlist **pvlist_ptr;
int *num_of_pvnames;
{
  struct CuAt *cuatp;

  struct listinfo stat_info;
  int index;

  /* Get a list of all configured physical volumes from the CuDv class */

  cuatp =  (struct CuAt *) odm_get_list(CuAt_CLASS, 
  					"attribute = 'pvid'",
					&stat_info,
                                        MAXNUMPVS, 1);

  if ((int)cuatp == -1) { errorexit(ODM_FAILURE); }

  if (stat_info.num == 0) { errorexit (ODM_OBJNOTFOUND); }
  else {
      *num_of_pvnames = stat_info.num;
      *pvlist_ptr = (struct pvlist *) malloc
                                       (sizeof (struct pvlist) * stat_info.num);

      if (*pvlist_ptr == NULL) {
          odm_free_list (cuatp, &stat_info);
          errorexit (MALLOC_FAILURE);
      }
      else {
          for (index = 0; index < stat_info.num ;index ++ ) {
             strcpy ( (*pvlist_ptr + index) -> pvname, (cuatp + index) -> name);
             sscanf((cuatp + index) -> value, "%08x%08x", &((*pvlist_ptr + index) ->pvid.word1),
                     &((*pvlist_ptr + index) ->pvid.word2));

            (*pvlist_ptr +index)->pvid.word3 = 0;
            (*pvlist_ptr +index)->pvid.word4 = 0;

          } /* endfor */

          odm_free_list (cuatp, &stat_info);
        } /* endif */
    } /* endif */
}


void get_id_name (name_ptr, id_ptr)
char *name_ptr;
struct unique_id *id_ptr;
{
   struct CuAt *cuatp;
   struct listinfo stat_info;
   char crit[128];

   sprintf (crit, "name = '%s' AND attribute = vgserial_id", name_ptr);
   cuatp =  (struct CuAt *) odm_get_list(CuAt_CLASS, crit, &stat_info,
                                         1,1);

   if ((int)cuatp == -1) { errorexit (ODM_FAILURE); }

   if (stat_info.num == 0) { errorexit (ODM_OBJNOTFOUND); }
   else {
       sscanf((cuatp ->value), "%08x%08x", &(id_ptr -> word1),
               &(id_ptr ->word2));
       odm_free_list(cuatp,&stat_info);
     }
}

int get_pvids(vgname, varyonvg) 
char   *vgname;
struct varyonvg  *varyonvg;
{

   struct CuAt *cuatp;
   struct listinfo stat_info;
   char crit[128];
   int error;
   int num,i;

     	sprintf(crit,"name = '%s' and attribute = '%s'",vgname,"pv");
	cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS, crit,&stat_info,LVM_MAXPVS,1);
	if ((int)cuatp == -1) odm_error(odmerrno);
	/* if there were no physical volumes, print an error message */
	if (!stat_info.num) {
         /*	fprintf(stderr,lvm_msg(PVS_NOT_FOUND),Prog); */
		odm_terminate();
		errorexit(OBJNOTFOUND);
	}
	else {
            num = stat_info.num;
            varyonvg->vvg_in.num_pvs = num;
	    for (i=0; i< num; i++) {
              sscanf(((cuatp+i)->value), "%08x%08x", &(varyonvg->vvg_in.pv[i].pv_id.word1),
                                                  &(varyonvg->vvg_in.pv[i].pv_id.word2));
	    }
	}
}

/*************************************************************************
*                                                                        *
* NAME: errorexit                                                          *
*                                                                        *
* FUNCTION: error exit 
*                                                                        *
**************************************************************************/

void errorexit (error)
int error;
{
  if (error == USAGE_ERROR) {
    fprintf(stderr,catgets(scmc_catd,LVMSET,1014,MSG_1014), Prog);
  }
  else if (error == CANTLOCK) {
    fprintf(stderr,catgets(scmc_catd,LVMSET,VG_LOCKED,MSG_VG_LOCKED), Prog);
  }
  else if (error == LVM_NOQUORUM) {
       fprintf(stderr,catgets(scmc_catd,LVMSET,NOQUORUM_LVM,MSG_NOQUORUM_LVM), Prog);
  } 
  else if (error == LVM_MISSINGPV) {
       fprintf(stderr,catgets(scmc_catd,LVMSET,MISSINGPV_LVM,MSG_MISSINGPV_LVM), Prog);
  }
  else {
	errmsg(error);	
  }
/*  odm_terminate(); */
  if ( error < 0 ) error = VARYON_FAILURE;
  exit(error);
}

/*************************************************************************
*                                                                        *
* NAME: main                                                             *
*                                                                        *
* RETURN VALUE DESCRIPTION:                                              *
*                          0  Command Successfully completed             *
*                          1  Unsuccessful in varying on the root volume *
*                          2  ODM Access Error                           *
*                          3  Object not found in ODM                    *
*                          4  Error from LVM_QUERYPV                     *
*                          5  MALLOC Failure                             *
*                          6  MKNOD Failure                              *
*                          10 No Kernel Module ID found via loadext      *
*                                                                        *
*                                                                        *
**************************************************************************/

main (argc, argv)
int argc;
char **argv;
{

  int error=NO_ERROR,ret;            /* return code */
                                     /* the query information: */
  struct querypv *querypv;           /* lvm record the query information */
                                     /* is returned in */

  struct pvlist 		*pvlist_ptr=NULL, *rootlist_ptr=NULL;
  struct pvlist 		pvnames_buffer [LVM_MAXPVS];
  int				num_of_pvnames=0, index, i;
  struct varyonvg 		varyonvg;
  int				noquorum=0;

Prog = argv[0];
setlocale(LC_ALL,"");
scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

/* use memset function */
memset (&varyonvg,'\0',sizeof (struct varyonvg));
umask(S_IRWXO);

/* parse command line arguments  */
getflg(argc,argv); 

varyonvg.vgname = &vgname[0];

if(lock_a_vg(Prog, vgname)) errorexit(CANTLOCK) ;

/* perform initialization function for odm use */
odm_initialize();

get_id_name(vgname, &varyonvg.vg_id); 

/* get a list of configured physical volumes from odm */
get_config_pvnames (&pvlist_ptr, &num_of_pvnames);

/* determine which of the physical volumes in the */
/* configured list of physical volumes belong to */
/* volume group and store in the varyonvg */
/* structure */

get_pvids(vgname,&varyonvg);

for (i=0; i<varyonvg.vvg_in.num_pvs; i++) {

#ifdef LVM_DEBUG
      printf ("i=%d,%08x%08x%08x%08x\n",i, varyonvg.vvg_in.pv[i].pv_id.word1, 
      varyonvg.vvg_in.pv[i].pv_id.word2,
      varyonvg.vvg_in.pv[i].pv_id.word3,
      varyonvg.vvg_in.pv[i].pv_id.word4);
#endif

      for (index = 0;index < num_of_pvnames; index++ ) {
#ifdef LVM_DEBUG
       printf ("index =%d pvname=%s\t\t%08x%08x%08x%08x\n",
			      index,
                              (pvlist_ptr + index) -> pvname, 
                              (pvlist_ptr + index) -> pvid.word1, 
                              (pvlist_ptr + index) -> pvid.word2, 
                              (pvlist_ptr + index) -> pvid.word3, 
                              (pvlist_ptr + index) -> pvid.word4); 
#endif
       if (!memcmp (&(varyonvg.vvg_in.pv[i].pv_id), &(pvlist_ptr[index].pvid), sizeof(struct unique_id))) {
         varyonvg.vvg_in.pv[i].pvname = pvnames_buffer[i].pvname;
         strcpy (varyonvg.vvg_in.pv[i].pvname, (pvlist_ptr + index) -> pvname);
	 break; 
       }
      }
#ifdef LVM_DEBUG
      printf ("i=%d, pvname =%s \n", i, varyonvg.vvg_in.pv[i].pvname);
#endif
}

/* retrieve the kernel module id from odm */
if (getkmid(&varyonvg.kmid)) errorexit(NOKMID_FOUND); 
#ifdef LVM_DEBUG
printf("got kmid\n");
#endif

if((varyonvg.vg_major = genmajor(vgname)) == -1)
          errorexit(GENMAJOR_FAILURE);
#ifdef LVM_DEBUG
printf("got majornum\n");
#endif

/* initialize the varyonvg structure and  call lvm_varyonvg  */

varyonvg.missname_von = TRUE;

if (s_flag) varyonvg.noopen_lvs = TRUE;

varyonvg.misspv_von = TRUE;
if (p_flag) varyonvg.misspv_von = FALSE;

if (f_flag) varyonvg.override = TRUE;

if ((noquorum=get_Q_vgdesc(vgname))==1) { 
   varyonvg.noquorum = TRUE;
   /* don't varyon a no-quorum VG with any missing PVs */
   /* unless force flag is specified */
    if (!f_flag) varyonvg.misspv_von = FALSE;
}

/*
 * This will be the logic for this function with the "-b" and "-u"
 * flags:
 * 	-b	-u
 *	0	0	default varyong, disk opened, reserved
 *	0	1	varyong, disk opened, not reserved
 *	1	0	break reservation, disk forced open, reserved
 *	1	1	break reservation, disk forced open, not reserved
 */

if (b_flag)
	varyonvg.break_res = TRUE;
else
	varyonvg.break_res = FALSE;

if (u_flag)
	varyonvg.unreserved_after_open = TRUE;
else
	varyonvg.unreserved_after_open = FALSE;

if ((error = lvm_varyonvg(&varyonvg)) < 0) {
#ifdef LVM_DEBUG
  printf("fatalerror\n");
#endif
  prtreport(&varyonvg);
  errorexit(error);
}

/*  At this point the VG is varied on */

if (!n_flag) {
        if( (ret = fork()) == 0) {
	    ret = execlp("syncvg", "syncvg","-v", vgname,0);
        }
}

if (error > 0 ) {
     error = 0;
     prtreport(&varyonvg);
     fprintf(stderr,catgets(scmc_catd,LVMSET,954,MSG_954), Prog,vgname);
}
exit (error);
}
