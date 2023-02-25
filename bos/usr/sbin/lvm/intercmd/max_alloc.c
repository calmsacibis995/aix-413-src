static char sccsid[] = "@(#)77	1.1  src/bos/usr/sbin/lvm/intercmd/max_alloc.c, cmdlvm, bos411, 9428A410j 4/3/91 11:10:17";

/*
 COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 FUNCTION: allocp
 ORIGINS: 27

 (C) COPYRIGHT International Business Machines Corp. 1989
 All Rights Reserved
 Licensed Materials - Property of IBM

 US Government Users Restricted Rights - Use, duplication or
 disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

 FILE NAME: max_alloc.c 

 FILE DESCRIPTION: 

 RETURN VALUE DESCRIPTION:

 EXTERNAL PROCEDURES CALLED: 
*/

#include <stdio.h>
#include <lvm.h>
#include "allocp.h"
#include "cmdlvm_msg.h"
#include <locale.h>

/*
 * extern functs and data
 */
extern char *lvm_msg();
extern char *Prog;
extern void *malloc();
extern int increasing(void *, void *);
extern int markregion(int, PV *[], REGION);
extern LVPP *getnewpps(TYPE, int, int, struct lvlist *, int *);

/*
 * local funcs
 */
static int select_pvs(PV *[], int, int, int, REGION, int);
static int place1copy(int, LVPP *, PV *[], int, int, int);
static int foundlp(int, PV *);
static void sortpps(LVPP *, int, PV *[], int);
static int pv_index(struct unique_id, PV *[], int);
static void assign_from_range(LVPP *, PV *, RANGE *, int);

/*
 * macros
 */
#define	min(x, y)	((x) < (y) ? (x) : (y))
#define	max(x, y)	((x) > (y) ? (x) : (y))

#define	closer(mid, save, cur, estart, elength)			\
	((save) == NULL ||					\
	(abs((mid) - ((cur)->estart + (cur)->elength / 2)) <	\
	 abs((mid) - ((save)->estart + (save)->elength / 2))))

/*
 *	max_alloc
 *
 *	- perform the 'maximum' interpolicy algorithm
 */
LVPP *
max_alloc(
TYPE		type,
int		numofcopies,
int		strict,
int		size,
int		pvct,
PV	       *pvs[],
int		numlvpps,
struct lvlist  *lvlist,
REGION		region,
int		upperbound,
int	       *newsize,
int	       *err_code
)
{
	LVMAP  *lvm;
	int	i, j, copyno;
	LVPP   *layout, *curcopy;
	int	numofpps[MAXCOPIES];	/* #  pps required for each copy */
	PV    **pv;

	*err_code = 0;

	/*
	 * get the initial layout
	 */
	layout = getnewpps(type, numofcopies, size, lvlist, numofpps);

	/*
	 * compute how many lps we really need
	 */
	for (*newsize = i = 0; i < numofcopies; *newsize += numofpps[i++])
		;
	if (*newsize == 0)  /* nothing needs to be allocated */
		return(NULL);

	/*
	 * mark the region on each pv
	 */
	(void) markregion(pvct, pvs, region);

	/*
	 * bring upperbound down if it wasn't set
	 */
	upperbound = min(upperbound, pvct);

	/*
	 * allocate 'lpnums' for each pv
	 */
	for (pv = &pvs[0]; pv < &pvs[pvct]; pv++) {
		(*pv)->lpnums = (int *) malloc(sizeof(int) *
					((*pv)->totallpcount + *newsize));
		(*pv)->totallpcount = 0;
		}

	/*
	 * mark pv's with lps already used
	 */
	for (i = 0; i < numofcopies; i++)
		for (lvm = lvlist[i].head; lvm != NULL; lvm = lvm->next) {
			pv = &pvs[lvm->pvindex];
			(*pv)->lpnums[(*pv)->totallpcount] = lvm->lpnum;
			(*pv)->totallpcount++;
			}

	/*
	 * sort the lps in increasing order
	 */
	for (pv = &pvs[0]; pv < &pvs[pvct]; pv++) {
		(*pv)->org_lpcount = (*pv)->totallpcount;
		qsort((void *) (*pv)->lpnums, (*pv)->totallpcount, sizeof(int),
			increasing);
		}

	/*
	 * compute which copy to begin on...
	 */
	if (type == EXTEND && numofpps[0] > 0)
		copyno = 1;
	else if (type == COPY || type == EXPLICIT) {
		copyno = -1;
		for (i = 0; i < numofcopies; i++)
			if (numofpps[i] > 0)  {
				copyno = i + 1;
				break;
				}
		}

	if (copyno < 0) {		/* hopefully this shouldn't happen */
		fprintf(stderr, lvm_msg(ALLOCP_NORESOURCE),Prog);
		*err_code = -1;
		return (NULL);
		}

	/*
	 * walk thru and lay the copies down
	 */
	for (curcopy = layout; copyno <= numofcopies;
	     curcopy += numofpps[copyno++])
		/*
		 * figure out what pvs we're really gonna use,
		 * and place a copy onto them.
		 */
		if (select_pvs(pvs, pvct, upperbound, numofcopies,
			region, numofpps[copyno - 1]) < 0 ||
		    place1copy(copyno, curcopy, pvs, upperbound,
				numofpps[copyno - 1], strict) != 0) {
			fprintf(stderr, lvm_msg(ALLOCP_NORESOURCE),Prog);
			*err_code = -1;
			return (NULL);
			}

	/*
	 * sort the pps (so that the layout will be sequential on
	 * each pv)
	 */
	sortpps(layout, *newsize, pvs, upperbound);

	return(layout);
}

