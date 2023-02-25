static char sccsid[] = "@(#)36	1.1  src/rspc/usr/lib/boot/softros/roslib/siosetup.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:24";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef DEBUG

/* For pass 3 planars, Guaranteed access time must be disabled. */
#define value0 0x23000400

#define value4 0x00100f00
#define value8 0x0100100f
#define valuec 0x561007ff       // This value must have the ISA divisor bits
                                // set to zero.  The ISA divisor has already
                                // been set by earlier code, so this code
                                // will be OR'd in with the original bit settings.

void siosetup(void) {

   unsigned int inval;

   out32(0x80800840, value0);
   out32(0x80800844, value4);
   out32(0x80800848, value8);

   inval = in32(0x8080084c);
   inval = (valuec & 0xFFF8FFFF) | (inval & 0x00070000);       // Don't change ISA divisor!
   out32(0x8080084c, inval);

}
#endif /*DEBUG*/
