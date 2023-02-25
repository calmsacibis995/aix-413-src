static char sccsid[] = "@(#)39	1.9.4.7  src/bos/usr/sbin/lvm/liblv/lsutil.c, cmdlvm, bos41J, 9523A_all 6/2/95 23:43:33";
/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: get_lv_data, distribution, efficiency_total, efficiency,
 *            number_of_megabytes, get_vgname, get_pvdescript,
 *            get_odm_data_chme, get_vgid, get_pvid, get_lvid, get_odm_data_emy,
 *            get_mtpt_lvid, get_lvnm_lvid, get_attr, get_type_lvid, prt_pvmap
 *
 * ORIGINS: 27
 *
 */

/*
 * FILENAME: lsutil.c
 *
 * FILE DESCRIPTION: Source file for common functions used by the
 *      logical volume list high-level commands (lslv, lspv, lsvg).

 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <lvm.h>
#include <cmdlvm.h>
#include <ls.h>
#include <string.h>
#include <odmi.h>
#include <sys/cfgodm.h>
#include <cmdlvm_msg.h>
#include <fstab.h>

#define ODMERROR -1
#define NAME      1
#define IDENT     0

extern struct CuAt *get_nm_id(), *getattr();
extern char *Prog;
extern convt_pvid_32();


/*
 * NAME: get_lv_data
 *
 * FUNCTION: Convert the querylv map (*copies) into a more convenient form which is organized by PV.  Also gather
 *           or calculate related LV information.  This data structure is defined so that it is easily used in
 *           several different circumstances.
 *
 * RETURN CODE: none
 *
 */
void get_lv_data (lvid, currentsize, copies, lv_data, numpvs, sourcepv)
struct lv_id
	*lvid;                    /* the lvid in binary format (INPUT) */
int
	currentsize;              /* number of LPs in the LV (INPUT) */
struct pp
	*copies[LVM_NUMCOPIES];   /* the old data structure -- the map from querylv (INPUT) */
struct lv_info2
	lv_data[];                /* the new data structure reorganized by PV (OUTPUT) */
int
	*numpvs;                  /* the number of PVs used by the LV (OUTPUT) */
char
	*sourcepv;                /* optional (maybe NULL) pvdescript for LVM query (INPUT) */
{
    int
	    i, j, k;          /* counters and array indexes */
    struct querypv
	    *querypv;         /* structure which contains the PV data from LVM */
    RETURN_CODE
	    get_rc,           /* return code from get_pvdescript */
	    query_rc;         /* return code for LVM query */
    BOOLEAN
	    pv_error = FALSE; /* TRUE if any error has occurred in a call to get_pvdescript */

    BOOLEAN get_pvdescript ();


    for (*numpvs=i=0; i<currentsize; i++) {  /* for each LP in the LV */
	for (k=0; k<LVM_NUMCOPIES; k++) {  /* for each LP copy, 1 to 3 */
	    if (copies[k] != NULL && copies[k][i].lp_num != 0) {  /* if the copy really exist */

		/* is the PV this PP (of the LP) is on already in the data structure? find where */
		for (j=0; j<*numpvs && !SAMEPVS(lv_data[j].pv_id, copies[k][i].pv_id); j++);

		if (j == *numpvs) {  /* if the PV is not already in the data structure, then put it in */
		    lv_data[*numpvs].pv_id.word1 = copies[k][i].pv_id.word1;
		    lv_data[*numpvs].pv_id.word2 = copies[k][i].pv_id.word2;

		    /* allocate storage to hold a list of PPs and a list of LPs */
		    lv_data[*numpvs].pp_list = (int *)getmem(currentsize * LVM_NUMCOPIES * sizeof(int));
		    lv_data[*numpvs].lp_list = (int *)getmem(currentsize * sizeof (int));

		    lv_data[*numpvs].max_lp = currentsize;

		    /* must query PV to find its size -- go ahead and save the PV map for later use */
		    query_rc = (RETURN_CODE) lvm_querypv (&lvid->vg_id, &lv_data[*numpvs].pv_id,
			&querypv, sourcepv);
		    if (query_rc == SUCCESS) {
			lv_data[*numpvs].pvsize = querypv->pp_count;
			lv_data[*numpvs].pvmap = querypv->pp_map;
		    }
		    else {  /* cannot query the pvmap so init size and map field with defaults */
			errmsg (query_rc);  /* print low-level lvm error message */
			if (!pv_error)
			lv_data[*numpvs].pvsize = UNDEFINED_INT;
			lv_data[*numpvs].pvmap = NULL;
			pv_error = TRUE;
		    }

		    /* get the pv descriptor and its format -- id or name */
		    get_rc=get_pvdescript(lv_data[*numpvs].pv_id, lv_data[*numpvs].pvdescript);
		    lv_data[*numpvs].pvdescript_type = (get_rc == SUCCESS) ? name : id;

		    /* allocate array of LP flag used to indicate which LPs are on this PV */
		    lv_data[(*numpvs)++].lp_flag = (BYTE *)get_zeroed_mem(currentsize+1,sizeof(BYTE));
		    lv_data[j].numpps = 0;
		    lv_data[j].numlps = 0;
		}  /* after the if -- we know j points to the current PV */

		lv_data[j].pp_list[lv_data[j].numpps++] = copies[k][i].pp_num;  /* save PP numbers */
		if (lv_data[j].lp_flag[copies[k][i].lp_num]++ == 0) { /* count same LP only once */
		    lv_data[j].lp_list[lv_data[j].numlps++] = copies[k][i].lp_num;
		}
	    }
	}
    }
}




