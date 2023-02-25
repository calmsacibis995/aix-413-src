static char sccsid[] = "@(#)18	1.1  src/rspc/usr/lib/boot/softros/roslib/cmos_sub.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:38:52";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: CMOS_read_byte
 *		CMOS_write_byte
 *		cmos_good
 *		cmosdump
 *		cmosinit
 *		cmosload
 *		crcgen
 *		get_NMI
 *		rol
 *		ror
 *		set_NMI
 *		show_cmos
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
#pragma title("cmos_sub.c : CMOS subroutines")
#pragma subtitle("IBM Confidential")
#pragma options source
#pragma options list
/*****************************************************************************/
/* FILE NAME:    cmos_sub.c                                                  */
/* DESCRIPTION:  This file contains routines for reading and writing CMOS.   */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/*               Include files....                                           */

#pragma options nosource

#include <sys/types.h>
#include <cmos_sub.h>           /* Include public function prototypes...   */
#include <io_sub.h>             /* Include external function prototypes... */
#include <misc_asm.h>

#pragma options source


/*---------------------------------------------------------------------------*/
/*               Global variables and common equate definitions.....         */

static unsigned char NMI_value = NMI_OFF;   /* Global variable equal to NMI_OFF(80) or NMI_ON(0) */

#define CMOS_ADDR_PORT  0x70
#define CMOS_DATA_PORT  0x71



/*---------------------------------------------------------------------------*/
/*               Public routine definitions.....                             */


/*****************************************************************************/
/*  Function: CMOS_read_byte                                                 */
/*            This routine will read a byte from the CMOS offset specified.  */
/*                                                                           */
/*  Input:    CMOS_addr - Address in CMOS to read from                       */
/*                                                                           */
/*  Output:   Value read from CMOS                                           */
/*****************************************************************************/

unsigned char CMOS_read_byte(unsigned int CMOS_addr)
 {
   unsigned char NMI_val, rc;
   unsigned int msr_value;

   msr_value = get_msr();
   if ((msr_value & MSR_EE_BIT_MASK) == MSR_EE_BIT_MASK) {
      /* If external interrupts are on, turn them off */
      reset_msr_ee();
   } /* endif */

   NMI_val = get_NMI();
   set_NMI(NMI_OFF);

   IO_Write(CMOS_ADDR_PORT | IO_PORT_MASK, CMOS_addr | NMI_OFF);
   rc = IO_Read(CMOS_DATA_PORT | IO_PORT_MASK);

   set_NMI(NMI_val);

   if ((msr_value & MSR_EE_BIT_MASK) == MSR_EE_BIT_MASK) {
      /* Restore the state of the external interrupt flag. */
      set_msr_ee();
   } /* endif */

   return(rc);
 }


/*****************************************************************************/
/*  Function: CMOS_write_byte                                                */
/*            This routine will write a byte value to CMOS at the offset     */
/*            specified.                                                     */
/*                                                                           */
/*  Input:    CMOS_addr - Address in CMOS to write to                        */
/*            value - value to write to CMOS                                 */
/*                                                                           */
/*  Output:   No return value                                                */
/*****************************************************************************/

void CMOS_write_byte(unsigned int CMOS_addr, unsigned char value)
 {
   unsigned char NMI_val;
   unsigned int msr_value;

   msr_value = get_msr();
   if ((msr_value & MSR_EE_BIT_MASK) == MSR_EE_BIT_MASK) {
      /* If external interrupts are on, turn them off */
      reset_msr_ee();
   } /* endif */

   NMI_val = get_NMI();
   set_NMI(NMI_OFF);

   IO_Write(CMOS_ADDR_PORT | IO_PORT_MASK, CMOS_addr | NMI_OFF);
   IO_Write(CMOS_DATA_PORT | IO_PORT_MASK, value);

   set_NMI(NMI_val);
   if ((msr_value & MSR_EE_BIT_MASK) == MSR_EE_BIT_MASK) {
      /* Restore the state of the external interrupt flag. */
      set_msr_ee();
   } /* endif */

   return;
 }

unsigned char get_NMI(void)
 {
    /* Return the value of the NMI_value variable */
    return(NMI_value);
 }

void set_NMI(unsigned char NMI_mask)
 {
   /* Set the static NMI_value variable to on or off.... */
   NMI_value = NMI_mask & NMI_ON_OR_OFF_MASK;

   /* Enable or disable NMI by writing to CMOS address port 0x70 */
   IO_Write(CMOS_ADDR_PORT | IO_PORT_MASK, CMOS_REG_D | NMI_mask);

   /* Read the CMOS data port */
   (void) IO_Read(CMOS_DATA_PORT | IO_PORT_MASK);

   return;
 }

