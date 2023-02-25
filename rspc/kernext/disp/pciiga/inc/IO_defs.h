/* @(#)76	1.1  src/rspc/kernext/disp/pciiga/inc/IO_defs.h, pciiga, rspc411, 9436D411b 9/5/94 16:31:51 */
/*
 *   COMPONENT_NAME: pciiga
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
 *   (C) COPYRIGHT International Business Machines Corp. 1994
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
    *(volatile unsigned char *)(ld->IO_base_address | (addr))

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
    *(volatile unsigned char *)(ld->IO_base_address | (addr)) = (value); \
    EIEIO;                                                           \
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

/*-------------------------------------------------------------------------*/
/*      Mode Table Definitions                                             */
/*-------------------------------------------------------------------------*/
typedef struct _CRT_MODE_TABLE
{
   char * crtc;
   char * ecrtc;
   char * dev_ecrtc;
   unsigned char DAC_Mode[4];
} CRT_MODE_TABLE;

typedef struct _BASE_MODE_TABLE
{
   short        screen_h;               /* Screen Height in pels            */
   short        screen_w;               /* Screen Width in pels             */
   long         regen_buffer_size;
   unsigned char misc_output[4];        /* One for each Monitor             */
   unsigned char xfudge[4];             /* One for each Monitor             */
   unsigned char yfudge[4];             /* One for each Monitor             */
   unsigned char seq[4];                /* seq,                             */
   unsigned char attributes[30];        /* attribute, and Graphics          */
} BASE_MODE_TABLE;

typedef struct _VESA_MODE
{
   short            mode_number;        /* Internal Mode Number             */
   BASE_MODE_TABLE *Base_Mode_Table;    /* Base Mode Table                  */
   unsigned char    Memory_Mode;        /* Sequencer 04h                    */
#  define             ExtMem      0x02  /* Allow access to 256KB            */
#  define             SeqAddr     0x04  /* Sequential addressing            */
#  define             Chain4      0x08  /* Chain 4 addressing mode          */
   unsigned char    Advanced_Function;  /* 4ae8h                            */
#  define             EnhFunc     0x01  /* Enable Enhanced Functions        */
#  define             ScrnSize    0x04  /* Screen Size (< 640 x 480)        */
#  define             LinAddr     0x10  /* Enable Linear Addressing         */
#  define             MMapIO      0x20  /* Enable Memory Mapped I/O         */
#  define             WrtPost     0x40  /* Enable Write Posting into FIFO   */
   unsigned char    DAC_Mode;           /* DAC Mode                         */
   unsigned char    bits_per_pel;       /* Bits per Pixel                   */
   CRT_MODE_TABLE  *Extended_Mode_Table[4]; /* Frequency Dependent Tables   */
} VESA_MODE;

static unsigned char dev_ecrtc_none[] = { /* Dac specific extensions not needed */
    0x00, 0x00,
};

#endif  /* _H_IO_DEFS */
