static char sccsid[] = "@(#)44	1.16.2.12  src/bos/usr/sbin/lvm/highcmd/lsvg.c, cmdlvm, bos411, 9428A410j 6/16/94 13:40:25";
/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: main, pfunction, lfunction, nfunction, no_vg_default, defaultvg,
 *            gather_default_lvm_data, gather_odm_data, get_default_odm_data,
 *            get_all_vgnames, get_vg_id
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
 * FILENAME: lsvg.c
 *
 * FILE DESCRIPTION: Contains main functions of lsvg (VG's physical volume list command).
 *
 *                   Remaining lsvg files (and C functions):
 *                              parselsvg.c   (getflg, parselsvg)
 *                              lsutil.c      (get_lv_data, distribution, efficiency_total, efficiency, number_of_megabytes,
 *                                             get_vgname, get_pvdescript, get_odm_data_chme, get_vgid, get_pvid, get_lvid,
 *                                             get_odm_data_emy)
 *                              lvmclib.a     (general LV utilities: setflg, usage_error, getmem, get_zeroed_mem,
 *                                                                   getregions, parsepair)
 *
 *                   This file's C Functions:  (Functions are indented to show the calling hierarchy.)
 *                                             ("*" indicates functions called from multiple places (listed more than once))
 *
 *                                             main
 *                                                     parselsvg (parselsvg.c)
 *                                                     pfunction
 *                                                               get_vg_id
 *                                                               distributions (lsutil.c)
 *                                                               get_pvdescript (lsutil.c)
 *                                                     lfunction
 *                                                               get_vg_id
 *                                                               get_lv_data (lsutil.c)
 *                                                               efficiency_total (lsutil.c)
 *                                                               get_odm_data_emy (lsutil.c)
 *                                                     nfunction
 *                                                               get_vg_pvname
 *                                                               gather_default_lvm_data
 *                                                               gather_odm_data
 *                                                     no_vg_default
 *                                                               get_all_vgnames
 *                                                               get_vg_id
 *                                                     defaultvg
 *                                                               get_vg_id
 *                                                               gather_default_lvm_data
 *                                                               gather_odm_data
 *
 *
 *      *****  SEE USER DOCUMENTATION FOR A FUNCTIONAL OVERVIEW OF LSVG  *****
 */

#define _ILS_MACROS	/* performance enhancement for isxdigit(), etc calls */
#include <stdio.h>
#include "lvm.h"
#include "cmdlvm.h"
#include "ls.h"
#include <string.h>

/* include file for message texts */
#include "cmdlvm_msg.h"
#include <locale.h>
#include <fstab.h>
#include <odmi.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <ctype.h>

extern int odmerrno;
extern int odm_errno();
extern int setfsent();
extern int endfsent();
extern struct fstab * getfsspec();
extern struct CuAt *get_nm_id();
extern void prt_pvmap();
extern void addvg(char *, char ***, int *, int *);

extern struct CuAt *getattr();

extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog="lsvg";    /* program name */

#define SHORT_UNDEFINED_STR "??"  /* default value for undefined string variable - only two chars wide */
#define QUORUM(total_vgdas) ((quorum) =  ((total_vgdas) / 2) + 1)
#define DEFAULT_PRINT(p1, p2, p3, p4) printf("%-15s %-24s %-15s %s\n", p1, p2, p3, p4) /* default function output */
#define P_PRINT(f1, f2, f3, f4, f5) printf("%-18s%-12s%-12s%-12s%s\n", f1, f2, f3, f4, f5)
#define L_PRINT(f1, f2, f3, f4, f5, f7, f8) \
	printf("%-20s%-11s%-6s%-6s%-5s%-14s%s\n", f1, f2, f3, f4, f5, f7, f8)
/* #define L_PRINT(f1, f2, f3, f4, f5, f6, f7, f8) \
	printf("%-20s%-11s%-4s%-4s%-5s%-7s%-14s%s\n", f1, f2, f3, f4, f5, f6, f7, f8)   */

/*********************************************/
/*            Added for Defect 3171          */
/*********************************************/
#define NAME 0
#define IDENT 1
#define FAIL 1
#define ODMERROR -1
extern int check_desc();
/*********************************************/


struct lv_odm_data {  /* used to store lv data retrieved from ODM for default option */
	char auto_on[STRSIZE];            /* VG auto-on indicator */
	char vgname[STRSIZE];             /* VG name the PV is contained in */
};




/*
 * NAME: main
 *
 * FUNCTION: Main function for lsvg.  Parse the command line and call the appropriate function for the choosen option.
 *
 * INPUT: Command line options
 *              lsvg
 *              lsvg -o [-n pvname]
 *              lsvg [-i] [-l | -p | -M] [-n pvname] vgdescript...
 *
 * OUTPUT: List information written to standard-out.
 *         Error messages to standard-error.
 *
 * RETURN CODE: 0 == successful
 *              non-zero == error
 */
main(argc,argv)
int argc;       /* number of "tokens" entered on command line - (INPUT) */
char **argv;    /* array of command line tokens -- argc specifies the array size (INPUT) */
{
	int
		i,        /* array index and loop counter */
		num_vgs;  /* number of VGs entered on the command line */
	int	exitstatus;	/* exit status of the lsvg */
	BOOLEAN
		loption,  /* boolean flag set to true if user selected "l" option from command line */
		ooption,  /* boolean flag set to true if user selected "o" option from command line */
		poption,  /* boolean flag set to true if user selected "p" option from command line */
		noption,  /* boolean flag set to true if user selected "n" option from command line */
		Moption;  /* boolean flag set to true if user selected "M" option from command line */
	char
		sourcepv[STRSIZE],                  /* contains sourcepv if noption is true */
		**vgdescript;			    /* contains command line argument if specified */

	void parselsvg(), pfunction(), lfunction(), Mfunction(), nfunction(), defaultvg(), no_vg_default();
	int quick_name_check();

	exitstatus = SUCCESS;		/* initialize to SUCCESS */

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

	parselsvg(argc, argv, &ooption, &loption, &Moption, &poption, &noption, sourcepv, &vgdescript, &num_vgs);



	/* note: if sourcepv is not specified, then NULL pointer is expected by lower level LVM subroutine */

	if (num_vgs == 0) { /* if no vgdescript was specified on the command line */
		if (noption) /* if a sourcepv was given */
			nfunction(sourcepv);
		else
			no_vg_default(ooption);
	}
	else for (i=0; i< num_vgs; i++) {  /* for each vgdescript on the command line (or from stdin if -i) */

		/*
		 * lock the vg
		 */
		if (quick_name_check(vgdescript[i])) {
			exitstatus = FAIL;	/* if at least one fails */
			continue;
		}

		if (lock_a_vg(Prog, vgdescript[i])) {
			exitstatus = FAIL;	/* if at least one fails */
			continue;
		}

		if (poption)
			pfunction(vgdescript[i], noption ? sourcepv : (char *) NULL);
		else if (loption)
			lfunction(vgdescript[i], noption ? sourcepv : (char *) NULL);
		else if (Moption)
			Mfunction(vgdescript[i], noption ? sourcepv : (char *) NULL);
		else {
			if (i != FIRST_TIME_THROUGH_LOOP)
				printf("\n");   /* leave blank line between default listings for multiple VGs */
			defaultvg(vgdescript[i]);
		}
	}
	exit (exitstatus);
}

/*
 * NAME: quick_name_check 
 *
 * FUNCTION: This function does a quick check in the odm for the volume group
 * 		name or id passed down in the lsvg <val> command.
 *
 * OUTPUT: none
 *
 * RETURN CODE: This returns a SUCCESS or FAIL depending on the odm_get_list
 *       
 * NOTE: If you are looking into this, you may be wondering why this function
 * 	was even created.  After all, before this function was stuck in here,
 *	if the user did an lsvg on a name that didn't exist, the library
 *	call lock_vg flag the error with the message about not finding
 *	the "lock" attribute.  This confused users who were unaware that
 *	the name they were using in lsvg was bad, and were trying to figure
 *	out why the "lock" attribute for their vgname was missing from odm.
 *	This extra function specifically points out the missing/bad name
 *	they used and quits the program earlier.
 *     
 *    
 */
int quick_name_check(vgdescript)
char
	*vgdescript;
{

        struct CuAt *cuatp_bn, *cuatp_bv;
        struct listinfo stat_info_bn, stat_info_bv;
        char crit_bn[CRITSIZE], crit_bv[CRITSIZE];
        int vgdesc;
        int temp; 

	if (vgdescript == NULL) return(FAIL);

        odm_initialize();
        odm_set_path(CFGPATH);

	/* the checks must be done for the case where the value of */
	/* vgdescript is the string name of the volume group or the*/
	/* actual vgid of the volume group.  Thus, you have        */
	/* bn = by_name, bv = by_vgid.				   */
 
        sprintf(crit_bn,"attribute = 'vgserial_id' and name = '%s'",vgdescript);
        sprintf(crit_bv,"attribute = 'vgserial_id' and value = '%s'",vgdescript);
        cuatp_bn=(struct CuAt *)odm_get_list(CuAt_CLASS,crit_bn,&stat_info_bn,1,1);
        cuatp_bv=(struct CuAt *)odm_get_list(CuAt_CLASS,crit_bv,&stat_info_bv,1,1);

        if ( ((int)cuatp_bn == ODMERROR) && ((int)cuatp_bv == ODMERROR))
		odm_error(odmerrno);
        if ( (cuatp_bn == NULL) && (cuatp_bv == NULL) )
         {
		fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,vgdescript);
          odm_terminate();
          return(FAIL);
         }

 odm_terminate();
 return(SUCCESS);

}


