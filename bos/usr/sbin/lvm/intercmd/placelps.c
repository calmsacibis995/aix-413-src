static char sccsid[] = "@(#)26  1.6  src/bos/usr/sbin/lvm/intercmd/placelps.c, cmdlvm, bos41J, 9507A 2/2/95 17:09:46";

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
* FILE NAME: placelps.c 
*
* FILE DESCRIPTION: 
*
* RETURN VALUE DESCRIPTION:
*
* EXTERNAL PROCEDURES CALLED: 
*/

#include  <stdio.h>
#include  <lvm.h>
#include  <allocp.h>

extern FILE   *debug;
extern int     strict;
extern int     o[MAXPV];
extern int    *w;
extern int     M; /* total number of lps to be placed */
extern PV    **gpv;
extern int     newlpcount[MAXPV];
extern LVPP   *glayout;


/* Returns the k th section in the free sections list */

RANGE  *section(k,currentpv)
    int   k;
    int   currentpv;
{
    RANGE  *sectionptr;
    int     j;

    sectionptr = gpv[currentpv]->free_range_head;
    if (k > gpv[currentpv]->free_section_count) return (NULL);
    j=0;
    while (j<k && sectionptr)
    {
       sectionptr = sectionptr-> newnext;
       j++;
    }
    return (sectionptr);
}

/* needs to be replaced with binary search */
int search(value,array,asize)
   int value,*array,asize;
{
    int i;
    for (i=0; i<asize; i++) {
      if (array[i] == value) return(1);
    }
    return(0);

}

/* Returns where to start the allocation of pp's in the section, so that
/* pps allocated will be centered on the desired region 
/* Updates the start and the length of the next section to be used */

updatesection(sectionptr,start,length,whatsleft)

   RANGE         *sectionptr; /* points to the current section */
   int           *start,*length; /* OUTPUT */
   int           whatsleft; /* number of lps left to be allocated */

{
   int     regionstart,sectionlength;
   int     se; /* last ppnum of the section */
   int     re; /* ppnum of last pp in the desired region in the free section */
   int     half,bighalf;
   int     x,r;
   int     a,b;
   int     split; /* = 1 if pps from the center of the free
                     section are taken and the section must be split into
                     two sections */ 
   RANGE  *ptr;

regionstart = sectionptr->intheregion;
re = sectionptr->intheregion + sectionptr->region_length;
sectionlength = sectionptr->newlength;
se = sectionptr->newstart + sectionptr->newlength;
split = 0;

if (whatsleft <= sectionlength && 
        regionstart > 0 && regionstart > sectionptr->newstart) 
{

  *length = whatsleft;

  x = whatsleft - sectionptr->region_length;

  if (x<=0) {
         *start = regionstart;
         sectionptr->newlength = regionstart - sectionptr->newstart;
         sectionptr->intheregion = 0;
         sectionptr->region_length = 0;
         split = 1;
  }
  else /* x > 0 */
  {
      a=regionstart-sectionptr->start;
      b = se - re;
      half = x/2;
      r = (x%2);
      bighalf = half + r;

      if ( (bighalf<=a) && (half<=b) ){
           *start = regionstart - bighalf;
           if (*start !=sectionptr->newstart) split = 1;
      }
      else

      if ( (bighalf<=b) && (half<=a) ){
           *start = re + bighalf - whatsleft;
           if ((*start+ *length) != se) split = 1;
      }

      else   /* region can not be centered */
      {
          if (a<b) {
              *start = sectionptr->start;
              sectionptr->start = *start + *length;
          }
          else   
          if (a>=b) {
              *start = se - whatsleft;
              sectionptr->newlength -= whatsleft;
          }
      }
  }
}
else { /* whatsleft >= sectionlength  || regionstart ==0 
          ||  regionstart == sectionptr->newstart */
         *start = sectionptr->newstart;
         *length = (sectionptr->newlength>whatsleft) ? whatsleft :
                    sectionptr->newlength;
         sectionptr->newlength -= *length ;
         if (sectionptr->newlength <=0) sectionptr->newlength = 0;
         sectionptr->newstart += *length;
}

if (split) {
  /* section is split into two sections,ptr points to the new section */
  ptr = (RANGE *)malloc(sizeof(RANGE));
  sectionptr->newlength = *start - sectionptr->newstart;
  ptr->newlength = se - *start - *length ;
  ptr->start = *start + *length;
  ptr->newstart = ptr->start;
  ptr->intheregion=0;
  ptr->region_length=0;
  ptr->next_range=sectionptr->next_range;
  ptr->newnext = sectionptr->next_range;
  sectionptr->newnext = ptr;
}

}

