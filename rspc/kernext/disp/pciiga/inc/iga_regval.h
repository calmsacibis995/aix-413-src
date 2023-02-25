/* @(#)84	1.2  src/rspc/kernext/disp/pciiga/inc/iga_regval.h, pciiga, rspc411, 9436E411a 9/8/94 17:09:41 */
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: none
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


/****START OF SPECIFICATIONS *******************************************
 *    DESCRIPTIVE NAME:  S3 PCI Graphics Adapter Register Values       *
 *                                                                     *
 *    FUNCTION:  Define register offsets and values                    *
 *                                                                     *
 *    ATTRIBUTES:                                                      *
 *    REFERENCES:  Homestake Graphics Adaptor Specification            *
 ***********************************************************************
 *                                                                     *
 ****END   OF SPECIFICATIONS ******************************************/

#define Bit7    0x80
#define Bit6    0x40
#define Bit5    0x20
#define Bit4    0x10
#define Bit3    0x08
#define Bit2    0x04
#define Bit1    0x02
#define Bit0    0x01

/* General defines for the S3 Adapter */

#define HGA_PCI_ID       0x3353b088       /* S3 86C928 PCI Vendor/Dev ID  */
#define  HOMESTAKE	0
#define  PONY		1
#define IGA_PCI_ID       0x3353c088       /* S3    864 PCI Vendor/Dev ID  */
#define IGA_PCI_ID2      0x3353c188       /* S3    864 PCI Vendor/Dev ID  */
#define IGA_PCI_ID3      0x3353c288       /* S3    864 PCI Vendor/Dev ID  */
#define IGA_PCI_ID4      0x3353c388       /* S3    864 PCI Vendor/Dev ID  */

#define PATTERN1	0x5f5f5f5f	  /* memory test pattern */
#define PATTERN2	0xa0a0a0a0	  /* memory test pattern */
#define IGA_MAX_SIZE    0x00200000        /* Largest memory to attach */
#define IGA_RAM_2MEG    0x00200000        /* 2 Meg frame buffer */
#define IGA_RAM_1MEG    0x00100000        /* 1 Meg frame buffer */
#define IGA_RAM_SIZE    0x00100000        /* 1 Meg frame buffer */
#define IO_SIZE         0x00010000        /* 64K IO space max */

/* General defines for the S3 86C928 chip */

#define POS_MODE_OPT_SEL 0x0102

#define DAC_MASK        0x03c6
#define DAC_CTL         0x03c6
#define DAC_READ_INDEX  0x03c7
#define DAC_STATUS      0x03c7
#define DAC_CURSOR_DATA 0x03c7
#define DAC_WRITE_INDEX 0x03c8
#define DAC_GENERAL_RD  0x03c8
#define DAC_DATA        0x03c9

#define DAC_X_LOW       0x03c8
#define DAC_X_HIGH      0x03c9
#define DAC_Y_LOW       0x03c6
#define DAC_Y_HIGH      0x03c7

#define ATTR_INDX       0x03c0      /* Attribute Controller Index Register    */
#define     ENB_PLT         Bit5    /*    Controls access to the PLT_REGs     */

#define     PLT_REG_00      0x00    /*    Palette Register 00                 */
#define     PLT_REG_01      0x01    /*    Palette Register 01                 */
#define     PLT_REG_02      0x02    /*    Palette Register 02                 */
#define     PLT_REG_03      0x03    /*    Palette Register 03                 */
#define     PLT_REG_04      0x04    /*    Palette Register 04                 */
#define     PLT_REG_05      0x05    /*    Palette Register 05                 */
#define     PLT_REG_06      0x06    /*    Palette Register 06                 */
#define     PLT_REG_07      0x07    /*    Palette Register 07                 */
#define     PLT_REG_08      0x08    /*    Palette Register 08                 */
#define     PLT_REG_09      0x09    /*    Palette Register 09                 */
#define     PLT_REG_10      0x0a    /*    Palette Register 10                 */
#define     PLT_REG_11      0x0b    /*    Palette Register 11                 */
#define     PLT_REG_12      0x0c    /*    Palette Register 12                 */
#define     PLT_REG_13      0x0d    /*    Palette Register 13                 */
#define     PLT_REG_14      0x0e    /*    Palette Register 14                 */
#define     PLT_REG_15      0x0f    /*    Palette Register 15                 */
#define     ATR_MODE        0x10    /*    Attribute Mode Control              */
#define     BDR_CLR         0x11    /*    Border Color                        */
#define     DISP_PLN        0x12    /*    Color Plane Enable  		      */
#define     H_PX_PAN        0x13    /*    Horizontal Pixel Panning            */
#define     PX_PAD          0x14    /*    Pixel Padding                       */
#define ATTR_DATA       0x03c0

