static char sccsid[] = "@(#)23	1.14.3.7  src/bos/usr/sbin/lvm/highcmd/lspv.c, cmdlvm, bos411, 9436C411a 9/8/94 09:03:21";
/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: main, lfunction, pfunction, defaultf, new_lvid, get_data, lv_distribution,
 *            get_default_odm_data
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
 * FILENAME: lspv.c
 *
 * FILE DESCRIPTION: Contains main functions of lspv (LV's physical volume list command).
 *
 *   Remaining lspv files (and C functions):
 *              parselspv.c   (getflg, parselslv)
 *              lsutil.c      (get_lv_data, distribution, efficiency_total, efficiency, number_of_megabytes,
 *                             get_vgname, get_pvdescript, get_odm_data_chme, get_vgid, get_pvid, get_lvid,
 *                             get_odm_data_emy)
 *              lvmclib.a     (general LV utilities: setflg, usage_error, getmem, get_zeroed_mem,
 *                                                   getregions, parsepair)
 *
 *   This file's C Functions:  (Functions are indented to show the calling hierarchy.)
 *                             ("*" indicates functions called from multiple places (listed more than once))
 *
 *   main
 *        parselvpv (parselspv.c)
 *        lfunction
 *                 get_pvid (lsutil.c)
 *                 get_data
 *                 lv_distribution
 *                 get_odm_data_emy (lsutil.c)
 *        pfunction
 *                 get_pvid (lsutil.c)
 *                 getregions (util3.c)
 *                 get_odm_data_emy (lsutil.c)
 *        no_pv_default
 *        defaultf
 *                 get_pvid (lsutil.c)
 *                 get_vgid )lsutil.c)
 *                 new_lvid
 *                 distributions (lsutil.c)
 *                 get_default_odm_data (lsutil.c)
 *
 *      *****  SEE USER DOCUMENTATION FOR A FUNCTIONAL OVERVIEW OF LSLV  *****
 */

#include <stdio.h>
#include "lvm.h"
#include "cmdlvm.h"
#include "ls.h"
#include <string.h>
#include <sys/cfgodm.h>

/* include file for message texts */
#include "cmdlvm_msg.h"
#include <locale.h>

extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog="lspv";    /* program name */
extern void  prt_pvmap();
extern struct CuAt *getattr();

#define FIRST_TIME_THROUGH_L_LOOP 0
#define FIRST_TIME_THROUGH_P_LOOP 1
#define L_PRINT(f1, f2, f3, f4, f5) printf("%-22s%-6s%-6s%-22s%s\n", f1, f2, f3, f4, f5)
#define P_PRINT_LINE(low, high, line) printf("%3d-%-6d%s\n", low, high, line)
#define P_PRINT_HEADER(h1, h2, h3, h4, h5, h6) printf("%-10s%-8s%-14s%-20s%-11s%s\n", h1, h2, h3, h4, h5, h6)
#define P_STRING_PRINT(line, s1, s2, s3, s4) sprintf(line,"%-8s%-14s%-20s%-11s", s1, s2, s3, s4)
#define DEFAULT_PRINT(p1, p2, p3, p4) printf("%-19s %-24s %-17s %s\n", p1, p2, p3, p4) /* default function output */
#define ODMERROR -1   /* Added for Defect  3171 */
#define PVATTR "pvid" /* Added for Defect 3171 */


struct lv_info  /* Used to store LVM data in a more convenient form for manipulation and calculation.  Each struct element
		   stores information for the PPs/LPs contained on ONE PV in the LV. */
{
	struct lv_id lv_id;               /* lvid */
	int *pp_list;                     /* list for this pvid the PPs that belong to the LV */
	int num_pps;                      /* number of PPs in pp_list */
	int num_lps;                      /* number of LPs in lp_list */
};

struct lv_odm_data {  /* used to store lv data retrieved from ODM for default option */
	char pvname[STRSIZE];             /* PV name */
	char vgname[STRSIZE];             /* VG name the PV is contained in */
};