/*
 * NAME: pfunction
 *
 * FUNCTION: List all the physical volumes (along with some state info) in the volume group
 *
 * OUTPUT: Data selected by default is written to standard out.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 *
 * NOTE: All of the data fields (some of them normally integer values) are converted to strings.  This is done
 *       since the default value (UNDEFINED_STR) is a string.  This approach was not required but it allows more
 *       descriptive fields to be displayed for the user (i.e. "??????" instead of "0" for unknown integer value).
 */
void pfunction (vgdescript, sourcepv)
char
	*vgdescript,    /* descriptor of the logical volume to list (INPUT) */
	*sourcepv;      /* optional (maybe NULL) pvdescript for LVM to indicate a particular PV to read info from (INPUT) */
{
	int
	    i, j,                        /* array indexs and loop counters */
	    freepps,                     /* number of free PPs on a PV */
	    free_dist[SREGION],          /* the distribution of free PPs on a PV, by region */
	    dummy[SREGION];              /* unused parameter to the distrubution function */
	char
	    pvdescript[STRSIZE],         /* descriptor (name or id) for a PV */
	    vgid_ascii[STRSIZE],         /* string representing vgid */
	    pvstate_ascii[STRSIZE],      /* string representing pv state */
	    totalpps_ascii[STRSIZE],     /* string representing total number of PPs on PV */
	    freepps_ascii[STRSIZE],      /* string representing number of free PPs on PV */
	    distrib_ascii[STRSIZE];      /* string representing free PP distribution */
	RETURN_CODE
		rc,                      /* return code for get_pvdescript */
		query_rc;                /* return code for LVM query */
	struct queryvg
		*queryvg;                /* pointer to required structure for querying VG data from LVM */
	struct querypv
		*querypv;                /* pointer required structure for querying LV data from LVM */
	struct unique_id
		pvid,                    /* binary pvid */
		vgid;                    /* binary vgid */

	RETURN_CODE get_pvdescript();
	void gather_odm_data(), gather_default_lvm_data(), distributions();


	if (get_vg_id(vgdescript, vgid_ascii) != SUCCESS) {  /* vgid is required to query LVM */
		exit (1);
	}

	get_uniq_id(vgid_ascii, &vgid);  /* translate ascii id to binary */

	query_rc = (RETURN_CODE) lvm_queryvg (&vgid, &queryvg, sourcepv);  /* retrieve VG data stored by LVM */
	if (query_rc == SUCCESS)  /* if able to get the VG map then process it */
	{
		for (i=0; i<queryvg->num_pvs; i++) {  /* for each PV in the VG */
			query_rc = (RETURN_CODE) lvm_querypv (&vgid, &queryvg->pvs[i].pv_id, &querypv, sourcepv);
			if (query_rc == SUCCESS) {  /* if pv data successfully retrieved then prepare it for output */
				sprintf(totalpps_ascii, "%d", querypv->pp_count);  /* translate count to ascii */

				for (freepps=j=0; j<querypv->pp_count; j++)  /* calculate number of free PPs */
					if (querypv->pp_map[j].pp_state == LVM_PPFREE)
						freepps++;
				sprintf(freepps_ascii, "%d", freepps);  /* translate free PP count to ascii */

				/* find the distribution by regions of free PPs across the PV */
				distributions (querypv->pp_map, querypv->pp_count, free_dist, dummy);
				sprintf(distrib_ascii, "%.2d..%.2d..%.2d..%.2d..%.2d",  /* translate distribtion to ascii */
				    free_dist[0], free_dist[1], free_dist[2], free_dist[3], free_dist[4]);

				/* translate PV state to an ascii representation */
				if (querypv->pv_state & LVM_PVACTIVE)  strcpy(pvstate_ascii, "active");
				else if (querypv->pv_state & LVM_PVMISSING)  strcpy(pvstate_ascii, "missing");
				else if (querypv->pv_state & LVM_PVREMOVED)  strcpy(pvstate_ascii, "removed");
				else strcpy(pvstate_ascii, UNDEFINED_STR);

				lvm_freebuf(querypv);
			}
			else {  /* if could not access the PV data, then fill data fields with "undefined" strings */
				errmsg (query_rc);  /* print low-level lvm error message */
				strcpy (totalpps_ascii, UNDEFINED_STR);
				strcpy (freepps_ascii, UNDEFINED_STR);
				strcpy (distrib_ascii, UNDEFINED_STR);
				strcpy (pvstate_ascii, UNDEFINED_STR);
			}

			rc = get_pvdescript(queryvg->pvs[i].pv_id, pvdescript);  /* translate id back to name */

			/* print headings first time through loop - it is done here to prevent error messages
			   from coming inbetween the headings and the actual data */
			if (i == FIRST_TIME_THROUGH_LOOP) {
				if (strcmp(vgdescript, vgid_ascii)==IDENTICAL)
					printf("vgid=%s\n", vgid_ascii);
				else
					printf("%s:\n", vgdescript);
				if (rc==SUCCESS)  /* if rc then pvdescript contains the PV name, print PV_NAME heading */
					P_PRINT ("PV_NAME", "PV STATE", "TOTAL PPs", "FREE PPs", "FREE DISTRIBUTION");
				else  /* pvdescript contains the PV id, then print heading to indicate PV ID */
					P_PRINT ("PV ID", "PV STATE", "TOTAL PPs", "FREE PPs", "FREE DISTRIBUTION");
			}

			/* output to stdout the data for one PV */
			P_PRINT(pvdescript, pvstate_ascii, totalpps_ascii, freepps_ascii, distrib_ascii);
		}
		lvm_freebuf(queryvg);
	}
	else  {
		errmsg (query_rc);  /* print low-level lvm error message */
		exit(1);
	      }
}




