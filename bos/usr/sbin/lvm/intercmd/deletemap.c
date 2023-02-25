static char sccsid[] = "@(#)28	1.2.1.1  src/bos/usr/sbin/lvm/intercmd/deletemap.c, cmdlvm, bos411, 9428A410j 8/14/91 15:03:49";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 * convert_list_array, fill_pp, rmcopy	
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <lvm.h>
#include "allocp.h"

/*
 * NAME: convert_list_array
 *
 * FUNCTION: Convert an array of 3 linked lists to an array of LPs
 *
 * INPUT: lvlist
 * OUTPUT: lp_array
 *
 * RETURN CODE: always 0 
 *
 */


int convert_to_array(lvlist,lp_array,max)
struct lvlist       lvlist[];     /* INPUT linked list of LPs in the LV */
struct lp_map       lp_array[];   /* OUTPUT array of LPs */
int   max;
{

int              copynum, lpnum;                       /* loop counters */
struct lv       *lv_ptr;
struct lp_map   *lp_ptr;

    /* go through the array of linked lists */
    for (copynum=0;  copynum<3;  copynum++) {

        if (lvlist[copynum].head != (LVMAP *) NULL)  {
            lv_ptr = lvlist[copynum].head;

            while (lv_ptr) {
              lp_array[lv_ptr->lpnum-1].copy[lp_array[lv_ptr->lpnum-1].num_of_copies] = lv_ptr;
              lp_array[lv_ptr->lpnum-1].num_of_copies++;
              lv_ptr = lv_ptr->next;        /* point to next element */
            }
        }

    }           /* end of outer for loop */

    return(0);
}


/*
 * NAME: fill_pp
 *
 * FUNCTION: fill in the struct pp pointed to by pp_ptr with information 
 *           pointed to by lv_ptr and pv.
 *
 *
 * RETURN CODE: always return 0 
 *
 */


int fill_pp(pp_ptr,lv_ptr,pv,new_size)
LVPP            *pp_ptr;
struct lv       **lv_ptr;
PV              *pv[];
int             *new_size;

{

      ASSIGN(pp_ptr->pv_id, pv[(*lv_ptr)->pvindex]->pvid);
      pp_ptr->lp_num = (long) (*lv_ptr)->lpnum;
      pp_ptr->pp_num = (long) (*lv_ptr)->ppnum;

     /* assign NULL to pointer so it won't be available again
        since a stale pp might also be out of region */
      *lv_ptr = (struct lv *) NULL;
      (*new_size)++;        /* update number of pps to be removed */

      return(0);
}



/*
 * NAME: rmcopy
 *
 * FUNCTION: Take the linked list lvlist and create a delete_array containing
 *           all the pp that need to be deleted to bring the size of number
 *           of mirror copies to the desired_copies.
 *
 * OUTPUT: 
 *         Error message written to standard error.
 *
 * RETURN CODE: pointer to the delete_array
 *
 */

extern char **Delete_pvids;
extern int Delete_pvidct;

LVPP 
*rmcopy(desired_copies,pv,lvlist,new_size)

int               desired_copies;    /* number of copies to be remained */
PV               *pv[];
struct lvlist     lvlist[];
int               *new_size;

