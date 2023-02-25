static char sccsid[] = "@(#)31	1.12  src/bos/usr/sbin/lvm/intercmd/place1copy.c, cmdlvm, bos411, 9428A410j 9/21/93 16:01:21";
/*
 *   COMPONENT_NAME: CMDLVM
 *
 *   FUNCTIONS: compare_capacity
 *		compare_value
 *		if
 *		place1copy
 *		pv_sets
 *		sort_sections
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include  <stdio.h>
#include  <lvm.h>
#include  <allocp.h>

extern FILE  *debug;

/* global min set size,  number of lps, pv capacities,
   used_pvs, safe_lp_count */

int                  M;
int                  best_case_desired_pp_count; /* minimum of M and largest_region_size */

PV                 **gpv;
int                  o[MAXPV]; /* order */
int                 *w; /* minimum of capacity and safe_lp_count */ 
int                 *W; /* global for place1copy.capacity */ 
int                  newlpcount[MAXPV];
LVPP                *glayout;
int                  x[MAXPV];
int                 *ypointers[MAXPV];

static int           min; /* number of pv's in the current best placement */
static int           desired_pp_count_current_best; 
static int           number_of_sections_current_best;

static REGION        desired_region;
static int           available_capacity; /* total capacity of already
                                            used pvs */
extern int increasing();
extern int pvbypv();

RANGE *sort_sections(ptr)

  RANGE   *ptr;
{
  int    swap;
  RANGE *head,*p,*q,*r;

head = ptr;
if (head == NULL) return(head);

do {
    swap = 0;
    if (debug)
      for (p=head; p; p=p->next_range) {
       fprintf(debug,"LENGTH=%d\n", p->length);
       fflush(debug);
      } 

    for (p=head; p->next_range; p=p->next_range) {
 
       q = p->next_range;

       if (debug) {
         fprintf(debug,"in the loop p->length = %d \n",p->length) ;
         if ( q == NULL) fprintf(debug,"1; q is null \n") ;
         if ( p == NULL) fprintf(debug,"1; p is null \n") ;
         fflush(debug);
       }

       if (q->length > p->length) {
           swap = 1;

           if (p == head) { head = q; }
           else { r->next_range = q; }

           if (debug) {
       	     if ( q == NULL) fprintf(debug,"2; q is null \n") ;
       	     if ( p == NULL) fprintf(debug,"2; p is null \n") ;
       	     if ( r == NULL) fprintf(debug,"2; r is null \n") ;
             fflush(debug);
           }
           p->next_range = q->next_range;
           q->next_range = p;
       }
       if (p->next_range == NULL) break;
       r = p;
    }
}
while (swap);

if (debug) {
  fprintf(debug,"Returning from sort_section\n") ;
  fflush(debug);
}

return(head);
}

int compare_capacity(s1,s2)
int *s1, *s2;    /* values for comparison (INPUT) */
{
	if (W[*s1] < W[*s2])
		return(1);
	else 
	if (W[*s1] > W[*s2])
		return(-1);
	return(0);
}

int compare_value(s1,s2)
int *s1, *s2;    /* values for comparison (INPUT) */
{
	if (*s1 < *s2)
		return(1);
	else 
	if (*s1 > *s2)
		return(-1);
        return(0);

} 


/* Finds the sets of pvs with total capacity equal or greater than
   M with minimum number of pvs in the set */

int pv_sets(count,s,k,r) 

   int   count;  /* number of pvs already in the set */
   long  s;      /* total capacity of the pvs already in the set */
   int   k;      /* index of the next pv to consider */ 
   long  r;      /* total capacity of the pvs not in the set yet */
{
   int j,ret;
   int n_of_sections;
   int ppsintheregion; 

x[k] = 1 ; 
count ++ ;

if (debug) {
  fprintf(debug,
     "s=%d r=%d x[%s]=1 count = %d \n",s,r,gpv[o[k]]->asciipvid,count);
  fflush(debug);
}

if ( count <= min) {

  if ((s + w[k]) >= M ) {

     if (count < min) { min = count; }

     if (debug) {
          fprintf(debug,"NEW PV SET: total capacity=%d \n",s+w[k]);
          for (j=0; j<=k; j++) 
              fprintf(debug,"x[%s]=%d \n\n\n",gpv[o[j]]->asciipvid,x[j]);
              fflush(debug);
     }

     /* Replace the current set of physical volumes with a new set */
     /* select the sets of free sections to use on each pv in the 
        current set and place the lps in these sections pvbypv */

     ret = pvbypv(k+1,&n_of_sections,&ppsintheregion,ypointers);


     if ( ppsintheregion > desired_pp_count_current_best ||
          n_of_sections < number_of_sections_current_best ) {

          if ( ppsintheregion > desired_pp_count_current_best)
             desired_pp_count_current_best = ppsintheregion; 

          if(n_of_sections < number_of_sections_current_best ) 
             number_of_sections_current_best = n_of_sections;
     
          if (debug) {
                fprintf(debug,"BETTER PLACEMENT FOUND");
          }
          newplacement(k+1,x,ypointers) ;

          /* if this is the best possible placement search no more */
           if (n_of_sections == 1 && 
                ppsintheregion == best_case_desired_pp_count)  {
				return(1); 
           }
     }

  }
  else {      /* Add one more physical volume to the set of pvs */
     pv_sets(count,s+w[k], k+1, r-w[k]);
  }
}

if ((s+r-w[k] ) >=M ) {
	x[k] = 0; 
        count -- ;
        if (debug) {
            fprintf(debug,"x[%d]=0\n",k);
            fflush(debug);
        }
	pv_sets(count,s,k+1,r-w[k]);
}

return(1);
}