/*
 * NAME: lfunction
 *
 * FUNCTION: List all the logical volumes (along with some state info) in the volume group
 *
 * OUTPUT: Data selected by default is written to standard out.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 *
 * NOTE: All of the data fields (some of them normally integer values) are converted to strings.  This is done
 *       since the default value (UNDEFINED_STR) is a string.  This approach was not required but it allows more
 *       descriptive fields to be displayed for the user (i.e. "??????" instead of "0" for unknown integer value).
 */

void lfunction (vgdescript, sourcepv)
char
	*vgdescript,    /* descriptor of the logical volume to list (INPUT) */
	*sourcepv;      /* optional (maybe NULL) pvdescript for LVM to indicate a particular PV to read info from (INPUT) */
{
	int
	    i, j,                       /* loop counters and array indexes */
	    pv_count,                   /* number of PVs used by an LV */
	    total_pps;                  /* number of PPs used by an LV */
	char
	    lvdescript[STRSIZE],        /* name of a LV being listed */
	    vgid_ascii[STRSIZE],        /* string representing vgid */
	    mount_point[STRSIZE],       /* mount point */
	    effic_ascii[STRSIZE],       /* efficiency value in ascii */
	    lv_state[STRSIZE],          /* state of the LV */
	    type[STRSIZE],              /* LV type */
	    total_pps_ascii[STRSIZE],   /* string representing total number of PPs on PV */
	    total_lps_ascii[STRSIZE],   /* string representing total number of LPs on PV */
	    total_pvs_ascii[STRSIZE];   /* number of PVs used by LV, in ascii format */
	PERCENTAGE
	    efficiency_value;           /* efficiency value for the PP allocation for an LV */
	RETURN_CODE
		rc,                     /* return code for get_odm_data_emy */
		query_rc;               /* return code for LVM query */
	struct lv_info2
		lv_data[LVM_MAXPVS];    /* the LVM query rearranged by PV -- see structure definition */
	struct queryvg
		*queryvg;               /* required structure for querying VG data from LVM */
	struct querylv
		*querylv;               /* required structure for querying LV data from LVM */
	struct lv_id
		lvid;                   /* lvid in binary format */
	struct unique_id
		vgid;                   /* vgid in binary format */
        struct fstab  
                *fsstruct;
        char 
                tempdev[STRSIZE];       /* device name for the lv */
        struct CuAt 
                *cuatp;
        int  temp;

	RETURN_CODE get_pvdescript(), get_odm_data_emy();
	void gather_odm_data(), gather_default_lvm_data();

/* vgid is required to query LVM */
if (get_vg_id(vgdescript, vgid_ascii) != SUCCESS) {  exit (1); }

get_uniq_id(vgid_ascii, &vgid);  /* translate ascii id to binary */

/* retrieve VG data stored by LVM */
query_rc = (RETURN_CODE) lvm_queryvg (&vgid, &queryvg, sourcepv);  

/* if able to get the VG map then process it */
if (query_rc == SUCCESS)  {

   odm_initialize();
   odm_set_path(CFGPATH);

   if(strcmp(vgdescript, vgid_ascii)==IDENTICAL) printf("vgid=%s\n",vgid_ascii);
   else printf("%s:\n", vgdescript);

   L_PRINT("LV NAME","TYPE","LPs","PPs","PVs","LV STATE","MOUNT POINT");
/*   L_PRINT("LV NAME","TYPE","LPs","PPs","PVs","EFFIC","LV STATE","MOUNT POINT");  */

   /* for each LV in the VG */
   for (i=0; i<queryvg->num_lvs; i++) {  

	query_rc = (RETURN_CODE) 
            lvm_querylv (&queryvg->lvs[i].lv_id, &querylv, sourcepv);
        /* if lv data successfully retrieved */
	if (query_rc == SUCCESS) {  
           /* zero the lv_data structure */
           memset((char *) lv_data, 0, sizeof(lv_data)); 
	   /* rearrange the LVM data into better organization - store by LV */
	   get_lv_data (&queryvg->lvs[i].lv_id, querylv->currentsize, 
                                querylv->mirrors, lv_data, &pv_count, sourcepv);

           /* translate count to ascii format */
	   sprintf(total_pvs_ascii, "%d", pv_count);  
           /* for each PV, count number of PPs */
	   for (total_pps=j=0; j<pv_count; j++) { 
					total_pps += lv_data[j].numpps;
	   }
	   sprintf(total_pps_ascii, "%d", total_pps);
           /* calculate efficiency */
/*	   efficiency_value = efficiency_total (lv_data, pv_count);   */
           /* convert numbers to ascii */
/*	   sprintf(effic_ascii, "%d%c", efficiency_value, '%');    */
	   sprintf(total_lps_ascii, "%d", querylv->currentsize);

           /* build the ascii versin of the LV state */
	   switch (querylv->open_close) {  
		case LVM_QLVOPEN     : strcpy(lv_state, "open"); break;
		case LVM_QLV_NOTOPEN : strcpy(lv_state, "closed"); break;
		default              : strcpy(lv_state, UNDEFINED_STR); break;
           }
	   if (strcmp(lv_state, UNDEFINED_STR) != IDENTICAL)
		switch (querylv->lv_state) {
		 case LVM_LVDEFINED    : strcat(lv_state,"/syncd"); break;
		 case CMDLVM_LVSTALE   : strcat(lv_state,"/stale"); break;
		 default               : strcat(lv_state, UNDEFINED_STR); break;
		}

        /* get the logical volume type from the database */
        cuatp = getattr(querylv->lvname,"type",FALSE,&temp);
	/* if getattr returns ODMERROR then and odm error has occured */
	if ((int)cuatp == -1) odm_error(odmerrno);
	/* if the pointer is not NULL then a match was found */
	if (cuatp != NULL) { strcpy(type,cuatp->value); }
        else strcpy(type,"???");

        /* get the mount point */
        sprintf(tempdev,"/dev/%s",querylv->lvname);
        setfsent();
        fsstruct = getfsspec(tempdev);
        if (fsstruct != NULL) strcpy(mount_point,fsstruct->fs_file);
        else strcpy(mount_point,"N/A");
        endfsent();

        strcpy(lvdescript,querylv->lvname);

	lvm_freebuf(querylv);

	}
	else { /* since query failed - use default values for the data fields */
               /* print low-level lvm error message */
                errmsg (query_rc); 
		strcpy (total_lps_ascii, SHORT_UNDEFINED_STR);
		strcpy (total_pps_ascii, SHORT_UNDEFINED_STR);
		strcpy (total_pvs_ascii, SHORT_UNDEFINED_STR);
/*		strcpy (effic_ascii, SHORT_UNDEFINED_STR);  */
		strcpy (lv_state, UNDEFINED_STR);
	}

	/* output to stdout the data for one LV */
/*	L_PRINT(lvdescript, type, total_lps_ascii, total_pps_ascii, 
                total_pvs_ascii, effic_ascii, lv_state, mount_point);  */
	L_PRINT(lvdescript, type, total_lps_ascii, total_pps_ascii, 
                total_pvs_ascii, lv_state, mount_point);
	}
        odm_terminate();
	lvm_freebuf(queryvg);
}
else { errmsg (query_rc); exit(1);  /* print low-level lvm error message */ }
}