/*
 * NAME: distributions
 *
 * FUNCTION: find the distribution of free and used partitions
 *
 * RETURN VALUE: pointer to array of 5 values -- the distribution of PPs in the 5 intra regions
 */
void distributions (pp_map, pvsize, free_dist, used_dist)
struct pp_map
	pp_map[];  /* list of PPs in PV (INPUT) */
int
	pvsize,     /* size of the PV the PPs belong to (INPUT) */
	free_dist[],
	used_dist[];
{
	int
		i, j;             /* loop counters and array indexes */
	struct REGION
		*regions;         /* the boundaries of the intra regions */

	struct REGION *getregions ();


	regions = getregions(pvsize);  /* calculate the intra region boundaries */

	for (i=0; i<SREGION; i++)  /* for each PP in the list */
		for (free_dist[i]=used_dist[i]=0, j=regions[i].low; j<=regions[i].high; j++)
			if (pp_map[j-1].pp_state == LVM_PPFREE)  /* subtract one since regions count starting at 1 */
				free_dist[i]++;
			else
				used_dist[i]++;
}




/*
 * NAME: efficiency_total
 *
 * FUNCTION: Calculate the total allocation efficiency by adding up the individual allocation efficiency for each PV.
 *
 * RETURN CODE: the allocation efficiency - a percentage (100 is the best and 0 is the worst)
 */
PERCENTAGE efficiency_total (lv_data, numpvs)
struct lv_info2
	lv_data[];  /* LV data (INPUT) */
int
	numpvs;     /* number of PVs used by the LV, also size of lv_data (INPUT) */
{
	int
		i,              /* loop counter and array index */
		total_pps=0;    /* total number of PPs used by the LV */
	long int
		total_efficiency=0;  /* culmulative efficiency value for all the PVs in the LV */

	int efficiency();


	for (i=0; i<numpvs; i++) {  /* for each PV used by the LV */
		total_efficiency += efficiency (lv_data[i].pp_list, lv_data[i].numpps, lv_data[i].pvsize) *
		    lv_data[i].numpps;  /* add in the efficiency for an individual PV */
		total_pps += lv_data[i].numpps;  /* keep track of total PPs to calculate the final percentage */

	}
	return ((int)(total_efficiency / total_pps));  /* return the efficiency percentage for the entire LV */
}




/*
 * NAME: efficiency
 *
 * FUNCTION: Calculate the allocation efficiency of a set of PPs on a single PV.  The best allocation is for the
 *           PPs to be consecutively allocation (e.g. 10, 11, 12, 13).  The worst allocation is for half the PPs
 *           to be on one side of the PV and the other half of the PPs to be on the other size of the PV (1,2,99,100).
 *           The measure used to calculate an efficiency value is the sum of distances from each PP to every other
 *           PP on the PV.  This number is used with the worst case to calculate an efficiency value.  The goal
 *           it to have the best allocation (consecutive PPs) to have a percentage of 100 and the worst (divided
 *           PPs on opposites sides of the PV) to have a value of 0, with all other values being equally distributed.
 *           This function appears to approximately meet that goal.
 *
 * RETURN CODE: allocation efficiency percentage (0-100%)
 *
 */
