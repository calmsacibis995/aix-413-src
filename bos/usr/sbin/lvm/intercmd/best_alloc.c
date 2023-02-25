static char sccsid[] = "@(#)25	1.8  src/bos/usr/sbin/lvm/intercmd/best_alloc.c, cmdlvm, bos411, 9428A410j 5/22/92 12:56:36";

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
* FILE NAME: best_alloc.c 
*
* FILE DESCRIPTION: 
*
* RETURN VALUE DESCRIPTION:
*
* EXTERNAL PROCEDURES CALLED: 
*/


#include <stdio.h>
#include <lvm.h>
#include "allocp.h"
#include "cmdlvm_msg.h"
#include <locale.h>

extern  FILE    *debug;
extern LVPP	*getnewpps();
extern char     *lvm_msg();
extern char     *Prog="allocp";    
extern nl_catd  scmc_catd; /* NLS Catalog descriptor for scmc conversion */

extern int             strict;

/*
 * NAME: increasing
 *
 * FUNCTION: Used by qsort to compare two integers.  
 *           Will cause sort in ascending order.
 *
 * RETURN CODE: -1 : first number is less than second
 *               0 : first number is equal to second
 *               1 : first number is greater than second
 */

int increasing(s1,s2)
int *s1, *s2;    /* values for comparison (INPUT) */
{
	if (*s1 < *s2)
		return(-1);
	else 
	if (*s1 > *s2)
		return(1);
	return(0);
}

#define	min(a, b)	((a) < (b) ? (a) : (b))

int
markregion(numofpvs,pv,region)
   int         numofpvs;
   PV          *pv[];
   REGION      region;
{
   int    k,re,rs,rl;
   int    largest_region_size;  
   RANGE  *rangeptr;

largest_region_size = 0;
for (k=0; k<numofpvs; k++) {
     getregion(&(pv[k]->region_start),&(pv[k]->region_length),
        pv[k]->highestppnum,region);
     /* mark the free sections in the desired region */
     rs = pv[k]->region_start;
     rl = pv[k]->region_length;
     re = rs + rl;
if (debug) {
       fprintf(debug,"region_start=%d, region_lenght= %d \n",pv[k]->region_start,pv[k]->region_length);
}
     for (rangeptr= pv[k]->free_range_head; rangeptr; rangeptr= rangeptr->next_range) {
	 rangeptr->intheregion = 0;
         rangeptr->region_length = 0; 
	/*
	 * case 1: start of range falls within region
	 */
	if (rs <= rangeptr->start && rangeptr->start < re) {
		rangeptr->intheregion = rangeptr->start;
		rangeptr->region_length =
			min(rangeptr->length, re - rangeptr->start);
		}
	/*
	 * case 2: start of region falls within range
	 */
	else if (rangeptr->start <= rs &&
		 rs < rangeptr->start + rangeptr->length) {
		rangeptr->intheregion = rs;
		rangeptr->region_length =
			rangeptr->start + rangeptr->length - rs;
		}

	 if (rangeptr->region_length > largest_region_size) 
			largest_region_size = rangeptr->region_length;

     }
}

return(largest_region_size);
}

/*
 * NAME: best_alloc
 *
 * FUNCTION:    Given the current state of the pvs and the
 *              lv to be extended, best_alloc decides on which
 *              pps to use.
 *
 * RETURNS                                             
 */