/*
 * NAME: Mfunction
 *
 * FUNCTION: List all the physical volumes (along with detailed info) in
 *           the volume group
 *
 * OUTPUT: Data selected by default is written to standard out.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status
 *              if fatal error.
 *
 */
void Mfunction (vgdescript, sourcepv)
char
	*vgdescript,    /* descriptor of the logical volume to list (INPUT) */
	*sourcepv;      /* optional (maybe NULL) pvdescript for LVM to indicate
					   a particular PV to read info from (INPUT) */
{
	int
	    i;                           /* array indexs and loop counters */
	char
	    pvid_ascii[STRSIZE],         /* string representing pvid */
	    vgid_ascii[STRSIZE];         /* string representing vgid */
	RETURN_CODE
		query_rc;                /* return code for LVM query */
	struct queryvg
		*queryvg;                /* pointer to required structure for
									querying VG data from LVM */
	struct unique_id
		vgid;                    /* binary vgid */




	if (get_vg_id(vgdescript, vgid_ascii) != SUCCESS) {  /* vgid is required to query LVM */
		exit (1);
	}

	get_uniq_id(vgid_ascii, &vgid);  /* translate ascii id to binary */

	query_rc = (RETURN_CODE) lvm_queryvg (&vgid, &queryvg, sourcepv);  /* retrieve VG data stored by LVM */
	if (query_rc == SUCCESS)  /* if able to get the VG map then process it */
	{
		for (i=0; i<queryvg->num_pvs; i++) {  /* for each PV in the VG */
			if (i == FIRST_TIME_THROUGH_LOOP) {
				if (strcmp(vgdescript, vgid_ascii)==IDENTICAL)
					printf("\nvgid=%s\n", vgid_ascii);
				else
					printf("\n%s\n", vgdescript);
			}

			/* convert pvid to string format */
			sprintf(pvid_ascii, "%08x%08x", queryvg->pvs[i].pv_id.word1, queryvg->pvs[i].pv_id.word2);
			/* output to stdout the data for one PV */
			prt_pvmap(pvid_ascii, vgid_ascii, sourcepv);
		}  /* end of for loop */
	  	lvm_freebuf(queryvg);
	}
	else  {
		errmsg (query_rc);  /* print low-level lvm error message */
		exit(1);
	      }
}






/*
 * NAME: nfunction
 *
 * FUNCTION: Display the data for the lsvg command based on the sourcepv. 
 *           All the characteristics of the specified VG are displayed.
 *
 * OUTPUT: Data selected by default is written to standard out.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 *
 * NOTE: All of the data fields (some of them normally integer values) are converted to strings.  This is done
 *       since the default value (UNDEFINED_STR) is a string.  This approach was not required but it allows more
 *       descriptive fields to be displayed for the user (i.e. "??????" instead of "0" for unknown integer value).
 */
void nfunction(sourcepv)
char
	*sourcepv;    /* physical volume name to access (INPUT) */
{
	int
	    odm_rc;

	char
	    vgname[STRSIZE],             /* volume group name */
	    vgid_ascii[STRSIZE],         /* string representing vgid */
	    vgstate_ascii[STRSIZE],      /* string representing vg state */
	    ppsize_ascii[STRSIZE],       /* string representing PP size */
	    num_lvs_ascii[STRSIZE],      /* string representing number of LVs */
	    totalpps_ascii[STRSIZE],     /* string representing total number of PPs on PV */
	    total_vgdas_ascii[STRSIZE],  /* string representing number of VG descriptor areas on the PV */
	    freepps_ascii[STRSIZE],      /* string representing number of free PPs on PV */
	    usedpps_ascii[STRSIZE],      /* string representing number of used PPs on PV */
	    stalepvs_ascii[STRSIZE],     /* string representing the number of stale PVs */
	    stalepps_ascii[STRSIZE],     /* string representing the number of stale PPs */
	    quorum_ascii[STRSIZE],       /* string representing the quorum count */
	    openlvs_ascii[STRSIZE],      /* string representing the number of open lvs */
	    maxlvs_ascii[STRSIZE],       /* string representing the max number of lvs */
	    activepvs_ascii[STRSIZE],    /* string representing the active number of pvs */
	    auto_on_ascii[STRSIZE],      /* string representing if a volume group is automatically varied on */
	    num_pvs_ascii[STRSIZE];      /* string representing the number of PVs in the VG */

	void gather_odm_data(), gather_default_lvm_data();

	if (get_vg_pvname(sourcepv, vgid_ascii) != SUCCESS) {  /* get the vgid - required to query LVM */
		bzero(vgid_ascii,STRSIZE);
	}

	/* get all the VG data that is stored in LVM */
	gather_default_lvm_data(vgid_ascii, sourcepv, ppsize_ascii, totalpps_ascii, maxlvs_ascii, freepps_ascii,
				num_lvs_ascii, usedpps_ascii, openlvs_ascii, quorum_ascii, num_pvs_ascii,
				total_vgdas_ascii, stalepvs_ascii, stalepps_ascii, activepvs_ascii,vgstate_ascii);

	/*
	 * lock the vg
	 */
	if (lock_a_vg(Prog, vgid_ascii))
		return;

	/* get all the VG data that is store in ODM */
	gather_odm_data(vgid_ascii, vgname, auto_on_ascii);

	/* if a no-quorum VG */
	if (get_Q_vgdesc(vgname)) strcpy(quorum_ascii,"1") ;

	/* display in columns the logical volume data */
	DEFAULT_PRINT("VOLUME GROUP:", vgname, "VG IDENTIFIER:", vgid_ascii);
	DEFAULT_PRINT("VG STATE:", vgstate_ascii, "PP SIZE:", ppsize_ascii);
	DEFAULT_PRINT("VG PERMISSION:", "read/write", "TOTAL PPs:", totalpps_ascii);
	DEFAULT_PRINT("MAX LVs:", maxlvs_ascii, "FREE PPs:", freepps_ascii);
	DEFAULT_PRINT("LVs:", num_lvs_ascii, "USED PPs:", usedpps_ascii);
	DEFAULT_PRINT("OPEN LVs:", openlvs_ascii, "QUORUM:", quorum_ascii);
	DEFAULT_PRINT("TOTAL PVs:", num_pvs_ascii, "VG DESCRIPTORS:", total_vgdas_ascii);
	DEFAULT_PRINT("STALE PVs:", stalepvs_ascii, "STALE PPs:", stalepps_ascii);
	DEFAULT_PRINT("ACTIVE PVs:", activepvs_ascii, "AUTO ON:", auto_on_ascii);
}







