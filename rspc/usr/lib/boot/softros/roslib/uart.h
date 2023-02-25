/* @(#)58	1.1  src/rspc/usr/lib/boot/softros/roslib/uart.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:40:09  */
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

#define COM1_TX 0x0
#define COM1_REC 0x0
#define COM1_LSTAT 0x5
#define COM1_SCRATCH 0x7
#define  ASYNC    0              // from console.c

//   National PC87312 Super IO Registers
#define         PC87312_INDEX_ADDR           0x398 // Port of Index Address
#define         PC87312_DATA_ADDR            0x399 // Port of Data Address

//   National PC87312 Super IO Configuration Registers
#define         PC87312_INDEX_FER            0x00  // Function Enable Register Index
#define         PC87312_INDEX_FAR            0x01  // Function Address Register Index
#define         PC87312_INDEX_PTR            0x02  // Power and Test Register Index
#define         PC87322_INDEX_FCR            0x03  // Function Control Register Index

//    National PC87312 Super IO Data
#define         PC87312_ID_BYTE           0x8800   // Power-On ID bytes

//    National PC87312 Super IO FER equates
#define         PC87312_FER_PAR_EN           0x01  // FER Parallel Port enabled
#define         PC87312_FER_UART1_EN         0x02  // FER UART1 enabled
#define         PC87312_FER_UART2_EN         0x04  // FER UART2 enabled
#define         PC87312_FER_FDC_EN           0x08  // FER Diskette Controller enabled
#define PC87312_FER_DEFAULT PC87312_FER_PAR_EN + PC87312_FER_UART1_EN + PC87312_FER_UART2_EN + PC87312_FER_FDC_EN
                                      // FER default value :  Parallel Port, UART 1, UART 2, FDC enabled

//    National PC87312 Super IO FAR equates
#define         PC87312_FAR_FDC_PRI          0     // FAR Primary Floppy controller
#define         PC87312_FAR_PAR_MASK         0x03  // FAR mask of Parallel port
#define         FARAddr3BC                   0x01  //   FAR parallel port bits value for LPT1
#define         FARAddr378                   0x00  //   FAR parallel port bits value for LPT2
#define         FARAddr278                   0x02  //   FAR parallel port bits value for LPT3
#define         PC87312_FAR_UART1_MASK       0x0C  // FAR mask of UART 1
#define         PC87312_FAR_UART2_MASK       0x30  // FAR mask of UART 2
#define         PC87312_FAR_COM34_MASK       0xC0  // FAR mask of COM 3, COM4 address
#define         PC87312_FAR_UART1_OFFSET     2     // UART1 FAR bits position
#define         PC87312_FAR_UART2_OFFSET     4     // UART2 FAR bits position
#define         PC87312_FAR_COM34_OFFSET     6     // COM3, COM4 address FAR bits position
#define         PC87312_FAR_DEFAULT          0x11  // FAR default value
                                                   //   Primary FDC, COM1, COM2, LPT1

//    National PC87312 Super IO PTR equates
#define         PC87312_PTR_DEFAULT          0x04  // PTR default value
                                                   //   Power down clocks option
#define         PC87312_PTR_LPT2_IRQ_MASK    0x80  // LPT2 IRQ mask

#define        PAR_CTR_OFFSET                2     // Parallel port control register offset
#define        PAR_CTR_SLIN                  8     // Printer select
#define        PAR_TEST_PORT                 0x3FF // use COM1 Scratchpad reg as parallel test port
#define        UART_TEST_PORT                0x3BC // use LPT1 DTR as serial test port

// PC87322 FCR Equates
#define        PC87322_FCR_DEFAULT           0xC9  // Use data rate, MFM, PPM off, no ZWS

//    Other UART equates
#define        NUMBER_OF_PLANAR_SERIAL_PORTS 2     // number of planar serial ports
#define        IIR_OFFSET                    2     // Serial port Interrupt ID register offset
#define        SPR_OFFSET                    7     // Serial port Scratchpad register offset
#define        INS8250_IIR_MASK              0xF8  // 8250 Interrupt ID register reserved bits mask
#define        SERIAL_TABLE1_ENTRIES         2     // number of possible UART addr. in ptstSerialTable 1
#define        SERIAL_TABLE2_ENTRIES         8     // number of possible UART addr. in ptstSerialTable 2
#define        CHKUART_FER_UART1_MASK        0x01  // UART1 enable mask
#define        CHKUART_FER_UART2_MASK        0x02  // UART2 enable mask
#define        CHKUART_FER_MASK              0x01  // UART mask for FER
#define        CHKUART_FAR_MASK              0x1E  // UART masks for FAR
#define        CHKUART_FER_UART2_OFFSET      1     // UART2 FER bits position
#define        CHKUART_FAR_UART1_OFFSET      1     // UART1 FAR bits position
#define        CHKUART_FAR_UART2_OFFSET      3     // UART2 FAR bits position
#define        FER_UART_MASK                 0x03  // UARTs FER bits mask
#define        FAR_UART_MASK                 0xFC  // UARTs FAR bits mask
#define        FER_UART_OFFSET               1     // FER UART bit position

//    Other parallel port equates
#define        NUMBER_OF_PLANAR_PARALLEL_PORTS  1  // number of planar parallel ports
#define        PARALLEL_TABLE_ENTRIES  3           // number of possible parallel addr
#define        CHKPAR_FER_PAR_MASK     0x01        // Parallel port enable mask
#define        CHKPAR_FER_MASK         CHKPAR_FER_PAR_MASK
#define        CHKPAR_FAR_MASK         0x60        // Parallel port mask for FAR
#define        CHKPAR_TEST_PATTERN     0xAA        // Select the printer for test
#define        FER_PAR_MASK            0x01        // Parallel port FER bits mask
#define        FAR_PAR_MASK            0x06        // Parallel port FAR bits mask
#define        FAR_PAR_OFFSET          1           // Parallel port FAR bits position

//    Common equates
typedef     unsigned char     Bool;                // Boolean type
#define        TRUE              1                 // Boolean True
#define        FALSE             0                 // Boolean False

struct Bytes {
   unsigned char   bH;                             // High byte of a word
   unsigned char   bL;                             // Low byte of a word
   };

typedef union {
   unsigned short wWord;                           // word
   struct Bytes     bByte;                         // word in bytes
   }  Word;

typedef struct PORTTEST {
   unsigned int      uiTestport;                   // Port tested
   unsigned char     uchMask;                      // Mask returned for the port tested
   unsigned char     uchUsed;                      // Indicate the usage of the prot
#define        NOT_USED             0              // Port not used
#define        USED_BY_ADAPTER      1              // Port used by adapter
#define        USED_BY_PLANAR       2              // Port used by planar
   }   PortTest;