PERCENTAGE efficiency (pp_list, numpps, pvsize)
int
	pp_list[],   /* list of PPs (INPUT) */
	numpps,      /* number of PPs, size of pp_list (INPUT) */
	pvsize;      /* size of the PV (INPUT) */
{
	int
		i, j, k,        /* loop counters and array indexes */
		current_pp;     /* current PP number */
	long int
		base=0;         /* the minimum distance possible between PPs */
	float
		sum,            /* cumulative sum of the distances */
		cluster_bound;  /* worst distance possible */


	for (i=0; i<numpps; i++)  /* for the number of PPs being examined */
		base += ((numpps-i) * (numpps-(1+i)) / 2) + ((i * (i+1))/2);  /* approximate minimum distance value */

	if (numpps>1)  /* make sure there is at least on PP -- must prevent divide by zero later on */
		cluster_bound = (numpps * (pvsize * numpps / 2)) - base;
	else
		cluster_bound = 1;

	/* calculate the total distance for the real PPs */
	for (sum=j=0; j<numpps; j++) {  /* for all PPs */
		current_pp = pp_list[j];
		for (k=0; k<numpps; k++) /* for all other PPs (note distance to itself is zero so no special case) */
			sum += DISTANCE (current_pp, pp_list[k]);
	}

	sum -= base;  /* adjust by base value so result will be in correct range (i.e. consecutive == 100%) */
	return ((int) ((1.0 - (sum / cluster_bound)) * 100));  /* return the final final percentage */
}




/*
 * NAME: number_of_megabytes
 *
 * FUNCTION: Convert a base two logarithm value to a megabyte value.
 *
 * RETURN VALUE: number of megabytes
 *
 */
int number_of_megabytes(power_of_2)
int power_of_2;  /* the logarithmic value to be converted to megabytes */
{
	long int numbytes;

	numbytes = 1L << power_of_2;  /* find the actual number of bytes the logarithm represents */
	return((int)(numbytes / MEGABYTE));  /* divide number of bytes by megabyte size to get the number of megabytes */
}




/*
 * NAME: get_vgname
 *
 * FUNCTION: Return the vgname for a particular vgid.
 *
 * RETURN CODE: 0 == SUCCESS
 *              non-zero == FAILURE
 */
RETURN_CODE get_vgname (vgid, vgname)
char *vgid,      /* lvid of the LV the data is returned for (INPUT) */
     *vgname;
{
	RETURN_CODE	rc;
	struct CuAt	*cuatp;
	struct listinfo	stat_info;
	char		crit[CRITSIZE],
			path_ptr;

	odm_initialize();
	path_ptr = odm_set_path(CFGPATH);

	if (vgid == NULL)
		return(FAILURE);
  	sprintf(crit,"attribute = 'vgserial_id' and value = '%s'",vgid);
  	cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
  	if ((int)cuatp == ODMERROR)
		odm_error(odmerrno);
  	if (cuatp == NULL)
   	{
    		fprintf(stderr,lvm_msg(ID_NOT_FOUND),Prog,vgid);
    		odm_terminate();
		free(path_ptr);
    		return(FAILURE);
   	}

  	sprintf(vgname,"%s",cuatp->name);
	odm_terminate();
	free(cuatp);
	free(path_ptr);
  	return(SUCCESS);
}


/*
 * NAME: get_pvdescript
 *
 * FUNCTION: Convert pvid to pvname.  
 *
 * RETURN CODE: 0 == SUCCESS (for returning pvname, the pvid is always returned)
 *              non-zero == FAILURE (to return the pvname)
 */
RETURN_CODE get_pvdescript (pvid, pvname)
struct unique_id pvid;
char *pvname;
{
	struct CuAt	*cuatp;
	struct listinfo	stat_info;	/* returns values from odm_get_list */
	char		crit[CRITSIZE],
			pvid_ascii[STRSIZE],
			*path_ptr;

	odm_initialize();
	path_ptr = odm_set_path(CFGPATH);

	sprintf(pvid_ascii,"%.8x%.8x0000000000000000",pvid.word1,pvid.word2);
	sprintf(crit,"attribute = 'pvid' and value = '%s'",pvid_ascii);
	cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
	if ((int)cuatp == -1)
		odm_error(odmerrno);
	if (cuatp == NULL)
	{
		pvid_ascii[16] = '\0';	/* zap trailing 0's */
		fprintf(stderr,lvm_msg(ID_NOT_FOUND),Prog,pvid_ascii);
		strcpy(pvname, pvid_ascii);
		odm_terminate();
		free(path_ptr);
		return(FAILURE);
	}
	else
	{
		sprintf(pvname,"%s",cuatp->name);
		odm_terminate();
		free(cuatp);
		free(path_ptr);
		return(SUCCESS);
	}
}

/*
 * NAME: get_odm_data_chme
 *
 * FUNCTION: Return the ODM data for a particular LV.
 *
 * RETURN CODE: 0 == SUCCESS
 *              non-zero == FAILURE
 */
RETURN_CODE get_odm_data_chme (lvid, odm)
char *lvid;      
struct lv_odm_data2 *odm;   
{
 struct CuAt *cuatp;
 struct CuDv *cudvp;
 struct listinfo stat_info;
 struct fstab *fsstruct;
 char crit[CRITSIZE];
 char lvname[DBNAMESIZE];
 char tempdev[DBNAMESIZE];
 int temp;
 