LVPP *best_alloc(type,numofcopies,strictness,size,numofpvs,pv,numlvpps,lvlist,
                           region,spread,upperbound,newsize,err_code)
   TYPE           type;
   int            numofcopies;
   int            strictness;
   int            size; 
   int            numofpvs;
   PV            *pv[];
   int            numlvpps;
   struct lvlist *lvlist;
   REGION         region;
   SPREAD         spread;
   int            upperbound;
   int           *newsize;
   int           *err_code;
{
   LVPP         *layout;
   LVPP         *copylayout; /* copy pointers into layout */
   int           numofpps[3]; /* number of pps required for each copy */
   int           lpcount[32];  /* no of pps used by this copy on each pv */
   int           capacity[32];
   RANGE        *freeranges[32],*rangeptr,*ptr;
   int           numofranges[32];
   int           i,k,j;
   LVMAP        *lpptr;
   int           highestlpnum,numoflps;
   int           ret;
   int           copynum;
   int           freepps;
   int           largest_region_size;

*err_code = 0;
strict = strictness;
layout = getnewpps(type,numofcopies,size,lvlist,numofpps);

if (debug){
       for(k=0,i=0; k<numofcopies; k++) {
            i += numofpps[k];
       }
       for(k=0; k<i; k++) {
           fprintf(debug,"layout[%d].lpnum=%d  ",k,layout[k].lp_num);
           if ( ((k+1)%3) == 0 ) fprintf(debug,"\n");
      }
      fprintf(debug,"\n");
      fflush(debug);
}

for ( k=0,*newsize=0 ; k<numofcopies; k++ ) { *newsize += numofpps[k]; }
if (*newsize == 0)  /* nothing needs to be allocated */
   return(NULL);

for (k=0,freepps=0; k<numofpvs; k++) {
           freepps += pv[k]->free_pp_count ;
}

if ( *newsize > freepps ) {
      fprintf(stderr, lvm_msg(ALLOCP_NORESOURCE),Prog);
      *err_code = -1;
      return (NULL);
}

for(k=0,highestlpnum=0; k<numofcopies; k++) {
      if (highestlpnum<lvlist[k].max_lp_num) highestlpnum=lvlist[k].max_lp_num;
}
highestlpnum += size;


for (k=0; k<numofpvs; k++) {
     pv[k]->lpnums = (int *) malloc( (pv[k]->totallpcount+
                                              (*newsize)) * sizeof(int));
     pv[k]->totallpcount=0;
}
   

for (k=0 ; k<numofcopies; k++){
      if(lvlist[k].head != NULL)  {
        for (lpptr= lvlist[k].head; lpptr; lpptr=lpptr->next){
                    j = lpptr->pvindex;
                    pv[j]->lpnums[pv[j]->totallpcount] = lpptr->lpnum;
                    pv[j]->totallpcount++;
        }
      }
}

for (j=0; j<numofpvs; j++) {
      pv[j]->org_lpcount = pv[j]->totallpcount;
      qsort (pv[j]->lpnums,pv[j]->totallpcount,sizeof(int),increasing);
}

if (debug) {
   fprintf(debug,"EXISTING LPNUMS PER PV:\n") ;
   for (j=0; j<numofpvs; j++) {
       fprintf(debug,"%s:",pv[j]->asciipvid);
       for (k=0; k<pv[j]->totallpcount; k++) {
              fprintf(debug,"%d ",pv[j]->lpnums[k]);
       }
       fprintf(debug,"\n");
   }
   fflush(debug);
}
   
   /* lp_count must be cummulative MISSING */

if(type==EXTEND && numofpps[0] > 0 ) {
         numoflps = numofpps[0];
         copynum=1;
         for (i=0; i<numofpvs; i++) {
           pv[i]->safelpcount = numofpps[0] ;
           /* copy 1 of all new lpnums */
           lpcount[i] = pv[i]->lpcount[0];
         }
}
else
if(type == COPY || type == EXPLICIT) {
         for (i=0; i< numofcopies; i++) 
                 if (numofpps[i] > 0)  {
                        numoflps = numofpps[i]; 
                        copynum = i+1;
                        break;
                 }
         for (i=0; i<numofpvs; i++) {
             pv[i]->safelpcount = 
                         (numofpps[copynum-1]>highestlpnum)?
                                   numofpps[copynum-1]:highestlpnum  ;
               if (strict) {
                   for (j=0; j<copynum; j++)  {
                         pv[i]->safelpcount -= pv[i]->lpcount[j]  ;
                   }
               }
             /* if there is no pps for the current copy yet */
             lpcount[i] = pv[i]->lpcount[copynum-2];
         }
}

for (i=0; i<numofpvs; i++) {
      capacity[i] = (pv[i]->safelpcount > pv[i]->free_pp_count) ? 
                        pv[i]->free_pp_count : pv[i]->safelpcount; 
}

   copylayout = layout;
   ret = 1;

  while (  copynum <= numofcopies && ret == 1 )
  {

	largest_region_size = markregion(numofpvs,pv,region);

	if (debug) {
 	fprintf (debug,"BEFORE place1copy \n") ; 

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
 	fprintf (debug,"end BEFORE place1copy \n") ; 
	}

   ret=place1copy (copynum,numofpvs,pv,numoflps,capacity,
                     numofranges,freeranges,lpcount,copylayout,
                     region,spread,upperbound,largest_region_size);

  copynum++;
  if (copynum <= numofcopies && ret == 1 ) {
      if (debug) {
         fprintf(debug,"COPYNUM=%d\n",copynum);
      }
      copylayout += numofpps[copynum-2];
      numoflps = numofpps[copynum-1];

      for (i=0; i<numofpvs; i++) {
        if (type == EXTEND) {
              pv[i]->safelpcount = numofpps[copynum-1]  ;
              if (strict) {
                 for (j=0; j<copynum; j++)  {
                         pv[i]->safelpcount -= pv[i]->lpcount[j]  ;
                 }
                 pv[i]->safelpcount += pv[i]->org_lpcount;
              }
        }
        else { /* COPY or EXPLICIT */
               pv[i]->safelpcount= 
                         (numofpps[copynum-1]>highestlpnum)?
                                   numofpps[copynum-1]:highestlpnum  ;

               if (strict) {
                   for (j=0; j<copynum; j++)  {
                         pv[i]->safelpcount -= pv[i]->lpcount[j]  ;
                   }
               }
        }

        capacity[i] = (pv[i]->safelpcount > pv[i]->free_pp_count) ? 
                          pv[i]->free_pp_count : pv[i]->safelpcount; 

        /* lpcount info is used to minimize the number of pvs used */
        /* that is pvs with lpcount > 0 are explored first */

        lpcount[i] = pv[i]->lpcount[copynum-1];
        if (debug) {
            fprintf(debug,"pv[%s].capacity= %d \n",pv[i]->asciipvid,
                  capacity[i]);
        }
      }
    }
    if (ret != 1) {
      fprintf(stderr, lvm_msg(ALLOCP_NORESOURCE),Prog);
      layout = NULL;
      *err_code = -1;
      break;
    }
  }

   return(layout);
}