/*
 * NAME: main
 *
 * FUNCTION: Main function for lspv.  Parse the command line and call the appropriate function for the choosen option.
 *
 * INPUT: Command line options
 *              lspv [-M | -l | -p] [-n pvname] [-v vgid] [pvdescript]
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
	BOOLEAN
		loption,  /* boolean flag set to true if user selected "l" option from command line */
		poption,  /* boolean flag set to true if user selected "p" option from command line */
		noption,  /* boolean flag set to true if user selected "n" option from command line */
		voption,  /* boolean flag set to true if user selected "v" option from command line */
		Moption;  /* boolean flag set to true if user selected "M" option from command line */
	char
		vgid[STRSIZE],          /* contains vgid if voption is true */
		sourcepv[STRSIZE],      /* contains sourcepv if noption is true */
		pvdescript[STRSIZE];    /* contains command line argument if specified */

	void parselspv(), lfunction(), pfunction(), no_pv_default(), defaultf();


	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

	parselspv(argc, argv, &loption, &poption, &Moption, &voption, vgid, &noption, sourcepv, pvdescript);

	if (loption)  /* if sourcepv is not specified, then NULL pointer is expected by lower level LVM subroutine */
		lfunction(pvdescript, voption ? vgid : (char *) NULL, noption ? sourcepv : (char *) NULL);
	else if (poption)  /* also use NULL to represent no lvdescript was specified */
		pfunction(pvdescript, voption ? vgid : (char *) NULL, noption ? sourcepv : (char *) NULL);
	else if (Moption) { /* also use NULL to represent no lvdescript was specified */
		if (!voption)
			get_vgid(pvdescript, vgid);
		
		if (lock_a_vg(Prog, vgid))
			return (FAILURE);
		
		prt_pvmap(pvdescript, voption ? vgid : (char *) NULL, noption ? sourcepv : (char *) NULL);
		}
	else if (strlen(pvdescript) == 0)
		no_pv_default();
	else
		defaultf(pvdescript, voption ? vgid : (char *) NULL, noption ? sourcepv : (char *) NULL);
	return (SUCCESS);
}





/*
 * NAME: lfunction
 *
 * FUNCTION: Implements the "l" option of the lspv command which prints out LV data by logical volume.
 *
 * OUTPUT: Data selected by the "l" option is written to standard out -- one data output line for each LV in the PV.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 */
void lfunction(pvdescript, vgid_option, sourcepv)
char
	*pvdescript,    /* descriptor of the physical volume to list (INPUT) */
	*vgid_option,   /* volume group descriptor - required to query pv info in LVM */
	*sourcepv;      /* optional (maybe NULL) pvname for LVM - indicates a particular PV to read info from (INPUT) */
{
	int
	    i,          /* array counter and loop index */
	    num_lvs,    /* number of logical volumes on the PV */
	    *distrib;   /* distribution of a LVs partitions across the regions of the PV */
	char
	    lvname[STRSIZE],             /* logical volume name */
	    dummy[STRSIZE],              /* unused parameter to get_odm_data_emy */
	    lvmount[STRSIZE],            /* mount point of the logical volume (if any) */
	    distrib_ascii[STRSIZE],      /* distrib values converted to ascii format */
	    num_lps_ascii[STRSIZE],      /* number of LPs in the logical volume in ascii format */
	    num_pps_ascii[STRSIZE],      /* number of PPs in the logical volume in ascii format */
	    pvid_ascii[STRSIZE],         /* string representing pvid */
	    vgid_ascii[STRSIZE];         /* string representing vgid */
	RETURN_CODE
		odm_rc,                  /* return code for get_odm_data_emy */
		query_rc;                /* return code for LVM query */
	PVDESCRIPT_TYPE
		pvdescript_type;         /* type of the pvdescript being used - id or name */
	struct lv_info
		*lv_data[LVM_MAXLPS];    /* data structure for each logical volume */
	struct unique_id
		pvid,                    /* binary pvid */
		vgid;                    /* binary vgid */
	struct querypv
	    *querypv;                    /* required structure for querying data from LVM */

	void get_data();
	RETURN_CODE get_odm_data_emy(), get_pvid();
	int *lv_distribution();

	if (get_pvid (pvdescript, pvid_ascii, &pvdescript_type) == SUCCESS) {  /* need pvid to query pv */
		get_uniq_id (pvid_ascii, &pvid);  /* parse pvid into binary format */
		if (vgid_option != NULL)  /* did the user enter vgid on the command line */
			strcpy (vgid_ascii, vgid_option);
		else if (get_vgid(pvid_ascii, vgid_ascii) != SUCCESS  &&  sourcepv == NULL) {  /* get vgid from ODM */
			exit (1);
		}
		get_uniq_id(vgid_ascii, &vgid);  /* convert ascii vgid into binary */
		/*
		 * lock the vg
		 */
		if (lock_a_vg(Prog, vgid_ascii))
			return;

		query_rc = (RETURN_CODE) lvm_querypv (&vgid, &pvid, &querypv, sourcepv);  /* retrieve pv data from LVM */

		if (query_rc == SUCCESS) {  /* if pv data successfully retrieved */
			get_data(querypv->pp_map, querypv->pp_count, &num_lvs, lv_data);  /* reorganize PV data by LVs */
			for (i=0; i<num_lvs; i++) {  /* for each LV using the PV */

				/* calculate the distribution of the LVs partitions on the PV - and translate to ascii */
				distrib = lv_distribution(lv_data[i]->pp_list, lv_data[i]->num_pps, querypv->pp_count);
				sprintf(distrib_ascii, "%.2d..%.2d..%.2d..%.2d..%.2d",
							distrib[0], distrib[1], distrib[2], distrib[3], distrib[4]);

				/* get the LV data from odm */
				odm_rc = get_odm_data_emy (&lv_data[i]->lv_id, lvname, dummy, lvmount);

				/* print headings first time through loop - it is done here to prevent error messages
				   from coming inbetween the headings and the actual data */
				if (i == FIRST_TIME_THROUGH_L_LOOP) {
					if (pvdescript_type == name)
						printf("%s:\n", pvdescript);
					else  /* was unable to get the pvname so use pvid instead */
						printf("pvid=%s:\n", pvid_ascii);
					if (odm_rc == SUCCESS)
						L_PRINT ("LV NAME", "LPs", "PPs", "DISTRIBUTION", "MOUNT POINT");
					else
						L_PRINT ("LV ID", "LPs", "PPs", "DISTRIBUTION", "MOUNT POINT");
				}
				sprintf(num_lps_ascii, "%d", lv_data[i]->num_lps);  /* translate to ascii */
				sprintf(num_pps_ascii, "%d", lv_data[i]->num_pps);

				/* write the data for one LV to standard out */
				L_PRINT (lvname, num_lps_ascii, num_pps_ascii, distrib_ascii, lvmount);
			}
			lvm_freebuf(querypv);
		}
		else {  /* cannot query the pv data so print error and leave function */
		       errmsg (query_rc);   /* print low-level lvm error message */
		}
	}
}




