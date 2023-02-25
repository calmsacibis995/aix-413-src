static char sccsid[] = "@(#)38	1.1  src/rspc/usr/lib/boot/softros/roslib/uart.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:28";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: CheckSerialPort
 *		CheckSerialSpecialCharacter
 *		Default87312
 *		InitParallel
 *		InitSerial
 *		ReadPC87312Regs
 *		WritePC87312Regs
 *		rxchar
 *		txchar
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

#pragma title("Sandalfoot UART")
#ifdef   LISTON
#pragma options source list
#endif

#include <stdio.h>
#include <io_sub.h>
#include <nvram.h>
#include <uart.h>
#include <textwind.h>

PortTest ptstSerialTable1[SERIAL_TABLE1_ENTRIES] = {
               {0x3F8,  0x01, NOT_USED},        // COM1, x'3F8 - 3FF'
               {0x2F8,  0x03, NOT_USED},        // COM2, x'2F8 - 2FF'
               };

PortTest ptstSerialTable2[SERIAL_TABLE2_ENTRIES] = {
               {0x3E8,  0x05, NOT_USED},        // COM3, x'3E8 - 3EF'
               {0x338,  0x0D, NOT_USED},        // COM3, x'338 - 33F'
               {0x2E8,  0x15, NOT_USED},        // COM3, x'2E8 - 2EF'
               {0x220,  0x1D, NOT_USED},        // COM3, x'220 - 227'
               {0x2E8,  0x07, NOT_USED},        // COM4, x'2E8 - 2EF'
               {0x238,  0x0F, NOT_USED},        // COM4, x'238 - 23F'
               {0x2E0,  0x17, NOT_USED},        // COM4, x'2E0 - 2E7'
               {0x228,  0x1F, NOT_USED},        // COM4, x'228 - 22F'
               };

PortTest ptstPar[PARALLEL_TABLE_ENTRIES] = {
               {0x3BC,  0x03, NOT_USED},        // LPT1, x'3BC - 3BE'
               {0x378,  0x01, NOT_USED},        // LPT2, x'378 - 37F'
               {0x278,  0x05, NOT_USED},        // LPT3, x'278 - 27F'
               };

int   iSerialPort = 0x3F8;                      // Serial port base address.
unsigned char  uchSerialBinaryOnly = FALSE;     // Binary mode flag

unsigned char CheckSerialSpecialCharacter(void) {
   int   iCount;

   for (iCount = 0; iCount < 10; iCount++) {
      spin(1);                               // wait for 1 millisecond
      if ((inb(iSerialPort + COM1_LSTAT) & 0x1) != 0) {
         return(TRUE);                       // data ready in the buffer
      } /* endif */
   } /* endfor */

   if (iCount == 10) {
      return(FALSE);       // if no data after 10 milliseconds,
                           //    we assume nothing coming.
   } /* endif */

   } /* endof CheckSerialSpecialCharacter */

// if we have a character, return it, else return -1

int rxchar(void){
   int   iData;            // data received

   if( (inb(iSerialPort + COM1_LSTAT) & 0x1) == 0) return -1;

   // data in the serial port buffer
   iData = (int) inb(iSerialPort + COM1_REC);      // get the data
   if (iData != ESC) {
      return(iData);                               // not escape character
   } /* endif */

   // escape character received
   if (uchSerialBinaryOnly == TRUE) {
      return(iData);                               // binary receive mode
   } /* endif */

   // not in binary mode
   if (CheckSerialSpecialCharacter() == FALSE)
      return(iData);                               // no additional character

   // more data in the buffer coming
   iData <<= 8;                                    // bump the received character
   iData |= (int) inb(iSerialPort + COM1_REC);     // get next character
   switch (iData & 0xFF) {
   case '[':
      if (CheckSerialSpecialCharacter() == FALSE) {
         return(iData);                            // no more character
      } else {
         // more data coming
         iData <<= 8;                              // bump the data one more time
         iData |= (int) inb(iSerialPort + COM1_REC);  // get next character
         switch (iData & 0xFF) {
         case 'A':
            return(UPARROW);
         case 'B':
            return(DOWNARROW);
         case 'C':
            return(RIGHT_ARROW);
         case 'D':
            return(LEFT_ARROW);
         default:
            return(iData);
         } /* endswitch */
      } /* endif */
   case 'A':
      return(UPARROW);
   case 'B':
      return(DOWNARROW);
   case 'C':
      return(RIGHT_ARROW);
   case 'D':
      return(LEFT_ARROW);
   case 'O':
      if (CheckSerialSpecialCharacter() == FALSE) {
         return(iData);                         // no more character
      } else {
         // escape O and more
         iData <<= 8;                           // bump one more time
         iData |= (int) inb(iSerialPort + COM1_REC);  // get the third character
         switch (iData & 0xFF) {
         case 'P':
            return(F1);
         case 'Q':
            return(F2);
         case 'R':
            return(F3);
         case 'S':
            return(F4);
         default:
            return(iData);
         } /* endswitch */
      } /* endif */
   default:
      return(iData);
   } /* endswitch */

}

