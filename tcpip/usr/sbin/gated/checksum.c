static char sccsid[] = "@(#)65	1.1  src/tcpip/usr/sbin/gated/checksum.c, tcprouting, tcpip411, GOLD410 12/6/93 17:24:51";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: ADDCARRY
 *		__PF2
 *		__PF3
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
 * checksum.c,v 1.9 1993/03/25 12:46:38 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#include "include.h"

/*
 * Checksum routine for Internet Protocol - Modified from 4.3+ networking in_chksum.c
 * 
 */

#define ADDCARRY(x)  (x > 65535 ? x -= 65535 : x)
#define REDUCE {l_util.l = sum; sum = l_util.s[0] + l_util.s[1]; ADDCARRY(sum);}

u_int16
inet_cksumv __PF3(v, register struct iovec *,
		  nv, register int,
		  len, register size_t)
{
    register int sum = 0;
    register int vlen = 0;
    register struct iovec *vp;
    int byte_swapped = 0;
    union {
	char c[2];
	u_int16 s;
    } s_util;
    union {
	u_int16 s[2];
	long l;
    } l_util;

    for (vp = v; nv && len; nv--, vp++) {
	register union {
	    caddr_t c;
	    u_int16 *s;
	} w;

	if (vp->iov_len == 0) {
	    continue;
	}
	w.c = vp->iov_base;
	if (vlen == -1) {

	    /*
	     * The first byte of this mbuf is the continuation of a word spanning between this mbuf and the last mbuf.
	     * 
	     * s_util.c[0] is already saved when scanning previous mbuf.
	     */
	    s_util.c[1] = *w.c;
	    sum += s_util.s;
	    w.c++;
	    vlen = vp->iov_len - 1;
	    len--;
	} else {
	    vlen = vp->iov_len;
	}
	if (len < vlen) {
	    vlen = len;
	}
	len -= vlen;

	/*
	 * Force to even boundary.
	 */
	if ((1 & (int) w.s) && (vlen > 0)) {
	    REDUCE;
	    sum <<= NBBY;
	    s_util.c[0] = *w.c;
	    w.c++;
	    vlen--;
	    byte_swapped = 1;
	}

	/*
	 * Unroll the loop to make overhead from branches &c small.
	 */
	while ((vlen -= 32) >= 0) {
	    sum += w.s[0];
	    sum += w.s[1];
	    sum += w.s[2];
	    sum += w.s[3];
	    sum += w.s[4];
	    sum += w.s[5];
	    sum += w.s[6];
	    sum += w.s[7];
	    sum += w.s[8];
	    sum += w.s[9];
	    sum += w.s[10];
	    sum += w.s[11];
	    sum += w.s[12];
	    sum += w.s[13];
	    sum += w.s[14];
	    sum += w.s[15];
	    w.s += 16;
	}
	vlen += 32;
	while ((vlen -= 8) >= 0) {
	    sum += w.s[0];
	    sum += w.s[1];
	    sum += w.s[2];
	    sum += w.s[3];
	    w.s += 4;
	}
	vlen += 8;
	if (vlen == 0 && byte_swapped == 0) {
	    continue;
	}
	REDUCE;
	while ((vlen -= 2) >= 0) {
	    sum += *w.s++;
	}
	if (byte_swapped) {
	    REDUCE;
	    sum <<= NBBY;
	    byte_swapped = 0;
	    if (vlen == -1) {
		s_util.c[1] = *w.c;
		sum += s_util.s;
		vlen = 0;
	    } else {
		vlen = -1;
	    }
	} else if (vlen == -1) {
	    s_util.c[0] = *w.c;
	}
    }
    assert(!len);
    if (vlen == -1) {

	/*
	 * The last buffer has odd # of bytes. Follow the standard (the odd byte may be shifted left by 8 bits or not as
	 * determined by endian-ness of the machine)
	 */
	s_util.c[1] = 0;
	sum += s_util.s;
    }
    REDUCE;
    return ~sum & 0xffff;
}


u_int16
inet_cksum __PF2(cp, void_t,
		 length, size_t)
{
    struct iovec iovec;

    iovec.iov_base = (caddr_t) cp;
    iovec.iov_len = (int) length;

    return inet_cksumv(&iovec, 1, length);
}


#ifdef	FLETCHER_CHECKSUM

/*
 * iso_cksum.c - compute the ISO (Fletcher) checksum.  Can be used for both computing the checksum and inserting it in the
 * packet, and for checking an already-checksummed packet
 */