/*
 * NAME: pfunction
 *
 * FUNCTION: Implements the "p" option of the lspv command which prints out detailed data for a single PV.
 *
 * OUTPUT: Data selected by the "p" option is written to standard out -- the status of each PP on the PV is shown.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 *
 */
void pfunction(pvdescript, vgid_option, sourcepv)
char
	*pvdescript,    /* descriptor of the physical volume to list (INPUT) */
	*vgid_option,   /* vgid in ascii format */
	*sourcepv;      /* optional (maybe NULL) pvdescript for LVM - indicates a particular PV to read info from (INPUT) */
{
	static char *regions_str[] = { "outer edge", "outer middle", "center", "inner middle", "inner edge" };  /* ascii description of regions */
	int
	    i, j,                 /* array counters and loops */
	    range_low,            /* lower bound on ranges of PPs that are to be associated with a single output line */
	    range_high;           /* upper bound on ranges of PPs that are to be associated with a single output line */
	char
	    line[110],            /* ascii data for one line of output */
	    previous_line[110],   /* previous ascii data for one line of output */
	    *pp_state_str,        /* ascii string representing the state of one PP */
	    lvname[STRSIZE],      /* LV name */
	    lvtype[STRSIZE],      /* type of the LV */
	    lvmount[STRSIZE],     /* mount point for the LV (if any) */
	    pvid_ascii[STRSIZE],  /* string representing pvid */
	    vgid_ascii[STRSIZE];  /* string representing vgid */
	RETURN_CODE
		odm_rc,           /* return code for get_odm_data_emy */
		query_rc;         /* return code for LVM query */
	PVDESCRIPT_TYPE
		pvdescript_type;  /* type of the pvdescript being used - id or name */
	struct unique_id
		pvid,             /* pvid */
		vgid;             /* vgid */
	struct querypv
	    *querypv;             /* required structure for querying data from LVM */
	struct REGION
		*regions;         /* the boundaries of the intra regions */

	struct REGION *getregions ();
	RETURN_CODE get_odm_data_emy(), get_pvid();

	if (get_pvid (pvdescript, pvid_ascii, &pvdescript_type) == SUCCESS) {  /* need pvid to query pv */
		get_uniq_id (pvid_ascii, &pvid);  /* parse pvid into binary format */
		if (vgid_option != NULL)  /* if vgid entered on the command line */
			strcpy (vgid_ascii, vgid_option);
		else if (get_vgid(pvid_ascii, vgid_ascii) != SUCCESS  &&  sourcepv == NULL) {  /* get vgid from ODM */
			exit (1);
		}

		get_uniq_id(vgid_ascii, &vgid);  /* convert the ascii vgid into binary format */

		/*
		 * lock the vg
		 */
		if (lock_a_vg(Prog, vgid_ascii))
			return;

		query_rc = (RETURN_CODE) lvm_querypv (&vgid, &pvid, &querypv, sourcepv);  /* retrieve pv data from LVM */

		if (query_rc == SUCCESS) {  /* if pv data successfully retrieved */
			regions = getregions(querypv->pp_count);  /* get the boundaries of the PV's 5 regions */
			for (i=0; i<SREGION; i++)  /* for each of the regions of the PV */
				for (j=regions[i].low; j<=regions[i].high; j++) {  /* for each PP in the region */

					switch (querypv->pp_map[j-1].pp_state) {  /* convert PP state to ascii format */
						case LVM_PPFREE     :  pp_state_str = "free"; break;
						case LVM_PPALLOC    :  pp_state_str = "used"; break;
						case CMDLVM_PPSTALE :  pp_state_str = "stale"; break;
						default             :  pp_state_str = UNDEFINED_STR; break;
					}

					if (querypv->pp_map[j-1].pp_state != LVM_PPFREE)  /* get data on the PP's LV */
						odm_rc = get_odm_data_emy (&querypv->pp_map[j-1].lv_id,
						    lvname, lvtype, lvmount);
					else
						lvname[0] = lvtype[0] = lvmount[0] = '\0';  /* PP is free so no LV */

					/* if first time through the loop, print headings.  This is done here to prevent
					   error messages from be displayed between the headings and the actual data */
					if (j == FIRST_TIME_THROUGH_P_LOOP) {
						if (pvdescript_type == name)
							printf("%s:\n", pvdescript);
						else  /* was unable to get the pvname so use pvid instead */
							printf("pvid=%s:\n", pvid_ascii);
						if (odm_rc == SUCCESS)  /* if lvname is known */
							P_PRINT_HEADER ("PP RANGE", "STATE", "REGION",
									"LV NAME", "TYPE", "MOUNT POINT");
						else
							P_PRINT_HEADER ("PP RANGE", "STATE", "REGION",
									"LV ID", "TYPE", "MOUNT POINT");
					}

					/* store one line of output in a string */
					P_STRING_PRINT (line, pp_state_str, regions_str[i], lvname, lvtype);
					strncat(line, lvmount, STRSIZE);

					/* if the data for the current line is the same as the previous line */
					if (strcmp(line, previous_line) == IDENTICAL  &&  j != FIRST_TIME_THROUGH_P_LOOP)
						range_high = j;
					else {  /* else the last line is unique so print it, if there is a previous line */
						if (j != FIRST_TIME_THROUGH_P_LOOP)
							P_PRINT_LINE (range_low, range_high, previous_line);
						range_low = range_high = j;
						strcpy(previous_line, line);
					}
				}
			P_PRINT_LINE (range_low, range_high, previous_line);  /* write to stdout one line of data */
			lvm_freebuf(querypv);
		}
		else {  /* cannot query the pv data so print error and leave function */
		       errmsg (query_rc);   /* print low-level lvm error message */
		     }
	}
}





