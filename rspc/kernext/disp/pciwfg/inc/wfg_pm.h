/* @(#)34  1.2 src/rspc/kernext/disp/pciwfg/inc/wfg_pm.h, pciwfg, rspc41J, 9521A_all 5/22/95 06:41:56 */
/* wfg_pm.h */
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
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */





#define POWER_ON        1
#define POWER_OFF       0
#define	CRT_POWER_ON	3
#define	CRT_POWER_OFF	2
#define	LCD_POWER_ON	5
#define	LCD_POWER_OFF	4

/* ----------------------------------------------------------- *
 *      PM structure for saving register data                  *
 * ----------------------------------------------------------- */

typedef struct _PM_SAVING_DATA
{
    uint   reg_type;
    uchar  reg_index;
    uchar  reg_data;
} PM_SAVING_DATA;

#define PR_SEQ   0x10
#define PR_GRF   0x20
#define PR_CRTC  0x30

#define SEQ_ULK  0x11
#define SEQ_LK   0x12

#define GRF_ULK  0x21
#define GRF_LK   0x22

#define ULK10    0x31
#define LK10     0x32
#define ULK1B    0x33
#define LK1B     0x34
#define ULK30    0x35
#define LK30     0x36

static PM_SAVING_DATA pr_savedata[] = {

  /********* PR20 unlock lock    ********************************************/
  { SEQ_ULK, PR20,  PR20_UNLOCK}, /* PR21_UNLOCK                            */
  { PR_SEQ,  PR21,  0x09       }, /* PR21  Disp Config & Scratch Pad        */
  { PR_SEQ,  PR22,  0x00       }, /* PR22  Scratch Pad                      */
  { PR_SEQ,  PR23,  0x00       }, /* PR23  Scratch Pad                      */
  { PR_SEQ,  PR45,  0x00       }, /* PR45  Video Signature Analyzer Control */
  { PR_SEQ,  PR45A, 0x00       }, /* PR45A Signature Analyzer Data 1        */
  { PR_SEQ,  PR45B, 0x00       }, /* PR45B Signature Analyzer Data 2        */
  { PR_SEQ,  PR58A, 0x00       }, /* PR58A Memory Map to I/O register       */
  { PR_SEQ,  PR63,  0x00       }, /* PR63  Read/Write Frame Buffer Control  */
  { PR_SEQ,  PR65,  0x00       }, /* PR65  Reserved for Future need         */
  { PR_SEQ,  PR71,  0x00       }, /* PR71  Programmable Refresh Timing      */
  { PR_SEQ,  PR73,  0x02       }, /* PR73  VGA Status Detect                */
  { SEQ_LK,  PR20,  PR20_LOCK  }, /* PR21_LOCK                              */

  /********* PR5  unlock lock    ********************************************/
  { GRF_ULK, PR5,   PR5_UNLOCK }, /* PR5_UNLOCK                             */
  { PR_GRF,  PR0A,  0x00       }, /* PR0A  Address Offset A                 */
  { PR_GRF,  PR0B,  0x00       }, /* PR0B  Address Offset B                 */
  { GRF_LK,  PR5,   PR5_LOCK   }, /* PR5_LOCK                               */

  /********* PR10 unlock lock    ********************************************/
  { ULK10,   PR10,  PR10_UNLOCK}, /* PR10 Unlock                            */
  { PR_CRTC, PR12,  0x00       }, /* PR12  Scratch Pad                      */
  { PR_CRTC, PR13,  0xFF       }, /* PR13  Interlace H/2 Start              */
  { LK10,    PR10,  PR10_LOCK  }, /* PR10 Lock                              */

  /********* PR1B unlock lock    ********************************************/
  { ULK1B,   PR1B,  PR1B_UNLOCK}, /* PR1B Unlock                            */
  { PR_CRTC, PR41,  0x00       }, /* PR41  Vertical Expansion Initial Value */
  { LK1B,    PR1B,  PR1B_LOCK  }, /* PR1B Lock                              */

  /********* PR30 unlock lock    ********************************************/
  { ULK30,   PR30,  PR30_UNLOCK}, /* PR30 Unlock                            */
  { PR_CRTC, PR33,  0xFF       }, /* PR33  Mapping RAM Address Counter      */
  { PR_CRTC, PR34,  0x00       }, /* PR34  Mapping RAM Data                 */
  { LK30,    PR30,  PR30_LOCK  }  /* PR30 Lock                              */

};