static int maxset(void *, void *);
static int cmp_pcounts(void *, void *);
static int cmp_free(void *, void *);

/*
 *	select_pvs
 *
 *	- select which pv's to use...
 */
static int
select_pvs(
PV     *pvs[],
int	pvct,
int	upperbound,
int	numofcopies,
REGION	region,
int	pps_needed)
{
	int i, j, pps;
	/*
	 * list of sort functions, in order of most picky to
	 * most general.
	 */
	static int (*cmpfuncs[])(void *, void *) = {
		cmp_pcounts,
		cmp_free,			/* most general */
		};

	/*
	 * 'maxset' insures we use pvs first that already contain lps
	 * for this lv
	 */
	qsort((void *) pvs, pvct, sizeof(pvs[0]), maxset);

	/*
	 * for each sort routine, see if this sort of the pv's
	 * gives us enough free pps to use
	 */
	for (i = 0; i < sizeof(cmpfuncs) / sizeof(cmpfuncs[0]); i++) {
		qsort((void *) pvs, upperbound, sizeof(pvs[0]), cmpfuncs[i]);

		for (pps = j = 0; j < upperbound;
		     pps += pvs[j++]->free_pp_count)
			;

		if (pps >= pps_needed)
			return (0);
		}

	return (-1);
}

/* 
 *	maxset
 *
 *	- qsort-type function that sorts by decreasing pcount and
 *	  decreasing pp count
 */
static int
maxset(void *p1, void *p2)
{
	int rc;
	PV **pv1 = (PV **) p1;
	PV **pv2 = (PV **) p2;

	if ((rc = ((*pv2)->totallpcount - (*pv1)->totallpcount)) == 0)
		rc = (*pv2)->free_pp_count - (*pv1)->free_pp_count;

	return (rc);
}

/*
 *	sort the pvs by increasing lp count & by decreasing free pp count
 */
static int
cmp_pcounts(void *p1, void *p2)
{
	int rc;
	PV **pv1 = (PV **) p1;
	PV **pv2 = (PV **) p2;

	if ((rc = ((*pv1)->totallpcount - (*pv2)->totallpcount)) == 0)
		rc = (*pv2)->free_pp_count - (*pv1)->free_pp_count;

	return (rc);
}

