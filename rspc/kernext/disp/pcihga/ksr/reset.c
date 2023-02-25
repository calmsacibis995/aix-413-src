static char sccsid[] = "@(#)40  1.3  src/rspc/kernext/disp/pcihga/ksr/reset.c, pcihga, rspc411, GOLD410 4/29/94 12:08:38";
/* reset.c */
/*
based on "@(#)46  1.7  src/bos/kernext/disp/wga/ksr/reset.c, bos, bos410 10/28/93 17:45:13";
 *
 * COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: hga_reset
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


/* #define SHOW_HW_CURSOR 1 *//* show the hardware cursor */
/* #define SHOW_COLOR_BACK 1 *//* show a striped outer border */


#include "hga_INCLUDES.h"
#include "hga_funct.h"
#include <sys/m_param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include "IO_defs.h"

BUGXDEF (dbg_reset);

void bt485_write();
unsigned char bt485_read();

/*----------------------------------------------------------------------
* IDENTIFICATION:    RESET
*
* DESCRIPTIVE NAME:  Reset Graphic Adaptor
*
* FUNCTIONS:         This routine issues the pixel interface commands needed
*                    to reset the adaptor into a base conditions.  The
*                    following operations are performed:
*                       1.  Set up operating mode (# of Bits/Pel)
*                       2.  Establish VRAM Interface
*                       3.  Set CRT Controller Registers
*                       4.  Clear the Screen
*                       5.  Disable the Sprite
* INPUTS:            NONE
*
* OUTPUTS            Resets Adaptor
*
* CALLED BY          INIT
*
*
* REFERENCES:  Graphics Adaptor Specification
*
---------------------------------------------------------------------*/
#define TEST_PATTERN     0x00000000     /* VRAM initial value */

static unsigned char initial_ext_regs0[] = {
                                /* Registers are arranged in the order that     */
                                /* they need to be loaded                       */
      0x31, 0x85,               /* index 31h, memory control                    */
      0x50, 0x00,               /* index 50h, ext sys cont 1                    */
      0x51, 0x00,               /* index 51h, ext sys cont 2                    */
      0x53, 0x00,               /* index 53h, ext mem cont 1                    */
      0x54, 0x38,               /* index 54h, ext mem cont 2                    */
      0x55, 0x00                /* index 55h, ext video dac control (JAK)       */
      };

static unsigned char initial_ext_regs1[] = {
      0x5c, 0x00,               /* index 5ch, GOP control                       */
      0x5d, 0x00,               /* index 5dh, ext horizontal overflow           */
      0x5e, 0x00,               /* index 5eh, ext vertical overflow             */
      0x60, 0x07,               /* index 60h, ext mem cont 3                    */
      0x61, 0x80,               /* index 61h, ext mem cont 4                    */
      0x62, 0xa1,               /* index 62h, ext mem cont 5                    */
      0x32, 0x00,               /* index 32h, backward compat 1                 */
      0x33, 0x00,               /* index 33h, backward compat 2                 */
      0x34, 0x00,               /* index 34h, backward compt 3                  */
      0x35, 0x00,               /* index 35h, crtc lock                         */
      0x36, 0x9f,               /* index 36h, r/o data (MUST BE WRITTEN TO!!)   */
      0x37, 0xff,               /* index 37h, r/o data                          */
      0x3a, 0x05,               /* index 3ah, misc 1                            */
      0x3b, 0x5a,               /* index 3bh, data transfer exec pos            */
      0x3c, 0x10,               /* index 3ch, interlace retrace start           */
      0x40, 0x09,               /* index 40h, sys-cnfg                          */
      0x43, 0x00,               /* index 43h, extended mode                     */
      0x45, 0x00,               /* index 45h, hw graphics cursor mode           */
      0x46, 0x00,               /* index 46h, hw graphics cursor orig x         */
      0x47, 0xff,               /* index 47h, hw graphics cursor orig x         */
      0x48, 0xfc,               /* index 48h, hw graphics cursor orig y         */
      0x49, 0xff,               /* index 49h, hw graphics cursor orig y         */
      0x4a, 0xff,               /* index 4ah, hw graphics cursor orig y         */
      0x4b, 0xff,               /* index 4bh, hw graphics cursor orig y         */
      0x4c, 0xff,               /* index 4ch, hw graphics cursor orig y         */
      0x4d, 0xff,               /* index 4dh, hw graphics cursor orig y         */
      0x4e, 0xff,               /* index 4eh, dsp start x pixel pos             */
      0x4f, 0xdf                /* index 4fh, dsp start y pixel pos             */
      };