/*
 * The variables c0, c1 and l (and X and Y, though these were eliminated) are as in ISO 8073.
 * 
 * The limit on the number of bytes to process before doing a MOD (MAXITER) is derived from what it takes to avoid overflowing
 * the 31 bit c1 during the summation (I computed an actual limit of 4102 for the worst case packet, which is conveniently
 * close to 4096).  Doing this minimizes the number of divisions which must be done.
 * 
 * It is hard to make this checksum go fast.  It very much requires byte-at-a-time processing, and sequential processing since
 * all computations are dependent on immediately previous results, so trying to run with 32-bit loads ends up losing much of
 * the potential advantage to complexity.
 * 
 * This implementation is a compromise.  Beyond minimizing the number of divides (the old OSPF checksum was doing one extra) it
 * inlines all inner loops 8-at-a-time.  This gives compilers an opportunity to fill at least some of the load-delay slots
 * by code rearrangement.  The results of this are dependent on the particular machine and compiler, but in no case has the
 * speed been observed to be worse than the old OSPF code even for short packets, and for a few machines the speedup is a
 * factor of two or better (for MIPS machines and compilers, in particular).
 */

#define	INLINE		8		/* number of inline computations */
#define	INSHIFT		3		/* i.e. inlined by 1<<INSHIFT */
#define	INLINEMASK	(INLINE - 1)	/* mask for uneven bits */

#define	MAXITER		4096		/* number of iterations before MOD */
#define	ITERSHIFT	12		/* log2 of number of interations */
#define	ITERMASK	(MAXITER - 1)	/* mask to tell if we need mod or not */
#define	MAXINLINE	(MAXITER/INLINE)/* number of inline iterations */

#define	MODULUS		255		/* modulus for checksum */

/*
 * The arguments are as follows
 * 
 * pkt   - start of packet to be checksummed len   - contiguous length of packet to be checksummed cksum - optional pointer to
 * location of checksum in packet.  If non-NULL we initialize to zero and fill in the checksum when done.
 * 
 * If cksum is non-NULL the return value is the value which was inserted in the packet, in host byte order.  If cksum is NULL
 * the results of the checksum sum are returned.  The return value should be zero for an already-checksummed packet.
 */
u_int32
iso_cksum __PF3(pkt, void_t,
		len, size_t,
		cksum, byte *)
{
    register s_int32 c0, c1;
    register byte *cp;
    register int n;
    register int l;

    c0 = c1 = 0;
    cp = (byte *) pkt;
    l = len;

    /*
     * Initialize checksum to zero if there is one
     */
    if (cksum) {
	*cksum = *(cksum + 1) = 0;
    }

    /*
     * Process enough of the packet to make the remaining length an even multiple of MAXITER (4096).  The switch() adds a
     * lot of code, but trying to do this with less results in a big slowdown for short packets.
     */
    n = l & ITERMASK;
    switch (n & INLINEMASK) {
    case 7:
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	break;
    case 6:
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	break;
    case 5:
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	break;
    case 4:
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	break;
    case 3:
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	break;
    case 2:
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	break;
    case 1:
	c1 += (c0 += (s_int32) (*cp++));
	break;
    case 0:
	break;
    }

    n >>= INSHIFT;
    while (n-- > 0) {
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
	c1 += (c0 += (s_int32) (*cp++));
    }

    /*
     * Now process the remainder in MAXITER chunks, with a mod beforehand to avoid overflow.
     */
    n = l >> ITERSHIFT;
    if (n > 0) {
	do {
	    register int iter = MAXINLINE;

	    if (cp != (byte *) pkt) {
		c0 %= MODULUS;
		c1 %= MODULUS;
	    }
	    do {
		c1 += (c0 += (s_int32) (*cp++));
		c1 += (c0 += (s_int32) (*cp++));
		c1 += (c0 += (s_int32) (*cp++));
		c1 += (c0 += (s_int32) (*cp++));
		c1 += (c0 += (s_int32) (*cp++));
		c1 += (c0 += (s_int32) (*cp++));
		c1 += (c0 += (s_int32) (*cp++));
		c1 += (c0 += (s_int32) (*cp++));
	    } while (--iter > 0);
	} while (--n > 0);
    }

    /*
     * Take the modulus of c0 now to avoid overflow during the multiplication below.  If we're computing a checksum for the
     * packet, do it and insert it.
     */
    c0 %= MODULUS;
    if (cksum) {

	/*
	 * c1 used for Y.  Can't overflow since we're taking the difference between two positive numbers
	 */
	c1 = (c1 - ((s_int32) (((byte *) pkt + l) - cksum) * c0)) % MODULUS;
	if (c1 <= 0) {
	    c1 += MODULUS;
	}

	/*
	 * Here we know c0 has a value in the range 0-254, and c1 has a value in the range 1-255.  If we subtract the sum
	 * from 255 we end up with something in the range -254-254, and only need correct the -ve value.
	 */
	c0 = MODULUS - c1 - c0;		/* c0 used for X */
	if (c0 <= 0) {
	    c0 += MODULUS;
	}
	*cksum++ = (byte) c0;
	*cksum = (byte) c1;

	return (u_int32) ((c0 << 8) | c1);
    }

    /*
     * Here we were just doing a check.  Return the results in a single value, they should both be zero.
     */
    return (u_int32) (((c1 % MODULUS) << 8) | c0);
}

#endif /* FLETCHER_CHECKSUM */


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
