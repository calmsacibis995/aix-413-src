static char sccsid[] = "@(#)78  1.1  src/rspc/kernext/pm/wakeup/comport.c, pwrsysx, rspc41J, 9510A 3/8/95 06:44:02";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: ComportIdleCall, ComportGetcharPolled, ComportPutcharPolled 
 *              ComportInitialize 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */ 
/*
 *   ABSTRACT:
 *
 *   This code is used for debug purpose only. 
 *   To initialize hardware-dependent portions of serial COM port.
 */

#define  CRLF_OUTBOUND  1                    /* LF->CR/LF conversion    */
#define  COM_BASE	COM1_BASE
#define  COM_IRQ	COM1_INTERRUPT_LEVEL

#include "typeport.h"
#include "comport.h"
#include "ioport.h"

/*++
   prototypes
--*/

VOID ComportIdleCall(
   VOID 
   );

USHORT ComportGetcharPolled(
   VOID
   );

USHORT ComportPutcharPolled(
   UCHAR ch
   );

USHORT ComportGetchar(
   VOID
   );

USHORT ComportPutchar(
   UCHAR ch
   );

VOID ComportInitialize(
   VOID
   );

VOID SetComIntStat(
   USHORT PortBase, 
   USHORT PortIRQ 
   );

/*++
   Start of COMPORT codes
--*/

VOID ComportIdleCall(
   VOID
   )

/*++

Routine Description:

    Execute Idle Process for COM port.

Arguments:

    None

Return Value:

    None

--*/

{
   UCHAR ch;
   do {
      ch = ReadPortUchar( COM_BASE+LSR );
      if (ch&LS_DR) {
         ch = ReadPortUchar( COM_BASE+RXB );
         ComportPutcharPolled( ch );
      }
   } while (ch&LS_DR); 
}

USHORT ComportGetcharPolled(
   VOID
   )

/*++

Routine Description:

    Get 1 char from COM port BY POLLING.

Arguments:

    None

Return Value:

    character data and status

--*/

{
   USHORT val;
   UCHAR ch;
   while (1) {
      ch = ReadPortUchar( COM_BASE+LSR );
      if (ch&LS_DR) {                                
         ch = ReadPortUchar( COM_BASE+RXB );       
         val = ch + RX_DATAREADY;
         break;
      } /* endif */
   } /* endwhile */
   return val;                  
} /* ComportGetcharPolled */


USHORT ComportPutcharPolled(
   UCHAR ch
   )

/*++

Routine Description:

    Put 1 char into COM port BY POLLING.

Arguments:

    character data

Return Value:

    status

--*/

{
   USHORT val;
   UCHAR sch;
   while (1) {
      ComportIdleCall();
      sch = ReadPortUchar( COM_BASE+LSR );    
      if (sch&LS_THRE) {                   
         WritePortUchar( ch, COM_BASE+TXB );
         break;
      } /* endif */
   } /* endwhile */
   val = TX_SUCCESS;
   return val;                                                                 
} /* ComportPutcharPolled */


USHORT ComportGetchar(
   VOID
   )

/*++

Routine Description:

    Get 1 char from COM port.

Arguments:

    None

Return Value:

    character data and status

--*/

{
   USHORT val;
   val = ComportGetcharPolled();
   return val;
} /* ComportGetchar */


USHORT ComportPutchar(
   UCHAR ch
   )

/*++

Routine Description:

    Put 1 char into COM port.

Arguments:

    character data

Return Value:

    status

--*/

{
   USHORT val;
#ifdef CRLF_OUTBOUND
   if (ch==ASCI_LF) {
      val = ComportPutcharPolled( ASCI_CR );
   } /* endif */
#endif
   val = ComportPutcharPolled( ch );
   return val;
} /* ComportPutchar */


VOID
ComportInitialize (
   VOID
   )

/*++

Routine Description:

    This routine initializes the serial COM port.

Arguments:

    None

Return Value:

    None

--*/

{

   SetupSpIO();

   SetComIntStat(COM_BASE,COM_IRQ);

} /* ComportInitialize */


VOID  SetComIntStat (
   USHORT PortBase,
   USHORT PortIRQ
   )

/*++

Routine Description:

    This routine initializes the specified serial COM port.

Arguments:

    None

Return Value:

    None

--*/

{
   UCHAR ch;

   WritePortUchar( 0, PortBase+IER );
   WritePortUchar( LC_DLAB, PortBase+LCR );
// WritePortUchar( LSB_19200, PortBase+LATLSB );
// WritePortUchar( MSB_19200, PortBase+LATMSB );
   WritePortUchar( LSB_9600, PortBase+LATLSB );
   WritePortUchar( MSB_9600, PortBase+LATMSB );
   WritePortUchar( LC_PNONE+LC_STOP1+LC_BITS8, PortBase+LCR );
   WritePortUchar( MC_OUT2+MC_DTR+MC_RTS, PortBase+MCR );
   WritePortUchar( 0, PortBase+IER );           // do not use INT
   WritePortUchar( FC_ENB+FC_RTL1+FC_RTL0, PortBase+FCR );
   ch = ReadPortUchar( PortBase+RXB );          // purge stray data
   ch = ReadPortUchar( PortBase+LSR );          // get LSR

   return;

} /* SetComIntStat */

/* END OF FILE */