 odm_initialize();
 odm_set_path(CFGPATH);

 /* Get the volume group name for this logical volume id */ 
 sprintf(crit,"attribute = 'lvserial_id' and value = '%s'",lvid);
 cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
 if ((int)cuatp == ODMERROR) odm_error(odmerrno);
 if (cuatp != NULL) 
  {
   sprintf(lvname,"%s",cuatp->name);           
   sprintf(odm->lvname,"%s",lvname);           
  }
 else 
  {
   fprintf(stderr,lvm_msg(LV_NOT_FOUND),Prog,lvid);
   odm_terminate();	
   return(FAILURE);
  }

 /* after we have the lvname from the id then get the vgname */
 sprintf(crit,"name = '%s'",lvname);
 cudvp = (struct CuDv *)odm_get_list(CuDv_CLASS,crit,&stat_info,1,1);
 if ((int)cudvp == ODMERROR) odm_error(odmerrno);
 if (cudvp != NULL) 
  {
   sprintf(odm->vgname,"%s",cudvp->parent);           
  }
 else
  {
   fprintf(stderr,lvm_msg(LV_NOT_FOUND2),Prog,lvname);
   odm_terminate();	
   return(FAILURE);
  }

 /* Get the label attribute */
 cuatp = getattr(lvname,"label",FALSE,&temp);
 if ((int)cuatp == ODMERROR) odm_error(odmerrno);
 if (cuatp != NULL) 
  {
   sprintf(odm->label,"%s",cuatp->value);
  }
 else
  {
   fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,"label");
   odm_terminate();
   return(FAILURE);
  }

 /* Get the type attribute */
 cuatp = getattr(lvname,"type",FALSE,&temp);
 if ((int)cuatp == ODMERROR) odm_error(odmerrno);
 if (cuatp != NULL) 
  {
   sprintf(odm->type,"%s",cuatp->value);
  }
 else
  {
   fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,"type");
   odm_terminate();
   return(FAILURE);
  }

 /* Get the intra policy attribute */
 cuatp = getattr(lvname,"intra",FALSE,&temp);
 if ((int)cuatp == ODMERROR) odm_error(odmerrno);
 if (cuatp != NULL) 
  {
   sprintf(&odm->intra_policy,"%c",cuatp->value[0]);
  }
 else
  {
   fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,"intra");
   odm_terminate();
   return(FAILURE);
  }

 /* Get the inter policy attribute */
 cuatp = getattr(lvname,"inter",FALSE,&temp);
 if ((int)cuatp == ODMERROR) odm_error(odmerrno);
 if (cuatp != NULL) 
  {
   sprintf(&odm->inter_policy,"%c",cuatp->value[0]);
  }
 else
  {
   fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,"inter");
   odm_terminate();
   return(FAILURE);
  }

 /* Get the upperbound attribute */
 cuatp = getattr(lvname,"upperbound",FALSE,&temp);
 if ((int)cuatp == ODMERROR) odm_error(odmerrno);
 if (cuatp != NULL) 
  {
   odm->upper_bound = atoi(cuatp->value);
  }
 else
  {
   fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,"upperbound");
   odm_terminate();
   return(FAILURE);
  }

 /* Get the strictness attribute */
 cuatp = getattr(lvname,"strictness",FALSE,&temp);
 if ((int)cuatp == ODMERROR) odm_error(odmerrno);
 if (cuatp != NULL) 
  {
   sprintf(&odm->strict,"%s",cuatp->value[0]);
  }
 else
  {
   fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,"strictness");
   odm_terminate();
   return(FAILURE);
  }

 /* Get the copies attribute */
 cuatp = getattr(lvname,"copies",FALSE,&temp);
 if ((int)cuatp == ODMERROR) odm_error(odmerrno);
 if (cuatp != NULL) 
  {
   odm->copies=atoi(cuatp->value);
  }
 else
  {
   fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,"copies");
   odm_terminate();
   return(FAILURE);
  }

 /* Get the relocatable attribute */
 cuatp = getattr(lvname,"relocatable",FALSE,&temp);
 if ((int)cuatp == ODMERROR) odm_error(odmerrno);
 if (cuatp != NULL) 
  {
   sprintf(&odm->relocatable,"%c",cuatp->value[0]);
  }
 else
  {
   fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,"relocatable");
   odm_terminate();
   return(FAILURE);
  }

 /* Get the state attribute */
 cuatp = getattr(odm->vgname,"state",FALSE,&temp);
 if ((int)cuatp == ODMERROR) odm_error(odmerrno);
 if (cuatp != NULL) 
  {
   sprintf(&odm->vgstate,"%c",cuatp->value[0]);
  }
 else
  {
   fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,"state");
   odm_terminate();
   return(FAILURE);
  }

  /* Get the mount point */
  strcpy(tempdev,"/dev/");
  strcat(tempdev,lvname);
  setfsent();
  fsstruct = getfsspec(tempdev);
  if (fsstruct != NULL)
   {
    sprintf(odm->mount_point,"%s",fsstruct->fs_file);
   }
  else
   {
    sprintf(odm->mount_point,"N/A");
   }
  endfsent();
