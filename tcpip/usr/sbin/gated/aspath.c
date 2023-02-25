static char sccsid[] = "@(#)62	1.1  src/tcpip/usr/sbin/gated/aspath.c, tcprouting, tcpip411, GOLD410 12/6/93 17:24:44";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: ATTR_ISSET
 *		ATTR_SET
 *		PATHHASH
 *		PATH_FIND_PATH
 *		PATH_FREE_PATH
 *		PATH_GET_OSPF_TAG
 *		__PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF4
 *		__PF5
 *		__PF6
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * aspath.c,v 1.21.2.1 1993/06/09 00:36:04 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */

/*
 * This stuff supports the decoding and manipulation of AS path attribute
 * data in the form BGP version 2/3 (and better?) likes it.  It is implemented
 * as a separate module since it is expected that other protocols (e.g.
 * IS-IS?) may be pressed into service to carry this gunk between border
 * gateways.
 *
 * The code maintains the (variable length) path attributes in a number
 * of fixed sized structures which are obtained using the task_block*()
 * facility.  For attributes to large to fit in these we malloc() enough
 * space and free when we no longer need them.  There is code here to read
 * raw attribute data into structures and return a pointer to that structure.
 * The code also maintains this stuff so that only one structure exists for
 * each distinct set of AS path attributes, this allowing users to test for
 * attribute equality by comparing pointers.
 *
 * The code is intended to be fully general, but is greatly optimized for
 * situations where the longest attribute list doesn't exceed the size of
 * the largest fixed-size structure we allocate (if you get more than a
 * few that do, change the code to allocate larger ones) and where there
 * are either no AS path attributes we don't understand, or where the
 * unknown attributes are sorted into ascending order when they arrive.
 */

#include "include.h"
#ifdef	PROTO_INET
#include "inet.h"
#ifdef PROTO_OSPF
#include "ospf.h"
#ifdef	notdef
#include "ospf_rtab.h"
#include "ospf_gated.h"
#endif
#endif	/* PROTO_OSPF */
#ifdef PROTO_BGP
#include "bgp.h"
#endif	/* PROTO_BGP */
#endif	/* PROTO_INET */


/*
 * Memory allocation.  We allocate structures with 32 and 64 byte
 * data areas.  Note our use of the data area is sloppy, this to
 * avoid having to copy anything.
 */
static struct path_size_list {
    u_short len;		/* length of data area */
    block_t block_indx;		/* block index for this size */
    int num_allocated;
} path_size_list[] = {
	{ 32,	0,	0 },
	{ 64,	0,	0 }
};

#define	NUMPATHSIZES	(sizeof (path_size_list)/sizeof (struct path_size_list))

/*
 * Statistics for tuning.
 */
int path_num_mallocs = 0;
int path_num_frees = 0;
int path_max_size = 0;

/*
 * We hash the path attributes into buckets to avoid long searches
 * and many comparisons.  We use the low order bits of the most distant
 * AS in the path for this.
 */
#define	PATHHASHSIZE	32
#define	PATHHASHMASK	(PATHHASHSIZE-1)
#define	PATHHASH(asp)	(((asp)->path_len == 0) \
			 ? (u_int) ((asp)->path_as & PATHHASHMASK) \
			 : (u_int) (*(PATH_ATTR_PTR((asp))-1) & PATHHASHMASK))

static as_path *path_list[PATHHASHSIZE] = { 0 };
static int path_list_members = 0;	/* number of paths we know about */


/*
 * Structures that are on the hash list are identified by an integer
 * path ID.  This is defined here.
 */
static u_long lastpathid = 0;

/*
 * Free a path structure.  We do this a fair bit.
 */
#define	PATH_FREE_PATH(pathp) \
    do { \
	if ((pathp) == lastpathfind) { \
	    lastpathfind = NULL; \
	} \
	if ((pathp)->path_size == 0) { \
	    path_num_frees++; \
	    task_mem_free(path_task, (caddr_t) (pathp)); \
	} else { \
	    register struct path_size_list *pslp; \
	    pslp = &path_size_list[(pathp)->path_size-1]; \
	    pslp->num_allocated--; \
	    task_block_free(pslp->block_indx, (void_t) (pathp)); \
	} \
    } while(0)

/*
 * Find a path structure in the hash list.  The hash argument ends up set
 * to the path hash, the asp argument to NULL if the path isn't found.
 */
#define	PATH_FIND_PATH(pathp, hash, asp) \
    do { \
	hash = PATHHASH((pathp)); \
	for ((asp) = path_list[hash]; (asp); (asp) = (asp)->path_next) { \
	    if (PATH_SAME((pathp), (asp))) { \
		break; \
	    } \
	} \
    } while (0)


/*
 * A teeny tiny optimization.  If we search for a path (in aspath_find())
 * and don't find it, remember the pointer here so we can avoid a second
 * search later.  If we add stuff to the hash table, or delete this
 * particular path, the pointer is NULLed.
 */
static as_path *lastpathfind = NULL;

/*
 * The task pointer is used as an indicator that we are already
 * initialized.
 */
static task *path_task = (task *) 0;

/*
 * We keep track of received attributes in a bit array so we can
 * figure out if something is sent twice.  These are in aid of
 * that end.
 */
#define	ATTR_BITARRAY_SIZE	(256 / NBBY)
#define	ATTR_SET(attr, a)	((a)[(attr) / NBBY] |= 1 << ((attr) % NBBY))
#define ATTR_ISSET(attr, a)	((a)[(attr) / NBBY] & 1 << ((attr) % NBBY))

/*
 * Gunk for pretty printing path attributes
 */
static const bits path_Orgs[] = {
	{ PATH_ORG_IGP,	"IGP" },
	{ PATH_ORG_EGP,	"EGP" },
	{ PATH_ORG_XX,	"Incomplete" },
	{ 0 }
};


/*
 * Some external stuff we may use someday.
 */
flag_t aspath_trace_flags = 0;	/* trace flags from parser(?) */

/*
 * Protocol-specific operations.  Only OSPF requires these for now.
 */
#ifdef PROTO_OSPF
#define	PATH_GET_OSPF_TAG(rt)	rt->rt_tag
#endif /* PROTO_OSPF */


/*
 * path_id_comp - called by qsort() when sorting paths by ID
 */
static int
aspath_id_comp __PF2(v1, void_t,
		     v2, void_t)
{
    as_path *pa1 = *((as_path **) v1);
    as_path *pa2 = *((as_path **) v2);

    if (pa1->path_id < pa2->path_id) {
	return -1;
    } else if (pa1->path_id > pa2->path_id) {
	return 1;
    } else {
	return 0;
    }
}


/*
 * aspath_dump_all - dump AS path info
 */
