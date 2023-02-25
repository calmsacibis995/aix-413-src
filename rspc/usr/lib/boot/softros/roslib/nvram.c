static char sccsid[] = "@(#)22	1.1  src/rspc/usr/lib/boot/softros/roslib/nvram.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:00";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: NVRAM_read_byte
 *		NVRAM_read_word
 *		NVRAM_write_byte
 *		NVRAM_write_word
 *		nvram_good
 *		nvramdump
 *		nvraminit
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
#ifdef DEBUG
#pragma pagesize(60)                          /* set listing page size   */
//#pragma linesize(132)                         /* set listing line size   */
#pragma title("nvram.c : NVRAM subroutines")
#pragma subtitle("IBM Confidential")
#pragma options source
#pragma options list
//************************** OCO SOURCE MATERIALS *****************************
//******* IBM CONFIDENTIAL (IBM CONFIDENTIAL-RESTRICTED when combined *********
//************** with the aggregated modules for this product) ****************
//*****************************************************************************
// FILE NAME:          nvram.c
//
// COPYRIGHT:          (C) Copyright IBM Corp. 1993, 1993
//
// DESCRIPTION: This file contains general routines for reading and writing
//              to NVRAM.
//
//
// REVISION HISTORY:
// -----------------------Sandalfoot-----------------------------------------
// Date      Track#  Who  Description
// --------  ------  ---  ---------------------------------------------------
//
//*****************************************************************************

/*---------------------------------------------------------------------------*/
/*               Include files....                                           */

#pragma options nosource

#include <stddef.h>
#include <nvram.h>              /* Include public function prototypes...   */
#include <io_sub.h>             /* Include external function prototypes... */

#pragma options source


/*---------------------------------------------------------------------------*/
/*               Common equate definitions.....                              */

#define IO_PORT_MASK                    0x80000000
#define NV_ADDR_HI     0x75
#define NV_HI_MASK     0xFF00
#define NV_ADDR_LO     0x74
#define NV_LO_MASK     0x00FF
//#define NV_DATA        0x76
#define NV_DATA        0x77             // Hopefully, this is temporary!



/*---------------------------------------------------------------------------*/
/*               Public routine definitions.....                             */

/*****************************************************************************/
/*  Function: NVRAM_write_byte                                               */
/*            This routine will write a byte value to NVRAM at the offset    */
/*            specified.                                                     */
/*                                                                           */
/*  Input:    NV_addr - Address in NVRAM to write to                         */
/*            NV_value - value to write to NVRAM                             */
/*                                                                           */
/*  Output:   No return value                                                */
/*****************************************************************************/

void NVRAM_write_byte(unsigned int NV_addr, unsigned char NV_value)
 {
  unsigned char nv_hi_addr_byte, nv_lo_addr_byte;

   nv_hi_addr_byte = (NV_addr & NV_HI_MASK) >> 8;
   nv_lo_addr_byte = NV_addr & NV_LO_MASK;
   IO_Write(NV_ADDR_HI | IO_PORT_MASK,  nv_hi_addr_byte);
   IO_Write(NV_ADDR_LO | IO_PORT_MASK,  nv_lo_addr_byte);
   IO_Write(NV_DATA | IO_PORT_MASK,  NV_value);
   return;
  }

/*****************************************************************************/
/*  Function: NVRAM_read_byte                                                */
/*            This routine will read a byte from the NVRAM offset specified. */
/*                                                                           */
/*  Input:    NV_addr - Address in NVRAM to read from                        */
/*                                                                           */
/*  Output:   Value read from NVRAM                                          */
/*****************************************************************************/

unsigned char NVRAM_read_byte(unsigned int NV_addr)
 {
  unsigned char rc;
  unsigned char nv_hi_addr_byte, nv_lo_addr_byte;

   nv_hi_addr_byte = (NV_addr & NV_HI_MASK) >> 8;
   nv_lo_addr_byte = NV_addr & NV_LO_MASK;
   IO_Write(NV_ADDR_HI | IO_PORT_MASK,  nv_hi_addr_byte);
   IO_Write(NV_ADDR_LO | IO_PORT_MASK,  nv_lo_addr_byte);

   rc = IO_Read(NV_DATA | IO_PORT_MASK);
   return(rc);
  }