/*
 *	just sort by decreasing pp count
 */
static int
cmp_free(void *p1, void *p2)
{
	PV **pv1 = (PV **) p1;
	PV **pv2 = (PV **) p2;

	return ((*pv2)->free_pp_count - (*pv1)->free_pp_count);
}

/*
 *	place1copy
 *
 *	- places a copy of the lv
 */
static int
place1copy(
int		copyno,
LVPP	       *layout,
PV	       *pvs[],
int		pvct,
int		lps,
int		strict)
{
	PV **pv;
	int cantuse;
	RANGE *r, *savein, *saveout;
	int midregion;

	/*
	 * how many pvs we can't use (to hopefully avoid an infinite loop)
	 */
	cantuse = 0;

	/*
	 * we start at -1 since we increment it in the loop before we use it
	 */
	pv = &pvs[-1];

	/*
	 * loop while we still have lps to allocate...
	 */
	while (lps > 0) {
		if (++pv >= &pvs[pvct])
			pv = &pvs[0];		/* wrap around */

		/*
		 * anything free on this pv?
		 */
		if ((*pv)->free_pp_count <= 0 ||
		    (strict && foundlp(layout->lp_num, *pv))) {
			if (++cantuse == pvct)	/* check all pvs? */
				return (-1);
			}

		else {
			cantuse = 0;
			savein = saveout = NULL;
			midregion = (*pv)->region_start +
				    (*pv)->region_length / 2;

			/*
			 * walk thru the free ranges on the pv, looking
			 * for the closest range within the region, and
			 * the closest outside of the region
			 */
			for (r = (*pv)->free_range_head; r; r = r->next_range){
				if (r->length <= 0)
					continue;

				if (r->intheregion > 0 &&
				    r->region_length > 0) {
					if (closer(midregion, savein, r,
						   intheregion, region_length))
						savein = r;
					}

				else if (closer(midregion, saveout, r,
						start, length))
					saveout = r;
				}

			/* 
			 * if we found either, get the pp's from it (but
			 * be sure to use the inside region range first)
			 */
			if (savein != NULL || saveout != NULL) {
				assign_from_range(layout, *pv,
						  savein ? savein : saveout,
						  midregion);
				layout++;
				lps--;
				}
			}
		}

	return (0);
}

/*
 *	foundlp
 *
 *	- see if a pv has a copy of 'lpnum' already on it
 */
static int
foundlp(int lpnum, PV *pv)
{
	extern void *bsearch();
	int *a = (int *) bsearch((void *) &lpnum, (void *) pv->lpnums,
		pv->totallpcount, sizeof(int), increasing);

	return ((int) a);
}

/*
 *	sortpps
 *
 *	- this walks thru the layout.  when it finds a pv for the first time
 *	  it then gathers all the pps for that pv in the layout.  it then
 *	  sorts them, then reassigns the pps back to the pv
 */
static void
sortpps(LVPP *layout, int layct, PV *pvs[], int pvct)
{
	int i, pvidx;
	LVPP *lay, *l;
	int *lps, lpct;

	/*
	 * get us some memory to sort in
	 */
	if ((lps = (int *) malloc(sizeof(*lps) * layct)) == NULL)
		return;

	/*
	 * we're gonna use the PV element 'highestppnum' to indicate
	 * that we've seen this pv before
	 */
	for (i = 0; i < pvct; i++)
		pvs[i]->highestppnum = 0;

	/*
	 * start searching the layout
	 */
	for (lay = layout; lay < layout + layct; lay++) {
		/*
		 * get the index of the pv and see if we've already
		 * seen it
		 */
		if ((pvidx = pv_index(lay->pv_id, pvs, pvct)) < 0 ||
		    pvs[pvidx]->highestppnum > 0)
			continue;

		/*
		 * mark it as seen
		 */
		pvs[pvidx]->highestppnum++;

		/*
		 * walk thru the remainder of the layout (including the
		 * one we're currently on) and save the pps
		 */
		lpct = 0;
		for (l = lay; l < layout + layct; l++)
			if (SAMEPV(lay->pv_id, l->pv_id))
				lps[lpct++] = l->pp_num;
		/*
		 * sort them in increasing order
		 */
		qsort((void *) lps, lpct, sizeof(int), increasing);

		/*
		 * walk back thru the layout and assign the ordered ones
		 * back
		 */
		lpct = 0;
		for (l = lay; l < layout + layct; l++)
			if (SAMEPV(lay->pv_id, l->pv_id))
				l->pp_num = lps[lpct++];
		}

	free((void *) lps);		/* done with this */
}