static void
aspath_dump_all __PF2(tp, task *,
		      fd, FILE *)
{
    register int i, nb;
    register byte *cp;
    register as_path *pa;
    register as_path **p_list;
    register int as;
    size_t needmem;

    /*
     * If no path data present, print this.  Else acquire enough
     * memory to sort the list of attributes by path ID
     */
    if (path_list_members == 0) {
	fprintf(fd, "\tNo path attributes in data base\n");
	return;
    }

    needmem = (size_t)(path_list_members * sizeof(as_path *));
    if (needmem <= (size_t) task_send_buffer_len) {
	p_list = task_get_send_buffer(as_path **);
    } else {
        p_list = (as_path **) task_mem_malloc(tp, needmem);
    }

    nb = 0;
    for (i = 0; i < PATHHASHSIZE; i++) {
	pa = path_list[i];
	while (pa) {
	    if (nb >= path_list_members) {
		nb++;
	    } else {
		p_list[nb++] = pa;
		pa = pa->path_next;
	    }
	}
    }

    assert(nb == path_list_members);

    qsort((caddr_t) p_list, nb, sizeof (pa), aspath_id_comp);

    /*
     * We now have the list sorted.  Run through and print the vital
     * statistics for each member.
     */
    for (nb = 0; nb < path_list_members; nb++) {
	pa = p_list[nb];
	fprintf(fd,
		"\tId %lu\tRefcount %d  Lengths: Path %d, Attr %d, Alloc'd %d\n\t",
		pa->path_id,
		pa->path_refcount,
		pa->path_len,
		pa->path_attr_len,
		(pa->path_size > 0)
		? path_size_list[pa->path_size - 1].len
		: (pa->path_len + pa->path_attr_len));
	cp = PATH_PTR(pa);
	if (pa->path_as) {
	    fprintf(fd, "\tAS: %d",
		    pa->path_as);
	}
	fprintf(fd, "\tPath: ");
	for (i = pa->path_len/2; i > 0; i--) {
	    as = (int)(*cp++) << 8;
	    as += (int)(*cp++);
	    fprintf(fd, "%d ", as);
	}
	switch (pa->path_origin) {
	case PATH_ORG_IGP:
	    fprintf(fd, "IGP\n");
	    break;

	case PATH_ORG_EGP:
	    fprintf(fd, "EGP\n");
	    break;

	case PATH_ORG_XX:
	    fprintf(fd, "Incomplete\n");
	    break;

	default:
	    fprintf(fd, "<0x%02x>\n",
		    pa->path_origin);
	    break;
	}

	while (cp < PATH_ATTR_PTR(pa) + pa->path_attr_len) {
	    int flags, code, len;

	    GET_PATH_ATTR(flags, code, len, cp);
	    fprintf(fd, "\t\tAttr flags %02x code %02x:",
		    flags,
		    code);
	    while (len-- > 0)
		fprintf(fd, " %02x", *cp++);
	    fprintf(fd, "\n");
	}
    }

    /*
     * Shit, glad that's done.  Free up the list space and return
     */
    if (needmem > (size_t) task_send_buffer_len) {
	(void) task_mem_free(tp, (caddr_t) p_list);
    }
}


/*
 * aspath_dump - pretty print an AS path for the sake of someone else's
 *	       dump routine
 */
void
aspath_dump __PF3(fd, FILE *,
		  pa, as_path *,
		  str, const char *)
{

    (void) fprintf(fd, "%sAS Path:", str);

    if (pa->path_as) {
	(void) fprintf(fd, " (%u) ",
		       pa->path_as);
    }
    
    if (pa->path_len) {
	register byte *cp;
	register int i, as;

	cp = PATH_PTR(pa);
	for (i = pa->path_len/2; i > 0; i--) {
	    as = (int)(*cp++) << 8;
	    as += (int)(*cp++);
	    (void) fprintf(fd, " %u", as);
	}
    }
    (void) fprintf(fd, " ");

    switch (pa->path_origin) {
    case PATH_ORG_IGP:
	fprintf(fd, "IGP (Id %lu)\n",
		pa->path_id);
	break;

    case PATH_ORG_EGP:
	fprintf(fd, "EGP (Id %lu)\n",
		pa->path_id);
	break;

    case PATH_ORG_XX:
	fprintf(fd, "Incomplete (Id %lu)\n",
		pa->path_id);
	break;

    default:
	fprintf(fd, "%02x (Id %lu)\n",
		pa->path_origin,
		pa->path_id);
	break;
    }
}



/*
 * aspath_trace - write out raw AS path data when told to
 */
void
aspath_trace __PF4(tflags, flag_t,
		   comment, const char *,
		   bufp, byte *,
		   buflen, int)
{
    register byte *cp;
    register byte *end;
    register int i;
    register int code;
    register int flags;
    register int len;
    byte *bpattr;
    int defprint;
    sockaddr_un addr;

    sockclear_in(&addr);
    cp = bufp;
    end = cp + buflen;

    while (cp < end) {
	if (PATH_ATTR_MINLEN(*cp) > (end - cp)) {
	    trace(tflags, 0,
		  "%sshort attribute: flags 0x%02x, %d data bytes left",
		  comment,
		  (int) *cp,
		  (end - cp));
	    return;
	}

	GET_PATH_ATTR(flags, code, len, cp);
	if (len > (end - cp)) {
	    trace(tflags, 0,
		  "%sshort attribute data: flags 0X%02x, code %d, length %d (> %d)",
		  comment,
		  flags,
		  code,
		  len,
		  (end - cp));
	    return;
	}

	defprint = 1;
	switch (code) {
	case PA_TYPE_ORIGIN:
	    if (len != 1) {
		break;
	    }
	    i = (int)*cp++;
	    trace(tflags, 0, "%sflags 0x%02x code Origin(%d): %s",
		  comment, flags, code, trace_state(path_Orgs, i));
	    defprint = 0;
	    break;

	case PA_TYPE_ASPATH:
	    if (len & 0x01) {
		break;
	    }
	    tracef("%sflags 0x%02x code AS_Path(%d):",
		   comment, flags, code);

	    len >>= 1;
	    defprint = 0;
	    if (len == 0) {
		trace(tflags, 0, " <null>");
		break;
	    }

	    while (len-- > 0) {
		i = (*cp++) << 8;
		i |= *cp++;
		tracef(" %d", i);
	    }
	    trace(tflags, 0, "");
	    break;

	case PA_TYPE_NEXTHOP:
	    if (len != 4) {
		break;
	    }

	    bpattr = (byte *) &sock2ip(&addr);
	    *bpattr++ = *cp++;
	    *bpattr++ = *cp++;
	    *bpattr++ = *cp++;
	    *bpattr++ = *cp++;
	    trace(tflags, 0,
		  "%sflags 0x%02x code Next_Hop(%d): %A",
		  comment,
		  flags,
		  code,
		  &addr);
	    defprint = 0;
	    break;

	case PA_TYPE_UNREACH:
	    if (len != 0) {
		break;
	    }
	    trace(tflags, 0, "%sflags 0x%02x code Unreachable(%d)",
		  comment,
		  flags,
		  code);
	    defprint = 0;
	    break;

	case PA_TYPE_METRIC:
	    if (len != 2) {
		break;
	    }
	    i = (*cp++) << 8;
	    i |= *cp++;
	    trace(tflags, 0, "%sflags 0x%02x code Metric(%d): %d",
		  comment,
		  flags,
		  code,
		  i);
	    defprint = 0;
	    break;
		
	default:
	    break;
	}

	if (defprint) {
	    tracef("%sflags 0x%x02 code %d length %d:",
		   flags,
		   code,
		   len);
	    while (len-- > 0)
		tracef(" %02x", (int)*cp++);
	    trace(tflags, 0, "");
	}
    }
}



/*
 * aspath_init - initialize the path task so that dumps print path info
 */
void
aspath_init __PF0(void)
{
    int i;

    /*
     * The path task is needed only for dumps of the AS path
     * data base.  Allocate the task and list the dump routine.
     */
    if (path_task == (task *)0) {
	path_task = task_alloc("ASPaths", TASKPRI_EXTPROTO);
	path_task->task_trace_flags = aspath_trace_flags;
	path_task->task_dump = aspath_dump_all;
	/* XXX - reinit to reset paths at init time? */
	if (!task_create(path_task)) {
	    task_quit(EINVAL);
	}
	for (i = 0; i < NUMPATHSIZES; i++) {
	    if (path_size_list[i].block_indx == 0) {
		path_size_list[i].block_indx =
		    task_block_init((size_t) (path_size_list[i].len + sizeof (as_path)));
	    }
	}
    } else {
	path_task->task_trace_flags = aspath_trace_flags;
    }

    aspath_match_index = task_block_init(sizeof (asmatch_t));
}


/*
 * aspath_alloc - allocate a path structure of at least the specified length
 */
as_path *
aspath_alloc __PF1(length, int)
{
    register as_path *asp;
    register int ps;

    for (ps = 0; ps < NUMPATHSIZES; ps++) {
	if (path_size_list[ps].len >= length) {
	    /*
	     * Got one of the standard sizes.  Get it and fill
	     * the size in.
	     */
	    asp = (as_path *) task_block_alloc(path_size_list[ps].block_indx);
	    path_size_list[ps].num_allocated++;
	    asp->path_size = ps + 1;
	    return asp;
	}
    }

    /*
     * Just allocate a big one which is the correct size.
     */
    asp = (as_path *) task_mem_calloc(path_task,
				      1,
				      (size_t) (sizeof (as_path) + length));
    path_num_mallocs++;
    return asp;
}


/*
 * aspath_free - free a path structure which may have been obtained from
 *   the tack_block stuff or may have been malloc'd, if the path is
 *   unreferenced.
 */
