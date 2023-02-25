static char sccsid[] = "@(#)22  1.11.1.14  src/bos/usr/sbin/lvm/highcmd/lslv.c, cmdlvm, bos41J, 173960.bos 3/20/95 21:57:42";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: main, lfunction, mfunction, pfunction, defaultf, init_querylv_struct,
 *            init_odm_struct, translate_lvm_data, translate_odm_data, stale_count,
 *            num_copies, in_band_percent, inband, list_distribution, strict_allocation
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
 * FILENAME: lslv.c
 *
 * FILE DESCRIPTION: Contains main functions of lslv (logical volume list command).
 *
 *   Remaining lslv files (and C functions):
 *              parselslv.c   (getflg, parselslv)
 *              lsutil.c      (get_lv_data, distribution, efficiency_total, efficiency, number_of_megabytes,
 *                             get_vgname, get_pvdescript, get_odm_data_chme, get_vgid, get_pvid, get_lvid,
 *                             get_odm_data_emy)
 *              lvmclib.a     (general LV utilities: setflg, getmem, get_zeroed_mem,
 *                                                   getregions, parsepair)
 *
 *   "lslv" C Functions:  (Functions are indented to show the calling hierarchy.)
 *                        ("*" indicates functions called from multiple places (listed more than once))
 *
 *      main
 *              parselslv (parselslv.c)
 *              lfunction
 *                      *get_lvid (lsutil.c)
 *                      *get_lv_data (lsutil.c)
 *                              *get_pvdescript (lsutil.c)
 *                      *get_odm_data_chme (lsutil.c)
 *                      num_copies
 *                      list_distribution
 *                      *efficiency
 *                      *in_band
 *              mfunction
 *                      *get_lvid (lsutil.c)
 *                      *get_odm_data_chme (lsutil.c)
 *                      *get_pvdescript (lsutil.c)
 *              pfunction
 *                      *get_lvid (lsutil.c)
 *                      *get_odm_data_chme (lsutil.c)
 *                      get_pvid (lsutil.c)
 *                      get_vgid (lsutil.c)
 *              defaultf
 *                      *get_lvid (lsutil.c)
 *                      *get_lv_data (lsutil.c)
 *                              *get_pvdescript (lsutil.c)
 *                      *get_odm_data_chme (lsutil.c)
 *                      init_querylv_struct
 *                      init_odm_struct
 *                      translate_lvm_data
 *                      translate_odm_data
 *                      in_band_percent
 *                              *in_band
 *                      efficiency_total (lsutil.c)
 *                              *efficiency (lsutil.c)
 *                      strict_allocation
 *                      stale_count
 *
 *      *****  SEE USER DOCUMENTATION FOR A FUNCTIONAL OVERVIEW OF LSLV  *****
 */

#define _ILS_MACROS	/* performance enhancement for isxdigit(), etc calls */
#include <stdio.h>
#include "lvm.h"
#include "cmdlvm.h"
#include "ls.h"
#include <string.h>
#include <fstab.h>
#include <odmi.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

extern int odmerrno;
extern int odm_errno();
extern int setfsent();
extern int endfsent();
extern struct fstab * getfsspec();
extern struct CuAt *getattr();
static void lock_vg_of_lv(char *);
static void convert_id_to_name(char *);

/* include file for message texts */
#include "cmdlvm_msg.h"
#include <locale.h>
#include <ctype.h>

extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *Prog="lslv";    /* program name */

#define NO_COLUMNS 0  /* singles no new columns have begun to be displayed with the "p" option */
#define P_COLUMNS 10  /* number of pp columns printed for the -p option (pfunction) */
#define LFORMAT_PRINT(p1, p2, p3, p4, p5) printf("%-17s %-13s %-13s %-13s %s\n", p1, p2, p3, p4, p5) /* lfunction output */
#define DEFAULT_PRINT(p1, p2, p3, p4) printf("%-19s %-22s %-15s %s\n", p1, p2, p3, p4)  /* default function output */
#define ODMERROR -1
#define UNDEF_STR  "?"
#define NAME 0
#define IDENT 1
#define FAIL 1
#define CHK_FAIL -1



/*
 * NAME: main
 *
 * FUNCTION: Main function for lslv.  Parse the command line and call the appropriate function for the choosen option.
 *
 * INPUT: Command line options
 *              lslv [-l | -m] [-n pvname] lvdescript
 *              lslv [-n pvname] -p pvdescript [lvdescript]
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
		moption,  /* boolean flag set to true if user selected "m" option from command line */
		poption,  /* boolean flag set to true if user selected "p" option from command line */
		noption;  /* boolean flag set to true if user selected "n" option from command line */
	char
		pvname[STRSIZE],        /* contains pvname if poption is true */
		sourcepv[STRSIZE],      /* contains sourcepv if noption is true */
		lvdescript[STRSIZE],    /* contains command line argument if specified */
		savelvdesc[STRSIZE];    /* place holder for lvdescript */

	void parselslv(), lfunction(), mfunction(), pfunction(), defaultf();

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

	parselslv(argc, argv, &loption, &moption, &poption, pvname, &noption, sourcepv, lvdescript);

	strcpy(savelvdesc, lvdescript);
	convert_id_to_name(lvdescript);

	if ((*lvdescript != NULL) || (*pvname != NULL)) {

		if (*lvdescript)
			lock_vg_of_lv(lvdescript);

		if (loption)  /* if sourcepv is not specified, then NULL pointer is expected by lower level LVM subroutine */
			lfunction(lvdescript, noption ? sourcepv : (char *) NULL);
		else if (moption)
			mfunction(lvdescript, noption ? sourcepv : (char *) NULL);
		else if (poption)  /* also use NULL to represent no lvdescript was specified */
			pfunction(strlen(lvdescript) != 0 ? lvdescript : NULL, pvname, noption ? sourcepv : (char *) NULL);
		else
			defaultf(lvdescript, noption ? sourcepv : (char *) NULL);

	} else if ((*lvdescript == NULL) && (*pvname == NULL)) {
		fprintf(stderr,lvm_msg(DEV_NOT_FOUND),Prog,"",savelvdesc);
		return(FAIL);
	}

	return (SUCCESS);
}





/*
 * NAME: lfunction
 *
 * FUNCTION: Implements the "l" option of the lslv command which prints out LV data by physical volume (PV).
 *
 * OUTPUT: Data selected by the "l" option is written to standard out -- one data output line for each PV used by LV.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 */