static unsigned char ecrtc_addrs[] = {  /* Extended CRT Controller Addresses    */
      0x42,                             /* Index 42h - Mode Control             */
      0x3b,                             /* Index 3bh - Data Transfer Execute Pos*/
      0x3c,                             /* Index 3ch - Interface Retrace Start  */
      0x31,                             /* Index 31h - Memory Configuration     */
      0x3a,                             /* Index 3ah - Miscellaneous 1          */
      0x40,                             /* Index 40h - System Configuration     */
      0x50,                             /* Index 50h - Extended System Control 1*/
      0x54,                             /* Index 54h - Extended Memory Control 2*/
      0x5d,                             /* Index 5dh - Ext Horizontal Overflow  */
      0x60,                             /* Index 60h - Extended Memory Control 3*/
      0x61,                             /* Index 61h - Extended Memory Control 4*/
      0x62,                             /* Index 62h - Extended Memory Control 5*/
      0x58,                             /* Index 58h - Linear Address Window Ctl*/
      0x33,                             /* Index 33h - Backward Compatibility 2 */
      0x43,                             /* Index 43h - Extended Mode            */
      0x13,                             /* Index 13h - Offset                   */
      0x5e,                             /* Index 5eh - Extended Vertical Overflw*/
      0x51,                             /* Index 51h - Extended System Control 2*/
      0x5c,                             /* Index 5ch - General Output Port      */
      0x34,                             /* Index 34h - Backward Compatibility 3 */
      0x00                              /* List Terminator                      */
      };

/*--------------------------------------------------------------------------*/
/*      Unlock all the S3 registers                                         */
/*--------------------------------------------------------------------------*/
#define S3_unlock(n) {                                                  \
   unsigned char t;                                                     \
                                                                        \
   PUT_CRTC_REG(REG_LOCK1, 0x48);/* Unlock the s3 vga registers.   */   \
   PUT_CRTC_REG(REG_LOCK2, 0xa5);/* Unlock system control and      */   \
   GET_CRTC_REG(VRE, t);    /* system extension registers.         */   \
   t &= ~0x80;              /* Reset the high-order bit of VRE to  */   \
   PCI_PUTC(CRTC_DATA, t);  /* insure CRT regs are unlocked.       */   \
   GET_CRTC_REG(SYS_CNFG, t);/* Read system config register and    */   \
   t |= 0x01;               /*   set bit 0 to unlock               */   \
   PCI_PUTC(CRTC_DATA, t);  /* the Enhanced Graphics Command group.*/   \
}

/*------------------------------------------------------------------*
 *               hga_reset()                                        *
 *------------------------------------------------------------------*/
long
hga_reset (vp, ld, ddf)
        struct vtmstruc *vp;
        struct hga_data *ld;
        struct hga_ddf *ddf;
{

        int             i;
        uchar           t, u, v;
        volatile uchar  x;      /* use when need to trigger via read */
        ulong           hga_cntl_val;  /* storage of current register values */
        volatile ulong  *hga_vram_addr;
        ulong           hga_vram_end;
        volatile uchar  *pData, *pIndex;
        struct fbdds    *dds;

        dds = (struct fbdds *) vp->display->odmdds;


        BUGLPR (dbg_reset, BUGNFO, ("Entering RESET\n"));

        ld->IO_base_address = PCI_BUS_ATT(&ddf->pci_io_map);

        /************* Load the Adapter Control Regs - START **************/

        BUGLPR (dbg_reset, BUGNTA, ("Starting reset\n"));

        PCI_PUTC(SETUP_VSE,0x1e);   /* Video Subsystem Enable Register   */
                                    /* Set bit 4 to place Video in Setup */
        PCI_PUTC(POS_MODE_OPT_SEL,0x01);/* Wake up the chip              */

        PCI_PUTS(ADVFUNC_CNTL,0);   /* Disable Graphics Engine Registers */
        PCI_PUTC(SETUP_VSE,0x0e);

        BUGLPR (dbg_reset, BUGNTA, ("Unlocking chip - 1\n"));

        S3_unlock(0);                   /* unlock the chip registers */