void
aspath_free __PF1(pathp, as_path *)
{
    if (pathp->path_refcount == 0) {
	PATH_FREE_PATH(pathp);
    }
}


/*
 * aspath_unlink - free a path structure which may have be obtained from
 *	       the task_block stuff or may have been malloc'd.  And
 *	       which may or may not be hashed.
 */
void
aspath_unlink __PF1(pathp, register as_path *)
{
    register as_path *asp;
    register u_int hash;

    if (pathp->path_refcount > 0) {
	/*
	 * This one is hooked into the hash list.  If
	 * there are multiple references, decrement the
	 * reference count and return.
	 */
	if (--(pathp->path_refcount) > 0) {
	    return;
	}

	/*
	 * Find and remove structure from the hash list.
	 */
	hash = PATHHASH(pathp);
	if (pathp == path_list[hash]) {
	    path_list[hash] = pathp->path_next;
	} else {
	    for (asp = path_list[hash]; asp; asp = asp->path_next) {
		if (asp->path_next == pathp) {
		    break;
		}
	    }
	    /* Could not find on hash list */
	    assert(asp);
	    asp->path_next = pathp->path_next;
	}

	/*
	 * Keep count of the number of structures in the path list.
	 */
	path_list_members--;

	/*
	 * Check to see if this is the last path ID we assigned.  If
	 * so, decrement the path ID.
	 */
	if (pathp->path_id == lastpathid) {
	    lastpathid--;
	}
    }

    /*
     * Now free the path structure.
     */
    PATH_FREE_PATH(pathp);
}


/*
 * aspath_find - search for an existing path in the table which matches
 *   the argument.  If found, free the argument path and return the
 *   linked one instead.  A path which has been found by aspath_find()
 *   should always be the one returned by aspath_rt_link() or aspath_rt_build().
 */
as_path *
aspath_find __PF1(pathp, register as_path *)
{
    register as_path *asp;
    register u_int hash;

    if (pathp->path_refcount == 0) {
	PATH_FIND_PATH(pathp, hash, asp);

	/*
	 * If one was found, free the old one and return this instead.
	 */
	if (asp) {
	    PATH_FREE_PATH(pathp);
	    return (asp);
	}

	/*
	 * Remember that we didn't find this path so we can perhaps
	 * avoid another search.
	 */
	lastpathfind = pathp;
    }

    /*
     * Return the path we were called with.
     */
    return (pathp);
}


inline static void
aspath_rt_unlink __PF1(ul_rt, rt_entry *)
{
    if (ul_rt->rt_aspath) {
	aspath_unlink(ul_rt->rt_aspath);
	ul_rt->rt_aspath = (as_path *) 0;
    }
}


/*
 *	aspath_rt_link - Verify that
 */
inline static void
aspath_rt_link __PF2(pl_rt, rt_entry *,
		     pathp, register as_path *)
{
    /* Release the previous path */
    aspath_rt_unlink(pl_rt);

    if (pathp->path_refcount) {
	/*
	 * Existing path, link it to this route
	 */
	pl_rt->rt_aspath = pathp;
    } else {
	/*
	 * Linking a new path, see if one like it already exists.  If
	 * this was searched for by the last call to aspath_find(), however,
	 * don't bother.
	 */
	register u_int hash = 0;
	register as_path *pl_asp;
	
	if (lastpathfind == pathp) {
	    pl_asp = NULL;
	    hash = PATHHASH(pathp);
	} else {
	    PATH_FIND_PATH(pathp, hash, pl_asp);
	}

	if (pl_asp) {
	    /*
	     * Found one the same.  Delete the one given as an
	     * argument and return this one instead with the
	     * reference count bumped.
	     */
	    PATH_FREE_PATH(pathp);
	    pl_rt->rt_aspath = pl_asp;
	} else {
	    /*
	     * This is a new one.  Assign it a path_id, set the reference
	     * count to 1 and hook it into the hash chain.  Then return it.
	     */
	    pathp->path_id = ++lastpathid;
	    pathp->path_next = path_list[hash];
	    path_list[hash] = pathp;
	    path_list_members++;
	    pl_rt->rt_aspath = pathp;
	    lastpathfind = NULL;	/* indicate hash list altered */
	}
    }

    pl_rt->rt_aspath->path_refcount++;
}


void
aspath_rt_free __PF1(rt, rt_entry *)
{
    /* Release the path */
    aspath_rt_unlink(rt);

#if	defined(PROTO_BGP) && defined(PROTO_OSPF)
    if (rt->rt_gwp->gw_proto == RTPROTO_BGP &&
      BIT_TEST(rt->rt_gwp->gw_flags, GWF_AUXPROTO)) {
	rt_entry *rt1;
	u_long id = ((bgpPeer *)(rt->rt_gwp->gw_task->task_data))->bgp_id;

	RT_ALLRT(rt1, rt->rt_head) {
	    /* XXX need to check for group membership someday */
	    if (rt1->rt_gwp->gw_proto == RTPROTO_OSPF_ASE &&
		id == ORT_ADVRTR(rt1)) {
		/* Null path */
		assert(rt1->rt_aspath);
		(void) rt_change_aspath(rt1,
					rt1->rt_metric,
					rt1->rt_tag,
					rt1->rt_preference,
					0,
					(sockaddr_un **)0,
					(as_path *) 0);
		return;
	    }
	} RT_ALLRT_END(rt1, rt->rt_head) ;
    }
#endif	/* defined(PROTO_BGP) && defined(PROTO_OSPF) */
}


/*
 * aspath_insert_aspath - insert the AS path in front of the attributes
 */
static void
aspath_insert_aspath __PF3(asp, as_path *,
			   pathp, byte *,
			   len, int)
{
    register byte *bpi, *bpo;
    register int i;

    /*
     * Make space for the path list.  Do the copy from the end
     * since the buffers may overlap.
     */
    i = asp->path_attr_len;
    bpi = PATH_PTR(asp) + i;
    bpo = bpi + len;
    while (i-- > 0) {
	*(--bpo) = *(--bpi);
    }

    /*
     * Now copy the path into the space just made
     */
    asp->path_len = i = len;
    bpi = pathp;
    bpo = PATH_PTR(asp);
    while (i-- > 0) {
	*bpo++ = *bpi++;
    }
}


/*
 * aspath_insert_attr - find where to insert an attribute into the list.
 *		      Do it.
 */
static void
aspath_insert_attr __PF5(asp, as_path *,
			 newflags, flag_t,
			 newcode, int,
			 newlen, int,
			 datap, byte *)
{
    register byte *bp, *bpend;
    register byte *bpattr;
    int flags, code, len;

    /*
     * Find where to insert the attribute.
     */
    bp = PATH_ATTR_PTR(asp);
    bpattr = bp;
    bpend = bp + asp->path_attr_len;
    while (bp < bpend) {
	GET_PATH_ATTR(flags, code, len, bp);
	if (code > newcode) {
	    break;
	}
	bp += len;
	bpattr = bp;
    }

    /*
     * If this wasn't the end of the list, we need to make space
     * for the new attributes.  bpattr points to where the space
     * needs to be made.
     */
    if (bpattr < bpend) {
	bp = bpend + PATH_ATTR_LEN(newlen);
	while (bpend > bpattr) {
	    *(--bp) = *(--bpend);
	}
    }

    /*
     * By now, one way or another, bpend points to the place to insert
     * this.  Copy it in.
     */
    PATH_PUT_ATTR(newflags, newcode, newlen, bpend);
    bpattr = datap;
    while (bpend < bp) {
	*bpend++ = *bpattr++;
    }
    asp->path_attr_len += PATH_ATTR_LEN(newlen);
}


/*
 * path_has_loop - check to see if the AS path attached to this has a loop
 */
static int
aspath_has_loop __PF1(asp, as_path *)
{
    register u_short pval;
    register u_short *p, *t;
    register u_short *pend;

    if (asp->path_len <= 2)
	return 0;

    p = PATH_SHORT_PTR(asp);
    pend = p + (asp->path_len / 2) - 1;
    while (p < pend) {
	pval = *p++;
	for (t = p + 1; t <= pend; t++) {
	    if (*t == pval) {
		return 1;
	    }
	}
    }
    return 0;
}