/*
 * NAME: no_vg_default
 *
 * FUNCTION: Display the default data for the lsvg command when no VGs are specified.  The VGs will then be listed.
 *
 * OUTPUT: Data selected by default is written to standard out.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 *
 */
void no_vg_default(ooption)
int
	ooption;      /* if TRUE only the open volume groups are listed, otherwise all defined VGs are listed */
{
	int
		i,              /* array index and loop counter */
		vg_number;      /* array index and loop counter for VGs */
	char
		vgid_ascii[STRSIZE],            /* string representing vgid */
		**vgnames,			/* list of volume group names */
		vgname[STRSIZE];                /* volume group name */
	struct unique_id
		vgid;                           /* vgid in binary format */
	struct queryvgs
		*queryvgs;                      /* pointer to required structure for querying VG data from LVM */
	RETURN_CODE
		rc;                             /* return code */
	mid_t
		kmid;                           /* kernal module id for LV device driver */

	RETURN_CODE get_vgname(), get_all_vgnames();


	rc = get_all_vgnames(&vgnames, &vg_number);  /* attempt to get list of all existing VGs from ODM */
	if (rc == SUCCESS  &&  vg_number>0)  /* if VG list successfully retrieved */
		if (ooption) {  /* are only the open VGs to be listed */
			rc = getkmid (&kmid);
			if (rc == SUCCESS) {  /* if id was successfully retrived */
				rc = (RETURN_CODE) lvm_queryvgs(&queryvgs, kmid);  /* query LVM for list of open VGs */
				if (rc== SUCCESS)  /* if LVM query was successful */
				{
					for (vg_number=0; vg_number<queryvgs->num_vgs; vg_number++) { /* for each open VG */
						sprintf(vgid_ascii, "%.8x%.8x", queryvgs->vgs[vg_number].vg_id.word1,
						    queryvgs->vgs[vg_number].vg_id.word2);  /* convert id to ascii */
						rc = get_vgname(vgid_ascii, vgname);  /* convert ascii id to name */
						if (rc == SUCCESS)  /* if vgname successfully retrieved */
							printf("%s\n", vgname);  /* display the name */
						else  /* otherwise display the id */
							printf("vgid=%s\n", vgid_ascii);
					}
					lvm_freebuf(queryvgs);
				}
				else {
					errmsg (rc);  /* print low-level lvm error message */
					exit(1);
				     }
			}
		}
		else
			for (i=0; i<vg_number; i++)
				printf("%s\n", vgnames[i]);
}




/*
 * NAME: defaultvg
 *
 * FUNCTION: Display the default data for the lsvg command.  All the characteristics of the specified VG are displayed.
 *
 * OUTPUT: Data selected by default is written to standard out.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 *
 * NOTE: All of the data fields (some of them normally integer values) are converted to strings.  This is done
 *       since the default value (UNDEFINED_STR) is a string.  This approach was not required but it allows more
 *       descriptive fields to be displayed for the user (i.e. "??????" instead of "0" for unknown integer value).
 */
void defaultvg(vgdescript)
char
	*vgdescript;    /* descriptor of the logical volume to list (INPUT) */
{
	char
	    vgname[STRSIZE],             /* volume group name */
	    vgid_ascii[STRSIZE],         /* string representing vgid */
	    vgstate_ascii[STRSIZE],      /* string representing vg state */
	    ppsize_ascii[STRSIZE],       /* string representing PP size */
	    num_lvs_ascii[STRSIZE],      /* string representing number of LVs */
	    totalpps_ascii[STRSIZE],     /* string representing total number of PPs on PV */
	    total_vgdas_ascii[STRSIZE],  /* string representing number of VG descriptor areas on the PV */
	    freepps_ascii[STRSIZE],      /* string representing number of free PPs on PV */
	    usedpps_ascii[STRSIZE],      /* string representing number of used PPs on PV */
	    stalepvs_ascii[STRSIZE],     /* string representing the number of stale PVs */
	    stalepps_ascii[STRSIZE],     /* string representing the number of stale PPs */
	    quorum_ascii[STRSIZE],       /* string representing the quorum count */
	    openlvs_ascii[STRSIZE],      /* string representing the number of open lvs */
	    maxlvs_ascii[STRSIZE],       /* string representing the max number of lvs */
	    activepvs_ascii[STRSIZE],    /* string representing the active number of pvs */
	    auto_on_ascii[STRSIZE],      /* string representing if a volume group is automatically varied on */
	    num_pvs_ascii[STRSIZE];      /* string representing the number of PVs in the VG */

	void gather_odm_data(), gather_default_lvm_data();


	if (get_vg_id(vgdescript, vgid_ascii) != SUCCESS) {  /* get the vgid - required to query LVM */
		exit (1);
	}

	/* get all the VG data that is stored in LVM */
	gather_default_lvm_data(vgid_ascii, NULL, ppsize_ascii, totalpps_ascii, maxlvs_ascii, freepps_ascii,
				num_lvs_ascii, usedpps_ascii, openlvs_ascii, quorum_ascii, num_pvs_ascii,
				total_vgdas_ascii, stalepvs_ascii, stalepps_ascii, activepvs_ascii, vgstate_ascii);

	/* if a no-quorum VG */
	if (get_Q_vgdesc(vgdescript)) strcpy(quorum_ascii,"1") ;

	/* get all the VG data that is store in ODM */
	gather_odm_data(vgid_ascii, vgname, auto_on_ascii);

	/* display in columns the logical volume data */
	DEFAULT_PRINT("VOLUME GROUP:", vgname, "VG IDENTIFIER:", vgid_ascii);
	DEFAULT_PRINT("VG STATE:", vgstate_ascii, "PP SIZE:", ppsize_ascii);
	DEFAULT_PRINT("VG PERMISSION:", "read/write", "TOTAL PPs:", totalpps_ascii);
	DEFAULT_PRINT("MAX LVs:", maxlvs_ascii, "FREE PPs:", freepps_ascii);
	DEFAULT_PRINT("LVs:", num_lvs_ascii, "USED PPs:", usedpps_ascii);
	DEFAULT_PRINT("OPEN LVs:", openlvs_ascii, "QUORUM:", quorum_ascii);
	DEFAULT_PRINT("TOTAL PVs:", num_pvs_ascii, "VG DESCRIPTORS:", total_vgdas_ascii);
	DEFAULT_PRINT("STALE PVs:", stalepvs_ascii, "STALE PPs", stalepps_ascii);
	DEFAULT_PRINT("ACTIVE PVs:", activepvs_ascii, "AUTO ON:", auto_on_ascii);
}