void lfunction(lvdescript, sourcepv)
char
	*lvdescript,    /* descriptor of the logical volume to list (INPUT) */
	*sourcepv;      /* optional (maybe NULL) pvdescript for LVM to indicate a particular PV to read info from (INPUT) */
{
	int
		i,                      /* loop counter */
		pv_count,               /* number of PVs in the LV */
		copy1,                  /* number of LPs on a PV with at least one PP assigned to it */
		copy2,                  /* number of LPs on a PV with at least two PPs assigned to it */
		copy3,                  /* number of LPs on a PV with at least three PPs assigned to it */
		*distrib_value;         /* distribution by region of PV's PPs belonging to LV */
	char
		lvid_ascii[STRSIZE],    /* lvid in ascii format */
		copies_str[STRSIZE],    /* copy1:copy2:copy3 in ascii format */
		inband_str[STRSIZE],    /* inband percentage in ascii format */
		effic_str[STRSIZE],     /* efficiency percentage in ascii format */
		distrib_str[STRSIZE];   /* the number of PPs in each region (r1:r2:r3:r4:r5) in ascii format */
	PERCENTAGE
		inband_value,           /* percentage of a PV's PPs belonging to LV that are allocated in intra region */
		efficiency_value;       /* rating of allocation efficiency for a PV's PPs belonging to the LV */
	RETURN_CODE
		odm_rc,                 /* return code from ODM query */
		query_rc;               /* return code for LVM query */
	struct lv_info2
		lv_data[LVM_MAXPVS];    /* the LVM query rearranged by PV -- see structure definition */
	struct lv_id
		lvid;                   /* the lvid in binary format */
	struct lv_odm_data2
		odm;                    /* the ODM data for the lv -- see structure definition */
	struct querylv
		*querylv;               /* required structure for querying data from LVM */

	int *list_distribution();
	void get_lv_data(), num_copies();
	PERCENTAGE inband(), efficiency();
	RETURN_CODE get_lvid();
        char   intra_policy[STRSIZE];
        char   mount_point[STRSIZE];
        char   tempdev[STRSIZE];
        struct fstab * fsstruct;



	if (get_lvid(lvdescript, lvid_ascii) != SUCCESS) {  /* must have lvid to query LVM */
		exit(1);
	}

	parsepair(lvid_ascii, ".", 3, 22, &lvid.vg_id, &lvid.minor_num);  /* translate binary lvid into ascii format */

odm_initialize();
odm_set_path(CFGPATH);
        getlvattr(INTRA,intra_policy,lvdescript);
odm_terminate();
odm_rc = SUCCESS;

/* get the mount point */
sprintf(tempdev,"/dev/%s",lvdescript);
setfsent();
fsstruct = getfsspec(tempdev);
if (fsstruct != NULL) strcpy(mount_point,fsstruct->fs_file);
else strcpy(mount_point,"N/A");
endfsent();

	query_rc = (RETURN_CODE) lvm_querylv (&lvid, &querylv, sourcepv);  /* retrieve all LVM data for lv */
	if (query_rc == SUCCESS) {  /* if data successfully retrieved */

		/* translate the LVM data into a different data structure that is oriented to this command */
		get_lv_data (&lvid, querylv->currentsize, querylv->mirrors, lv_data, &pv_count, sourcepv);

		for (i=0; i< pv_count; i++) {  /* for each PV that contains some of the LV's PPs */

			num_copies (lv_data[i].lp_flag, lv_data[i].max_lp, &copy1, &copy2, &copy3);  /* get copy data */
			sprintf (copies_str, "%.3d:%.3d:%.3d", copy1, copy2, copy3);  /* translate data into ascii format */

			if (odm_rc == SUCCESS  &&  lv_data[i].pvsize != UNDEFINED_INT) {  /* if all data is available */
				inband_value = inband (lv_data[i].pp_list, lv_data[i].numpps,  /* get inband percent */
				    lv_data[i].pvsize, intra_policy);
				sprintf (inband_str, "%d%s", inband_value, "%");  /* translate inband to ascii format */
			}
			else  /* if unable to get inband, fill in with special string which indicates no data available */
				strcpy (inband_str, UNDEF_STR);

			if (lv_data[i].pvsize != UNDEFINED_INT) {  /* if required data is available -- the PV size */
				/* calculate the distribution of the PPs in the PV (by regions) */
				distrib_value = list_distribution (lv_data[i].pp_list, lv_data[i].numpps, lv_data[i].pvsize);
				sprintf (distrib_str, "%.3d:%.3d:%.3d:%.3d:%.3d", distrib_value[0], distrib_value[1],
					 distrib_value[2], distrib_value[3], distrib_value[4]);  /* translate to ascii */
			}
			else  /* if unable to get distrib, fill in with special string which indicates no data available */
				strcpy (distrib_str, UNDEF_STR);

			/* the headings are printed here so error messages will not come between headings and the data */
			if (i == 0) {  /* if this is first time through the loop */
				if (odm_rc == SUCCESS)  /* if the lvname and mount point is available */
					printf("%s:%s\n", lvdescript, mount_point);  /* print out as first line */
				else  /* if the name is not known, the lvid will have to be used */
					printf("lvid=%s\n", lvid_ascii);  /* print lvid as first line */

				if (lv_data[i].pvdescript_type == name)  /* if pvname is known, use "PV" for heading */
					LFORMAT_PRINT ("PV", "COPIES","IN BAND","DISTRIBUTION","");
				else  /* if the pvname is not known use "PVID" as heading */
					LFORMAT_PRINT ("PVID","COPIES","IN BAND","DISTRIBUTION","");
			}

			/* output a line of list data -- this is the data for a single PV */
			LFORMAT_PRINT (lv_data[i].pvdescript, copies_str, inband_str, distrib_str,"");
		}
		lvm_freebuf(querylv);
	}
	else {  /* was unable to get LVM data therefore nothing to list -- fatal error */
		errmsg (query_rc);  /* print low-level lvm error message */
	}
}





/*
 * NAME: mfunction
 *
 * FUNCTION: Implements the "m" option of the lslv command which prints out the lvm_querylv map with logical names.
 *
 * OUTPUT: Data selected by the "m" option is written to standard out -- the allocation map info for each LP in LV.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 *
 */