/*
 * aspath_attr - decode path attributes and return a pointer to an
 *	       attribute structure
 */
as_path *
aspath_attr __PF6(bufp, byte *,
		  buflen, int,
		  my_as, as_t,
		  api, as_path_info *,
		  error_code, int *,
		  error_data, byte **)
{
    register byte *bp, *bpend;
    register byte *ap;
    register as_path *asp;
    register byte *bpattr;
    flag_t flags;
    int code, len;
    int lastcode;
    byte received[ATTR_BITARRAY_SIZE];
    static byte missing_attr;

    /*
     * Local macro.  We do this alot.
     */
#define	PA_RETURN_ERROR(code, data) \
    *error_code = (code); \
    *error_data = (data); \
    goto belch

    /*
     * Initialize return values
     */
    api->api_flags = 0;
    lastcode = 0;

    /*
     * Initialize received array
     */
    bzero((caddr_t) received, sizeof (received));

    /*
     * Initialize buffer pointers
     */
    bp = bufp;
    bpend = bp + buflen;

    /*
     * Get an attribute structure with buflen bytes of data area.
     * Note that we will waste some of this since the total length
     * we store in here will certainly be less than buflen bytes.
     * Maybe try to do this better later on.
     */
    asp = aspath_alloc(buflen);
    ap = PATH_PTR(asp);

    /*
     * Include the AS of the caller.  If we are running in several
     * ASes the AS paths should be treated as distinct.
     */
    asp->path_as = my_as;

    /*
     * Walk through the buffered data, picking off and processing
     * individual attributes.
     */
    while (bp < bpend) {
	/*
	 * Make sure we've got enough data for the
	 * flags-code-length triple.
	 */
	if ((bpend - bp) < PATH_ATTR_MINLEN(*bp)) {
	    /*
	     * XXX This error wasn't considered in the spec.  Use
	     * malformed for now.
	     */
	    PA_RETURN_ERROR(PA_ERR_MALFORMED, NULL) ;
	}

	/*
	 * Save the start of the data for error reporting, then
	 * decode flags-code-length.
	 */
	bpattr = bp;
	GET_PATH_ATTR(flags, code, len, bp);

	/*
	 * XXX If there is insufficient data in the packet for the
	 * length given, return an error.
	 */
	if ((bpend - bp) < len) {
	    PA_RETURN_ERROR(PA_ERR_MALFORMED, NULL) ;
	}

	/*
	 * Check to see if we got this one before.  If so this
	 * is an error.
	 */
	if (ATTR_ISSET(code, received)) {
	    PA_RETURN_ERROR(PA_ERR_MALFORMED, NULL) ;
	}
	ATTR_SET(code, received);

	/*
	 * Check for the easy flags error
	 */
	if (!BIT_TEST(flags, PA_FLAG_OPT)) {
	    if (!BIT_TEST(flags, PA_FLAG_TRANS) || BIT_TEST(flags, PA_FLAG_PARTIAL)) {
		PA_RETURN_ERROR(PA_ERR_FLAGS, bpattr) ;
	    }
	}

	/*
	 * Do code-specific processing
	 */
	switch (code) {
	case PA_TYPE_INVALID:
	    /*
	     * I think this is always invalid.  XXX No suitable
	     * error for this either.
	     */
	    PA_RETURN_ERROR(PA_ERR_MALFORMED, NULL) ;
	    /*NOTREACHED*/

	case PA_TYPE_ORIGIN:
	    if (len != 1) {
		PA_RETURN_ERROR(PA_ERR_LENGTH, bpattr) ;
	    }
	    if (BIT_TEST(flags, PA_FLAG_OPT)) {
		PA_RETURN_ERROR(PA_ERR_FLAGS, bpattr) ;
	    }
	    asp->path_origin = *bp++;
	    if (asp->path_origin != PATH_ORG_IGP &&
		asp->path_origin != PATH_ORG_EGP &&
		asp->path_origin != PATH_ORG_XX) {
		PA_RETURN_ERROR(PA_ERR_ORIGIN, bpattr) ;
	    }
	    break;

	case PA_TYPE_ASPATH:
	    if (len & 0x1) {
		/* Odd length? */
		PA_RETURN_ERROR(PA_ERR_LENGTH, bpattr) ;
	    }
	    if (BIT_TEST(flags, PA_FLAG_OPT)) {
		/* Opt flag set? */
		PA_RETURN_ERROR(PA_ERR_FLAGS, bpattr) ;
	    }
	    if (asp->path_attr_len == 0) {	/* always true? */
		/*
		 * Paths should be short.  Do the copy
		 * manually, as this also saves updating
		 * ap and bp.
		 */
		asp->path_len = len;
		while (len-- != 0) {
		    *ap++ = *bp++;
		}
	    } else {
		aspath_insert_aspath(asp, bp, len);
		bp += len;
		ap += len;
	    }

	    /*
	     * Check the path for a loop now while we still
	     * have the pointer to the attribute.  Only check
	     * if there is more than 1 AS in the list
	     */
	    if (asp->path_len > 2 && aspath_has_loop(asp)) {
		PA_RETURN_ERROR(PA_ERR_ASLOOP, bpattr) ;
	    }
	    break;

	case PA_TYPE_NEXTHOP:
	    if (len != 4) {
		/* Invalid length? */
		PA_RETURN_ERROR(PA_ERR_LENGTH, bpattr) ;
	    }
	    if (BIT_TEST(flags, PA_FLAG_OPT)) {
		/* Opt flag set? */
		PA_RETURN_ERROR(PA_ERR_FLAGS, bpattr) ;
	    }
	    /*
	     * Copy address in.  It's already in network
	     * byte order.  Use bpattr as a tmp.
	     */
	    BIT_SET(api->api_flags, APIF_NEXTHOP);
	    sockclear_in(api->api_nexthop);
	    bpattr = (byte *) &sock2ip(api->api_nexthop);
	    *bpattr++ = *bp++;
	    *bpattr++ = *bp++;
	    *bpattr++ = *bp++;
	    *bpattr++ = *bp++;

	    /*
	     * Don't bother checking this here.  We'll
	     * leave that to the consumer of the data.
	     */
	    break;

	case PA_TYPE_UNREACH:
	    if (len != 0) {
		/* Invalid length? */
		PA_RETURN_ERROR(PA_ERR_LENGTH, bpattr) ;
	    }
	    if (BIT_TEST(flags, PA_FLAG_OPT)) {
		/* Opt flag set? */
		PA_RETURN_ERROR(PA_ERR_FLAGS, bpattr) ;
	    }
	    BIT_SET(api->api_flags, APIF_UNREACH);
	    break;
	    
	case PA_TYPE_METRIC:
	    if (len != 2) {
		/* Invalid length? */
		PA_RETURN_ERROR(PA_ERR_LENGTH, bpattr) ;
	    }
	    if ((flags & PA_FLAG_ALL) != PA_FLAG_OPT) {
		PA_RETURN_ERROR(PA_ERR_FLAGS, bpattr) ;
	    }
	    BIT_SET(api->api_flags, APIF_METRIC);
	    api->api_metric = (*bp++) << 8;
	    api->api_metric |= (*bp++);
	    break;

	default:
	    /*
	     * Everything we don't know about comes here.
	     * The OPTIONAL flag must be set (since we don't
	     * know it).  If the TRANSITIVE flag is clear
	     * we simply ignore the attribute (after making
	     * sure the PARTIAL bit wasn't set).  If the
	     * TRANSITIVE flag is set we set the PARTIAL bit
	     * and insert in the buffer in the proper spot.
	     */
	    if (!BIT_TEST(flags, PA_FLAG_OPT)) {
		PA_RETURN_ERROR(PA_ERR_UNKNOWN, bpattr) ;
	    }
	    if (!BIT_TEST(flags, PA_FLAG_TRANS)) {
		if (BIT_TEST(flags, PA_FLAG_PARTIAL)) {
		    PA_RETURN_ERROR(PA_ERR_FLAGS, bpattr) ;
		}
		break;	/* Forget it */
	    }
	    BIT_SET(flags, PA_FLAG_PARTIAL);
	    if (code > lastcode) {		/* fast path */
		asp->path_attr_len += PATH_ATTR_LEN(len);
		PATH_PUT_ATTR(flags, code, len, ap);
		while (len-- > 0) {
		    *ap++ = *bp++;
		}
		lastcode = code;
	    } else {
		aspath_insert_attr(asp, flags, code, len, bp);
		bp += len;
		ap += PATH_ATTR_LEN(len);
	    }
	    break;
	}
    }

    /*
     * From our point of view there are only two mandatory attributes,
     * origin and AS path.  Return an error if either of these didn't
     * show up.
     */
    if (!ATTR_ISSET(PA_TYPE_ORIGIN, received) ||
	!ATTR_ISSET(PA_TYPE_ASPATH, received)) {
	if (!ATTR_ISSET(PA_TYPE_ORIGIN, received)) {
	    missing_attr = PA_TYPE_ORIGIN;
	} else {
	    missing_attr = PA_TYPE_ASPATH;
	}
	PA_RETURN_ERROR(PA_ERR_MISSING, &missing_attr) ;
    }
    
    /*
     * Done!  Return the pointer.
     */
    return asp;

    /*
     * We come here for errors.  Free the attribute structure and return
     * nothing.
     */
belch:
    aspath_unlink(asp);
    return NULL;

#undef	PA_RETURN_ERROR
}


