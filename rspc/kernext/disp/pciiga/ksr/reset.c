static char sccsid[] = "@(#)59	1.3  src/rspc/kernext/disp/pciiga/ksr/reset.c, pciiga, rspc41B, 9504A 12/15/94 16:46:48";
/*
 *
 * COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: iga_reset
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


#include "iga_INCLUDES.h"
#include "iga_funct.h"
#include <sys/m_param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include "IO_defs.h"

BUGXDEF (dbg_reset);
GS_MODULE(iga_reset);

void sdac_write();
void sdac_write2();
unsigned char sdac_read();

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

/*--------------------------------------------------------------------------*/
/*      Turn on and off H+V Sync                                            */
/*      Needed so won't burn out monitors                                   */
/*--------------------------------------------------------------------------*/
#define S3_Disable_Sync  { \
    PUT_SEQ_REG(UNLK_EXT_SEQ, 0x06); \
    PUT_SEQ_REG(EXT_SEQ,  0x50); \
}
#define S3_Enable_Sync   { \
	PUT_SEQ_REG(EXT_SEQ,  0x00); \
	PUT_SEQ_REG(UNLK_EXT_SEQ, 0x00); \
}

/*------------------------------------------------------------------*
 *               iga_reset()                                        *
 *------------------------------------------------------------------*/
long
iga_reset (vp, ld, ddf)
        struct vtmstruc *vp;
        struct iga_data *ld;
        struct iga_ddf *ddf;
{

        int             i;
        uchar           t, u, v;
        volatile uchar  x;      /* use when need to trigger via read */
        ulong           iga_cntl_val;  /* storage of current register values */
        volatile ulong  *iga_vram_addr;
        ulong           iga_vram_end;
        volatile uchar  *pData, *pIndex;
        struct fbdds    *dds;

	IGA_ENTER_TRC0(ddf,iga_reset,2,IGA_RESET);

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

	IGA_PARM_TRC0(ddf,iga_reset,2,IGA_RESET,IGA_UNLOCK_S3);

        S3_unlock(0);                   /* unlock the chip registers */


		/* Disable Horizontal and Vertical Sync */
		S3_Disable_Sync;


        PCI_PUTS(SUBSYS_CNTL,0X8000);   /* Reset Graphics Engine            */
        PCI_PUTS(SUBSYS_CNTL,0X4000);   /* Remove Reset Pulse               */
                                        /* (all interrupts off)             */
        PUT_CRTC_REG( SYS_CNFG, 0);

        if( ddf->VendorID == IGA_PCI_ID      /* Check for Pony(Polo) version */
          ||  ddf->VendorID == IGA_PCI_ID2
          ||  ddf->VendorID == IGA_PCI_ID3
          ||  ddf->VendorID == IGA_PCI_ID4)
        {                               
            GET_CRTC_REG(EX_DAC_CT, u); /* Disable DAC Reads -use gpio port */
            PCI_PUTC(CRTC_DATA,u | ENB_GIR);
            if (0 != (PCI_GETC(DAC_GENERAL_RD) & 0x10)) /* If bit 4 == 1, then */
                ddf->dev_type = PONY;       /* it's a Pony                     */
            else
                ddf->dev_type = HOMESTAKE;  /* it's a Homestake                */
            PCI_PUTC(CRTC_DATA,u);          /* Restore to normal access        */
        }

	IGA_PARM_TRC1(ddf,iga_reset,2,IGA_RESET,IGA_DEV_TYPE,ddf->dev_type);

        /* Disable video */

        BUGLPR (dbg_reset, BUGNFO, ("Disable video\n"));

        x = PCI_GETC(INPUT_STATUS1);/* Reset attribute address flip flop     */
        PCI_PUTC(ATTR_INDX,   0);   /* Disable video palette (turns off vid) */

        BUGLPR (dbg_reset, BUGNTA, ("Doing reset\n"));

        /* reset the adapter */
        PUT_SEQ_REG(CLK_MODE, 0x20);/* Set bit 5 of index 01 to turn the    */
                                    /* screen off                           */

#if 0 
/* Bits 0-1 of SR0 (Reset Sequencer Register) are not functional for the 864 */
        PUT_SEQ_REG(RST_SYNC, 0x00);/* Clear bit 0 & 1 of index 0 to        */
                                    /* generate a reset                     */
        PUT_SEQ_REG(RST_SYNC, 0x03);/* Set bit 0 & 1 of to release reset    */
#endif