void mfunction(lvdescript, sourcepv)
char
	*lvdescript,    /* descriptor of the logical volume to list (INPUT) */
	*sourcepv;      /* optional (maybe NULL) pvdescript for LVM to indicate a particular PV to read info from (INPUT) */
{
	int
		cptr;                   /* loop counter which corresponds to a LP each time though loop */
	char
		lvid_ascii[STRSIZE],    /* lvid in ascii format */
		pvname[STRSIZE];        /* pvname for a pvid */
	RETURN_CODE
		odm_rc,                 /* return code from ODM query */
		query_rc,               /* return code for LVM query */
		getpv_rc;               /* return code for get_pvdescript */
	struct lv_id
		lvid;                   /* the lvid in binary format */
	struct lv_odm_data2
		odm;                    /* the ODM data for the lv -- see structure definition */
	struct querylv
		*querylv;               /* required structure for querying data from LVM */

	RETURN_CODE get_odm_data_chme(), get_lvid(), get_pvdescript();


	if (get_lvid(lvdescript, lvid_ascii) != SUCCESS) {  /* must have lvid to query LVM */
		exit(1);
	}

	odm_rc = get_odm_data_chme (lvid_ascii, &odm);  /* retrieve the lv data stored by ODM */

	parsepair(lvid_ascii, ".", 3, 22, &lvid.vg_id, &lvid.minor_num);  /* translate binary lvid into ascii format */

	query_rc = (RETURN_CODE) lvm_querylv (&lvid, &querylv, sourcepv);  /* retrieve all lv data stored by LVM */
	if (query_rc == SUCCESS) {
		for(cptr=0; cptr < querylv->currentsize; cptr++) {  /* for each LP in the LV */

			getpv_rc = get_pvdescript (querylv->mirrors[0][cptr].pv_id, pvname);  /* translate pvid to pvname */

			/* the headings are printed here so error messages will not come between headings and the data */
			if (cptr == 0) {  /* if this is first time through the loop */
				if (odm_rc == SUCCESS)  /* if the lvname and mount point is available */
					printf("%s:%s\n", odm.lvname, odm.mount_point);
				else  /* if the name is not known, the lvid will have to be used */
					printf("lvid=%s\n", lvid_ascii);
				if (getpv_rc == SUCCESS)  /* if pvname is known, use "PV" for heading */
					printf("LP    PP1  PV1               PP2  PV2               PP3  PV3\n");
				else  /* if the pvname is not known use "PVID" as heading */
					printf("LP    PP1  PVID1             PP2  PVID2             PP3  PVID3\n");
			}

			printf("%.4d  ", querylv->mirrors[0][cptr].lp_num);  /* output the LP number */

			printf("%.4d %-16s  ", querylv->mirrors[0][cptr].pp_num, pvname);  /* output first PP & PV name */

			/* if the LP has a second PP/copy allocated to it */
			if (querylv->mirrors[1] != NULL  && querylv->mirrors[1][cptr].pp_num != 0) {
				/* translate pvid to pvname -- since get_pvdescript successful before it probably is now */
				get_pvdescript (querylv->mirrors[1][cptr].pv_id, pvname);
				printf("%.4d %-16s  ", querylv->mirrors[1][cptr].pp_num, pvname);  /* output 2nd PP & PV */
			}

			/* if the LP has a third PP/copy allocated to it */
			if (querylv->mirrors[2] != NULL  && querylv->mirrors[2][cptr].pp_num != 0) {
				/* translate pvid to pvname -- since get_pvdescript successful before it probably is now */
				get_pvdescript (querylv->mirrors[2][cptr].pv_id, pvname);
				printf("%.4d %-16s  ", querylv->mirrors[2][cptr].pp_num, pvname);  /* output 3nd PP & PV */
			}

			printf("\n");  /* output end-of-line for this LP */
		}
		lvm_freebuf(querylv);
	}
	else {  /* was unable to get LVM data therefore nothing to list -- fatal error */
		errmsg (query_rc);  /* print low-level lvm error message */
	     }
}





/*
 * NAME: pfunction
 *
 * FUNCTION: Implements the "p" option of the lslv command which prints out detailed LV data for a single PV.
 *
 * OUTPUT: Data selected by the "p" option is written to standard out -- the status of each PP on the PV is shown.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 *
 */
void pfunction(lvdescript, pvdescript, sourcepv)
char
	*lvdescript,    /* descriptor of the logical volume to highlight on the PV (INPUT) */
	*pvdescript,    /* descriptor of the physical volume to list (INPUT) */
	*sourcepv;      /* optional (maybe NULL) pvdescript for LVM - indicates a particular PV to read info from (INPUT) */
{
	int
		filled,                   /* number of columns filled with LP data - distingushed from blank columns */
		column,                   /* number of columns output */
		first_column_pp,          /* pp number of first partition listed in an output column */
		i, j;                     /* loop counters and array indexes */
	char
		lp_string[STRSIZE],       /* contains the state of a LP; e.g "FREE" */
		lvid_ascii[STRSIZE],      /* lvid in ascii format */
		vgid_ascii[STRSIZE],      /* vgid in ascii format */
		pvid_ascii[STRSIZE],      /* pvid in ascii format */
		tag;                      /* tag field for LPs belonging to lvdescript -- "?" if stale LP, otherwise " " */
	RETURN_CODE
		odm_rc,                   /* return code from ODM query */
		query_rc;                 /* return code for LVM query */
	PVDESCRIPT_TYPE
		pvdescript_type;          /* type of the pvdescript being used - id or name */
	struct querypv
		*querypv;                 /* the lv data retrieved from the LVM -- see structure definition in lvm.h */
	struct REGION
		*regions;                 /* the boundaries of each of the five regions of a PV */
	struct unique_id
		pvid,                     /* pvid in binary format */
		vgid;                     /* vgid in binary format */
	struct lv_odm_data2
		odm;                      /* the ODM data for the lv -- see structure definition */
	static struct lv_id
		lvid = { { 0, 0 } , 0 };  /* the lvid in binary format */

	RETURN_CODE get_odm_data_chme(), get_lvid(), get_pvid();
	struct REGION *getregions();


	if (lvdescript != NULL) {  /* lvdescript is optional -- so if present then parse it and get its associated data */
		if (get_lvid(lvdescript, lvid_ascii) != SUCCESS) {
			exit(1);
		}
		parsepair(lvid_ascii, ".", 3, 22, &lvid.vg_id, &lvid.minor_num); /* translate binary lvid to ascii format */
		vgid.word1 = lvid.vg_id.word1;    vgid.word2 = lvid.vg_id.word2; /* vg is known in this case from lvid */
		odm_rc = get_odm_data_chme (lvid_ascii, &odm);  /* retrieve the lv data stored by ODM */
	}
	else {  /* no lvdescript specified -- must get vgid by using the pvdescript, it is required for pv query */
		odm_rc = get_vgid (pvdescript, vgid_ascii);
		if (odm_rc != SUCCESS) {
			exit (1);
		}
		get_uniq_id (vgid_ascii, &vgid);  /* parse the ascii vgid into binary format */
		/*
		 * lock vg (note that in the "lvdescript != NULL" case, the
		 * vg was locked in main)
		 */
		if (lock_a_vg(Prog, vgid_ascii))
			return;
	}
	if (get_pvid (pvdescript, pvid_ascii, &pvdescript_type) == SUCCESS) {  /* must also have pvid to query pv */
		get_uniq_id (pvid_ascii, &pvid);  /* parse pvid into binary format */
		query_rc = (RETURN_CODE) lvm_querypv (&vgid, &pvid, &querypv, sourcepv);  /* retrieve pv data from LVM */

		if (query_rc == SUCCESS) {  /* if pv data successfully retrieved */
			regions = getregions(querypv->pp_count);  /* get the boundaries of the PV's 5 regions */

			if (lvdescript != NULL) {  /* if lvdescript then displace pv, lv, and mount point on first line */
				if (pvdescript_type == name)
					printf("%s:", pvdescript);
				else  /* was unable to get the pvname so use pvid instead */
					printf("pvid=%s:", pvid_ascii);
				if (odm_rc == SUCCESS)  /* if the lvname and mount point was retrieved from odm */
					printf("%s:%s\n", odm.lvname, odm.mount_point);
				else  /* was unable to get the lvname so use lvid instead */
					printf("lvid=%s\n", lvid_ascii);
			}
			else  /* no lvdescript was used so only display pv info on first line */
				if (pvdescript_type == name)
					printf("%s:::\n", pvdescript);
				else  /* was unable to get the pvname so use pvid instead */
					printf("pvid=%s\n", pvid_ascii);

			for (i=0; i<SREGION; i++) {  /* for each of the regions of the PV */
				/* Print out all the LPs in one region and leave a blank line to separate next region. */
				/* The PP numbers are displayed in far right column so must align using blank columns. */
				/* Note LP numbers are only displayed if belong to lvdescript LV, otherwise LP status. */
				for (filled=column=0,first_column_pp=j=regions[i].low;
				     j<=regions[i].high || column != NO_COLUMNS; j++) {
					if (j<=regions[i].high) {  /* if more LPs in regions */
						if (querypv->pp_map[j-1].pp_state == LVM_PPFREE)
							sprintf(lp_string,"%-7s", "FREE");
						else if (SAMELVID(querypv->pp_map[j-1].lv_id, lvid)) {
							tag = (querypv->pp_map[j-1].pp_state == CMDLVM_PPSTALE) ? '?' : ' ';
							sprintf(lp_string,"%.4d%c  ", querypv->pp_map[j-1].lp_num, tag);
						}
						else if (querypv->pp_map[j-1].pp_state == CMDLVM_PPSTALE)
							sprintf(lp_string,"%-7s", "STALE");
						else
							sprintf(lp_string,"%-7s", "USED");
						filled++;
					}
					else  /* no more LPs in regions so fill remaining columns with blanks */
						sprintf(lp_string,"%-7s", " ");

					printf("%s", lp_string);  /* display data for one LP */
					if (++column == P_COLUMNS) {  /* if row of LPs has been filled */

						/* print the range of PPs the LPs are associated with */
						printf("  %3d-%-d\n", first_column_pp, first_column_pp + (filled-1));
						first_column_pp += filled;  /* set PP number for first column of new row */
						filled=column=0;  /* reset counters for the next row */
					}
				}
				if (i < (SREGION-1)) printf("\n");  /* if another region is left, leave blank line */
			}
			lvm_freebuf(querypv);
		}
		else {  /* cannot query the pv data so print error and leave function */
		       errmsg (query_rc);  /* print low-level lvm error message */
		}
	}
}


