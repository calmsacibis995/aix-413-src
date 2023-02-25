/* @(#)96  1.2  src/rspc/kernext/disp/pciwfg/inc/wfg_regval.h, pciwfg, rspc412, rspc.9448A 11/29/94 05:41:56 */
/* wfg_regval.h */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
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

/*
* COMPONENT NAME:    WD90C24A2 PCI Device Driver include file
*
* FUNCTIONS:         KSR device driver include file
*
* ORGINS:            27
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when
* combined with aggregated modules for this product)
*                  MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1991
* All Rights Reserved
*
* US Government Users  Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/


/****START OF SPECIFICATIONS ************************************************
 * DESCRIPTIVE NAME:  Western Digital PCI Graphics Adapter Register Values  *
 *                                                                          *
 * FUNCTION:  Define register offsets and values                            *
 *                                                                          *
 * ATTRIBUTES:                                                              *
 * REFERENCES:  WD90C24A2 Graphics Adaptor Specification                    *
 ****************************************************************************
 *                                                                          *
 ****END   OF SPECIFICATIONS ***********************************************/

#define Bit7    0x80
#define Bit6    0x40
#define Bit5    0x20
#define Bit4    0x10
#define Bit3    0x08
#define Bit2    0x04
#define Bit1    0x02
#define Bit0    0x01

/* General defines for the Western Digital Adapter */

#define WFG_PCI_ID      0x1c104ac2          /* WD90C24A2 PCI Vendor/Dev ID  */
#define ENTRY   	0
#define ADVANCED	1

#define PATTERN1	0x5f5f5f5f	  /* memory test pattern */
#define PATTERN2	0xa0a0a0a0	  /* memory test pattern */
#define WFG_MAX_SIZE    0x00100000        /* Largest memory to attach */
#define WFG_RAM_1MEG    0x00100000        /* 1 Meg frame buffer */
#define IO_SIZE         0x00010000        /* 64K IO space max */

/* General defines for the Western Digital 90C24A2 chip */

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
#define Western_Digital_VSY_INT_SET  Bit0
#define Western_Digital_GE_BSY_SET   Bit1
#define Western_Digital_FIFO_OVF_SET Bit2
#define Western_Digital_FIFO_EMP_SET Bit3
#define Western_Digital_VALID_INTERRUPTS 0x0f /* all of the above */
#define Western_Digital_INTR_ENABLE_DATA 0x00 /* disable interrupts */

#define CURSOR_X                  64    /* max pixels in x direction       */
#define CURSOR_Y                  64    /* max pixels in y direction       */
#define BYTES_CURSOR_ROW      (CURSOR_X / 8)
#define LAST_CURSOR_BYTE      (BYTES_CURSOR_ROW * CURSOR_Y)