odm_terminate();
return(SUCCESS);
}

/*
 * NAME: get_vgid
 *
 * FUNCTION: Translate the pvdescript to a vgid. 
 *
 * RETURN CODE: 0 == SUCCESS
 *              non-zero == FAILURE
 */
RETURN_CODE get_vgid(pvdescript, vgid_ascii)
char *pvdescript; 
char *vgid_ascii;  
{
	struct CuAt	cuat,
			*cuatp;
	char		crit[CRITSIZE],
			vgname[DBNAMESIZE],
			*path_ptr,
			*pvid,
			longpvid[CUPVPVID];	/* string to hold a 32 pvid */
	int		pv_desc,
			temp;

	odm_initialize();
	path_ptr = odm_set_path(CFGPATH);

	pv_desc = check_desc(pvdescript);

	if (pv_desc == NAME)
	{
		sprintf(crit,"name = '%s' and attribute = '%s'",pvdescript,"pvid");
		if ((int)odm_get_first(CuAt_CLASS,crit,&cuat) > 0)
			pvid = cuat.value;
		else
		{
			fprintf(stderr,lvm_msg(ID_NOT_FOUND),Prog,pvdescript);
			odm_terminate();
			free(path_ptr);
			return(FAILURE);     
		}
	}
	else
	{
		convt_pvid_32(pvdescript,longpvid);
		pvid = longpvid;
	}

	sprintf(crit,"attribute = '%s' and value = '%s'","pv",pvid);

	if ((int)odm_get_first(CuAt_CLASS,crit,&cuat) > 0)
	{
		bzero(vgname,DBNAMESIZE);
		strncpy(vgname,cuat.name,DEVNAMESIZE);
		cuatp = getattr(vgname,"vgserial_id",FALSE,&temp);

		/* if getattr returns ODMERROR then and odm error has occured */
		if ((int)cuatp == ODMERROR)
			odm_error(odmerrno);
		/* if the pointer is not NULL then a match was found */

		if (cuatp != NULL)
			sprintf(vgid_ascii,"%s",cuatp->value);
		else /* else the object was not found, print error and exit */
		{
			fprintf(stderr,lvm_msg(DEV_NOT_FOUND),Prog,"",vgname);
			odm_terminate();
			free(path_ptr);
			return(FAILURE);
		}
		free(path_ptr);
		free(cuatp);
	}
	else
	{
		fprintf(stderr,lvm_msg(PV_NOT_IN_VG),Prog,pvdescript);
		odm_terminate();
		free(path_ptr);
		return(FAILURE);
	}

	odm_terminate();
	return(SUCCESS);
}

/*
 * NAME:  get_pvid
 *
 * FUNCTION: Translate pvdescript to pvid.
 *
 * RETURN CODE: 0 == SUCCESS
 *              1 == FAILURE
 */
RETURN_CODE get_pvid(pvdescript, pvid_ascii, pvdescript_type)
char *pvdescript;
char *pvid_ascii;
PVDESCRIPT_TYPE *pvdescript_type;  
{
	struct CuAt	*cuatp;
	char		pvid[CUPVPVID],
			*path_ptr;
	int		temp;
	int		pv_desc;

	odm_initialize();
	path_ptr = odm_set_path(CFGPATH);

	pv_desc = check_desc(pvdescript);

	if (pv_desc == NAME)
	{
		bzero(pvid,CUPVPVID);
		/* Get the ident from odm */
		cuatp = getattr(pvdescript,"pvid",FALSE,&temp);

		if ((int)cuatp == ODMERROR)
			odm_error(odmerrno);
		if (cuatp != NULL)
		{
			if ((strncmp(cuatp->value,"none",4)) == 0)
			{
				sprintf(pvid_ascii,"0000000000000000");
				*pvdescript_type=name;
			}
			else
			{
				strncpy(pvid_ascii,cuatp->value,16);
				pvid_ascii[16]=0;
				*pvdescript_type=name;
			}
		}
		else
		{
			fprintf(stderr,lvm_msg(DEV_NOT_FOUND),Prog,"",pvdescript);
			odm_terminate();
			free(path_ptr);
			return(FAILURE);
		}

		free(cuatp);
	}   
	else  /* This is already an ident */
	{
		sprintf(pvid_ascii,"%s",pvdescript);
		*pvdescript_type=id;
	}

	odm_terminate();
	free(path_ptr);
	return(SUCCESS);
}


