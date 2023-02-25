static char sccsid[] = "@(#)89  1.1  src/rspc/kernext/pm/wakeup/spiosup.c, pwrsysx, rspc41J, 9510A 3/8/95 06:44:17";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: WriteSpIOPortUchar, ReadSpIOPortUchar, SetupSpIO
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
 *   To initialize hardware-dependent portions of Super I/O chip.
 */

#include "typeport.h"
#include "spiosup.h"
#include "ioport.h"

/*++ 
    Prototypes
--*/

VOID WriteSpIOPortUchar(
    UCHAR Data,
    UCHAR Index
    );

UCHAR ReadSpIOPortUchar(
    UCHAR Index
    );

VOID SetupSpIO(
    VOID
    );

/*++
    Start of codes 
--*/

VOID WriteSpIOPortUchar(
    UCHAR Data,
    UCHAR Index
    )
{
    WritePortUchar(Index,SpIO_INDEX);
    WritePortUchar(Data,SpIO_DATA);   
    WritePortUchar(Data,SpIO_DATA);
}

UCHAR ReadSpIOPortUchar(
    UCHAR Index
    )
{
    UCHAR ch;
    WritePortUchar(Index,SpIO_INDEX);
    ch = ReadPortUchar(SpIO_DATA);
    return ch;
}

VOID SetupSpIO(
    VOID 
    )
/*++

Routine Description:

    This routine initializes the super i/o registers. 

Arguments:

    None

Return Value:

    None

--*/

{
    UCHAR ch;

    ch = ReadSpIOPortUchar(FER_INDEX);

    if ((ch & FER_UR1ENB)==0) {
       ch |= FER_UR1ENB; 
    }
    if ((ch & FER_UR2ENB)==0) {
       ch |= FER_UR2ENB;
    }

    WriteSpIOPortUchar(ch,FER_INDEX);

    ch = FAR_DEFAULT;  /* COM1=3F8, COM2=2F8, COM3=3E8, COM4=2E8 */ 
    WriteSpIOPortUchar(ch,FAR_INDEX);

} /* SetupSpIO */

/* END OF FILE */