/*
 * NAME: getlvattr
 *
 * FUNCTION: Gets a logical volume attribute from the CuAt class.
 *
 * RETURN VALUE DESCRIPTION: 
 */
int
getlvattr(attr,lvchartstics,lvname)
char *attr;		/* attribute searching for (INPUT) */
char *lvchartstics;	/* (INPUT & OUTPUT) */
char *lvname;		/* logical volume name */
{
struct CuAt *cuatp;
int temp;

	cuatp = getattr(lvname,attr,FALSE,&temp);
	if ((int)cuatp == -1) odm_error(odmerrno);
	if (cuatp != NULL) sprintf(lvchartstics,"%s",cuatp->value);
        else strcpy(lvchartstics,UNDEF_STR);
}

getodmdata(lvdescript,vgname,type,inter_policy,intra_policy,strict,relocatable,vgstate, copies_ascii,upper_bound_ascii,label,stripe_width,stripe_size)

   char   *lvdescript,*vgname;
   char   *type,*inter_policy,*intra_policy,*strict,*relocatable,*vgstate,
           *copies_ascii,*upper_bound_ascii,*label,*stripe_width,*stripe_size;

{
   
   char tempstr[STRSIZE];
   struct CuDv *cudvp;
   struct listinfo stat_info;
   char crit[CRITSIZE],
   	vgid_ascii[STRSIZE];      /* vgid in ascii format */

   struct unique_id vgid;
   RETURN_CODE query_rc;

   odm_initialize();
   odm_set_path(CFGPATH);

   /* get the logical volume characteristics from the database */

     	sprintf(crit,"name = '%s'",lvdescript);
	cudvp = (struct CuDv *)odm_get_list(CuDv_CLASS,crit,&stat_info,1,1);
	if ((int)cudvp == ODMERROR) odm_error(odmerrno);
	if (cudvp != NULL) strcpy(vgname,cudvp->parent);           

	/* The original logic for figuring out the vgstate of     */
	/* the lv has been scrapped.  The original logic depended */
	/* on information from the odm, which could be unreliable */
	/* The design and associated functions, were lifted from  */
	/* the lsvg code, which was correctly altered to query the*/
	/* disk itself to see if it (and thus the vg) was varied  */
	/* on.							  */

        if (get_vg_id(vgname,vgid_ascii) != SUCCESS)
		exit(1);

	/* convert ascii vg id to binary for querying LVM */
        get_uniq_id(vgid_ascii, &vgid);

        query_rc = lvm_chkvaryon(&vgid);
        if (query_rc < LVM_SUCCESS)
		strcpy(vgstate , "inactive");
        else
		strcpy(vgstate , "active/complete");


        getlvattr("type",type,lvdescript);

        getlvattr(INTER,tempstr,lvdescript);
	switch (tempstr[0]) {
		case 'm'     : strcpy(inter_policy , "minimum"); break;
		case 'x'     : strcpy(inter_policy , "maximum"); break;
		case 'p'     : strcpy(inter_policy , "map"); break;
		default      : strcpy(inter_policy , UNDEF_STR); break;
	}
        getlvattr(INTRA,tempstr,lvdescript);
	switch (tempstr[0])  {
		case 'e'     : strcpy(intra_policy , "edge"); break;
		case 'm'     : strcpy(intra_policy , "middle"); break;
		case 'c'     : strcpy(intra_policy , "center"); break;
		case 'i'     : 
                               if (tempstr[1] == 'e')
                                    strcpy(intra_policy , "inner edge"); 
                               else
                               if (tempstr[1] == 'm')
                                    strcpy(intra_policy , "inner middle"); 
          
                               break;
		default      : strcpy(intra_policy , UNDEF_STR); break;
	}
        getlvattr(STRICTNESS,tempstr,lvdescript);
	switch (tempstr[0]) {
		case 'y'     : strcpy(strict , "yes"); break;
		case 'n'     : strcpy(strict , "no"); break;
		default      : strcpy(strict , UNDEF_STR); break;
	}
        getlvattr(RELOCATABLE,tempstr,lvdescript);
	switch (tempstr[0]) {
		case 'y'     : strcpy(relocatable,"yes"); break;
		case 'n'     : strcpy(relocatable,"no"); break;
		default      : strcpy(relocatable,UNDEF_STR); break;
	}


    getlvattr(COPIES,copies_ascii,lvdescript);

#ifdef COMPLETE
	if (odm->copies > 0  &&  odm->copies <= LVM_NUMCOPIES)
		sprintf(copies_ascii, "%d", odm->copies);
	else
		strcpy (copies_ascii, UNDEF_STR);
#endif

   getlvattr(UPPERBOUND,upper_bound_ascii,lvdescript);

#ifdef COMPLETE
	if (odm->upper_bound == NUMBER_OF_PVS_FLAG)
		sprintf(upper_bound_ascii, "%s", "maximum");
	else if (odm->upper_bound > 0 && odm->upper_bound <= LVM_MAXPVS)
		sprintf(upper_bound_ascii, "%d", odm->upper_bound);
	else
		sprintf(upper_bound_ascii, "%s", UNDEF_STR);
#endif

   getlvattr(LABEL,label,lvdescript);
   getlvattr(STRIPE_WIDTH,stripe_width,lvdescript);
   getlvattr(STRIPE_SIZE,stripe_size,lvdescript);
}