// wait until the output buffer can take a character
//  and then output it

void txchar(char c){

// wait for clear to send
   while ( (inb(iSerialPort + 6) & 0x10) == 0);

// wait for transmit buffer empty
   while ( (inb(iSerialPort + COM1_LSTAT) & 0x40) == 0);

   switch (c) {
   case UPPERLEFT:
   case UPPERRIGHT:
   case LOWERLEFT:
   case LOWERRIGHT:
   case UPPERLEFT2:
   case UPPERRIGHT2:
   case LOWERLEFT2:
   case LOWERRIGHT2:
   case HLDIVIDEBAR2:
   case HRDIVIDEBAR2:
      c = '+';                   // box corners
      break;
   case HORIZONTAL:
   case HORIZONTAL2:
   case UHALFHORIZONTAL:
      c = '-';                   // horizontal lines
      break;
   case VERTICAL:
   case VERTICAL2:
   case RHALFVERTICAL:
      c = '|';                   // vertical lines
      break;
   case MINBUTTON:
      c = 'M';
      break;
   case SCROLLTOP:
      c = '^';
      break;
   case SCROLLBOT:
      c = 'v';
      break;
   case SCROLLBAR:
      c = '@';
      break;
   case RIGHTTRIANGLE:
   case RIGHTHICKY:
      c = '>';
      break;
   case LEFTTRIANGLE:
   case LEFTHICKY:
      c = '<';
      break;
   default:
      if (c > '~') {
         c = ' ';
      } /* endif */
      break;
   } /* endswitch */

   outb(iSerialPort + COM1_TX,c);
   return;

}

unsigned char ReadPC87312Regs(unsigned char uchPortAddr)
{
   out8(PC87312_INDEX_ADDR, uchPortAddr);
   return(in8(PC87312_DATA_ADDR));
}     // end of ReadPC87312Regs

void WritePC87312Regs(unsigned char uchPortAddr, unsigned char uchData)
{
   out8(PC87312_INDEX_ADDR, uchPortAddr);
   out8(PC87312_DATA_ADDR, uchData);
   out8(PC87312_DATA_ADDR, uchData);
}     // end of WritePC87312Regs

void Default87312(void)
{

   WritePC87312Regs(PC87312_INDEX_FER, PC87312_FER_DEFAULT);
   WritePC87312Regs(PC87312_INDEX_FAR, PC87312_FAR_DEFAULT);

}  // end of Default87312

Bool CheckSerialPort(unsigned int uiAddr)
{
   unsigned char  uchTemp, uchTemp2;

   uchTemp = in8(uiAddr + SPR_OFFSET);           // read the scratchpad register
   out8(uiAddr + SPR_OFFSET, ~uchTemp);          // write the negated scratchpad data
   out8(UART_TEST_PORT, uchTemp);  // The bus may be float if device not present.
   out8(UART_TEST_PORT, uchTemp);                // write something to the bus

   if ((uchTemp2 = in8(uiAddr + SPR_OFFSET)) != ((unsigned char)(~uchTemp))) {
      // if the serial port does not exists, we won't be able to write
      // 8250 does not have scratchpad register. use interrupt identification register instead
         out8(UART_TEST_PORT, INS8250_IIR_MASK);
         out8(UART_TEST_PORT, INS8250_IIR_MASK);
         if ((in8(uiAddr + IIR_OFFSET)) & INS8250_IIR_MASK) {
            // if 8250 exists, these bits will be all zeroes.
            //    if not, 8250 does not exist
            return(TRUE);
         } else {
            // all 0's means it exist
            return(FALSE);
         } /* endif */
   } else {
      // the serial port is used. restore the scratchpad data and return
      out8(uiAddr + SPR_OFFSET, uchTemp);
      return(FALSE);
   } /* endif */
}  // end of CheckSerialPort