/*
 *	pv_index
 *
 *	- figure out the index of 'pvid' in the pv array
 */
static int
pv_index(struct unique_id pvid, PV *pvs[], int pvct)
{
	int pvidx;

	for (pvidx = 0; pvidx < pvct; pvidx++)
		if (SAMEPV(pvid, pvs[pvidx]->pvid))
			return (pvidx);

	return (-1);
}

/*
 *	assign_from_range
 *
 *	- assign a pp from a selected range
 */
static void
assign_from_range(LVPP *layout, PV *pv, RANGE *range, int midregion)
{
	int ppnum;
	RANGE *new;

/*
 * figure out which pp to allocate within the range...  what we're doing
 * here is:  if the range overlaps the requested region, either use the
 * middle of the region itself (if it's in the range), or use either the
 * high side or low side of the range (whichever's closet).  if the range
 * doesn't overlap the requested region, just use the high or low side.
 */

	/*
	 * if we're in the region....
	 */
	if (range->intheregion > 0 && range->region_length > 0) {
		/*
		 * try to use the center of the region first...
		 */
		if (range->intheregion <= midregion &&
		    midregion < range->intheregion + range->region_length)
			ppnum = midregion;
		/*
		 * else figure out which side to use
		 */
		else if (abs(midregion - range->intheregion) <
			 abs(midregion - (range->intheregion +
				range->region_length - 1)))
			ppnum = range->intheregion;
		else
			ppnum = range->intheregion + range->region_length - 1;
		}

	/*
	 * figure out which side to use... (outside the region)
	 */
	else if (abs(midregion - range->start) <
		 abs(midregion - (range->start + range->length - 1)))
		ppnum = range->start;

	else
		ppnum = range->start + range->length - 1;

	/*
	 * start of range? (easy case #1)
	 */
	if (ppnum == range->start) {
		/*
		 * if the ppnum is at the beginning
		 * of this region, just increment
		 * the beginning of the region also
		 */
		if (range->start == range->intheregion) {
			range->intheregion++;
			range->region_length--;
			}
		range->start++;
		range->length--;
		}

	/*
	 * end of range? (easy case #2)
	 */
	else if (ppnum == range->start + range->length - 1) {
		if (range->start + range->length ==
		    range->intheregion + range->region_length)
			range->region_length--;
		range->length--;
		}

	else {
		/*
		 * else, we have to split the range.  todo this we
		 * need to setup a new range.
		 */
		new = (RANGE *) malloc(sizeof(*new));
		new->start = ppnum + 1;
		new->length = range->start + range->length -
				new->start;
		new->newstart = new->newlength = 0;
		new->newnext = NULL;
		new->intheregion = new->start;
		new->region_length = range->intheregion + range->region_length -
					new->start;
		new->next_range = range->next_range;

		/*
		 * adjust old range
		 */
		range->length -= new->length + 1;
		range->region_length -= new->region_length + 1;
		range->next_range = new;
		}

	/*
	 * assign pp and pv to the layout
	 */
	layout->pp_num = ppnum;
	ASSIGN(layout->pv_id, pv->pvid);

	/*
	 * add this lpnum to the current pv's
	 * lpnums array
	 */
	pv->lpnums[pv->totallpcount++] = layout->lp_num;
	qsort((void *) pv->lpnums, pv->totallpcount, sizeof(int), increasing);

	pv->free_pp_count--;
}