/*****************************************************************************/
/*  Function: show_cmos                                                      */
/*            This routine displays the contents of CMOS                     */
/*                                                                           */
/*  Input:    None                                                           */
/*                                                                           */
/*  Output:   No return value                                                */
/*****************************************************************************/

void show_cmos()
 {
   int i;
   for (i=0; i<CMOSSIZE; i++) {
      if (i%8 == 0) {
         printf("\n%02X:  ",i);
      } /* endif */
      printf("%02X ",CMOS_read_byte(i));
   } /* endfor */
   printf("\n");
 }

/*****************************************************************************/
/*  Function: cmos_good                                                      */
/*            This routine determines whether the contents of CMOS are valid */
/*                                                                           */
/*  Input:    None                                                           */
/*                                                                           */
/*  Output:   TRUE if CMOS CRC is good                                       */
/*            FALSE if CMOS shouldn't be trusted                             */
/*****************************************************************************/
int cmos_good()
{
   int i,data;
   int partial;
   char locked = TRUE;

   partial = 0xFFFF;
   for (i=CMOS_CRC_START; i<=CMOS_CRC_1; i++) {
      partial = crcgen(partial,CMOS_read_byte(i));
   } /* endfor */
   if (partial != 0) {
      return FALSE;
   } /* endif */

   partial = 0;
   for (i=CMOS_POP; i<CMOS_POP+15; i++) {
      data = CMOS_read_byte(i);
      if (data == 0xFF) {
         locked = FALSE;
         partial += data;
      } /* endif */
   } /* endfor */
   if (locked == FALSE && partial != CMOS_read_byte(CMOS_POP+15)) {
      return FALSE;
   } /* endif */

   partial = 0;
   locked = TRUE;
   for (i=CMOS_PAP; i<CMOS_PAP+15; i++) {
      data = CMOS_read_byte(i);
      if (data == 0xFF) {
         locked = FALSE;
         partial += data;
      } /* endif */
   } /* endfor */
   if (locked == FALSE && partial != CMOS_read_byte(CMOS_POP+15)) {
      return FALSE;
   } /* endif */
   return TRUE;
}

#define rol(x,y) ( ( ((x)<<(y)) | ((x)>>(16 - (y))) ) & 0xFFFF)
#define ror(x,y) ( ( ((x)>>(y)) | ((x)<<(16 - (y))) ) & 0xFFFF)

/*************************************************************************/
/* Name:crcgen                                                           */
/*                                                                       */
/* Function: Calculates the next CRC value for the next byte in a stream */
/*           Uses the CCITT Polynomial: x**16 + x**12 + x**5 + 1         */
/*                                                                       */
/*           Al Yanes (Tuscon) provided the following XOR tree:          */
/*                                                                       */
/* Where                                                                 */
/*    ^        is defined to be the exclusive or function                */
/*    d0-d7    denotes the incoming data bits                            */
/*    c0-c15   denotes the newly calculated crc (n state)                */
/*    p0-p15   denotes the previous crc (n-1 state)                      */
/*    pd0-pd7  denotes p(x) .xor. d(x)                                   */
/*                                                                       */
/*    c(0)  = p(8)  ^ pd(0) ^ pd(4)                                      */
/*    c(1)  = p(9)  ^ pd(1) ^ pd(5)                                      */
/*    c(2)  = p(10) ^ pd(2) ^ pd(6)                                      */
/*    c(3)  = p(11) ^ pd(0) ^ pd(3) ^ pd(7)                              */
/*    c(4)  = p(12) ^ pd(1)                                              */
/*    c(5)  = p(13) ^ pd(2)                                              */
/*    c(6)  = p(14) ^ pd(3)                                              */
/*    c(7)  = p(15) ^ pd(0) ^ pd(4)                                      */
/*    c(8)  = pd(0) ^ pd(1) ^ pd(5)                                      */
/*    c(9)  = pd(1) ^ pd(2) ^ pd(6)                                      */
/*    c(10) = pd(2) ^ pd(3) ^ pd(7)                                      */
/*    c(11) = pd(3)                                                      */
/*    c(12) = pd(0) ^ pd(4)                                              */
/*    c(13) = pd(1) ^ pd(5)                                              */
/*    c(14) = pd(2) ^ pd(6)                                              */
/*    c(15) = pd(3) ^ pd(7)                                              */
/*                                                                       */
/* Note:                                                                 */
/*       Bit 0 is the MSB                                                */
/* Input:                                                                */
/*       oldcrc         previous CRC value                               */
/*       data           next data byte                                   */
/* Returns:                                                              */
/*       new CRC value                                                   */
/*                                                                       */
/*************************************************************************/
int crcgen(unsigned int oldcrc, unsigned char data)
{
   unsigned int pd, crc;

   pd = ((oldcrc>>8) ^ data) << 8;

   crc = 0xFF00 & (oldcrc << 8);
   crc |= pd >> 8;
   crc ^= rol(pd,4) & 0xF00F;
   crc ^= ror(pd,3) & 0x1FE0;
   crc ^= pd & 0xF000;
   crc ^= ror(pd,7) & 0x01E0;
   return crc;
}