int place1copy (copynum,numofpvs,pv,numoflps,capacity,
                numofranges,freeranges,
                lpcount,copylayout,region,spread,upperbound,
		largest_region_size)

int         copynum;
int         numofpvs;
PV         *pv[];
int         numoflps;
int         capacity[];
int         numofranges[];
RANGE      *freeranges[];
int         lpcount[];
LVPP       *copylayout;
REGION      region;
SPREAD      spread;
int         upperbound;
int         largest_region_size;

{
   int      capacitysum,usedpvcount;
   int      i,j,k,ret_code,done;
   RANGE    *sectionptr;
   int      capacity2[32]; 
   int      n_of_sections, ppsintheregion;
   int      p[MAXPV],temp[MAXPV];

/* pvcount set to number of pvs already in use by this copy 
   equal to the number of nonzero entries i pv is used array */

 min = numofpvs; 
 M=numoflps;
 w = &capacity2[0]; 
 W = &capacity[0]; 	/* need global of capacity for compare_capacity */
 glayout = copylayout ;
 gpv = pv;
 desired_region = region;

 if (largest_region_size > numoflps)  best_case_desired_pp_count = numoflps;
 else best_case_desired_pp_count = largest_region_size;



 for (i=0, j=0, capacitysum=0, available_capacity=0, usedpvcount=0 ; 
        i<numofpvs; i++){
    capacitysum += capacity[i];
    if (lpcount[i] >0 && capacity[i] > 0) {
              usedpvcount++;
              capacity2[j] = capacity[i];
              available_capacity += capacity[i];
              o[j]=i;
              x[j] = 1;
              j++;
    }
 }
 if (capacitysum < M ) return (-1);

 j = usedpvcount;
 i = 0;
 done = 0;
 while ( j<numofpvs ) {
 	x[j] = 0;
 	while(!done && lpcount[i]>0 && capacity[i] > 0) {
               i++;
               if (i==numofpvs) done = 1;
        }
        if (done) break;
 	o[j] = i;
     capacity2[j] = capacity[i];
 	i++;
 	j++;
 }
 if (usedpvcount > 0) {
   for (j=usedpvcount, i=0; j<numofpvs; j++,i++) {
	    temp[i]=j;
   }
   qsort (&temp[0],numofpvs-usedpvcount,sizeof(int),compare_capacity);
   for (i=0; i<numofpvs-usedpvcount; i++) {
	   p[i+usedpvcount]= o[temp[i]];
   }
   for (i=usedpvcount; i<numofpvs; i++) {
	o[i] = p[i];
        /* make sure to sort the capacity2's to match the o's */
	w[i] = capacity[o[i]];
   }
 }
 else {
      qsort (&o[0],numofpvs,sizeof(int),compare_capacity);
      /* make sure to sort the capacity2's to match the o's */
      qsort (&w[0],numofpvs,sizeof(int),compare_value);
 }

 desired_pp_count_current_best = 0;
 number_of_sections_current_best = 0;
 if(debug) fprintf(debug,"numofpvs=%d \n", numofpvs);
 for(i=0; i<numofpvs; i++) { 
    /* Sort the sections according to size */
    pv[i]->free_range_head = sort_sections(pv[i]->free_range_head) ; 
    if(debug) fprintf(debug,"sorted pv[%d] \n", i);
    number_of_sections_current_best += pv[i] -> free_section_count; 
 }
 number_of_sections_current_best++ ;

 /* Allocate space for the section vectors y */
 for(i=0; i<numofpvs; i++) { 
    j=o[i];
    ypointers[j]=(int *)malloc(pv[j]->free_section_count * sizeof(int))	;
 }

 for(i=0; i<numofpvs; i++) { newlpcount[i] = 0; }

 if ( numoflps > available_capacity) 
     ret_code = pv_sets(usedpvcount,(long)available_capacity,
                 usedpvcount,(long)capacitysum-available_capacity);

 else  {
     ret_code = pvbypv(usedpvcount,&n_of_sections,&ppsintheregion,ypointers);
     newplacement(usedpvcount,x,ypointers);
 }

 /* Update the free section lists for all pvs */

 for(i=0; i<numofpvs; i++) {
  free(ypointers[i])	;
  if (newlpcount[i] > 0) {
        j=o[i];
        pv[j]->totallpcount += newlpcount[i];
        pv[j]->lpcount[copynum-1] +=newlpcount[i];
        qsort (pv[j]->lpnums,pv[j]->totallpcount,sizeof(int),increasing);
        sectionptr = pv[j]->free_range_head;

        while (sectionptr) {
           sectionptr->start = sectionptr->newstart;
           sectionptr->length = sectionptr->newlength;
           if (sectionptr->next_range != sectionptr->newnext)
                pv[j]->free_section_count++ ;
           sectionptr->next_range = sectionptr->newnext;
               
           sectionptr = sectionptr->next_range;
       }
  }
 }

if (debug) {
 for(i=0; i<numofpvs; i++) {
 fprintf(debug,"pv[%s].totallpcount=%d ",pv[i]->asciipvid,pv[i]->totallpcount);
 for(j=0; j<3; j++){
    fprintf(debug,"lpcount[%d]=%d ",j,pv[i]->lpcount[j]);
 }
 fprintf(debug,"\n");
 }
 fprintf(debug,"LPNUMS:\n") ;
 for (j=0; j<numofpvs; j++) {
      	 fprintf(debug,"%s: capacity=%d ",pv[j]->asciipvid,capacity[o[j]]);
    	 for (k=0; k<pv[j]->totallpcount; k++) {
              fprintf(debug,"%d ",pv[j]->lpnums[k]);
              if ( (k%10) == 0 ) fprintf(debug,"\n");
         }
         fprintf(debug,"\n");
 }
 fflush(debug);
}

 return(ret_code);
}
