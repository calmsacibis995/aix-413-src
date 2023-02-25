static char sccsid[] = "@(#)12	1.2  src/rspc/usr/lib/boot/softros/lib/libstand/am_con.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:34:29";
#ifndef lint
#endif
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: ConsoleInit
 *		ConsoleRead
 *		ConsoleWrite
 *		_Readchar
 *		_Writechar
 *		con_set
 *		console_setup
 *		getch
 *		special
 *		status
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef DEBUG

#include <types.h>
#include <stand.h>
#include "am_con.h"

/*   This file contains a very primitive character device I/O subsystem.  It */
/* will handle stdin, stdout, stderr as 'special' devices and redirect them  */
/* to the currently active console.  It implements much of the standard tty  */
/* device driver cooked mode processing.  We try to stand standard POSIX and */
/* often succeed, but read the code if you need to know for sure what is     */
/* happening.                                                                */
/*                                                                           */
/*   We will handle stdin(0), stdout(1), and stderr(2) specially here.  This */
/* module keeps a pointer to the 'real' file descriptor of the system console*/
/* device and will vector through that to get and put characters.            */
/*                                                                           */
/* Minor    Description                                                      */
/*   0      Console - Will redirect to someone else                          */
/*   1      serial port 1                                                    */
/*   2      serial port 2                                                    */


/* Local definitions                                                         */
#define MAXMINOR  12               /* Number of minor devices supported      */

/* cio - This is a pointer to the read/write/status routines for consoles    */
struct cio
  {
  int (*cstat)();
  char (*cgetc)();
  void (*cputc)();
  short iport;
  short oport;
  short flag1;                     /* Standard flags                         */
  short curpos;
  char *description;               /* Holds a description                    */
  };
#define F1NOOUT 0x0001             /* Means flush output (dec ^O)            */
#define F1BIT8  0x0002             /* Means handle as 8 bits of data         */
#define F1XOFF  0x0004             /* Means wait for XON                     */


static int console_minor = 0;      /* Default console is com1                */

/* Define the various possible console device                                */
static struct cio com1 =  {sio_stat, sio_getc, sio_putc,  1, 1, 0, 0, "com1"};
static struct cio com2 =  {sio_stat, sio_getc, sio_putc,  2, 2, 0, 0, "com2"};

static struct cio *minors[MAXMINOR] =
  {
  &com1,                           /* Console is minor 0                   */
  &com1,                           /* Serial port 1 is minor 1             */
  &com2                            /* Serial port 2 is minor 2             */
  };

/* Define the buffered input subsystem data structures                       */
#define CBUFSIZE 100
static char cinbuf[CBUFSIZE];
static int chead = 0;              /* Pointer to first char in buffer        */
static int ctail = 0;              /* Pointer to charcater to display        */

/* status - This routine will check the console input device and buffer any  */
/*          characters it finds.  It will also act on the control characters */
/*          that it finds.                                                   */
static void status()

{
  char (*inchar)();                /* Pointer to input routine               */
  int  (*cstat)();                 /* Status routine to see if data waiting  */
  int mdev;                        /* Always use the console device          */
  int iport;                       /* Input port to use                      */
  int tmp;                         /* Used to index cinbuf                   */
  char c;                          /* A character that we read               */

  /* Figure out the current console stuff                                    */
  mdev   = console_minor;          /* Always use the default console         */
  inchar = minors[mdev]->cgetc;    /* Blocking read of a character           */
  cstat  = minors[mdev]->cstat;    /* Status routine to read from            */
  iport  = minors[mdev]->iport;    /* Minor number to pass read subrotuine   */


  /* Do XOFF wait here                                                       */
  if (minors[mdev]->flag1 & F1XOFF) /* Are we held off here?                 */
    {                              /* YES - Wait for XON to show up          */
    c = 0x00;                      /* Start with nothing                     */
    while (c != 0x11)              /* Loop until character read is XON       */
	{
	if(sio_stat(1))
            c = sio_getc(1);
	else
            if((*(cstat))(iport))
                c = (*(inchar))(iport);
	}
    minors[mdev]->flag1 &= !F1XOFF; /* Turn off the wait for me flag         */
    }

  /* If there is no data waiting, then just return                           */
  if ((!(*(cstat))(iport)) && (!sio_stat(1)))
    return;

  /* Data was waiting, read it and see if we need to do anything             */
  if(sio_stat(1))
	c = sio_getc(1);
  else
  	c = (*(inchar))(iport);

  switch (c)
    {
    case 0x13:                     /* XOFF - stop output for a while         */
      minors[mdev]->flag1 |= F1XOFF;
      c = 0;                       /* We don't store this guy                */
      break;

    case 0x11:                     /* XON - Should not see this but....      */
      minors[mdev]->flag1 &= !F1XOFF;
      c = 0;                       /* Ignore it if we do                     */
      break;

    case 0x0f:                     /* ^O means toggle output dump            */
      minors[mdev]->flag1 ^= F1NOOUT;
      c = 0;
      break;

    }

  /* If there is still data waiting, put it in the buffer                    */
  if (c == 0)                      /* Did we kill the character?             */
    return;                        /* YES - All done here then               */

  /* Remember: Polled I/O is one big critical section.  Don't try this at    */
  /*           home.                                                         */
  cinbuf[chead++] = c;             /* Save the byte in the buffer            */
  tmp = (chead >= CBUFSIZE) ? 0 : chead;
  if (tmp != ctail)                /* Would we eat our tail?                 */
    chead = tmp;                   /*  NO - Then it is safe to remember c    */
}



