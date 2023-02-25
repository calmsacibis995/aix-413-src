static char sccsid[] = "@(#)32	1.6  src/bos/usr/sbin/lvm/intercmd/pvbypv.c, cmdlvm, bos411, 9428A410j 6/11/91 11:09:21";

#include  <stdio.h>
#include  <lvm.h>
#include <allocp.h>

extern FILE  *debug;
int           strict;

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
* FILE NAME: pvbypv.c 
*
* FILE DESCRIPTION:  Places the logical partitions on physical volumes.
*
* RETURN VALUE DESCRIPTION:
*
* EXTERNAL PROCEDURES CALLED:  None
*/

extern RANGE  *section();
extern int     M; /* total number of lps to be placed */
extern PV    **gpv;
extern int     o[MAXPV];
extern int     compare_value();
extern int    *w;
extern int     newlpcount[MAXPV];
extern LVPP   *glayout;
extern int     x[MAXPV];

static int    *y;
static int    *besty;
static int    *c;
static int     C;
static int     currentpv;
static int     pvindex;

static int     min;    /* number of sections in the 
                          current best solution */
static int     max;    /* number of pps in the desired region in 
                          the current best solution */

int   
sectionsets(s,k,r,intheregion,count) 

   long  s;  /* total number of pps in the current set */
   int   k;  /* next section to add to the section set */
   long  r;  		/* total number of free pps for section k+1 to the last section */
   int   intheregion;   /* number of lps placed in the desired region */
   int   count;         /* number of sections the lps fall into */
{
   int j;
   int regionsize;


/*  Put the k th section into the sections set */
y[k] = 1 ; 
/* increment number of sections in the set */
count ++ ;

/* update the count of pps in the desired region */
regionsize = section(k,currentpv)->region_length; 
if ((C - s) < regionsize) regionsize = C - s; 
intheregion += regionsize;

if(debug) {
 fprintf(debug,"s=%d r=%d k=%d count=%d \n",s,r,k,count);
 fprintf(debug,"intheregion=%d max=%d \n",intheregion,max);
 fflush(debug);
}

if ((s + c[k]) >= C ) { /* the selected sections have enough capacity to 
                               hold all lps */
      if ((count == min && intheregion > max) || count < min) {
	/* same size section set but has more pps in the region */
	/* or smaller set of sections that holds all lps */
	if (debug) {
              fprintf(debug, "New section set: sum = %d",s+c[k]);
              for (j=0; j<=k; j++) fprintf(debug,"y[%d]=%d \n",j,y[j]);
              fprintf(debug,"\n\n");
              fflush(debug);
	}
	if (intheregion > max) max = intheregion;
	if (count < min) min = count; 

	/* save this set */	
        for (j=0; j<=k; j++) besty[j] = y[j];
      }
}
else {  /* not enough capacity  put one more section into the set */
       sectionsets(s+c[k], k+1, r-c[k],intheregion,count);
}

if ((s+r-c[k] ) >=C ) { /* there is enough no. of free pps on this pv 
                           without using this section   */
	/* take the current section out */
	y[k] = 0; 
	count -- ;
	intheregion -= regionsize;
	if (debug) {
		fprintf(debug,"y[%d]=0\n",k);
		fflush(debug);
	}
	sectionsets(s,k+1,r-c[k],intheregion,count);
}
}


int pvbypv(pvsetsize,n_of_sections,pps_in_the_region,bestypointers)

	int  pvsetsize;		 /* no of pvs */
	int *n_of_sections;      /* no of sections the lv is split into */
	int *pps_in_the_region;  /* no of pps of the lv that are placed in
					the desired region */
        int *bestypointers[];
{
      int  ysize,j,l,totalc;
      RANGE *sectionptr;
      LVPP  *ptr; 
      int   lpsplaced;

*n_of_sections = 0;
*pps_in_the_region = 0;

for (j=0,lpsplaced=0; j<pvsetsize ; j++) {

        sectionptr = gpv[o[j]]->free_range_head;
        gpv[o[j]]->free_section_count = 0; 

        /* Reset the section starts end lengths */
        while (sectionptr) {
                sectionptr->newstart = sectionptr->start;
                sectionptr->newlength = sectionptr->length;
                sectionptr->newnext = sectionptr->next_range;
                sectionptr = sectionptr->next_range;
                gpv[o[j]]->free_section_count ++; 
        }

     if (x[j]) { /* if o[j] th pv is in the pvs selected */
        pvindex = j;
        currentpv = o[j];
        ysize=gpv[o[j]]->free_section_count ;

        /* initial minimum number of sections needed */
        min = ysize + 1; 
        max=0;

        y = (int *) malloc(ysize * sizeof(int) );
        c = (int *) malloc( ysize * sizeof(int) );

        besty = bestypointers[o[j]] ; 

        sectionptr = gpv[o[j]]->free_range_head;
        for (l=0,totalc=0; l<ysize  ; l++,sectionptr= sectionptr->next_range) { 
          c[l] = sectionptr->length;
          totalc += c[l];
        } 
      
        for (l=0; l<ysize ; l++)  y[l] = 0;  

        /* C is the number of pps the current pv must provide */
        C = w[j];

        /* M is the total number of lps to be placed */
        if (C > (M - lpsplaced)) C=M-lpsplaced;
        if (C > 0)  {
		sectionsets(0,0,totalc,0,0);
		*n_of_sections += min;
		*pps_in_the_region += max ;
	}
        lpsplaced += C;
        free(y);
        free(c);
     }
}

for (j=0,lpsplaced=0; j<pvsetsize ; j++) {

        sectionptr = gpv[o[j]]->free_range_head;
        gpv[o[j]]->free_section_count = 0; 

        /* Reset the section starts end lengths */
        while (sectionptr) {
                sectionptr->newstart = sectionptr->start;
                sectionptr->newlength = sectionptr->length;
                sectionptr->newnext = sectionptr->next_range;
                sectionptr = sectionptr->next_range;
                gpv[o[j]]->free_section_count ++; 
        }
}

return(1);
}