unsigned char InitSerial(void)
{
   unsigned char  uchResult = 0;
   unsigned char  uchFound = 0;
   unsigned char  uchTemp;
   int                  iIndex;

   // Check UART 1
   for (iIndex = 0; iIndex < SERIAL_TABLE1_ENTRIES; iIndex++) {
      if (CheckSerialPort(ptstSerialTable1[iIndex].uiTestport) == TRUE) {
         // the serial port does not exists
         uchResult |= (ptstSerialTable1[iIndex].uchMask & CHKUART_FER_MASK);   // Get the FER mask
         uchResult |= ((ptstSerialTable1[iIndex].uchMask & CHKUART_FAR_MASK) << CHKUART_FAR_UART1_OFFSET); // Get the FAR mask
         ptstSerialTable1[iIndex].uchUsed = (unsigned char)USED_BY_PLANAR;
         uchFound++;
         iSerialPort = ptstSerialTable1[iIndex].uiTestport;
         break;
      } else {
         ptstSerialTable1[iIndex].uchUsed = (unsigned char)USED_BY_ADAPTER;
      } /* endif */
   } /* endfor */

   // Check UART 2
   if (uchResult) {
      for (iIndex++; iIndex < SERIAL_TABLE1_ENTRIES; iIndex++) {
         if (CheckSerialPort(ptstSerialTable1[iIndex].uiTestport) == TRUE) {
            // the serial port does not exists
            uchResult |= ((ptstSerialTable1[iIndex].uchMask & CHKUART_FER_MASK) << CHKUART_FER_UART2_OFFSET); // Get the FER masks
            uchResult |= ((ptstSerialTable1[iIndex].uchMask & CHKUART_FAR_MASK) << CHKUART_FAR_UART2_OFFSET); // Get the FAR mask
         ptstSerialTable1[iIndex].uchUsed = (unsigned char)USED_BY_PLANAR;
            uchFound++;
            break;
         } else {
            ptstSerialTable1[iIndex].uchUsed = (unsigned char)USED_BY_ADAPTER;
         } /* endif */
      } /* endfor */
   } /* endif */

   switch (uchResult & (CHKUART_FER_UART1_MASK + CHKUART_FER_UART2_MASK)) {
   case 0:
      // both serial ports are not set yet
      for (iIndex = 0; iIndex < (SERIAL_TABLE2_ENTRIES / 2); iIndex++) {
         if (CheckSerialPort(ptstSerialTable2[iIndex].uiTestport) == TRUE) {
            // COM3 tested does not exists. see if the corresponding pair COM4 exists.
            if (CheckSerialPort(ptstSerialTable2[iIndex + (SERIAL_TABLE2_ENTRIES / 2)].uiTestport) == TRUE) {
               // COM4 does not exists.
               if (uchFound < NUMBER_OF_PLANAR_SERIAL_PORTS) {
                  uchResult |= (ptstSerialTable2[iIndex].uchMask & CHKUART_FER_MASK); // Get the FER mask
                  uchResult |= ((ptstSerialTable2[iIndex].uchMask & CHKUART_FAR_MASK) << CHKUART_FAR_UART1_OFFSET); // Get the FAR mask
                  uchResult &= (~PC87312_FAR_UART2_MASK);      // clear UART 2 COM port seletcion
                  uchResult |= ((ptstSerialTable2[iIndex + (SERIAL_TABLE2_ENTRIES / 2)].uchMask
                         & CHKUART_FER_MASK) << CHKUART_FER_UART2_OFFSET); // Get the FER mask
                  uchResult |= ((ptstSerialTable2[iIndex + (SERIAL_TABLE2_ENTRIES / 2)].uchMask
                         & CHKUART_FAR_MASK) << CHKUART_FAR_UART2_OFFSET); // Get the FAR mask
                  ptstSerialTable2[iIndex].uchUsed = (unsigned char)USED_BY_PLANAR;
                  ptstSerialTable2[iIndex + (SERIAL_TABLE2_ENTRIES / 2)].uchUsed = (unsigned char)USED_BY_PLANAR;
                  iSerialPort = ptstSerialTable2[iIndex].uiTestport; //syy
                  uchFound += 2;
               } else {
                  ptstSerialTable2[iIndex].uchUsed = (unsigned char)NOT_USED;
                  ptstSerialTable2[iIndex + (SERIAL_TABLE2_ENTRIES / 2)].uchUsed = (unsigned char)NOT_USED;
               } /* endif */
            } else {
               ptstSerialTable2[iIndex].uchUsed = (unsigned char)NOT_USED;
               ptstSerialTable2[iIndex + (SERIAL_TABLE2_ENTRIES / 2)].uchUsed = (unsigned char)USED_BY_ADAPTER;
            } /* endif */
         } else {
            ptstSerialTable2[iIndex].uchUsed = (unsigned char)USED_BY_ADAPTER;
            if (CheckSerialPort(ptstSerialTable2[iIndex + (SERIAL_TABLE2_ENTRIES / 2)].uiTestport) == TRUE) {
               ptstSerialTable2[iIndex + (SERIAL_TABLE2_ENTRIES / 2)].uchUsed = (unsigned char)NOT_USED;
            } else {
               ptstSerialTable2[iIndex + (SERIAL_TABLE2_ENTRIES / 2)].uchUsed = (unsigned char)USED_BY_ADAPTER;
            } /* endif */
         } /* endif */
      } /* endfor */
      break;
   case 1:
      // serial 1 is set, but not serial 2
      for (iIndex = 0; iIndex < SERIAL_TABLE2_ENTRIES; iIndex++) {
         if (ptstSerialTable2[iIndex].uchUsed == NOT_USED) {
            if (CheckSerialPort(ptstSerialTable2[iIndex].uiTestport) == TRUE) {
               // the serial port does not exists
               if (uchFound < NUMBER_OF_PLANAR_SERIAL_PORTS) {
                  uchResult |= ((ptstSerialTable2[iIndex].uchMask & CHKUART_FER_MASK) << CHKUART_FER_UART2_OFFSET); // Get the FER mask
                  uchResult |= ((ptstSerialTable2[iIndex].uchMask & CHKUART_FAR_MASK) << CHKUART_FAR_UART2_OFFSET); // Get the FAR mask
                  ptstSerialTable2[iIndex].uchUsed = (unsigned char)USED_BY_PLANAR;
                  uchFound++;
               } else {
                  ptstSerialTable2[iIndex].uchUsed = (unsigned char)NOT_USED;
               } /* endif */
            } else {
               ptstSerialTable2[iIndex].uchUsed = (unsigned char)USED_BY_ADAPTER;
            } /* endif */
         } /* endif */
      } /* endfor */
      break;
   case 2:
      // serial 1 not set, but serial 2 is set. impossible
      break;
   case 3:
      // both serial ports are set
      for (iIndex = 0; iIndex < SERIAL_TABLE2_ENTRIES; iIndex++) {
         if (CheckSerialPort(ptstSerialTable2[iIndex].uiTestport) == FALSE) {
            ptstSerialTable2[iIndex].uchUsed = (unsigned char)USED_BY_ADAPTER;
         } else {
            ptstSerialTable2[iIndex].uchUsed = (unsigned char)NOT_USED;
         } /* endif */
      } /* endfor */
      break;
   default:
      // impossible values
     break;
   } /* endswitch */

   return(uchResult);

   }  // end of InitSerial