/* _Writechar - This routine will subselect the correct minor device and     */
/*              send output to it.                                           */
static void _Writechar(int mdev, char c)

{
  void (*outchar)();               /* Pointer to real output device          */
  int oport;

  /* Check for characters waiting before any real output takes place         */
  status();                        /* Note that XON/XOFF happens here        */

  /* If we are flushing output, then we are done                             */
  if (minors[mdev]->flag1 & F1NOOUT)
    return;

  /* Get the routine to output a byte and call it                            */
  outchar = minors[mdev]->cputc;   /* Get output pointer                     */
  oport   = minors[mdev]->oport;   /* and port to use                        */

  (*(outchar))(oport, c);
  return;
}


/* _Readchar - This routine will subselect the correct minor device and      */
/*             read one character from it.                                   */
static char _Readchar(int mdev)

{
  char c;                          /* Character to read                      */

  /* Wait for a character to show up in the buffer                           */
  while (chead == ctail)           /* status() updates chead                 */
    status();

  /*   Get the byte from the buffer.  Notice we always buffer it so special  */
  /* processing takes place.                                                 */
  c = cinbuf[ctail++];             /* Remeber critical section comments      */
  ctail = (ctail >= CBUFSIZE) ? 0 : ctail;

  /* Happy trails                                                            */
  return(c);
}


/* special - This output routine handles how we display special characters   */
/*           on whatever output device.  It is called with a minor number    */
/*           and a character.  It will write the character with all special  */
/*           processing.  In some cases the character will be translated,    */
/*           and this routine will return that translation.                  */
static char special(int mdev, char c)

{
  char ctran;                      /* The translated value                   */
  int i;

  /* Setup the default translation                                           */
  ctran = c;                       /* Assume gigo                            */

  /* Since this is an internal routine we trust calling parms, just do it.   */
  switch (c)
    {
    case 0:                        /* We don't do anything will nulls        */
      break;

    case '\b':                     /* Backspace echos as bs, space, bs       */
      _Writechar(mdev, c);
      _Writechar(mdev, ' ');
      _Writechar(mdev, c);
      minors[mdev]->curpos--;      /* Remember we just moved over one char   */
      if (minors[mdev]->curpos < 0)
        minors[mdev]->curpos = 79;
      break;

    case '\t':                     /* Tab - Use Eric/Mott 4 spaces / tab     */
      i = 4 - (minors[mdev]->curpos % 4);
      minors[mdev]->curpos += i;
      while (i)
        {
        _Writechar(mdev, ' ');
        i--;
        }
      break;

    case '\f':                     /* Form feed will clear the screen        */
      for (i=0;i<25;i++)
        _Writechar(mdev,'\n');     /* Output 25 line feeds                   */
      break;

    case '\r':                     /* <CR> will echo as <CR>                 */
      _Writechar(mdev,'\r');
      minors[mdev]->curpos = 0;    /* Remember we are at the left most spot  */
      break;

    case '\n':                     /* <NL> is echoed as <CR><LF>             */
      _Writechar(mdev,'\r');
      _Writechar(mdev,'\n');
      ctran = '\n';                /* We also translate this one on input    */
      minors[mdev]->curpos = 0;    /* Remember we are at the left most spot  */
      break;

    case 0x0f:                     /* ^O only shows up here on output        */
      minors[mdev]->flag1 &= !F1NOOUT;
      ctran = 0;                   /* We just ignore the byte for output     */
      break;                       /* It will turn on display again          */

    case 0x04:                     /* Don't really write a ^D                */
      break;

    default:                       /* Everything else will just echo         */
      _Writechar(mdev, c);
      minors[mdev]->curpos++;      /* Remember we just moved over one char   */
      break;

    }

  /* All done with special output, process translation and return            */
  return(ctran);
}