/*
 * NAME: gather_default_lvm_data
 *
 * FUNCTION: Return LVM volume group information in user oriented ascii format.
 *
 * RETURN CODE: none
 */
void gather_default_lvm_data(vgid_ascii, sourcepv, ppsize_ascii, totalpps_ascii, maxlvs_ascii, freepps_ascii,
			     num_lvs_ascii, usedpps_ascii, openlvs_ascii, quorum_ascii, num_pvs_ascii,
			     total_vgdas_ascii, stalepvs_ascii, stalepps_ascii, activepvs_ascii,vgstate_ascii)
char
	    vgid_ascii[],                /* string representing VG identifier (INPUT) */
	    sourcepv[],                  /* maybe NULL, passed to LVM if descriptor is to be read from this PV */
	    ppsize_ascii[],              /* string representing PP size (OUTPUT) */
	    totalpps_ascii[],            /* string representing total number of PPs on PV (OUTPUT) */
	    maxlvs_ascii[],              /* string representing the max number of lvs */
	    freepps_ascii[],             /* string representing number of free PPs on PV */
	    num_lvs_ascii[],             /* string representing number of LVs */
	    usedpps_ascii[],             /* string representing number of used PPs on PV */
	    openlvs_ascii[],             /* string representing the number of open lvs */
	    quorum_ascii[],              /* string representing the quorum count */
	    num_pvs_ascii[],             /* string representing the number of PVs in the VG */
	    total_vgdas_ascii[],         /* string representing number of VG descriptor areas on the PV */
	    stalepvs_ascii[],            /* string representing the number of stale PVs */
	    stalepps_ascii[],            /* string representing the number of stale PPs */
	    activepvs_ascii[],           /* string representing the active number of pvs */
	    vgstate_ascii[];
{
	RETURN_CODE
		query_rc;                /* return code for LVM query */
	int
	    i, j,                        /* counters and array indexs */
	    usedpps=0,                   /* number of used PPs in the VG */
	    totalpps=0,                  /* total number of PPs in the VG */
	    openlvs=0,                   /* number of LVs open in the VG */
	    activepvs=0,                 /* number of PVs that are active in the VG */
	    stalepvs=0,                  /* number of PVs which contain stale PPs */
	    stalepps=0,                  /* number of stale PPs in the VG */
        quorum=0,
	    set_vgid=1,			 /* boolean for vgid check from odm */
	    previous_stale,              /* saves the previous number of stale PPs to determine if the count has grown */
	    num_megabytes;               /* number of megabytes of storage in the VG */
	struct unique_id
		pvid,                    /* binary pvid */
		vgid;                    /* binary vgid */
	struct querypv
	    *querypv;                    /* pointer to structure returned from querying PV data from LVM */
	struct querylv
	    *querylv;                    /* pointer to structure returned from querying LV data from LVM */
	struct queryvg
	    *queryvg;                    /* pointer to structure returned from querying VG data from LVM */

	RETURN_CODE get_pvid();


	get_uniq_id(vgid_ascii, &vgid);  /* convert ascii vg id to binary for querying LVM */
	if (vgid_ascii[0] == '\0') set_vgid = FALSE;

	query_rc = lvm_chkvaryon(&vgid);
	if (query_rc < LVM_SUCCESS) {
             strcpy(vgstate_ascii, "inactive");
        }
        else {
	     strcpy(vgstate_ascii, "active");
        }

	query_rc = (RETURN_CODE) lvm_queryvg (&vgid, &queryvg, sourcepv);  /* retrieve VG data stored by LVM */
	if (!set_vgid) 
		sprintf(vgid_ascii, "%.8x%.8x", vgid.word1,vgid.word2);

	if (query_rc == SUCCESS) {  /* if able to get the VG map then process it */

		/* check the validity of data values, then convert to ascii is valid, use default/"undefined" otherwise */
		if (queryvg->ppsize >= LVM_MINPPSIZ  &&  queryvg->ppsize <= LVM_MAXPPSIZ) {
			num_megabytes = number_of_megabytes(queryvg->ppsize);
			sprintf(ppsize_ascii, "%ld megabyte(s)", num_megabytes);
		}
		else
			strcpy (ppsize_ascii, UNDEFINED_STR);
		if (strcmp(ppsize_ascii, UNDEFINED_STR)!=IDENTICAL)
			sprintf(freepps_ascii, "%d (%d megabytes)", queryvg->freespace, queryvg->freespace * num_megabytes);
		else
			strcpy (ppsize_ascii, UNDEFINED_STR);
		if (queryvg->num_lvs >= 0  &&  queryvg->num_lvs <= queryvg->maxlvs)
			sprintf(num_lvs_ascii, "%d", queryvg->num_lvs);
		else
			strcpy (num_lvs_ascii, UNDEFINED_STR);
		if (queryvg->maxlvs >= 0  &&  queryvg->maxlvs <= LVM_MAXLVS)
			sprintf(maxlvs_ascii, "%d", queryvg->maxlvs);
		else
			strcpy (maxlvs_ascii, UNDEFINED_STR);
		if (queryvg->num_pvs > 0  &&  queryvg->num_pvs <= LVM_MAXPVS)
			sprintf(num_pvs_ascii, "%d", queryvg->num_pvs);
		else
			strcpy (num_pvs_ascii, UNDEFINED_STR);

              
		sprintf(quorum_ascii, "%d", QUORUM(queryvg->total_vgdas));
		sprintf(total_vgdas_ascii, "%d", queryvg->total_vgdas);

		/* the LV maps must be queried to gathre all of the LV data */
		if ((i=strcmp(num_lvs_ascii, UNDEFINED_STR))!=IDENTICAL) {   /* if number of LVs is known (illegal value) */
			for (i=0; i<queryvg->num_lvs; i++) {  /* for each LV in the VG */
				query_rc = (RETURN_CODE) lvm_querylv (&queryvg->lvs[i].lv_id, &querylv, sourcepv);
				if (query_rc == SUCCESS)  /* if data successfully retrieved */
				{
					if (querylv->open_close & LVM_QLVOPEN)  /* count the open LVs */
						openlvs++;
					lvm_freebuf(querylv);
				}
				else {   /* if unsuccessfull query to LVM then exit the loop */
					errmsg (query_rc);  /* print low-level lvm error message */
					break;
				     }
			}
			if (query_rc == SUCCESS)  /* if all lv data successfully retrived from LVM */
				sprintf(openlvs_ascii, "%d", openlvs);
			else  /* use default value */
				strcpy(openlvs_ascii, UNDEFINED_STR);
		}
		else  /* use default value since the number of LVs is not know */
			strcpy(openlvs_ascii, UNDEFINED_STR);

		/* the PV maps must be queried to gather all of the PV data */
		if (strcmp(num_pvs_ascii, UNDEFINED_STR)!=IDENTICAL) {  /* if number of PVs is known (illegal value) */
			for (i=0; i<queryvg->num_pvs; i++)  /* for each PV */
				if (queryvg->pvs[i].state & LVM_PVACTIVE)  /* count PV if it is active */
					activepvs++;
			sprintf(activepvs_ascii, "%d", activepvs);  /* convert the count to ascii */
			for (i=0; i<queryvg->num_pvs; i++) {  /* for each PV */
				query_rc = (RETURN_CODE) lvm_querypv (&vgid, &queryvg->pvs[i].pv_id, &querypv, sourcepv); /* query PV data */
				if (query_rc == SUCCESS) {  /* if pv data successfully retrieved */
					totalpps += querypv->pp_count;  /* add PV's PPs to total */
					previous_stale = stalepps;  /* save current number of stale PPs */
					for (j=0; j<querypv->pp_count; j++)  /* count number of stale PPs on PV */
						if (querypv->pp_map[j].pp_state == CMDLVM_PPSTALE)
							stalepps++;
					if (stalepps > previous_stale)  /* if the PV contains stale PPs */
						stalepvs++;  /* count the number of stale PVs */
					lvm_freebuf(querypv);
				}
				else {
					errmsg (query_rc);  /* print low-level lvm error message */
					break;  /* query failure so exit loop */
				}
			}
			if (query_rc == SUCCESS) {  /* if all pv data successfully retrieved */
				usedpps = totalpps - queryvg->freespace;
				sprintf(totalpps_ascii, "%d (%d megabytes)", totalpps, totalpps * num_megabytes);
				sprintf(usedpps_ascii, "%d (%d megabytes)", usedpps, usedpps * num_megabytes);
				sprintf(stalepps_ascii, "%d", stalepps);
				sprintf(stalepvs_ascii, "%d", stalepvs);
			}
			else {  /* one of the PVs had a query failure, so use default values for the totals */
				strcpy (totalpps_ascii, UNDEFINED_STR);
				strcpy (usedpps_ascii, UNDEFINED_STR);
				strcpy (stalepps_ascii, UNDEFINED_STR);
				strcpy (stalepvs_ascii, UNDEFINED_STR);
				strcpy (activepvs_ascii, UNDEFINED_STR);
			}
		}
		else {  /* if number of PVs was not known (illegal value), use default values for the data fields */
			strcpy (totalpps_ascii, UNDEFINED_STR);
			strcpy (usedpps_ascii, UNDEFINED_STR);
			strcpy (stalepps_ascii, UNDEFINED_STR);
			strcpy (stalepvs_ascii, UNDEFINED_STR);
			strcpy (activepvs_ascii, UNDEFINED_STR);
		}
		lvm_freebuf(queryvg);
	}
	else {  /* unable to get VG map so print error and fill in default values for the associated fields */
		errmsg (query_rc);  /* print low-level lvm error message */
		strcpy (quorum_ascii, UNDEFINED_STR);
		strcpy (total_vgdas_ascii, UNDEFINED_STR);
		strcpy (openlvs_ascii, UNDEFINED_STR);
		strcpy (num_pvs_ascii, UNDEFINED_STR);
		strcpy (maxlvs_ascii, UNDEFINED_STR);
		strcpy (freepps_ascii, UNDEFINED_STR);
		strcpy (usedpps_ascii, UNDEFINED_STR);
		strcpy (totalpps_ascii, UNDEFINED_STR);
		strcpy (num_lvs_ascii, UNDEFINED_STR);
		strcpy (ppsize_ascii, UNDEFINED_STR);
		strcpy (stalepps_ascii, UNDEFINED_STR);
		strcpy (stalepvs_ascii, UNDEFINED_STR);
		strcpy (activepvs_ascii, UNDEFINED_STR);
		exit(1);
	}
}




