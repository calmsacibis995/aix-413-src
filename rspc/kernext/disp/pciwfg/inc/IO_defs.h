/* @(#)86  1.2 src/rspc/kernext/disp/pciwfg/inc/IO_defs.h, pciwfg, rspc41J, 9507C 1/27/95 03:08:13 */
/*
 *   COMPONENT_NAME: pciwfg
 *
 *   FUNCTIONS: GET_CRTC_REG
 *              GET_GRF_REG
 *              GET_SEQ_REG
 *              PCI_GETC
 *              PCI_GETS
 *              PCI_PUTC
 *              PCI_PUTL
 *              PCI_PUTS
 *              PUT_ATTR_REG
 *              PUT_CRTC_REG
 *              PUT_GRF_REG
 *              PUT_SEQ_REG
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_IO_DEFS
#define _H_IO_DEFS

#ifndef LITTLE_ENDIAN
/*---------------------------------------------------------------------------*/
/*                    PCI Bus is big-endian (As it started)                  */
/*---------------------------------------------------------------------------*/
/*      define byte reversed inline code                                     */
/*---------------------------------------------------------------------------*/
#ifndef REV_PRAGMAS
#define REV_PRAGMAS
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
#endif /* REV_PRAGMAS */
#endif /* BIG_ENDIAN */

/*---------------------------------------------------------------------------*/
/*  Function: PCI_GETC()                                                     */
/*            This routine will read a byte from the I/O addr specified.     */
/*                                                                           */
/*  IO_base_address must be set up for access to the IO Bus                  */
/*                                                                           */
/*  Input:    addr - Address of I/O location to read from                    */
/*                                                                           */
/*  Output:   Value read from I/O address                                    */
/*---------------------------------------------------------------------------*/
#define PCI_GETC(addr)                                          \
    *(volatile unsigned char *)(ld->IO_base_address | ((addr) & 0x0000FFFF))

/*---------------------------------------------------------------------------*/
/*  Function: PCI_PUTC()                                                     */
/*            This routine will write a byte value to the specified I/O      */
/*            address.                                                       */
/*                                                                           */
/*  IO_base_address must be set up for access to the IO Bus                  */
/*                                                                           */
/*  Input:    addr -     I/O address to write to                             */
/*            value -    value to write to I/O address                       */
/*                                                                           */
/*  Output:   No return value                                                */
/*---------------------------------------------------------------------------*/
#define PCI_PUTC(addr, value)   {                                    \
    *(volatile unsigned char *)(ld->IO_base_address | ((addr) & 0x0000FFFF)) \
    = (unsigned char)(value) ; EIEIO;                                        \
}

/*---------------------------------------------------------------------------*/
/*  Function: PCI_PUTS()                                                     */
/*            This routine will write a 16 bit value to the specified I/O    */
/*            address.                                                       */
/*                                                                           */
/*  IO_base_address must be set up for access to the IO Bus                  */
/*                                                                           */
/*  Input:    addr -     I/O address to write to (Must be 16 bit aligned)    */
/*            value    - value to write to I/O address                       */
/*                                                                           */
/*  Output:   No return value                                                */
/*---------------------------------------------------------------------------*/
#ifndef  LITTLE_ENDIAN
/*      BIG_ENDIAN PCI BUS     */
/*
   *(volatile unsigned short *)(ld->IO_base_address | (addr)) =
            (unsigned short)(((value) << 8) + ((value) >> 8));
*/
#define PCI_PUTS(addr, value)    {                           \
   st_2r((value),(ld->IO_base_address | (addr)));                \
   EIEIO;                                                    \
}
#else
/*      LITTLE_ENDIAN PCI BUS     */
#define PCI_PUTS(addr, value)     {                          \
   *(volatile unsigned short *)(ld->IO_base_address | (addr)) =  \
            (unsigned short)(value);                         \
   EIEIO;                                                    \
}
#endif /* BIG_ENDIAN */

/*---------------------------------------------------------------------------*/
/*  Function: PCI_GETS()                                                     */
/*            This routine will read a 16 bit value from the specified I/O   */
/*            address.                                                       */
/*                                                                           */
/*  IO_base_address must be set up for access to the IO Bus                  */
/*                                                                           */
/*  Input:    addr -     I/O address to read from (Must be 16 bit aligned)   */
/*                                                                           */
/*  Output:   value at specified address                                     */
/*---------------------------------------------------------------------------*/
#ifndef  LITTLE_ENDIAN
/*      BIG_ENDIAN PCI BUS     */
#define PCI_GETS(addr)                                       \
   ld_2r(ld->IO_base_address | (addr));                      \
   EIEIO;
#else
/*      LITTLE_ENDIAN PCI BUS     */
#define PCI_GETS(addr)                                        \
   *(volatile unsigned short *)(ld->IO_base_address | (addr));\
   EIEIO;
#endif /* BIG_ENDIAN */