/*
 * NAME: get_lvid
 *
 * FUNCTION: Translate lvdescript to lvid. 
 *
 * RETURN CODE: 0 == SUCCESS
 *              non-zero == FAILURE
 */
RETURN_CODE get_lvid(lvdescript, lvid)
char *lvdescript;
char lvid[];   
{
	struct CuAt	*cuatp;
	int		temp;
	int		lv_desc;
	char		crit[CRITSIZE],
			*path_ptr;
	struct listinfo	stat_info;
 
	odm_initialize();
	path_ptr = odm_set_path(CFGPATH);

	lv_desc = check_desc(lvdescript);

	if (lv_desc == NAME)
	{
		cuatp = getattr(lvdescript,"lvserial_id",FALSE,&temp);
		/* if getattr returns ODMERROR then and odm error has occured */
		if ((int)cuatp == ODMERROR)
			odm_error(odmerrno);

		/* if the pointer is not NULL then a match was found */
		if (cuatp != NULL)
			sprintf(lvid,"%s",cuatp->value);
		else /* else the object was not found, print error and exit */
		{
			fprintf(stderr,lvm_msg(DEV_NOT_FOUND),Prog,"",lvdescript);
			odm_terminate();
			free(path_ptr);
			return(FAILURE);
		}
	}
	else
	{
		sprintf(lvid,"%s",lvdescript);

		sprintf(crit,"value = '%s'",lvdescript);
		cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);

		/* if getattr returns ODMERROR then and odm error has occured */
		if ((int)cuatp == ODMERROR)
			odm_error(odmerrno);

		/* if the pointer is not NULL then a match was found */
		if (cuatp != NULL)
			sprintf(lvdescript,"%s",cuatp->name);
		else /* else the object was not found, print error and exit */
		{
			fprintf(stderr,lvm_msg(DEV_NOT_FOUND),Prog,"",lvdescript);
			odm_terminate();
			free(path_ptr);
			return(FAILURE);
		}
	}

	odm_terminate();
	free(cuatp);
	free(path_ptr);
	return(SUCCESS);
}



/*
 * NAME: get_odm_data_emy
 *
 * FUNCTION: Return ODM data for a particular LV.
 *
 * RETURN CODE: 0 == SUCCESS
 *              non-zero == FAILURE
 */
RETURN_CODE get_odm_data_emy (lv_id, lvname, lvtype, lvmount)
struct lv_id * lv_id; 
char lvname[];   /* the LV name for the lvid (OUTPUT) */
char lvtype[];   /* the LV type for the lvid (OUTPUT) */
char lvmount[];  /* the file system mount point for the lvid, if any (OUTPUT) */
{
 struct CuAt *cuatp;
 struct listinfo stat_info;
 struct fstab *fsstruct;
 char crit[CRITSIZE];
 char lv_id_ascii[STRSIZE];  /* ascii version of the lvid */
 char tempdev[DBNAMESIZE];
 int temp;

 odm_initialize();
 odm_set_path(CFGPATH);

 sprintf(lv_id_ascii, "%.8x%.8x.%d", lv_id->vg_id.word1, lv_id->vg_id.word2,
                                       lv_id->minor_num);  /* get ascii id */

 /* Get the LV name for the lvid. */
 sprintf(crit,"attribute = 'lvserial_id' and value = '%s'",lv_id_ascii);
 cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
 if ((int)cuatp == ODMERROR) odm_error(odmerrno);
 if (cuatp == NULL) 
  {
   fprintf(stderr,lvm_msg(ID_NOT_FOUND),Prog,lv_id_ascii);
   odm_terminate();
   return(FAILURE);
  }
 else
  {
   sprintf(lvname,"%s",cuatp->name);
  }

 /* Get the mount point for the lvid. */
 strcpy(tempdev,"/dev/");
 strcat(tempdev,lvname);
 setfsent();
 fsstruct = getfsspec(tempdev);
 if (fsstruct != NULL)
  {
   /* truncate at STRSIZE and not exceed the string+'\0' */
   sprintf(lvmount,"%.49s",fsstruct->fs_file);
  }
 else
  {
   sprintf(lvmount,"N/A");
  }
 endfsent();

 /* Get the type attribute for the lvname. */
 cuatp = getattr(lvname,TYPEA,FALSE,&temp);
 if ((int)cuatp == ODMERROR) odm_error(odmerrno);
 if (cuatp != NULL) 
  {
   sprintf(lvtype,"%s",cuatp->value);
  }
 else  
  {
   sprintf(lvtype,"%s",UNDEFINED_STR);
  }

odm_terminate();
return(SUCCESS);
}


