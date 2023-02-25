static char sccsid[] = "@(#)27  1.7  src/bos/usr/sbin/lvm/intercmd/map_alloc.c, cmdlvm, bos41J, 9507A 2/2/95 17:07:46";

#include <stdio.h>
#include <lvm.h>
#include "allocp.h"
#include "cmdlvm_msg.h"
#include <locale.h>

extern  FILE    *debug;
extern	LVPP	*getnewpps();
extern char     *lvm_msg();
extern char     *Prog="allocp";    
 /* NLS Catalog descriptor for scmc conversion */
extern nl_catd  scmc_catd;        


/*
 COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 FUNCTION: allocp
 ORIGINS: 27

 (C) COPYRIGHT International Business Machines Corp. 1989
 All Rights Reserved
 Licensed Materials - Property of IBM

 US Government Users Restricted Rights - Use, duplication or
 disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

 FILE NAME: best_alloc.c 

 FILE DESCRIPTION: 

 RETURN VALUE DESCRIPTION:

 EXTERNAL PROCEDURES CALLED: 
*/

LVPP *map_alloc(type,numofcopies,size,numofpvs,pv,numlvpps,lvlist,newsize,
	            err_code)
TYPE           type;
int            numofcopies;
int            size; 
int            numofpvs;
PV            *pv[];
int            numlvpps;
struct lvlist *lvlist;
int           *newsize;
int           *err_code;
{
	int i, count;
	RANGE *ptr;
	int numofpps[3]; /* number of pps required for each copy */
	LVPP *layout;
  
	*err_code = 0;

	/*
	 * get layout
	 */
	layout = getnewpps(type, numofcopies, size, lvlist, numofpps);

	/*
	 * compute newsize
	 */
	for (i = 0, *newsize = 0; i < numofcopies; i++)
		*newsize += numofpps[i];
	if (*newsize == 0)
		return(NULL);

	/*
	 * we overload 'intheregion'.  insert() (in mapread.c) used it
	 * to hold the index into the pv table
	 */
#define	the_pvid	intheregion

	/*
	 * walk through the list (pv[0] contains the whole map)
	 */
	count = 0;
	for (ptr = pv[0]->free_range_head; ptr != NULL && count < *newsize;
	     ptr = ptr->next_range)
		/*
		 * assign the entire range...
		 */
		for (i = 0; i < ptr->length && count < *newsize; i++) {
	      		ASSIGN(layout[count].pv_id, pv[ptr->the_pvid]->pvid);
	      		layout[count++].pp_num = ptr->start + i; 
			}

	/*
	 * not enough map entries to fill the request?
	 */
	if (count < *newsize) {
		fprintf(stderr, lvm_msg(ALLOCP_MAPFILELOW),Prog);
		*err_code = -1;
		return (NULL);
		}

	return(layout);
}
