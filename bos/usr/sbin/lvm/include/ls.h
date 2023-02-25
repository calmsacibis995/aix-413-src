/* @(#)19       1.6  src/bos/usr/sbin/lvm/include/ls.h, cmdlvm, bos411, 9428A410j 3/24/94 11:18:44 */

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
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
 * FILENAME: ls.h
 *
 * FILE DESCRIPTION: Include file for logical volume list commands.
 */

#define UNDEFINED_STR "???????"   /* default value for undefined string variable */
#define UNDEFINED_INT -999        /* default value for undefined int variable */
#define UNDEFINED_CHR '\0'        /* default value for undefined char variable */

#define IDENTICAL 0
#define NO_COLUMNS 0              /* singles no new columns have begun to be displayed with the "p" option */
#define FIRST_TIME_THROUGH_LOOP 0

#define CMDLVM_PPSTALE 3          /* LVM_PPFREE + LVM_PPSTALE -- for each comparision */
#define CMDLVM_LVSTALE 3          /* LVM_LVDEFINED + LVM_LVSTALE  -- for each comparision */

typedef enum { name, id } PVDESCRIPT_TYPE;
typedef int PERCENTAGE;


struct lv_info2  /* Used to store LVM data in a more convenient form for manipulation and calculation.  Each struct element
		   stores information for the PPs/LPs contained on ONE PV in the LV. */
{
	char pvdescript[STRSIZE];         /* pvdescript from command line */
	PVDESCRIPT_TYPE pvdescript_type;  /* type of pvdescript - pvid or pvname */
	struct unique_id pv_id;           /* pvid */
	int pvsize;                       /* total PPs in the pv */
	int *pp_list;                     /* list for this pvid the PPs that belong to the LV */
	int numpps;                       /* number of PPs in pp_list */
	int *lp_list;                     /* list (non-ordered) of LPs for PPs in pp_list */
	int numlps;                       /* number of LPs in lp_list */
	BYTE *lp_flag;                    /* list of flags to indicate if a LP is on pv - one flag for each possible LP */
	int max_lp;                       /* maximum LP number used in lv, also size of lp_flag list */
	struct pp_map *pvmap;             /* the querypv map for the PV */
};


struct lv_odm_data2 {  /* used to store lv data retrieved from ODM */
	char type[STRSIZE];               /* LV type */
	char label[LABELLENGTH];               /* LV label */
	char vgname[STRSIZE];             /* VG name the LV is contained in */
	char mount_point[STRSIZE];        /* filesystem mount point of LV -- if one exist */
	char lvname[STRSIZE];             /* name of LV */
	char vgstate;                     /* state of VG */
	char intra_policy;                /* intra-PV allocation policy for the LV */
	char inter_policy;                /* inter-PV allocation policy for the LV */
	int upper_bound;                  /* PV upper_bound for allocation policy */
	char strict;                      /* allocation strictness value */
	int copies;                       /* number of copies allocated for the PV */
	char relocatable;                 /* LV relocation flag */
};
