static char sccsid[] = "@(#)17	1.1  src/tcpip/usr/sbin/gated/rt_radix.c, tcprouting, tcpip411, GOLD410 12/6/93 17:27:16";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: Bcmp
 *		PROTO_MATCH
 *		RT_TABLE
 *		RT_TABLES
 *		RT_TREETOP
 *		RT_WALK
 *		RT_WHOLE
 *		__PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF4
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
 *  rt_radix.c,v 1.25.2.2 1993/06/23 20:14:37 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_RT_VAR
#include "include.h"


/*
 * Radix search tree node layout.
 */

typedef struct _radix_node {
    struct	_radix_mask *rnode_mklist;	/* list of masks contained in subtree */
    struct	_radix_node *rnode_p;		/* parent */
    short	rnode_b;			/* bit offset; -1-index(netmask) */
    char	rnode_bmask;			/* node: mask for bit test*/
    u_char	rnode_flags;			/* enumerated next */
#define RNODEF_ROOT	0x01			/* leaf is root leaf for tree */
    union {
	struct {				/* leaf only data: */
	    sockaddr_un *rnode_Key;		/* object of search */
	    sockaddr_un *rnode_Mask;		/* netmask, if present */
	    struct _radix_node *rnode_Dupedkey;
	    rt_head *rnode_Rth;
	} rnode_leaf;
	struct {				/* node only data: */
	    int	rnode_Off;			/* byte offset into sockaddr to start compare */
	    struct _radix_node *rnode_L;	/* progeny */
	    struct _radix_node *rnode_R;	/* progeny */
	} rnode_node;
    } rnode_u;
} radix_node;

#define MAXKEYLEN 32

#define rnode_dupedkey	rnode_u.rnode_leaf.rnode_Dupedkey
#define rnode_key	rnode_u.rnode_leaf.rnode_Key
#define rnode_mask 	rnode_u.rnode_leaf.rnode_Mask
#define	rnode_rth	rnode_u.rnode_leaf.rnode_Rth
#define rnode_off	rnode_u.rnode_node.rnode_Off
#define rnode_l		rnode_u.rnode_node.rnode_L
#define rnode_r		rnode_u.rnode_node.rnode_R

/*
 * Annotations to tree concerning potential routes applying to subtrees.
 */

typedef struct _radix_mask {
	short	rm_b;			/* bit offset; -1-index(netmask) */
	char	rm_unused;		/* cf. rnode_bmask */
	u_char	rm_flags;		/* cf. rnode_flags */
	struct	_radix_mask *rm_mklist;	/* more masks to try */
	sockaddr_un *rm_mask;		/* the mask */
	int	rm_refs;		/* # of references to this struct */
} radix_mask;


#define	RT_TREETOP(af)	rnode_treetops[af]