/*
 * NAME: defaultf
 *
 * FUNCTION: Display the default data for the lslv command.  All the characteristics of the LV are displayed.
 *
 * OUTPUT: Data selected by default is written to standard out.
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 *
 * NOTE: All of the data fields (some of them normally integer values) are converted to strings.  This is done
 *       since the default value (UNDEF_STR) is a string.  This approach was not required but it allows more
 *       descriptive fields to be displayed for the user (i.e. "??????" instead of "0" for unknown integer value).
 */
void defaultf(lvdescript, sourcepv)
char
	*lvdescript,    /* descriptor of the logical volume to list (INPUT) */
	*sourcepv;      /* optional (maybe NULL) pvdescript for LVM to indicate a particular PV to read info from (INPUT) */
{
	int
	    i,         /* counter and array index */
	    pv_count,  /* number of PVs allocated on by the LV */
	    in_band;   /* percentage of PPs allocation in the inter-PV region */
	char
            tempdev[STRSIZE],
            mount_point[STRSIZE],
            vgname[STRSIZE],
            type[STRSIZE],
            label[LABELLENGTH],
	    permission[STRSIZE],     /* string representing LV permission */
	    write_verify[STRSIZE],   /*  LV write verify state */
            mwc[STRSIZE],            /* mirror write consistency */ 
	    mirror_policy[STRSIZE],  /* string representing LV mirror policy */
	    allocation[STRSIZE],     /* string indicating if the current LV 
                               allocation is strict or non-strict */
	    inter_policy[STRSIZE],   /*  LV inter-PV allocation policy */
	    intra_policy[STRSIZE],   /*  LV intra-PV allocation policy */
	    strict[STRSIZE],      /*  strictness is  requirement */
	    relocatable[STRSIZE],/*  if the LV can be relocated (by reorgvg) */
	    bb_policy[STRSIZE],/*  LV bad block relocation policy */
	    vg_state[STRSIZE], /* string representing the volume group state */
	    pvdist[STRSIZE], /* the distribution of LPs across PVs */
	    upper_bound_ascii[STRSIZE],  /*  number of PVs used for LV  */
	    lvid_ascii[STRSIZE],    /* string representing LV id */
	    copies_ascii[STRSIZE], /*  no of copies currently allocated to LV */
	    stale_ascii[STRSIZE],     /*  number of stale PP in LV */
	    alloc_ascii[STRSIZE],     /* copy of the allocation string */
	    in_band_ascii[STRSIZE],   /* % of PPs used  in  region */
	    ppsize_ascii[STRSIZE],    /*  size of PPs in the LV/VG */
	    lv_state[STRSIZE],        /*  current state of the LV */
	    currentsize_ascii[STRSIZE], /*  current size in LPs of the LV */
	    total_pps_ascii[STRSIZE],  /* tot num of PPs allocated to the LV */
	    maxsize_ascii[STRSIZE],    /*  max num of LPs the LV can have */
	    stripe_width[STRSIZE],     /*  stripe width (number of PVs) */
	    stripe_size[STRSIZE];      /*  stripe size in Kbytes or Mbytes */
	RETURN_CODE
		retodm,        /* return code from ODM query */
		retlvm;        /* return code for LVM query */
	struct querylv
	    *querylv;       /* required structure for querying data from LVM */
	struct lv_info2
	    lv_data[LVM_MAXPVS];    /* LV map split and organized by PV */
	struct lv_id lvid;          /* the lvid in binary format */
    struct fstab * fsstruct;

	int in_band_percent(),  get_lvid();
	RETURN_CODE  get_lvid(), stale_count();
	BOOLEAN strict_allocation();
	void get_lv_data(), init_querylv_struct(), translate_lvm_data() ;

    /* must have lvid to query LVM */
	if (get_lvid(lvdescript, lvid_ascii) != SUCCESS) {  
		exit(1);
	}

/* translate binary lvid into ascii format */
parsepair(lvid_ascii, ".", 3, 22, &lvid.vg_id, &lvid.minor_num);  

retlvm = getlvmdata (sourcepv,lvid,lv_data,mwc,permission,write_verify,mirror_policy, 
       bb_policy, lv_state, ppsize_ascii, currentsize_ascii, maxsize_ascii,
       alloc_ascii,total_pps_ascii,stale_ascii,allocation,&pv_count);

retodm = getodmdata(lvdescript,vgname,type,inter_policy,intra_policy,strict,
	relocatable,vg_state, copies_ascii,upper_bound_ascii,label,
	stripe_width,stripe_size);

/* get the mount point */
sprintf(tempdev,"/dev/%s",lvdescript);
setfsent();
fsstruct = getfsspec(tempdev);
if (fsstruct != NULL) strcpy(mount_point,fsstruct->fs_file);
else strcpy(mount_point,"N/A");
endfsent();

#ifdef DETAIL
/* if both LVM and ODM information is available */
if (retlvm == SUCCESS  &&  retodm == SUCCESS) { 
    /* calculate the inband percentage */
	in_band = in_band_percent (lv_data, pv_count, intra_policy);  
	if (in_band != UNDEFINED_INT)  /* if able to calculate inband */
			sprintf(in_band_ascii, "%d%c", in_band, '%');
	else  /* if not able to calculate inband */
			strcpy (in_band_ascii, UNDEF_STR);
}
else 
      strcpy (in_band_ascii, UNDEF_STR);
#endif

/* display in columns the logical volume data */
DEFAULT_PRINT("LOGICAL VOLUME:",lvdescript, "VOLUME GROUP:", vgname);
DEFAULT_PRINT("LV IDENTIFIER:", lvid_ascii, "PERMISSION:", permission);
DEFAULT_PRINT("VG STATE:", vg_state, "LV STATE:", lv_state);
DEFAULT_PRINT("TYPE:", type , "WRITE VERIFY:", write_verify);
DEFAULT_PRINT("MAX LPs:", maxsize_ascii, "PP SIZE:", ppsize_ascii);
DEFAULT_PRINT("COPIES:", copies_ascii, "SCHED POLICY:", mirror_policy);
DEFAULT_PRINT("LPs:", currentsize_ascii, "PPs:", total_pps_ascii);
DEFAULT_PRINT("STALE PPs:", stale_ascii, "BB POLICY:", bb_policy);
DEFAULT_PRINT("INTER-POLICY:", inter_policy, "RELOCATABLE:", relocatable);
DEFAULT_PRINT("INTRA-POLICY:", intra_policy, "UPPER BOUND:",upper_bound_ascii);
DEFAULT_PRINT("MOUNT POINT:", mount_point, "LABEL:", label);
DEFAULT_PRINT("MIRROR WRITE CONSISTENCY:", mwc,"","");
DEFAULT_PRINT("EACH LP COPY ON A SEPARATE PV ?:",strict,"", "");

if (atoi(stripe_width) > 0) {
	DEFAULT_PRINT("STRIPE WIDTH:",stripe_width,"", "");
	DEFAULT_PRINT("STRIPE SIZE:",stripe_size,"", "");
}


#ifdef DETAIL
/* display in columns the allocation distribution - 
     "pvname":"number of lps":"number of pps" */
printf ("%s", "PV:LP:PP DISTRIBUTION:    ");

 /* if the correct data is available and pvcount is valid */
if (retlvm == SUCCESS  &&  pv_count <= LVM_MAXPVS) 
for (i=0; i < pv_count; i++) {  /* for each PV used by the LV */
   if ((i+1)%3==0) printf("\n");
   sprintf(pvdist, 
   "%s:%.2d:%.2d", lv_data[i].pvdescript, lv_data[i].numlps, lv_data[i].numpps);
   printf ("   %-23s", pvdist);
}
else  /* unable to display distribution so use default value */
	printf(UNDEF_STR);
	printf("\n");
#endif
}