/*                                                                           */
/* Regular Paradise Registers    (0x3CE/0x3CF)                               */
/*                                                                           */
#define  PR0A        0x09        /* Address Offset A                         */
#define  PR0B        0x0a        /* Address Offset B                         */
#define  PR1         0x0b        /* Memory Size                              */
#define  PR2         0x0c        /* Video Select                             */
#define  PR3         0x0d        /* CRT Lock Control                         */
#define  PR4         0x0e        /* Video Control                            */
#define  PR5         0x0f        /* Unlock PR0-PR4                           */
#define  PR5_LOCK    0x00        /*   protect PR0-PR4                        */
#define  PR5_UNLOCK  0x05        /*   unprotect PR0-PR4                      */
/*                                                                           */
/* Regular Paradise Registers    (0x3?4/0x3?5)                               */
/*                                                                           */
#define  PR10        0x29        /* Unlock PR11-PR17 & Device ID registers   */
#define  PR10_LOCK   0x00        /*   protect PR11-PR17                      */
#define  PR10_UNLOCK 0x85        /*   unprotect PR11-PR17                    */
#define  PR11        0x2a        /* EGA Switches                             */
#define  PR11_LOCK   0x95        /*   protect Misc. Output & Clocking Mode   */
#define  PR11_UNLOCK 0x90        /*   unprotect Misc. Output & Clocking Mode */
#define  PR12        0x2b        /* Scratch Pad                              */
#define  PR13        0x2c        /* Interlace H/2 Start                      */
#define  PR14        0x2d        /* Interlace H/2 End                        */
#define  PR15        0x2e        /* Miscellaneous Control 1                  */
#define  PR16        0x2f        /* Miscellaneous Control 2                  */
#define  PR17        0x30        /* Miscellaneous Control 3                  */
#define  PR18A       0x3d        /* CRTC Vertical Timing Overflow            */
/*                                                                           */
/* Paradise Extended Registers   (0x3C4/0x3C5)                               */
/*                                                                           */
#define  PR20        0x06        /* Unlock Paradise Extended Registers       */
#define  PR20_LOCK   0x00        /*   protect PR21-PR73                      */
#define  PR20_UNLOCK 0x48        /*   unprotect PR21-PR73                    */
#define  PR21        0x07        /* Display Configuraiton Status             */
#define  PR22        0x08        /* Scratch Pad                              */
#define  PR23        0x09        /* Scratch Pad                              */
#define  PR30A       0x10        /* Write Buffer & FIFO Control              */
#define  PR31        0x11        /* System Interface Contro                  */
#define  PR32        0x12        /* Miscellaneous Control 4                  */
#define  PR33A       0x13        /* DRAM Timing & 0 Wait State Control       */
#define  PR34A       0x14        /* Display Memory Mapping                   */
#define  PR35A       0x15        /* FPUSR0, FPUSR1 Output Select             */
#define  PR45        0x16        /* Video Signal Analyzer Control            */
#define  PR45A       0x17        /* Signal Analyzer Data I                   */
#define  PR45B       0x18        /* Signal Analyzer Data II                  */
#define  PR57        0x19        /* Feature Register I                       */
#define  PR58        0x20        /* Feature Register II                      */
#define  PR59        0x21        /* Memory Arbitration Cycle Setup           */
#define  PR62        0x24        /* FR Timing                                */
#define  PR63        0x25        /* Read/Write FIFO Control                  */
#define  PR58A       0x26        /* Memory Map to I/O Register for BitBlt    */
#define  PR64        0x27        /* CRT Lock Control II                      */
#define  PR65        0x28        /* reserved                                 */
#define  PR66        0x29        /* Feature Register III                     */
#define  PR68        0x31        /* Programmable Clock Selection             */
#define  PR69        0x32        /* Programmable VCLK Frequency              */
#define  PR70        0x33        /* Mixed Voltage Override                   */
#define  PR71        0x34        /* Programmable Refresh Timing              */
#define  PR72        0x35        /* Unlock PR68                              */
#define  PR72_LOCK   0x00        /*   protect PR68                           */
#define  PR72_UNLOCK 0x50        /*   unprotect PR68                         */
#define  PR73        0x36        /* VGA Status Detect                        */
/*                                                                           */
/* Flat Panel Paradise Registers (0x3?4/0x3?5)                               */
/*                                                                           */
#define  PR18        0x31        /* Flat Panel Status                        */
#define  PR19        0x32        /* Flat Panel Control I                     */
#define  PR1A        0x33        /* Flat Panel Control II                    */
#define  PR1B        0x34        /* Unlock Flat Panel Registers              */
#define  PR1B_LOCK          0x00 /*   protect PR18-PR44 & shadow registers   */
#define  PR1B_UNLOCK_SHADOW 0x06 /*   unprotect shadow CRTC registers        */
#define  PR1B_UNLOCK_PR     0xa0 /*   unprotect PR18-PR44                    */
#define  PR1B_UNLOCK        (PR1B_UNLOCK_SHADOW | PR1B_UNLOCK_PR)
#define  PR36        0x3b        /* Flat Panel Height Select                 */
#define  PR37        0x3c        /* Flat Panel Blinking Control              */
#define  PR39        0x3e        /* Color LCD Control                        */
#define  PR41        0x37        /* Vertical Expansion Initial Value         */
#define  PR44        0x3f        /* Powerdown & Memory Refresh Control       */
/*                                                                           */
/* Mapping RAM Registers         (0x3?4/0x3?5)                               */
/*                                                                           */
#define  PR30        0x35        /* Unlock Mapping RAM                       */
#define  PR30_LOCK   0x00        /*   protect PR33-PR35                      */
#define  PR30_UNLOCK 0x30        /*   unprotect PR33-PR35                    */
#define  PR33        0x38        /* Mapping RAM Address Counter              */
#define  PR34        0x39        /* Mapping RAM Data                         */
#define  PR35        0x3a        /* Mapping RAM & Powerdown Control          */
/*                                                                           */
/* Extended Paradise Registers ... BitBlt, H/W Cursor, and Line Drawing      */
/*                                                                           */
#define  EPR_INDX    0x23c0      /* Index Control                            */
#define  EPR_DATA    0x23c2      /* Register Access Port                     */
#define  EPR_BitBlt  0x23c4      /* BitBlt I/O Port                          */
/*                                                                           */
/* Local Bus Registers                                                       */
/*                                                                           */