#define	RT_TABLES(af) { register int af = -1; \
			     while (++af < AF_MAX) { \
				 if (RT_TREETOP(af))

#define	RT_TABLES_END(af)    } }

/*
 * Routines to build and maintain radix trees for routing lookups.
 */

static block_t rt_mask_block_index;
static block_t rt_node_block_index;

static radix_node *rnode_treetops[AF_MAX];
#define	AF_MASK	AF_UNSPEC

#undef Bcmp
#define Bcmp(a, b, l) (l ? bcmp((caddr_t) a, (caddr_t) b, (size_t) l) : 0)
/*
 * The data structure for the keys is a radix tree with one way
 * branching removed.  The index rnode_b at an internal node n represents a bit
 * position to be tested.  The tree is arranged so that all descendants
 * of a node n have keys whose bits all agree up to position rnode_b - 1.
 * (We say the index of n is rnode_b.)
 *
 * There is at least one descendant which has a one bit at position rnode_b,
 * and at least one with a zero there.
 *
 * A route is determined by a pair of key and mask.  We require that the
 * bit-wise logical & of the key and mask to be the key.
 * We define the index of a route associated with the mask to be
 * the first bit number in the mask where 0 occurs (with bit number 0
 * representing the highest order bit).
 * 
 * We say a mask is normal if every bit is 0, past the index of the mask.
 * If a node n has a descendant (k, m) with index(m) == index(n) == rnode_b,
 * and m is a normal mask, then the route applies to every descendant of n.
 * If the index(m) < rnode_b, this implies the trailing last few bits of k
 * before bit b are all 0, (and hence consequently true of every descendant
 * of n), so the route applies to all descendants of the node as well.
 *
 * The present version of the code makes no use of normal routes,
 * but similar logic shows that a non-normal mask m such that
 * index(m) <= index(n) could potentially apply to many children of n.
 * Thus, for each non-host route, we attach its mask to a list at an internal
 * node as high in the tree as we can go. 
 */

inline static radix_node *
rnode_search __PF2(s_v, sockaddr_un *,
		   s_head, radix_node *)
{
    register radix_node *s_x = s_head;
    register byte *cp = (byte *) s_v;

    while (s_x->rnode_b >= 0) {
	if (s_x->rnode_bmask & cp[s_x->rnode_off]) {
	    s_x = s_x->rnode_r;
	} else {
	    s_x = s_x->rnode_l;
	}
    }

    return s_x;
}


#ifdef	notdef
/* Search with mask */
static inline radix_node *
rnode_search_m __PF3(v, byte *
		     head, radix_node *,
		     m, byte *)
{
    register radix_node *x;

    for (x = head; x->rnode_b >= 0;) {
	if ((x->rnode_bmask & m[x->rnode_off]) &&
	    (x->rnode_bmask & v[x->rnode_off])) {
	    x = x->rnode_r;
	} else {
	    x = x->rnode_l;
	}
    }
    return x;
}


/* find best match for key v in trie rooted at head */
static radix_node *
rnode_match __PF3(v, sockaddr_un *
		  head, radix_node *,
		  proto_mask, flag_t)
{
    register radix_node *t = head, *x;
    register byte *cp = v, *cp2, *cp3;
    byte *cplim;
    radix_node *saved_t;
    int off = t->rnode_off, matched_off;

    /*
     * Open code rnode_search(v, head) to avoid overhead of extra
     * subroutine call.
     */
    /*
     * Search based on mask and address compare 
     * - bit compare (rnode_b) moves from bit 31 down to 0 or until 
     *   mask & addr no longer match
     */
    for (; t->rnode_b >= 0; ) {
	if (t->rnode_bmask & cp[t->rnode_off]) {
	    t = t->rnode_r;
	} else {
	    t = t->rnode_l;
	}
    }
    /*
     * See if we match exactly as a host destination
     */
    cp += off;
    cp2 = t->rnode_key + off;
    cplim = v + socksize(v);
    for (; cp < cplim; cp++, cp2++) {
	if (*cp != *cp2) {
	    goto on1;
	}
    }
    
    /*
     * This extra grot is in case we are explicitly asked
     * to look up the default.  Ugh!
     */
    if (BIT_TEST(t->rnode_flags, RNODEF_ROOT) && t->rnode_dupedkey) {
	t = t->rnode_dupedkey;
    }
    return t;

on1:
    /* Beginning of CP */
    matched_off = cp - v;
    /* Matched exactly up to parent of t */
    saved_t = t;
    do {
	/* duped keys are ones with same addr but different masks */
	if (t->rnode_mask) {
	    /*
	     * Even if we don't match exactly as a hosts;
	     * we may match if the leaf we wound up at is
	     * a route to a net.
	     */
	    cp3 = matched_off + t->rnode_mask;
	    cp2 = matched_off + t->rnode_key;
	    for (; cp < cplim; cp++) {
		if ((*cp2++ ^ *cp) & *cp3++) {
		    break;
		}
	    }		
	    /* Found best match */
	    if (cp == cplim) {
		return t;
	    }
	    /* back to the beginning of cp */
	    cp = matched_off + v;
	}
    } while (t = t->rnode_dupedkey);

    t = saved_t;
    /* start searching up the tree */
    do {
	register radix_mask *m;
	t = t->rnode_p;
	/* List of masks of children of t's parent
	 * remember that had exact match up to t's parent
	 */
	if (m = t->rnode_mklist) {
	    sockaddr_un *masked_key = sockdup(v);

	    /*
	     * After doing measurements here, it may
	     * turn out to be faster to open code
	     * rnode_search_m here instead of always
	     * copying and masking.
	     */
	    off = MIN(t->rnode_off, matched_off);
	    do {
		sockcopy(v, masked_key);
		sockmask(masked_key, m->rm_mask);
		
		x = rnode_search(masked_key, t);
		while (x && x->rnode_mask != m->rm_mask) {
		    x = x->rnode_dupedkey;
		}
		/* Found Match */
		if (x &&
		    !Bcmp(masked_key->a.sa_data, x->rnode_key + off, socksize(v) - off)) {
		    return x;
		}
	    } while (m = m->rm_mklist);
	}
    } while (t != head);

    return 0;
}
#endif	/* notdef */
		

static radix_node *
rnode_newpair __PF3(v, sockaddr_un *,
		    b, int,
		    node, register radix_node *)
{
    register radix_node *t = (radix_node *) task_block_alloc(rt_node_block_index);

    t->rnode_b = b;
    t->rnode_bmask = 0x80 >> (b & 7);
    t->rnode_l = node;
    t->rnode_off = b >> 3;
    node->rnode_b = -1;
    node->rnode_key = v;
    node->rnode_p = t;
    node->rnode_flags = t->rnode_flags = 0;

    return t;
}


static radix_node *
rnode_insert __PF4(v, sockaddr_un *,
		   head, radix_node *,
		   dupentry, int *,
		   node, radix_node *)
{
    register radix_node *t = rnode_search(v, head);
    register byte * cp = (byte *) v + head->rnode_off;
    register int b;

    /*
     * find first bit at which v and t->rnode_key differ
     */
    {
	register byte *cp2 = (byte *) t->rnode_key + head->rnode_off;
	register int cmp_res;
	byte *cplim = (byte *) v + MAX(socksize(v), socksize(t->rnode_key));

	while (cp < cplim) {
	    if (*cp2++ != *cp++) {
		goto on1;
	    }
	}
	if (socksize(v) == socksize(t->rnode_key)) {
	    /* Already exists */

	    *dupentry = 1;
	    return t;
	}

on1:
	*dupentry = 0;
	cmp_res = (cp[-1] ^ cp2[-1]) & 0xff;
	for (b = (cp - (byte *) v) << 3; cmp_res; b--) {
	    cmp_res >>= 1;
	}
    }

    /*
     * Then insert v into the tree
     */
    {
	register radix_node *p, *x = head;

	/* Locate insertion point */
	cp = (byte *) v;
	do {
	    p = x;
	    if (cp[x->rnode_off] & x->rnode_bmask) {
		x = x->rnode_r;
	    } else {
		x = x->rnode_l;
	    }
	} while (b > (unsigned) x->rnode_b); /* x->rnode_b < b && x->rnode_b >= 0 */

	/* Allocate internal and external nodes */
	t = rnode_newpair(v, b, node);

	/* Insert to the left or right of parent */
	if (!(cp[p->rnode_off] & p->rnode_bmask)) {
	    p->rnode_l = t;
	} else {
	    p->rnode_r = t;
	}

	/* Set backlinks to parents */
	x->rnode_p = t; t->rnode_p = p; /* frees x, p as temp vars below */

	/* Hang existing tree and new leaf off of new internal node */
	if (!(cp[t->rnode_off] & t->rnode_bmask)) {
	    t->rnode_r = x;
	    t->rnode_l = node;
	} else {
	    t->rnode_r = node;
	    t->rnode_l = x;
	}
    }

    return node;
}


#ifdef	notdef
static radix_node *
rnode_addmask __PF3(netmask, byte *
		    search, int,
		    skip, int)
{
    register radix_node *x;
    register byte *cp, *cplim;
    register int b, mlen, j;
    int maskduplicated;

    mlen = *(u_char *)netmask;
    if (search) {
	x = rnode_search(netmask, RT_TREETOP(AF_MASK));
	mlen = *(u_char *)netmask;
	if (!Bcmp(netmask, x->rnode_key, mlen)) {
	    return x;
	}
    }
    x = (radix_node *) task_mem_calloc((task *) 0, 1, MAXKEYLEN + 2 * sizeof (*x));
    cp = (byte *)(x + 2);
    bcopy((caddr_t) netmask, (caddr_t) cp, mlen);
    netmask = cp;
    x = rnode_insert(netmask, RT_TREETOP(AF_MASK), &maskduplicated, x);
    /*
     * Calculate index of mask.
     */
    cplim = netmask + mlen;
    for (cp = netmask + skip; cp < cplim; cp++) {
	if (*(u_char *)cp != 0xff) {
	    break;
	}
    }
    b = (cp - netmask) << 3;
    if (cp != cplim) {
	if (*cp != 0) {
	    gotOddMasks = 1;
	    for (j = 0x80; j; b++, j >>= 1) {
		if (!(j & *cp)) {
		    break;
		}
	    }
	}
    }
    x->rnode_b = -1 - b;
    return x;
}
#endif	/* notdef */


/*
 * Add node to radix tree
 */
static inline radix_node *
rnode_addroute __PF4(v, sockaddr_un *,
		     netmask, sockaddr_un *,
		     head, radix_node *,
		     treenode, radix_node *)
{
    register radix_node *t, *x, *tt;
    short b = 0, b_leaf;
    int mlen = netmask ? socksize(netmask) : 0;
    int keyduplicated;
    byte * cplim;
    radix_mask *m, **mp;
    radix_node *saved_tt;

    /*
     * Deal with duplicated keys: attach node to previous instance
     */
    saved_tt = tt = rnode_insert(v, head, &keyduplicated, treenode);
    if (keyduplicated) {
	do {
	    if (tt->rnode_mask == netmask) {
		return tt;
	    }
	    t = tt;
	    if (!netmask
		|| (tt->rnode_mask
		    && mask_refines(netmask, tt->rnode_mask))) {
		break;
	    }
	} while (tt = tt->rnode_dupedkey);

	/* First init the new node */
	x = treenode;
	x->rnode_key = v;
	x->rnode_b = -1;
	x->rnode_p = saved_tt->rnode_p;

	/* Then insert it on the list in order */
	if (tt == saved_tt) {
	    /* Replace first on list */

	    if (x->rnode_p->rnode_r == saved_tt) {
		x->rnode_p->rnode_r = x;
	    } else {
		x->rnode_p->rnode_l = x;
	    }
	    x->rnode_dupedkey = saved_tt;
	    x->rnode_flags = saved_tt->rnode_flags;
	    BIT_RESET(saved_tt->rnode_flags, RNODEF_ROOT);
	} else if (tt) {
	    /* Insert before this node */

	    for (tt = saved_tt;
		 tt->rnode_dupedkey != t;
		 tt = tt->rnode_dupedkey) ;

	    tt->rnode_dupedkey = x;
	    x->rnode_dupedkey = t;
	} else {
	    /* Insert after this node */

	    x->rnode_dupedkey = t->rnode_dupedkey;
	    t->rnode_dupedkey = x;
	}
	tt = x;
    }

    /*
     * Put mask in tree.
     */
    if (netmask) {
	tt->rnode_mask = netmask;
#ifdef	notdef
	tt->rnode_b = x->rnode_b;
#endif	/* notdef */
    }
    t = saved_tt->rnode_p;
    b_leaf = -1 - t->rnode_b;
    if (t->rnode_r == saved_tt) {
	x = t->rnode_l;
    } else {
	x = t->rnode_r;
    }
    /* Promote general routes from below */
    if (x->rnode_b < 0) { 
	if (x->rnode_mask && (x->rnode_b >= b_leaf) && !x->rnode_mklist) {
	    m = (radix_mask *) task_block_alloc(rt_mask_block_index);
	    if (m) {
		m->rm_b = x->rnode_b;
		m->rm_mask = x->rnode_mask;
		x->rnode_mklist = t->rnode_mklist = m;
	    }
	}
    } else if (x->rnode_mklist) {
	/*
	 * Skip over masks whose index is > that of new node
	 */
	for (mp = &x->rnode_mklist; m = *mp; mp = &m->rm_mklist)
	    if (m->rm_b >= b_leaf) {
		break;
	    }
	t->rnode_mklist = m;
	*mp = 0;
    }
    /* Add new route to highest possible ancestor's list */
    if (!netmask || (b > t->rnode_b )) {
	return tt; /* can't lift at all */
    }
    b_leaf = tt->rnode_b;
    do {
	x = t;
	t = t->rnode_p;
    } while (b <= t->rnode_b && x != head);
    /*
     * Search through routes associated with node to
     * insert new route according to index.
     * For nodes of equal index, place more specific
     * masks first.
     */
    cplim = (byte *) netmask + mlen;
    for (mp = &x->rnode_mklist; m = *mp; mp = &m->rm_mklist) {
	if (m->rm_b < b_leaf) {
	    continue;
	}
	if (m->rm_b > b_leaf) {
	    break;
	}
	if (m->rm_mask == netmask) {
	    m->rm_refs++;
	    tt->rnode_mklist = m;
	    return tt;
	}
	if (mask_refines(netmask, m->rm_mask)) {
	    break;
	}
    }

    m = (radix_mask *) task_block_alloc(rt_mask_block_index);
    m->rm_b = b_leaf;
    m->rm_mask = netmask;
    m->rm_mklist = *mp;
    *mp = m;
    tt->rnode_mklist = m;

    return tt;
}


static radix_node *
rnode_delete __PF2(tt, register radix_node *,
		   head, radix_node *)
{
    register radix_node *t, *p, *x = head;
    int b, head_off = x->rnode_off, vlen =  * (u_char *) tt->rnode_key;
    radix_mask *m, *saved_m, **mp;
    radix_node *dupedkey;
    radix_node *saved_tt = rnode_search(tt->rnode_key, x);

    if (!saved_tt ||
	Bcmp(tt->rnode_key + head_off, saved_tt->rnode_key + head_off, vlen - head_off)) {
	return 0;
    }
    dupedkey = saved_tt->rnode_dupedkey;
    if (tt->rnode_mask || !(saved_m = m = tt->rnode_mklist)) {
	/* No mask list */
	goto on1;
    }
    if (m->rm_mask != tt->rnode_mask) {
	trace(TR_ALL, LOG_ERR, "rnode_delete: inconsistent annotation");
	goto on1;
    }
    if (--m->rm_refs >= 0) {
	/* Mask list still referenced */
	goto on1;
    }
    b = -1 - tt->rnode_b;
    t = saved_tt->rnode_p;
    if (b > t->rnode_b) {
	/* Wasn't lifted at all */
	goto on1;
    }
    do {
	x = t;
	t = t->rnode_p;
    } while (b <= t->rnode_b && x != head);
    for (mp = &x->rnode_mklist; m = *mp; mp = &m->rm_mklist) {
	if (m == saved_m) {
	    *mp = m->rm_mklist;
	    task_block_free(rt_mask_block_index, (void_t) m);
	    break;
	}
    }
    if (!m) {
	trace(TR_ALL, LOG_ERR, "rnode_delete: couldn't find our annotation\n");
    }

on1:
    /*
     * Eliminate us from tree
     */
    if (BIT_TEST(tt->rnode_flags, RNODEF_ROOT)) {
	return 0;
    }
    t = tt->rnode_p;
    if (dupedkey) {
	if (tt == saved_tt) {
	    x = dupedkey;
	    x->rnode_p = t;
	    if (t->rnode_l == tt) {
		t->rnode_l = x;
	    } else {
		t->rnode_r = x;
	    }
	} else {
	    for (p = saved_tt; p && p->rnode_dupedkey != tt;) {
		p = p->rnode_dupedkey;
	    }
	    assert(p);	/* Could not find our node */

	    p->rnode_dupedkey = tt->rnode_dupedkey;
	}
	goto out;
    }
    if (t->rnode_l == tt) {
	x = t->rnode_r;
    } else {
	x = t->rnode_l;
    }
    p = t->rnode_p;
    if (p->rnode_r == t) {
	p->rnode_r = x;
    } else {
	p->rnode_l = x;
    }
    x->rnode_p = p;

    /*
     * Demote routes attached to us.
     */
    if (t->rnode_mklist) {
	if (x->rnode_b >= 0) {
	    for (mp = &x->rnode_mklist; m = *mp; mp = &m->rm_mklist) ;
	    *mp = t->rnode_mklist;
	} else {
	    for (m = t->rnode_mklist; m;) {
		radix_mask *mm = m->rm_mklist;

		if (m == x->rnode_mklist && --m->rm_refs < 0) {
		    x->rnode_mklist = 0;
		    task_block_free(rt_mask_block_index, (void_t) m);
		} else  {
		    trace(TR_ALL, LOG_ERR, "rnode_delete: Orphaned Mask %x at %x\n",
			  m,
			  x);
		}
		m = mm;
	    }
	}
    }

    /* Free parent */
    task_block_free(rt_node_block_index, (void_t) t);

out:
    return tt;
}

static void
rnode_inithead __PF4(head, radix_node **,
		     offset, int,
		     min, sockaddr_un *,
		     max, sockaddr_un *)
{
    register radix_node *t, *tt, *ttt;

    if (!*head) {
	*head = t = rnode_newpair(min,
				  offset,
				  (radix_node *) task_block_alloc(rt_node_block_index));
	ttt = (radix_node *) task_block_alloc(rt_node_block_index);
	t->rnode_r = ttt;
	t->rnode_p = t;
	tt = t->rnode_l;
	tt->rnode_flags = t->rnode_flags = RNODEF_ROOT;
	tt->rnode_b = -1 - offset;
	*ttt = *tt;
	ttt->rnode_key = max;
    }

    return;
}


/**/

#define RT_WALK(head, rth)	{ \
				     register radix_node *prn = head; \
				     while (prn) { \
					 register radix_node *rn; \
					 while (prn->rnode_b >= 0) { \
					     prn = prn->rnode_l; \
					 } \
					 for (rn = BIT_TEST(prn->rnode_flags, RNODEF_ROOT) ? prn->rnode_dupedkey : prn; \
					      rn; \
					      rn = rn->rnode_dupedkey) { \
						 rth = rn->rnode_rth;

#define RT_WALK_END(head, rth)		 } \
					 while (prn->rnode_p->rnode_r == prn) { \
					     prn = prn->rnode_p; \
					     if (BIT_TEST(prn->rnode_flags, RNODEF_ROOT)) { \
						 prn = (radix_node *) 0; \
						 break; \
					     } \
					 } \
					 if (prn) { \
					     prn = prn->rnode_p->rnode_r; \
					 } \
				     } \
				 }

/*
 *	Macro to scan through entire active routing table
 */
#define RT_WHOLE(head, rt)	{ rt_head *rth; RT_WALK(head, rth) RT_ALLRT(rt, rth)

#define	RT_WHOLE_END(head, rt)	RT_ALLRT_END(rt, rth) RT_WALK_END(head, rth) }


#define RT_TABLE(head, rt)	{ rt_head *rth; RT_WALK(head, rth) RT_IFRT(rt, rth)

#define	RT_TABLE_END(head, rt)	RT_IFRT_END RT_WALK_END(head, rth) }


/*
 *	Locate the rt_head for the given destination
 */
rt_head *
rt_table_locate __PF2(dst, sockaddr_un *,
		      mask, sockaddr_un *)
{
    register radix_node *rn;
    rt_head *rth = (rt_head *) 0;

    if (rn = RT_TREETOP(socktype(dst))) {
	register byte *cp = (byte *) dst, *cp2;
	byte *cplim = cp + socksize(dst);
	int off = rn->rnode_off;

	/* Search down the tree */
	while (rn->rnode_b >= 0) {
	    if (rn->rnode_bmask & cp[rn->rnode_off]) {
		rn = rn->rnode_r;
	    } else {
		rn = rn->rnode_l;
	    }
	}

	/* See if we have an exact match */
	if (socksize(dst) != socksize(rn->rnode_key)) {
	    goto Return;
	}
	cp += off;
	cp2 = (byte *) rn->rnode_key + off;
	while (cp < cplim) {
	    if (*cp++ != *cp2++) {
		goto Return;
	    }
	}

	/* Check for default */
	if (BIT_TEST(rn->rnode_flags, RNODEF_ROOT)) {
	    rn = rn->rnode_dupedkey;
	}

	if (mask) {
	    /* Find the right mask */
	    for (; rn; rn = rn->rnode_dupedkey) {
		if (rn->rnode_mask == mask) {
		    rth = rn->rnode_rth;
		    break;
		}
	    }
	} else if (rn) {
	    /* Return the first one */
	    rth = rn->rnode_rth;
	}
    }

 Return:
    return rth;
}


#define	PROTO_MATCH(rn, good, bad, proto_mask) { \
	register rt_entry *rt; \
	RT_ALLRT(rt, (rn)->rnode_rth) { \
	    if (!BIT_TEST(rt->rt_state, bad) && \
		BIT_TEST(rt->rt_state, good) && \
		BIT_TEST(proto_mask, RTPROTO_BIT(rt->rt_gwp->gw_proto))) { \
		return rt; \
	    } \
	} RT_ALLRT_END(rt, (rn)->rnode_rth) ; \
    }

