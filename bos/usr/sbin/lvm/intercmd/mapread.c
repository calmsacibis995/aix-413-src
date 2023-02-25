static char sccsid[] = "@(#)29  1.9  src/bos/usr/sbin/lvm/intercmd/mapread.c, cmdlvm, bos41J 5/25/95 10:27:02";

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
#include <string.h>
#include "lvm.h"
#include <allocp.h>
#include <locale.h>
#include "cmdlvm_msg.h"
#include <odmi.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

extern int odm_initialize();
extern int odm_terminate();
extern struct CuAt *getattr();
extern char *odm_set_path(); 


extern FILE *debug;
extern char *Prog;          /* program name */
extern nl_catd  scmc_catd;  /* NLS Catalog descriptor for scmc conversion */
extern char  *lvm_msg();


#define ODMERROR -1
#define PVATTR "pvid"
#define ZEROPV "0000000000000000"
#define PVNONE "none"
#define CFGPATH "/etc/objrepos"
#define OBJNOTFOUND 3
#define PVIDLENGTH 17

/*
 * NAME: get_id_pvnm
 *
 * DESCRIPTION:  Gets a specified physical volume id given the pv name.
 * 	Gets the identifier from the CuAt class in ODM.
 *
 * INPUT: The physical volume identifier in pvname.
 *
 * OUTPUT: The physical volume identifier in pv_id.
 *
 * RETURN VALUE DESCRIPTION:
 *			0: successful.
 *			2: If ODM error.
 *			3: Identifier not found and errflag set.
 *			
 */

int
get_id_pvnm(char *name, char *type, char *id)
{
struct CuAt *cuatp;
int temp,ret_code;

odm_initialize();
/* set the config database path for the classes */
odm_set_path(CFGPATH);

/* get pvid from pvname, from odm */
cuatp = getattr(name, type,FALSE,&temp);

/* if getattr returns ODMERROR then an odm error has occurred */
if ((int)cuatp == ODMERROR) /* assign error = appropriate integer */
	return (362); /* unknown odm error */

/* if the pointer is not NULL then a match was found */
if (cuatp != NULL)
{
	if ((strncmp(cuatp->value,PVNONE,4)) == 0)
		sprintf(id,"%s",ZEROPV);
	else {
	  strncpy(id,cuatp->value,16);
        }    
        id[16] = '\0';
        ret_code = 1;
}
/* else the object was not found, print error and exit */
else {
     id = NULL;
     ret_code = 0;
}	
odm_terminate(); 
return(ret_code);
}

/*
 * return 1 if we've already seen this pp
 */
static int 
seen_this_free_pp(PV *pv, int ppnum)
{
	RANGE *ptr;

	for (ptr = pv->free_range_head; ptr != NULL; ptr = ptr->next_range)
		if (ptr->start <= ppnum && ppnum < ptr->start + ptr->length)
			return (1);

	return (0);
}

/* insert ppnum into the free range list of pv[i] */

static void
insert(i,pv,ppnum,spread,isallocated)
   int   i;
   PV   *pv[];
   int   ppnum;
   SPREAD spread;
   int  isallocated;		/* is this ppnum already allocated? */