#define     mclk_stn_color                  0x05     /* MCLK 44.3MHz */

/*-----------------------------------------------------------------------*/
/*       WD90C24A2  PR register   < Initital Value >                     */
/*-----------------------------------------------------------------------*/
/*                                   CRT  TFT  Sim  STN  Sim  STNC STNC */
/*                               all only only   32 only   16 sim  only */
/*                              ---- ---- ---- ---- ---- ---- ---- ---- */
#define     PR0A_ALL            0x00                                    
#define     PR0B_ALL            0x00                                   
#define     PR1_ALL             0xc5                                  

#define     PR2_CRT                  0x00                            
#define     PR2_TFT                       0x01                      
#define     PR2_S32                            0x01                
#define     PR2_STN                                 0x01          
#define     PR2_S16                                      0x01    
#define     PR2_STNC                                          0x01 

#define     PR3_ALL             0x00                              
#define     PR4_ALL             0x40                            
#define     PR4_CRT                  0x61

#define     PR12_ALL            0x00                           
#define     PR12_244LP          0xe8                          

#define     PR13_ALL            0x00                         
#define     PR14_ALL            0x00                        
#define     PR15_ALL            0x00                       
#define     PR16_ALL            0x42                      

#define     PR17_ALL            0x00                     
#define     PR17_244LP          0x40                    

#define     PR18_CRT_TFT             0x43                 /* single panel */
#define     PR18_CRT_STN             0x00                 /* dual panel */
#define     PR18_TFT                      0xc7            /* old d7h */
#define     PR18_S32                           0x47       /* old 57h */
#define     PR18_STN                                0x80              
#define     PR18_S16                                     0x00        
#define     PR18_STNC                                         0x00  

#define     PR19_DISABLE        0x40                               
#define     PR19_CRT                 0x64                         
#define     PR19_TFT                      0x54                   
#define     PR19_S32                           0x74             
#define     PR19_STN                                0x54       
#define     PR19_S16                                     0x74 
#define     PR19_STNC                                         0x74 
#define     PR19_STNC_ONLY                                         0x54 

#define     PR39_CRT                 0x04                              
#define     PR39_TFT                      0x20                        
#define     PR39_S32                           0x24                  
#define     PR39_STN                                0x00            
#define     PR39_S16                                     0x04      
#define     PR39_STNC                                         0x24

#define     PR1A_ALL            0x00                          /* except STNC*/
#define     PR1A_STNC                                         0x60   /* STNC*/

#define     PR36_ALL            0xef                                   

#define     PR37_CRT                 0x9a                             
#define     PR37_TFT                      0x9a                       
#define     PR37_S32                           0x9a                 
#define     PR37_STN                                0x9a           
#define     PR37_S16                                     0x1a     
#define     PR37_STNC                                         0x9a 

#define     PR18A_ALL           0x00                              
#define     PR41_ALL            0x00                             
#define     PR44_ALL            0x00                            
#define     PR33_ALL            0x00                           
#define     PR34_ALL            0x00                          

#define     PR35_ALL            0x22                            /* old 0a2h */
#define     PR35_SUSPEND        0xa2                             

#define     PR21_ALL            0x00                            
#define     PR22_ALL            0x00                           
#define     PR23_ALL            0x00                          

#define     PR30A_CRT                0xc1                    
#define     PR30A_TFT                     0xc1              
#define     PR30A_S32                          0xc1        
#define     PR30A_STN                               0xc1  
#define     PR30A_S16                                    0xe1  
#define     PR30A_STNC                                        0xe1 

#define     PR31_ALL            0x25                              
#define     PR32_ALL            0x00                             

#define     PR33A_ALL           0x80                            
#define     PR33A_STNC          0x83                           

#define     PR34A_ALL           0x00                          
#define     PR35A_ALL           0x00                         

#define     PR45_ALL            0x00                        
#define     PR45A_ALL           0x00                       
#define     PR45B_ALL           0x00                      
#define     PR57_ALL            0x31                     
#define     PR58_ALL            0x00                    
#define     PR58A_ALL           0x00                   