/*
 *	Lookup a route the way the kernel would
 */
rt_entry *
rt_lookup __PF4(good, flag_t,
		bad, flag_t,
		dst, sockaddr_un *,
		proto_mask, flag_t)
{
    radix_node *head = RT_TREETOP(socktype(dst));
    register radix_node *t = head, *x;
    register byte *cp = (byte *) dst, *cp2, *cp3;
    byte *cplim;
    radix_node *saved_t;
    int off = t->rnode_off, matched_off;

    /*
     * Open code rnode_search(v, head) to avoid overhead of extra
     * subroutine call.
     */
    /*
     * Search based on mask and address compare 
     * - bit compare (rnode_b) moves from bit 31 down to 0 or until 
     *   mask & addr no longer match
     */
    for (; t->rnode_b >= 0; ) {
	if (t->rnode_bmask & cp[t->rnode_off]) {
	    t = t->rnode_r;
	} else {
	    t = t->rnode_l;
	}
    }
    /*
     * See if we match exactly as a host destination
     */
    cp += off;
    cp2 = (byte *) t->rnode_key + off;
    cplim = (byte *) dst + socksize(dst);
    if (socksize(dst) != socksize(t->rnode_key)) {
	goto on1;
    }
    for (; cp < cplim; cp++, cp2++) {
	if (*cp != *cp2) {
	    goto on1;
	}
    }
    
    /*
     * This extra grot is in case we are explicitly asked
     * to look up the default.  Ugh!
     */
    if (BIT_TEST(t->rnode_flags, RNODEF_ROOT) && t->rnode_dupedkey) {
	t = t->rnode_dupedkey;
    }

    PROTO_MATCH(t, good, bad, proto_mask);

on1:
    /* Beginning of CP */
    matched_off = cp - (byte *) dst;
    /* Matched exactly up to parent of t */
    saved_t = t;
    do {
	/* duped keys are ones with same addr but different masks */
	if (t->rnode_mask) {
	    /*
	     * Even if we don't match exactly as a hosts;
	     * we may match if the leaf we wound up at is
	     * a route to a net.
	     */
	    cp3 = matched_off + (byte *) t->rnode_mask;
	    cp2 = matched_off + (byte *) t->rnode_key;
	    for (; cp < cplim; cp++) {
		if ((*cp2++ ^ *cp) & *cp3++) {
		    break;
		}
	    }		
	    /* Found best match */
	    if (cp == cplim) {
		PROTO_MATCH(t, good, bad, proto_mask);
	    }
	    /* back to the beginning of cp */
	    cp = matched_off + (byte *) dst;
	}
    } while (t = t->rnode_dupedkey);

    t = saved_t;
    /* start searching up the tree */
    do {
	register radix_mask *m;
	t = t->rnode_p;
	/* List of masks of children of t's parent
	 * remember that had exact match up to t's parent
	 */
	if (m = t->rnode_mklist) {
	    sockaddr_un *masked_key = sockdup(dst);

	    /*
	     * After doing measurements here, it may
	     * turn out to be faster to open code
	     * rnode_search_m here instead of always
	     * copying and masking.
	     */
	    off = MIN(t->rnode_off, matched_off);
	    do {
		sockcopy(dst, masked_key);
		sockmask(masked_key, m->rm_mask);
		
		x = rnode_search(masked_key, t);
		while (x && x->rnode_mask != m->rm_mask) {
		    x = x->rnode_dupedkey;
		}
		/* Found Match */
		if (x &&
		    !Bcmp(masked_key + off, x->rnode_key + off, socksize(dst) - off)) {
		    PROTO_MATCH(x, good, bad, proto_mask);
		}
	    } while (m = m->rm_mklist);
	}
    } while (t != head);

    return (rt_entry *) 0;
}
#undef	PROTO_MATCH