{
   RANGE  *ptr,*range_ptr;
   int     done;

   done = 0;

   /*
    * special-case code for the -ep case (we should allocate to
    * the lp exactly as the input map dictates)
    */

   if (spread == MAP) {
	RANGE *last;

	/*
	 * intheregion isn't used in the map case anyway.
	 * we'll use it to remember the index into the pv table.
	 */
#define	the_pvid	intheregion

	/*
	 * find last entry in range list
	 */
	if ((last = pv[0]->free_range_head) != NULL)
		for ( ; last->next_range != NULL; last = last->next_range)
			;

	/*
	 * can we append this pp to the last range?
	 */
	if (last != NULL && i == last->the_pvid &&
	    ppnum == last->start + last->length)
		last->length++;

	/*
	 * nope, create a new range entry
	 */
	else {
		ptr = (RANGE *) malloc(sizeof(RANGE));
		ptr->start = ppnum;
		ptr->length = 1;
		ptr->the_pvid = i;		/* index into pv table */
		ptr->next_range = NULL;

		/*
		 * append to end of list
		 */
		if (last == NULL)
			pv[0]->free_range_head = ptr;
		else
			last->next_range = ptr;
		}

	return;
#undef	the_pvid
	}

   if (ppnum > pv[i]->highestppnum) pv[i]->highestppnum = ppnum;

   /*
    * if this ppnum was allocated, we simply wanted to track the
    * highest ppnum on the pv.  do *not* add this to the free list!
    * do the same if we've already seen this pp.  This was moved from
    * the front of insert() because it was causing a bug when used in
    * conjuction with spread == MAP.  When using a user map, pv[0] is
    * the only valid pointer, however seen_this_free_pp is based on
    * all the pointers, pv[i].
    */
   if (isallocated || seen_this_free_pp(pv[i], ppnum))
	return;

   pv[i]->free_pp_count++;  

   ptr = pv[i]->free_range_head;
   if (ptr != NULL){
      if (ppnum < ptr->start-1){
                  range_ptr = (RANGE *) malloc(sizeof(RANGE));
                  range_ptr->start = ppnum;
                  range_ptr->length = 1;
                  range_ptr->next_range = ptr;
                  pv[i]->free_range_head = range_ptr;
                  pv[i]->free_section_count ++;
                  done = 1;
      }
   }
   else {
                  range_ptr = (RANGE *) malloc(sizeof(RANGE));
                  range_ptr->start = ppnum;
                  range_ptr->length = 1;
                  range_ptr->next_range = NULL;
                  pv[i]->free_range_head = range_ptr;
                  pv[i]->free_section_count ++;
                  done = 1;
   }
   if (!done)for (ptr=pv[i]->free_range_head; ptr; ptr=ptr->next_range){
       if (ppnum == (ptr->start+ptr->length)) {
           ptr->length++;
           done = 1;
           break; 
       }
       if (ppnum == ptr->start-1) {
           ptr->start --;
           ptr->length++;
           done = 1;
           break; 
       }
       else
       if (ppnum > ptr->start+ptr->length) {
           if (ptr->next_range) {
               if (ppnum < ptr->next_range->start-1) break;
           }
           else break; 
       }
   } 
   if (ptr && !done) {
                  range_ptr = (RANGE *) malloc(sizeof(RANGE));
                  range_ptr->next_range = ptr->next_range ;
                  ptr->next_range = range_ptr;
                  range_ptr->start = ppnum;
                  range_ptr->length = 1;
                  pv[i]->free_section_count ++;
                  done = 1;
   }
   /* join sections that may be adjoint */
     ptr=pv[i]->free_range_head; 
     while (ptr != NULL){
         if (ptr->next_range == NULL) break;
         if (ptr->start+ptr->length == (ptr->next_range)->start) { 
           ptr->length += ptr->next_range->length;
           ptr->next_range = ptr->next_range->next_range;
           pv[i]->free_section_count --;
         }
         else ptr = ptr->next_range;
     }
}

/*
 *
 * FUNCTION: Reads maps from stdin and returns data structures which contain all the information.
 *
 * Standard input format is :
 * token: pvid1:ppnum1   state  type  lvid  lpnum  
 * token  = PVMAP, LVMAP or MODIFY 
 *
 * The order of input is all PVMAP records, followed by all LVMAP
 * records and then all MODIFY records
 * PVMAP records must be ordered wrt pvid and the records of 
 * the same pv must be in ascending order wrt ppnum.
 * LVMAP records belonging to the same logical partition 
 * must be placed consecutively. 
 * All MODIFY entries must be at the end of the input file.
 * Any logical partiton in a MODIFY record must not be in also
 * a LVMAP record
 */

int mapread (thelvid, numpv, pv, numlvpp, lvmap, nummodpp, spread)

LVID  thelvid;      /* specifies lv the allocation is for - 
         a seperate list is made for all lp's/pp's of this lv (INPUT) */

int   *numpv;
PV    *pv[];        /* data structure containing free sections read - 
                       the state-of-the-world (OUTPUT) */
int
    *numlvpp;     /* number of pp lv has */

struct lvlist 
        lvmap[];

int
    *nummodpp;    /* number of pp entries in "modpp" data structure (OUTPUT) */
