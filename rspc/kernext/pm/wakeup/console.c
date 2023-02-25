static char sccsid[] = "@(#)80  1.2  src/rspc/kernext/pm/wakeup/console.c, pwrsysx, rspc41J, 9517A_all 4/16/95 17:08:22";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: HrPrint, HrPutc, HrPuts, HrGetc, HrGets, HrInitConsole 
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
 *   To control the debug console input/output.
 */

#include "typeport.h"
#include "console.h"

/*
   Define screen size etc.
*/

#define ZLEN_CHAR(x)  (x < 0x10)
#define ZLEN_SHORT(x) ((x < 0x10) + (x < 0x100) + (x < 0x1000))
#define ZLEN_LONG(x)  ((x < 0x10) + (x < 0x100) + (x < 0x1000) \
	+ (x < 0x10000) + (x < 0x100000)+(x < 0x1000000)+(x < 0x10000000))

/*
   Prototypes
*/

VOID
HrPrint(
    PCHAR,
    ...
    );


VOID
HrPuts(
    PCHAR
    );


VOID
HrPutc(
    CHAR
    );


PCHAR
HrGets (
    PCHAR,
    USHORT
    );


UCHAR
HrGetc(
    VOID
    );


VOID
HrInitConsole(
    VOID
    );


/*
   Internal routines
*/

VOID
putx(
    ULONG,
    BOOLEAN
    );


VOID
puti(
    LONG
    );


VOID
putu(
    ULONG
    );


VOID
putch(
    CHAR
    );


USHORT GET_KEY(
    VOID
    );


VOID
HrPrint(
    PCHAR cp,
    ...
    )

/*++

Routine Description:

    Standard printf function with a subset of formating features supported.

    Currently handles

     %d, %ld - signed short, signed long
     %u, %lu - unsigned short, unsigned long
     %c, %s  - character, string
     %x, %lx - unsigned print in hex, unsigned long print in hex

    Does not do:

     - field width specification
     - floating point.

Arguments:

    cp - pointer to the format string, text string.


Returns:

    Nothing


--*/

{
    USHORT b,c,w,len;
    USHORT FixedDigit = 0;
    PUCHAR ap;
    ULONG  l;

    /*
       Cast a pointer to the first word on the stack
    */
   
    ap = (PUCHAR)&cp + sizeof(PCHAR);
    
    /*
       Process the arguments using the descriptor string
    */

    l = 0;
    len = ZLEN_LONG(l);

    while (b = *cp++)
        {
        if (b == '%')
            {
            c = *cp++;

            switch (c)
                {
                case 'd':
                    puti((long)*((int *)ap));
                    ap += sizeof(int);
                    break;

                case 's':
                    HrPuts(*((PCHAR *)ap));
                    ap += sizeof (char *);
                    break;

                case 'c':
                    putch(*((char *)ap));
                    ap += sizeof(int);
                    break;

                case 'x':
                case 'X':
                    w = *((USHORT *)ap);
                    len = ZLEN_SHORT(w);
                    while(len--) putch('0');
                    putx((ULONG)*((USHORT *)ap),(c=='X'));
                    ap += sizeof(int);
                    break;

                case 'u':
                    putu((ULONG)*((USHORT *)ap));
                    ap += sizeof(int);
                    break;

                case '0':
                    c = *cp++;                  // passthru next
                case '1':
                case '2':
                case '4':
                case '8':
                    FixedDigit = (USHORT) (c-'0');
                    c = *cp++;

                    switch (c) {

                    case 'x':
                    case 'X':
                        switch (FixedDigit) {
                        case 1:
                            l = (ULONG) *((ULONG *)ap);
                            l&= 15;
                            len = 0;

                           break;
                        case 2:
                            l = (ULONG) *((ULONG *)ap);
                            len = ZLEN_CHAR(l);

                           break;
                        case 4:
                            l = (ULONG) *((ULONG *)ap);
                            len = ZLEN_SHORT(l);

                           break;
                        case 8:
                            l = (ULONG) *((ULONG *)ap);
                            len = ZLEN_LONG(l);

                           break;
                        default:
                           break;
                        } /* endswitch */
                        while(len--) putch('0');
                        putx(l,(c=='X'));
                        ap += sizeof(int);
                        break;

                    case 'l':
                        c = *cp++;
                        switch (c) {
                        case 'x':
                        case 'X':
                           switch (FixedDigit) {
                           case 1:
                              l = (ULONG) *((ULONG *)ap);
                              l&= 15;
                              len = 0;
                              break;
                           case 2:
                              l = (ULONG) *((ULONG *)ap);
                              len = ZLEN_CHAR(l);
                              break;
                           case 4:
                              l = (ULONG) *((ULONG *)ap);
                              len = ZLEN_SHORT(l);
                              break;
                           case 8:
                              l = (ULONG) *((ULONG *)ap);
                              len = ZLEN_LONG(l);
                              break;
                           default:
                              break;
                           } /* endswitch */
                           while(len--) putch('0');
                           putx(l,(c=='X'));
                           ap += sizeof(int);
                        default:
                           break;
                        } /* endswitch */
                        break;

                    default:
                        break;

                    } /* endswitch */
                    break;

                case 'l':
                    c = *cp++;

                    switch(c) {

                    case 'u':
                        putu(*((ULONG *)ap));
                        ap += sizeof(long);
                        break;

                    case 'x':
                    case 'X':
                        l = *((ULONG *)ap);
                        len = ZLEN_LONG(l);
                        while(len--) putch('0');
                        putx(*((ULONG *)ap),(c=='X'));
                        ap += sizeof(long);
                        break;

                    case 'd':
                        puti(*((ULONG *)ap));
                        ap += sizeof(long);
                        break;

                    }
                    break;

                default :
                    putch((char)b);
                    putch((char)c);
                }
            }
        else
            putch((char)b);
        }

}