/*
 *	Insert this head into the tree
 */
void
rt_table_add __PF1(rth, rt_head *)
{
    radix_node *rn;
    
    /* Allocate radix nodes */
    rth->rth_radix_node = (radix_node *) task_block_alloc(rt_node_block_index);

    /* Install this node */
    rn = rnode_addroute(rth->rth_dest,
			rth->rth_dest_mask,
			RT_TREETOP(socktype(rth->rth_dest)),
			rth->rth_radix_node);

    /* Link the head to this node */
    rn->rnode_rth = rth;
}


/*
 *	Remove this head from the tree
 */
void
rt_table_delete __PF1(rth, rt_head *)
{
    rnode_delete(rth->rth_radix_node, RT_TREETOP(socktype(rth->rth_dest)));

    task_block_free(rt_node_block_index, (void_t) rth->rth_radix_node);
}


/**/

block_t rtlist_block_index = 0;
rt_list *rt_change_list = 0;			/* Number of the active list */

	    
/* Build a change list of all active routes */
/* 	Routes not to be advised and hidden routes are not located */
rt_list *
rthlist_active __PF1(af, int)
{
    rt_entry *rt;
    rt_list *rtl = (rt_list *) 0;

    switch (af) {
    case AF_UNSPEC:
	RT_TABLES(af1) {
	    radix_node *head = RT_TREETOP(af1);

	    if (head) {
		RT_TABLE(head, rt) {
		    if (BIT_TEST(rt->rt_state, RTS_NOADVISE)) {
			/* Not to be announced */
			continue;
		    }
		    if (BIT_TEST(rt->rt_state, RTS_HIDDEN|RTS_DELETE) && !BIT_TEST(rt->rt_state, RTS_HOLDDOWN)) {
			/* Deleted or hidden, unless in holddown */
			continue;
		    }
		    RTLIST_ADD(rtl, rt->rt_head);
		} RT_TABLE_END(head, rt) ;
	    }
	} RT_TABLES_END(af1) ;
	break;

    default:
	RT_TABLE(RT_TREETOP(af), rt) {
	    if (BIT_TEST(rt->rt_state, RTS_NOADVISE)) {
		/* Not to be announced */
		continue;
	    }
	    if (BIT_TEST(rt->rt_state, RTS_HIDDEN|RTS_DELETE) && !BIT_TEST(rt->rt_state, RTS_HOLDDOWN)) {
		/* Deleted or hidden, unless in holddown */
		continue;
	    }
	    RTLIST_ADD(rtl, rt->rt_head);
	} RT_TABLE_END(RT_TREETOP(af), rt) ;
    }

    return rtl ? rtl->rtl_root : rtl;
}