SPREAD spread;
{
	short int
	    state;                /* contains pp state from one input line */
	int i,                    /* line counter and loop index */
	    lpnum,                /* contains lp number from one line */
	    ppnum,                /* contains pp number from one line */
            ppnum2,               /* contains the end of a range */
	    pvptr;                /* index into pv array */

    int     previous_lpnum = -1;

	char
	    line[LSIZE];          /* one input line */
	char
	    token[STRSIZE],       /* is PVMAP, LVMAP or MODIFY */
	    ppstr[STRSIZE],       /* contains pvid:pp string from one line */
	    type[STRSIZE],        /* contains pp type string from one line */
	    lvstr[STRSIZE];       /* contains lvid string from one line */
	ID   pvid;                 /* contains converted pvid from one line */
	LVID thislvid;             /* contains converted lvid from one line */
        char  *pvidascii;
        char  pvidstr[17];
        char *pvname;
        char *ptr;
        int   copynum;
        int   pvmap,lvmappe,modify,error;

	*numpv = *numlvpp = *nummodpp = 0;

/* read line from standard in until no more found */
for (i=1; fgets(line, LSIZE, stdin) != NULL; i++) { 

	if (strlen(line) <= 1) continue;  /* skip over empty lines */

        /* break the tokens from the line into individual fields */
	if ( sscanf(line, "%25s%25s%hd%25s%25s%d", 
		    token, ppstr, &state, type, lvstr, &lpnum)  < 1 ) {
		fprintf(stderr,lvm_msg(ALLOCP_LINERROR),"allocp",i);
		exit(i);
	}

         pvmap=lvmappe=modify=0;
	 pvmap=(strcmp(token,"PVMAP:")==0);
	 lvmappe=(strcmp(token,"LVMAP:")==0);
	 modify=(strcmp(token,"MODIFY:")==0);
 
         if (pvmap || lvmappe || modify) {
          /* confirm delimiter exist - find its location */
          if ( !(ptr=strpbrk(ppstr,":")))  {
		fprintf(stderr,lvm_msg(ALLOCP_LINERROR),"allocp",i);
	        exit(-1);	
          }
	  else {
           /* read/convert the number portion - right of delimiter location */
	   sscanf (ptr+1, "%d", &ppnum); 
          }
          pvidascii = strtok (ppstr, ":") ;
        }

	if (modify) /* && (equal_ids(&thelvid,&thislvid))  */
        {
           /* find which pv the pp belongs to */
           pvptr = getpvptr (numpv, pv, pvidascii); 
#ifdef OLD
           insert(pvptr,pv,ppnum, spread, 0);
#endif
	   rmlvmap(lvmap,pvptr, ppnum,lpnum,state,&copynum); 
           pv[pvptr]->totallpcount --;
           pv[pvptr]->lpcount[copynum] --;
         }

	else if (lvmappe) /* && (equal_ids(&thelvid,&thislvid)) */  {  
	    /*
	     * only store the lv info iff the lv is allocated.
	     * this allows us to fill in holes!
	     */
	    if (state) {
            /* save the pvid in the pv list */
	    pvptr = getpvptr (numpv, pv, pvidascii); 
            /* add the lpnum to the lvlist */
	    getlvmap(lvmap,pvptr, ppnum,lpnum,state,&copynum); 
            (pv[pvptr]->totallpcount)++;
            (pv[pvptr]->lpcount[copynum])++;
            (*numlvpp)++;
	    }
	}
	else if (pvmap) { 
                        /* find which pv the pp belongs to */
         		pvptr = getpvptr (numpv, pv, pvidascii); 

                        insert(pvptr,pv,ppnum, spread, state);

	}
        else {  /* line does not start with a token name */
           /* confirm delimiter exist - find its location */
           if ( !(ptr=strpbrk(token,":")))  {
		fprintf(stderr,lvm_msg(ALLOCP_LINERROR),"allocp",i);
	        exit(-1);	
           }
	   else {
             /* read/convert the number portion - right of delimiter location */
	     sscanf (ptr+1, "%d", &ppnum); 
           }
           if ( (ptr=strpbrk(token,"-")))  {
             sscanf (ptr+1, "%d", &ppnum2);
           }
           else ppnum2 = -1;

           pvname = strtok (token, ":") ;

	   if( !get_id_pvnm(pvname,PVATTR, &pvidstr[0] ))
           {
		fprintf(stderr,lvm_msg(ALLOCP_LINERROR),"allocp",i);
	        exit(-1);	
           }
           if (debug) {
              fprintf(debug,"in mapread; pvidascii=%s\n",pvidstr);
              fflush(debug);
           }
           

           /* find which pv the pp belongs to */
            pvptr = getpvptr (numpv, pv, pvidstr); 
           insert(pvptr,pv,ppnum, spread, 0);
           ppnum ++;
           while (ppnum <= ppnum2) {
                insert(pvptr,pv,ppnum, spread, 0);
                ppnum++;
           }
        }
}
}