        PCI_PUTS(SUBSYS_CNTL,0X8000);   /* Reset Graphics Engine            */
        PCI_PUTS(SUBSYS_CNTL,0X4000);   /* Remove Reset Pulse               */
                                        /* (all interrupts off)             */

        PUT_CRTC_REG( SYS_CNFG, 0);

	if( ddf->VendorID = HGA_PCI_ID) /* Check for Pony(Polo) version */
        {                               
            GET_CRTC_REG(EX_DAC_CT, u); /* Disable DAC Reads -use gpio port */
            PCI_PUTC(CRTC_DATA,u | ENB_GIR);
            if (0 != (PCI_GETC(DAC_GENERAL_RD) & 0x10)) /* If bit 4 == 1, then */
                ddf->dev_type = PONY;       /* it's a Pony                     */
	    else
        	ddf->dev_type = HOMESTAKE;  /* it's a Homestake                */
            PCI_PUTC(CRTC_DATA,u);          /* Restore to normal access        */
	}

        /* Disable video */

        BUGLPR (dbg_reset, BUGNFO, ("Disable video\n"));

        x = PCI_GETC(INPUT_STATUS1);/* Reset attribute address flip flop     */
        PCI_PUTC(ATTR_INDX,   0);   /* Disable video palette (turns off vid) */

        BUGLPR (dbg_reset, BUGNTA, ("Doing reset\n"));

        /* reset the adapter */
        PUT_SEQ_REG(CLK_MODE, 0x20);/* Set bit 5 of index 01 to turn the    */
                                    /* screen off                           */

        PUT_SEQ_REG(RST_SYNC, 0x00);/* Clear bit 0 & 1 of index 0 to        */
                                    /* generate a reset                     */
        PUT_SEQ_REG(RST_SYNC, 0x03);/* Set bit 0 & 1 of to release reset    */

        GET_SEQ_REG(CLK_MODE, t);   /* Index 01                             */
        t &= ~0x20;
        PUT_SEQ_REG(CLK_MODE, t);   /* Clear bit 5 to turn the screen on    */

        PUT_SEQ_REG(MEM_MODE, 0x02);/* Set index 04h and set bit 1 to allow  */
                                    /* access to all of 256k of VGA memory   */

        t = PCI_GETC(MISC_OUTPUT_R);    /* Read misc output register         */
        t |= 0x0d;                      /* Set for Extended Clocking, set in */
        PCI_PUTC(MISC_OUTPUT_W,t);      /* extended register 42h; color mode */

        BUGLPR (dbg_reset, BUGGID, ("Unlocking chip - 2\n"));

        S3_unlock(0);                  /* unlock the chip registers */

        BUGLPR (dbg_reset, BUGGID, ("Initializing crt regs - initial\n"));

        pData = initial_ext_regs0;
        for ( i = 0; i < (sizeof(initial_ext_regs0)/2); i++)
        {
            /* Initialize some extended registers*/
            PUT_CRTC_REG( *pData, *(pData+1));
            pData +=2;
        }

        /* Pick up the desired address and set it in Linear Address regs */

        t = (ddf->base_address >> 24) & 0x3f; /* get High part; reset high bits */
        PUT_CRTC_REG(LAW_POS_H, t);/* Index 59h - Linear Address Pos High */

        u = (ddf->base_address >> 16) & 0xff; /* get Low part */
        PUT_CRTC_REG(LAW_POS_L, u);/* Index 5Ah - Linear Address Pos Low  */

	/* Check how much memory is installed */
        ld->bus_base_addr = PCI_BUS_ATT(&ddf->pci_mem_map);
        hga_vram_addr  = ld->bus_base_addr + HGA_RAM_1MEG;
        *hga_vram_addr = PATTERN1;
        *(hga_vram_addr +1) = PATTERN2;
	EIEIO;
	
	if( *hga_vram_addr == PATTERN1 && *(hga_vram_addr +1) == PATTERN2 )
	{
        	ddf->pci_mem_map.size = HGA_RAM_2MEG;
        	ddf->fb_size = HGA_RAM_2MEG;
                                 /* Index 58h, Linear Address window control */
        	PUT_CRTC_REG(LAW_CTL, ENB_LA | LAW_SIZE_2M);/* Define 2MB */
	}
	else
	{
        	ddf->pci_mem_map.size = HGA_RAM_1MEG;
        	ddf->fb_size = HGA_RAM_1MEG;
                                 /* Index 58h, Linear Address window control */
        	PUT_CRTC_REG(LAW_CTL, ENB_LA | LAW_SIZE_1M);/* Define 1MB */
	}
	
