/* @(#)31	1.1  src/rspc/usr/lib/boot/softros/include/termios.h, rspc_softros, rspc411, GOLD410 4/18/94 15:53:39  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: none
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

#ifndef _TERMIOS
#define _TERMIOS

#define NCCS	16

typedef unsigned short	tcflag_t;
typedef unsigned char	cc_t;

struct termios {
    tcflag_t	c_iflag;
    tcflag_t	c_oflag;
    tcflag_t	c_cflag;
    tcflag_t	c_lflag;
    char	c_line;
    cc_t	c_cc[ NCCS ];
};

/* Control Characters */

#define VINTR     0
#define VQUIT     1
#define VERASE    2
#define VKILL     3
#define VEOF      4
#define VEOL      5
#define VSTART    7
#define VSTOP     8
#define VSUSP     9

/* Baud Rates: */

#define B0      0x00000000
#define B50     0x00000001
#define B75     0x00000002
#define B110    0x00000003
#define B134    0x00000004
#define B150    0x00000005
#define B200    0x00000006
#define B300    0x00000007
#define B600    0x00000008
#define B1200   0x00000009
#define B1800   0x0000000a
#define B2400   0x0000000b
#define B4800   0x0000000c
#define B9600   0x0000000d
#define B19200  0x0000000e
#define B38400  0x0000000f

/* c_iflag bits */

#define IGNBRK  0x00000001
#define BRKINT  0x00000002
#define IGNPAR  0x00000004
#define PARMRK  0x00000008
#define INPCK   0x00000010
#define ISTRIP  0x00000020
#define INLCR   0x00000040
#define IGNCR   0x00000080
#define ICRNL   0x00000100
#define IXON    0x00000200
#define IXOFF   0x00000400

/* c_oflag bits */

#define OPOST   0x00000001

/* c_cflag bits */

#define CSIZE   0x00000030
#define CS5     0x00000000
#define CS6     0x00000010
#define CS7     0x00000020
#define CS8     0x00000030
#define CSTOPB  0x00000040
#define CREAD   0x00000080
#define PARENB  0x00000100
#define PARODD  0x00000200
#define HUPCL   0x00000400
#define CLOCAL  0x00000800

/* c_lflag bits */

#define ISIG    0x00000001
#define ICANON  0x00000002
#define ECHO    0x00000008
#define ECHOE   0x00000010
#define ECHOK   0x00000020
#define ECHONL  0x00000040
#define NOFLSH  0x00000080
#define TOSTOP  0x00010000
#define IEXTEN  0x00200000


/* function prototypes */

extern int tcgetattr( int, struct termios * );
extern int tcsetattr( int, int option, struct termios * );

#endif		/* _TERMIOS */