/* Build a change list of all active routes */
/* 	Routes not to be advised and hidden routes are not located */
rt_list *
rtlist_active __PF1(af, int)
{
    rt_entry *rt;
    rt_list *rtl = (rt_list *) 0;

    switch (af) {
    case AF_UNSPEC:
	RT_TABLES(af1) {
	    radix_node *head = RT_TREETOP(af1);

	    if (head) {
		RT_TABLE(head, rt) {
		    if (BIT_TEST(rt->rt_state, RTS_NOADVISE)) {
			/* Not to be announced */
			continue;
		    }
		    if (BIT_TEST(rt->rt_state, RTS_HIDDEN|RTS_DELETE) && !BIT_TEST(rt->rt_state, RTS_HOLDDOWN)) {
			/* Deleted or hidden, unless in holddown */
			continue;
		    }
		    RTLIST_ADD(rtl, rt);
		} RT_TABLE_END(head, rt) ;
	    }
	} RT_TABLES_END(af1) ;
	break;

    default:
	RT_TABLE(RT_TREETOP(af), rt) {
	    if (BIT_TEST(rt->rt_state, RTS_NOADVISE)) {
		/* Not to be announced */
		continue;
	    }
	    if (BIT_TEST(rt->rt_state, RTS_HIDDEN|RTS_DELETE) && !BIT_TEST(rt->rt_state, RTS_HOLDDOWN)) {
		/* Deleted or hidden, unless in holddown */
		continue;
	    }
	    RTLIST_ADD(rtl, rt);
	} RT_TABLE_END(RT_TREETOP(af), rt) ;
    }

    return rtl ? rtl->rtl_root : rtl;
}