#define SEQ_INDX        0x03c4      /* Sequencer Index Register               */
#define     RST_SYNC        0x00    /*    Reset                               */
#define       ASY_RST         Bit0  /*      Asynchronous Reset                */
#define       SYN_RST         Bit1  /*      Synchronous Reset                 */
#define     CLK_MODE        0x01    /*    Clocking Mode                       */
#define     EN_WT_PL        0x02    /*    Enable Write Plane                  */
#define     CH_FONT_SL      0x03    /*    Character Font Select               */
#define     MEM_MODE        0x04    /*    Memory Mode Control                 */
#define     UNLK_EXT_SEQ    0x08    /*    Unlock Extended Sequencer Register  */
#define     EXT_SEQ         0x0d    /*    Extended Sequencer Register         */
#define SEQ_DATA        0x3c5

#define GRF_INDX        0x3ce       /* Graphics Controller Index Register     */
#define     SET_RST_DT      0x00    /*    SetReset                            */
#define     EN_S_R_DT       0x01    /*    Enable Set/Reset                    */
#define     COLOR_CMP       0x02    /*    Color Compare       		      */
#define     WT_ROP_RTC      0x03    /*    Raster Operation/Rotate Counter     */
#define     RD_PL_SL        0x04    /*    Read Plane Select                   */
#define     GRP_MODE        0x05    /*    Graphics Controller Mode            */
#define     MISC_GM         0x06    /*    Memory Map Mode Control             */
#define     CMP_DNTC        0x07    /*    Color Don't Care                    */
#define     BIT_MASK        0x08    /*    Bit Mask                            */
#define GRF_DATA        0x3cf

#define CRTC_INDX       0x3d4       /* CRT Controller Index Register          */
#define     H_TOTAL         0x00    /*    Horizontal Total                    */
#define     H_D_END         0x01    /*    Horizontal Display End              */
#define     S_H_BLNK        0x02    /*    Start Horizontal Blank              */
#define     E_H_BLNK        0x03    /*    End   Horizontal Blank              */
#define     S_H_SY_P        0x04    /*    Start Horizontal Sync Position      */
#define     E_H_SY_P        0x05    /*    End   Horizontal Sync Position      */
#define     V_TOTAL         0x06    /*    Vertical Total                      */
#define     OVFL_REG        0x07    /*    CRTC Overflow                       */
#define     P_R_SCAN        0x08    /*    Preset Row Scan                     */
#define     MAX_S_LN        0x09    /*    Maximum Scan Line                   */
#define     CSSL            0x0a    /*    Cusrsor Start Scan Line             */
#define     CESL            0x0b    /*    Cusrsor End   Scan Line             */
#define     STA_H           0x0c    /*    Start Address High                  */
#define     STA_L           0x0d    /*    Start Address Low                   */
#define     CLA_H           0x0e    /*    Cursor Location Address High        */
#define     CLA_L           0x0f    /*    Cursor Location Address Low         */
#define     VRS             0x10    /*    Vertical Retrace Start              */
#define     VRE             0x11    /*    Vertical Retrace End                */
#define     VDE             0x12    /*    Vertical Display End                */
#define     SCREEN_OFFSET   0x13    /*    Offset                              */
#define     ULL             0x14    /*    Underline Location                  */
#define     SVB             0x15    /*    Start Vertical Blank                */
#define     EVB             0x16    /*    End   Vertical Blank                */
#define     CRT_MD          0x17    /*    CRTC Mode Control                   */
#define     LCM             0x18    /*    Line Compare                        */
#define     GCCL            0x22    /*    CPU Latch Data                      */
#define     REG_LOCK1       0x38    /*    Register Lock 1                     */
#define     REG_LOCK2       0x39    /*    Register Lock 2                     */
#define     SYS_CNFG        0x40    /*    System Configuration                */
#define     EX_DAC_CT       0x55    /*    Extended DAC Control                */
#define       ENB_GIR         Bit2  /*      Enable General Input Port Read    */
#define     LAW_CTL         0x58    /*    Linear Address Window Control       */
#define       LAW_SIZE_64     0     /*      00 = 64 KBytes (Default)          */
#define       LAW_SIZE_1M     1     /*      01 = 1 MBytes                     */
#define       LAW_SIZE_2M     2     /*      10 = 2 MBytes                     */
#define       LAW_SIZE_4M     3     /*      11 = 4 MBytes                     */
#define       ENB_LA          Bit4  /*      Enable Linear Addressing          */
#define       LMT_WPE         Bit5  /*      Limit Entry Depth for Write Post  */
#define       SAM_256         Bit6  /*      Serial Access Mode 256 Words Cntl */
#define     LAW_POS_H       0x59    /*    Linear Address Window Position(H)   */
#define     LAW_POS_L       0x5a    /*    Linear Address Window Position(L)   */
#define CRTC_DATA       0x3d5

#define MISC_OUTPUT_R   0x3cc
#define MISC_OUTPUT_W   0x3c2

#define INPUT_STATUS0   0x3c2
#define INPUT_STATUS1   0x3da

#define FEATURE_CONTROL_W 0x3da
#define FEATURE_CONTROL_R 0x3ca