/* Assigns pps to the lps */
int 
placelps(numofsections,y,C,currentpv,pvindex) 
       int numofsections;
       int *y;
       int C;
       int currentpv;
       int pvindex;
{
       LVPP   *ptr;
       int     i,j,k,jj;
       RANGE  *sectionptr;
       int     count2;
       int     regionstart,regionlength;
       int     samepv;
       int     start,length,whatsleft;
       int     yindex;
       
/* Find the first section to be allocated */
for (j=0; j<numofsections; j++){ if (y[j]) break ; }
yindex = j;
if (yindex == numofsections) return;

newlpcount[pvindex] = 0;

/* currentpv is the index of the current pv into the pv array */
sectionptr = gpv[currentpv]->free_range_head;

/*  Reset the start and length of the free sections to their original values */
while (sectionptr) {
                sectionptr->newstart  = sectionptr->start;
                sectionptr->newlength = sectionptr->length;
                sectionptr->newnext   = sectionptr->next_range;
                sectionptr            = sectionptr->next_range;
}

if (debug) {
  fprintf(debug,"PV=%s \n",gpv[currentpv]->asciipvid);
  if ( gpv[pvindex]->free_range_head != NULL){
       sectionptr = gpv[currentpv]->free_range_head;
       while (sectionptr) {
          fprintf(debug,"start=%d, length=%d  region=%d \n", sectionptr->start,
             sectionptr->length,sectionptr->region_length);
          sectionptr = sectionptr->next_range;
       }
  } 
}

sectionptr = section(yindex,currentpv);

whatsleft = C; /* number of lps that need pps */
updatesection(sectionptr,&start,&length,whatsleft);

/* assign pps to all lps of the current copy */
for(ptr=glayout, count2=0; count2 < M ; count2++,ptr++) { 

    if ( sectionptr ){
         if( !strict || !search(ptr->lp_num,gpv[currentpv]->lpnums,
                            gpv[currentpv]->totallpcount)){
               samepv=0;
               if SAMEPV(ptr->pv_id,gpv[currentpv]->pvid) samepv = 1;

               /* some lps may have pps assigned on the current pv 
                  when the current pv was in another set of pvs 
                  if this is the case, or the lp was never
                  assigned a pp (pp_num=-1) */

               if (samepv || ptr->pp_num==-1) {
                  if (!samepv) ASSIGN(ptr->pv_id,gpv[currentpv]->pvid);
                  if (debug) {
                        fprintf(debug,"lpnum=%d to %s %d\n",
                        ptr->lp_num,gpv[currentpv]->asciipvid, start);
                        fflush(debug);
                 }

                 whatsleft --;
                 /* assign a new pp_num */
                 ptr->pp_num = start;
                 start++;
                 length --;
                 /* add this lpnum to the current pv's lpnums array */
                 gpv[currentpv]->lpnums[gpv[currentpv]->
                           totallpcount+newlpcount[pvindex]]=ptr->lp_num;
                 newlpcount[pvindex] ++;

                 if (length == 0) { /* no more pps left in this section */
                   j++;
                   while ( j< numofsections) {
                        if (y[j]) break;
                        j++;
                   }
                   if (j == numofsections) return;
                   /* get the next section */
                   sectionptr=section(j,currentpv);
                   updatesection(sectionptr,&start,&length,whatsleft);
                 }
               }
        }
     }
    else break;
}

if (debug) {
  fprintf(debug,"%s LPNUMS:", gpv[currentpv]->asciipvid) ;
  for (k=0; k<gpv[currentpv]->totallpcount+newlpcount[pvindex]; k++) {
              fprintf(debug,"%d ",gpv[currentpv]->lpnums[k]);
  }
  fprintf(debug,"\n");
  }
}

int 
newplacement(n,x,ypointers)
 int n, *x ;
 int *ypointers[];
{
 int    j,numofsections;
 int    pvindex,currentpv,*y,C;
 LVPP   *ptr;
 int    l;

/* No lp is assigned a pp yet */
for(ptr=glayout,l=0; l < M ; ptr++,l++)  ptr->pp_num = -1; 

for (j=0; j<n; j++) {
  newlpcount[j] = 0;
  if (x[j]) {
	pvindex = j;
	currentpv = o[j];
        C = w[j];
        y = ypointers[currentpv];
	numofsections = gpv[currentpv]->free_section_count;
        placelps(numofsections,y,C,currentpv,pvindex) ;
  }
}
}