/*
 * aspath_format - format a set of path attributes into an outgoing message.
 *   This routine is responsible for ensuring that the AS path is updated
 *   appropriately in the outgoing message.
 */
byte *
aspath_format __PF5(myas, as_t,
		    asp, as_path *,
		    asip, as_path_info *,
		    next_hop_ptr, byte **,
		    bufp, byte *)
{
    register byte *cp;
    u_int len;

    /*
     * If it is a minimum unreachable packet, do this quickly.  Otherwise
     * we do it the long way.  We write the attributes into the packet in
     * ascending numeric order.
     */
    cp = bufp;
    if (BIT_TEST(asip->api_flags, APIF_UNREACH) && asp == NULL) {
	/*
	 * First the origin
	 */
	PATH_PUT_ATTR(PA_FLAG_TRANS, PA_TYPE_ORIGIN, PA_LEN_ORIGIN, cp);
	*cp++ = PATH_ORG_XX;

	/*
	 * Next the AS path.  This includes the local AS if this is
	 * external, or a zero-length AS path otherwise.
	 */
	if (BIT_TEST(asip->api_flags, APIF_INTERNAL)) {
	    PATH_PUT_ATTR(PA_FLAG_TRANS, PA_TYPE_ASPATH, 0, cp);
	} else {
	    PATH_PUT_ATTR(PA_FLAG_TRANS, PA_TYPE_ASPATH, PATH_ATTR_AS_SIZE, cp);
	    PATH_PUT_AS(myas, cp);
	}

	/*
	 * Next the next_hop.  If he didn't include one in the path_info
	 * just skip the field.
	 */
	PATH_PUT_ATTR(PA_FLAG_TRANS, PA_TYPE_NEXTHOP, PA_LEN_NEXTHOP, cp);
	if (next_hop_ptr != NULL) {
	    *next_hop_ptr = cp;
	}
	if (BIT_TEST(asip->api_flags, APIF_NEXTHOP)) {
	    PATH_PUT_NEXTHOP(asip->api_nexthop, cp);
	} else {
	    cp += PA_LEN_NEXTHOP;
	}

	/*
	 * Last is the unreachable attribute.
	 */
	PATH_PUT_ATTR(PA_FLAG_TRANS, PA_TYPE_UNREACH, PA_LEN_UNREACH, cp);
    } else {
        int add_path_as = 0;

	/*
	 * Origin
	 */
	PATH_PUT_ATTR(PA_FLAG_TRANS, PA_TYPE_ORIGIN, PA_LEN_ORIGIN, cp);
	*cp++ = asp->path_origin;

	/*
	 * AS path.  We've got to compute the length once we've massaged
	 * the thing, so figure out what we need to do now.
	 */
	len = asp->path_len;
	if (asp->path_as != 0 && asp->path_as != myas) {
	    add_path_as = 1;
	    len += PATH_ATTR_AS_SIZE;
	}
	if (!BIT_TEST(asip->api_flags, APIF_INTERNAL)) {
	    len += PATH_ATTR_AS_SIZE;
	}

	PATH_PUT_ATTR(PA_FLAG_TRANS, PA_TYPE_ASPATH, len, cp);
	if (!BIT_TEST(asip->api_flags, APIF_INTERNAL)) {
	    PATH_PUT_AS(myas, cp);
	}
	if (add_path_as) {
	    PATH_PUT_AS(asp->path_as, cp);
	}
	bcopy((caddr_t)PATH_PTR(asp), (caddr_t)cp, (size_t)asp->path_len);
	cp += asp->path_len;

	/*
	 * Next hop.
	 */
	PATH_PUT_ATTR(PA_FLAG_TRANS, PA_TYPE_NEXTHOP, PA_LEN_NEXTHOP, cp);
	if (next_hop_ptr != NULL) {
	    *next_hop_ptr = cp;
	}
	if (BIT_TEST(asip->api_flags, APIF_NEXTHOP)) {
	    PATH_PUT_NEXTHOP(asip->api_nexthop, cp);
	} else {
	    cp += PA_LEN_NEXTHOP;
	}

	/*
	 * Unreachable
	 */
	if (BIT_TEST(asip->api_flags, APIF_UNREACH)) {
	    PATH_PUT_ATTR(PA_FLAG_TRANS, PA_TYPE_UNREACH, PA_LEN_UNREACH, cp);
	}

	/*
	 * Metric
	 */
	if (BIT_TEST(asip->api_flags, APIF_METRIC) && asip->api_metric != -1) {
	    PATH_PUT_ATTR(PA_FLAG_OPT, PA_TYPE_METRIC, PA_LEN_METRIC, cp);
	    PATH_PUT_METRIC(asip->api_metric, cp);
	}

	/*
	 * Now any optional gunk we're carrying.
	 */
	if (asp->path_attr_len > 0) {
	    bcopy((caddr_t)PATH_ATTR_PTR(asp), (caddr_t)cp,
	      (size_t)asp->path_attr_len);
	    cp += asp->path_attr_len;
	}
    }

    /*
     * We're done.  Return.
     */
    return (cp);
}


/*
 * aspath_rt_build - take an arbitrary route and fill in the path attribute
 *	  	   pointer to the appropriate structure.
 */