/*---------------------------------------------------------------------------*/
/*  Function: PCI_PUTL()                                                     */
/*            This routine will write a 32 bit value to the specified I/O    */
/*            address.                                                       */
/*                                                                           */
/*  Input:    addr -     I/O address to write to (Must be 32 bit aligned)    */
/*            value -    value to write to I/O address                       */
/*                                                                           */
/*  Output:   No return value                                                */
/*---------------------------------------------------------------------------*/
#ifndef  LITTLE_ENDIAN
#define PCI_PUTL(addr, value)   {                            \
    st_4r((value),(ld->IO_base_address | (addr)));               \
    EIEIO;                                                   \
}
#else
#define PCI_PUTL(addr, value)   {                            \
   *(volatile unsigned long *)(ld->IO_base_address | (addr)) =   \
            (unsigned long)(value);                          \
    EIEIO;                                                   \
}
#endif /* BIG_ENDIAN */

#define DAC_PALETTE \
              (volatile unsigned char *)(ld->IO_base_address | (DAC_DATA))

/*--------------------------------------------------------------------------*/
/*      Macros to access indirect registers                                 */
/*--------------------------------------------------------------------------*/
#define PUT_ATTR_REG(indx, data) {  /* Attribute Controller Registers */    \
                                                                            \
   PCI_PUTC(ATTR_INDX,(indx));                                              \
   PCI_PUTC(ATTR_DATA,(data));                                              \
}
#define PUT_CRTC_REG(indx, data) {  /* CRT Controller Registers       */    \
                                                                            \
   PCI_PUTC(CRTC_INDX,(indx));                                              \
   PCI_PUTC(CRTC_DATA,(data));                                              \
}
#define GET_CRTC_REG(indx, tgt) {   /* CRT Controller Registers       */    \
                                                                            \
   PCI_PUTC(CRTC_INDX,(indx));                                              \
   (tgt) = PCI_GETC(CRTC_DATA);                                             \
}
#define PUT_SEQ_REG(indx, data) {   /* Sequencer Registers            */    \
                                                                            \
   PCI_PUTC(SEQ_INDX,(indx));                                               \
   PCI_PUTC(SEQ_DATA,(data));                                               \
}
#define GET_SEQ_REG(indx, tgt) {    /* Sequencer Registers            */    \
                                                                            \
   PCI_PUTC(SEQ_INDX,(indx));                                               \
   (tgt) = PCI_GETC(SEQ_DATA);                                              \
}
#define PUT_GRF_REG(indx, data) {   /* Graphics Controller Registers  */    \
                                                                            \
   PCI_PUTC(GRF_INDX,(indx));                                               \
   PCI_PUTC(GRF_DATA,(data));                                               \
}
#define GET_GRF_REG(indx, tgt) {    /* Graphics Controller Registers  */    \
                                                                            \
   PCI_PUTC(GRF_INDX,(indx));                                               \
   (tgt) = PCI_GETC(GRF_DATA);                                              \
}
#define PUT_EPR_REG(indx, data) {   /* Extended Paradise  Registers   */    \
                                                                            \
   PCI_PUTS(EPR_INDX,(indx));                                               \
   PCI_PUTS(EPR_DATA,(data));                                               \
}
#define GET_EPR_REG(indx, tgt) {    /* Extended Paradise Registers    */    \
                                                                            \
   PCI_PUTS(EPR_INDX,(indx));                                               \
   (tgt) = PCI_GETS(EPR_DATA);                                              \
}

/*-------------------------------------------------------------------------*/
/*      Mode Table Definitions                                             */
/*-------------------------------------------------------------------------*/

typedef struct _PARADISE_MODE_TABLE
{
   unsigned char   pr_crtc_disable[6];
   int             pr_seq_length;
   unsigned char   pr_seq[36];
   unsigned char   pr_gcr[12];
   int             pr_crtc_length;
   unsigned char   pr_crtc[80];            /* Paradise CRTC Table             */
} PARADISE_MODE_TABLE;

typedef struct _BASE_MODE_TABLE
{
   short        screen_h;                /* Screen Height in pels           */
   short        screen_w;                /* Screen Width in pels            */
   unsigned char misc_output[2];         /* One for each Monitor            */
   unsigned char seq[4];                 /* seq,                            */
   unsigned char attributes[21];         /* attribute, and Graphics         */
   unsigned char gcr[9];                 /* Graphics controller             */
} BASE_MODE_TABLE;

typedef struct _V_MODE
{
   unsigned char    Memory_Mode;         /* Sequencer 04h                   */
   unsigned long    bits_per_pel;        /* Bits per Pixel                  */
   BASE_MODE_TABLE *Base_Mode_Table;     /* Base Mode Table                 */
   PARADISE_MODE_TABLE  *PR_Mode_Table;  /* Paradise Mode Table             */
   unsigned char    *crtc_iso;           /* Frequency Dependent Tables      */
   unsigned char    *crtc_non_iso;       /* Frequency Dependent Tables      */
} V_MODE;

#endif  /* _H_IO_DEFS */