/*
 * NAME: get_mtpt_lvid
 *
 * FUNCTION: Gets the mount point for the filesystem logical volume 
 *	from the /etc/filesystems file.  Opens the file with
 *	'setfsent', get the structure associated with the /dev
 *	entry, and closes the file with 'endfsent'.
 *
 * RETURN VALUE DESCRIPTION: Prints the mount point to standard out.
 *
 */
void
get_mtpt_lvid(lvid)
char *lvid;
{
struct fstab *fsstruct;
char tempdev[DBNAMESIZE];
struct CuAt *cuatp;

	strcpy(tempdev,"/dev/");
	cuatp = get_nm_id(lvid,FALSE,LVSERIAL_ID);
	strcat(tempdev,cuatp->name);
	setfsent();
	fsstruct = getfsspec(tempdev);
	if (fsstruct != NULL)
		printf("%s\n",fsstruct->fs_file);
	else
		printf("N/A\n");
	endfsent();

}
/*
 * NAME: get_lvnm_lvid
 *
 * FUNCTION: Gets the logical volume name for the logical volume identifier.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 */

void
get_lvnm_lvid(lvid)
char *lvid;
{
	/* print the name to standard out if found */
	/* send the lv type to search for */
	get_nm_id(lvid,TRUE,LVSERIAL_ID);
} 
/*
 * NAME: get_attr
 *
 * FUNCTION: Gets an attribute from the CuAt class.
 *	Prints the attribute if found to standard out.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Does not return a value.
 */

void
get_attr(dvname,attribute)
char *dvname;
char *attribute;
{
struct CuAt *cuatp;
int temp;

	cuatp = getattr(dvname,attribute,FALSE,&temp);
	if ((int)cuatp == ODMERROR)
		odm_error(odmerrno);
	if (cuatp != NULL) 
		printf("%s\n",cuatp->value);
	else {
		fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,attribute);
		odm_terminate();
		exit(OBJNOTFOUND);
	}		
}
/*
 * NAME: get_type_lvid 
 *
 * FUNCTION: Gets the type attribute value for the logical volume identifier.
 *	Gets the name from the CuAt class using the identifier then
 *	gets the type value from the CuAt class using the name.
 *
 * RETURN VALUE DESCRIPTION: Does not return if a match is not found, and
 *	will print error message to standard error.
 *	Will print the value of the type attribute for
 *	the logical volume identifier to standard out.
 */
void 
get_type_lvid(lvid)
char *lvid;
{
struct CuAt *cuatp;
char lvname[DBNAMESIZE];

	cuatp = get_nm_id(lvid,FALSE,LVSERIAL_ID);
	strcpy(lvname,cuatp->name);
	get_attr(lvname,TYPEA);
}




/*
 * NAME: prt_pvmap
 *
 * FUNCTION: Implements the "M" option of the lspv command which prints out
 * pv map (which is used as input to some high level commands) for a single PV.
 *
 * OUTPUT: Data selected by the "M" option is written to standard out. 
 *         Error message written to standard error.
 *
 * RETURN CODE: none -- but function will exit program with non-zero status if fatal error.
 *
 */
void prt_pvmap(pvdescript, vgid_option, sourcepv)
char
	*pvdescript,    /* descriptor of the physical volume to list (INPUT) */
	*vgid_option,   /* vgid in ascii format */
	*sourcepv;      /* optional (maybe NULL) pvdescript for LVM - indicates a particular PV to read info from (INPUT) */