/*
 * NAME: no_pv_default
 *
 * FUNCTION: Display the default data for the lspv command when no PVs are specified.  The PVs will then be listed.
 *
 * OUTPUT: Data selected by default is written to standard out.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 *
 */
void no_pv_default()
{
 struct CuDv  *cudvp;
 struct CuAt  *cuatp, cuat;
 struct listinfo stat_info;
 char crit[CRITSIZE];
 char vgname[DBNAMESIZE];
 int index;
 int temp;
 int num_of_disks;

 odm_initialize();
 odm_set_path(CFGPATH);

 /* Get volumes of all hdisk */
 cudvp=(struct CuDv *)odm_get_list(CuDv_CLASS,
 		"PdDvLn like 'disk*' and status = '1'",
		&stat_info,MAXNUMPVS,1);

 if (((int)cudvp == ODMERROR) || (stat_info.num == 0))
  {
   odm_terminate();
   exit(FAILURE);
  }
 num_of_disks=stat_info.num;
 for (index=0; index < num_of_disks; index++)
  {
   /* Get PVID */
   cuatp=getattr((cudvp+index)->name,PVATTR,FALSE,&temp);
   if (temp != 0)
    {
     /* Get Volume group name */
     sprintf(crit,"attribute = '%s' and value = '%s'","pv",cuatp->value);
     if ((int)odm_get_first(CuAt_CLASS,crit,&cuat) > 0)
        strncpy(vgname,cuat.name,(int)DEVNAMESIZE);
     else
        strcpy(vgname,"None");
     printf("%-15s%-16.16s    %-15s\n",(cudvp+index)->name,cuatp->value,vgname);
    }
  } 

 /* Get volumes of all read/write optical */
 cudvp=(struct CuDv *)odm_get_list(CuDv_CLASS,
 		"PdDvLn like 'rwoptical*' and status = '1'",
		&stat_info,MAXNUMPVS,1);

 if ((int)cudvp == ODMERROR)
  {
   odm_terminate();
   exit(FAILURE);
  }
 num_of_disks=stat_info.num;
 for (index=0; index < num_of_disks; index++)
  {
   /* Get PVID */
   cuatp=getattr((cudvp+index)->name,PVATTR,FALSE,&temp);
   if (temp != 0)
    {
     /* Get Volume group name */
     sprintf(crit,"attribute = '%s' and value = '%s'","pv",cuatp->value);
     if ((int)odm_get_first(CuAt_CLASS,crit,&cuat) > 0)
        strncpy(vgname,cuat.name,(int)DEVNAMESIZE);
     else
        strcpy(vgname,"None");
     printf("%-15s%-16.16s    %-15s\n",(cudvp+index)->name,cuatp->value,vgname);
    }
  } 
odm_free_list(cudvp, &stat_info);
odm_terminate();
}