#define SUBSYS_STAT     0x42e8
#define SUBSYS_CNTL     0x42e8
#define SETUP_VSE       0x46e8
#define ADVFUNC_CNTL    0x4ae8
#define CUR_Y           0x82e8
#define CUR_X           0x86e8
#define DESTY_AXSTP     0x8ae8
#define DESTX_DIASTP    0x8ee8
#define ERR_TERM        0x92e8
#define MAJ_AXIS_PCNT   0x96e8
#define GP_STAT         0x9ae8
#define CMD             0x9ae8
#define SHORT_STROKE    0x9ee8
#define BKGD_COLOR      0xa2e8
#define FRGD_COLOR      0xa6e8
#define WRT_MASK        0xaae8
#define RD_MASK         0xaee8
#define COLOR_COMPARE   0xb2e8
#define BKGD_MIX        0xb6e8
#define FRGD_MIX        0xbae8
#define RD_REG_DT       0xbee8
#define MULTI_FUNC      0xbee8

#define PIX_TRANS       0xe2e8
#define PIX_TRANS_EXT   0xe2ea

/* Possible interrupt conditions */
#define S3_VSY_INT_SET  Bit0
#define S3_GE_BSY_SET   Bit1
#define S3_FIFO_OVF_SET Bit2
#define S3_FIFO_EMP_SET Bit3
#define S3_VALID_INTERRUPTS 0x0f /* all of the above */
#define S3_INTR_ENABLE_DATA 0x00 /* disable interrupts */

/* S3 SDAC setup values */

#define SDAC_REG_0_DATA      Bit7 | /* CR07 -- Cmd Reg 3 can be addressed */ \
                              Bit5 | /* CR05 -- Enable SETUP               */ \
                              Bit3 | /* CR03 -- Green Sync Enable          */ \
                              Bit1   /* CR01 -- 8-bit DAC operation        */
#define SDAC_REG_1_DATA      Bit6   /* CR16 -- Four 8-bit pixels          */
#define SDAC_REG_2_DATA      Bit5   /* CR25 -- PORTSEL -- Nonmasked       */
                                     /* CR21,20 = 00 --> cursor disabled   */
#define SDAC_REG_3_DATA      Bit2   /* CR32 -- 64 x 64 cursor             */

#define SDAC_PXL_READ_MSK_DATA 0xff
#define SDAC_ENB_CR3         Bit7   /* Enable access to CR 3              */

/*---------------------  S3 SDAC Register Offsets  ---------------------*/
                                        /* 3210 -- RS                      */
#define SDAC_RAMDAC_WR_ADDR 0x000003c8 /* 0000                            */
#define SDAC_DAC_PALETTE    0x000003c9 /* 0001                            */
#define SDAC_PIXEL_MASK     0x000003c6 /* 0010                            */
#define SDAC_RAMDAC_RD_ADDR 0x000003c7 /* 0011                            */
#define SDAC_COLOR_WR_ADDR  0x010003c8 /* 0100                            */
#define SDAC_COLOR_DATA     0x010003c9 /* 0101                            */
#define SDAC_COMMAND_0      0x010003c6 /* 0110                            */
#define SDAC_COLOR_RD_ADDR  0x010003c7 /* 0111                            */
#define SDAC_COMMAND_1      0x020003c8 /* 1000                            */
#define SDAC_COMMAND_2      0x020003c9 /* 1001                            */
#define SDAC_RAMDAC_STATUS  0x020003c6 /* 1010                            */
#define SDAC_COMMAND_3      0x020003c6 /* 1010                            */
#define SDAC_CUR_RAM_ARRAY  0x020003c7 /* 1011                            */
#define SDAC_CUR_X_LOW_REG  0x030003c8 /* 1100                            */
#define SDAC_CUR_X_HIGH_REG 0x030003c9 /* 1101                            */
#define SDAC_CUR_Y_LOW_REG  0x030003c6 /* 1110                            */
#define SDAC_CUR_Y_HIGH_REG 0x030003c7 /* 1111                            */

#define SDAC_CUR_COLOR_DATA (volatile uchar *)(ld->IO_base_address | DAC_DATA)
#define SDAC_CUR_ARRAY_DATA (volatile uchar *)(ld->IO_base_address | DAC_CURSOR_DATA)
#define SDAC_DAC_X_LOW      (volatile uchar *)(ld->IO_base_address | DAC_X_LOW)
#define SDAC_DAC_X_HIGH     (volatile uchar *)(ld->IO_base_address | DAC_X_HIGH)
#define SDAC_DAC_Y_LOW      (volatile uchar *)(ld->IO_base_address | DAC_Y_LOW)
#define SDAC_DAC_Y_HIGH     (volatile uchar *)(ld->IO_base_address | DAC_Y_HIGH)

#define CURSOR_X                  64    /* max pixels in x direction       */
#define CURSOR_Y                  64    /* max pixels in y direction       */
#define BYTES_CURSOR_ROW      (CURSOR_X / 8)
#define LAST_CURSOR_BYTE      (BYTES_CURSOR_ROW * CURSOR_Y)