        GET_SEQ_REG(CLK_MODE, t);   /* Index 01                             */
        t &= ~0x20;
        PUT_SEQ_REG(CLK_MODE, t);   /* Clear bit 5 to turn the screen on    */

        PUT_SEQ_REG(MEM_MODE, 0x02);/* Set index 04h and set bit 1 to allow  */
                                    /* access to all of 256k of VGA memory   */


        t = PCI_GETC(MISC_OUTPUT_R);    /* Read misc output register         */
        t |= 0x0d;                      /* Set for Extended Clocking, set in */
        PCI_PUTC(MISC_OUTPUT_W,t);      /* extended register 42h; color mode */

        BUGLPR (dbg_reset, BUGGID, ("Unlocking chip - 2\n"));

	IGA_PARM_TRC0(ddf,iga_reset,2,IGA_RESET,IGA_UNLOCK_S3);

        S3_unlock(0);                  /* unlock the chip registers */

        BUGLPR (dbg_reset, BUGGID, ("Initializing crt regs - initial\n"));

	IGA_PARM_TRC0(ddf,iga_reset,2,IGA_RESET,IGA_INIT_EXT_REGS_0);

        pData = initial_ext_regs0;
        for ( i = 0; i < (sizeof(initial_ext_regs0)/2); i++)
        {
            /* Initialize some extended registers*/
        BUGLPR (dbg_reset, BUGGID, ("Init some extended registers 0x%x - 0x%x \n", *pData, *(pData +1)));
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
        iga_vram_addr  = ld->bus_base_addr + IGA_RAM_1MEG;


        BUGLPR (dbg_reset, BUGNTA, ("ld->bus_base_addr = 0x%x\n",  ld->bus_base_addr));
        BUGLPR (dbg_reset, BUGNTA, ("iga_vram_addr = 0x%x\n", iga_vram_addr));
        *iga_vram_addr = PATTERN1;
        *(iga_vram_addr +1) = PATTERN2;
        EIEIO;
        
        if( *iga_vram_addr == PATTERN1 && *(iga_vram_addr +1) == PATTERN2 )
        {
                ddf->pci_mem_map.size = IGA_RAM_2MEG;
                ddf->fb_size = IGA_RAM_2MEG;
                                 /* Index 58h, Linear Address window control */
                PUT_CRTC_REG(LAW_CTL, ENB_LA | LAW_SIZE_2M);/* Define 2MB */
        }
        else
        {
                ddf->pci_mem_map.size = IGA_RAM_1MEG;
                ddf->fb_size = IGA_RAM_1MEG;
                                 /* Index 58h, Linear Address window control */
                PUT_CRTC_REG(LAW_CTL, ENB_LA | LAW_SIZE_1M);/* Define 1MB */
        }
        
	IGA_PARM_TRC1(ddf,iga_reset,2,IGA_RESET,IGA_FB_SIZE,ddf->fb_size);

        PCI_BUS_DET(ld->bus_base_addr);

	IGA_PARM_TRC0(ddf,iga_reset,2,IGA_RESET,IGA_INIT_EXT_REGS_1);

        pData = initial_ext_regs1;
        for (i=0;i<(sizeof(initial_ext_regs1)/2);i++)
        {
            /* Initialize more extended registers*/
        BUGLPR (dbg_reset, BUGGID, ("Extended registers 0x%x - 0x%x 0x%x\n", i, *pData, *(pData +1)));
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

	IGA_PARM_TRC0(ddf,iga_reset,2,IGA_RESET,IGA_INIT_ATTR);

        pData = ld->v_mode->Base_Mode_Table->attributes;
        for (i=0;i<=0x13 ;i++ )
        {
            PUT_ATTR_REG( (unsigned char) i, *pData++);
        } /* endfor */

        BUGLPR (dbg_reset, BUGGID, ("Programming Graphics controller\n"));

        for (i=0; i <= 0x08 ;i++ )
        {                               /* Program Graphics Controller       */
        BUGLPR (dbg_reset, BUGGID, ("Graphics controller 0x%x - 0x%x\n", i, *pData));
            PUT_GRF_REG( (unsigned char) i, *pData++);
        } /* endfor */

        PUT_SEQ_REG(MEM_MODE, ld->v_mode->Memory_Mode);

        /* Enable Direct BitMap Addressing */

        BUGLPR (dbg_reset, BUGGID, ("Enabling Direct BitMap Addressing\n"));

        PCI_PUTS(ADVFUNC_CNTL, ld->v_mode->Advanced_Function|LinAddr);

        BUGLPR (dbg_reset, BUGGID, ("Programming CRT controller\n"));

	IGA_PARM_TRC0(ddf,iga_reset,2,IGA_RESET,IGA_INIT_CRTC);

        pData = ld->v_mode->Extended_Mode_Table[ld->monitor]->crtc;
        for (i=0;i<=0x18 ;i++)
        {                              /* Program Crt Controller  */
        BUGLPR (dbg_reset, BUGGID, ("Crt controller 0x%x - 0x%x\n", i, *pData));
            PUT_CRTC_REG( (unsigned char) i, *pData++);
        } /* endfor */

        BUGLPR (dbg_reset, BUGGID, ("Programming Extended CRT controller\n"));

	IGA_PARM_TRC0(ddf,iga_reset,2,IGA_RESET,IGA_INIT_ECRTC);

        pData = ld->v_mode->Extended_Mode_Table[ld->monitor]->ecrtc;
        pIndex = ecrtc_addrs;
        while (*pIndex != 0)
        {                              /* Program Extended Crt Controller */
        BUGLPR (dbg_reset, BUGGID, ("Crt controller 0x%x - 0x%x\n", *pIndex, *pData));
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

        BUGLPR (dbg_reset, BUGNFO, ("Programming RAMDAC\n"));

        if( ddf->VendorID == IGA_PCI_ID         /* Check for Pony(Polo) version */
          ||  ddf->VendorID == IGA_PCI_ID2
          ||  ddf->VendorID == IGA_PCI_ID3
          ||  ddf->VendorID == IGA_PCI_ID4)
        {

	   IGA_PARM_TRC0(ddf,iga_reset,2,IGA_RESET,IGA_INIT_SDAC);

           PCI_PUTC(0x3c6, 0xff);

/* sdac_write( ld, SDAC_COMMAND_0,  SDAC_REG_0_DATA); */

        	pData = ld->v_mode->Extended_Mode_Table[ld->monitor]->DAC_Mode;
BUGLPR (dbg_reset, BUGNFO, ("ld->monitor = 0x%x\n", ld->monitor));
BUGLPR (dbg_reset, BUGNFO, ("ld->v_mode->mode_number = 0x%x\n", ld->v_mode->mode_number));
BUGLPR (dbg_reset, BUGNFO, ("*pData = 0x%x *pData+1 = 0x%x *pData+1 = 0x%x\n", *pData, *(pData+1), *(pData+2)));
            sdac_write( ld, SDAC_COMMAND_0,  *pData);

            for (i=2; i<8; i++)
            {
                sdac_write2(ld, i, *(pData + 1), *(pData + 2));
            }
            sdac_write2(ld, 0x0a, 0x41, 0x26);
            sdac_write2(ld, 0x0e, 0x22, 0x00);

#if 0
            sdac_write( ld, SDAC_COMMAND_1,  SDAC_REG_1_DATA);
            x = sdac_read( ld, SDAC_COMMAND_2);
            sdac_write( ld, SDAC_COMMAND_2, (x & ~(Bit1 | Bit0)));/* disable cursor */
            PCI_PUTC( DAC_WRITE_INDEX, SDAC_ENB_CR3); /* Select CMD Reg 3 */
            sdac_write( ld, SDAC_COMMAND_3,  SDAC_REG_3_DATA);
#else
#endif
        }
        else
        {
            BUGLPR (dbg_reset, BUGGID, ("Unknown adapter: 0x%8.8x\n",ddf->VendorID));

			S3_Enable_Sync;
            PCI_BUS_DET(ld->IO_base_address);

	    IGA_PARM_TRC0(ddf,iga_reset,2,IGA_RESET,IGA_UNKNOWN_DEVICE);

            return(-1);          /* What have we here ? */
        }

		/* Enable Horizontal and Vertical Sync */
		S3_Enable_Sync;

        PCI_BUS_DET(ld->IO_base_address);

        /* load the palette */
        load_palette (ld);

        ld->bus_base_addr = PCI_BUS_ATT(&ddf->pci_mem_map);

        BUGLPR (dbg_reset, BUGNFO, ("\tadapter memory real address = 0x%8.8x\n", ddf->base_address ));
        BUGLPR (dbg_reset, BUGNFO, ("\taccessing adapter memory via  0x%8.8x\n", ld->bus_base_addr ));
        /*
         * Set all of video memory to 0's.  This will force the default
         * color 0 in the palette ram to be displayed across the entire
         * screen
         */

        iga_vram_addr          = ld->bus_base_addr;
        iga_vram_end           = ld->bus_base_addr + ddf->fb_size -1;

        BUGLPR (dbg_reset, BUGNFO, ("Clearing VRAM to 0x%8.8x.\n",TEST_PATTERN));
        BUGLPR (dbg_reset, BUGGID, ("VRAM_BASE = 0x%x\n", iga_vram_addr));

	IGA_PARM_TRC0(ddf,iga_reset,2,IGA_RESET,IGA_CLEAR_VRAM);

        for ( ; iga_vram_addr < iga_vram_end ; iga_vram_addr++ )
        {
                *iga_vram_addr = TEST_PATTERN;
        }
        BUGLPR (dbg_reset, BUGGID, ("VRAM_END addr = 0x%x\n", iga_vram_end));

        PCI_BUS_DET(ld->bus_base_addr); /* detach bus memory */

        /* Enable video */
        BUGLPR (dbg_reset, BUGGID, ("Enable video\n"));

        ld->IO_base_address = PCI_BUS_ATT(&ddf->pci_io_map);

        x = PCI_GETC(INPUT_STATUS1);     /* Re-enable video */
        PCI_PUTC(ATTR_INDX, ENB_PLT);

        BUGLPR (dbg_reset, BUGNFO, ("Leaving RESET\n"));

        PCI_BUS_DET(ld->IO_base_address);

	IGA_EXIT_TRC0(ddf,iga_reset,2,IGA_RESET);

        return (0);
}
/*--------------------------------------------------------------------------- */
/*      sdac_write()                                                          */
/*                                                                            */
/*      Write routine for the S3 sdac                                         */
/*                                                                            */
/*      SDAC regs need 4 bits to address them.  The 928 splits them up into   */
/*      two groups of two (RS[3:2] and RS[1:0]).                              */
/*--------------------------------------------------------------------------- */
void sdac_write(struct iga_data *ld, int index, unsigned char data)
{
        unsigned char   u, x;
        unsigned short  port;

        GET_CRTC_REG(EX_DAC_CT, u);        /* select Extended DAC control reg */
        u &= ~0x03;                        /* clear RS bits                   */
        PCI_PUTC(CRTC_DATA, (u | 0x01));   /* Set RS[3:2] to the proper index */

        port = index & 0x0fff;          /* isolate port address            */
        x    = index >> 24;             /* get desired RS[1:0] bits        */
        PCI_PUTC(port, data);           /* Output the data requested       */

        PCI_PUTC(CRTC_DATA, u);         /* Restore RS bits to 0            */
        return;
}

/*--------------------------------------------------------------------------- */
/*      sdac_write2()                                                         */
/*                                                                            */
/*      Write routine for the S3 sdac                                         */
/*                                                                            */
/*      SDAC regs need 4 bits to address them.  The 928 splits them up into   */
/*      two groups of two (RS[3:2] and RS[1:0]).                              */
/*--------------------------------------------------------------------------- */
void sdac_write2(struct iga_data *ld, int index, unsigned char data1, unsigned char data2)
{
        unsigned char   u, x;
        unsigned short  port;

        GET_CRTC_REG(EX_DAC_CT, u);         /* select Extended DAC control reg */
        u &= ~0x03;                         /* clear RS bits                   */
        PCI_PUTC(CRTC_DATA, (u | 0x01));    /* Set RS[3:2] to the proper index */

        PCI_PUTC(DAC_WRITE_INDEX, index);
        PCI_PUTC(DAC_DATA, data1);                /* Output the data requested       */
        if (index < 0x0e)
        {
            PCI_PUTC(DAC_DATA, data2);
        }

        PCI_PUTC(CRTC_DATA, u);             /* Restore RS bits to 0            */
        return;
}


/*---------------------------------------------------------------------------*/
/*      sdac_read()                                                         */
/*                                                                           */
/*      Read routine for the S3 sdac                                         */
/*                                                                           */
/*      SDAC regs need 4 bits to address them.  The 928 splits them up into */
/*      two groups of two (RS[3:2] and RS[1:0]).                             */
/*---------------------------------------------------------------------------*/
unsigned char sdac_read(ld, index)
struct iga_data *ld;
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