unsigned char InitParallel(void)
{
   unsigned char  uchResult = 0;
   unsigned char  uchTemp, uchFound;
   int            iIndex;

   for (iIndex = 0, uchFound = 0; iIndex < PARALLEL_TABLE_ENTRIES; iIndex++) {

      out8(ptstPar[iIndex].uiTestport, CHKPAR_TEST_PATTERN);    // write the test pattern to DTR
      out8(PAR_TEST_PORT, ~CHKPAR_TEST_PATTERN); // The bus may be float if device not present.
      out8(PAR_TEST_PORT, ~CHKPAR_TEST_PATTERN);                // write something to the bus

      uchTemp = in8(ptstPar[iIndex].uiTestport);
      if (uchTemp != ((unsigned char)CHKPAR_TEST_PATTERN)) {

         // If the parallel port does not exists, we won't be able to write the test pattern
         if (uchFound == 0) {
            uchResult = ptstPar[iIndex].uchMask;                // Get the FER and FAR masks
            ptstPar[iIndex].uchUsed = (unsigned char)USED_BY_PLANAR;
            uchFound++;
         } else {
            ptstPar[iIndex].uchUsed = (unsigned char)NOT_USED;
         } /* endif */
      } else {
         ptstPar[iIndex].uchUsed = (unsigned char)USED_BY_ADAPTER;
      } /* endif */
   } /* endfor */

   out8(ptstPar[iIndex].uiTestport + PAR_CTR_OFFSET, PAR_CTR_SLIN);  // select the printer
   return(uchResult);

   }  // end of InitParallel