rt_list *
rtlist_all __PF1(af, int)
{
    register rt_entry *rt;
    register rt_list *rtl = (rt_list *) 0;

    switch (af) {
    case AF_UNSPEC:
	RT_TABLES(af1) {
	    radix_node *head = RT_TREETOP(af1);

	    if (head) {
		RT_WHOLE(head, rt) {
		    RTLIST_ADD(rtl, rt);
		} RT_WHOLE_END(head, rt) ;
	    }
	} RT_TABLES_END(af1) ;
	break;

    default:
	RT_WHOLE(RT_TREETOP(af), rt) {
	    RTLIST_ADD(rtl, rt);
	} RT_WHOLE_END(RT_TREETOP(af), rt) ;
    }

    return rtl ? rtl->rtl_root : rtl;
}


rt_list *
rthlist_all __PF1(af, int)
{
    register rt_head *rth;
    register rt_list *rtl = (rt_list *) 0;

    switch (af) {
    case AF_UNSPEC:
	RT_TABLES(af1) {
	    radix_node *head = RT_TREETOP(af1);

	    if (head) {
		RT_WALK(head, rth) {
		    RTLIST_ADD(rtl, rth);
		} RT_WALK_END(head, rth) ;
	    }
	} RT_TABLES_END(af1) ;
	break;

    default:
	RT_WALK(RT_TREETOP(af), rth) {
	    RTLIST_ADD(rtl, rth);
	} RT_WALK_END(RT_TREETOP(af), rth) ;
    }

    return rtl ? rtl->rtl_root : rtl;
}


/* Build a change list for a specific protocol */
rt_list *
rtlist_proto __PF2(af, int,
		   protos, flag_t)
{
    register rt_entry *rt = (rt_entry *) 0;
    register rt_list *rtl = (rt_list *) 0;

    if (af == AF_UNSPEC) {
	/* All tables */

	RT_TABLES(af1) {
	    radix_node *head = RT_TREETOP(af1);

	    if (head) {
		RT_WHOLE(head, rt) {
		    if (RTPROTO_BIT(rt->rt_gwp->gw_proto) & protos) {
			RTLIST_ADD(rtl, rt);
		    }
		} RT_WHOLE_END(head, rt) ;
	    }
	} RT_TABLES_END(af1) ;
    } else {
	/* Only tables for this family */

	RT_WHOLE(RT_TREETOP(af), rt) {
	    if (RTPROTO_BIT(rt->rt_gwp->gw_proto) & protos) {
		RTLIST_ADD(rtl, rt);
		break;
	    }
	} RT_WHOLE_END(RT_TREETOP(af), rt) ;
    }

    return rtl ? rtl->rtl_root : rtl;
}