#undef rol
#undef ror

/*************************************************************************/
/* Name:cmosdump                                                         */
/*                                                                       */
/* Function: dumps contents of cmos data area to a file                  */
/*                                                                       */
/* Input:                                                                */
/*       file           pointer to file handle                           */
/* Returns:                                                              */
/*       void                                                            */
/*                                                                       */
/*************************************************************************/
//AIX// void cmosdump(FILE *file)
//AIX// {
//AIX//    int i;
//AIX//    int buffer[29];
//AIX// 
//AIX//    for (i=CMOS_DATA_START;i<CMOS_DATA_END ; i+=8) {
//AIX//       sprintf(buffer,"%02X: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
//AIX//                      i,
//AIX//                      CMOS_read_byte(i),
//AIX//                      CMOS_read_byte(i+1),
//AIX//                      CMOS_read_byte(i+2),
//AIX//                      CMOS_read_byte(i+3),
//AIX//                      CMOS_read_byte(i+4),
//AIX//                      CMOS_read_byte(i+5),
//AIX//                      CMOS_read_byte(i+6),
//AIX//                      CMOS_read_byte(i+7));
//AIX//       fwrite(buffer,29,1,file);
//AIX//    } /* endfor */
//AIX// }

/*************************************************************************/
/* Name:cmosinit                                                         */
/*                                                                       */
/* Function: initializes crc and checksum areas                          */
/*                                                                       */
/* Input:                                                                */
/*       none                                                            */
/* Returns:                                                              */
/*       void                                                            */
/*                                                                       */
/*************************************************************************/
void cmosinit()
{
   int i;
   int partial = 0xFFFF;

   for (i=CMOS_CRC_START;i<=CMOS_CRC_END ; i++) {
      partial = crcgen(partial,CMOS_read_byte(i));
   } /* endfor */
   CMOS_write_byte(CMOS_CRC_0,partial/256);
   CMOS_write_byte(CMOS_CRC_1,partial%256);
   partial = 0;
   for (i=CMOS_POP; i<CMOS_POP+15; i++) {
      partial += CMOS_read_byte(i);
   } /* endfor */
   CMOS_write_byte(CMOS_POP+15,partial&0xFF);
   partial = 0;
   for (i=CMOS_PAP; i<CMOS_PAP+15; i++) {
      partial += CMOS_read_byte(i);
   } /* endfor */
   CMOS_write_byte(CMOS_PAP+15,partial&0xFF);
}

/*************************************************************************/
/* Name:cmosload                                                         */
/*                                                                       */
/* Function: loads contents of cmos from file                            */
/*                                                                       */
/* Input:                                                                */
/*       filename - pointer to filename string                           */
/* Returns:                                                              */
/*       void                                                            */
/*                                                                       */
/*************************************************************************/
//AIX// void cmosload(char *filename)
//AIX// {
//AIX//    int i;
//AIX//    int temp;
//AIX//    int count=0;
//AIX//    unsigned char buffer[30];
//AIX//    unsigned int  data[CMOS_DATA_SIZE];
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
//AIX//    buffer[29] = '\0';
//AIX//    for (i=CMOS_DATA_START; i<=CMOS_DATA_END; i+=8) {
//AIX//       fread(buffer,29,1,file);
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
//AIX//    if (count == CMOS_DATA_SIZE) {
//AIX//       for (i=CMOS_DATA_START ;i<=CMOS_DATA_END ;i++) {
//AIX//          CMOS_write_byte(i,data[i-CMOS_DATA_START]);
//AIX//       } /* endfor */
//AIX//    } else {
//AIX//       printf("Couldn't read all data from file %s\n",filename);
//AIX//    } /* endif */
//AIX//    fclose(file);
//AIX// }
//AIX// 
//AIX// 
#endif /*DEBUG*/