/*
 * NAME: gather_odm_data
 *
 * FUNCTION: Return ODM volume group information in user oriented ascii format.
 *
 * RETURN CODE: none
 */
void gather_odm_data (vgid_ascii, vgname,  auto_on_ascii)
char *vgid_ascii, *vgname, *auto_on_ascii;
{
	RETURN_CODE
		odm_rc;       /* return code from ODM query */
	struct lv_odm_data
		default_odm;  /* the ODM data for the LV -- see structure definition */

	odm_rc = get_default_odm_data (vgid_ascii, &default_odm);  /* retrieve the lv data stored by ODM */
	if (odm_rc != SUCCESS) {  /* if cannot get the ODM data */
		strcpy (vgname, UNDEFINED_STR);
		strcpy (auto_on_ascii, UNDEFINED_STR);
	}
	else {
		switch (default_odm.auto_on[0]) {  /* translate auto on state to ascii format */
			case 'y' : strcpy(auto_on_ascii, "yes");  break;
			case 'n' : strcpy(auto_on_ascii, "no");  break;
			default  : strcpy(auto_on_ascii, UNDEFINED_STR);  break;
		}
		strcpy (vgname, default_odm.vgname);
	}
}



/*
 * NAME: get_default_odm_data
 *
 * FUNCTION: Return the ODM data for a particular VG.
 *
 * RETURN CODE: 0 == SUCCESS
 *              non-zero == FAILURE
 */
RETURN_CODE get_default_odm_data (vgid, default_odm)
char *vgid;
struct lv_odm_data *default_odm;
{
        struct CuAt *cuatp;
        struct listinfo stat_info;
        char crit[CRITSIZE];
        int vgdesc;
        int temp; 

	if (vgid == NULL) return(FAIL);

        odm_initialize();
        odm_set_path(CFGPATH);

