/* @(#)76  1.1 src/rspc/kernext/pci/pcivca/inc/vcamac.h, pcivca, rspc411, 9434B411a 8/24/94 07:29:30 */
/******************************************************************************
 * COMPONENT_NAME: (PCIVCA) PCI Video Capture Device Driver
 *
 * FUNCTIONS: vcamac.h -  header file
 *
 * ORIGINS:     27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

#define EIEIO __iospace_eieio() /* Enforce Instruction Execution In Order */

/*
 * Define pragma for inline sync instruction
 */
void st_2r();                           /* prototype for 2-byte store        */
void st_4r();                           /* prototype for 4-byte store        */
short ld_2r();                          /* prototype for 2-byte load         */
long ld_4r();                           /* prototype for 4-byte load         */
#pragma mc_func st_2r { "7C60272C" }    /* sthbrx (vol_short_int)(gr4,0)=gr3 */
#pragma mc_func st_4r { "7C60252C" }    /* stbrx (vol_int)(gr4,0)=gr3        */
#pragma mc_func ld_2r { "7C601E2C" }    /* lhbrx gr3=(vol_int)(gr3,0)        */
#pragma mc_func ld_4r { "7C601C2C" }    /* lwbrx gr3=(vol_int)(gr3,0)        */
#pragma reg_killed_by st_2r             /* (none killed)                     */
#pragma reg_killed_by st_4r             /* (none killed)                     */
#pragma reg_killed_by ld_4r gr3         /* gr3 killed                        */
#pragma reg_killed_by ld_2r gr3         /* gr3 killed                        */
#pragma isolated_call (st_2r)
#pragma isolated_call (st_4r)
#pragma isolated_call (ld_4r)
#pragma isolated_call (ld_2r)


/*
 * Macros to convert to and from bus byte order.  Note that these macros
 * assume (and require) that shorts are 16 bits and longs are 32 bits.
 * Conversions both directions are provided for consistancy sake only;
 * i.e., the operations are isomorphic.
 *
 *      StoCBO()        Convert short to Canonical Byte Order
 *      LtoCBO()        Convert long to Canonical Byte Order
 *      CBOtoS()        Convert short from Canonical Byte Order
 *      CBOtoL()        Convert long from Canonical Byte Order
 */

#define StoCBO(h)       ((ushort) (                   \
                        (((ushort)(h)<<8) & 0xff00) | \
                        (((ushort)(h)>>8) & 0x00ff)   \
                        ))
#define LtoCBO(w)       ((ulong) (                         \
                        (((ulong)(w)>>24)             )  | \
                        (((ulong)(w)>> 8) & 0x0000ff00)  | \
                        (((ulong)(w)<< 8) & 0x00ff0000)  | \
                        (((ulong)(w)<<24)             )    \
                        ))
#define CBOtoS(h)       StoCBO(h)
#define CBOtoL(w)       LtoCBO(w)

#define BYTE_REV(n)                                               \
        ((((n) & 0x000000FF) << 24) + (((n) & 0x0000FF00) << 8) + \
        (((n) & 0x00FF0000) >> 8) + (((n) & 0xFF000000) >> 24))

#ifdef VCADBG
#define VCADBG_0(A)                      {printf(A);}
#define VCADBG_1(A,B)                    {printf(A,B);}
#define VCADBG_2(A,B,C)                  {printf(A,B,C);}
#define VCADBG_3(A,B,C,D)                {printf(A,B,C,D);}
#define VCADBG_4(A,B,C,D,E)              {printf(A,B,C,D,E);}
#define VCADBG_5(A,B,C,D,E,F)            {printf(A,B,C,D,E,F);}
#define VCADBG_6(A,B,C,D,E,F,G)          {printf(A,B,C,D,E,F,G);}
#else
#define VCADBG_0(A)
#define VCADBG_1(A,B)
#define VCADBG_2(A,B,C)
#define VCADBG_3(A,B,C,D)
#define VCADBG_4(A,B,C,D,E)
#define VCADBG_5(A,B,C,D,E,F)
#define VCADBG_6(A,B,C,D,E,F,G)
#endif

