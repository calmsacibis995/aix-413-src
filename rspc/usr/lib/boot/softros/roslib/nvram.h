/* @(#)41	1.1  src/rspc/usr/lib/boot/softros/roslib/nvram.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:33  */
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
//************************** OCO SOURCE MATERIALS *****************************
//******* IBM CONFIDENTIAL (IBM CONFIDENTIAL-RESTRICTED when combined *********
//************** with the aggregated modules for this product) ****************
//*****************************************************************************
// FILE NAME:          nvram.h
//
// COPYRIGHT:          (C) Copyright IBM Corp. 1993, 1993
//
// DESCRIPTION: This file contains public routine prototypes and equates for
//              the functions in nvram.c
//
// REVISION HISTORY:
// -----------------------Sandalfoot-----------------------------------------
// Date      Track#     Who  Description
// --------  ---------- ---  ------------------------------------------------
//
//*****************************************************************************

#ifndef _H_NVRAM
#define _H_NVRAM
#include <stdio.h>

#define NVRAM_DATA_START             0x000
#define NVRAM_DATA_END               0xFFF
#define NVRAM_CRC_START              0x000
#define NVRAM_CRC_END                0x1FD
#define NVRAM_CRC_0                  0x1FE
#define NVRAM_CRC_1                  0x1FF

#define NVRAM_DATA_SIZE              0x1000

/*----- 87312 Registers Usage -----*/
#define  NVRAM_87312_START          (NVRAM_DATA_START + 0)         // 87312 registers NVRAM starting address
#define  NVRAM_87312_FER            (NVRAM_87312_START + 0)        // FER
#define  NVRAM_87312_FAR            (NVRAM_87312_START + 1)        // FAR
#define  NVRAM_87312_PTR            (NVRAM_87312_START + 2)        // PTR
#define  NVRAM_87312_SIZE           (NVRAM_87312_PTR - NVRAM_87312_START + 1)  // 87312 registers set size

/*----- Serial Ports Usage -----*/
#define  NVRAM_SERIAL1_START        (NVRAM_DATA_START + NVRAM_87312_SIZE) // Serial port 1 NVRAM starting address
#define  NVRAM_SERIAL1_LCR          (NVRAM_SERIAL1_START + 0)      // Line Control Register
#define  NVRAM_SERIAL1_BAUDRATE_LO  (NVRAM_SERIAL1_START + 1)      // Baud Rate Low byte
#define  NVRAM_SERIAL1_BAUDRATE_HI  (NVRAM_SERIAL1_BAUDRATE_LO + 1)// Baud Rate High byte
#define  NVRAM_SERIAL_SIZE          (NVRAM_SERIAL1_BAUDRATE_HI - NVRAM_SERIAL1_START + 1) // Size of serial port regiters set in NVRAM

#define  NVRAM_SERIAL2_START        (NVRAM_SERIAL1_START + NVRAM_SERIAL_SIZE) // Serial port 2 NVRAM starting address
#define  NVRAM_SERIAL2_LCR          (NVRAM_SERIAL2_START + 0)      // Line Control Register
#define  NVRAM_SERIAL2_BAUDRATE_LO  (NVRAM_SERIAL2_START + 1)      // Baud Rate Low byte
#define  NVRAM_SERIAL2_BAUDRATE_HI  (NVRAM_SERIAL2_BAUDRATE_LO + 1) // Baud Rate High byte

#define  NVRAM_SERIAL3_START        (NVRAM_SERIAL1_START + (2 * NVRAM_SERIAL_SIZE)) // Serial port 3 NVRAM starting address
#define  NVRAM_SERIAL3_LCR          (NVRAM_SERIAL3_START + 0)       // Line Control Register
#define  NVRAM_SERIAL3_BAUDRATE_LO  (NVRAM_SERIAL3_START + 1)       // Baud Rate Low byte
#define  NVRAM_SERIAL3_BAUDRATE_HI  (NVRAM_SERIAL3_BAUDRATE_LO + 1) // Baud Rate High byte

#define  NVRAM_SERIAL4_START        (NVRAM_SERIAL1_START + (3 * NVRAM_SERIAL_SIZE)) // Serial port 4 NVRAM starting address
#define  NVRAM_SERIAL4_LCR          (NVRAM_SERIAL4_START + 0)       // Line Control Register
#define  NVRAM_SERIAL4_BAUDRATE_LO  (NVRAM_SERIAL4_START + 1)       // Baud Rate Low byte
#define  NVRAM_SERIAL4_BAUDRATE_HI  (NVRAM_SERIAL4_BAUDRATE_LO + 1) // Baud Rate High byte

/*---------------------------------------------------------------------------*/
/*               Public function prototypes....                              */

unsigned char NVRAM_read_byte( unsigned int );
void NVRAM_write_byte( unsigned int, unsigned char );
unsigned int NVRAM_read_word(unsigned short NV_addr);
void NVRAM_write_word(unsigned short NV_addr, unsigned int NV_data);
void nvramdump(FILE *file);
void nvraminit();
int nvram_good();
void nvramload(char *filename);

#endif  /* _H_NVRAM */