void
aspath_rt_build __PF2(rt, register rt_entry *,
		      asp, register as_path *)
{
    register gw_entry *gwp;
    register as_t as;
    register int origin;
    
    /*
     * If the entry already has a path associated with it, complain
     * and return.
     */
    assert(!rt->rt_aspath);

    /*
     * Fetch a pointer to the gw entry.  Evaluate what we need to
     * do based on protocol (external OSPF is the only one we worry
     * about specially at this point).
     */
    gwp = rt->rt_gwp;

    /*
     * If the aspath pointer is non-null this is essentially an
     * aspath_rt_link().  If the route came from an IBGP neighbour, however,
     * we need to search through looking for the corresponding OSPF_ASE
     * route.  If we find it we have some work to do.
     */
    if (asp) {
	aspath_rt_link(rt, asp);
#if	defined(PROTO_BGP) && defined(PROTO_OSPF)
	if (gwp->gw_proto == RTPROTO_BGP
	  && BIT_TEST(gwp->gw_flags, GWF_AUXPROTO)) {
	    rt_entry *rt1;
	    u_long id = ((bgpPeer *)(gwp->gw_task->task_data))->bgp_id;

	    RT_ALLRT(rt1, rt->rt_head) {
	        /* XXX need to check for group membership someday */
		if (rt1->rt_gwp->gw_proto == RTPROTO_OSPF_ASE
		  && id == ORT_ADVRTR(rt1)) {
		    (void) rt_change_aspath(rt1,
					    rt1->rt_metric,
					    rt1->rt_tag,
					    rt1->rt_preference,
					    0,
					    (sockaddr_un **)0,
					    asp);
		    return;
		}
	    } RT_ALLRT_END(rt1, rt->rt_head) ;
	}
#endif	/* defined(PROTO_BGP) && defined(PROTO_OSPF) */
	return;
    }

    switch (gwp->gw_proto) {
#ifdef PROTO_OSPF
    case RTPROTO_OSPF_ASE:
	/*
	 * We'll need to check out the tag.  Fetch it from the OSPF
	 * data base.  This is a crock of shit, I wish the tag definitions
	 * were closer to real life.
	 */
        {
	    u_long tag = PATH_GET_OSPF_TAG(rt);
	    
	    if (PATH_OSPF_ISTRUSTED(tag)) {
		switch(tag & PATH_OSPF_TAG_LEN_MASK) {
		case PATH_OSPF_TAG_LEN_0:
		    as = 0;
		    if (PATH_OSPF_ISCOMPLETE(tag)) {
		    	origin = PATH_ORG_IGP;
		    } else {
		    	origin = PATH_ORG_EGP;
		    }
		    break;

		case PATH_OSPF_TAG_LEN_1:
		    as = tag & PATH_OSPF_TAG_AS_MASK;
		    if (PATH_OSPF_ISCOMPLETE(tag)) {
		    	origin = PATH_ORG_IGP;
		    } else {
		    	origin = PATH_ORG_EGP;
		    }
		    break;

		case PATH_OSPF_TAG_LEN_2:
		    if (PATH_OSPF_ISCOMPLETE(tag)) {
#ifdef	PROTO_BGP
			/*
			 *  The tag says that we need to wait for the IBGP route to get the
			 *  path info.  So look for a valid IBGP route with the same router ID
			 */
			rt_entry *rt1;
			u_long adv_rtr = ORT_ADVRTR(rt);

			RT_ALLRT(rt1, rt->rt_head) {
			    /* XXX need to check for group membership someday */
			    if (rt1->rt_gwp->gw_proto == RTPROTO_BGP &&
				BIT_TEST(rt1->rt_gwp->gw_flags, GWF_AUXPROTO) &&
				!BIT_TEST(rt1->rt_state, RTS_DELETE|RTS_HOLDDOWN) &&
				adv_rtr == ((bgpPeer *)(rt1->rt_gwp->gw_task->task_data))->bgp_id &&
				(asp = rt1->rt_aspath)) {
				/* This is the one!  Bump the refcount and return this path */

    				aspath_rt_link(rt, asp);
				return;
			    }
			} RT_ALLRT_END(rt1, rt->rt_head) ;

			/*
			 * Couldn't find a good route, when the IBGP route arrives it will
			 * fill in the pointer.
			 */
#endif	/* PROTO_BGP */
			return;
		    }

		    as = tag & PATH_OSPF_TAG_AS_MASK;
		    origin = PATH_ORG_XX;
		    if (gwp->gw_local_as != 0 && gwp->gw_local_as == as) {
			as = 0;
		    }
		    break;

		default:	/* really case PATH_OSPF_TAG_LEN_X: */
		    /*
		     * This is invalid, complain and return a NULL path.
		     */
		    trace(TR_ALL, LOG_WARNING,
			  "path_rt: OSPF ASE tag 0x%08x has invalid length",
			  tag);
		    return;
		}
	    } else {
		/*
		 * What can we do here?  Make up a local, incomplete path.
		 */
		as = 0;
		origin = PATH_ORG_XX;
	    }
	}
	break;
#endif /* PROTO_OSPF */

#ifdef PROTO_BGP
    case RTPROTO_BGP:
	/*
	 * This can't happen.  Complain and die.
	 */
	assert(FALSE);
#endif /* PROTO_BGP */	

    default:
	/*
	 * Here we need to decide whether this was received via an EGP
	 * or an IGP and set the tags accordingly.  Note that an EGP
	 * protocol session with the same local and remote AS numbers
	 * is treated as an IGP.  XXX This still screws up in the
 	 * situation where the AS of a remote EGP is not the same
	 * as the local end, but is the same as another "local" AS.  Fix me.
	 */
	as = gwp->gw_peer_as;
	if (as != 0 && as != gwp->gw_local_as) {
	    origin = PATH_ORG_EGP;
	} else {
	    as = 0;
	    origin = PATH_ORG_IGP;
	}
	break;
    }


    /*
     * Okay, we have an origin, and possibly an AS.  Fill in the AS path.
     */
    asp = aspath_alloc(sizeof (as_t));	/* XXX may have to change for IS-IS */
    asp->path_as = gwp->gw_local_as;
    asp->path_origin = origin;
    if (as != 0) {
        register byte *cp = PATH_PTR(asp);
	
	asp->path_len = 2;
	*cp++ = (as >> 8) & 0xff;
	*cp = as & 0xff;
    }

    /*
     * Connect path to route
     */
    aspath_rt_link(rt, asp);
}


/*
 * aspath_prefer - determine which route is preferred based on their AS paths.
 *   Return <0 if the first, >0 if the second, and == 0 if they are the same.
 */
int
aspath_prefer __PF2(rt1, rt_entry *,
		    rt2, rt_entry *)
{
    as_path *asp1, *asp2;

    /*
     * Fetch the AS path pointers.
     */
    asp1 = rt1->rt_aspath;
    asp2 = rt2->rt_aspath;

    /*
     * If both pointers are null, return no preference.  If one is
     * null, return the other.
     */
    if (asp1 == NULL) {
	if (asp2 == NULL) {
	    return (0);
	}
	return (1);
    } else if (asp2 == NULL) {
	return (-1);
    }

    /*
     * If these came from the same AS, and were received by the
     * same protocol, their metrics should be comparable.  This
     * covers the case where they both are from local EGP neighbours,
     * or from local external BGP neighbours, or where one arrived
     * via external BGP and the other arrived via internal BGP after
     * being received from a BGP or EGP neighbour.  Notably absent
     * is the case where one came from local EGP and the other from
     * remote EGP via BGP.
     */
    if (asp1->path_len > 0 && asp2->path_len > 0
      && (*PATH_SHORT_PTR(asp1) == *PATH_SHORT_PTR(asp2))
      && rt1->rt_gwp->gw_proto == rt2->rt_gwp->gw_proto
      && rt1->rt_metric != (-1) && rt2->rt_metric != (-1)) { /*XXX Fix me*/
	if (rt1->rt_metric < rt2->rt_metric) {
	    return (-1);
	} else if (rt1->rt_metric > rt2->rt_metric) {
	    return (1);
	}
    }

    /*
     * If the AS path pointers are different, see if we can make
     * something of the differences.
     */
    if (asp1 != asp2) {
	/*
	 * See if the origins are distinguishable.  If so, return the
	 * one with the smallest numeric code (see codes above).
	 */
	if (asp1->path_origin != asp2->path_origin) {
	    if (asp1->path_origin < asp2->path_origin) {
		return (-1);
	    }
	    return (1);
	}

	/*
	 * See if the AS paths are different lengths.  If so prefer the
	 * shorter one.
	 */
	if (asp1->path_len != asp2->path_len) {
	    if (asp1->path_len < asp2->path_len) {
		return (-1);
	    }
	    return (1);
	}
    }

    /*
     * Here the paths are of equal length and have the same origin (and
     * might be identical).  If one is from an internal protocol and one
     * is from an external protocol, prefer the external version.
     */
    if (rt1->rt_gwp->gw_peer_as == 0
      || rt1->rt_gwp->gw_peer_as == rt1->rt_gwp->gw_local_as) {
	if (rt2->rt_gwp->gw_peer_as != 0
	  && rt2->rt_gwp->gw_peer_as != rt2->rt_gwp->gw_local_as)
	    return (1);
    } else if (rt2->rt_gwp->gw_peer_as == 0
      || rt2->rt_gwp->gw_peer_as == rt2->rt_gwp->gw_local_as) {
	return (-1);
    }

    /*
     * Here we have absolutely no idea of which to prefer.  Let someone
     * else take care of this.
     */
    return (0);
}


#ifdef	PROTO_OSPF
/*
 * aspath_tag_dump - produce a printable version of an OSPF tag
 */