VOID HrPuts(
    PCHAR StringPointer
    )
/*++

Routine Description:

    Writes a string on the display at the current cursor position

Arguments:

    cp - pointer to ASCIIZ string to display.


Returns:

    Nothing



--*/

{
    char c;

    while(c = *StringPointer++)
        putch(c);

}



VOID HrPutc(
    CHAR c
    )
/*++

Routine Description:

    Writes a character on the display at the current position.

Arguments:

    c - character to write


Returns:

    Nothing


--*/

{

    putch(c);           /* call internal */

}



PCHAR
HrGets(
    PCHAR sptr,
    USHORT lenmax
    )
/*++

Routine Description:

    Get the keyboard ASCIIZ string (till CR pressed).

    This function has
    - string echo back at the current display position
    - BS key available for character deletion
    - no scancode acceptable for string
      (In the future special key strokes may be used for string retrieve.)


Arguments:

    sptr   - string buffer
    lenmax - string maximum length(actual buffer size-1 for NUL termination)


Returns:

    original string buffer pointer returned


--*/

{
    PCHAR tptr;
    USHORT lencnt = 0;
    CHAR ch;

#define  PROMPTER    ''
    tptr = sptr;
    while (lencnt<lenmax) {
        ch = HrGetc();
        if (ch==ASCI_XTEN1||ch==ASCI_XTEN2) {
            ch = HrGetc();          /* ignore scancode */
        } else {
            switch (ch) {
            case ASCI_CR:
                *sptr++ = ch;
                lencnt++;
                putch('\n');
                *sptr = ASCI_NUL;   /* delete CR code  */
                return tptr;

            case ASCI_BS:
                if (lencnt!=0) {
                    sptr--;
                    lencnt--;
                    putch(ASCI_BS);
                } /* endif */
                break;

            default:
                *sptr++ = ch;
                lencnt++;
                putch(ch);
                break;

            } /* endswitch */
        } /* endif */
    } /* endwhile */
    *sptr++ = ASCI_NUL;

    return tptr;

}



UCHAR
HrGetc(
    VOID
    )
/*++

Routine Description:

    Get the keyboard scancode (one byte only waiting)

Arguments:

    Nothing


Returns:

    Keyboard data returned


--*/

{
    USHORT keyscan;
    UCHAR   ch;

    while ((keyscan=GET_KEY())==0) {
    } /* endwhile */
    ch = (UCHAR) keyscan;

    return ch;

}


VOID
HrInitConsole(
    VOID
    )

/*++

Routine Description:

    Initialize COM port(flag only)

Arguments:

    Nothing

Returns:

    Nothing

--*/

{
    ComportInitialize();     
}



//
// Write a hex short to display
//

VOID putx(
    ULONG x,
    BOOLEAN cflag
    )
/*++

Routine Description:

    Writes hex long to the display at the current cursor position.

Arguments:

    x  - ulong to write.

Returns:

    Nothing


--*/

{
    ULONG j;

    if (x/16)
        putx(x/16,cflag);

    if((j=x%16) > 9) {
        if (cflag) {
            putch((char)(j+'A'- 10));
        } else {
            putch((char)(j+'a'- 10));
        }
    } else {
        putch((char)(j+'0'));
    }
}


VOID puti(
    LONG i
    )
/*++

Routine Description:

    Writes a long integer on the display at the current cursor position.

Arguments:

    i - the integer to write to the display.

Returns:

    Nothing


--*/

{
    if (i<0)
        {
        i = -i;
        putch((char)'-');
        }
    if (i/10)
        puti(i/10);

    putch((char)((i%10)+'0'));
}


VOID putu(
    ULONG u
    )
/*++

Routine Description:

    Write an unsigned long to display

Arguments:

    u - unsigned


--*/

{
    if (u/10)
        putu(u/10);

    putch((char)((u%10)+'0'));

}


VOID putch(
    CHAR c
    )
/*++

Routine Description:

    Writes a character on the display at the current position.

Arguments:

    c - character to write


Returns:

    Nothing


--*/



{
    ComportPutchar(c);
    return;
}



USHORT GET_KEY(
    VOID
    )
/*++

Routine Description:

    Get the keyboard scancode as compatible as BIOS INT 16h AH=00h

Arguments:

    Nothing

Returns:

    Keyboard data returned

--*/

{
    USHORT  Key;
    Key = ComportGetchar();

    return Key;

}

/* END OF FILE */