        PCI_BUS_DET(ld->bus_base_addr);

        pData = initial_ext_regs1;
        for (i=0;i<(sizeof(initial_ext_regs1)/2);i++)
        {
            /* Initialize more extended registers*/
            PUT_CRTC_REG( *pData, *(pData+1));
            pData +=2;
        }

        BUGLPR (dbg_reset, BUGGID, ("Initializing sequencer\n"));

        /* Assuming we know what monitor we've got ...  */
        pData = ld->v_mode->Base_Mode_Table->seq;
        for (i=1;i<=4 ;i++ )
        {                               /* Load sequencer  */
            PUT_SEQ_REG((unsigned char) i, *pData++);
        }

        BUGLPR (dbg_reset, BUGGID, ("Initializing for monitor %d\n", ld->monitor));

        pData = &ld->v_mode->Base_Mode_Table->misc_output[ld->monitor];
        PCI_PUTC(MISC_OUTPUT_W, *pData);        /* Misc Output Regs */

        BUGLPR (dbg_reset, BUGGID, ("Starting sequencer\n"));

        PUT_SEQ_REG(RST_SYNC, 0x03);    /* Start Sequencer  */

        BUGLPR (dbg_reset, BUGGID, ("Initializing attributes\n"));

        x = PCI_GETC(INPUT_STATUS1);    /* Program Attributes - Start by     */
                                        /* clearing the address/data FF      */
        pData = ld->v_mode->Base_Mode_Table->attributes;
        for (i=0;i<=0x13 ;i++ )
        {
            PUT_ATTR_REG( (unsigned char) i, *pData++);
        } /* endfor */

        BUGLPR (dbg_reset, BUGGID, ("Programming Graphics controller\n"));

        for (i=0; i <= 0x08 ;i++ )
        {                               /* Program Graphics Controller       */
            PUT_GRF_REG( (unsigned char) i, *pData++);
        } /* endfor */

        PUT_SEQ_REG(MEM_MODE, ld->v_mode->Memory_Mode);

        /* Enable Direct BitMap Addressing */

        BUGLPR (dbg_reset, BUGGID, ("Enabling Direct BitMap Addressing\n"));

        PCI_PUTS(ADVFUNC_CNTL, ld->v_mode->Advanced_Function|LinAddr);

        BUGLPR (dbg_reset, BUGGID, ("Programming CRT controller\n"));

        pData = ld->v_mode->Extended_Mode_Table[ld->monitor]->crtc;
        for (i=0;i<=0x18 ;i++)
        {                              /* Program Crt Controller  */
            PUT_CRTC_REG( (unsigned char) i, *pData++);
        } /* endfor */

        BUGLPR (dbg_reset, BUGGID, ("Programming Extended CRT controller\n"));

        pData = ld->v_mode->Extended_Mode_Table[ld->monitor]->ecrtc;
        pIndex = ecrtc_addrs;
        while (*pIndex != 0)
        {                              /* Program Extended Crt Controller */
            PUT_CRTC_REG( *pIndex++, *pData++);
        } /* endfor */

        if(ddf->dev_type == PONY)       /* Set controls for Pony(Polo)     */
	{
	    GET_CRTC_REG(LAW_CTL,u)
	    PCI_PUTC(CRTC_DATA, SAM_256 | u); /* set 256 serial access cntl */
	} /* endif */

        PCI_PUTC(FEATURE_CONTROL_W,0x04);/* Feature Control - Vertical Sync Ctl*/

        /*************************************/
        /* Now initialize the RAMDAC         */
        /*************************************/

        BUGLPR (dbg_reset, BUGGID, ("Programming RAMDAC\n"));

        if( ddf->VendorID == HGA_PCI_ID )
        {
            bt485_write( ld, BT485_COMMAND_0,  BT485_REG_0_DATA);
            bt485_write( ld, BT485_COMMAND_1,  BT485_REG_1_DATA);
            x = bt485_read( ld, BT485_COMMAND_2);
            bt485_write( ld, BT485_COMMAND_2, (x & ~(Bit1 | Bit0)));/* disable cursor */
            PCI_PUTC( DAC_WRITE_INDEX, BT485_ENB_CR3); /* Select CMD Reg 3 */
            bt485_write( ld, BT485_COMMAND_3,  BT485_REG_3_DATA);
        }
        else
        {
            BUGLPR (dbg_reset, BUGGID, ("Unknown adapter: 0x%8.8x\n",ddf->VendorID));

            PCI_BUS_DET(ld->IO_base_address);

            return(-1);          /* What have we here ? */
        }

