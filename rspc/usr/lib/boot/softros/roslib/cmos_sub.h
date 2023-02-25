/* @(#)23	1.1  src/rspc/usr/lib/boot/softros/roslib/cmos_sub.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:02  */
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
/*****************************************************************************/
/* FILE NAME:    cmos_sub.h                                                  */
/*               Public routine prototypes for cmos_sub.c                    */
/*****************************************************************************/

#ifndef _H_CMOS_SUB
#define _H_CMOS_SUB

#include <stdio.h>

/*---------------------------------------------------------------------------*/
/*               Common CMOS equate definitions....                          */

#define NMI_ON                      0x0
#define NMI_OFF                     0x80
#define NMI_ON_OR_OFF_MASK          0x80  /* No other bits matter */
#define CMOSSIZE                    64
#define CMOS_REG_SECONDS            0x00
#define CMOS_REG_MINUTES            0x02
#define CMOS_REG_HOURS              0x04
#define CMOS_REG_DAYOFWEEK          0x06
#define CMOS_REG_DAYOFMONTH         0x07
#define CMOS_REG_MONTH              0x08
#define CMOS_REG_YEAR               0x09
#define CMOS_REG_C                  0x0C
#define CMOS_REG_D                  0x0D

#define CMOS_DATA_START             0x10
#define CMOS_UNATTENDED             0x10
#define CMOS_CRC_START              0x10
#define CMOS_CRC_END                0x1D
#define CMOS_CRC_0                  0x1E
#define CMOS_CRC_1                  0x1F
#define CMOS_POP                    0x20
#define CMOS_PAP                    0x30
#define CMOS_REG_YEAR_HIGH          0x37
#define CMOS_DATA_END               0x3F
                                    
#define CMOS_DATA_SIZE              0x30
#define UNATTENDED_MASK             0x80

#define CMOS_UNATTENDED    0x10
#define CMOS_POP           0x20
#define CMOS_PAP           0x30

#define UNATTENDED_MASK    0x80

#endif  /* _H_CMOS_SUB */

/*---------------------------------------------------------------------------*/
/*               Public function prototypes....                              */


unsigned char CMOS_read_byte(unsigned int CMOS_addr);
void CMOS_write_byte(unsigned int CMOS_addr, unsigned char value);

unsigned char get_NMI(void);
void set_NMI(unsigned char);
void show_cmos();
int cmos_good();
int crcgen(unsigned int oldcrc, unsigned char data);
void cmosdump(FILE *file);
void cmosinit();
void cmosload(char *filename);