/* remove a pp marked modify from the lvlist */
rmlvmap(lvmap,pvindex,ppnum,lpnum,state,copy)

   struct lvlist lvmap[];
   int   pvindex,ppnum,lpnum ;
   short int state;
   int *copy;
{
    LVMAP   *ptr,*prev;
    int     i;

    for(i=0; i<3; i++) {
             ptr = lvmap[i].head;     
             prev = ptr;
             while (ptr) {
                 if (ptr->lpnum == lpnum
                     && ptr->ppnum == ppnum
                     && ptr->pvindex==pvindex) break; 
                 prev = ptr;
                 ptr = ptr->next;
             } 
             if (ptr) {  /* lpnum is found in copy i */
                  *copy = i;
                  if (prev==ptr) lvmap[i].head = ptr->next;
                  prev -> next = ptr->next;
                  lvmap[*copy].num_of_lps --;
                  break;
             }
        }
}

/* put the next lp pp in the correct copy list */

getlvmap(lvmap,pvindex,ppnum,lpnum,state,copy)

   struct lvlist lvmap[];
   int   pvindex,ppnum,lpnum ;
   short int state;
   int *copy;
{
    LVMAP   *ptr,*ptr2,*lpptr;
    int     copynum,i;

    ptr = (LVMAP *) malloc (sizeof(LVMAP)) ;
    ptr->pvindex = pvindex;
    ptr->ppnum = ppnum;
    ptr->lpnum = lpnum;
    ptr->state = state;
    ptr->next = NULL;

    for(i=0; i<3; i++) {
        if (lpnum > lvmap[i].max_lp_num) {
             /* copy i does not have lpnum */
             copynum = i;
             break;
        }
        else {
             ptr2 = lvmap[i].head;     
             while (ptr2) {
                 if (ptr2->lpnum == lpnum) break; 
                 ptr2 = ptr2->next;
             } 
             if (!ptr2) {  /* lpnum is not found in copy i */
                  copynum = i;
                  break;
             }
        }
    }

    if ( lvmap[copynum].head == NULL ) {
          lvmap[copynum].head = ptr;
    }
    else {
         ptr2 = lvmap[copynum].head;
         if ( ptr2->lpnum > lpnum ) {
            lvmap[copynum].head = ptr;
            ptr->next = ptr2;
         }
         else {
            while ( ptr2 -> next ) {
             if (lpnum > ptr2->lpnum && lpnum < ptr2->next->lpnum) break;
             ptr2 = ptr2->next;
            }
            ptr->next = ptr2->next;
            ptr2->next = ptr;
         }
    }
    lvmap[copynum].num_of_lps ++;
    if ( lvmap[copynum].max_lp_num < lpnum) lvmap[copynum].max_lp_num = lpnum ;
    *copy = copynum;

    if (debug) {
    fprintf(debug,"copynum:%d %d %d lpnum:%d\n",copynum,pvindex,ppnum,lpnum);

    lpptr= lvmap[copynum].head;
    for (;lpptr; lpptr = lpptr->next) {
       fprintf(debug,"%d %d %d ",lpptr->pvindex,lpptr->ppnum,lpptr->lpnum);
    }
    fprintf(debug,"\n");
    }
}

/*
 * NAME: getpvptr
 *
 * FUNCTION: Return pointer/index into "pv" data structure for a specific pvid.  If the pvid is not in the "pv"
 *           data structure, insert it and then return its pointer/index.
 *
 * RETURN VALUE: index to pvid entry in "pv" data structure
 *
 */

int getpvptr (numpv, pv, pvidascii)

int  *numpv;        /* current number of pvs (INPUT/OUTPUT) */
PV   *pv[];         /* "pv" data structure (INPUT/OUTPUT) */
char *pvidascii;    /* pvid the index is to be found for (INPUT) */
{
	int p,i,j;          /* index to and counter for "pv" */
        struct unique_id pv_id;

	for (p=0; p<*numpv; p++)
            if SAMEASCIIPV(pv[p]->asciipvid,pvidascii) return(p);

	/* pvid not found so insert */

        i = *numpv;
    	pv[i] = (PV *) malloc (sizeof(PV)); 
        pv[i]->free_section_count = 0;
        pv[i]->free_pp_count = 0; 
        pv[i]->free_range_head = NULL;
        pv[i]->highestppnum = 0;
        for (j=0; j< 3; j++) pv[i]->lpcount[j] = 0;
        pv[i]->totallpcount = 0;

        get_uniq_id(pvidascii,&pv_id);

        strcpy(pv[i]->asciipvid,pvidascii);
        ASSIGN(pv[i]->pvid,pv_id); 

        (*numpv)++;
	return(i);  
}