/* Build a change list based on the gateway pointer */
rt_list *
rtlist_gw __PF1(gwp, gw_entry *)
{
    int af = (gwp && gwp->gw_addr) ? socktype(gwp->gw_addr) : AF_UNSPEC;
    rt_entry *rt = (rt_entry *) 0;
    rt_list *rtl = (rt_list *) 0;

    if (af == AF_UNSPEC) {
	/* All families */

	RT_TABLES(af1) {
	    radix_node *head = RT_TREETOP(af1);

	    if (head) {
		RT_WHOLE(head, rt) {
		    if (rt->rt_gwp == gwp) {
			RTLIST_ADD(rtl, rt);
		    }
		} RT_WHOLE_END(head, rt) ;
	    }
	} RT_TABLES_END(af1) ;
    } else {
	/* Just this family */

	RT_WHOLE(RT_TREETOP(af), rt) {
	    if (rt->rt_gwp == gwp) {
		RTLIST_ADD(rtl, rt);
	    }
	} RT_WHOLE_END(RT_TREETOP(af), rt) ;
    } 

    return rtl ? rtl->rtl_root : rtl;
}



#ifdef	RT_SANITY
/*
 *	Check the integrity of the routing table
 */
void
rt_sanity __PF0(void)
{
    rt_head *rth;
    rt_entry *rt;
    int bounds_error = FALSE;
    byte *high = (byte *) sbrk(0);
    extern byte *end;

#define	BOUNDS(p)	((byte *) (p) < end || (byte *) (p) > high)

    RT_TABLES(af) {
	radix_node *head = RT_TREETOP(af);

	if (head) {
	    RT_WALK(head, rth) {
		int route = 0;
		int head_error = FALSE;

		/* First check for bounds on radix tree */
		if (BOUNDS(rn)) {
		    trace(TR_ALL, 0, "rt_sanity: bounds check on radix node pointer (%X)",
			  rn);
		    head_error = TRUE;
		    bounds_error = TRUE;
		} else if (BOUNDS(rn->rnode_p)) {
		    trace(TR_ALL, 0, "rt_sanity: bounds check on parent tree pointer (%X)",
			  rn->rnode_p);
		    head_error = TRUE;
		    bounds_error = TRUE;
		    /* XXX - Some more checks on the tree */
		} else if (BOUNDS(rth)) {
		    trace(TR_ALL, 0, "rt_sanity: bounds check on head pointer (%X)",
			  rth);
		    head_error = TRUE;
		    bounds_error = TRUE;
		} else {
		    /* If there is an active route, it had better be the first one */
		    if (rth->rth_active && rth->rth_active != rth->rt_forw) {
			trace(TR_ALL, 0, "rt_sanity: active route (%X != %X)",
			      rth->rth_active,
			      rth->rt_forw);
			head_error = TRUE;
		    }

		    /* Then check the routes */
		    RT_ALLRT(rt, rth) {
			int route_error = FALSE;
		
			/* First check for bounds check on pointers */
			if (BOUNDS(rt)) {
			    trace(TR_ALL, 0, "rt_sanity: bounds check on route pointer (%X)",
				  rt);
			    route_error = TRUE;
			    bounds_error = TRUE;
			} else {
			    if (rt->rt_head != rth) {
				trace(TR_ALL, 0, "rt_sanity: upward pointer (%X != %X)",
				      rt->rt_head,
				      rth);
				route_error = TRUE;
			    }

			    if (BOUNDS(rt->rt_forw)) {
				trace(TR_ALL, 0, "rt_sanity: bounds check on forward route pointer (%X)",
				      rt->rt_forw);
				route_error = TRUE;
				bounds_error = TRUE;
			    } else if (rt->rt_forw->rt_back != rt) {
				trace(TR_ALL, 0, "rt_sanity: forward route pointer (%X != %X)",
				      rt->rt_forw->rt_back,
				      rt);
				route_error = TRUE;
			    }
			
			    if (BOUNDS(rt->rt_back)) {
				trace(TR_ALL, 0, "rt_sanity: bounds check on backward route pointer (%X)",
				      rt->rt_back);
				route_error = TRUE;
				bounds_error = TRUE;
			    } else if (rt->rt_back->rt_forw != rt) {
				trace(TR_ALL, 0, "rt_sanity: backward route pointer (%X != %X)",
				      rt->rt_back->rt_forw,
				      rt);
				route_error = TRUE;
			    }
			}
		    
			if (route_error) {
			    trace(TR_ALL, 0, "rt_sanity: route %d",
				  route);
			    break;
			}
		
			route++;
		    } RT_ALLRT_END(rt, rth) ;

		    if (head_error) {
			trace(TR_ALL, 0, "rt_sanity: dest %A mask %A",
			      rth->rth_dest,
			      rth->rth_dest_mask);
		    }
		}
	    } RT_WALK_END(head, rth) ;
	}
    } RT_TABLES_END(af) ;

    if (bounds_error) {
	trace(TR_ALL, 0, "rt_sanity: bounds: low %#X high %#X",
	      end,
	      high);
    }
}
#endif	/* RT_SANITY */