void PC87312Init(void)
{
   Word  wdTemp;
   unsigned char   uchTemp;
   unsigned char  uchFER = PC87312_FER_FDC_EN;             // Enable FDC
   unsigned char  uchFAR = PC87312_FAR_FDC_PRI;            // Set Primary FDC

   // Read the ID bytes of PC87312. These bytes are available only once after Power On
   wdTemp.bByte.bH = in8(PC87312_INDEX_ADDR);
   wdTemp.bByte.bL = in8(PC87312_INDEX_ADDR);

   if (wdTemp.wWord != PC87312_ID_BYTE) {
      // invalid ID bytes
      return;
   } /* endif */

    // Read FER
   uchTemp = ReadPC87312Regs(PC87312_INDEX_FER);
   if (uchTemp != PC87312_FER_DEFAULT) {
      // invalid FER
      WritePC87312Regs(PC87312_INDEX_FER, PC87312_FER_DEFAULT);
      if (ReadPC87312Regs(PC87312_INDEX_FER) != PC87312_FER_DEFAULT) {
         // unable to write FER
         return;
      } /* endif */
   } /* endif */

   // Read FAR
   uchTemp = ReadPC87312Regs(PC87312_INDEX_FAR);
   if (uchTemp != PC87312_FAR_DEFAULT) {
      // invalid FAR
      WritePC87312Regs(PC87312_INDEX_FAR, PC87312_FAR_DEFAULT);
      if (ReadPC87312Regs(PC87312_INDEX_FAR) != PC87312_FAR_DEFAULT) {
         // unable to write FAR
         return;
      } /* endif */
   } /* endif */

     // Read PTR
   uchTemp = ReadPC87312Regs(PC87312_INDEX_PTR);
   if (uchTemp != PC87312_PTR_DEFAULT) {
      // invalid PTR
      WritePC87312Regs(PC87312_INDEX_PTR, PC87312_PTR_DEFAULT);
      if (ReadPC87312Regs(PC87312_INDEX_PTR) != PC87312_PTR_DEFAULT) {
         // unable to write PTR
         return;
      } else {
      } /* endif */
   } /* endif */

    // Initialize FER and FAR for Serial and Parallel ports
   WritePC87312Regs(PC87312_INDEX_FER, PC87312_FER_FDC_EN);
   if (ReadPC87312Regs(PC87312_INDEX_FER) != ((unsigned char)PC87312_FER_FDC_EN)) {
      return;
   } else {
   } /* endif */
   uchFER |= ((uchTemp = InitParallel()) & FER_PAR_MASK);
   uchFAR |= ((uchTemp & FAR_PAR_MASK) >> FAR_PAR_OFFSET);
   uchFER |= (((uchTemp = InitSerial()) & FER_UART_MASK) << FER_UART_OFFSET);
   uchFAR |= (uchTemp & FAR_UART_MASK);

   WritePC87312Regs(PC87312_INDEX_FER, uchFER);
   WritePC87312Regs(PC87312_INDEX_FAR, uchFAR);

    // Read FCR
   uchTemp = ReadPC87312Regs(PC87322_INDEX_FCR);
   if (uchTemp != PC87322_FCR_DEFAULT) {
      // invalid FCR
      WritePC87312Regs(PC87322_INDEX_FCR, PC87322_FCR_DEFAULT);
      if (ReadPC87312Regs(PC87322_INDEX_FCR) != PC87322_FCR_DEFAULT) {
         // unable to write FCR
         return;
      } /* endif */
   } /* endif */

}     // end of PC87312Init()

void init_uart(void){
int i;

   // Initialize PC87312
      PC87312Init();

// let's do port A to 19,200, no parity, 8-bits, 1 stop bit, no FIFO

        outb(iSerialPort+3, 0x80);    // set DLAB to get to BAUD regs
        if (nvram_good() == TRUE) {
           i = ((ReadPC87312Regs(PC87312_INDEX_FAR) & PC87312_FAR_UART1_MASK)
                >> 2) * NVRAM_SERIAL_SIZE;
           outb(iSerialPort, NVRAM_read_byte(NVRAM_SERIAL1_BAUDRATE_LO + i));
           outb(iSerialPort + 1, NVRAM_read_byte(NVRAM_SERIAL1_BAUDRATE_HI + i));
           outb(iSerialPort + 3, NVRAM_read_byte(NVRAM_SERIAL1_LCR + i));
        } else {
           outb(iSerialPort,   0x0c);    // LSB of C for 9600
           outb(iSerialPort+1, 0x00);    // MSB of C for 9600
           outb(iSerialPort+3, 0x03);    // 8 bit data, no parity, 1 stop
        } /* endif */
        outb(iSerialPort+2, 0x00);    // FIFO off
        outb(iSerialPort+4, 0x03);    // modem control

        inb(iSerialPort+5);           // clear status reg
        inb(iSerialPort);             // clear receive reg

        outb(iSerialPort+7, 0x00);    // clear scratch reg
        outb(iSerialPort+1, 0x00);    // no interrupts

//      for(i=0;i<100;i++){
//      inb(iSerialPort+5);           // clear status reg
//      inb(iSerialPort);             // clear receive reg
//      }

}
#endif /*DEBUG*/