/* ConsoleWrite - This routine is the 'device driver' for major number       */
/*                0, all of the character I/O devices.  It is called         */
/*                with a file descriptor, which contains a dev_t and         */
/*                thus a minor number along with a character to write        */
/*                to a console device.  It will use the minor number         */
/*                from that dev_t to index into the static array of          */
/*                handlers to call the correct I/O routine.  By              */
/*                convention minor 0 is the console.                         */
/*                  Also note that much of the normal UNIX tty cooked        */
/*                mode code is built in here.                                */
void ConsoleWrite(int fildes, char c)

{
  int mdev;                        /* The minor number to mess with          */
  int i;                           /* Used to clear the screen               */

  /* For now, get the minor number for the console from a global             */
  mdev = console_minor;            /* Read it now                            */

  /* Make sure we have a valid device                                        */
  if ((mdev < 0) || (mdev >= MAXMINOR))
    return;

  /* If we are in 8 bit mode just write the byte and return                  */
  if (minors[mdev]->flag1 & F1BIT8)
    {
    _Writechar(mdev, c);
    return;
    }

  /* Do special processing                                                   */
  special(mdev, c);
}



/* ConsoleRead - See the comments on ConsoleWrite to understand minor        */
/*               usage.  Basically this guy just returns.                    */
char ConsoleRead(int fildes)

{
  int mdev;                        /* The minor number to mess with          */
  int i;                           /* Used to clear the screen               */
  char c;                          /* Holds the byte we just read            */

  /* For now, get the minor number for the console from a global             */
  mdev = console_minor;            /* Read it now                            */

  /* Make sure we have a valid device                                        */
  if ((mdev < 0) || (mdev >= MAXMINOR))
    return(-1);

  /* Get the character & fix bit 8                                           */
  c = _Readchar(mdev);
  if (!minors[mdev]->flag1 & F1BIT8)
    c &= 0x7F;                     /* Strip off top bit most of the time     */

  /* Apply special processing                                                */
  if (c == 15)                     /* Are we looking at ^O?                  */
    {                              /* YES - toggle DEC dump output bit       */
    minors[mdev]->flag1 ^= F1NOOUT;
    c = 0;                         /* Pretend it is a nothing                */
    }

  /* Echo the character                                                      */
  c = special(mdev, c);

  /* All done                                                                */
  return(c);
}

/* ConsoleInit - Fill in the POSIX stuff that we really are not using right  */
/*               now.....                                                    */
int ConsoleInit(int fildes)
{

        /*
         *  Default terminal settings are ignore parity, strip, convert
         *  newlines to/from carriage returns, 7 bit characters, echo
         *  characters, and canonical mode.
         */

/*
        DDESC.t.c_iflag = IGNPAR | ISTRIP | ICRNL;
        DDESC.t.c_cflag = CSIZE  | CS7;
        DDESC.t.c_lflag = ECHO   | ECHOE  | ICANON;
        DDESC.t.c_oflag = 0;
*/
        return( 1 );
}

#define ASYNC  0		/* Put these defines here for now don't forget*/
#define S3     1
#define WEITEK 2
#define WEITEK_CONSOLE  11
#define WEITEK_CON1	 6
#define WEITEK_CON2	 7
#define S3_CONSOLE      10
#define S3_CON1		 4
#define S3_CON2		 5


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// getch	- Get a char from stdin without echo
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

getch(void)
{
return(_Readchar(console_minor));
}
#endif /*DEBUG*/