/*
 * NAME: defaultf
 *
 * FUNCTION: Display the default data for the lspv command.  All the characteristics of the PV are displayed.
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
void defaultf(pvdescript, vgid_option, sourcepv)
char
	*pvdescript,    /* descriptor of the logical volume to list (INPUT) */
	*vgid_option,   /* vgid in ascii format */
	*sourcepv;      /* optional (maybe NULL) pvdescript for LVM to indicate a particular PV to read info from (INPUT) */
{
	int
	    i,                           /* counter and array index */
	    ppsize,                      /* the ppsize in megabytes */
	    lv_count=0,                  /* count of the number of LVs using the PV */
	    freepps=0,                   /* number of free PPs on the PV */
	    usedpps=0,                   /* number of used PPs on the PV */
	    stalepps=0,                  /* number of stale PPs on the PV */
	    free_dist[SREGION],          /* the distribute of free PPs on the PV, according to region */
	    used_dist[SREGION];          /* the distribute of used PPs on the PV, according to region */

	char
	    pvid_ascii[STRSIZE],         /* string representing pvid */
	    vgid_ascii[STRSIZE],         /* string representing vgid */
	    pvstate_ascii[STRSIZE],      /* string representing pv state */
	    stalepps_ascii[STRSIZE],     /* string representing number of stale PPs */
	    allocatable_ascii[STRSIZE],  /* string representing if the pv can be allocated on */
	    ppsize_ascii[STRSIZE],       /* string representing PP size */
	    lv_count_ascii[STRSIZE],     /* string representing number of LVs */
	    totalpps_ascii[STRSIZE],     /* string representing total number of PPs on PV */
	    vgdas_ascii[STRSIZE],        /* string representing number of VG descriptor areas on the PV */
	    freepps_ascii[STRSIZE],      /* string representing number of free PPs on PV */
	    usedpps_ascii[STRSIZE],      /* string representing number of used PPs on PV */
	    free_dist_ascii[STRSIZE],    /* string representing distribution of free PPs on PV */
	    used_dist_ascii[STRSIZE];    /* string representing distribution of used PPs on PV */
	RETURN_CODE
		odm_rc,                  /* return code from ODM query */
		query_rc;                /* return code for LVM query */
	PVDESCRIPT_TYPE
		pvdescript_type;         /* type of the pvdescript being used - id or name */
	struct unique_id
		pvid,                    /* pvid */
		vgid;                    /* vgid */
	struct querypv
	    *querypv;                    /* required structure for querying data from LVM */
	struct lv_odm_data
	    default_odm;                 /* the ODM data for the LV -- see structure definition */

	void distributions();
	RETURN_CODE get_pvid();


	if (get_pvid(pvdescript, pvid_ascii, &pvdescript_type) != SUCCESS) {  /* need lvid to query LVM */
		exit(1);
	}

	get_uniq_id(pvid_ascii, &pvid); /* translate ascii id into binary */

	if (vgid_option != NULL)  /* if vgid entered on the commmand line */
		strcpy (vgid_ascii, vgid_option);
	else if (get_vgid(pvid_ascii, vgid_ascii) != SUCCESS  &&  sourcepv == NULL) {  /* get vgid from ODM */
		exit (1);
	}
	get_uniq_id(vgid_ascii, &vgid);  /* convert the vgid from ascii format to binary */

	/*
	 * lock the vg
	 */
	if (lock_a_vg(Prog, vgid_ascii))
		return;

	query_rc = (RETURN_CODE) lvm_querypv (&vgid, &pvid, &querypv, sourcepv);  /* retrieve PV data stored by LVM */
	if (query_rc == SUCCESS) {  /* if able to get the PV map then process it */
		sprintf(vgid_ascii, "%.8x%.8x", vgid.word1, vgid.word2);  /* convert vgid to ascii - may never had ascii */
		for (i=0; i<querypv->pp_count; i++) {  /* for each PP in the PV map */
			switch (querypv->pp_map[i].pp_state) {  /* count the number of free, used, and stale PPs */
				case LVM_PPFREE     : freepps++; break;
				case CMDLVM_PPSTALE : stalepps++;
				case LVM_PPALLOC    : usedpps++; break;
				default             : break;
			}
			/* if the PP is allocated to a LV that has not already been counted, then count the LV */
			if (querypv->pp_map[i].pp_state != LVM_PPFREE  &&  new_lvid (&querypv->pp_map[i].lv_id))
				lv_count++;
		}

		if (querypv->pv_state & LVM_PVACTIVE)  strcpy(pvstate_ascii, "active");  /* varyon state of the PV */
		else if (querypv->pv_state & LVM_PVMISSING)  strcpy(pvstate_ascii, "missing");
		else if (querypv->pv_state & LVM_PVREMOVED)  strcpy(pvstate_ascii, "removed");
		else strcpy(pvstate_ascii, UNDEFINED_STR);

		if (querypv->pv_state & LVM_PVNOALLOC) strcpy(allocatable_ascii, "no");  /* is PV allocatable */
		else strcpy(allocatable_ascii, "yes");

		/* convert integer values to ascii, also convert some of the sizes in PPs to megabytes */
		ppsize = number_of_megabytes(querypv->ppsize);
		sprintf(freepps_ascii, "%d (%d megabytes)", freepps, freepps * ppsize);
		sprintf(usedpps_ascii, "%d (%d megabytes)", usedpps, usedpps * ppsize);
		sprintf(totalpps_ascii, "%d (%d megabytes)", querypv->pp_count, querypv->pp_count * ppsize);
		sprintf(ppsize_ascii, "%d megabyte(s)", ppsize);
		sprintf(vgdas_ascii, "%d", querypv->pvnum_vgdas);
		sprintf(stalepps_ascii, "%d", stalepps);
		sprintf(lv_count_ascii, "%d", lv_count);

		/* convert the to distribution arrays into ascii distribution format */
		distributions(querypv->pp_map, querypv->pp_count, free_dist, used_dist);
		sprintf(free_dist_ascii, "%.2d..%.2d..%.2d..%.2d..%.2d",
			free_dist[0], free_dist[1], free_dist[2], free_dist[3], free_dist[4]);
		sprintf(used_dist_ascii, "%.2d..%.2d..%.2d..%.2d..%.2d",
			used_dist[0], used_dist[1], used_dist[2], used_dist[3], used_dist[4]);

		lvm_freebuf(querypv);
	}
	else {  /* unable to get PV map so print error and fill in default values for the associated fields */
		errmsg (query_rc);   /* print low-level lvm error message */
		strcpy (pvstate_ascii, UNDEFINED_STR);
		strcpy (allocatable_ascii, UNDEFINED_STR);
		strcpy (freepps_ascii, UNDEFINED_STR);
		strcpy (usedpps_ascii, UNDEFINED_STR);
		strcpy (totalpps_ascii, UNDEFINED_STR);
		strcpy (stalepps_ascii, UNDEFINED_STR);
		strcpy (lv_count_ascii, UNDEFINED_STR);
		strcpy (free_dist_ascii, UNDEFINED_STR);
		strcpy (used_dist_ascii, UNDEFINED_STR);
		strcpy (ppsize_ascii, UNDEFINED_STR);
		strcpy (vgdas_ascii, UNDEFINED_STR);
	}

	odm_rc = get_default_odm_data (pvid_ascii, &default_odm);  /* retrieve the PV data stored by ODM */
	if (odm_rc != SUCCESS) {  /* if cannot get the ODM data */
		strcpy (default_odm.pvname, UNDEFINED_STR);
		strcpy (default_odm.vgname, UNDEFINED_STR);
	}

	/* display in columns the logical volume data */
	DEFAULT_PRINT("PHYSICAL VOLUME:", default_odm.pvname, "VOLUME GROUP:", default_odm.vgname);
	DEFAULT_PRINT("PV IDENTIFIER:", pvid_ascii, "VG IDENTIFIER", vgid_ascii);
	DEFAULT_PRINT("PV STATE:", pvstate_ascii,"","");
	DEFAULT_PRINT("STALE PARTITIONS:", stalepps_ascii, "ALLOCATABLE:", allocatable_ascii);
	DEFAULT_PRINT("PP SIZE:", ppsize_ascii, "LOGICAL VOLUMES:", lv_count_ascii);
	DEFAULT_PRINT("TOTAL PPs:", totalpps_ascii, "VG DESCRIPTORS:", vgdas_ascii);
	DEFAULT_PRINT("FREE PPs:", freepps_ascii, "", "");
	DEFAULT_PRINT("USED PPs:", usedpps_ascii, "", "");
	DEFAULT_PRINT("FREE DISTRIBUTION:", free_dist_ascii, "", "");
	DEFAULT_PRINT("USED DISTRIBUTION:", used_dist_ascii, "", "");
}