/*****************************************************************************/
/*  Function: NVRAM_read_word                                                */
/*            This routine will read a word from the NVRAM offset specified. */
/*                                                                           */
/*  Input:    NV_addr - Address in NVRAM to read from                        */
/*                                                                           */
/*  Output:   Value read from NVRAM                                          */
/*                                                                           */
/*  NOTE: This routine returns big-endian values.  In other words, the byte  */
/*        at NV_addr is assumed to be the most significant byte and the      */
/*        byte at NV_addr+3 is assumed to be the least significant byte.     */
/*****************************************************************************/

unsigned int NVRAM_read_word(unsigned short NV_addr)
 {
    unsigned int return_value;

    return_value = (NVRAM_read_byte(NV_addr) << 24)   +
                   (NVRAM_read_byte(NV_addr+1) << 16) +
                   (NVRAM_read_byte(NV_addr+2) << 8)  +
                   NVRAM_read_byte(NV_addr+3);

    return(return_value);
 }


/*****************************************************************************/
/*  Function: NVRAM_write_word                                               */
/*            This routine will write a word value to NVRAM at the offset    */
/*            specified.                                                     */
/*                                                                           */
/*  Input:    NV_addr - Address in NVRAM to write to                         */
/*            NV_value - value to write to NVRAM                             */
/*                                                                           */
/*  Output:   No return value                                                */
/*                                                                           */
/*  NOTE: This routine writes big-endian values.  In other words, the byte   */
/*        written to NV_addr is the most significant byte and the byte       */
/*        written to NV_addr+3 is the least significant byte.                */
/*****************************************************************************/

void NVRAM_write_word(unsigned short NV_addr, unsigned int NV_data)
 {

   NVRAM_write_byte(NV_addr, (unsigned char)(NV_data >> 24));
   NVRAM_write_byte(NV_addr+1, (unsigned char)(NV_data >> 16) );
   NVRAM_write_byte(NV_addr+2, (unsigned char)(NV_data >> 8) );
   NVRAM_write_byte(NV_addr+3, (unsigned char)NV_data );

   return;
 }

/*************************************************************************/
/* Name:nvramdump                                                        */
/*                                                                       */
/* Function: dumps contents of cmos data area to a file                  */
/*                                                                       */
/* Input:                                                                */
/*       file           pointer to file handle                           */
/* Returns:                                                              */
/*       void                                                            */
/*                                                                       */
/*************************************************************************/
//AIX// void nvramdump(FILE *file)
//AIX// {
//AIX//    int i;
//AIX//    int buffer[29];
//AIX// 
//AIX//    for (i=NVRAM_DATA_START;i<NVRAM_DATA_END ; i+=8) {
//AIX//       sprintf(buffer,"%03X: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
//AIX//                      i,
//AIX//                      NVRAM_read_byte(i),
//AIX//                      NVRAM_read_byte(i+1),
//AIX//                      NVRAM_read_byte(i+2),
//AIX//                      NVRAM_read_byte(i+3),
//AIX//                      NVRAM_read_byte(i+4),
//AIX//                      NVRAM_read_byte(i+5),
//AIX//                      NVRAM_read_byte(i+6),
//AIX//                      NVRAM_read_byte(i+7));
//AIX//       fwrite(buffer,30,1,file);
//AIX//    } /* endfor */
//AIX// }

/*************************************************************************/
/* Name:nvraminit                                                        */
/*                                                                       */
/* Function: initializes crc                                             */
/*                                                                       */
/* Input:                                                                */
/*       none                                                            */
/* Returns:                                                              */
/*       void                                                            */
/*                                                                       */
/*************************************************************************/
void nvraminit()
{
   int i;
   int partial = 0xFFFF;

   for (i=NVRAM_CRC_START;i<=NVRAM_CRC_END ; i++) {
      partial = crcgen(partial,NVRAM_read_byte(i));
   } /* endfor */
   NVRAM_write_byte(NVRAM_CRC_0,partial/256);
   NVRAM_write_byte(NVRAM_CRC_1,partial%256);
}