#define     PR59_ALL_SIVa       0x35                  
#define     PR59_CRT                 0x15                      /* for SIV-B */
#define     PR59_TFT                      0x15                       
#define     PR59_S32                           0x15                 
#define     PR59_STN                                0x35           
#define     PR59_S16                                     0x35     
#define     PR59_STNC                                         0x03  

#define     PR60_ALL            0x00                               
#define     PR61_ALL            0x00                              
#define     PR62_ALL            0x3c                             
#define     PR63_ALL            0x00                            

#define     PR64_ALL            0x03                       /*enhncd v-exp */

#define     PR65_ALL            0x00                                   

#define     PR66_CRT                 0x40                             
#define     PR66_TFT                      0x40                       
#define     PR66_S32                           0x40                 
#define     PR66_STN                                0x40           
#define     PR66_S16                                     0x40     
#define     PR66_STNC                                         0x40  

#define     PR68_CRT                 0x0d                          
#define     PR68_TFT                      0x0d                    
#define     PR68_S32                           0x0d          /*  old 0bh */
#define     PR68_STN                                0x1d      
#define     PR68_S16                                     0x0d
#define     PR68_STNC                                         0x0d      
#define     PR68_STNC_ONLY                                         0x05

#define     PR69_ALL            0x00                                  
#define     PR69_STNC_ONLY                                         0x4c 

#define     PR70_ALL            0x24                                   
#define     PR71_ALL            0x00                                  
#define     PR73_ALL            0x01                                 

/*--------------------------------------------------------------------------*/
/*       WD90C24A2  Shadow Register   < Initial Value >                     */
/*--------------------------------------------------------------------------*/
/* For TFT color Only Mode */
#define     CRTC00_TFT                    0x5f 
#define     CRTC02_TFT                    0x50 
#define     CRTC03_TFT                    0x82 
#define     CRTC04_TFT                    0x54 
#define     CRTC05_TFT                    0x80 
#define     CRTC06_TFT                    0x0b 
#define     CRTC07_TFT                    0x3e 
#define     CRTC10_TFT                    0xea 
#define     CRTC11_TFT                    0x8c 
#define     CRTC15_TFT                    0xe7 
#define     CRTC16_TFT                    0x04 

/* For TFT color Simultaneos Mode */
#define     CRTC00_S32                         0x5f 
#define     CRTC02_S32                         0x50 
#define     CRTC03_S32                         0x82
#define     CRTC04_S32                         0x54
#define     CRTC05_S32                         0x80
#define     CRTC06_S32                         0x0b
#define     CRTC07_S32                         0x3e
#define     CRTC10_S32                         0xea
#define     CRTC11_S32                         0x8c
#define     CRTC15_S32                         0xe7
#define     CRTC16_S32                         0x04

/* For STN Color Simultaneos Mode */
#define     CRTC00_STNC                                       0x60
#define     CRTC02_STNC                                       0x50
#define     CRTC03_STNC                                       0x83
#define     CRTC04_STNC                                       0x55
#define     CRTC05_STNC                                       0x81
#define     CRTC06_STNC                                       0x0e
#define     CRTC07_STNC                                       0x3e
#define     CRTC10_STNC                                       0xea
#define     CRTC11_STNC                                       0x8e
#define     CRTC15_STNC                                       0xe7
#define     CRTC16_STNC                                       0x04

/* For STN Color Simultaneos Mode */
#define     CRTC00_STNC_ISO                                   0x67
#define     CRTC02_STNC_ISO                                   0x50
#define     CRTC03_STNC_ISO                                   0x8a
#define     CRTC04_STNC_ISO                                   0x57
#define     CRTC05_STNC_ISO                                   0x88

/* For STN Color LCD only Mode */
#define     CRTC00_STNC_ONLY                                       0x67 
#define     CRTC02_STNC_ONLY                                       0x50 
#define     CRTC03_STNC_ONLY                                       0x82 
#define     CRTC04_STNC_ONLY                                       0x55 
#define     CRTC05_STNC_ONLY                                       0x81 
#define     CRTC06_STNC_ONLY                                       0xe6 
#define     CRTC07_STNC_ONLY                                       0x1f 
#define     CRTC10_STNC_ONLY                                       0xe0 
#define     CRTC11_STNC_ONLY                                       0x82 
#define     CRTC15_STNC_ONLY                                       0xe0 
#define     CRTC16_STNC_ONLY                                       0xe2 