/*
 * NAME: new_lvid
 *
 * FUNCTION:
 *
 *
 *
 * RETURN CODE: FALSE if the lvid is not new
 *              TRUE if the lvid is not
 */
BOOLEAN new_lvid (lv_id)
struct lv_id
	*lv_id;
{
	int i;
	BOOLEAN new_id = TRUE;
	static struct lv_id
		ids[LVM_MAXLVS];
	static int id_count;


	for (i=0; i<id_count; i++)
		if (SAMELVID(*lv_id, ids[i])) {
			new_id = FALSE;
			break;
		}

	if (new_id) {
		ids[id_count].vg_id.word1 = lv_id->vg_id.word1;
		ids[id_count].vg_id.word2 = lv_id->vg_id.word2;
		ids[id_count++].minor_num = lv_id->minor_num;
	}

	return(new_id);
}




/*
 * NAME: get_data
 *
 * FUNCTION: Convert the querypv map (*copies) into a more convenient form which is organized by LV.  Also gather
 *           or calculate related PV information.  This data structure is defined so that it is easily used in
 *           different circumstances.
 *
 * RETURN CODE: none
 *
 */
void get_data (pp_map, pvsize, num_lvs, lv_data)
struct pp_map
	pp_map[];    /* PV map (INPUT) */