/*
 * NAME: init_querylv_struct
 *
 * FUNCTION: Allocate and initialize with default values the structure used to query LVM for LV information.
 *
 * RETURN CODE: none
 */
void init_querylv_struct (querylv)
struct querylv **querylv;  /* the allocated and initialized structure (OUTPUT) */
{
	*querylv = (struct querylv *) getmem (sizeof(struct querylv));
	(*querylv)->permissions = UNDEFINED_INT;
	(*querylv)->open_close = UNDEFINED_INT;
	(*querylv)->write_verify = UNDEFINED_INT;
	(*querylv)->mirror_policy = UNDEFINED_INT;
	(*querylv)->bb_relocation = UNDEFINED_INT;
	(*querylv)->lv_state = UNDEFINED_INT;
	(*querylv)->ppsize = UNDEFINED_INT;
	(*querylv)->currentsize = UNDEFINED_INT;
}



/*
 * NAME: getlvmdata
 *
 * FUNCTION: Take the LVM query data, verify it, and translate it to a user oriented string.
 *
 * RETURN CODE: none
 */

int  getlvmdata (sourcepv, lvid,lv_data,mwc,permission, write_verify, mirror_policy, 
       bb_policy, lv_state, ppsize_ascii, currentsize_ascii, maxsize_ascii,
       alloc_ascii,total_pps_ascii,stale_ascii,allocation,pvcount)

char   *sourcepv;
struct lv_id lvid;       /* the lvid in binary format */
struct lv_info2 lv_data[];  /* LV map split and organized by PV */
char
*mwc,           /*  mirror write consistency */
*permission,    /*  LV permission (OUIPUT) */
*write_verify,  /* string representing LV write verify state (OUTPUT) */
*mirror_policy, /* string representing LV mirror policy (OUTPUT) */
*bb_policy,     /*  LV bad block relocation policy (OUTPUT) */
*lv_state,      /* string representing current state of the LV (OUTPUT) */
*ppsize_ascii,  /* string representing size of PPs in the LV/VG (OUTPUT) */
*currentsize_ascii,  /*  current size in LPs of the LV (OUTPUT) */
*maxsize_ascii,      /*  the maxm num of LPs for the LV (OUTPUT) */
*alloc_ascii,
*total_pps_ascii,
*stale_ascii,
*allocation;
int *pvcount;
{
struct querylv    *querylv;        /* structure for LVM query data (INPUT) */
long int numbytes, num_megabytes;  /* number of bytes/megabytes in a PP */
int
            query_rc,pv_count,i,
	    total_pps=0,     /* count of total number of PPs in the LV */
	    stale_pps;       /* count of the number of stale LPs in the LV */

strcpy (alloc_ascii, UNDEF_STR);
strcpy (stale_ascii, UNDEF_STR);
strcpy (total_pps_ascii, UNDEF_STR);
init_querylv_struct (&querylv);

/* retrieve all lv data stored by LVM */
query_rc = (RETURN_CODE) lvm_querylv (&lvid, &querylv, sourcepv);  
if (query_rc != 0) {
	strcpy (mwc, UNDEF_STR);
	strcpy (permission, UNDEF_STR);
	strcpy (write_verify, UNDEF_STR);
	strcpy (mirror_policy, UNDEF_STR);
	strcpy (bb_policy, UNDEF_STR);
	strcpy (lv_state, UNDEF_STR);
	strcpy (ppsize_ascii, UNDEF_STR);
	strcpy (currentsize_ascii, UNDEF_STR);
	strcpy (maxsize_ascii, UNDEF_STR);
	return(-1);
}

/* if able to get the LV map then process it */
get_lv_data (&lvid, querylv->currentsize, 
                 querylv->mirrors, lv_data, &pv_count, sourcepv);
*pvcount = pv_count;

/* for each PV, count the number of PPs used to get total */
for (i=0; i<pv_count; i++)  total_pps += lv_data[i].numpps;
sprintf(total_pps_ascii, "%d", total_pps);

allocation = strict_allocation(lv_data, pv_count) ? 
                 "strict" : "non-strict";  /* determine strictness */

        /* count number of stale PPs */
if (stale_count (&lvid, lv_data, pv_count, &stale_pps) == SUCCESS)  
                /* convert numbers to ascii */
		sprintf(stale_ascii, "%d", stale_pps);  
else
            /* since stale count is undefined use default string */
	    sprintf(stale_ascii, "%s", UNDEF_STR);  

strcpy (alloc_ascii, allocation);  /* copy value to common variable */

/* convert the querylv data to mneumonic strings for output to user */

         switch (querylv->mirwrt_consist) {
    		case 1: strcpy(mwc , "on");
                    break;
    		case 2: strcpy(mwc , "off");
                     break;
    		default : strcpy(mwc , UNDEF_STR) ;
        }
	switch (querylv->permissions) {
		case LVM_RDONLY : strcpy(permission , "read only"); break;
		case LVM_RDWR   : strcpy(permission , "read/write"); break;
		default         : strcpy(permission , UNDEF_STR); break;
	}
	switch (querylv->write_verify) {
		case LVM_VERIFY   : strcpy(write_verify , "on"); break;
		case LVM_NOVERIFY : strcpy(write_verify , "off"); break;
		default           : strcpy(write_verify , UNDEF_STR); break;
	}
	switch (querylv->mirror_policy) {
		case LVM_PARALLEL   : strcpy(mirror_policy , "parallel"); break;
		case LVM_SEQUENTIAL : strcpy(mirror_policy , "sequential"); break;
		case LVM_STRIPED    : strcpy(mirror_policy , "striped"); break;
		default             : strcpy(mirror_policy , UNDEF_STR); break;
	}
	switch (querylv->bb_relocation) {
		case LVM_RELOC   : strcpy(bb_policy , "relocatable"); break;
		case LVM_NORELOC : strcpy(bb_policy , "non-relocatable"); break;
		default          : strcpy(bb_policy , UNDEF_STR); break;
	}
	switch (querylv->open_close) {
		case LVM_QLVOPEN     : strcpy(lv_state, "opened"); break;
		case LVM_QLV_NOTOPEN : strcpy(lv_state, "closed"); break;
		default              : strcpy(lv_state, UNDEF_STR); break;
	}
	if (strcmp(lv_state, UNDEF_STR))
		switch (querylv->lv_state) {
			case LVM_LVDEFINED    : strcat(lv_state,"/syncd"); break;
			case CMDLVM_LVSTALE   : strcat(lv_state,"/stale"); break;
			default               : strcat(lv_state, UNDEF_STR); break;
		}
	if (querylv->ppsize >= LVM_MINPPSIZ  &&  querylv->ppsize <= LVM_MAXPPSIZ) {
		numbytes = 1L << querylv->ppsize;
		num_megabytes = numbytes / MEGABYTE;
		sprintf(ppsize_ascii, "%ld megabyte(s)", num_megabytes);
	}
	else
		strcpy (ppsize_ascii, UNDEF_STR);


	if (querylv->currentsize > 0  &&  querylv->currentsize <= MAXLV)
		sprintf (currentsize_ascii, "%d", querylv->currentsize);
	else
		strcpy (currentsize_ascii, UNDEF_STR);


	if (querylv->maxsize > 0  &&  querylv->maxsize <= MAXLV)
		sprintf (maxsize_ascii, "%d", querylv->maxsize);
	else
		strcpy (maxsize_ascii, UNDEF_STR);

	lvm_freebuf(querylv);
}