        sprintf(crit,"attribute = 'vgserial_id' and value = '%s'",vgid);
        cuatp=(struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
        if ((int)cuatp == ODMERROR) odm_error(odmerrno);
        if (cuatp == NULL)
         {
          fprintf(stderr,lvm_msg(ID_NOT_FOUND),Prog,vgid);
          odm_terminate();
          return(FAIL);
         }
        sprintf(default_odm->vgname,"%s",cuatp->name);
        vgdesc=check_desc(default_odm->vgname);
        switch (vgdesc)
         {
          case NAME:
                    cuatp=getattr(default_odm->vgname,AUTO_ON,FALSE,&temp);
                    if ((int)cuatp == ODMERROR) odm_error(odmerrno);
                    if (cuatp != NULL)
                     {
                      sprintf(default_odm->auto_on,"%s",cuatp->value);
                      break;
                     }
                    else
                     {
                      fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,AUTO_ON);
                      odm_terminate();
                      return(FAIL);
                     }
          default:
                    cuatp=get_nm_id(default_odm->vgname,FALSE,VGSERIAL_ID);
                    cuatp=getattr(cuatp->name,AUTO_ON,FALSE,&temp);
                    if ((int)cuatp == ODMERROR) odm_error(odmerrno);
                    if (cuatp != NULL)
                     {
                      sprintf(default_odm->auto_on,"%s",cuatp->value);
                     }
                    else
                     {
                      fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,AUTO_ON);
                      odm_terminate();
                      return(FAIL);
                     }
         }      
 odm_terminate();
 return(SUCCESS);
}



/*
 * NAME: get_all_vgnames
 *
 * FUNCTION: Return the list of all volume groups stored in ODM.
 *
 * RETURN CODE: 0 == SUCCESS
 *              non-zero == FAILURE
 */

RETURN_CODE get_all_vgnames(vgnames, vg_number)
char ***vgnames;  
int *vg_number;          
{
 struct CuDv *cudvp;
 struct listinfo stat_info;
 char crit[CRITSIZE];
 int num=0;

 odm_initialize();
 odm_set_path(CFGPATH);

 sprintf(crit,"PdDvLn = '%s'",VGUNIQUETYPE);
 cudvp=(struct CuDv *)odm_get_list(CuDv_CLASS,crit,&stat_info,MAXNUMVGS,1);
 if ((int)cudvp == ODMERROR) odm_error(odmerrno);
 if (!stat_info.num)
  {
   fprintf(stderr,lvm_msg(NO_VGS_NOW),Prog);
   odm_terminate();
   return(FAIL);
  }
 else
  {
   int vg_allocated;

   *vg_number = vg_allocated = 0;
   num=stat_info.num;
   while (num > 0)
    {
     addvg(cudvp->name, vgnames, vg_number, &vg_allocated);
     cudvp++;
     num--;
    }
  }   
odm_terminate();
return(SUCCESS);
}





/*
 * NAME: get_vg_pvname
 *
 * FUNCTION: Translate the pvname to an ascii vgid.  This is currently a C interface to the shell command getlvodm.
 *
 * RETURN CODE: 0 == SUCCESS
 *              non-zero == FAILURE
 */
RETURN_CODE get_vg_pvname(sourcepv, vgid_ascii)
char *sourcepv,	*vgid_ascii; 
{
 struct CuAt *cuatp,cuat;
 int pvdesc;
 char *pvid;
 char crit[CRITSIZE];
 char vgname[DBNAMESIZE];
 int temp;

 odm_initialize();
 odm_set_path(CFGPATH);

 pvdesc=check_desc(sourcepv);
 
 switch(pvdesc)
  {
   case NAME:
              sprintf(crit,"name = '%s' and attribute = '%s'",sourcepv,"pvid");
			  if ((int)odm_get_first(CuAt_CLASS,crit,&cuat) > 0)
				  pvid=cuat.value;
			  else {pvid=""; odm_error(odmerrno);}
              break;
   default:
              pvid=sourcepv;
              break;
  } 
  sprintf(crit,"attribute = '%s' and value = '%s'","pv",pvid);
  if ((int)odm_get_first(CuAt_CLASS,crit,&cuat) <= 0)
  {
   fprintf(stderr,lvm_msg(PV_NOT_IN_VG),Prog,sourcepv);
   odm_terminate();
   return(FAIL);
  }
  else
  {
   bzero(vgname,DBNAMESIZE);
   strncpy(vgname,cuat.name,DEVNAMESIZE);
   cuatp=getattr(vgname,VGSERIAL_ID,FALSE,&temp);
   if ((int)cuatp == ODMERROR) odm_error(odmerrno);
   if (cuatp != NULL)
   {
    sprintf(vgid_ascii,"%16s",cuatp->value);
   }
   else
   {
    fprintf(stderr,lvm_msg(DEV_NOT_FOUND),Prog,lvm_msg(VGINTRO),sourcepv);
   }
  }
return(SUCCESS);
}


/*
 * NAME: get_vg_id
 *
 * FUNCTION: Translate the pvdescript to an ascii vgid.  
 *
 * RETURN CODE: 0 == SUCCESS
 *              non-zero == FAILURE
 */
RETURN_CODE get_vg_id(vgdescript, vgid_ascii)
char *vgdescript, *vgid_ascii;
{
 struct CuAt *cuatp;
 int vg_desc;
 int temp;

 odm_initialize();
 odm_set_path(CFGPATH);
 
 vg_desc=check_desc(vgdescript);

 switch(vg_desc)
  {
   case NAME: 
             cuatp=getattr(vgdescript,VGSERIAL_ID,FALSE,&temp);
             if (cuatp != NULL)
             {
              sprintf(vgid_ascii,"%16s",cuatp->value);
             }
             else
             {
              fprintf(stderr,lvm_msg(DEV_NOT_FOUND),Prog,lvm_msg(VGINTRO),vgdescript);
              odm_terminate();
              return(FAIL);
             }
             break;
   default:
             sprintf(vgid_ascii,"%s",vgdescript);
             break;
  }
odm_terminate();
return(SUCCESS);
}

/*
 * NAME: check_desc
 *
 * FUNCTION: Checks the descriptor to decide if it is a name or identifier. 
 *	If there is a decimal then it will be an identifier.  And if
 *	there is a non-hex character then it will be a name.
 */

check_desc(dvdesc)
char *dvdesc;	/* returns the descriptor type (OUTPUT) */
{
char *tempptr;
int id;

   /* default value for the dvdesc_type */
   id = TRUE;
   tempptr = dvdesc;
   /* parse the descriptor until the end and */
   /* test if any non-hex characters are found */
   while ((*tempptr != '\0') && (*tempptr != '.'))
    {
     if (!isxdigit((int)*tempptr)) id = 0;	/* then not id */
     tempptr++;
    }
    /* if id is still possible test the length */
    if (id)
     {
      if ((strlen(dvdesc) == VGIDLENGTH) || (*tempptr == '.')
                || (strlen(dvdesc) == CUPVPVID))
       {
        return(IDENT);     
       }
      else
       {
        if (strlen(dvdesc) <= DEVNAMESIZE)
         {
          return(NAME);
         }
        else
         {
          fprintf(stderr,lvm_msg(BAD_VGID),Prog);
         }
       }
      }
     else
      {
         return(NAME);
      }
}