int
	pvsize;      /* size of PV and number of PV map entries (INPUT) */
int
	*num_lvs;    /* number of LVs using the PV (INPUT) */
struct lv_info
	*lv_data[];  /* the PV map organized by LV (OUTPUT) */
{
	int
		i, j, k;  /* counters and array indexes */
	int
		*lp_num[LVM_MAXLPS];  /* array of pointers to list of LPs - by LVs, used to count LPs per LV */

	for (*num_lvs=i=0; i<pvsize; i++) {  /* for each PP on the PV */
		if (pp_map[i].pp_state != LVM_PPFREE) {  /* if PP is allocated to a LV */

			/* scan the current list of lvid's to see if this id is already in the list */
			for (j=0; j<*num_lvs && !SAMELVID(lv_data[j]->lv_id, pp_map[i].lv_id); j++);

			if (j == *num_lvs) {  /* if the PV is not already in the data structure, then put it in */
				lv_data[j] = (struct lv_info *) get_zeroed_mem(1, sizeof(struct lv_info)); /* alloc space */
				lv_data[j]->lv_id.vg_id.word1 = pp_map[i].lv_id.vg_id.word1; /* save lvid */
				lv_data[j]->lv_id.vg_id.word2 = pp_map[i].lv_id.vg_id.word2;
				lv_data[j]->lv_id.minor_num = pp_map[i].lv_id.minor_num;
				lv_data[j]->pp_list = (int *) getmem((pvsize-i) * sizeof(int));  /* alloc space for PPs */
				lp_num[j] = (int *) getmem((pvsize-i) * sizeof(int));  /* alloc space for LPs */
				lp_num[j][lv_data[j]->num_lps++] = pp_map[i].lp_num;  /* save current LP number */
				(*num_lvs)++;
			}  /* after the if -- we know j points to the current LV */

			lv_data[j]->pp_list[lv_data[j]->num_pps++] = i + 1;  /* save PP number */
			for (k=0; k<lv_data[j]->num_lps; k++)  /* for all the LV's LP number so far encountered */
				if (lp_num[j][k] == pp_map[i].lp_num)  /* does the current LP match one in the list */
					break;  /* if match was found then exist */
			if (k == lv_data[j]->num_lps)  /* if the LP not found in the list, add it - this keeps LP count */
				lp_num[j][lv_data[j]->num_lps++] = pp_map[i].lp_num;
		}
	}
}