/*
 * NAME: stale_count
 *
 * FUNCTION: Count the number of stale PPs allocated to a LV.  A failure is returned if enought data is not available.
 *
 * RETURN VALUE: 0 == SUCCESS
 *               non_zero == FAILURE
 */
RETURN_CODE stale_count(lvid, lv_data, numpvs, stale)
struct lv_id
	*lvid;      /* the lvid in binary format (INPUT) */
struct lv_info2
	lv_data[];  /* the LV data separated and organized by PV (INPUT) */
int
	numpvs,     /* the number of PV used by the LV -- also size of lv_data (INPUT) */
	*stale;     /* the number of stale PP in the LV (OUTPUT) */
{
	int
		i, j;        /* array indexes and counters */
	RETURN_CODE
		rc=SUCCESS;  /* return of the function */

	for (*stale=i=0; i<numpvs; i++)  /* for each PV used by the LV */
		if (lv_data[i].pvmap != NULL) {  /* if the querypv map is stored -- it will be if the map is accessable */
			for (j=0; j<lv_data[i].pvsize; j++)
				if (lv_data[i].pvmap[j].pp_state == CMDLVM_PPSTALE &&  /* if stale PP */
				    SAMELVID(lv_data[i].pvmap[j].lv_id, *lvid))   /* ..and PP belongs to LV */
					(*stale)++;                                 /* ..then increment stale count */
		}
		else {  /* all the data was not available so return error */
			rc=FAILURE;
			break;
		}
	return (rc);
}




/*
 * NAME: num_copies
 *
 * FUNCTION: Calculate the number of LPs with one AT LEAST one copy, AT LEAST two copies, and three copies.
 *
 * RETURN CODE: none
 */
void num_copies (lp_list, max_lp, copy1, copy2, copy3)
BYTE
	lp_list[];  /* list with number of PPs allocated to each LP */
int
	max_lp,     /* max LP number in the LV, also size of lp_list */
	*copy1,     /* number of LPs with at least one copy/PP allocated to it */
	*copy2,     /* number of LPs with at least two copies/PPs allocated to it */
	*copy3;     /* number of LPs with at least one copies/PPs allocated to it */
{
	int i;      /* loop counter */

	for (*copy1 = *copy2 = *copy3 = 0, i=1; i<=max_lp; i++)  /* for each LP in the LV */
		switch (lp_list[i]) {  /* lp_list contains 0, 1, 2, or 3 for the number of PPs */
			case  3 : (*copy3)++;
			case  2 : (*copy2)++;
			case  1 : (*copy1)++;
			default : break;
		}
}





/*
 * NAME: in_band_percent
 *
 * FUNCTION: Calculate the number of PPs in the LV that are allocated in the intra-PV region (edge, middle, or center)
 *
 * RETURN VALUE: percentage of PPs in LV that are allocated in the intra-PV region (if the return value is UNDEFINED_INT
 *               then there was not enought information to finish computing the inband percentage)
 */
PERCENTAGE in_band_percent (lv_data, numpvs, intra_policy)
struct lv_info2
	lv_data[];     /* the logical volume data organized by PV's */
int
	numpvs;        /* number of PVs used by the LV, also size of lv_data */
char
	intra_policy;  /* the intra-PV region */
{
	int
		i,                 /* loop counter and array index */
		pps_total=0;       /* current count of the total number of PPs in the LV */
	long int
		in_band_total=0;   /* current cumulative value of inband for the PVs processed */
	BOOLEAN
		error=FALSE;       /* if TRUE then error occurred since there was not enought information available */


	for (i=0; i<numpvs; i++)   /* for each PV used by the LV */
		if (lv_data[i].pvsize != UNDEFINED_INT) {  /* if the PV size is known */
			in_band_total += inband (lv_data[i].pp_list, lv_data[i].numpps, lv_data[i].pvsize, intra_policy) *
			    lv_data[i].numpps;  /* compute inband for one PV and add to the inband total */
			pps_total += lv_data[i].numpps;  /* keep count of the total number of PPs in the LV */
		}
		else {  /* if size was not known then cannot compute region boundaries */
			error = TRUE;
			break;
	}
	/* if error return UNDEFINED_INT otherwise return the computed inband percentage */
	return(error ? UNDEFINED_INT : (int) (in_band_total / pps_total));
}