{
	int
	    i;                    /* array counters */
	char
	    *pp_state_str,       /* ascii string representing the state of one PP */
		*copy,
	    pvname[STRSIZE],      /* PV name */
	    lvname[STRSIZE],      /* LV name */
	    lvid_ascii[STRSIZE],  /* string representing lvid */
	    pvid_ascii[STRSIZE],  /* string representing pvid */
	    vgid_ascii[STRSIZE];  /* string representing vgid */
	RETURN_CODE
		query_rc;
	PVDESCRIPT_TYPE
		pvdescript_type;  /* type of the pvdescript being used - id or name */
	struct unique_id
		pvid,             /* pvid */
		vgid;             /* vgid */
	struct querypv
	    *querypv;         /* required structure for querying data from LVM */

    struct CuAt *cuatp;
	RETURN_CODE get_pvdescript(), get_pvid();
        int prev_pp_free,start,count;



prev_pp_free = count = 0;

	if (get_pvid (pvdescript, pvid_ascii, &pvdescript_type) == SUCCESS) {  /* need pvid to query pv */
		get_uniq_id (pvid_ascii, &pvid);  /* parse pvid into binary format */
		if (strcmp (pvdescript, pvid_ascii) == IDENTICAL)
			get_pvdescript(pvid, pvname);  /* translate id to name */
		else 
			strcpy (pvname, pvdescript);   
		if (vgid_option != NULL)  /* if vgid entered on the command line */
			strcpy (vgid_ascii, vgid_option);
		else if (get_vgid(pvid_ascii, vgid_ascii) != SUCCESS  &&  sourcepv == NULL) {  /* get vgid from ODM */
			exit (1);
		}

		get_uniq_id(vgid_ascii, &vgid);  /* convert the ascii vgid into binary format */

		if ((query_rc=lvm_querypv(&vgid, &pvid, &querypv, sourcepv)) == SUCCESS) {
		/* if pv data successfully retrieved */

			for (i=0; i < querypv->pp_count; i++) { /* for each PP of the PV */

					if (querypv->pp_map[i].pp_state != LVM_PPFREE) {
					/* get data on the PP's LV */

                    if (prev_pp_free == 1) { /* end of a free section */
                         if (count > 1) 
                               printf("%s:%d-%d\n",pvname,start,i);
                         else printf("%s:%d\n",pvname,i);

                         prev_pp_free = 0;
                         count = 0;
                    }
						/* convert lvid to string format */
					    sprintf(lvid_ascii, "%08x%08x.%ld", querypv->pp_map[i].lv_id.vg_id.word1, querypv->pp_map[i].lv_id.vg_id.word2, querypv->pp_map[i].lv_id.minor_num);
						/* get the lvname */
	                    cuatp = get_nm_id(lvid_ascii,FALSE,LVSERIAL_ID);
						if (cuatp != NULL) strcpy(lvname,cuatp->name);
						else lvname[0] = '\0';

						if (querypv->pp_map[i].pp_state & LVM_PPSTALE)
						    pp_state_str = "stale"; 
                        else
							pp_state_str = "";

						switch ((int) querypv->pp_map[i].copy) {
							case 1:
							case 2:
								copy = ":1";    /* primary copy */
								break;
							case 3:
							case 4:
								copy = ":2";    /* secondary copy */
								break;
							case 5:
								copy = ":3";    /* tertiary copy */
								break;
							default:             /* value 0 and others */
								copy = "  ";    /* no other copy */
								break;
						}    /* end of switch */
                        
				    printf("%s:%d",pvname,i+1);
				    if (lvname[0] != NULL) {
				        printf("\t%s:%d",lvname,querypv->pp_map[i].lp_num);
				        printf("%s\t%s\n",copy,pp_state_str);
				    }
				    else printf("\n");
               }
		     	else { /* pp is free */
                        if (prev_pp_free) count ++;
                        else {
                            prev_pp_free = 1;
                            count = 1;
                            start = i+1;
                         }
						lvname[0] = '\0';  /* PP is free so no LV */
               }

            }      /* end of for loop */
           if (count > 1) 
               printf("%s:%d-%d\n",pvname,start,i);
           else 
            if (count ==1) printf("%s:%d\n",pvname,i);

		}          /* end of if */ 
		else {  /* cannot query the pv data so print error and leave function */
		       errmsg (query_rc);   /* print low-level lvm error message */
		     }

	}              /* end of if get_pvid successful */
}


/*
 * NAME: check_desc
 *
 * FUNCTION: Checks the descriptor to decide if it is a name or identifier. 
 *	If there is a decimal then it could be a name or an identifier.
 *	If the character string is 16, then it must be an identifier,
 *	otherwise it will be treated as a name.
 */

check_desc(dvdesc)
char *dvdesc;	/* returns the descriptor type (OUTPUT) */
{
char *tempptr;
int id, counter;

	/* default value for the dvdesc_type */
	tempptr = dvdesc;
	counter = 0;

	/* parse the descriptor until the end and */
	/* count how many characters there are    */
	while ((*tempptr != '\0') && (*tempptr != '.'))
	{
		counter++;
		tempptr++;
	}
	if (counter < VGIDLENGTH)
		id = FALSE;
	else
		id = TRUE;	

	/* if id is still possible test the length */
	if (id)
	{
		if ((strlen(dvdesc) == VGIDLENGTH) || (*tempptr == '.')
		    || (strlen(dvdesc) == CUPVPVID))
			return(IDENT);     
		else
		{
			if (strlen(dvdesc) <= DEVNAMESIZE)
				return(NAME);
			else
				fprintf(stderr,lvm_msg(BAD_VGID),Prog);
		}
	}
	else
		return(NAME);
}