{

struct lp_map   *lp_array;   /* array of LPs */
LVPP            *delete_array; /* pointer to the array of PPs to be deleted */
LVPP            *pp_ptr;
LVMAP           *lv_ptr;
int    delete_size;
int    lp_array_size;
int    copynum, lpnum;    /* loop counters */
int    max_size = 0;
int    max_of_max = 0;      /* maximum possible LPs of the whole LV */
ID *pvids;		/* actual pvids to remove */
int i;			/* index */


    /*
     * if pvids given, convert pvid ascii to real pvids
     */
    if (Delete_pvidct > 0) {
	pvids = (ID *) malloc(Delete_pvidct * sizeof(ID));
	for (i = 0; i < Delete_pvidct; i++)
		sscanf(*(Delete_pvids + i), "%08x%08x",
			&(pvids + i)->word1, &(pvids + i)->word2);
	}

    /* calculate the maximum possible size of PPs to be deleted */
    for (copynum=0;  copynum<3;  copynum++) {
       if (lvlist[copynum].head != (LVMAP *) NULL) {
           max_size += lvlist[copynum].max_lp_num;
           max_of_max = MAX(max_of_max, lvlist[copynum].max_lp_num);
       }
    }

    /* allocate memory for the needed arrays */
    delete_size = max_size * sizeof (struct pp);
    lp_array_size = max_of_max * 3 * sizeof (struct lp_map);
    delete_array = (LVPP *) malloc (delete_size);
    lp_array = (struct lp_map *) malloc (lp_array_size);

    bzero(delete_array, delete_size);
    bzero(lp_array, lp_array_size);

    /* convert linked list to array */
    convert_to_array(lvlist,lp_array,max_of_max);

    pp_ptr = delete_array;
    for (lpnum=1;  lpnum <= max_of_max;  lp_array++, lpnum++)  {

	  if (Delete_pvidct > 0)
          /* try to remove the pp's associated with a particular pv first */
          for (copynum=2;
               (lp_array->num_of_copies > desired_copies) && (copynum >= 0) ;
               copynum--)  {

		if ((lv_ptr = lp_array->copy[copynum]) != (struct lv *) NULL)
		    for (i = 0; i < Delete_pvidct; i++)
			if (SAMEPV((*(pvids + i)), pv[lv_ptr->pvindex]->pvid)) {
			    fill_pp(pp_ptr,&lv_ptr,pv,new_size);
			    pp_ptr++;/* increment delete_array pointer */
			    lp_array->num_of_copies-- ;
			    break;
			    }
             }

          /* try to remove the STALE PPs first, starting with 3rd copy */
          for (copynum=2;
               (lp_array->num_of_copies > desired_copies) && (copynum >= 0) ;
               copynum--)  {

             if ( ((lv_ptr = lp_array->copy[copynum]) != (struct lv *) NULL)  &&
                 (lv_ptr->state & LVM_PPSTALE)   )    {

                  fill_pp(pp_ptr,&lv_ptr,pv,new_size);
                  pp_ptr++;            /* increment delete_array pointer */
                  lp_array->num_of_copies-- ;
             }          /* end of if */
          }             /* end of inner for loop */


          /* enforce strictness next */
          /* if copy 1 and 3 are on the same pv, remove copy 3 etc... */
          if ( (lp_array->num_of_copies > desired_copies )  &&
               (lp_array->copy[0] != (struct lv *) NULL)  &&
               (lp_array->copy[2] != (struct lv *) NULL)  &&
               (lp_array->copy[0]->pvindex == lp_array->copy[2]->pvindex) 
             )   {
              fill_pp(pp_ptr,&(lp_array->copy[2]),pv,new_size);
              lp_array->num_of_copies--;
              pp_ptr++;            /* increment delete_array pointer */
          }

          if ( (lp_array->num_of_copies > desired_copies )  &&
               (lp_array->copy[1] != (struct lv *) NULL)  &&
               (lp_array->copy[2] != (struct lv *) NULL)  &&
               (lp_array->copy[1]->pvindex == lp_array->copy[2]->pvindex) 
             )   {
              fill_pp(pp_ptr,&(lp_array->copy[2]),pv,new_size);
              lp_array->num_of_copies--;
              pp_ptr++;            /* increment delete_array pointer */
          }

          if ( (lp_array->num_of_copies > desired_copies )  &&
               (lp_array->copy[0] != (struct lv *) NULL)  &&
               (lp_array->copy[1] != (struct lv *) NULL)  &&
               (lp_array->copy[0]->pvindex == lp_array->copy[1]->pvindex) 
             )   {
              fill_pp(pp_ptr,&(lp_array->copy[1]),pv,new_size);
              lp_array->num_of_copies--;
              pp_ptr++;            /* increment delete_array pointer */
          }


          /* remove copy that is out of region next, starting with 3rd copy */
          for (copynum=2;
              (lp_array->num_of_copies > desired_copies) && (copynum >= 0) ;
               copynum--)  {

             if ( ((lv_ptr = lp_array->copy[copynum]) != (struct lv *) NULL)  &&
                  (lv_ptr->ppnum < pv[lv_ptr->pvindex]->region_start )  &&
                  (lv_ptr->ppnum > (pv[lv_ptr->pvindex]->region_start + pv[lv_ptr->pvindex]->region_length) )
                )    {

                  fill_pp(pp_ptr,&lv_ptr,pv,new_size);
                  lp_array->num_of_copies--;
                  pp_ptr++;            /* increment delete_array pointer */

              }          /* end of if */

          }             /* end of inner for loop */


          /* remove any extra copy next, starting with 3rd copy */
          for (copynum=2;
              (lp_array->num_of_copies > desired_copies) && (copynum >= 0) ;
               copynum--)  {

             if ((lv_ptr = lp_array->copy[copynum]) != (struct lv *) NULL)    {
                  fill_pp(pp_ptr,&lv_ptr,pv,new_size);
                  lp_array->num_of_copies--;
                  pp_ptr++;            /* increment delete_array pointer */
             }          /* end of if */

          }             /* end of inner for loop */

    }                /* end of outer for lpnum loop */

    /*
     * if pvids given, delete real pvids
     */
    if (Delete_pvidct > 0)
	free((void *) pvids);

    return(delete_array);

}