/*
 * NAME: inband
 *
 * FUNCTION: Calculate the inband (number of PPs in LV allocated in intra-PV region) for a single PV in the LV.
 *
 * RETURN CODE: the percentage of PPs in LV on PV that are allocated in the intra-PV region
 */
PERCENTAGE inband(pp_list, numpps, pvsize, intra_policy)
int
	pp_list[],            /* list of PPs on PV that are used by the LV (INPUT) */
	numpps,               /* number of PPs in pp_list (INPUT) */
	pvsize;               /* size of the PV (INPUT) */
char
	*intra_policy;         /* intra_policy of the LV */
{
	int
		j,            /* loop counter and array index */
		ppnum,        /* current pp number */
		in_band=0;    /* count of number of PPs found inband */
	int
		low1,         /* lower boundary of first intra region */
		low2,         /* lower boundary of second intra region */
		high1,        /* upper boundary of first intra region */
		high2;        /* upper boundary of second intra region */
	struct REGION
		*regions;     /* region boundaries for the PV */

	struct REGION *getregions ();


	regions = getregions(pvsize);  /* calculate the region boundaries for the PV */
	switch (intra_policy[0]) {        /* switch on the intra region being examined to get the correct boundaries */
		case 'e' : low1 = regions[0].low;
			   high1 = regions[0].high;
			   break;
		case 'm' : low1 = regions[1].low;
			   high1 = regions[1].high;
			   break;
		case 'c' : low1 = regions[2].low;
			   high1 = regions[2].high;
			   low2 = 0;    /* there is only on center region */
			   high2 = 0;
			   break;
                case 'i' : if (intra_policy[1]== 'e')   {
			      low2 = regions[4].low;
			      high2 = regions[4].high;
                           }
                           else
                           if (intra_policy[1] == 'm') {
			   low2 = regions[3].low;
			   high2 = regions[3].high;
                           }
                           break;

		default  : low1 = low2 = high1 = high2 = 0;
	}
	for (j=0; j<numpps; j++) {  /* for each PP */
		ppnum = pp_list[j];
		if ((ppnum >= low1 && ppnum <= high1) || (ppnum >= low2 && ppnum <= high2))  /* if in the intra region */
			in_band++;
	}
	return( ( int ) ((( float ) in_band / numpps) * 100));  /* return the percentage of inband */
}




/*
 * NAME: list_distribution
 *
 * FUNCTION: distribution
 *
 * RETURN VALUE: pointer to array of 5 values -- the distribution of PPs in the 5 intra regions
 */
int *list_distribution (pp_list, numpps, pvsize)
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
 * NAME: strict_allocation
 *
 * FUNCTION: Determine whether or not an allocation is strict for an LV (no LP with more that on PP on any PV).
 *
 * RETURN CODE:  strict == TRUE
 *               non-strict == FALSE
 */
BOOLEAN strict_allocation (lv_data, numpvs)
struct lv_info2
	lv_data[];  /* the LV data (INPUT) */
int
	numpvs;     /* number of PV used by LV, size of lv_data (OUTPUT) */
{
	int
		i, j;               /* loop counters and array indexes */
	BOOLEAN
		strict_flag=TRUE;   /* the allocation is considered stict until found to be otherwise */

	for (i=0; i<numpvs; i++) {  /* for each PV in the LV */
		for (j=1; j<=lv_data[i].max_lp; j++)  /* for every LP in used by the LV */
			if (lv_data[i].lp_flag[j] > 1) {  /* does any LP on this PV have multiple PPs */
				strict_flag = FALSE;  /* if multiple PPs the non-strict */
				break;  /* since known to be non-strict, exit */
			}
	}
	return (strict_flag);
}

/*
 *	lock 'lvdescript's volume group
 */
static void
lock_vg_of_lv(char *lvdescript)
{
	struct CuDv *cudvp;
	char crit[CRITSIZE];
	struct listinfo stat_info;

	odm_initialize();
	odm_set_path(CFGPATH);

	sprintf(crit,"name = '%s'",lvdescript);
	cudvp = (struct CuDv *)odm_get_list(CuDv_CLASS,crit,&stat_info,1,1);

	if ((int)cudvp == ODMERROR) odm_error(odmerrno);

	if (cudvp != NULL && lock_a_vg(Prog, cudvp->parent))
		exit(1);

	odm_terminate();
}

/*
 * NAME: get_vg_id
 *
 * FUNCTION: Translate the vg name to an ascii vgid.  
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
              			sprintf(vgid_ascii,"%16s",cuatp->value);
             		else
			{
				fprintf(stderr,lvm_msg(DEV_NOT_FOUND),Prog,
					lvm_msg(VGINTRO),vgdescript);
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
int NoOfPeriods = 0;

	/* default value for the dvdesc_type */
	id = TRUE;
	tempptr = dvdesc;

	/* test how many periods are embeded in the devdesc */
	while ((*tempptr != '\0'))
	{
		if (*tempptr == '.')
		   NoOfPeriods++;

		tempptr++ ;
	}
	if( (NoOfPeriods > 1) || (!NoOfPeriods) || (NoOfPeriods == 1 &&
		((int)((int) strchr(dvdesc,'.') - (int)dvdesc)) != VGIDLENGTH))	
	/*
	 * valid ID cannot one period, cannot be void of periods, and
	 * must of 16 chars before the ONE period.
	 */
		return(NAME);     

	/* parse the descriptor until the end and */
	/* test if any non-hex characters are found */
	tempptr = dvdesc;
	while ((*tempptr != '\0') && (*tempptr != '.') && id)
	{
		if (!isxdigit((int)*tempptr))
			id = 0;	/* then not id */
		tempptr++;
	}

	/* if id is still possible test the length */
	if (id)
		if ((strlen(dvdesc) == VGIDLENGTH) || (*tempptr == '.')
			|| (strlen(dvdesc) == CUPVPVID))
			return(IDENT);     
		else
			if (strlen(dvdesc) <= DEVNAMESIZE)
				return(NAME);
			else
			{
				fprintf(stderr,lvm_msg(681),Prog);
				return(CHK_FAIL);
			}
	else
		return(NAME);
}

void convert_id_to_name(convert_string)
char	*convert_string;
{
	struct	CuAt *cuatp;
	int	type_flag, temp;

	type_flag = check_desc(convert_string);	

	/* 
	 * if the string is too long (CHK_FAIL) then we will null
	 * out the string.
	 */
	if (type_flag == CHK_FAIL)
		*convert_string = NULL;
	else 
	{
                odm_initialize();
                odm_set_path(CFGPATH);
		if (type_flag == IDENT) {

			cuatp = get_nm_id(convert_string,FALSE,LVSERIAL_ID);
			if (cuatp != NULL)
				strcpy(convert_string, cuatp->name);

		} 
		else if (type_flag == NAME) {

			cuatp = getattr(convert_string,LVSERIAL_ID,FALSE,&temp);
			if (cuatp == NULL)
				*convert_string = NULL;
		}
		odm_terminate();
	}
}