/*
 * NAME: lv_distribution
 *
 * FUNCTION: lv_distribution
 *
 * RETURN VALUE: pointer to array of 5 values -- the distribution of PPs in the 5 intra regions
 */
int *lv_distribution (pp_list, numpps, pvsize)
int
	pp_list[],  /* list of PPs in LV that belong to the PV (INPUT) */
	numpps,     /* number of PPs in the list, size of pp_list (INPUT) */
	pvsize;     /* size of the PV the PPs belong to (INPUT) */
{
	int
		i,                /* loop counter and array index */
		*distrib_value;   /* the five distribution values */
	struct REGION
		*regions;         /* the boundaries of the intra regions */

	struct REGION *getregions ();


	regions = getregions(pvsize);  /* calculate the intra region boundaries */
	distrib_value = (int *) get_zeroed_mem (SREGION, sizeof(int));  /* allocate space that will be returned */
	for (i=0; i<numpps; i++)  /* for each PP in the list */
		if (pp_list[i] <= regions[0].high)       /* first region */
			distrib_value[0]++;
		else if (pp_list[i] <= regions[1].high)  /* second region */
			distrib_value[1]++;
		else if (pp_list[i] <= regions[2].high)  /* third region */
			distrib_value[2]++;
		else if (pp_list[i] <= regions[3].high)  /* fourth region */
			distrib_value[3]++;
		else if (pp_list[i] <= regions[4].high)  /* fifth region */
			distrib_value[4]++;
	return (distrib_value);  /* return the distribution counters */
}


/*
 * NAME: get_default_odm_data
 *
 * FUNCTION: Return the ODM data for a particular PV.
 *
 * RETURN CODE: 0 == SUCCESS
 *              non-zero == FAILURE
 */
RETURN_CODE get_default_odm_data (pvid, default_odm)
char *pvid;   /* lvid of the LV the data is returned for (INPUT) */
struct lv_odm_data *default_odm;
{
 struct CuAt *cuatp, cuat;
 char crit[CRITSIZE];
 char pvname[DBNAMESIZE];
 char pvid_32[CUPVPVID];
 int temp;

 if (pvid == NULL) return(FAILURE);

 odm_initialize();
 odm_set_path(CFGPATH);

 get_pvname(pvid,pvname);
 sprintf(default_odm->pvname,"%s",pvname);
 convt_pvid_32(pvid,pvid_32);
 sprintf(crit,"attribute = '%s' and value = '%s'","pv",pvid_32);
 if ((int)odm_get_first(CuAt_CLASS,crit,&cuat) > 0) 
  {
   sprintf(default_odm->vgname,"%s",cuat.name);
  }
 else
  {
   fprintf(stderr,lvm_msg(PV_NOT_IN_VG),Prog,pvname);
   odm_terminate();
   return(FAILURE);
  }
   
return(SUCCESS);
}