#ifdef	PROTO_SNMP
rt_head *
rt_table_getnext __PF4(dst, sockaddr_un *,
		       af, int,
		       flag, flag_t,
		       proto_mask, flag_t)
{
    int skip = FALSE;
    register radix_node *nrn = RT_TREETOP(af);

    if (nrn) {
	register rt_head *rth;
	
	if (dst) {
	    register byte *cp = (byte *) dst;
	    register byte *cp2;
	    byte *cplim = cp + socksize(dst);
	    int off = nrn->rnode_off;

	    /* Search down the tree */
	    while (nrn->rnode_b >= 0) {
		if (nrn->rnode_bmask & cp[nrn->rnode_off]) {
		    nrn = nrn->rnode_r;
		} else {
		    nrn = nrn->rnode_l;
		}
	    }

	    for (cp = (byte *) dst + off,
		 cp2 = (byte *) nrn->rnode_key + off;
		 cp < cplim;
		 cp++, cp2++) {
		if (*cp != *cp2) {
		    break;
		}
	    }

	    if (cp == cplim) {
		/* Exact match, skip one */
		skip = TRUE;
	    } else {
		/* Start back at the top and do a lexigraphic search */

		for (nrn = RT_TREETOP(af);
		     nrn->rnode_b >= 0;
		     nrn = (*cp > *cp2) ? nrn->rnode_r : nrn->rnode_l) {
		    /* Find greatest in left subtree */
		    register radix_node *orn = nrn->rnode_l;

		    while (orn->rnode_b >= 0) {
			orn = orn->rnode_r;
		    }

		    /* Compare */
		    for (cp = (byte *) dst + off,
			 cp2 = (byte *) orn->rnode_key + off;
			 cp < cplim;
			 cp++, cp2++) {
			if (*cp != *cp2) {
			    break;
			}
		    }
		}
	    }
	}
	
	/* Return first valid entry */
	RT_WALK(nrn, rth) {
	    if (skip) {
		skip = FALSE;
		continue;
	    }
	    
	    if (!rth->rth_active && !rth->rth_holddown) {
		continue;
	    }

	    if (proto_mask) {
		register rt_entry *rt;

		    RT_ALLRT(rt, rth) {
			if (!BIT_TEST(rt->rt_state, RTS_DELETE|flag) &&
			    BIT_TEST(proto_mask, RTPROTO_BIT(rt->rt_gwp->gw_proto))) {
			    return rth;
			}
		    } RT_ALLRT_END(rt, rth) ;
	    } else {
		return rth;
	    }
	} RT_WALK_END(nrn, rth) ;
    }

    return (rt_head *) 0;
}
#endif	/* PROTO_SNMP */

/*
 *	Display information about the radix tree, including a visual
 *	representation of the tree itself 
 */
void
rt_table_dump __PF2(tp, task *,
		    fd, FILE *)
{
    struct {
	radix_node *rn;
	int	state;
	char	right;
    } stack[MAXKEYLEN*NBBY], *sp;
    char prefix[MAXKEYLEN*NBBY];
    int i = MAXKEYLEN * NBBY;

    while (i--) {
	prefix[i] = ' ';
    }

    (void) fprintf(fd,
		   "\tRadix trees ('+' = normal, '*' = root; '-' = single, '=' = duped)\n\n");

    RT_TABLES(af) {

	sp = stack;
	
	if (sp->rn = RT_TREETOP(af)) {
	    int off = sp->rn->rnode_off;

	    sp->state = 0;
	    sp->right = -1;

	    (void) fprintf(fd, "\tRadix tree for %s (%d) offset %d:\n\n",
			   af ? gd_lower(trace_value(task_domain_bits, af)) : "masks",
			   af,
			   off);

	    if (rtaf_info[af].rtaf_dests < 200) {

		/* If the tree is small enough, format it */
		do {
		    radix_node *rn = sp->rn;
		    int ii = (sp - stack) * 4;

		    switch (sp->state) {
		    case 0:
			sp->state++;
			if (rn->rnode_b >= 0 && rn->rnode_r) {
			    sp++;
			    sp->rn = rn->rnode_r;
			    sp->right = TRUE;
			    sp->state = 0;
			    continue;
			}
			/* Fall through */

		    case 1:
			sp->state++;
	
			(void) fprintf(fd, "\t\t%.*s%c",
				       ii, prefix,
				       BIT_TEST(rn->rnode_flags, RNODEF_ROOT) ? '*' : '+');
			if (rn->rnode_b < 0) {
			    radix_node *drn;

			    if (!BIT_TEST(rn->rnode_flags, RNODEF_ROOT) || !(drn = rn->rnode_dupedkey)) {
				drn = rn;
			    }

			    (void) fprintf(fd, "%c%A\n",
					   rn->rnode_dupedkey ? '=' : '-',
					   drn->rnode_key);
			} else {
			    char number[4];

			    (void) sprintf(number,
					   "%3d",
					   rn->rnode_b - (off * NBBY));

			    for (i = 0; i < 3; i++) {
				if (number[i] == ' ') {
				    number[i] = '-';
				} else {
				    break;
				}
			    }
			
			    (void) fprintf(fd, "%s+\n",
					   number);
			}

			switch (sp->right) {
			case TRUE:
			    prefix[ii] = '|';
			    break;

			case FALSE:
			    prefix[ii] = ' ';
			    break;
			}

			if (rn->rnode_b >= 0 && rn->rnode_l) {
			    sp++;
			    sp->rn = rn->rnode_l;
			    sp->right = FALSE;
			    sp->state = 0;
			    continue;
			}

		    case 2:
			/* Pop the stack */
			sp--;
		    }
		} while (sp >= stack) ;

	    }

	    (void) fprintf(fd, "\n");
	}
    } RT_TABLES_END(af) ;

    (void) fprintf(fd, "\n");
}


void
rt_table_init_family __PF4(af, int,
			   offset, int,
			   min, sockaddr_un *,
			   max, sockaddr_un *)
{
    trace(TR_INT, 0, "rt_table_init_family: Initializing radix tree for %s",
	  trace_value(task_domain_bits, af));

    rnode_inithead(&RT_TREETOP(af), offset, min, max);

    trace(TR_INT, 0, NULL);
}


/*
 *	Initialize rt heads for all protocols 
 */
void
rt_table_init __PF0(void)
{
    rt_mask_block_index = task_block_init(sizeof (radix_mask));
    rt_node_block_index = task_block_init(sizeof (radix_node));

#ifdef	notyet
    {
	byte *cp, *cplim;
	static char rnode_zeros[MAXKEYLEN], rnode_ones[MAXKEYLEN];

	/* Init all ones mask */
	for (cp = rnode_ones, cplim = rnode_ones + MAXKEYLEN; cp < cplim; *cp++ = -1) ;

	/* Init mask tree */
	rt_table_init_family(AF_UNSPEC,
			     0,
			     (sockaddr_un *) rnode_zeros,
			     (sockaddr_un *) rnode_ones);
    }
#endif	/* notyet */
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
