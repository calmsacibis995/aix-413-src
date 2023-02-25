static char sccsid[] = "@(#)30	1.3  src/bos/usr/sbin/lvm/intercmd/getnewpps.c, cmdlvm, bos411, 9428A410j 5/30/91 15:41:10";

/*
* COMPONENT_NAME: (cmdlvm) Logical Volume Commands
* FUNCTION: allocp
* ORIGINS: 27
*
* (C) COPYRIGHT International Business Machines Corp. 1989
* All Rights Reserved
* Licensed Materials - Property of IBM
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
* FILE NAME: getnewpps.c 
*
* FILE DESCRIPTION: 
*
* RETURN VALUE DESCRIPTION:
*
* EXTERNAL PROCEDURES CALLED: 
*/

/*
 * NAME: getregions
 *
 * FUNCTION:  Calculates the five region boundaries for a physical volume.
 *
 * RETURN VALUE: pointer to struct containing starting and ending pp for each region
 *
 */

#include <stdio.h>
#include <lvm.h>
#include "allocp.h"

#define SREGION 5

int getregion(start,length,numpps,region)
   int  *start,*length,numpps;
   REGION region;
{
	static struct {
             int   high;
             int low;
        }
        regions[SREGION];  /* the pp bounds for each region (OUTPUT) */

	int i, low = 1;                  /* index and counter */
	static int count[SREGION];       /* holds number of pps in each region */
	static int extra_index[SREGION] = { 0, 4, 1, 3, 2   };  /* defines assignment order for extra pps */

	/* Find number of partitions in each region. */
	/* first, find the number that evenly fits all the regions */
	for (i=0; i<SREGION; i++)
		count[i] = numpps / SREGION;
	/* second, add any remaining partitions using defined order for adding extras */
	for (i=0; i<(numpps % SREGION); count[extra_index[i++]]++);
	/* Using the number of pps in each region, find the lowest pp number and
	   the highest for each region. */
	for (i=0; i<SREGION; i++) {
		regions[i].low = low;
		regions[i].high = (low += count[i]) - 1;
	}

   switch(region){
     case(OUTEREDGE):
                    *start = regions[0].low;
                    *length = regions[0].high - regions[0].low + 1;
                    break;
     case(OUTERMIDDLE):
                    *start = regions[1].low;
                    *length = regions[1].high - regions[1].low + 1;
                    break;
     case(CENTER):
                    *start = regions[2].low;
                    *length = regions[2].high - regions[2].low + 1;
                    break;
     case(INNERMIDDLE):
                    *start = regions[3].low;
                    *length = regions[3].high - regions[3].low + 1;
                    break;
     case(INNEREDGE):
                    *start = regions[4].low;
                    *length = regions[4].high - regions[4].low + 1;
                    break;
  }
}


extern FILE *debug;

LVPP *getnewpps(type,numofcopies,size,lvlist,numofpps)
   TYPE           type;
   int            numofcopies;
   int            size; 
   struct lvlist *lvlist;
   int           *numofpps;
{
   int     maxlpnum,k,j,l,prevlpnum;
   int     newsize;
   LVPP   *layout;
   LVMAP  *ptr;

   maxlpnum = 0;
   for (k=0 ; k<numofcopies; k++){
          if(lvlist[k].max_lp_num > maxlpnum) maxlpnum = lvlist[k].max_lp_num;
   }
   if (debug) fprintf(debug,"maxlpnum in getnewpps is %d",maxlpnum) ;

   if (type==EXTEND) {
        newsize = size * (numofcopies) ;
        for (k=0 ; k<numofcopies; k++){ numofpps[k] = size; }
        layout = (LVPP *)malloc(newsize * sizeof(LVPP));

        for (k=0; k< newsize ; k +=size) {
          for(j=1; j<=size; j++) {
               layout[k+j-1].lp_num = maxlpnum+j;
               layout[k+j-1].pp_num = -1;
          }
        }
   }
   else
   if (type==COPY || type==EXPLICIT) {

        newsize = 3 * maxlpnum;
        layout = (LVPP *)malloc(newsize * sizeof(LVPP));

        for (k=0,l=0; k<numofcopies; k++){ 
           if (lvlist[k].num_of_lps == 0) {
                     numofpps[k] = maxlpnum; 
                     for(j=1; j<=maxlpnum; j++) {
                 	layout[l].lp_num = j;
                 	layout[l].pp_num = -1;
                        l++;
                     }
           }
           else {
              numofpps[k] =0;
              prevlpnum = 0;
              for (ptr=lvlist[k].head; ptr; ptr=ptr->next) {
                 if (ptr->lpnum > prevlpnum+1) {
                      for (j=prevlpnum+1; j<ptr->lpnum; j++) {
                 	layout[l].lp_num = j;
                 	layout[l].pp_num = -1;
                        numofpps[k] ++ ;
                        l++;
                      }
                 }
                 prevlpnum = ptr-> lpnum;
              }
              for (j = prevlpnum+1; j<= maxlpnum; j++) {
                 	layout[l].lp_num = j;
                 	layout[l].pp_num = -1;
                        l++;
                        numofpps[k] ++ ;
              }
           }
        }
   }
   return (layout);
}