/*************************************************************************/
/* Name:nvram_good                                                       */
/*                                                                       */
/* Function: indicates whether the crc across nvram is good              */
/*                                                                       */
/* Input:                                                                */
/*       none                                                            */
/* Returns:                                                              */
/*       0  -    if crc is correct (residual = 0)                        */
/*       1  -    contents of nvram not good                              */
/*                                                                       */
/*************************************************************************/
int nvram_good()
{
   int i;
   int partial = 0xFFFF;

   for (i=NVRAM_CRC_START;i<=NVRAM_CRC_END ; i++) {
      partial = crcgen(partial,NVRAM_read_byte(i));
   } /* endfor */
   partial = crcgen(partial,NVRAM_read_byte(NVRAM_CRC_0));
   partial = crcgen(partial,NVRAM_read_byte(NVRAM_CRC_1));
   if (partial) {
      return FALSE;
   } else {
      return TRUE;
   } /* endif */
}

/*************************************************************************/
/* Name:nvramload                                                        */
/*                                                                       */
/* Function: loads contents of cmos from file                            */
/*                                                                       */
/* Input:                                                                */
/*       filename - pointer to filename string                           */
/* Returns:                                                              */
/*       void                                                            */
/*                                                                       */
/*************************************************************************/
//AIX// 
//AIX// Don't need this for AIX makes me have to include all the FAT stuff
//AIX// 
//AIX// void nvramload(char *filename)
//AIX// {
//AIX//    int i;
//AIX//    int temp;
//AIX//    int count=0;
//AIX//    unsigned char buffer[31];
//AIX//    unsigned int  data[NVRAM_DATA_SIZE];
//AIX//    FIND_T f;
//AIX//    FILE *file;
//AIX// 
//AIX//    printf("\n");
//AIX//    f.SearchName = filename;
//AIX//    f.attribute = 0;
//AIX// 
//AIX//    i = ffind_first(&f);
//AIX// 
//AIX//    if (!i) {
//AIX//       printf("File %s not found\n",filename);
//AIX//       return;
//AIX//    }
//AIX// 
//AIX//    if (f.FileSize == 0) {
//AIX//       printf("File %s is empty\n",filename);
//AIX//       return;
//AIX//    } /* endif */
//AIX//    if ((file = fopen(filename,"r")) == NULL) {
//AIX//       printf("Couldn't open file %s\n",filename);
//AIX//       return;
//AIX//    } /* endif */
//AIX// 
//AIX//    buffer[30] = '\0';
//AIX//    for (i=NVRAM_DATA_START; i<=NVRAM_DATA_END; i+=8) {
//AIX//       fread(buffer,30,1,file);
//AIX//       if (sscanf(buffer,"%x: %x %x %x %x %x %x %x %x\r\n",
//AIX//                         &temp,
//AIX//                         data+count,
//AIX//                         data+count+1,
//AIX//                         data+count+2,
//AIX//                         data+count+3,
//AIX//                         data+count+4,
//AIX//                         data+count+5,
//AIX//                         data+count+6,
//AIX//                         data+count+7) != 9) {
//AIX//          printf("load failed, data not in correct format\n");
//AIX//       } else {
//AIX//          count += 8;
//AIX//       } /* endif */
//AIX//    } /* endfor */
//AIX// 
//AIX//    if (count == NVRAM_DATA_SIZE) {
//AIX//       for (i=NVRAM_DATA_START ;i<=NVRAM_DATA_END ;i++) {
//AIX//          NVRAM_write_byte(i,data[i-NVRAM_DATA_START]);
//AIX//       } /* endfor */
//AIX//    } else {
//AIX//       printf("Couldn't read all data from file %s\n",filename);
//AIX//    } /* endif */
//AIX//    fclose(file);
//AIX// }
#endif /*DEBUG*/