char *
aspath_tag_dump __PF2(as, as_t,
		      tag, u_long)
{
    static char buf[60];	/* as much as we need? */
    const char *origin;
    as_t tag_as;
    u_int tag_tag;

    if (BIT_TEST(tag, PATH_OSPF_TAG_TRUSTED)) {
	/*
	 * Fetch the origin based on the length and the complete bit
	 */
	if ((tag & PATH_OSPF_TAG_LEN_MASK) == PATH_OSPF_TAG_LEN_X) {
	    /* Invalid */
	    (void) sprintf(buf, "Invalid tag: %08x", tag);
	    return (buf);
	} else if ((tag & PATH_OSPF_TAG_LEN_MASK) == PATH_OSPF_TAG_LEN_2) {
	    origin = BIT_TEST(tag, PATH_OSPF_TAG_COMPLETE)
	      ? "IBGP" : "Incomplete";
	} else {
	    origin = BIT_TEST(tag, PATH_OSPF_TAG_COMPLETE) ? "IGP" : "EGP";
	}
	tag_as = tag & PATH_OSPF_TAG_AS_MASK;
	tag_tag = (tag & PATH_OSPF_TAG_USR_MASK) >> PATH_OSPF_TAG_USR_SHIFT;
	(void) sprintf(buf, "%u Path: (%u) %u %s",
	  tag_tag,
	  as,
	  tag_as,
	  origin);
    } else {
	sprintf(buf, "%u", tag);
    }
    return buf;
}


/*
 * aspath_tag_ospf - produce a suitable tag for OSPF based on the route
 *		   attributes.
 */
u_int32
aspath_tag_ospf __PF3(as, as_t,
		      rt, rt_entry *,
		      arbtag, metric_t)
{
    register gw_entry *gwp;
    register u_long tag = (arbtag << PATH_OSPF_TAG_USR_SHIFT) & PATH_OSPF_TAG_USR_MASK;

    /*
     * We need to treat BGP and routes from other OSPF's specially
     * here.  Otherwise we can do a stock thing.
     */
    gwp = rt->rt_gwp;
    switch (gwp->gw_proto) {
#ifdef PROTO_BGP
	register as_path *asp;
	register int send_via_bgp;

    case RTPROTO_BGP:
	/*
	 * Fetch the AS path.  If the path is zero or one AS's long,
	 * and there are no special attributes, we can encode it entirely
	 * within the tag.  Otherwise we need to tag it so the receiver
	 * knows to wait for internal BGP.
	 */
	send_via_bgp = 0;
	asp = rt->rt_aspath;
	if (asp->path_as != as) {
	    tag |= ((u_long)(asp->path_as)) & PATH_OSPF_TAG_AS_MASK;
	    if (asp->path_len > 0 || asp->path_attr_len > 0) {
		send_via_bgp = 1;
	    }
	} else if (asp->path_len > 0) {
	    register byte *cp = PATH_PTR(asp);
	    
	    tag |= (*cp++) << 8;
	    tag |= *cp;
	    if (asp->path_len > 2 || asp->path_attr_len > 0) {
		send_via_bgp = 1;
	    }
	} else {
	    if (asp->path_attr_len > 0) {
		send_via_bgp = 1;
	    }
	}

	if (send_via_bgp) {
	    /*
	     * Fixed upper attributes here.
	     */
	    tag |= (PATH_OSPF_TAG_TRUSTED|PATH_OSPF_TAG_COMPLETE|PATH_OSPF_TAG_LEN_2);
	} else if (asp->path_origin == PATH_ORG_XX) {
	    /*
	     * This one is incomplete
	     */
	    tag |= (PATH_OSPF_TAG_TRUSTED|PATH_OSPF_TAG_LEN_2);
	} else {
	    /*
	     * Set the length to zero or one based on whether we have
	     * an AS or not.  Mark it complete if the origin is IGP.
	     */
	    if (tag == 0) {
	        tag |= (PATH_OSPF_TAG_TRUSTED|PATH_OSPF_TAG_LEN_0);
	    } else {
		tag |= (PATH_OSPF_TAG_TRUSTED|PATH_OSPF_TAG_LEN_1);
	    }
	    
	    if (asp->path_origin == PATH_ORG_IGP) {
		tag |= PATH_OSPF_TAG_COMPLETE;
	    }
	}
	break;
#endif /* PROTO_BGP */

    case RTPROTO_OSPF_ASE:
	/*
	 * This is a slightly tricky case.  If the tag is user-set we
	 * send it on as incomplete.  If both OSPF's are operating
	 * in the same AS we send the tag through unchanged.  Otherwise,
	 * if it is a length 0 tag we send it on as a length 1 tag with
	 * the route's AS appended.  Otherwise, we send it on tagged for
	 * BGP.
	 */
	tag |= PATH_GET_OSPF_TAG(rt) & ~PATH_OSPF_TAG_USR_MASK;
	if (!(tag & PATH_OSPF_TAG_TRUSTED)) {
	    tag = PATH_OSPF_TAG_TRUSTED|PATH_OSPF_TAG_LEN_2;
	}

	if (gwp->gw_local_as != as) {
	    if ((tag & PATH_OSPF_TAG_LEN_MASK) == PATH_OSPF_TAG_LEN_0) {
		tag &= ~(PATH_OSPF_TAG_AS_MASK|PATH_OSPF_TAG_LEN_MASK);
		tag |= PATH_OSPF_TAG_LEN_1|
		    (gwp->gw_local_as & PATH_OSPF_TAG_AS_MASK);
	    } else if ((tag & (PATH_OSPF_TAG_LEN_MASK|PATH_OSPF_TAG_COMPLETE))
		       == PATH_OSPF_TAG_LEN_2 && (tag & PATH_OSPF_TAG_AS_MASK) == 0) {
		tag |= (gwp->gw_local_as & PATH_OSPF_TAG_AS_MASK);
	    } else {
		tag = PATH_OSPF_TAG_TRUSTED |
		    PATH_OSPF_TAG_LEN_2 |
			PATH_OSPF_TAG_COMPLETE
			    |(gwp->gw_local_as & PATH_OSPF_TAG_AS_MASK);
	    }
	}	
	break;

    default:
	/*
	 * The standard place we end up.  Here the route protocol is either
	 * operating as an EGP or an IGP, and may or may not be operating
	 * in the same AS as the OSPF which is importing the route.
	 */
	if (gwp->gw_local_as == 0 || (gwp->gw_peer_as == 0 && gwp->gw_local_as == as)) {
	    /*
	     * Assume its from our IGP
	     */
	    tag |= PATH_OSPF_TAG_TRUSTED |
		PATH_OSPF_TAG_COMPLETE |
		    PATH_OSPF_TAG_LEN_0;
	} else if (gwp->gw_peer_as == 0) {
	    /*
	     * From an IGP running in another AS
	     */
	    tag |= PATH_OSPF_TAG_TRUSTED |
		PATH_OSPF_TAG_COMPLETE |
		    PATH_OSPF_TAG_LEN_1 |
			(gwp->gw_local_as & PATH_OSPF_TAG_AS_MASK);
	} else if (gwp->gw_local_as == as) {
	    /*
	     * From an EGP running in this AS
	     */
	    tag |= PATH_OSPF_TAG_TRUSTED |
		PATH_OSPF_TAG_LEN_1 |
		    (gwp->gw_peer_as & PATH_OSPF_TAG_AS_MASK);
	} else {
	    /*
	     * All that is left is that we received the route from an
	     * EGP whose local AS is not that of the OSPF.  We'll need
	     * to send this via BGP to get it right.
	     */
	    tag |= PATH_OSPF_TAG_TRUSTED |
		PATH_OSPF_TAG_COMPLETE |
		    PATH_OSPF_TAG_LEN_2 |
			(gwp->gw_local_as & PATH_OSPF_TAG_AS_MASK);
	}
	break;
    }
    return htonl(tag);
}
#endif /* PROTO_OSPF */


#ifdef PROTO_BGP
/*
 * aspath_adv_ibgp - determine if this route should be advertised via
 *		   internal BGP or not.
 * XXX igp_proto should actually be a pointer to something which describes
 *     the state of the IGP.  e.g. IS-IS might or might not carry all
 *     path attributes internally.
 * XXX what about milnet?  Think about route server.
 */