        PCI_BUS_DET(ld->IO_base_address);

	/* load the palette */
	load_palette (ld);

        ld->bus_base_addr = PCI_BUS_ATT(&ddf->pci_mem_map);

        BUGLPR (dbg_reset, BUGGID,
            ("\tadapter memory real address = 0x%8.8x\n", ddf->base_address ));
        BUGLPR (dbg_reset, BUGGID,
            ("\taccessing adapter memory via  0x%8.8x\n", ld->bus_base_addr ));
        /*
         * Set all of video memory to 0's.  This will force the default
         * color 0 in the palette ram to be displayed across the entire
         * screen
         */

        hga_vram_addr          = ld->bus_base_addr;
        hga_vram_end           = ld->bus_base_addr + ddf->fb_size -1;

        BUGLPR (dbg_reset, BUGNFO, ("Clearing VRAM to 0x%8.8x.\n",TEST_PATTERN));
        BUGLPR (dbg_reset, BUGGID, ("VRAM_BASE = 0x%x\n", hga_vram_addr));

        for ( ; hga_vram_addr < hga_vram_end ; hga_vram_addr++ )
        {
                *hga_vram_addr = TEST_PATTERN;
        }
        BUGLPR (dbg_reset, BUGGID, ("VRAM_END addr = 0x%x\n", hga_vram_end));

        PCI_BUS_DET(ld->bus_base_addr); /* detach bus memory */

        /* Enable video */
        BUGLPR (dbg_reset, BUGGID, ("Enable video\n"));

        ld->IO_base_address = PCI_BUS_ATT(&ddf->pci_io_map);

        x = PCI_GETC(INPUT_STATUS1);     /* Re-enable video */
        PCI_PUTC(ATTR_INDX, ENB_PLT);

        BUGLPR (dbg_reset, BUGNFO, ("Leaving RESET\n"));

        PCI_BUS_DET(ld->IO_base_address);

        return (0);
}
/*--------------------------------------------------------------------------- */
/*      bt485_write()                                                         */
/*                                                                            */
/*      Write routine for the BT485 dac                                       */
/*                                                                            */
/*      BT485 regs need 4 bits to address them.  The 928 splits them up into  */
/*      two groups of two (RS[3:2] and RS[1:0]).                              */
/*--------------------------------------------------------------------------- */
void bt485_write(ld, index, data)
struct hga_data *ld;
int index;
unsigned char data;
{
        unsigned char   v, x;
        unsigned short  port;

        port = index & 0x0fff;          /* isolate port address            */
        x    = index >> 24;             /* get desired RS[1:0] bits        */
        GET_CRTC_REG(EX_DAC_CT, v);     /* select Extended DAC control reg */
        v &= ~0x03;                     /* clear RS bits                   */
        PCI_PUTC(CRTC_DATA, (v | x));   /* Set RS[3:2] to the proper index */
        PCI_PUTC(port, data);           /* Output the data requested       */
        PCI_PUTC(CRTC_DATA, v);         /* Restore RS bits to 0            */
        return;
}

/*---------------------------------------------------------------------------*/
/*      bt485_read()                                                         */
/*                                                                           */
/*      Read routine for the BT485 dac                                       */
/*                                                                           */
/*      BT485 regs need 4 bits to address them.  The 928 splits them up into */
/*      two groups of two (RS[3:2] and RS[1:0]).                             */
/*---------------------------------------------------------------------------*/
unsigned char bt485_read(ld, index)
struct hga_data *ld;
int index;
{
unsigned char   v, x;
unsigned short  port;

        port = index & 0x0fff;          /* isolate port address            */
        x    = index >> 24;             /* get desired RS[1:0] bits        */
        GET_CRTC_REG(EX_DAC_CT, v);     /* select Extended DAC control reg */
        v &= ~0x03;                     /* clear RS bits                   */
        PCI_PUTC(CRTC_DATA, (v | x));   /* Set RS[3:2] to the proper index */
        x = PCI_GETC( port );           /* Get the data requested          */
        PCI_PUTC( CRTC_DATA, v);        /* Restore RS Select to normal     */
        return x;
}