int
aspath_adv_ibgp __PF3(as, as_t,
		      igp_proto, proto_t,
		      rt, rt_entry *)
{
    register u_long tag;
    register gw_entry *gwp;
    register as_path *asp;

    /*
     * So far we only know how to handle OSPF
     */
    gwp = rt->rt_gwp;

    switch (igp_proto) {
#ifdef PROTO_OSPF
    case RTPROTO_OSPF_ASE:
    case RTPROTO_OSPF:
	switch(gwp->gw_proto) {
	case RTPROTO_BGP:
	    /*
	     * If there are extra attributes, it goes via internal BGP.
	     * If the path wouldn't fit in an OSPF tag it goes via IBGP.
	     */
	    asp = rt->rt_aspath;
	    if (asp->path_attr_len > 0
		|| (asp->path_as != as && asp->path_len > 0)
		|| (asp->path_len > 2)) {
		return TRUE;
	    }
	    return FALSE;
	    
	case RTPROTO_OSPF_ASE:
	    /*
	     * Check out the tag.  If this route was received
	     * by an OSPF operating in our same AS, don't send
	     * via IBGP.  Else if the route was received directly
	     * by the other AS, don't send either.
	     */
	    tag = PATH_GET_OSPF_TAG(rt);
	    if (!PATH_OSPF_ISTRUSTED(tag)) {
		return FALSE;
	    }
	    if (gwp->gw_local_as == as) {
		/* can send via OSPF using same tag */
		return FALSE;
	    }
	    if ((tag & PATH_OSPF_TAG_LEN_MASK) == PATH_OSPF_TAG_LEN_0
		|| (((tag & (PATH_OSPF_TAG_COMPLETE|PATH_OSPF_TAG_LEN_MASK))
		     == (PATH_OSPF_TAG_LEN_2))
		    && ((tag & PATH_OSPF_TAG_AS_MASK) == 0))) {
		/* IGP/EGP with zero length AS */
		return FALSE;
	    }
	    break;

	default:
	    /*
	     * In the normal case the only thing we need to send
	     * via IBGP is stuff that came from an EGP operating in
	     * another AS
	     */
	    if (gwp->gw_peer_as == 0 || gwp->gw_local_as == as) {
	        return FALSE;
	    }
	    break;
	}
#endif /* PROTO_OSPF */

    default:
	/*
	 * Just barf, we can't deal with anything else yet.
	 */
	assert(FALSE);
    }

    return TRUE;
}
#endif	/* PROTO_BGP */


/**/
/* Tag support */
tag_t
tag_rt __PF1(rt, rt_entry *)
{
    tag_t tag;

    switch (rt->rt_gwp->gw_proto) {
#ifdef	PROTO_OSPF
    case RTPROTO_OSPF_ASE:
	tag = PATH_GET_OSPF_TAG(rt);
	if (PATH_OSPF_ISTRUSTED(tag)) {
	    tag = (tag & PATH_OSPF_TAG_USR_MASK) >> PATH_OSPF_TAG_USR_SHIFT;
	}
	break;
#endif	/* PROTO_OSPF */

    default:
	tag = 0;
    }

    return tag;
}


/**/
/* AS path matching code */

block_t aspath_match_index = 0;

char *
aspath_adv_origins __PF1(mask, flag_t)
{
    static char line[LINE_MAX];
    
    if (mask == -1) {
	(void) strcpy(line, "all");
    } else {
	const bits *p;
	
	*line = (char) 0;
	for (p = path_Orgs; p->t_name; p++) {
	    if ((1 << p->t_bits) & mask) {
		if (*line) {
		    (void) strcat(line, " | ");
		}
		(void) strcat(line, gd_lower(p->t_name));
	    }
	}
    }

    return line;
}

void
aspath_adv_free __PF1(psp, void_t)
{
    task_block_free(aspath_match_index, psp);
}


char *
aspath_adv_print __PF1(psp, void_t)
{
    asmatch_t *asmp = (asmatch_t *) psp;
    static char line[BUFSIZ];
    int i, j;
    asp_table *astp = asmp->nfa;

    sprintf(line, "[ start: ");

    for (i = 0; i < 1 + ASP_ACC / ASP_WORD; i++) {
	sprintf(&line[strlen(line)], "%#x ",
		asmp->start[i]);
    }

    for (i = 0; i < ASP_ACC; i++, astp++) {
	NFA_NONZERO(astp->next, j);
	if (j || astp->as) {
	    sprintf(&line[strlen(line)], "as: %d states: ",
		    astp->as);
	    for (j = 0; j < 1 + ASP_ACC / ASP_WORD; j++) {
		sprintf(&line[strlen(line)], "%#x ",
			astp->next[j]);
	    }
	}
    }
    sprintf(&line[strlen(line)], "] origins %s",
	    aspath_adv_origins(asmp->origin_mask));

    return line;
}


int
aspath_adv_compare __PF2(psp1, void_t,
			 psp2, void_t)
{
    asmatch_t *asmp1 = (asmatch_t *) psp1;
    asmatch_t *asmp2 = (asmatch_t *) psp2;

    /* XXX - need to be a bit more intellegent about this */
    return asmp1 == asmp2;
}


int
aspath_adv_match __PF2(psp, void_t,
		       asp, register as_path *)
{
    register u_short *lp;
    register u_short *p = PATH_SHORT_PTR(asp);
    asmatch_t *asmp = (asmatch_t *) psp;
    flag_t state[1 + ASP_ACC / ASP_WORD], new[1 + ASP_ACC / ASP_WORD];

    if (!asp) {
	return FALSE;
    }

    if (!BIT_TEST(asmp->origin_mask, 1 << asp->path_origin)) {
	return FALSE;
    }

    NFA_ASG(state, asmp->start);

    for (lp = p + (asp->path_len/sizeof (u_short)) - 1;
	 p <= lp;
	 p++)  {
	register int i;
	int nonzero;
	
	NFA_ZERO(new);
	NFA_CLR(state, ASP_ACC);
	for (i = 0; i < ASP_ACC; i++) {
	    if (NFA_ISSET(state, i)) {
		if (!asmp->nfa[i].as || asmp->nfa[i].as == *p) {
		    NFA_OR(new, new, asmp->nfa[i].next);
		    if (p == lp
			&& NFA_ISSET(new, ASP_ACC)) {
			return TRUE;
		    }
		}
		NFA_CLR(state, i);
		NFA_NONZERO(state, nonzero);
		if (!nonzero) {
		    break;
		}
	    }
	}
	NFA_ASG(state, new);
	NFA_NONZERO(state, nonzero);
	if (!nonzero) {
	    break;
	}
    }

    return FALSE;
}


/*
 * ------------------------------------------------------------------------
 * 
 * 	GateD, Release 3
 * 
 * 	Copyright (c) 1990,1991,1992,1993 by Cornell University
 * 	    All rights reserved.
 * 
 * 	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY
 * 	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * 	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * 	AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	Royalty-free licenses to redistribute GateD Release
 * 	3 in whole or in part may be obtained by writing to:
 * 
 * 	    GateDaemon Project
 * 	    Information Technologies/Network Resources
 * 	    200 CCC, Garden Avenue
 * 	    Cornell University
 * 	    Ithaca, NY  14853-2601  USA
 * 
 * 	GateD is based on Kirton's EGP, UC Berkeley's routing
 * 	daemon	 (routed), and DCN's HELLO routing Protocol.
 * 	Development of GateD has been supported in part by the
 * 	National Science Foundation.
 * 
 * 	Please forward bug fixes, enhancements and questions to the
 * 	gated mailing list: gated-people@gated.cornell.edu.
 * 
 * 	Authors:
 * 
 * 		Jeffrey C Honig <jch@gated.cornell.edu>
 * 		Scott W Brim <swb@gated.cornell.edu>
 * 
 * ------------------------------------------------------------------------
 * 
 *       Portions of this software may fall under the following
 *       copyrights:
 * 
 * 	Copyright (c) 1988 Regents of the University of California.
 * 	All rights reserved.
 * 
 * 	Redistribution and use in source and binary forms are
 * 	permitted provided that the above copyright notice and
 * 	this paragraph are duplicated in all such forms and that
 * 	any documentation, advertising materials, and other
 * 	materials related to such distribution and use
 * 	acknowledge that the software was developed by the
 * 	University of California, Berkeley.  The name of the
 * 	University may not be used to endorse or promote
 * 	products derived from this software without specific
 * 	prior written permission.  THIS SOFTWARE IS PROVIDED
 * 	``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * 	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * 	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
